#version 420 core

/*
Fullscreen Flip Y Effect - for Oculus VR
*/

layout(binding=0) uniform sampler2D diffuseTexture;

in vec2 PASSUV;

out vec4 outColor;

void main( void )
{
	outColor = texture( diffuseTexture, PASSUV );
	outColor.a = 1.0;	// The MirrorTexture is in SRGB space; we need to convert it to RGBA
}