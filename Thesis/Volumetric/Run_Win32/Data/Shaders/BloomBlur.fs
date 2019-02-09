#version 430

/*
Fullscreen Bloom Effect - used by the Forward Renderer as a Post-render step
*/

layout(binding=0) uniform sampler2D diffuseTexture;
layout(binding=1) uniform sampler2D bloomTexture;

in vec2 PASSUV;

out vec4 outColor;

vec4 SampleFullscreenUVTexture( sampler2D tex, vec2 uv )
{
	return texture( tex, uv );
}

vec4 Blur( vec4 color )
{
	vec4 finalColor = color;

	float uvStep = 0.005;
	int numSteps = 5;
	
	float weight = 0.5;
	float weightDivisor = 5.0;
	
	#ifdef BLUR_X
		for ( int step = 0; step < numSteps; step++ )
		{
			weight /= weightDivisor;
			vec4 sampleLeft = SampleFullscreenUVTexture( bloomTexture, PASSUV - vec2( ( step * uvStep ), 0.0 ) );
			vec4 sampleRight = SampleFullscreenUVTexture( bloomTexture, PASSUV + vec2( ( step * uvStep ), 0.0 ) );
			finalColor = mix( finalColor, sampleLeft, weight );
			finalColor = mix( finalColor, sampleRight, weight );
		}
	#endif
	
	#ifdef BLUR_Y
		for ( int step = 0; step < numSteps; step++ )
		{
			weight /= weightDivisor;
			vec4 sampleBottom = SampleFullscreenUVTexture( bloomTexture, PASSUV - vec2( 0.0, ( step * uvStep ) ) );
			vec4 sampleTop = SampleFullscreenUVTexture( bloomTexture, PASSUV + vec2( 0.0, ( step * uvStep ) ) );
			finalColor = mix( finalColor, sampleBottom, weight );
			finalColor = mix( finalColor, sampleTop, weight );
		}
	#endif
	
	return finalColor;
}

void main( void )
{
	vec4 diffuseColor = SampleFullscreenUVTexture( diffuseTexture, PASSUV );
	vec4 bloomColor = SampleFullscreenUVTexture( bloomTexture, PASSUV );
	bloomColor = Blur( bloomColor );
	bloomColor.w = diffuseColor.w;
	outColor = clamp( ( diffuseColor + bloomColor ), vec4( 0.0 ), vec4( 1.0 ) );
}