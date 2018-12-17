#pragma once

#include "Engine/Renderer/UniformBuffer.hpp"
#include <string>
#include <vector>

struct PropertyBlockInfo;

struct PropertyInfo
{

public:
	PropertyInfo( const PropertyBlockInfo* owningBlockInfo )
		:	m_owningBlockInfo( owningBlockInfo )
	{

	}
	PropertyInfo( const PropertyBlockInfo* owningBlockInfo, const std::string& name, size_t offset, size_t size, size_t count )
		:	m_owningBlockInfo( owningBlockInfo ),
			m_name( name ),
			m_offset( offset ),
			m_size( size ),
			m_count( count )
	{

	}
	PropertyInfo( const PropertyBlockInfo* owningBlockInfo, const char* name, size_t offset, size_t size, size_t count )
		:	m_owningBlockInfo( owningBlockInfo ),
			m_name( name ),
			m_offset( offset ),
			m_size( size ),
			m_count( count )
	{

	}

public:
	std::string m_name = "";
	size_t m_offset = 0;
	size_t m_size = 0;
	size_t m_count = 0;
	const PropertyBlockInfo* m_owningBlockInfo = nullptr;

};

struct PropertyBlockInfo
{

public:
	PropertyBlockInfo( const std::string& name, size_t maxSize = 0 )
		:	m_name( name ),
			m_maxSize( maxSize )
	{

	}
	PropertyBlockInfo( const char* name, size_t maxSize = 0 )
		:	m_name( name ),
			m_maxSize( maxSize )
	{

	}

	bool ContainsProperty( const std::string& name ) const;
	PropertyInfo GetPropertyInfo( const std::string& propertyName ) const;

public:
	std::string m_name = "";
	size_t m_maxSize = 0;
	unsigned int m_bindPoint = 0U;
	std::vector< PropertyInfo > m_propertyInfos;

};

struct SamplerBlockInfo
{

public:
	SamplerBlockInfo( const std::string& name, unsigned int bindPoint )
		:	m_name( name ),
			m_bindPoint( bindPoint )
	{

	}

public:
	std::string m_name = "";
	unsigned int m_bindPoint = 0U;

};

class ShaderProgramInfo
{

	friend class ShaderProgram;
	friend class Material;

public:
	ShaderProgramInfo() {}

	const PropertyBlockInfo* FindBlockInfo( const char* blockName ) const;
	const PropertyBlockInfo* FindBlockInfo( const std::string& blockName ) const;
	const PropertyBlockInfo* FindContainingBlock( const char* propertyName ) const;
	const PropertyBlockInfo* FindContainingBlock( const std::string& propertyName ) const;
	const PropertyInfo* FindPropertyInfo( const char* propertyName ) const;
	const PropertyInfo* FindPropertyInfo( const std::string& propertyName ) const;

private:
	std::vector< PropertyBlockInfo > m_propertyBlockInfos;
	std::vector< SamplerBlockInfo > m_samplerBlockInfos;

};

class PropertyBlock : public UniformBuffer
{

public:
	PropertyBlock( const PropertyBlockInfo* info )
		:	m_info( info )
	{

	}
	~PropertyBlock() {}

	void SetCPUData( const std::string& propertyName, const size_t byteCount, const void* data );
	void Bind();
	const PropertyBlockInfo* GetInfo() const
	{
		return m_info;
	}

private:
	const PropertyBlockInfo* m_info = nullptr;

};
