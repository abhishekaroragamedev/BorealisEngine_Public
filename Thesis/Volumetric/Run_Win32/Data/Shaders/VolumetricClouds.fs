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

#define SHADOW_NUM_STEPS 6.0f
#define RAYMARCH_NUM_STEPS 40.0f
#define RAYMARCH_MIN_FORWARD 0.1f
#define TRANSMITTANCE_EXIT_VALUE 0.01f
#define HENYEY_GREENSTEIN_G_VALUE 0.8f
#define FRAME_RAY_INTERSECT_DEPTH_DIFF_THRESHOLD 0.01f
#define AMBIENT_LIGHT_MIN_FACTOR 0.1f
#define AMBIENT_LIGHT_MAX_FACTOR 1.f
#define RAY_SPHERE_EXTRA_CURVATURE 0.f

#define PHASE_FUNCTION_USE_MIE
#define SAMPLING_SPHERICAL
//#define CLOUDS_DEBUG

// Optimization - do not use amortization without temporal upsampling
//#define ENABLE_AMORTIZATION
#define AMORTIZATION_FACTOR 2
//#define AMORTIZE_INTERLACED
//#define AMORTIZE_RADIAL
#define REPROJECTION_BLEND_FACTOR 0.95f

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
	float TEMPORAL_REPROJECTION_ENABLED;
	float SUN_MOVED_THIS_FRAME;
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
layout ( binding = 7 ) uniform sampler2D prevDepthTarget;

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

vec3 GetPrevRayFinalViewPosition()
{
	vec3 worldPos = ScreenToWorld( FAR_Z );
	vec4 ndcPrev = OLD_VIEW * vec4( worldPos, 1.0f );
	ndcPrev = PROJECTION * ndcPrev;
	ndcPrev /= ndcPrev.w;
	vec2 uv = RangeMapVec2( ndcPrev.xy, vec2( -1.0f ), vec2( 1.0f ), vec2( 0.0f ), vec2( 1.0f ) );
	float depthValue = texture( prevDepthTarget, uv ).x;
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
#ifdef SAMPLING_SPHERICAL
	float height = length( pos );
#else
	float height = pos.y;
#endif
	float signs = height - CLOUD_LAYER_ALTITUDE;
	signs *= ( CLOUD_LAYER_ALTITUDE + CLOUD_LAYER_HEIGHT ) - height;
	signs = max( sign( signs ), 0.f );
	return signs * RangeMapFloat( height, CLOUD_LAYER_ALTITUDE, ( CLOUD_LAYER_ALTITUDE + CLOUD_LAYER_HEIGHT ), 0.0f, 1.0f );
}

