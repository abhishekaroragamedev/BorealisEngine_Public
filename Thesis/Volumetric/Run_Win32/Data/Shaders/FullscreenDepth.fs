#version 430

layout(binding = 0) uniform sampler2D gTexDiffuse;

in vec4 PASSCOLOR;
in vec2 PASSUV;

out vec4 outColor; 

void main( void )
{
   vec4 diffuse = texture( gTexDiffuse, PASSUV );

   // Uncomment to make depth look more pronounced for drawing
   diffuse.x *= diffuse.x * diffuse.x * diffuse.x * diffuse.x * diffuse.x;

   diffuse.y = diffuse.x;
   diffuse.z = diffuse.x;

   outColor = diffuse * PASSCOLOR;  
}
