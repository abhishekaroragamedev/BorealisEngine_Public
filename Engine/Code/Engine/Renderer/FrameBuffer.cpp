#include "Engine/Core/StringUtils.hpp"
#include "Engine/Renderer/Framebuffer.hpp"
#include "Engine/Renderer/GLFunctionBinding.hpp"
#include "Engine/Renderer/GLUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Tools/DevConsole.hpp"

FrameBuffer::FrameBuffer()
{
	glGenFramebuffers( 1, &m_handle );
}

FrameBuffer::~FrameBuffer()
{
	if ( m_handle != NULL )
	{
		glDeleteFramebuffers( 1, &m_handle );
		m_handle = NULL;
	}
}

unsigned int FrameBuffer::GetHandle() const
{
	return m_handle;
}

unsigned int FrameBuffer::GetWidth() const
{
	return m_colorTargets[ 0 ]->m_dimensions.x;	// Both targets' dimensions will be the same
}

unsigned int FrameBuffer::GetHeight() const
{
	return m_colorTargets[ 0 ]->m_dimensions.y;
}

unsigned int FrameBuffer::GetColorTargetCount() const
{
	return static_cast< unsigned int >( m_colorTargets.size() );
}

bool FrameBuffer::IsMultiSample() const
{
	return (	( m_colorTargets.size() > 0 && m_colorTargets[ 0 ]->GetType() == TextureType::TEXTURE_TYPE_2D_MULTISAMPLE )	||
				( m_depthStencilTarget != nullptr && m_depthStencilTarget->GetType() == TextureType::TEXTURE_TYPE_2D_MULTISAMPLE )
	);
}

void FrameBuffer::SetColorTarget( Texture* colorTarget, unsigned int index /* = 0U */ )
{
	if ( colorTarget == nullptr )
	{
		return;
	}
	while ( index >= static_cast< unsigned int >( m_colorTargets.size() ) )
	{
		if ( index > static_cast< unsigned int >( m_colorTargets.size() ) )
		{
			ConsolePrintf( Rgba::YELLOW, "WARNING: FrameBuffer::SetColorTarget: null color targets will be set as index far exceeds current count." );
		}
		m_colorTargets.push_back( nullptr );
	}
	m_colorTargets[ index ] = colorTarget;
	
	MarkDirty();
}

void FrameBuffer::RemoveColorTarget( unsigned int index )
{
	Texture* target = m_colorTargets[ index ];
	m_colorTargets.erase( m_colorTargets.begin() + index );

	glBindFramebuffer( GL_FRAMEBUFFER, m_handle ); 
	switch( target->GetType() )
	{
		case TextureType::TEXTURE_TYPE_2D	:
		{
			glFramebufferTexture( GL_FRAMEBUFFER, ( GL_COLOR_ATTACHMENT0 + index ), 0, 0 );	// Unbind this TEXTURE
			break;
		}
		case TextureType::TEXTURE_TYPE_2D_MULTISAMPLE	:
		{
			glFramebufferTexture2D( GL_FRAMEBUFFER, ( GL_COLOR_ATTACHMENT0 + index ), GL_TEXTURE_2D_MULTISAMPLE, 0, 0 );	// Unbind this MULTISAMPLE TEXTURE
			break;
		}
	}

	MarkDirty();
}

void FrameBuffer::RemoveColorTarget( Texture* colorTarget )
{
	for ( size_t index = 0; index < m_colorTargets.size(); index++ )
	{
		if ( m_colorTargets[ index ] == colorTarget )
		{
			RemoveColorTarget( static_cast< unsigned int >( index ) );
			MarkDirty();
			return;
		}
	}
	ConsolePrintf( Rgba::YELLOW, "WARNING: FrameBuffer::RemoveColorTarget() - Target to be removed could not be found." );
}

void FrameBuffer::SetDepthStencilTarget( Texture* depthTarget )
{
	m_depthStencilTarget = depthTarget;
	MarkDirty();
}

void FrameBuffer::MarkDirty()
{
	m_isDirty = true;
}

#pragma region Finalize

void FrameBuffer::BindColorTargets()
{
	std::vector< GLenum > targets;
	for ( size_t colorTargetIndex = 0; colorTargetIndex < m_colorTargets.size(); colorTargetIndex++ )
	{
		GLenum attachmentPoint = GL_COLOR_ATTACHMENT0 + static_cast< GLenum >( colorTargetIndex );
		
		switch( m_colorTargets[ colorTargetIndex ]->GetType() )
		{
			case TextureType::TEXTURE_TYPE_2D	:
			{
				glFramebufferTexture( GL_FRAMEBUFFER, attachmentPoint, m_colorTargets[ colorTargetIndex ]->m_textureID, 0 );
				break;
			}
			case TextureType::TEXTURE_TYPE_2D_MULTISAMPLE	:
			{
				glFramebufferTexture2D( GL_FRAMEBUFFER, attachmentPoint, GL_TEXTURE_2D_MULTISAMPLE, m_colorTargets[ colorTargetIndex ]->m_textureID, 0 );
				break;
			}
		}

		targets.push_back( attachmentPoint );
	}
	glDrawBuffers( static_cast< GLsizei >( m_colorTargets.size() ), targets.data() );
}

void FrameBuffer::BindDepthTarget()
{
	if ( m_depthStencilTarget == nullptr )
	{
		glFramebufferTexture( GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, NULL, 0 );
	}
	else
	{
		switch( m_depthStencilTarget->GetType() )
		{
			case TextureType::TEXTURE_TYPE_2D	:
			{
				glFramebufferTexture( GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, m_depthStencilTarget->m_textureID, 0 );
				break;
			}
			case TextureType::TEXTURE_TYPE_2D_MULTISAMPLE	:
			{
				glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, m_depthStencilTarget->m_textureID, 0 );
				break;
			}
		}
	}
}

bool FrameBuffer::Finalize()
{
	if ( m_isDirty )
	{
		glBindFramebuffer( GL_FRAMEBUFFER, m_handle ); 

		BindColorTargets();
		BindDepthTarget();

		#if defined( _DEBUG )
				GLenum status = glCheckFramebufferStatus( GL_FRAMEBUFFER );
				if ( status != GL_FRAMEBUFFER_COMPLETE )
				{
					Stringf( "Failed to create FrameBuffer:  %u", status );
					ConsolePrintf( "Failed to create FrameBuffer:  %u", status );
					return false;
				}
		#endif
		m_isDirty = false;
		return true;
	}
	else
	{
		return false;
	}
}

#pragma endregion
