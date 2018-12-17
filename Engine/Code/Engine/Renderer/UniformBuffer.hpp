#pragma once

#include "Engine/Renderer/RenderBuffer.hpp"

class UniformBuffer : public RenderBuffer
{

	friend class Material;

public:
	UniformBuffer();
	~UniformBuffer();

public:
	void SetCPUData( const size_t byteCount, const void* data );
	void SetGPUData( const size_t byteCount, const void* data );
	bool CopyToGPU();	// Assumes pre-set data
	bool CopyToGPU( const size_t byteCount, const void* data ) override;
	void DeleteRawData();

	template < typename T >
	void SetData( const T& data )
	{
		SetCPUData( sizeof( T ), &data );
	}
	template < typename T >
	void SetDataAndCopyToGPU( const T& data )
	{
		SetGPUData( sizeof( T ), &data );
	}
	template < typename T >
	const T* As() const
	{
		return reinterpret_cast< const T* >( GetCPUData() );
	}
	template < typename T >
	T* As()
	{
		return reinterpret_cast< T* >( GetCPUData() );
	}

	void* GetCPUData();
	const void* GetCPUData() const;
	unsigned int GetDataSize() const;
	bool IsDirty() const;

public:
	template < typename T >
	static UniformBuffer* For( T structure )
	{
		UniformBuffer* ubo = new UniformBuffer;
		ubo->SetData< T >( structure );
		return ubo;
	}

protected:
	void* m_rawData = nullptr;
	unsigned int m_rawDataSize = 0U;
	bool m_isDirty = false;

};