vec3 GetSampleUVW( vec3 pos )
{
	// Map x, y, z to the hemispherical dome of radius between CLOUD_LAYER_ALTITUDE and ( CLOUD_LAYER_ALTITUDE + CLOUD_LAYER_HEIGHT )
	vec3 uvw;

#ifdef SAMPLING_SPHERICAL
	// Spherical UVs - the cloud layer exists between an inner and outer radius
	// Maintain dimension ratios - the shape texture's dimensions are 128, 32, 128, so x and z should have 4 times the world space range of y
	float radius = length( pos );

	float halfArcLength = 2.f * CLOUD_LAYER_HEIGHT;		// Since the shape texture has 32 y and 128 x/z pixels

	float xRadius = length( vec2( pos.x, pos.y ) );		// Remove Z - solve as a 2D hemisphere
	float xArcThetaRadians = halfArcLength / xRadius;	// Basic circumference/arc length formula for a circle
	float xMax = xRadius * sin( xArcThetaRadians );		// If theta is the angle between the 90 degree line and this pos, then y = cos and x/z = sin
	float zRadius = length( vec2( pos.z, pos.y ) );		// Remove X - solve as a 2D hemisphere
	float zArcThetaRadians = halfArcLength / zRadius;	// Basic circumference/arc length formula for a circle
	float zMax = zRadius * sin( zArcThetaRadians );		// If theta is the angle between the 90 degree line and this pos, then y = cos and x/z = sin

	// Map UW (XZ) along the hemisphere
	vec2 uvMappedXZ = RangeMapVec2( pos.xz, vec2( -xMax, -zMax ), vec2( xMax, zMax ), vec2( 0.0f ), vec2( 1.0f ) );

	uvw.x = uvMappedXZ.x;
	uvw.y = RangeMapFloat( radius, CLOUD_LAYER_ALTITUDE, ( CLOUD_LAYER_ALTITUDE + CLOUD_LAYER_HEIGHT ), 0.f, 1.f );
	uvw.z = uvMappedXZ.y;
#else
	// Linear UVs - the cloud layer exists in a cube
	// Maintain dimension ratios - the shape texture's dimensions are 128, 32, 128, so x and z should have 4 times the world space range of y
	uvw.x = RangeMapFloat( pos.x, -CLOUD_LAYER_HEIGHT * 2.f, CLOUD_LAYER_HEIGHT * 2.f, 0.0f, 1.0f );
	uvw.y = RangeMapFloat( pos.y, CLOUD_LAYER_ALTITUDE, ( CLOUD_LAYER_ALTITUDE + CLOUD_LAYER_HEIGHT ), 0.0f, 1.0f );
	uvw.z = RangeMapFloat( pos.z, -CLOUD_LAYER_HEIGHT * 2.f, CLOUD_LAYER_HEIGHT * 2.f, 0.0f, 1.0f );
#endif

	return uvw;
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

float GetCloudShape( vec3 uvw, float mip )
{
	vec4 shape = textureLod( noiseShape, uvw, mip );
	vec4 inverseShape = vec4( 1.0f ) - shape;
	inverseShape.x = shape.x;	// Preserve perlin noise
	shape = mix( shape, inverseShape, SHAPE_WORLEY_INVERSE );

	float shapeVal = shape.x * 1.6f;
	shapeVal = RangeMapFloat( shapeVal, 1.f - shape.y, 1.f, 0.f, 1.f );
	shapeVal = RangeMapFloat( shapeVal, 1.f - shape.z, 1.f, 0.f, 1.f );
	shapeVal = RangeMapFloat( shapeVal, 1.f - shape.w, 1.f, 0.f, 1.f );
	shapeVal = clamp( shapeVal, 0.f, 1.f );
	return shapeVal;
}

float GetCloudDetail( vec3 uvw, float mip )
{
	vec3 detailOffset = vec3( DETAIL_TEXTURE_OFFSET, 0.0f, 0.0f ) * 0.05f;

	vec3 detail = textureLod( noiseDetail, uvw + detailOffset, mip ).xyz;
	vec3 inverseDetail = vec3( 1.0f ) - detail;
	detail = mix( detail, inverseDetail, DETAIL_WORLEY_INVERSE );

	float detailVal = detail.x;
	detailVal = RangeMapFloat( detailVal, 1.f - detail.y, 1.f, 0.f, 1.f );
	detailVal = RangeMapFloat( detailVal, 1.f - detail.z, 1.f, 0.f, 1.f );
	detailVal = clamp( detailVal, 0.0f, 1.0f );
	return detailVal;
}

#ifdef CLOUDS_DEBUG
void SampleCloud( vec3 pos, out float muS, out float muE, out float gradient, float useMips )
{
	// Cube to debug
	vec3 center = vec3( 0.f, 5200.f, 0.f );
	vec3 extents = vec3( 25.6f, 6.4f, 25.6f ) * 10.f;

	vec3 dist = pos - center;
	vec3 signs = sign( extents - abs(dist) );
	signs = max( signs, vec3( 0.f ) );

	vec3 mins = center - extents;
	vec3 maxs = center + extents;

	vec3 normalCoords = clamp( pos - mins, vec3( 0.f ), 2.f * extents );
	normalCoords /= 2.f * extents;
	//normalCoords.xz += vec2( 0.005f * CURRENT_TIME );
	vec3 shapeCoords = normalCoords * vec3( SHAPE_TEXTURE_TILE_FACTOR, 1.f, SHAPE_TEXTURE_TILE_FACTOR );
	muS = GetCloudShape( shapeCoords, 1.f );
	//muS = texture( noiseShape, vec3( 0.f, norm ).r;
	//muS = pow( muS, 10.f );
	vec2 weatherCoords = normalCoords.xz * WEATHER_TEXTURE_TILE_FACTOR;
	vec4 weatherVal = texture( weather, weatherCoords );
	//muS = RangeMapFloat( muS, 1.f -weatherVal.r, 1.f, 0.f, 1.f );
	//muS *= weatherVal.r;

	//float parabolic = texture( heightSignal, vec2( 0.5f, normalCoords.y ) ).r;
	float altitude = RangeMapFloat( normalCoords.y, 0.f, 1.f, mins.y, maxs.y );
	float parabolic = ( altitude - mins.y ) * ( altitude - maxs.y ) * ( -1.f / ( extents.y * extents.y ) );
	parabolic = normalCoords.y * ( normalCoords.y - 1.f ) * -1.f;
	parabolic = max( parabolic, 0.f );
	//muS = clamp( muS, (1.0f - parabolic), 1.0f );
	muS = RangeMapFloat( muS, 1.f - parabolic, 1.0f, 0.0f, 1.0f );
	//muS *= parabolic;
	vec3 detailCoords = normalCoords * vec3( DETAIL_TEXTURE_TILE_FACTOR, 1.f, DETAIL_TEXTURE_TILE_FACTOR );
	float detail = GetCloudDetail( detailCoords, 1.f );
	//detail = pow( detail, 10.f );
	float inverseDetail = 1.0f - detail;
	float detailBlended = mix( detail, inverseDetail, pow( detailCoords.y, 5.f ) );
	////cloud = clamp( cloud, detailBlended, 1.0f );
	muS = RangeMapFloat( muS, detailBlended, 1.0f, 0.0f, 1.0f );
	//muS -= detailBlended;
	//gradient = HeightGradient( ( weatherVal.gb ), pos );
	muS = RangeMapFloat( muS, pow( 1.f - detailCoords.y, 10.f ), 1.f, 0.f, 1.f );
	//muS = clamp( muS, 0.0f, 1.0f );
	muS *= MAX_DENSITY; // Definitely < 1.f
	muS *= signs.x * signs.y * signs.z;

	muE = max( muS, EPSILON );
	gradient = 1.f;
}
#else
void SampleCloud( vec3 pos, out float muS, out float muE, out float gradient, float useMips )
{
	vec3 sampleUVW = GetSampleUVW( pos );
	float mip = 1.f;
	float isInCloudLayer = 1.f;

	// Determine whether or not "pos" is in the cloud layer
#ifdef SAMPLING_SPHERICAL
	float lengthSquared = dot( pos, pos );
	float cloudMinSquared = dot( CLOUD_LAYER_ALTITUDE, CLOUD_LAYER_ALTITUDE );
	float cloudMaxSquared = dot( ( CLOUD_LAYER_ALTITUDE + CLOUD_LAYER_HEIGHT ), ( CLOUD_LAYER_ALTITUDE + CLOUD_LAYER_HEIGHT ) );
	float productOfDiffs = ( lengthSquared - cloudMinSquared ) * ( cloudMaxSquared - lengthSquared );
#else
	float height = pos.y;
	float productOfDiffs = ( height - CLOUD_LAYER_ALTITUDE ) * ( ( CLOUD_LAYER_ALTITUDE + CLOUD_LAYER_HEIGHT ) - height );
#endif
	isInCloudLayer = max( sign( productOfDiffs ), 0.f );	// If the point is inside the cloud, the product of diffs (above) will be positive

	// Get the base shape/density of the cloud at "pos"
	vec3 shapeCoords = sampleUVW * vec3( SHAPE_TEXTURE_TILE_FACTOR, SHAPE_TEXTURE_TILE_FACTOR, SHAPE_TEXTURE_TILE_FACTOR );
	float cloud = GetCloudShape( shapeCoords, mip );

	// Get the coverage value provided by the artist weather texture
	vec2 weatherCoords = sampleUVW.xz * WEATHER_TEXTURE_TILE_FACTOR;
	vec4 weatherVal = textureLod( weather, weatherCoords, mip );
	cloud = clamp( cloud, 1.f - weatherVal.r, 1.f );			// Dilate the base density - eliminate densities lower than ( 1 - coverage )
	cloud = RangeMapFloat( cloud, (1.f - weatherVal.r), 1.0f, 0.0f, 1.0f );

	// Store the normalized height of "pos" within the cloud layer
	float normalizedHeight = GetNormalizedHeight( pos );

	// Dilate the density by a parabolic function to be more forgiving of low densities in the center and less of those at the top and bottom of the cloud layer
	float parabolic = normalizedHeight * ( normalizedHeight - 1.f ) * -4.f;
	parabolic = max( parabolic, 0.f );
	cloud = clamp( cloud, 1.f - parabolic, 1.f );
	cloud = RangeMapFloat( cloud, 1.f - parabolic, 1.0f, 0.0f, 1.0f );

	// Dilate the density using the detail texture
	// Since the detail texture is 32x32x32 and the shape texture is 128x32x128, we reduce the physical xz-dimension of a full range of UW values for sampling
	vec3 detailCoords = sampleUVW * vec3( 0.25f * DETAIL_TEXTURE_TILE_FACTOR, DETAIL_TEXTURE_TILE_FACTOR, 0.25f * DETAIL_TEXTURE_TILE_FACTOR );
	float detail = GetCloudDetail( detailCoords, 1.f );	// Worley noise by itself adds a billowy shape to the clouds when used to erode
	float inverseDetail = 1.0f - detail;				// Worley noise, when inverted, adds wispy shapes to the clouds when used to erode
	float detailBlended = mix( detail, inverseDetail, normalizedHeight );	// We want more billowy shapes at lower heights and wispy shapes on the top of the clouds
	cloud = clamp( cloud, detailBlended, 1.f );
	cloud = RangeMapFloat( cloud, detailBlended, 1.0f, 0.0f, 1.0f );
	
	cloud = RangeMapFloat( cloud, pow( 1.f - normalizedHeight, 1.f ), 1.f, 0.f, 1.f );	// Apply a final gradient to make the clouds more dense over height
	cloud *= MAX_DENSITY;	// The integrated density over the raymarch will be very high, so attentuate it - more of an artist/subjective parameter
	
	/*
		( 1 - ( 2 * ( x - 0.5 ) ) ) for x in [0, 1] is mostly equal to 1, and sharply drops at either edge
		Since clouds are rendered in a finite layer, wrapping of the 3D textures in the vertical direction
		may cause the bottoms of clouds to be cut off and instead appear at the very top of the cloud layer.
		This looks particularly bad because the lighting results in these top portions appearing much brighter
		than the rest of the clouds owing to their low density and increased ambient light (see the Raymarch()
		function)
		To counteract this, this function is used to feather off the density of the cloud at the edges
	*/
	cloud *= 1.f - ( 2.f * ( normalizedHeight - 0.5f ) );
	
	cloud *= isInCloudLayer;

	muS = clamp( cloud, 0.0f, 1.0f );
	muS *= sign( pos.y - TERRAIN_HEIGHT );	// We shouldn't see any clouds below the terrain
	muS = max( muS, 0.0f );

	float muA = CLOUD_ABSORPTION_COEFFICIENT * muS;
	muE = max( ( muS + muA ), EPSILON );
}
#endif

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

	vec3 phaseFunction = texture( mieLUT, uv ).xyz;
	phaseFunction *= MIE_PHASE_FUNCTION_MAX_VALUE;
	light *= phaseFunction;
#endif

	return light;
}

