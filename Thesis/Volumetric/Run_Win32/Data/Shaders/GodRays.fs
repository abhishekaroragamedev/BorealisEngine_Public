#version 430

#include "MathUtils/RangeMap.glsl"

#define DIRECTIONAL_LIGHT_INDEX 0
#define MAX_LIGHTS 8

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
	float	NUM_SAMPLES;
	float	WEIGHT;
	float	DECAY;
	float	EXPOSURE;
};

layout ( binding = 0 ) uniform sampler2D sceneTex;
layout ( binding = 1 ) uniform sampler2D scatTrans;

in vec4 PASSCOLOR;
in vec2 PASSUV;

out vec4 outColor; 

Light GetSun()
{
	return lights[ DIRECTIONAL_LIGHT_INDEX ];
}

vec3 GetSunWorldPos()
{
	return GetSun().position;
}

vec2 GetSunUVPos()
{
	vec4 worldPos = vec4( GetSunWorldPos(), 1.f );
	vec4 ndcPos = PROJECTION * VIEW * worldPos;
	ndcPos /= ndcPos.w;

	vec2 uvPos = RangeMapVec2( ndcPos.xy, vec2(-1.f), vec2(1.f), vec2(0.f), vec2(1.f) );
	return uvPos;
}

bool ShouldRender()
{
	vec2 sunUV = GetSunUVPos();
	return (
		sunUV.x >= 0.f && sunUV.x <= 1.f &&
		sunUV.y >= 0.f && sunUV.y <= 1.f
	);
}

void main( void )
{
	vec3 color = texture( sceneTex, PASSUV ).xyz;

	if ( ShouldRender() )
	{
		vec3 incrementalColor = vec3(0.f);
		vec2 samplePoint = PASSUV;
		vec2 step = samplePoint - GetSunUVPos();
		step *= 1.f / NUM_SAMPLES;
		float decayFactor = 1.f;

		for ( int i = 0; i < NUM_SAMPLES; i++ )
		{
			samplePoint -= step;

			vec4 scatTransSample = texture( scatTrans, samplePoint );
			incrementalColor += scatTransSample.xyz * scatTransSample.w * decayFactor * WEIGHT;

			decayFactor *= DECAY;
		}
		color += incrementalColor * EXPOSURE;
	}

	outColor = vec4( color, 1.0f );
}
