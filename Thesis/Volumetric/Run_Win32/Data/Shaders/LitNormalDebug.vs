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
	mat4 MODEL;
};

in vec2 UV;
in vec3 POSITION;
in vec3 NORMAL;
in vec4 TANGENT;

out vec2 PASSUV;
flat out vec3 PASSNORMAL;
flat out vec3 PASSTANGENT;
flat out vec3 PASSBITANGENT;

void main( void )
{
   vec4 local_pos = vec4( POSITION, 1 );
   vec4 world_pos = MODEL * local_pos;
   vec4 clip_pos = PROJECTION * VIEW * world_pos;
   gl_Position = clip_pos;
   
   vec4 world_normal = MODEL * vec4( NORMAL, 0 );

   PASSUV = UV;
   PASSNORMAL = world_normal.xyz;
   PASSTANGENT = ( MODEL * vec4( TANGENT.xyz, 0 ) ).xyz;
   PASSBITANGENT = cross( PASSTANGENT, world_normal.xyz ) * TANGENT.w;	// TANGENT.w is 1.0 if the normal is pointing towards the viewer
}
