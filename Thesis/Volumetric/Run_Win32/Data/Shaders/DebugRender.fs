#version 430

layout(binding = 0) uniform sampler2D gTexDiffuse;

in vec4 PASSCOLOR;
in vec4 PASSRENDERCOLOR;
in vec2 PASSUV;

// Outputs
out vec4 outColor; 

// Entry Point
void main( void )
{
   vec4 diffuse = texture( gTexDiffuse, PASSUV );
   
   // multiply is component-wise
   // so this gets (diff.x * passColor.x, ..., diff.w * passColor.w)
   outColor = diffuse * PASSCOLOR * PASSRENDERCOLOR;  
}
