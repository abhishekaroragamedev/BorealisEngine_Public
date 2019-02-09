#version 430

#include "MathUtils/RangeMap.glsl"

#define LIGHT_RADIUS_SCALE 2.0

layout(binding = 0) uniform sampler2D gTexDiffuse;

layout(binding=1, std140) uniform CameraUBO
{
	mat4 VIEW;
	mat4 PROJECTION;
	vec3 EYE_POSITION;
	float PADDING;
};

uniform LightProps
{
	vec4 LIGHTCOLORANDINTENSITY;
	float LIGHTRADIUS;
};

in vec3 PASSLOCALPOS;
in vec2 PASSUV;

out vec4 outColor; 

void main( void )
{
   vec4 diffuse = texture( gTexDiffuse, PASSUV );
   float distanceFromCenter = length( PASSLOCALPOS );
   float brightnessFactor = RangeMapFloat( distanceFromCenter, 0.0, LIGHTRADIUS, 1.0, 0.0 );
   vec4 intensityMultiplier = vec4(LIGHTCOLORANDINTENSITY.xyz, brightnessFactor) * LIGHTCOLORANDINTENSITY.w;
   outColor = diffuse * intensityMultiplier;
}
