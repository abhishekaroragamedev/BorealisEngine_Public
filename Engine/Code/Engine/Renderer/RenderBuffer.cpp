#include "Engine/Core/Vertex.hpp"
#include "Engine/Renderer/GLFunctionBinding.hpp"
#include "Engine/Renderer/GLUtils.hpp"
#include "Engine/Renderer/RenderBuffer.hpp"

RenderBuffer::RenderBuffer()
{
	
}

RenderBuffer::~RenderBuffer()
{
	if ( m_handle != NULL )
	{
		glDeleteBuffers( 1, &m_handle ); 
		m_handle = NULL; 
	}
}

GLuint RenderBuffer::GetHandle() const
{
	return m_handle;
}

bool RenderBuffer::CopyToGPU( const size_t byteCount, const void* data )
{
	// handle is a GLuint member - used by OpenGL to identify this buffer
	// if we don't have one, make one when we first need it [lazy instantiation]
	if ( m_handle == NULL )
	{
		glGenBuffers( 1, &m_handle ); 
	}

	return true; 
}

VertexBuffer::VertexBuffer()
{
	TODO( "Modify once there are different kinds of vertices" )
	m_vertexStride = sizeof( Vertex_3DPCU );
}

VertexBuffer::~VertexBuffer()
{

}

bool VertexBuffer::CopyToGPU( const size_t byteCount, const void* data )
{
	if ( RenderBuffer::CopyToGPU( byteCount, data ) )
	{
		glBindBuffer( GL_ARRAY_BUFFER, m_handle ); 
		glBufferData( GL_ARRAY_BUFFER, byteCount, data, GL_DYNAMIC_DRAW );
		m_bufferSize = static_cast< unsigned int >( byteCount ); 
		m_vertexCount = m_bufferSize / m_vertexStride;
		return true;
	}
	else
	{
		return false;
	}
}

void VertexBuffer::Finalize() const
{
	glBindBuffer( GL_ARRAY_BUFFER, m_handle );
}

IndexBuffer::IndexBuffer()
{
	m_indexStride = sizeof( unsigned int );
}

IndexBuffer::~IndexBuffer()
{

}

bool IndexBuffer::CopyToGPU( const size_t byteCount, const void* data )
{
	if ( RenderBuffer::CopyToGPU( byteCount, data ) )
	{
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_handle ); 
		glBufferData( GL_ELEMENT_ARRAY_BUFFER, byteCount, data, GL_DYNAMIC_DRAW ); 
		m_bufferSize = static_cast< unsigned int >( byteCount );
		m_indexCount = m_bufferSize / m_indexStride;
		return true;
	}
	else
	{
		return false;
	}
}

void IndexBuffer::Finalize() const
{
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_handle ); 
}