float GetSceneShadow( vec3 worldPosition )
{
	float transmittance = 1.f;

	vec3 sunDirection = normalize( GetSun().position - worldPosition );

	// Throw out rays from this position to the inner and outer radii of the cloud layer
	float dummyRoot, topMin, topMax;
	RaySphereIntersect( CLOUD_LAYER_ALTITUDE, worldPosition, sunDirection, dummyRoot, topMin );
	RaySphereIntersect( ( CLOUD_LAYER_ALTITUDE + CLOUD_LAYER_HEIGHT ), worldPosition, sunDirection, dummyRoot, topMax );

	float stepCount = SHADOW_NUM_STEPS * SHADOWS_ENABLED;
	float stepLength = ( topMax - topMin ) / max( stepCount, 1.f );
	vec3 startPosition = worldPosition + ( topMin * sunDirection);
	vec3 endPosition = worldPosition + ( topMax * sunDirection );

	float currentStep = stepLength * 0.5f;	// Offset by half a step just to not sample right at the edge (which may result in wrong values)
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
	RaySphereIntersect( ( CLOUD_LAYER_ALTITUDE + CLOUD_LAYER_HEIGHT ), worldPosition, sunDirection, topMin, topMax );
	float endDistance = topMax;

	// March from the current point to the outermost point on the cloud nearest to the sun
	vec3 startPosition = worldPosition;

	float initialStepCount = SHADOW_NUM_STEPS * SHADOWS_ENABLED;
	float stepCount = initialStepCount;
	float minStepLength = 1.f;
	float maxStepLength = endDistance / max( initialStepCount, 1.f );

	float stepLength = minStepLength;

	float currentStep = stepLength * 0.5f;		// Offset by half a step just to not sample right at the edge (which may result in wrong values)
	while ( stepCount > 0.f )
	{
		vec3 samplePos = startPosition + ( currentStep * sunDirection );

		float muS = 0.0f;
		float muE = 0.0f;
		float gradient = 0.0f;
		SampleCloud( samplePos, muS, muE, gradient, 0.f );

		float beer = exp( -muE * stepLength );
		transmittance *= beer;
		if ( transmittance < TRANSMITTANCE_EXIT_VALUE )
		{
			break;
		}

		stepLength = mix( minStepLength, maxStepLength, ( initialStepCount - stepCount ) / max( initialStepCount, 1.f ) );
		currentStep += stepLength;
		stepCount -= 1.0f;
	}

	return clamp( transmittance, 0.0f, 1.0f );
}

