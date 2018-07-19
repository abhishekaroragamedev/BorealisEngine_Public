#version 420 core

in vec3 POSITION;
in vec4 COLOR;
in vec2 UV;

out vec4 PASSCOLOR;
out vec2 PASSUV;

void main( void )
{
   gl_Position = vec4( POSITION, 1 );
   PASSCOLOR = COLOR;
   PASSUV = UV;
}
