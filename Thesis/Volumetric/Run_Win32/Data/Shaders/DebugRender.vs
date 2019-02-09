// define the shader version (this is required)
#version 430

layout(binding=1, std140) uniform CameraUBO
{
	mat4 VIEW;
	mat4 PROJECTION;
	vec3 EYE_POSITION;
	float PADDING;
};

uniform MiscUniformsVS
{
	vec4 STARTCOLOR;
	vec4 ENDCOLOR;
	float TIMENORMALIZED;
};

// Attributes - input to this shasder stage (constant as far as the code is concerned)
in vec3 POSITION;
in vec4 COLOR;
in vec2 UV;

out vec4 PASSCOLOR;
out vec4 PASSRENDERCOLOR;
out vec2 PASSUV;

// Entry point - required.  What does this stage do?
void main( void )
{
   // for now, we're going to set the 
   // clip position of this vertex to the passed 
   // in position. 
   // gl_Position is a "system variable", or have special 
   // meaning within the shader.
   vec4 local_pos = vec4( POSITION, 1 );
   vec4 clip_pos = PROJECTION * VIEW * local_pos;
   gl_Position = clip_pos;

   PASSCOLOR = COLOR;
   PASSUV = UV;
   PASSRENDERCOLOR = mix( STARTCOLOR, ENDCOLOR, TIMENORMALIZED );
}
