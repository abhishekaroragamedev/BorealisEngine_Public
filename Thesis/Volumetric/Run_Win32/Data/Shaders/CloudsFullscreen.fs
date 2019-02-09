#version 430

#define GAMMA_EXPONENT 1.0f / 2.2f
#define PI 3.142f
#define DIRECTIONAL_LIGHT_INDEX 0
#define MAX_LIGHTS 8

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

layout ( binding = 0 ) uniform sampler2D sceneTex;
layout ( binding = 1 ) uniform sampler2D scatTrans;
layout ( binding = 2 ) uniform sampler2D sceneNormals;
layout ( binding = 3 ) uniform sampler2D volumetricShadow;
layout ( binding = 4 ) uniform sampler2D sceneSpecular;
layout ( binding = 5 ) uniform sampler2D depthTarget;

in vec4 PASSCOLOR;
in vec2 PASSUV;

out vec4 outColor; 

Light GetSun()
{
	return lights[ DIRECTIONAL_LIGHT_INDEX ];
}

vec3 GetWorldNormal()
{
	vec3 worldNormal = ( texture( sceneNormals, PASSUV ) ).xyz;
	worldNormal = ( worldNormal * vec3( 2.0, 2.0, 1.0 ) ) - vec3( 1.0, 1.0, 0.0 );
	return worldNormal;
}

vec3 GetDiffuseLight( float shadow )
{
	vec3 worldNormal = GetWorldNormal();
	vec3 lightDirection = GetSun().direction;
	float incidentLightIntensity = dot( -lightDirection, worldNormal );
	vec3 lightColor = GetSun().colorAndIntensity.xyz * GetSun().colorAndIntensity.z * shadow;
	vec3 diffuseLight = max( incidentLightIntensity, 0.0 ) * lightColor;
	diffuseLight = clamp( diffuseLight, vec3(0.0), vec3(1.0) );

	return diffuseLight;
}


vec4 GetSceneColor()
{
	vec4 sceneColor = texture( sceneTex, PASSUV );
	float shadow = texture( volumetricShadow, PASSUV ).x;
	float depth = texture( depthTarget, PASSUV ).x;

	vec3 sceneLight = GetDiffuseLight( shadow ) + texture( sceneSpecular, PASSUV ).xyz;
	vec3 visibleLight = mix( sceneLight, vec3( 1.0f ), floor(depth) );	// If not rendered to depth, use existing color - no scene normals will be present, so this color will be lost (eg. sky)
	
	visibleLight = clamp( visibleLight, vec3( 0.0f ), vec3( 1.0f ) );
	sceneColor *= vec4( visibleLight, 1.0f );

	return sceneColor;
}

void main( void )
{
	vec4 albedo = GetSceneColor();
	vec4 scatTransColor = texture( scatTrans, PASSUV );
	vec3 scatteredLight = scatTransColor.xyz;
	float transmittance = scatTransColor.w;

	vec3 finalSceneColor = albedo.xyz * transmittance + scatteredLight;
	finalSceneColor = clamp(finalSceneColor, vec3(0.0f), vec3(1.0f) );
	finalSceneColor = pow( finalSceneColor, vec3(GAMMA_EXPONENT) );

	outColor = vec4( finalSceneColor, 1.0f );
}
