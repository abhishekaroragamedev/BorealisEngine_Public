#version 430

#include "MathUtils/RangeMap.glsl"

layout(binding = 8) uniform sampler3D gTexVolume;

uniform FSUniforms
{
	vec4 CHANNEL_MASK;
	float NUM_LAYERS;
	float LAYER_MASK;
	float WORLEY_INVERSE;
};

in vec4 PASSCOLOR;
in vec2 PASSUV;

out vec4 outColor; 

void main( void )
{
   float w = ( LAYER_MASK + 1.0f ) / NUM_LAYERS;
   vec3 uvw = vec3( PASSUV, w );

   vec4 diffuse = textureLod( gTexVolume, uvw, 0.0f );
   diffuse *= CHANNEL_MASK;

   // Hardcoded because it's a debug tool - also, only one of shape or detail will work at a time :P
   float minVal = min( diffuse.x, min( diffuse.y , min( diffuse.z, diffuse.w ) ) );
   if ( minVal > 0.0f )	// Debug combined
   {
		diffuse.yzw = vec3( 1.0f ) - diffuse.yzw;
		diffuse.x = diffuse.x + ( 0.5f * diffuse.y * diffuse.x + 0.5f * diffuse.z * diffuse.y * diffuse.x + 0.5 * diffuse.w * diffuse.z * diffuse.y * diffuse.x ) / 2.5f;	// Shape
		// diffuse.x = ( diffuse.x + diffuse.y + diffuse.z ) * 0.33f;
		diffuse.x = clamp( diffuse.x, 0.0f, 1.0f );
		diffuse.x = RangeMapFloat( diffuse.x, 0.4f, 1.0f, 0.0f, 1.0f );
		diffuse.x = clamp( diffuse.x, 0.0f, 1.0f );
   }
   else	// Debug single channel
   {
		diffuse.x = max( diffuse.x, max( diffuse.y , max( diffuse.z, diffuse.w ) ) );
		float inverseDiffuseX = 1.0f - diffuse.x;
		diffuse.x = mix( diffuse.x, inverseDiffuseX, WORLEY_INVERSE );
   }

   diffuse.y = diffuse.x;
   diffuse.z = diffuse.x;
   diffuse.a = 1.0f;

   outColor = diffuse * PASSCOLOR;
}
