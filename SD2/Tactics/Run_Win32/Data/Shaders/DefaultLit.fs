#version 420 core

#define EPSILON 0.00001

#define MAX_LIGHTS 8
#define SHADOW_Z_BIAS 0.25
#define SHADOW_CASTING_LIGHT_INDEX 0 // Only the first light ever casts shadows for now

layout(binding=0) uniform sampler2D texDiffuse;
layout(binding=1) uniform sampler2D texNormal;

layout (binding=5) uniform sampler2DShadow shadowMap;
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

layout(binding=5, std140) uniform FogSettings
{
	vec4 fogColor;
	float fogNearFactor;
	float fogFarFactor;
	float fogNearZ;
	float fogFarZ;
};

in float PASSDEPTH;
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
   mat3 surfaceToWorld = transpose( mat3( PASSTANGENT, PASSBITANGENT, PASSNORMAL ) );
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

float IsLit( int lightIndex )
{	
	Light currentLight = lights[ lightIndex ];

	vec3 worldPos = PASSWORLDPOS - ( currentLight.direction * SHADOW_Z_BIAS );
	vec4 lightClipSpacePos = currentLight.lightProjection * currentLight.lightView * vec4( worldPos, 1.0 );
	vec3 lightNDCPos = lightClipSpacePos.xyz;
	lightNDCPos /= lightClipSpacePos.w;
	lightNDCPos = ( lightNDCPos + vec3( 1.0 ) ) * 0.5;
	float zValue = texture( shadowMap, lightNDCPos ).r;
	return zValue;
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
	
	return specularLight;
}

vec4 BlendWithFog( vec4 color )
{
	float depthFactor = ( PASSDEPTH - fogNearZ ) / ( fogFarZ - fogNearZ );
	float fogFactor = mix( fogNearFactor, fogFarFactor, depthFactor );
	fogFactor *= fogColor.w;
	vec4 fogColorOpaque = vec4( fogColor.xyz, 1.0 );
	vec4 effectiveColor = mix( color, fogColorOpaque, fogFactor );
	return effectiveColor;
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
   
   finalColor = finalColor * vec4( surfaceLight, 1);
   finalColor = finalColor + vec4( specularLight, 0 );
   
   finalColor = clamp( finalColor, vec4(0), vec4(1) );
   outColor = finalColor;
   
   #ifdef USE_TEXTURE_ALPHA
	outColor.w = texColor.w;
   #endif
   
   #ifdef USE_FOG
	outColor = BlendWithFog( outColor );
   #endif
   
   outBloom = vec4( 0.0 );
   #ifdef USE_SPECULAR
	vec3 specularBloom = max( ( specularLight - vec3( 1.0 ) ), vec3( 0.0 ) );
	outBloom = vec4( specularBloom, 0.0 );
	outBloom = clamp( outBloom, vec4( 0.0 ), vec4( 1.0 ) );
	outBloom.w = 1.0;
   #endif
}