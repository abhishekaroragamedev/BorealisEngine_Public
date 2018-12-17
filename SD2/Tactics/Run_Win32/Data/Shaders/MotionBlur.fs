#version 420 core

/*
Motion Blur Effect - used by the Forward Renderer as a Post-render step
*/

layout(binding=0) uniform sampler2D diffuseTexture;

layout(binding=5, std140) uniform MiscUniformsFS
{
	float BLUR_FACTOR;	// 0 -> no blur, 1 -> the blur is fully from the edge to the center
};

in vec2 PASSUV;

out vec4 outColor;

vec4 Blur( vec4 color )
{
	float blurFactorClamped = clamp( BLUR_FACTOR, 0.0, 1.0 );	// The "Speed"
	float blurMultiplier = abs( PASSUV.x - 0.5 ) + abs( PASSUV.y - 0.5 );	// How close this pixel is to the edge of the screen
	blurMultiplier *= blurFactorClamped;
	
	vec4 finalColor = color;
	vec4 blurColor = color;

	float uvStep = 0.005;
	int numSteps = int(20.0 * blurMultiplier);	// More stretch towards the edge of the screen
	
	float weight = 1.0;
	float weightDivisor = 5.0;
	
	#ifdef BLUR_X
		for ( int step = 0; step < numSteps; step++ )
		{
			weight /= weightDivisor;
			vec4 sampleLeft = texture( diffuseTexture, ( PASSUV - vec2( ( step * uvStep ), 0.0 ) ) );
			vec4 sampleRight = texture( diffuseTexture, ( PASSUV - vec2( ( step * uvStep ), 0.0 ) ) );
			blurColor = mix( blurColor, sampleLeft, weight );
			blurColor = mix( blurColor, sampleRight, weight );
		}
	#endif
	
	#ifdef BLUR_Y
		for ( int step = 0; step < numSteps; step++ )
		{
			weight /= weightDivisor;
			vec4 sampleBottom = texture( diffuseTexture, ( PASSUV - vec2( 0.0, ( step * uvStep ) ) ) );
			vec4 sampleTop = texture( diffuseTexture, ( PASSUV - vec2( 0.0, ( step * uvStep ) ) ) );
			blurColor = mix( blurColor, sampleBottom, weight );
			blurColor = mix( blurColor, sampleTop, weight );
		}
	#endif
	
	finalColor = mix( finalColor, blurColor, blurMultiplier );	// More blur color towards the edge of the screen
	
	return finalColor;
}

void main( void )
{
	vec4 diffuseColor = texture( diffuseTexture, PASSUV );
	outColor = clamp( Blur( diffuseColor ), vec4( 0.0 ), vec4( 1.0 ) );
}