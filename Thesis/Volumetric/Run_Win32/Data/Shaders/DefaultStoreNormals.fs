#version 430

#define EPSILON 0.00001

#define DIRECTIONAL_LIGHT_INDEX 0
#define MAX_LIGHTS 8
#define SHADOW_Z_BIAS 0.25
#define SHADOW_CASTING_LIGHT_INDEX 0 // Only the first light ever casts shadows for now

layout(binding=0) uniform sampler2D texDiffuse;
layout(binding=1) uniform sampler2D texNormal;
layout(binding=2) uniform sampler2D texSpecular;
layout(binding=3) uniform sampler2D texEmissive;

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

layout( binding = 2, std140 ) uniform ActiveLights
{
	vec4 ambientLightColorAndIntensity;
	Light lights[ MAX_LIGHTS ];
};

layout(binding=3, std140) uniform ObjectLightProperties
{
	float specularAmount;
	float specularPower;
	vec2 padding;
};

in float PASSDEPTH;
in vec4 PASSCOLOR;
in vec2 PASSUV;
in vec3 PASSWORLDPOS;
in vec3 PASSNORMAL;
in vec3 PASSTANGENT;
in vec3 PASSBITANGENT;

layout(location=0) out vec4 outColor; 
layout(location=1) out vec4 outNormals; 
layout(location=2) out vec4 outSpecular; 

vec3 GetLightDisplacement( int lightIndex )
{
	return ( PASSWORLDPOS - lights[ lightIndex ].position );
}

vec3 GetLightDirection( int lightIndex )
{
	vec3 lightDirection = normalize( GetLightDisplacement( lightIndex ) );
	lightDirection = mix( lightDirection, normalize( lights[ lightIndex ].direction ), lights[ lightIndex ].directionFactor );
	return lightDirection;
}

vec3 GetWorldNormal()
{
   vec3 surfaceNormal = ( texture( texNormal, PASSUV ) ).xyz;
   surfaceNormal = ( surfaceNormal * vec3( 2.0f, 2.0f, 1.0f ) ) - vec3( 1.0f, 1.0f, 0.0f );
   mat3 surfaceToWorld = transpose( mat3( PASSTANGENT, PASSBITANGENT, PASSNORMAL ) );
   vec3 worldNormal = surfaceNormal * surfaceToWorld;
   return worldNormal;
}

vec3 GetOverallLightColor( int lightIndex )
{
	vec3 lightDirection = GetLightDirection( lightIndex );
	
	vec3 lightColor = lights[ lightIndex ].colorAndIntensity.xyz;
	float lightIntensity = lights[ lightIndex ].colorAndIntensity.w;
	
	Light currentLight = lights[ lightIndex ];
	//lightColor *= lightIntensity;
	return lightColor;
}

vec3 CalculateSpecular()
{
	vec3 specularLight = vec3( 0.0 );
	
	vec3 worldNormal = GetWorldNormal();
	vec3 lightDirection = GetLightDirection( DIRECTIONAL_LIGHT_INDEX );
	vec3 lightColor = GetOverallLightColor( DIRECTIONAL_LIGHT_INDEX );
	
	float incidentLightIntensity = dot( -lightDirection, worldNormal );
	lightColor = max( incidentLightIntensity, 0.0 ) * lightColor;
	
	vec3 eyeDirection = normalize( EYE_POSITION - PASSWORLDPOS );
	vec3 reflection = reflect( lightDirection, worldNormal );
	float specularFactor = max( dot( reflection, eyeDirection ), 0.0 );
	specularFactor = specularAmount * pow( specularFactor, specularPower );
	specularFactor *= texture( texSpecular, PASSUV ).r;	// Multiply by constant to see effect - too dim as of now
	vec3 reflectedLight = lightColor * specularFactor;
	specularLight = reflectedLight;
	
	return specularLight;
}

void main( void )
{  
   vec4 texColor = texture( texDiffuse, PASSUV );
   vec4 finalColor = texColor * PASSCOLOR;// vec4(ambientLightColorAndIntensity.xyz, 1.0f ) * ambientLightColorAndIntensity.w;
   finalColor = clamp( finalColor, vec4( 0.0f ), vec4( 1.0f ) );
   outColor = finalColor;
   outColor.w = PASSCOLOR.w;

   vec3 worldNormal = GetWorldNormal();
   worldNormal = ( worldNormal + vec3( 1.0f, 1.0f, 0.0f ) ) * vec3( 0.5f, 0.5f, 1.0f );
   outNormals = vec4( worldNormal, 1.0f );

   outSpecular = vec4( CalculateSpecular(), 1.0f );
}