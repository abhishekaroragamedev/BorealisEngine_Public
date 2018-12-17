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

uniform float BLOCKSIZE;
uniform float THICKNESS;

// Attributes - input to this shasder stage (constant as far as the code is concerned)
in vec3 POSITION;
in vec4 COLOR;
in vec2 UV;

out vec3 PASSPOSITION;
out vec4 PASSCOLOR;
out vec2 PASSUV;
out float PASSBLOCKSIZE;
out float PASSTHICKNESS;

// Entry point - required.  What does this stage do?
void main( void )
{
   // for now, we're going to set the 
   // clip position of this vertex to the passed 
   // in position. 
   // gl_Position is a "system variable", or have special 
   // meaning within the shader.
   vec4 local_pos = vec4( POSITION, 1 );
   vec4 clip_pos = PROJECTION * VIEW * MODEL * local_pos;
   gl_Position = clip_pos;
   
   float IPOSITION;
   PASSPOSITION = POSITION;
   PASSCOLOR = COLOR;
   PASSUV = UV;
   PASSBLOCKSIZE = BLOCKSIZE;
   PASSTHICKNESS = THICKNESS;
}