vec4 Raymarch( float minDistance, float maxDistance, vec3 totalScatteredLight, float transmittance )
{
	vec3 viewDir = GetRayDirection();
	vec3 startPos = ScreenToWorld( NEAR_Z );

	float stepLength = max( ( maxDistance - minDistance ), RAYMARCH_MIN_FORWARD ) / RAYMARCH_NUM_STEPS;
	int numSteps = 0;

	float offset = RAYMARCH_OFFSET_FRACTION * stepLength;
	minDistance += stepLength - offset;

	float dist = minDistance;
	while ( dist < maxDistance )
	{
		vec3 currentPos = startPos + ( dist * viewDir );

		float muS = 0.0f;
		float muE = 0.0f;
		float gradient = 0.0f;
		SampleCloud( currentPos, muS, muE, gradient, 1.f );

		// In-scattering = ( [ { sunlight * phase function } + ambient light ] * muS )
		vec3 inScattering = GetLightAtPosition( currentPos ).xyz;	// The direct sunlight at the position
		inScattering = ApplyPhaseFunction( currentPos, -viewDir, inScattering );				// The sunlight scattered in the view direction
		
		float selfShadowing = GetCloudShadow(currentPos);
		inScattering *= selfShadowing;
		
		float ambientLightFactor = mix( AMBIENT_LIGHT_MIN_FACTOR, AMBIENT_LIGHT_MAX_FACTOR, GetNormalizedHeight( currentPos ) );	// Ambient light increases with height, but clamp this factor to control lighting
		inScattering += ambientLightColorAndIntensity.xyz * ambientLightColorAndIntensity.w * ambientLightFactor;					// Direction-independent ambient light within the medium

		inScattering *= muS;										// We need some medium to scatter in the view direction at all; the more medium present, the more scattering occurs
		inScattering = clamp( inScattering, vec3( 0.0f ), vec3( 1.0f ) );

		float beer = exp( -muE * stepLength );
		float powder = 1.0f - exp( -muE * stepLength * 2.0f );
		float extinction = beer * powder * 2.0f;
		vec3 outScattering = inScattering * beer;

		inScattering -= outScattering;
		inScattering /= muE;

		totalScatteredLight += inScattering * transmittance;
		transmittance *= beer;
		if ( numSteps > RAYMARCH_NUM_STEPS || transmittance < TRANSMITTANCE_EXIT_VALUE )
		{
			break;
		}

		dist += stepLength;
		numSteps++;
	}

	vec4 scatteringTransmittance = vec4( totalScatteredLight, transmittance );
	scatteringTransmittance = clamp( scatteringTransmittance, vec4( 0.0f ), vec4( 1.0f ) );
	return scatteringTransmittance;
}

