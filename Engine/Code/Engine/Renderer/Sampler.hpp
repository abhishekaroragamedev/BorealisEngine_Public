#pragma once
#include "Engine/Renderer/External/gl/glcorearb.h"
#include "Engine/Renderer/External/gl/glext.h"

class Rgba;
class Vector4;

enum SamplerWrapMode
{
	SAMPLER_WRAP_WRAP,
	SAMPLER_WRAP_CLAMP_EDGE,
	SAMPLER_WRAP_CLAMP_BORDER
};

enum SamplerSampleMode
{
	SAMPLER_LINEAR,
	SAMPLER_NEAREST,
	SAMPLER_LINEAR_MIPMAP_LINEAR
};

struct SamplerOptions
{
	SamplerWrapMode m_wrapMode = SamplerWrapMode::SAMPLER_WRAP_WRAP;
	SamplerSampleMode m_sampleMode = SamplerSampleMode::SAMPLER_LINEAR;
};

class Sampler
{

public:
	Sampler( SamplerOptions options );
	~Sampler();

	bool Create( SamplerOptions options );
	void Destroy();
	GLuint GetHandle() const;

	void SetBorderColor( const Rgba& color );
	void SetBorderColor( const Vector4& color );
	void UseForShadows();

private:
	GLuint m_samplerHandle = 0;

};
