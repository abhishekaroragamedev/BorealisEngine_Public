#version 430

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
	uvToPass.y = 1.0 - uvToPass.y;
	PASSUV = uvToPass;
}