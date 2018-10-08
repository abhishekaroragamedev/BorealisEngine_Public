#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/Vector4.hpp"
#include "Engine/Renderer/GLFunctionBinding.hpp"
#include "Engine/Renderer/GLUtils.hpp"
#include "Engine/Renderer/Sampler.hpp"

Sampler::Sampler( SamplerOptions options ) 
	: m_samplerHandle( NULL )
{
	Create( options );
}

Sampler::~Sampler()
{
	Destroy();
}

bool Sampler::Create( SamplerOptions options )
{
	// create the sampler handle if needed; 
	if ( m_samplerHandle == NULL ) {
		glGenSamplers( 1, &m_samplerHandle ); 
		if ( m_samplerHandle == NULL ) {
			return false; 
		}
	}

	// Wrapping
	switch( options.m_wrapMode )
	{
		case SamplerWrapMode::SAMPLER_WRAP_WRAP		:
		{
			glSamplerParameteri( m_samplerHandle, GL_TEXTURE_WRAP_S, GL_REPEAT );
			glSamplerParameteri( m_samplerHandle, GL_TEXTURE_WRAP_T, GL_REPEAT );
			glSamplerParameteri( m_samplerHandle, GL_TEXTURE_WRAP_R, GL_REPEAT );
			break;
		}
		case SamplerWrapMode::SAMPLER_WRAP_CLAMP_EDGE	:
		{
			glSamplerParameteri( m_samplerHandle, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
			glSamplerParameteri( m_samplerHandle, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
			glSamplerParameteri( m_samplerHandle, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE );
			break;
		}
		case SamplerWrapMode::SAMPLER_WRAP_CLAMP_BORDER	:
		{
			glSamplerParameteri( m_samplerHandle, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
			glSamplerParameteri( m_samplerHandle, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
			glSamplerParameteri( m_samplerHandle, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER );
			break;
		}
	}

	// Filtering;
	switch( options.m_sampleMode )
	{
		case SamplerSampleMode::SAMPLER_LINEAR	:			
		{
			glSamplerParameteri( m_samplerHandle, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
			glSamplerParameteri( m_samplerHandle, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
			break;
		}
		case SamplerSampleMode::SAMPLER_NEAREST	:
		{
			glSamplerParameteri( m_samplerHandle, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
			glSamplerParameteri( m_samplerHandle, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
			break;
		}
		case SamplerSampleMode::SAMPLER_LINEAR_MIPMAP_LINEAR	:
		{
			glSamplerParameteri( m_samplerHandle, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
			glSamplerParameteri( m_samplerHandle, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
			break;
		}
	}

	return true; 
}

void Sampler::Destroy()
{
	if ( m_samplerHandle != NULL ) {
		glDeleteSamplers( 1, &m_samplerHandle ); 
		m_samplerHandle = NULL; 
	}
}

GLuint Sampler::GetHandle() const
{
	return m_samplerHandle;
}

void Sampler::SetBorderColor( const Rgba& color )
{
	Vector4 rgbaAsFloats = color.GetAsFloats();
	SetBorderColor( rgbaAsFloats );
}

void Sampler::SetBorderColor( const Vector4& color )
{
	GLfloat rgba[ 4 ] = {
		color.x,
		color.y,
		color.z,
		color.w
	};
	glSamplerParameterfv( m_samplerHandle, GL_TEXTURE_BORDER_COLOR, rgba );
}

void Sampler::UseForShadows()
{
	glSamplerParameteri( m_samplerHandle, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
	glSamplerParameteri( m_samplerHandle, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
	glSamplerParameteri( m_samplerHandle, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER );
	SetBorderColor( Rgba::WHITE );
	glSamplerParameteri( m_samplerHandle, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE );
	glSamplerParameteri( m_samplerHandle, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL );
}
