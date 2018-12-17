#version 420 core

layout(binding = 0) uniform sampler2D gTexDiffuse;

in vec3 PASSPOSITION;
in vec4 PASSCOLOR;
in vec2 PASSUV;
in float PASSBLOCKSIZE;
in float PASSTHICKNESS;

// Outputs
out vec4 outColor; 

bool IsPositionOnEdge()
{
	vec3 positionMod = mod( PASSPOSITION, PASSBLOCKSIZE );
	return ( 
			( ( positionMod.x < PASSTHICKNESS || positionMod.x > ( 1.0 - PASSTHICKNESS ) ) && ( positionMod.y < PASSTHICKNESS || positionMod.y > ( 1.0 - PASSTHICKNESS ) ) ) ||
			( ( positionMod.y < PASSTHICKNESS || positionMod.y > ( 1.0 - PASSTHICKNESS ) ) && ( positionMod.z < PASSTHICKNESS || positionMod.z > ( 1.0 - PASSTHICKNESS ) ) ) ||
			( ( positionMod.z < PASSTHICKNESS || positionMod.z > ( 1.0 - PASSTHICKNESS ) ) && ( positionMod.x < PASSTHICKNESS || positionMod.x > ( 1.0 - PASSTHICKNESS ) ) )
	);
}

// Entry Point
void main( void )
{
	vec4 diffuse = texture( gTexDiffuse, PASSUV );
		
	// multiply is component-wise
	// so this gets (diff.x * passColor.x, ..., diff.w * passColor.w)
	outColor = diffuse * PASSCOLOR;
	
	if ( IsPositionOnEdge() )
	{
		outColor *= 0.75;
		outColor.a = 1;
	}
	
}
