#version 420 core

layout(binding=1, std140) uniform CameraUBO
{
	mat4 VIEW;
	mat4 PROJECTION;
	vec3 EYE_POSITION;
	float PADDING;
};

in vec3 POSITION;
in vec4 COLOR;
in vec2 UV;

out vec4 PASSCOLOR;
out vec2 PASSUV;

void main( void )
{
	vec4 ndc_pos = vec4( POSITION, 1 );
	ndc_pos = PROJECTION * VIEW * ndc_pos;
	gl_Position = ndc_pos;

	PASSCOLOR = COLOR;
	PASSUV = UV;
}
