#version 430

/*
Shadow Map
*/

layout(binding=1, std140) uniform CameraUBO
{
	mat4 VIEW;
	mat4 PROJECTION;
	vec3 EYE_POSITION;
	float PADDING;
};

layout(binding=4, std140) uniform ModelMatrix
{
	mat4 MODEL;
};

in vec3 POSITION;

void main( void )
{
	vec4 worldPos = MODEL * vec4( POSITION, 1 );
	gl_Position = PROJECTION * VIEW * worldPos;
}