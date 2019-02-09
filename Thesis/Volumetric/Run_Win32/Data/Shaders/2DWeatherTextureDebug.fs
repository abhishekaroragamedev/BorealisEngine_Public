#version 430

layout(binding = 0) uniform sampler2D gTexDiffuse;

uniform FSUniforms
{
	vec4 CHANNEL_MASK;
	float UV_OFFSET;
};

in vec4 PASSCOLOR;
in vec2 PASSUV;

out vec4 outColor; 

void main( void )
{
   vec2 uv = PASSUV + vec2( 0.0f, UV_OFFSET );	// Change between x and y to debug different scrolling
   vec4 diffuse = texture( gTexDiffuse, uv );
   diffuse *= CHANNEL_MASK;
   diffuse.a = 1.0f;

   outColor = diffuse * PASSCOLOR;
}
