#version 430

layout(binding=1, std140) uniform CameraUBO
{
	mat4 VIEW;
	mat4 PROJECTION;
	vec3 EYE_POSITION;
	float PADDING;
};

layout(binding=4, std140) uniform ModelMatrix
{
	vec3 POSITION;
};

void main( void )
{
   vec4 local_pos = vec4( POSITION, 1 ); 
   vec4 clip_pos = PROJECTION * local_pos;

   gl_Position = clip_pos;
}
