#version 420 core

/*
Fullscreen Flip Y Effect - for Oculus VR
*/

in vec3 POSITION; // Assumes a clip position
in vec2 UV;

out vec2 PASSUV;

void main( void )
{
	gl_Position = vec4( POSITION, 1 );
	
	vec2 uvToPass = UV;
	uvToPass.y = 1.0 - uvToPass.y;	// The Mirror Texture's origin is on the top-left; ours is on the bottom-left
	PASSUV = uvToPass;
}