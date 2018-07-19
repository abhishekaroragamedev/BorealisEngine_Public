// define the shader version (this is required)
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

// Attributes - input to this shasder stage (constant as far as the code is concerned)
in vec3 POSITION;
in vec4 COLOR;
in vec2 UV;
in vec3 NORMAL;
in vec4 TANGENT;

out vec4 PASSCOLOR;
out vec2 PASSUV;
out vec3 PASSWORLDPOS;
out vec3 PASSNORMAL;
out vec3 PASSTANGENT;
out vec3 PASSBITANGENT;
out mat4 PASSMODEL;

// Entry point - required.  What does this stage do?
void main( void )
{
   // for now, we're going to set the 
   // clip position of this vertex to the passed 
   // in position. 
   // gl_Position is a "system variable", or have special 
   // meaning within the shader.
   vec4 world_pos = MODEL * vec4( POSITION, 1 );
   vec4 clip_pos = PROJECTION * VIEW * world_pos;
   gl_Position = clip_pos;

   vec4 world_normal = MODEL * vec4( NORMAL, 0 );
   
   PASSCOLOR = COLOR;
   PASSUV = UV;
   PASSWORLDPOS = world_pos.xyz;
   PASSNORMAL = world_normal.xyz;
   PASSTANGENT = ( MODEL * vec4( TANGENT.xyz, 0 ) ).xyz;
   PASSBITANGENT = cross( PASSTANGENT, world_normal.xyz ) * TANGENT.w;	// TANGENT.w is 1.0 if the normal is pointing towards the viewer
   PASSMODEL = MODEL;
}
