#version 430

#define LIGHT_RADIUS_SCALE 5.0

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
in vec2 UV;

out vec3 PASSLOCALPOS;
out vec2 PASSUV;

void main( void )
{
	PASSLOCALPOS = POSITION;
	PASSUV = UV;

   vec4 local_pos = vec4( POSITION, 1 );
   vec4 clip_pos = PROJECTION * VIEW * MODEL * local_pos;
   gl_Position = clip_pos;
}
