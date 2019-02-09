#version 430

#include "MathUtils/AngleUtils.glsl"
#include "MathUtils/RandomUtils.glsl"
#include "MathUtils/RangeMap.glsl"

#define PI 3.142f
#define EPSILON 0.000000001f
#define RADIANS_TO_DEGREES 180.0f / 3.142f

#define DIRECTIONAL_LIGHT_INDEX 0
#define MAX_LIGHTS 8
#define NEAR_Z 0.1f
#define FAR_Z 5000.0f

#define CLOUD_ABSORPTION_COEFFICIENT 0.0f

#define SHADOW_NUM_STEPS 4.0f
#define RAYMARCH_NUM_STEPS 200.0f
#define RAYMARCH_MIN_FORWARD 0.1f
#define RAYMARCH_STEP_LENGTH 0.1f
#define RAYMARCH_STEP_CHANGE_DENSITY_THRESHOLD 0.01f
#define RAYMARCH_DISTANCE_STEP_THRESHOLD 20.0f
#define TRANSMITTANCE_EXIT_VALUE 0.01f
#define HENYEY_GREENSTEIN_G_VALUE 0.8f

#define PHASE_FUNCTION_USE_MIE

// Optimization - do not use amortization without temporal upsampling
#define ENABLE_AMORTIZATION
#define AMORTIZATION_FACTOR 2
//#define AMORTIZE_INTERLACED
//#define AMORTIZE_RADIAL
#define REPROJECTION_BLEND_FACTOR 0.f

layout ( binding = 0, std140 ) uniform TimeUBO
{
	float CURRENT_TIME;
	float LAST_FRAME_TIME;
	float FPS;
	int FRAME_COUNT;
};

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
	mat4 OLD_VIEW;					// For Temporal upsampling
	float RAYMARCH_OFFSET_FRACTION;	// For Temporal upsampling
	float TERRAIN_HEIGHT;
	float CLOUD_LAYER_ALTITUDE;
	float CLOUD_LAYER_HEIGHT;
	float SHADOWS_ENABLED;
	float TEMPORAL_UPSAMPLING_ENABLED;
	float WEATHER_TEXTURE_TILE_FACTOR;
	float SHAPE_TEXTURE_TILE_FACTOR;
	float DETAIL_TEXTURE_TILE_FACTOR;
	float DETAIL_TEXTURE_PANNING;
	float DETAIL_TEXTURE_OFFSET;
	float SHAPE_WORLEY_INVERSE;
	float DETAIL_WORLEY_INVERSE;
	float MAX_DENSITY;
	float SUNLIGHT_INTENSITY;
	float MIE_PHASE_FUNCTION_MAX_VALUE;
	float DEBUG_VIEW_LODS;
};

layout ( binding = 0 ) uniform sampler2D weather;
layout ( binding = 1 ) uniform sampler2D heightSignal;
layout ( binding = 2 ) uniform sampler3D noiseShape;
layout ( binding = 3 ) uniform sampler3D noiseDetail;
layout ( binding = 4 ) uniform sampler2D mieLUT;
layout ( binding = 5 ) uniform sampler2D prevScatTrans;
layout ( binding = 6 ) uniform sampler2D depthTarget;

in vec2 PASSUV;

out vec4 outScatTrans;
out vec4 outVolShadow;

