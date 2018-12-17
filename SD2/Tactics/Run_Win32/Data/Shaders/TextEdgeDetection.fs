#version 420 core

layout(binding = 0) uniform sampler2D gTexDiffuse;

in vec4 PASSCOLOR;
in vec2 PASSUV;

float SOBEL_FILTER[] = {
	0, 1, 0
	-1, 0, -1,
	0, 1, 0
};

out vec4 outColor;

float GetEdgeColor()
{
	vec2 textureSize2D = textureSize( gTexDiffuse, 0 );
	vec2 stepSize = vec2( ( 1 / textureSize2D.x ), ( 1 / textureSize2D.y ) );

	int DIAMETER = 3;	// Based on the SOBEL_FILTER array
	int RADIUS = 1;
	
	float edgeFactor = 0.0;
	
	for ( int y = 0; y < DIAMETER; y++ )
	{
		for ( int x = 0; x < DIAMETER; x++ )
		{
			vec2 offset = vec2( ( x - RADIUS, y - RADIUS ) );	// Works because (1,1) represents the current pixel
			vec2 uvOffset = vec2( ( stepSize.x * offset.x ), ( stepSize.y * offset.y ) );
			
			int index = ( y * DIAMETER ) + x;
			edgeFactor += SOBEL_FILTER[ index ] * texture( gTexDiffuse, fract( PASSUV + uvOffset ) ).r;
		}
	}
	
	return edgeFactor;
}

void main( void )
{
	vec4 diffuse = texture( gTexDiffuse, PASSUV );
	vec4 edgeColor = vec4( vec3( GetEdgeColor() ), 1 );
	outColor = diffuse * PASSCOLOR;
}
