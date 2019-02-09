#version 430

#include "MathUtils/RangeMap.glsl"

#define PI 3.142f
#define EPSILON 0.000000001f
#define RADIANS_TO_DEGREES 180.0f / 3.142f
#define GAMMA_EXPONENT 1.0f / 2.2f

#define DIRECTIONAL_LIGHT_INDEX 0
#define MAX_LIGHTS 8
#define NEAR_Z 0.1f
#define FAR_Z 5000.0f

#define RAYLEIGH_MOLECULAR_DENSITY ( 2.504f * 1e25 )
#define RAYLEIGH_SCALE_HEIGHT 2500.0f
#define MIE_MOLECULAR_DENSITY ( 2.504f * 1e25 )
#define MIE_SCALE_HEIGHT 1000.0f

#define RED_LIGHT_WAVELENGTH ( 685.0f * 1e-9 )
#define GREEN_LIGHT_WAVELENGTH ( 532.5f * 1e-9 )
#define BLUE_LIGHT_WAVELENGTH ( 472.5f * 1e-9 )

#define RAYMARCH_LENGTH 25000.0f
#define RAYMARCH_NUM_STEPS 40.0f
#define OUT_SCATTERING_NUM_STEPS 4.0f

//#define MIE_USE_LOOKUP

layout ( binding = 1, std140 ) uniform CameraUBO
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

layout( binding = 2, std140 ) uniform ActiveLights
{
	vec4 ambientLightColorAndIntensity;
	Light lights[ MAX_LIGHTS ];
};

layout ( binding = 9, std140 ) uniform Config
{
	float ATMOSPHERE_LAYER_ALTITUDE;
	float ATMOSPHERE_LAYER_HEIGHT;
	float SUNLIGHT_INTENSITY;
	float TERRAIN_HEIGHT;
	float MIE_PHASE_FUNCTION_MAX_VALUE;
};

layout ( binding = 0 ) uniform sampler2D texFullscreen;
layout ( binding = 1 ) uniform sampler2D shadowMap;
layout ( binding = 2 ) uniform sampler2D mieLUT;

in vec2 PASSUV;

out vec4 outColor; 

Light GetSun()
{
	return lights[ DIRECTIONAL_LIGHT_INDEX ];
}

vec3 GetDirectLight( vec3 pos )
{
	vec4 light = GetSun().colorAndIntensity;
	light.xyz *= SUNLIGHT_INTENSITY;

	vec3 displacement = pos - GetSun().position;
	float dist = length( displacement );
	float attenuation = GetSun().attenuation.x + ( dist * GetSun().attenuation.y ) + ( dist * dist * GetSun().attenuation.z );
	attenuation = max( attenuation, 1.0f );

	light /= attenuation;
	light.w = 1.0f;
	return light.xyz;
}

vec3 ScreenToWorld( float zDist )
{
	vec3 viewPos = vec3( 0.0f, 0.0f, zDist );
	vec4 ndc = PROJECTION * vec4( viewPos, 1.0f );
	ndc /= ndc.w;

	vec2 ndcXY = RangeMapVec2( PASSUV, vec2( 0.0f ), vec2( 1.0f ), vec2( -1.0f ), vec2( 1.0f ) );
	ndc.xy = ndcXY;

	mat4 inverseProjection = inverse( PROJECTION );
	vec4 viewPosW = inverseProjection * ndc;
	viewPosW /= viewPosW.w;
	viewPos = viewPosW.xyz;

	vec4 worldPosW = ( inverse( VIEW ) * vec4( viewPos, 1.0f ) );
	vec3 worldPos = worldPosW.xyz;
	return worldPos;
}

vec3 GetSunlightDirection( vec3 pos )
{
	return normalize( pos - GetSun().position );
}

float RayleighDensity( vec3 pos )
{
	float altitude = max( length( pos ) - TERRAIN_HEIGHT, 0.0f );
	return exp( -altitude / RAYLEIGH_SCALE_HEIGHT );
}

