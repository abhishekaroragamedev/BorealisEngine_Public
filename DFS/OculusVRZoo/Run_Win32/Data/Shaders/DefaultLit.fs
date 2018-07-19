#version 420 core

#define EPSILON 0.00001

#define MAX_LIGHTS 8
#define SHADOW_Z_BIAS 0.005
#define SHADOW_CASTING_LIGHT_INDEX 0 // Only the first light ever casts shadows for now
#define DETAIL_TEXTURE_MIN_DISTANCE 0.25
#define DETAIL_TEXTURE_MAX_DISTANCE 1.5
#define DETAIL_TEXTURE_SAMPLE_FREQUENCY 30.0

layout(binding=0) uniform sampler2D texDiffuse;
layout(binding=1) uniform sampler2D texNormal;
layout(binding=2) uniform sampler2D texSpecular;
layout(binding=3) uniform sampler2D texEmissive;
layout(binding=4) uniform sampler2D texDetail;

layout (binding=5) uniform sampler2D shadowMap;
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

layout(binding=1, std140) uniform CameraUBO
{
	mat4 VIEW;
	mat4 PROJECTION;
	vec3 EYE_POSITION;
	float PADDING;
};

layout(binding=3, std140) uniform ObjectLightProperties
{
	float specularAmount;
	float specularPower;
	vec2 padding;
};

in vec4 PASSCOLOR;
in vec2 PASSUV;
in vec3 PASSWORLDPOS;
in vec3 PASSNORMAL;
in vec3 PASSTANGENT;
in vec3 PASSBITANGENT;
in mat4 PASSMODEL;

layout(location=0) out vec4 outColor; 
layout(location=1) out vec4 outBloom; 

vec3 GetWorldNormal()
{
   vec3 surfaceNormal = ( texture( texNormal, PASSUV ) ).xyz;
   surfaceNormal = ( surfaceNormal * vec3( 2.0, 2.0, 1.0 ) ) - vec3( 1.0, 1.0, 0.0 );
   
   #ifdef USE_DETAIL
	vec3 detailNormal = ( texture( texDetail, vec2( PASSUV.x * DETAIL_TEXTURE_SAMPLE_FREQUENCY, PASSUV.y * DETAIL_TEXTURE_SAMPLE_FREQUENCY ) ) ).xyz;
	detailNormal = ( detailNormal * vec3( 2.0, 2.0, 1.0 ) ) - vec3( 1.0, 1.0, 0.0 );
	
	float distanceFromPixel = length( EYE_POSITION - PASSWORLDPOS );
	
	float blendFactor = ( distanceFromPixel - DETAIL_TEXTURE_MIN_DISTANCE ) / ( DETAIL_TEXTURE_MAX_DISTANCE - DETAIL_TEXTURE_MIN_DISTANCE );
	blendFactor = clamp( blendFactor, 0.0, 1.0 );
	blendFactor *= 0.5;	// Only ever allow the detail texture to take 50% of the weight at max
	
	surfaceNormal = mix( detailNormal, surfaceNormal, blendFactor );
	surfaceNormal = normalize( surfaceNormal );
   #endif
   
   mat3 surfaceToWorld = transpose(	
							mat3( normalize( PASSTANGENT ),
								  normalize( PASSBITANGENT ),
								  normalize( PASSNORMAL )
							) 
   );
   vec3 worldNormal = surfaceNormal * surfaceToWorld;
   return worldNormal;
}

vec3 GetAmbientLight()
{
	vec3 ambientLight = ambientLightColorAndIntensity.xyz * ambientLightColorAndIntensity.w;
	ambientLight = clamp( ambientLight, vec3(0.0), vec3(1.0) );
	return ambientLight;
}

vec3 GetLightDisplacement( int lightIndex )
{
	return ( PASSWORLDPOS - lights[ lightIndex ].position );
}

float GetLightDistance( int lightIndex )
{
	return length( GetLightDisplacement( lightIndex ) );
}

vec3 GetLightDirection( int lightIndex )
{
	vec3 lightDirection = normalize( GetLightDisplacement( lightIndex ) );
	lightDirection = mix( lightDirection, normalize( lights[ lightIndex ].direction ), lights[ lightIndex ].directionFactor );
	return lightDirection;
}

int IsLit( int lightIndex )
{
	if ( lightIndex != SHADOW_CASTING_LIGHT_INDEX )
	{
		return 1;
	}
		
	Light currentLight = lights[ SHADOW_CASTING_LIGHT_INDEX ];
	vec4 lightClipSpacePos = currentLight.lightProjection * currentLight.lightView * PASSMODEL * vec4( PASSWORLDPOS, 1.0 );
	vec3 lightNDCPos = lightClipSpacePos.xyz;
	lightNDCPos /= lightClipSpacePos.w;
	vec2 shadowMapUV = ( lightNDCPos.xy + vec2( 1.0 ) ) * vec2( 0.5 );
	
	float zValue = texture( shadowMap, shadowMapUV ).r;
	zValue = ( zValue * 2.0 ) - 1.0;
	
	if ( ( lightNDCPos.z - SHADOW_Z_BIAS ) <= zValue )
	{
		return 1;
	}
	else
	{
		return ( 1 - currentLight.castsShadows );
	}
}

