#version 420 core

in vec2 PASSUV;

out vec4 outColor; 

void main( void )
{
   outColor = vec4( PASSUV, 0, 1 );
}