Light GetSun()
{
	return lights[ DIRECTIONAL_LIGHT_INDEX ];
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

vec3 GetRayDirection()
{
	return normalize( ScreenToWorld( 10.0f ) - ScreenToWorld( 1.0f ) );
}

vec3 GetRayFinalViewPosition()
{
	float depthValue = texture( depthTarget, PASSUV ).x;
	float ndcZ = RangeMapFloat( depthValue, 0.0f, 1.0f, -1.0f, 1.0f );
	vec2 ndcXY = RangeMapVec2( PASSUV, vec2( 0.0f ), vec2( 1.0f ), vec2( -1.0f ), vec2( 1.0f ) );
	vec3 ndc = vec3( ndcXY, ndcZ );

	mat4 inverseProjection = inverse( PROJECTION );
	vec4 viewPos = inverseProjection * vec4( ndc, 1.0f );
	viewPos /= viewPos.w;
	viewPos.w = 1.0f;

	viewPos.z = min( viewPos.z, FAR_Z );

	return viewPos.xyz;
}

vec3 GetRayFinalWorldPosition()
{
	vec3 viewPos = GetRayFinalViewPosition();

	mat4 inverseView = inverse( VIEW );
	vec4 worldPos = inverseView * vec4( viewPos, 1.0f );

	return worldPos.xyz;
}

vec4 GetLightAtPosition( vec3 worldPosition )
{
	vec4 light = GetSun().colorAndIntensity;
	light.xyz *= SUNLIGHT_INTENSITY;

	vec3 displacement = worldPosition - GetSun().position;
	float dist = length( displacement );
	float attenuation = GetSun().attenuation.x + ( dist * GetSun().attenuation.y ) + ( dist * dist * GetSun().attenuation.z );
	attenuation = max( attenuation, 1.0f );

	light /= attenuation;
	light.w = 1.0f;
	return light;
}

float GetWorldPosHash( vec3 worldPos )
{
	float hash = worldPos.x + 3971.351f * worldPos.y + 7341.8246f * worldPos.z;
	return PseudoRand( hash );
}

/*
	 For the raymarch, (pos + t * viewDir), this function returns t for a sphere centered at the origin with the provided radius.
	 DERIVATION
	 The line (pos + t * viewDir) intersects the sphere ( R = radius ).
	 The equation for this intersection is
	 ( pos + t * viewDir - center )^2 = radius^2
	 -> pos^2 + 2 * pos * t * viewDir + t^2 * viewDir^2 + center^2 - 2* center * pos - 2 * center * t * viewDir = radius^2
	 -> ( viewDir^2 ) * t^2 + ( 2 * pos * viewDir - 2 * center * viewDir ) * t + ( center^2 + pos^2 - radius^2 - 2 * center * pos ) = 0
	 -> a = ( viewDir^2 )
	 -> b = ( 2 * pos * viewDir - 2 * center * viewDir )
	 -> c = ( center^2 + pos^2 - radius^2 - 2 * center * pos )
	 ==> t = ( -b (+/-) sqrt( b^2 - 4 * a * c ) )/( 2 * a )
	 Note that viewDir is never zero owing to it being a direction vector.
	 In this case, vector multiplication is achieved through dot product.
*/
bool RaySphereIntersect( float radius, vec3 startPos, vec3 dir, out float minRoot, out float maxRoot )
{
	float a = dot( dir, dir );
	float b = 2.0f * dot( startPos, dir );
	float c = dot( startPos, startPos ) - ( radius * radius );

	float discriminant = b * b - 4.0f * a * c;
	if ( discriminant < 0.0f )
	{
		minRoot = RAYMARCH_MIN_FORWARD;
		maxRoot = RAYMARCH_MIN_FORWARD;
		return false;
	}

	float discriminantSqrt = sqrt( discriminant );
	minRoot = ( -b - discriminantSqrt ) / ( 2.0f * a );
	minRoot = max( minRoot, RAYMARCH_MIN_FORWARD );
	maxRoot = ( -b + discriminantSqrt ) / ( 2.0f * a );
	maxRoot = max( maxRoot, RAYMARCH_MIN_FORWARD );

	return true;
}

float GetNormalizedHeight( vec3 pos )
{
	float height = length( pos );
	height = clamp( height, CLOUD_LAYER_ALTITUDE, ( CLOUD_LAYER_ALTITUDE + CLOUD_LAYER_HEIGHT ) );
	return RangeMapFloat( height, CLOUD_LAYER_ALTITUDE, ( CLOUD_LAYER_ALTITUDE + CLOUD_LAYER_HEIGHT ), 0.0f, 1.0f );
}

vec3 GetSampleUVW( vec3 pos )
{
	// Map x, y, z to the hemispherical dome of radius between CLOUD_LAYER_ALTITUDE and ( CLOUD_LAYER_ALTITUDE + 2.0f * CLOUD_LAYER_HEIGHT )
	vec3 uvw;
	pos.y = max( pos.y, TERRAIN_HEIGHT );
	float radius = length( pos );
	radius = clamp( radius, CLOUD_LAYER_ALTITUDE, ( CLOUD_LAYER_ALTITUDE + 2.0f * CLOUD_LAYER_HEIGHT ) );
	uvw.y = RangeMapFloat( radius, CLOUD_LAYER_ALTITUDE, ( CLOUD_LAYER_ALTITUDE + 2.0f * CLOUD_LAYER_HEIGHT ), 0.0f, 1.0f );
	
	// Ellipse formed by the intersection of the terrain with a sphere of radius "radius"
	float theta = asin( TERRAIN_HEIGHT / radius );
	float a = radius * cos( theta );
	float b = radius - TERRAIN_HEIGHT;
	float arcLength = a * b;	// Something complicated... skip for now

	// Viewing the hemisphere from above
	vec2 uvMappedXZ = CartesianToPolar( pos.xz );
	uvMappedXZ.x = clamp( uvMappedXZ.x, 0.0f, a );
	uvMappedXZ = PolarToCartesian( uvMappedXZ );
	uvMappedXZ = RangeMapVec2( uvMappedXZ, vec2( -a ), vec2( a ), vec2( 0.0f ), vec2( 1.0f ) );

	uvw.x = uvMappedXZ.x;
	uvw.z = uvMappedXZ.y;

	return uvw;	// Return spherical pos
}

float GetMipLevel( vec3 pos )
{
	vec4 viewPos = VIEW * vec4( pos, 1.0f );
	float z = viewPos.z;
	z = RangeMapFloat( z, -1.0f, 1.0f, 0.0f, 1.0f );
	float mipLevel = mix( 0.0f, 7.0f, max( sign(z - 400.0f ), 0.0f ) );
	mipLevel = mix( mipLevel, 2.0f, max( sign(z - 500.0f ), 0.0f ) );
	mipLevel = mix( mipLevel, 3.0f, max( sign(z - 600.0f ), 0.0f ) );
	mipLevel = mix( mipLevel, 4.0f, max( sign(z - 700.0f ), 0.0f ) );
	mipLevel = mix( mipLevel, 5.0f, max( sign(z - 800.0f ), 0.0f ) );
	mipLevel = mix( mipLevel, 6.0f, max( sign(z - 900.0f ), 0.0f ) );
	mipLevel = mix( mipLevel, 7.0f, max( sign(z - 1000.0f ), 0.0f ) );
	mipLevel = mix( mipLevel, 8.0f, max( sign(z - 5000.0f ), 0.0f ) ); // Invalid - will color as magenta
	return mipLevel;
}

vec4 GetMipLevelColor( float mipLevel )
{
	vec4 color = vec4( 1.0f );
	color = mix( color, vec4( 0.25f, 0.0f, 0.0f, 1.0f ), max( sign( mipLevel - 0.999f ), 0.0f ) );
	color = mix( color, vec4( 0.4f, 0.0f, 0.0f, 1.0f ), max( sign( mipLevel - 1.999f ), 0.0f ) );
	color = mix( color, vec4( 0.5f, 0.0f, 0.0f, 1.0f ), max( sign( mipLevel - 2.999f ), 0.0f ) );
	color = mix( color, vec4( 0.6f, 0.0f, 0.0f, 1.0f ), max( sign( mipLevel - 3.999f ), 0.0f ) );
	color = mix( color, vec4( 0.7f, 0.0f, 0.0f, 1.0f ), max( sign( mipLevel - 4.999f ), 0.0f ) );
	color = mix( color, vec4( 0.8f, 0.0f, 0.0f, 1.0f ), max( sign( mipLevel - 5.999f ), 0.0f ) );
	color = mix( color, vec4( 1.0f, 0.0f, 0.0f, 1.0f ), max( sign( mipLevel - 6.999f ), 0.0f ) );
	color = mix( color, vec4( 1.0f, 0.0f, 1.0f, 1.0f ), max( sign( mipLevel - 7.999f ), 0.0f ) );
	return color;
}

float HeightSignalParabolic( vec2 heightAndAltitude, vec3 pos, vec3 uvw )
{
	if ( abs( heightAndAltitude.x - 0.0f ) < EPSILON )
	{
		return 0.0f;
	}
	float height = RangeMapFloat( heightAndAltitude.x, 0.0f, 1.0f, 0.0f, CLOUD_LAYER_HEIGHT );
	float altitude = RangeMapFloat( heightAndAltitude.y, 0.0f, 1.0f, CLOUD_LAYER_ALTITUDE, ( CLOUD_LAYER_ALTITUDE + CLOUD_LAYER_HEIGHT ) );

	float posAltitude = length( pos );
	posAltitude = clamp( posAltitude, altitude, ( altitude + height ) );

	// Substitute the height (within the cloud layer) of pos (spherical?) as x in ( x - a ).( x - a - h ).(-4/h^2) to obtain a signal in [0,1]
	float heightSignal = ( posAltitude - altitude ) * ( posAltitude - ( altitude + height ) ) * ( -4.0f / height * height );
	heightSignal = clamp( heightSignal, 0.0f, 1.0f );

	return heightSignal;
}

float HeightGradient( vec2 heightAndAltitude, vec3 pos )
{
	float height = RangeMapFloat( heightAndAltitude.x, 0.0f, 1.0f, 0.0f, CLOUD_LAYER_HEIGHT );
	float altitude = RangeMapFloat( heightAndAltitude.y, 0.0f, 1.0f, CLOUD_LAYER_ALTITUDE, ( CLOUD_LAYER_ALTITUDE + CLOUD_LAYER_HEIGHT ) );

	float posAltitude = length( pos );
	posAltitude = clamp( posAltitude, altitude, ( altitude + height ) );

	float gradient = RangeMapFloat( posAltitude, altitude, ( altitude + height ), 0.0f, 1.0f );
	return gradient;
}

float GetCloudShape( vec3 uvw, float mip )
{
	vec4 shape = textureLod( noiseShape, uvw, mip );
	vec4 inverseShape = vec4( 1.0f ) - shape;
	inverseShape.x = shape.x;	// Preserve perlin noise
	shape = mix( shape, inverseShape, SHAPE_WORLEY_INVERSE );

	float worley = ( shape.y + 0.5f * smoothstep( 0.0f, 1.0f, shape.y ) * shape.z + 0.25f * smoothstep( 0.0f, 1.0f, shape.y ) * smoothstep( 0.0f, 1.0f, shape.z ) * shape.w );

	float shapeVal = shape.x + 0.5f * worley * shape.x;	// PERLIN * (SUM OF WORLEY)
	shapeVal = clamp( shapeVal, 0.0f, 1.0f );
	shapeVal = RangeMapFloat( shapeVal, 0.4f, 1.0f, 0.0f, 1.0f );
	shapeVal = clamp( shapeVal, 0.0f, 1.0f );
	return shapeVal;
}

float GetCloudDetail( vec3 uvw, float mip )
{
	vec3 detailOffset = vec3( DETAIL_TEXTURE_OFFSET, 0.0f, 0.0f ) * 0.025f;

	vec3 detail = textureLod( noiseDetail, uvw + detailOffset, mip ).xyz;
	vec3 inverseDetail = vec3( 1.0f ) - detail;
	detail = mix( detail, inverseDetail, DETAIL_WORLEY_INVERSE );

	float detailVal = detail.x + 0.5f * detail.y * smoothstep( 0.0f, 1.0f, detail.x ) + 0.25f * detail.z * smoothstep( 0.0f, 1.0f, detail.y ) * smoothstep( 0.0f, 1.0f, detail.x );
	detailVal = clamp( detailVal, 0.0f, 1.0f );
	return detailVal;
}

void SampleCloud( vec3 pos, out float muS, out float muE, out float gradient, float useMips )
{
	vec3 sampleUVW = GetSampleUVW( pos );
	float mip = GetMipLevel( pos ) * useMips;

	vec4 weatherVal = textureLod( weather, ( sampleUVW.xz * WEATHER_TEXTURE_TILE_FACTOR ), mip );

	float cloud = GetCloudShape( sampleUVW * SHAPE_TEXTURE_TILE_FACTOR, mip );

	//cloud = clamp( cloud, (1.0f - weatherVal.r), 1.0f );
	//cloud = RangeMapFloat( cloud, (1.0f - weatherVal.r), 1.0f, 0.0f, 1.0f );
	cloud *= weatherVal.r;

	float parabolic = HeightSignalParabolic( weatherVal.gb, pos, sampleUVW );
	//cloud = clamp( cloud, (1.0f - parabolic), 1.0f );
	//cloud = RangeMapFloat( cloud, (1.0f - parabolic), 1.0f, 0.0f, 1.0f );
	cloud *= parabolic;

	float detail = GetCloudDetail( sampleUVW * DETAIL_TEXTURE_TILE_FACTOR, mip );
	float inverseDetail = 1.0f - detail;
	float detailBlended = mix( detail, inverseDetail, 0.0f );
	//cloud = clamp( cloud, detailBlended, 1.0f );
	//cloud = RangeMapFloat( cloud, detailBlended, 1.0f, 0.0f, 1.0f );
	cloud -= detailBlended;

	gradient = HeightGradient( ( weatherVal.gb ), pos );
	cloud *= gradient;
	
	cloud = clamp( cloud, 0.0f, 1.0f );

	muS = cloud * MAX_DENSITY;				// Restrict max density of cloud
	muS *= sign( pos.y - TERRAIN_HEIGHT );	// We shouldn't see any clouds below the terrain
	muS = max( muS, 0.0f );

	float muA = CLOUD_ABSORPTION_COEFFICIENT * muS;
	muE = max( ( muS + muA ), EPSILON );
}

vec3 ApplyPhaseFunction( vec3 pos, vec3 viewDir, vec3 light )
{
	vec3 lightDir = normalize( pos - GetSun().position );
	float cosTheta = dot( lightDir, viewDir );

#ifndef PHASE_FUNCTION_USE_MIE
	/*
		Henyey-Greenstein phase function
		--------------------------------
		(1 - g^2) / (4 * PI * (1 + g^2 - 2gcosTheta)^1.5)
	*/
	float g = HENYEY_GREENSTEIN_G_VALUE;
	float gSquared = g * g;

	float denominator = ( 1.0f + gSquared - ( 2.0f * g * cosTheta ) );
	denominator *= sqrt( denominator );	// pow( 1.5f )
	denominator *= 4.0f * PI;

	float phaseFunction = ( 1.0f - gSquared ) / denominator;
	light *= phaseFunction;
#else
	float theta = acos( cosTheta );
	theta = theta * RADIANS_TO_DEGREES * 10.0f;	// 1800 entries
	vec2 uv = vec2(
		RangeMapFloat( theta, 0.0f, 1800.0f, 0.0f, 1.0f ),
		0.0f
	);

	vec3 phaseFunction =texture( mieLUT, uv ).xyz;
	phaseFunction *= MIE_PHASE_FUNCTION_MAX_VALUE;
	light *= phaseFunction;
#endif

	return light;
}

float GetSceneShadow( vec3 worldPosition )
{
	float transmittance = 1.f;

	vec3 sunDirection = normalize( GetSun().position - worldPosition );

	float dummyRoot, topMin, topMax;
	RaySphereIntersect( CLOUD_LAYER_ALTITUDE, worldPosition, sunDirection, dummyRoot, topMin );
	RaySphereIntersect( ( CLOUD_LAYER_ALTITUDE + 2.0f * CLOUD_LAYER_HEIGHT ), worldPosition, sunDirection, dummyRoot, topMax );

	float stepCount = 6.f * SHADOWS_ENABLED;
	float stepLength = (topMax - topMin) / ( stepCount + 1.f );
	vec3 startPosition = worldPosition + ( topMin * sunDirection);
	vec3 endPosition = worldPosition + (topMax * sunDirection);

	//return length(worldPosition)/(CLOUD_LAYER_ALTITUDE);// + 2.f * CLOUD_LAYER_HEIGHT);

	float currentStep = stepLength;
	while ( stepCount > 0.f )
	{
		vec3 samplePos = startPosition + ( currentStep * sunDirection );

		float muS = 0.0f;
		float muE = 0.0f;
		float gradient = 0.0f;
		SampleCloud( samplePos, muS, muE, gradient, 0.f );

		float beer = exp( -muE * stepLength );
		transmittance *= beer;

		if ( transmittance < 0.1f )
		{
			break;
		}
		
		currentStep += stepLength;
		stepCount -= 1.f;
	}
	
	return clamp( transmittance, 0.1f, 1.0f );
}

float GetCloudShadow( vec3 worldPosition )
{
	float transmittance = 1.0f;

	vec3 sunDirection = normalize( GetSun().position - worldPosition );

	float topMin, topMax;
	RaySphereIntersect( ( CLOUD_LAYER_ALTITUDE + 2.0f * CLOUD_LAYER_HEIGHT ), worldPosition, sunDirection, topMin, topMax );
	float endDistance = topMax;

	// March from the given point to the outermost point on the cloud nearest to the sun
	vec3 startPosition = worldPosition;
	vec3 endPosition = startPosition + ( endDistance * sunDirection );

	float stepCount = 6.0f * SHADOWS_ENABLED;
	float stepLength = stepCount / max( stepCount, 1.f );

	vec3 perpendicularRight = cross( sunDirection, vec3( 1.0f, 0.0f, 0.0f ) );
	vec3 perpendicularForward = cross( sunDirection, vec3( 0.0f, 0.0f, 1.0f ) );
	float perpBlend = ceil( dot( perpendicularRight, perpendicularRight ) ); // Will be zero if perpendicularRight was along sunDirection
	vec3 perpendicular = mix( perpendicularForward, perpendicularRight, perpBlend );

	float currentStep = 0.f;
	while ( stepCount > 0 )
	{
		vec3 samplePos = startPosition + ( currentStep * sunDirection );
		samplePos += currentStep * perpendicular * 0.33f;	// Cone sampling

		float muS = 0.0f;
		float muE = 0.0f;
		float gradient = 0.0f;
		SampleCloud( samplePos, muS, muE, gradient, 0.f );

		float beer = exp( -muE * stepLength );
		float powder = 1.0f - exp( -muE * 2.0f * stepLength );
		float extinction = 2.0f * beer * powder;
		transmittance *= beer;
		if ( transmittance < TRANSMITTANCE_EXIT_VALUE )
		{
			break;
		}

		perpendicular = cross( perpendicular, sunDirection );	// Rotate by 90 degrees
		currentStep += stepLength;
		stepCount -= 1.0f;
	}

	return clamp( transmittance, 0.0f, 1.0f );
}

vec4 Raymarch( float minDistance, float maxDistance, vec3 totalScatteredLight, float transmittance, out float cloudFrontDist )
{
	vec3 viewDir = GetRayDirection();
	vec3 startPos = ScreenToWorld( NEAR_Z );

	float minStepLength = RAYMARCH_STEP_LENGTH;
	float maxStepLength = max( ( maxDistance - minDistance ), RAYMARCH_MIN_FORWARD ) / RAYMARCH_NUM_STEPS;

	float dist = minDistance;
	dist = max( dist, RAYMARCH_MIN_FORWARD );

	// Debug
	cloudFrontDist = minDistance;

	// Disable offsets till they seem to be needed
	//float distOffset = fract( CURRENT_TIME ) * minStepLength;
	//minDistance -= distOffset;
	//maxDistance += distOffset;

	bool hitCloud = false;
	while ( dist < maxDistance )
	{
		vec3 currentPos = startPos + ( dist * viewDir );

		float muS = 0.0f;
		float muE = 0.0f;
		float gradient = 0.0f;
		SampleCloud( currentPos, muS, muE, gradient, 1.f );

		if ( !hitCloud && muE > RAYMARCH_STEP_CHANGE_DENSITY_THRESHOLD )
		{
			hitCloud = true;
			dist -= maxStepLength;
			minDistance = dist;
			cloudFrontDist = dist;
			continue;
		}

		// In-scattering = ( [ { sunlight * phase function } + ambient light ] * muS )
		vec3 inScattering = GetLightAtPosition( currentPos ).xyz;	// The direct sunlight at the position
		inScattering = ApplyPhaseFunction( currentPos, -viewDir, inScattering );				// The sunlight scattered in the view direction
		inScattering = clamp( inScattering, vec3( 0.0f ), vec3( 1.0f ) );
		
		float selfShadowing = GetCloudShadow(currentPos);
		vec3 shadowColor = vec3(0.15f, 0.175f, 0.2f);
		//inScattering = clamp( inScattering, shadowColor, vec3(1.0f) );
		//inScattering = mix( shadowColor, inScattering, smoothstep(0.0f, 1.0f, selfShadowing) );								// Scattered light remaining after self-shadowing
		inScattering *= selfShadowing;
		inScattering += ambientLightColorAndIntensity.xyz * ambientLightColorAndIntensity.w * GetNormalizedHeight( currentPos );				// Direction-independent ambient light within the medium
		inScattering *= muS;										// We need some medium to scatter in the view direction at all; the more medium present, the more scattering occurs

		float stepLength = mix( minStepLength, maxStepLength, clamp( max( dist - ( minDistance + RAYMARCH_DISTANCE_STEP_THRESHOLD ), 0.0f ), 0.0f, 1.0f ) );	// Smaller steps closer to the camera
		
		float beer = exp( -muE * stepLength );
		float powder = 1.0f - exp( -muE * stepLength * 2.0f );
		float extinction = beer * powder * 2.0f;
		vec3 outScattering = inScattering * beer;

		inScattering -= outScattering;
		inScattering /= muE;

		totalScatteredLight += inScattering * transmittance;
		transmittance *= beer;
		if ( transmittance < TRANSMITTANCE_EXIT_VALUE )
		{
			break;
		}

		dist += stepLength;
	}

	cloudFrontDist = (hitCloud)? cloudFrontDist : FAR_Z * 200.0f;	// Make it ridiculously large if the cloud wasn't hit

	vec4 scatteringTransmittance = vec4( totalScatteredLight, transmittance );
	scatteringTransmittance = clamp( scatteringTransmittance, vec4( 0.0f ), vec4( 1.0f ) );
	return scatteringTransmittance;
}

bool WasNDCInViewport( vec3 ndc )
{
	return ( 
		( abs( ndc.x ) - 1.0f <= EPSILON ) &&
		( abs( ndc.y ) - 1.0f <= EPSILON )
	);
}

bool WasWorldPosInViewport( vec3 worldPos )
{
	vec4 ndcPrev = OLD_VIEW * vec4( worldPos, 1.0f );
	ndcPrev = PROJECTION * ndcPrev;
	ndcPrev /= ndcPrev.w;
	return WasNDCInViewport( worldPos );
}

bool ShouldRaycastThisFrame()
{
	#ifdef ENABLE_AMORTIZATION
		#ifdef AMORTIZE_INTERLACED
			ivec2 fragCoordsMod = ivec2( gl_FragCoord.xy ) % ivec2( AMORTIZATION_FACTOR );
			int frag1DIndexMod = 2 * fragCoordsMod.y + fragCoordsMod.x;
			int frameCountMod = FRAME_COUNT % ( AMORTIZATION_FACTOR * AMORTIZATION_FACTOR );
			return ( frag1DIndexMod == frameCountMod );
		#elifdef AMORTIZE_RADIAL
			
		#else
			vec2 cellWidth = vec2( 1.0f ) / AMORTIZATION_FACTOR;
			int amortizationFactorInt = int( AMORTIZATION_FACTOR );
			int frameCountMod = FRAME_COUNT % ( amortizationFactorInt * amortizationFactorInt );	// To allow for the division below to work correctly
			vec2 frameModAmortizationFactor = vec2(
				float( frameCountMod % ( amortizationFactorInt ) ),	// Rows amortize faster than columns - this helps with vertical movement
				float( frameCountMod / ( amortizationFactorInt ) )
			);
			
			vec2 minUVsThisFrame = frameModAmortizationFactor * cellWidth;
			vec2 maxUVsThisFrame = ( frameModAmortizationFactor + 1.0f ) * cellWidth;	// Takes care of wrap-around case at end
			return (
				//!WasWorldPosInViewport( ScreenToWorld( FAR_Z ) ) ||
				(
					PASSUV.x >= minUVsThisFrame.x && PASSUV.y >= minUVsThisFrame.y &&
					PASSUV.x <= maxUVsThisFrame.x && PASSUV.y <= maxUVsThisFrame.y
				)
			);
		#endif
	#else
		return true;
	#endif
}

vec4 BlendPreviousScatteringTransmittance( vec4 scatteringTransmittance, float raymarchZ )
{
	vec3 worldPos = ScreenToWorld( raymarchZ );
	vec4 ndcPrev = OLD_VIEW * vec4( worldPos, 1.0f );
	ndcPrev = PROJECTION * ndcPrev;
	ndcPrev /= ndcPrev.w;
	
	vec4 blendedScatteringTransmittance = scatteringTransmittance;
	if ( WasNDCInViewport( ndcPrev.xyz ) )
	{
		float blendFactor = REPROJECTION_BLEND_FACTOR;
		blendFactor *= 1.0f - floor( scatteringTransmittance.w );	// If this pixel doesn't march through the cloud, no point in blending
		vec2 uv = RangeMapVec2( ndcPrev.xy, vec2( -1.0f ), vec2( 1.0f ), vec2( 0.0f ), vec2( 1.0f ) );
		vec4 previousScatteringTransmittance = texture( prevScatTrans, uv );
		blendedScatteringTransmittance = mix( scatteringTransmittance, previousScatteringTransmittance, blendFactor );
		
		// Force convergence if close - floats don't converge properly otherwise
		float maxDiff = max (
			abs(scatteringTransmittance.r - previousScatteringTransmittance.r),
			max(	abs(scatteringTransmittance.g - previousScatteringTransmittance.g),
					max(	abs(scatteringTransmittance.b - previousScatteringTransmittance.b),
							abs(scatteringTransmittance.a - previousScatteringTransmittance.a)
						)
				)
		);
		blendedScatteringTransmittance = mix( blendedScatteringTransmittance, scatteringTransmittance, ceil( max( 0.05f - maxDiff, 0.0f ) ) );
	}

	return mix( scatteringTransmittance, blendedScatteringTransmittance, TEMPORAL_UPSAMPLING_ENABLED );
}

vec4 UsePreviousScatteringTransmittance( float raymarchZ )
{
	vec3 worldPos = ScreenToWorld( raymarchZ );
	vec4 ndcPrev = OLD_VIEW * vec4( worldPos, 1.0f );
	ndcPrev = PROJECTION * ndcPrev;
	ndcPrev /= ndcPrev.w;
	
	if ( WasNDCInViewport( ndcPrev.xyz ) )
	{
		vec2 uv = RangeMapVec2( ndcPrev.xy, vec2( -1.0f ), vec2( 1.0f ), vec2( 0.0f ), vec2( 1.0f ) );
		vec4 previousScatteringTransmittance = texture( prevScatTrans, uv );
		return previousScatteringTransmittance;
	}

	vec4 scatteringTransmittance = vec4( vec3(0.0f), 1.0f );	// Must freshly compute
	return scatteringTransmittance;
}

vec4 TraceSceneForScatteredLight()
{
	vec3 totalScatteredLight = vec3( 0.0f );
	float transmittance = 1.0f;
	vec4 scatteringTransmittance = vec4( totalScatteredLight, transmittance );

	if ( TEMPORAL_UPSAMPLING_ENABLED == 0.0f || ShouldRaycastThisFrame() )
	{
		vec3 viewDir = GetRayDirection();
		vec3 startPos = ScreenToWorld( NEAR_Z );
		float depth = GetRayFinalViewPosition().z;

		float terrainMin, terrainMax;
		float bottomMin, bottomMax;
		float topMin, topMax;

		bool intersectsTerrain = RaySphereIntersect( TERRAIN_HEIGHT, ScreenToWorld( NEAR_Z ), GetRayDirection(), terrainMin, terrainMax );
		bool intersectsCloudBottom = RaySphereIntersect( CLOUD_LAYER_ALTITUDE, ScreenToWorld( NEAR_Z ), GetRayDirection(), bottomMin, bottomMax );
		RaySphereIntersect( ( CLOUD_LAYER_ALTITUDE + 2.0f * CLOUD_LAYER_HEIGHT ), ScreenToWorld( NEAR_Z ), GetRayDirection(), topMin, topMax );

		if ( intersectsTerrain )
		{
			vec3 finalPos = startPos + ( bottomMin * viewDir );
			float signY = sign( finalPos.y - TERRAIN_HEIGHT );
			bottomMin *= signY;
			bottomMin = max( bottomMin, RAYMARCH_MIN_FORWARD );

			finalPos = startPos + ( bottomMax * viewDir );
			signY = sign( finalPos.y - TERRAIN_HEIGHT );
			bottomMax *= signY;
			bottomMax = max( bottomMax, RAYMARCH_MIN_FORWARD );

			finalPos = startPos + ( topMin * viewDir );
			signY = sign( finalPos.y - TERRAIN_HEIGHT );
			topMin *= signY;
			topMin = max( topMin, RAYMARCH_MIN_FORWARD );

			finalPos = startPos + ( topMax * viewDir );
			signY = sign( finalPos.y - TERRAIN_HEIGHT );
			topMax *= signY;
			topMax = max( topMax, RAYMARCH_MIN_FORWARD );
		}

		float bottomDistance = bottomMin;
		float topDistance = topMin;

		if ( !intersectsCloudBottom )
		{
			bottomDistance = topMax;
		}

		float minDistance = min( bottomDistance, topDistance );
		float maxDistance = max( bottomDistance, topDistance );
		maxDistance = min( maxDistance, depth );

		// Debug
		float cloudFrontDist = minDistance;

		scatteringTransmittance = Raymarch( minDistance, maxDistance, totalScatteredLight, transmittance, cloudFrontDist );

		if ( intersectsCloudBottom && scatteringTransmittance.w > TRANSMITTANCE_EXIT_VALUE )
		{
			bottomDistance = bottomMax;
			topDistance = topMax;
			minDistance = min( bottomDistance, topDistance );
			maxDistance = max( bottomDistance, topDistance );
			maxDistance = min( maxDistance, depth );

			float secondCloudFrontDist = minDistance;
			scatteringTransmittance = Raymarch( minDistance, maxDistance, scatteringTransmittance.xyz, scatteringTransmittance.w, secondCloudFrontDist );
			cloudFrontDist = min( cloudFrontDist, secondCloudFrontDist );
		}

		scatteringTransmittance = BlendPreviousScatteringTransmittance( scatteringTransmittance, FAR_Z );	// TODO: Is FAR_Z the best value?

		// Debug
		float cloudFrontMipLevel = GetMipLevel( startPos + cloudFrontDist * viewDir );
		vec4 lodColor = GetMipLevelColor( cloudFrontMipLevel );
		scatteringTransmittance *= ( lodColor * DEBUG_VIEW_LODS ) + ( vec4( 1.0f ) * ( 1.0f - DEBUG_VIEW_LODS ) );
	}
	else
	{
		scatteringTransmittance	= UsePreviousScatteringTransmittance( FAR_Z );	// TODO: Is FAR_Z the best value?
	}
	return scatteringTransmittance;
}

vec3 GetSceneVolumetricShadow( float transmittance )
{
	float depth = texture( depthTarget, PASSUV ).x;
	if ( depth < 1.f )
	{
		vec3 depthPos = GetRayFinalWorldPosition();
		float sceneShadow = GetSceneShadow(depthPos);
		vec3 shadowCol = vec3(sceneShadow);
		return shadowCol;
	}

	return vec3(1.f);
}

void main( void )
{
	vec4 scatteringTransmittance = TraceSceneForScatteredLight();
	vec3 scatteredLight = scatteringTransmittance.xyz;
	float transmittance = scatteringTransmittance.w;
	outScatTrans = scatteringTransmittance;

	vec3 sceneShadow = GetSceneVolumetricShadow(scatteringTransmittance.w);
	vec4 sceneShadowFrag = vec4( sceneShadow, 1.f );
	outVolShadow = sceneShadowFrag;
}
