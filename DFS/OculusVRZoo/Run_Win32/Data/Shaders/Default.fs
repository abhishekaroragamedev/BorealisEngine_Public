#version 420 core

layout(binding = 0) uniform sampler2D gTexDiffuse;

in vec4 PASSCOLOR;
in vec2 PASSUV;

out vec4 outColor; 

void main( void )
{
   vec4 diffuse = texture( gTexDiffuse, PASSUV );
   outColor = diffuse * PASSCOLOR;  
}
