#version 420 core

/*
Fullscreen Bloom Effect - used by the Forward Renderer as a Post-render step
*/

in vec3 POSITION; // Assumes a clip position
in vec2 UV;

out vec2 PASSUV;

void main( void )
{
	gl_Position = vec4( POSITION, 1 );
	PASSUV = UV;
}