vec3 GetOverallLightColor( int lightIndex )
{
	vec3 lightDirection = GetLightDirection( lightIndex );
	
	vec3 lightColor = lights[ lightIndex ].colorAndIntensity.xyz;
	float lightIntensity = lights[ lightIndex ].colorAndIntensity.w;
	
	Light currentLight = lights[ lightIndex ];
	
	float coneLightIntensity = 0.0; 
	vec3 frustumCenterDirection = normalize( currentLight.direction );
	float projectionFraction = dot( lightDirection, frustumCenterDirection );
	
	float innerAngleCos = cos( radians( currentLight.innerAngle ) * 0.5 );
	float outerAngleCos = cos( radians( currentLight.outerAngle ) * 0.5 );
	
	if ( projectionFraction <= outerAngleCos )
	{
		coneLightIntensity = 0.0;
	}
	else if ( projectionFraction > outerAngleCos )
	{
		coneLightIntensity = lightIntensity * ( ( projectionFraction - outerAngleCos ) / ( innerAngleCos - outerAngleCos ) );
		coneLightIntensity = clamp( coneLightIntensity, 0.0, lightIntensity );
	}

	lightIntensity = mix( lightIntensity, coneLightIntensity, currentLight.coneFactor );
	
	#ifdef APPLY_ATTENUATION
		float lightDistance = GetLightDistance( lightIndex );
		float attenuation = currentLight.attenuation.x + ( currentLight.attenuation.y * lightDistance ) + ( currentLight.attenuation.z * lightDistance * lightDistance );
		if ( attenuation > 1.0 )
		{
			lightIntensity /= attenuation;
		}
	#endif
	
	lightColor *= IsLit( lightIndex );
	lightColor *= lightIntensity;
	return lightColor;
}

vec3 GetTotalDiffuseLight( vec3 worldNormal )
{
	vec3 diffuseLight = vec3( 0.0 );
	
	#ifdef USE_DIFFUSE
		for ( int i = 0; i < MAX_LIGHTS; i++ )
		{
			vec3 lightDirection = GetLightDirection( i );
			vec3 lightColor = GetOverallLightColor( i );
			
			float incidentLightIntensity = dot( -lightDirection, worldNormal );
			vec3 diffuseLightFromLight = max( incidentLightIntensity, 0.0 ) * lightColor;
			diffuseLight += diffuseLightFromLight;
		}
		diffuseLight = clamp( diffuseLight, vec3(0.0), vec3(1.0) );
	#endif
	
	return diffuseLight;
}

vec3 GetTotalSpecularLight( vec3 worldNormal )
{
	vec3 specularLight = vec3( 0.0 );
	
	#ifdef USE_SPECULAR
		for ( int i = 0; i < MAX_LIGHTS; i++ )
		{
			vec3 lightDirection = GetLightDirection( i );
			vec3 lightColor = GetOverallLightColor( i );
		
			float incidentLightIntensity = dot( -lightDirection, worldNormal );
			lightColor = max( incidentLightIntensity, 0.0 ) * lightColor;
		
			vec3 eyeDirection = normalize( EYE_POSITION - PASSWORLDPOS );
			vec3 reflection = reflect( lightDirection, worldNormal );
			float specularFactor = max( dot( reflection, eyeDirection ), 0.0 );
			specularFactor = specularAmount * pow( specularFactor, specularPower );
			vec3 reflectedLight = lightColor * specularFactor;
			specularLight += reflectedLight;
		}
	#endif
	
	vec3 sampledSpecular = texture( texSpecular, PASSUV ).xyz;
	specularLight = specularLight * sampledSpecular;
	
	return specularLight;
}

void main( void )
{	
   vec3 worldNormal = GetWorldNormal();

   vec3 surfaceLight = GetAmbientLight() + GetTotalDiffuseLight( worldNormal );
   vec3 specularLight = GetTotalSpecularLight( worldNormal );
   
   vec4 texColor = texture( texDiffuse, PASSUV );
   
   vec4 finalColor = texColor;
   
   #ifdef USE_COLOR
	finalColor = finalColor * PASSCOLOR;
   #endif
   
   finalColor = finalColor * vec4( surfaceLight, 1 );
   finalColor = finalColor + vec4( specularLight, 0 );
   
   #ifdef USE_EMISSIVE
	 vec4 emissiveLight = texture( texEmissive, PASSUV );
	 finalColor = finalColor + vec4( emissiveLight.xyz, 0 );
   #endif
   
   //finalColor = clamp( finalColor, vec4(0), vec4(1) );
   outColor = finalColor;
   
   #ifdef USE_TEXTURE_ALPHA
	outColor.w = texColor.w;
   #endif
   
   outBloom = vec4( 0.0 );
   #ifdef USE_SPECULAR
	vec3 specularBloom = max( ( specularLight - vec3( 1.0 ) ), vec3( 0.0 ) );
	outBloom = vec4( specularBloom, 0.0 );
	outBloom = clamp( outBloom, vec4( 0.0 ), vec4( 1.0 ) );
	outBloom.w = 1.0;
   #endif
}