float RayleighPhaseFunction( vec3 viewDir, vec3 pos )
{
	vec3 lightDir = GetSunlightDirection( pos );
	float cosTheta = dot( lightDir, -viewDir );
	return ( 3.0f / ( 16.0f * PI ) * ( 1.0f + ( cosTheta * cosTheta ) ) );
}

float MieDensity( vec3 pos )
{
	float altitude = max( length( pos ) - TERRAIN_HEIGHT, 0.0f );
	return exp( -altitude / MIE_SCALE_HEIGHT );
}

vec3 MiePhaseFunction( vec3 viewDir, vec3 pos )
{
	vec3 lightDir = GetSunlightDirection( pos );
	float cosTheta = dot( lightDir, -viewDir );
	float theta = acos( cosTheta );
	theta = theta * RADIANS_TO_DEGREES * 10.0f;	// 1800 entries
	vec2 uv = vec2(
		RangeMapFloat( theta, 0.0f, 1800.0f, 0.0f, 1.0f ),
		0.0f
	);

	vec3 phaseFunction = vec3( 1.0f );
#ifdef MIE_USE_LOOKUP
	phaseFunction = texture( mieLUT, uv ).xyz;
	phaseFunction *= MIE_PHASE_FUNCTION_MAX_VALUE;
#else
	// From https://www.shadertoy.com/view/lslXDr
	float g = 0.78f;
	float gg = g * g;
	float cc = cosTheta * cosTheta;

	float a = ( 1.0f - gg ) * ( 1.0f + cc );
	float b = 1.0f + gg - 2.0f * g * cosTheta;
	b *= sqrt( b );
	b *= 2.0f + gg;

	phaseFunction = vec3( ( 3.0f / 8.0f / PI ) * ( a / b ) );

#endif
	return phaseFunction;
}

float OutScatteringRayleighDensity( vec3 startPos, vec3 endPos )
{
	float density = 0.0f;
	float dist = length( endPos - startPos );
	vec3 lightDir = normalize( endPos - startPos );

	float stepLength = dist / OUT_SCATTERING_NUM_STEPS;
	startPos += 0.5f * stepLength;
	float currentDist = 0.0f;

	while ( currentDist < dist )
	{
		vec3 pos = startPos + ( lightDir * currentDist );
		density += RayleighDensity( pos );
		currentDist += stepLength;
	}

	density *= stepLength;
	//density = clamp( density, 0.0f, 1.0f );
	return density;
}

float OutScatteringMieDensity( vec3 startPos, vec3 endPos )
{
	float density = 0.0f;
	float dist = length( endPos - startPos );
	vec3 lightDir = normalize( endPos - startPos );

	float stepLength = dist / OUT_SCATTERING_NUM_STEPS;
	startPos += 0.5f * stepLength;
	float currentDist = 0.0f;

	while ( currentDist < dist )
	{
		vec3 pos = startPos + ( lightDir * currentDist );
		density += MieDensity( pos );
		currentDist += stepLength;
	}

	density *= stepLength;
	//density = clamp( density, 0.0f, 1.0f );
	return density;
}

bool RaySphereIntersect( float radius, vec3 startPos, vec3 dir, out float minRoot, out float maxRoot )
{
	float a = dot( dir, dir );
	float b = 2.0f * dot( startPos, dir );
	float c = dot( startPos, startPos ) - ( radius * radius );

	float discriminant = b * b - 4.0f * a * c;
	if ( discriminant < 0.0f )
	{
		minRoot = 0.1f;
		maxRoot = 0.1f;
		return false;
	}

	float discriminantSqrt = sqrt( discriminant );
	minRoot = ( -b - discriminantSqrt ) / ( 2.0f * a );
	minRoot = max( minRoot, 0.1f );
	maxRoot = ( -b + discriminantSqrt ) / ( 2.0f * a );
	maxRoot = max( maxRoot, 0.1f );

	return true;
}

