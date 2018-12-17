#version 420 core

layout(binding=0) uniform sampler2D texDiffuse;

in vec2 PASSUV;
in vec4 PASSCOLOR;

out vec4 outColor; 

void main( void )
{
   vec4 diffuse = texture(texDiffuse, PASSUV);
   outColor = PASSCOLOR;
   outColor *= diffuse;
   outColor.a = diffuse.a;
}
