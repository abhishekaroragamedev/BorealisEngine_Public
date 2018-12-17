#version 420 core

layout(binding = 0) uniform sampler2D gTexDiffuse;

in vec4 PASSCOLOR;
in vec2 PASSUV;
in float PASSBLENDFACTOR;

mat4 GRAYSCALE_TRANSFORM = mat4(
	vec4( 0.2126, 0.7152, 0.0722, 0 ),
	vec4( 0.2126, 0.7152, 0.0722, 0 ),
	vec4( 0.2126, 0.7152, 0.0722, 0 ),
	vec4( 0, 0, 0, 1 )
);

out vec4 outColor; 

void main( void )
{
	vec4 diffuse = texture( gTexDiffuse, PASSUV );
	vec4 finalColor = diffuse * PASSCOLOR * GRAYSCALE_TRANSFORM;
	outColor = mix( diffuse, finalColor, PASSBLENDFACTOR );
}
