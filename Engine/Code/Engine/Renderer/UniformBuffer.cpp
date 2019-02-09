#include "Engine/Renderer/GLFunctionBinding.hpp"
#include "Engine/Renderer/UniformBuffer.hpp"
#include <cstdlib>

UniformBuffer::UniformBuffer()
{

}

UniformBuffer::~UniformBuffer()
{
	DeleteRawData();
}

void UniformBuffer::SetCPUData( const size_t byteCount, const void* data )
{
	DeleteRawData();
	
	m_rawData = malloc( byteCount );
	memcpy( m_rawData, data, byteCount );
	m_rawDataSize = static_cast< unsigned int >( byteCount );

	m_isDirty = true;
}

void UniformBuffer::SetGPUData( const size_t byteCount, const void* data )
{
	SetCPUData( byteCount, data );
	CopyToGPU( byteCount, data );
	m_isDirty = false;
}

bool UniformBuffer::CopyToGPU()
{
	return CopyToGPU( m_rawDataSize, m_rawData );
}

bool UniformBuffer::CopyToGPU( const size_t byteCount, const void* data ) /*override*/
{
	if ( RenderBuffer::CopyToGPU( byteCount, data ) )
	{
		glBindBuffer( GL_ARRAY_BUFFER, m_handle ); 
		glBufferData( GL_ARRAY_BUFFER, byteCount, data, GL_DYNAMIC_DRAW );
		m_isDirty = false;
		return true;
	}
	return false;
}

void UniformBuffer::DeleteRawData()
{
	if ( m_rawData != nullptr )
	{
		delete m_rawData;
		m_rawData = nullptr;
		m_rawDataSize = 0U;
		m_isDirty = true;
	}
}

void* UniformBuffer::GetCPUData()
{
	m_isDirty = true;
	return m_rawData;
}

const void* UniformBuffer::GetCPUData() const
{
	return m_rawData;
}

unsigned int UniformBuffer::GetDataSize() const
{
	return m_rawDataSize;
}

bool UniformBuffer::IsDirty() const
{
	return m_isDirty;
}
