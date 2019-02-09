#version 430

#include "MathUtils/RangeMap.glsl"

#define MAX_LIGHTS 8
#define SHADOW_Z_BIAS 0.25

#define DRAW_SPHERE_FOG
//#define DRAW_NOISE_FOG

layout(binding=1, std140) uniform CameraUBO
{
	mat4 VIEW;
	mat4 PROJECTION;
	vec3 EYE_POSITION;
	float PADDING;
};

struct Light		// std140
{
	vec3 position;
	int castsShadows;
	vec4 colorAndIntensity;
	vec3 attenuation;
	float padding0;
	vec3 direction;
	float padding1;
	float directionFactor;
	float coneFactor;
	float innerAngle;
	float outerAngle;
	mat4 lightView;
	mat4 lightProjection;
};

layout(binding=2, std140) uniform ActiveLights
{
	vec4 ambientLightColorAndIntensity;
	Light lights[ MAX_LIGHTS ];
};

layout(binding = 0) uniform sampler2D texFullscreen;
layout(binding = 1) uniform sampler2D shadowMap;

layout(binding = 8) uniform sampler3D noiseShape;

in vec4 PASSCOLOR;
in vec2 PASSUV;

out vec4 outColor; 

vec3 GetCameraPostion()
{
	return EYE_POSITION;
}

vec3 GetCameraRight()
{
	vec3 right = vec3(
		VIEW[0].x,
		VIEW[1].x,
		VIEW[2].x
	);
	return right;
}

vec3 GetCameraUp()
{
	vec3 up = vec3(
		VIEW[0].y,
		VIEW[1].y,
		VIEW[2].y
	);
	return up;
}

vec3 GetCameraForward()
{
	vec3 forward = vec3(
		VIEW[0].z,
		VIEW[1].z,
		VIEW[2].z
	);
	return forward;
}

vec3 ScreenToWorld(float zDistance)	// uses PASSUV as input
{
	vec2 ndcXY = RangeMapVec2(PASSUV, vec2(0.0), vec2(1.0), vec2(-1.0), vec2(1.0));

	vec3 viewSpaceXYZ;
	float xyMultiplier = zDistance;
	viewSpaceXYZ.x = ( ndcXY.x / PROJECTION[0].x ) * xyMultiplier;
	viewSpaceXYZ.y = ( ndcXY.y / PROJECTION[1].y ) * xyMultiplier;
	viewSpaceXYZ.z = zDistance;

	vec4 worldSpaceXYZW = inverse(VIEW) * vec4(viewSpaceXYZ,1.0);
	vec3 worldPos = worldSpaceXYZW.xyz;

	return worldPos;
}

vec3 NDCToView(float NDCZ)
{
	vec3 ndc = vec3( RangeMapVec2(PASSUV, vec2(0.0), vec2(1.0), vec2(-1.0), vec2(1.0)), NDCZ );
	vec4 ndcW = vec4( ndc, 1.0 );
	ndcW = inverse( PROJECTION ) * ndcW;
	return ndcW.xyz / ndcW.w;
}

vec3 NDCToWorld(float NDCZ)
{
	vec3 viewSpaceXYZ = NDCToView(NDCZ);
	vec4 worldSpaceXYZW = inverse(VIEW) * vec4(viewSpaceXYZ,1.0);
	vec3 worldPos = worldSpaceXYZW.xyz;

	return worldPos;
}

float GetNDCZ()
{
	float zValue = texture( shadowMap, PASSUV ).r;
	zValue = RangeMapFloat( zValue, 0.0, 1.0, -1.0, 1.0 );
	return zValue;
}

float RaySphereIntersection( vec3 sphereCenter, float sphereRadius )	// Returns 1 if the ray intersects
{
	float radiusSquared = sphereRadius * sphereRadius;

	vec3 start = GetCameraPostion();
	vec3 rayDir = ScreenToWorld( 1.0 ) - ScreenToWorld( 0.1 );

	vec3 toCenter = sphereCenter - start;
	float toCenterLengthSquared = ( toCenter.x * toCenter.x ) + ( toCenter.y * toCenter.y ) + ( toCenter.z * toCenter.z );
	if ( toCenterLengthSquared - radiusSquared > 0.0 )	// Is the start point outside the sphere?
	{
		vec3 perpendicular = cross( rayDir, toCenter );
		float perpendicularLengthSquared = ( perpendicular.x * perpendicular.x ) + ( perpendicular.y * perpendicular.y ) + ( perpendicular.z * perpendicular.z );
		float difference = radiusSquared - perpendicularLengthSquared;
		difference = clamp( difference, 0.0, 1.0 );
		return difference;
	}
	else
	{
		return 1.0;
	}
}

// Demo credits: https://www.shadertoy.com/view/XlBSRz - Sebastien Hillaire

#define D_DEMO_FREE

#define D_FOG_NOISE 1.0

// Height fog multiplier to show off improvement with new integration formula
#define D_STRONG_FOG 0.0

// Enable/disable volumetric shadow (single scattering shadow)
#define D_VOLUME_SHADOW_ENABLE 1

#define SPHERE_CENTER vec3( 0.0, 0.0, 0.0 )
#define SPHERE_RADIUS 5.0

vec3 getSceneColor()
{
	return texture2D(texFullscreen, PASSUV).xyz;
}

float getClosestDistance(vec3 p)
{
	float minD = 50.0;

	float d = max(20.0, -p.y);
	minD = min(minD, d);

	d = max(0.0,20.0-p.y);
	minD = min(minD, d);
	
	d = max(80.0,-p.x);
	minD = min(minD, d);

	d = max(0.0,80.0-p.x);
	minD = min(minD, d);

	d = max(80.0,-p.z);
	minD = min(minD, d);

	d = max(0.0,80.0-p.z);
	minD = min(minD, d);

	return minD;
}

