#version 430

in vec3 POSITION;
in vec4 COLOR;
in vec2 UV;

out vec2 PASSUV;

void main( void )
{
	gl_Position = vec4( POSITION, 1.0f );
	PASSUV = UV;
}