bool WasNDCInViewport( vec3 ndc, bool checkZ )
{
	return ( 
		( abs( ndc.x ) <= 1.f ) &&
		( abs( ndc.y ) <= 1.f ) &&
		( !checkZ || ( abs( ndc.z ) <= 1.f ) )
	);
}

bool WasWorldPosInViewport( vec3 worldPos, bool checkZ )
{
	vec4 ndcPrev = OLD_VIEW * vec4( worldPos, 1.0f );
	ndcPrev = PROJECTION * ndcPrev;
	ndcPrev /= ndcPrev.w;
	return WasNDCInViewport( ndcPrev.xyz, checkZ );
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

bool CanPreviousFrameBeUsed( vec3 ndcPrev, bool rayIntersectedDepth )
{

	return (													// Only allow reprojection if..
			TEMPORAL_REPROJECTION_ENABLED > 0.f					// Reprojection is enabled
		&&	FRAME_COUNT > 1										// We have previous frames to blend with in the first place
		&&	SUN_MOVED_THIS_FRAME < 1.f							// If the sun didn't move (because scattering-transmittance information changes)
		&&	WasNDCInViewport( ndcPrev, false )					// If the pixel being blended was within last frame's viewport
		&&	!rayIntersectedDepth								// If this ray's intersection with depth this frame wasn't significantly different from last frame's
	);
}

vec4 BlendPreviousScatteringTransmittance( vec4 scatteringTransmittance, float raymarchZ, bool rayIntersectedDepth )
{
	vec3 worldPos = ScreenToWorld( raymarchZ );
	vec4 ndcPrev = OLD_VIEW * vec4( worldPos, 1.0f );	// Use last frame's view matrix here - where in the viewport was this pixel/ray last frame?
	ndcPrev = PROJECTION * ndcPrev;
	ndcPrev /= ndcPrev.w;
	
	vec4 blendedScatteringTransmittance = scatteringTransmittance;
	if ( CanPreviousFrameBeUsed( ndcPrev.xyz, rayIntersectedDepth ) )
	{
		float blendFactor = REPROJECTION_BLEND_FACTOR;
		vec2 uv = RangeMapVec2( ndcPrev.xy, vec2( -1.0f ), vec2( 1.0f ), vec2( 0.0f ), vec2( 1.0f ) );
		vec4 previousScatteringTransmittance = texture( prevScatTrans, uv );
		blendedScatteringTransmittance = mix( scatteringTransmittance, previousScatteringTransmittance, blendFactor );
	}

	return mix( scatteringTransmittance, blendedScatteringTransmittance, TEMPORAL_REPROJECTION_ENABLED );
}

bool DidRayIntersectDepth( float depth, float prevDepth, float minDistance, float maxDistance )
{
	bool rayIntersectedCurrentDepth = ( depth < minDistance ) || ( ( maxDistance - depth ) > EPSILON  );
	bool rayIntersectedPrevDepth = ( prevDepth < minDistance ) || ( ( maxDistance - prevDepth ) > EPSILON );
	// Based on the actual depth value, increase the need for a difference between prevDepth and current depth (be more forgiving for objects up close)
	return ( 
			( abs( depth - prevDepth ) > ( FRAME_RAY_INTERSECT_DEPTH_DIFF_THRESHOLD * depth ) )
		&&	( rayIntersectedCurrentDepth || rayIntersectedPrevDepth )
	);
}

vec4 TraceSceneForScatteredLight()
{
	vec3 totalScatteredLight = vec3( 0.0f );
	float transmittance = 1.0f;
	vec4 scatteringTransmittance = vec4( totalScatteredLight, transmittance );
	bool rayIntersectedDepth = false;

	if ( TEMPORAL_REPROJECTION_ENABLED <= 0.f || ShouldRaycastThisFrame() )
	{
		vec3 viewDir = GetRayDirection();
		vec3 startPos = ScreenToWorld( NEAR_Z );
		float depth = GetRayFinalViewPosition().z;
		float prevDepth = GetPrevRayFinalViewPosition().z;

		float terrainMin, terrainMax;
		float bottomMin, bottomMax;
		float topMin, topMax;

		// Spherical raymarch
		bool intersectsTerrain = RaySphereIntersect( TERRAIN_HEIGHT, ScreenToWorld( NEAR_Z ), GetRayDirection(), terrainMin, terrainMax );
		bool intersectsCloudBottom = RaySphereIntersect( CLOUD_LAYER_ALTITUDE, ScreenToWorld( NEAR_Z ), GetRayDirection(), bottomMin, bottomMax );
		RaySphereIntersect( ( CLOUD_LAYER_ALTITUDE + CLOUD_LAYER_HEIGHT ), ScreenToWorld( NEAR_Z ), GetRayDirection(), topMin, topMax );

		float bottomDistance = bottomMin;
		float topDistance = topMin;

		if ( !intersectsCloudBottom )
		{
			bottomDistance = topMax;
		}

		// First raymarch - "Entering" the hemisphere (not run when below the clouds)
		float minDistance = min( bottomDistance, topDistance );
		float maxDistance = max( bottomDistance, topDistance );
		rayIntersectedDepth = DidRayIntersectDepth( depth, prevDepth, minDistance, maxDistance );
		maxDistance = min( maxDistance, depth );

#ifdef CLOUDS_DEBUG
		// Put whatever changes to the raymarch min/max you want here
		minDistance = NEAR_Z;
		maxDistance = 1000.f;
		intersectsCloudBottom = false;
#endif

		scatteringTransmittance = Raymarch( minDistance, maxDistance, totalScatteredLight, transmittance );

		if ( intersectsCloudBottom && scatteringTransmittance.w > TRANSMITTANCE_EXIT_VALUE )
		{
			bottomDistance = bottomMax;
			topDistance = topMax;
			minDistance = min( bottomDistance, topDistance );
			maxDistance = max( bottomDistance, topDistance );
			rayIntersectedDepth = rayIntersectedDepth || DidRayIntersectDepth( depth, prevDepth, minDistance, maxDistance );
			maxDistance = min( maxDistance, depth );

#ifdef CLOUDS_DEBUG
		// Put whatever changes to the raymarch min/max you want here
		minDistance = 1.f;
		maxDistance = 50.f;
#endif

			scatteringTransmittance = Raymarch( minDistance, maxDistance, scatteringTransmittance.xyz, scatteringTransmittance.w );
		}
		
		scatteringTransmittance = BlendPreviousScatteringTransmittance( scatteringTransmittance, FAR_Z, rayIntersectedDepth );
	}

	return scatteringTransmittance;
}

vec3 GetSceneVolumetricShadow()
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
	outScatTrans = scatteringTransmittance;

	vec3 sceneShadow = GetSceneVolumetricShadow();
	vec4 sceneShadowFrag = vec4( sceneShadow, 1.f );
	outVolShadow = sceneShadowFrag;
}