vec3 GetSkyColor()
{
	vec3 startPos = ScreenToWorld( NEAR_Z );
	vec3 viewDir = normalize( ScreenToWorld( 10.0f ) - startPos );
	float atmosphereOuterRadius = TERRAIN_HEIGHT + RAYMARCH_LENGTH;

	//vec3 scatCoeffRayleigh = vec3( 0.2f, 3.f, 10.f );
	vec3 scatCoeffRayleigh = vec3( 3.8f, 13.5f, 33.1f );
	vec3 scatCoeffMie = vec3( 21.0f );
	float mieExp = 1.1f;

	float nearAtmos;
	float farAtmos;
	float nearTerr;
	float farTerr;
	// Why this won't work: Rayleigh/Mie particles are lost at height
	RaySphereIntersect( atmosphereOuterRadius, ScreenToWorld( NEAR_Z ), viewDir, nearAtmos, farAtmos );
	RaySphereIntersect( TERRAIN_HEIGHT, ScreenToWorld( NEAR_Z ), viewDir, nearTerr, farTerr );
	float minDistance = max( farTerr, 0.1f );
	float maxDistance = farAtmos;
	//maxDistance = max( maxDistance, 0.1f );

	float stepLength = max( maxDistance - minDistance, 0.0f ) / RAYMARCH_NUM_STEPS;
	float dist = maxDistance - 0.5f * stepLength;

	float rayleighStepExtinction = 0.0f;
	float mieStepExtinction = 0.0f;

	vec3 sumRayleighDensityStep = vec3( 0.0f );
	vec3 sumMieDensityStep = vec3( 0.0f );
	while ( dist > minDistance )
	{
		vec3 pos = startPos + ( dist * viewDir );

		float rayleighDenStep = RayleighDensity( pos ) * stepLength;
		rayleighStepExtinction += rayleighDenStep * 0.00005f;

		float mieDenStep = MieDensity( pos ) * stepLength;
		mieStepExtinction += mieDenStep * 0.0001f;

		float near, far;
		vec3 sunDir = -GetSunlightDirection( pos );
		RaySphereIntersect( atmosphereOuterRadius, pos, sunDir, near, far );

		vec3 farPos = pos + ( sunDir * far );
		float rayleighOutScatteringExtinction = OutScatteringRayleighDensity( pos, farPos ) * 0.00005f;
		float mieOutScatteringExtinction = OutScatteringMieDensity( pos, farPos ) * 0.00005f;

		vec3 attenuation = exp( -( rayleighStepExtinction + rayleighOutScatteringExtinction ) * scatCoeffRayleigh - ( mieStepExtinction + mieOutScatteringExtinction ) * scatCoeffMie * mieExp );

		sumRayleighDensityStep += rayleighDenStep * attenuation;
		sumMieDensityStep += mieDenStep * attenuation;

		dist -= stepLength;
	}

	vec3 finalPos = startPos + ( maxDistance * viewDir ); 
	vec3 scatteredLight = (
		( sumRayleighDensityStep * scatCoeffRayleigh * RayleighPhaseFunction( viewDir, finalPos ) )
		+
		( sumMieDensityStep * scatCoeffMie * MiePhaseFunction( viewDir, finalPos ) )
	);
	scatteredLight *= 0.00025f;
	scatteredLight = clamp( scatteredLight, vec3( 0.0f ), vec3( 1.0f ) );
	return scatteredLight;
}

vec3 GetSceneColor()
{
	vec3 sceneColor = texture( texFullscreen, PASSUV ).xyz;

	float depthValue = texture( shadowMap, PASSUV ).r;
	depthValue = floor( depthValue );
	vec3 skyColor = GetSkyColor();
	//return skyColor;
	return mix( sceneColor, skyColor, depthValue );
}

void main( void )
{
	outColor = vec4( GetSceneColor(), 1.0f );
}