vec3 evaluateLight(in vec3 pos)
{
	vec3 lightPos = lights[1].position;
	vec3 lightCol = lights[1].colorAndIntensity.xyz * lights[1].colorAndIntensity.w;
	vec3 L = lightPos-pos;
	return lightCol * 1.0/dot(L,L);
}

void getParticipatingMedia(out float muS, out float muE, in vec3 pos)
{
	const float sphereRadius = SPHERE_RADIUS;
	float sphereFog = 0.0;
	float muA = 0.0;

#ifdef DRAW_SPHERE_FOG
	sphereFog = clamp((sphereRadius-length(pos-SPHERE_CENTER))/sphereRadius, 0.0,1.0);
	sphereFog = (1.0-(1.0-sphereFog)*(1.0-sphereFog));
	muA = 0.1f * sphereFog;
#endif

	vec3 maxDistance = vec3( 1.0 );
	vec3 displacement = pos - vec3( 0.0 );
	vec3 distance = clamp( abs(displacement), vec3( 0.0 ), maxDistance );
	vec3 distanceFactors = (maxDistance - distance)/maxDistance;
	distanceFactors = ceil( distanceFactors );

	float noiseFog = 0.0;
#ifdef DRAW_NOISE_FOG
	vec3 uvw = clamp( displacement, -maxDistance, maxDistance );
	uvw = RangeMapVec3( uvw, -maxDistance, maxDistance, vec3( 0.0 ), vec3( 1.0 ) );
	vec4 texelColor = texture( noiseShape, uvw );
	noiseFog = 0.25 * texelColor.r + 0.25 * texelColor.g + 0.25 * texelColor.b + 0.25 * texelColor.a;
	noiseFog *= min( distanceFactors.x, min( distanceFactors.y, distanceFactors.z ) );
#endif

	const float constantFog = 0.0;
	muS = constantFog + sphereFog + noiseFog;

	
	muE = max(0.000000001, muA + muS); // to avoid division by zero extinction
}

float phaseFunction()
{
	return 1.0/(4.0*3.14);
}

float volumetricShadow(in vec3 from, in vec3 to)
{
	const float numStep = 16.0; // quality control. Bump to avoid shadow alisaing
	float shadow = 1.0;
	float muS = 0.0;
	float muE = 0.0;
	float dd = length(to-from) / numStep;
	for(float s=0.5; s<(numStep-0.1); s+=1.0)// start at 0.5 to sample at center of integral part
	{
		vec3 pos = from + (to-from)*(s/(numStep));
		getParticipatingMedia(muS, muE, pos);
		shadow *= exp(-muE * dd);
	}
	return shadow;
}

void traceScene(out vec3 finalPos, out vec3 albedo, out vec4 scatTrans)
{
	float iterNum = 0.0;
	const float numIter = 200.0;// * RaySphereIntersection( SPHERE_CENTER, SPHERE_RADIUS );

	const float stepLength = 0.1;

    float muS = 0.0;
    float muE = 0.0;

    vec3 lightPos = lights[1].position;

    // Initialise volumetric scattering integration (to view)
    float transmittance = 1.0;
    vec3 scatteredLight = vec3(0.0, 0.0, 0.0);

	float d = 0.1; // hack: always have a first step of 1 unit to go further
	vec3 p = vec3(0.0, 0.0, 0.0);
    float dd = 0.0;
	while ( iterNum < numIter )
	{
		vec3 p = ScreenToWorld(d);
		getParticipatingMedia(muS, muE, p);

		// See slide 28 at http://www.frostbite.com/2015/08/physically-based-unified-volumetric-rendering-in-frostbite/
		// S - ideal incoming light to the current point, opposite the view direction
		vec3 idealLight = evaluateLight(p) * muS * phaseFunction() * volumetricShadow(p,lightPos); // Volumetric shadow handles the relative direction of light here

		// S * e^(-muE * stepLength) = idealLight * extinction function = light remaining after marching through the stepLength towards the viewer
		// This / muE -> Unknown, need to figure out -> Perhaps cancelling out the muS above?
		float extinctionFunction = exp(-muE * stepLength);
		vec3 remainingLight = idealLight * extinctionFunction; 
		vec3 inScattering = (idealLight - remainingLight) / muE; // integrate along the current step segment

		// Accumulate total scattered light over this stepLength
		scatteredLight += transmittance * inScattering; // accumulate and also take into account the transmittance from previous steps

		// Decrease transmittance over the step
		transmittance *= extinctionFunction;

		dd = getClosestDistance(p);
		if ( dd < 1.0 )
		{
			break;
		}

		d += stepLength;

		iterNum += 1.0;
	}

	albedo = getSceneColor();

    finalPos = ScreenToWorld(min(d, NDCToView(GetNDCZ()).z));

    scatTrans = vec4(scatteredLight, transmittance);
}

void main( void )
{
	vec2 uv = PASSUV;

	vec3 finalPos;
	vec3 albedo = vec3( 0.0, 0.0, 0.0 );
    vec4 scatTrans = vec4( 0.0, 0.0, 0.0, 0.0 );
    traceScene(finalPos, albedo, scatTrans);

    vec3 color = (albedo/3.14) * volumetricShadow(finalPos, lights[1].position);
    color = color * scatTrans.w + scatTrans.xyz;

	color = pow(color, vec3(1.0/2.2)); // simple linear to gamma, exposure of 1.0

	outColor = vec4(color, 1.0);
}
