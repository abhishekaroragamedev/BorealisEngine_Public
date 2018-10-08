#version 420 core

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

out vec2 PASSUV;

void main( void )
{
   vec4 local_pos = vec4( POSITION, 1 );
   vec4 world_pos = MODEL * local_pos;
   vec4 clip_pos = PROJECTION * VIEW * world_pos;
   gl_Position = clip_pos;

   PASSUV = UV;
}
