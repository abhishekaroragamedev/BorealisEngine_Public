#include "Engine/Renderer/GLFunctionBinding.hpp"
#include "Engine/Renderer/PropertyBlock.hpp"
#include "Engine/Tools/DevConsole.hpp"

bool PropertyBlockInfo::ContainsProperty( const std::string& name ) const
{
	for ( PropertyInfo property : m_propertyInfos )
	{
		if ( property.m_name == name )
		{
			return true;
		}
	}
	return false;
}

PropertyInfo PropertyBlockInfo::GetPropertyInfo( const std::string& propertyName ) const
{
	for ( PropertyInfo property : m_propertyInfos )
	{
		if ( property.m_name == propertyName )
		{
			return property;
		}
	}
	return PropertyInfo( this );
}

const PropertyBlockInfo* ShaderProgramInfo::FindBlockInfo( const char* blockName ) const
{
	return FindBlockInfo( std::string( blockName ) );
}

const PropertyBlockInfo* ShaderProgramInfo::FindBlockInfo( const std::string& blockName ) const
{
	for ( size_t blockIndex = 0; blockIndex < m_propertyBlockInfos.size(); blockIndex++ )
	{
		if ( m_propertyBlockInfos[ blockIndex ].m_name == blockName )
		{
			return &m_propertyBlockInfos[ blockIndex ];
		}
	}
	return nullptr;
}

const PropertyBlockInfo* ShaderProgramInfo::FindContainingBlock( const char* propertyName ) const
{
	return FindContainingBlock( std::string( propertyName ) );
}

const PropertyBlockInfo* ShaderProgramInfo::FindContainingBlock( const std::string& propertyName ) const
{
	for ( size_t blockIndex = 0; blockIndex < m_propertyBlockInfos.size(); blockIndex++ )
	{
		for ( PropertyInfo propertyInfo : m_propertyBlockInfos[ blockIndex ].m_propertyInfos )
		{
			if ( propertyInfo.m_name == propertyName )
			{
				return &m_propertyBlockInfos[ blockIndex ];
			}
		}
	}
	return nullptr;
}

const PropertyInfo* ShaderProgramInfo::FindPropertyInfo( const char* propertyName ) const
{
	return FindPropertyInfo( std::string( propertyName ) );
}

const PropertyInfo* ShaderProgramInfo::FindPropertyInfo( const std::string& propertyName ) const
{
	for ( size_t blockIndex = 0; blockIndex < m_propertyBlockInfos.size(); blockIndex++ )
	{
		for ( size_t propertyIndex = 0; propertyIndex < m_propertyBlockInfos[ blockIndex ].m_propertyInfos.size(); propertyIndex++ )
		{
			if ( m_propertyBlockInfos[ blockIndex ].m_propertyInfos[ propertyIndex ].m_name == propertyName )
			{
				return &m_propertyBlockInfos[ blockIndex ].m_propertyInfos[ propertyIndex ];
			}
		}
	}
	return nullptr;
}

void PropertyBlock::SetCPUData( const std::string& propertyName, const size_t byteCount, const void* data )
{
	if ( !GetInfo()->ContainsProperty( propertyName ) )
	{
		ConsolePrintf( Rgba::RED, "ERROR: PropertyBlock::SetCPUData() : could not find property with name %s. Aborting...", propertyName.c_str() );
		return;
	}
	size_t propertyOffset = GetInfo()->GetPropertyInfo( propertyName ).m_offset;
	if ( m_rawData == nullptr )
	{
		m_rawData = malloc( m_info->m_maxSize );
		m_rawDataSize = static_cast< unsigned int >( m_info->m_maxSize );
	}

	memcpy( ( reinterpret_cast< char* >( m_rawData ) + propertyOffset ), data, byteCount ); // Typecast to char since it is one byte in size
	m_isDirty = true;
}

void PropertyBlock::Bind()
{
	glBindBufferBase( GL_UNIFORM_BUFFER, m_info->m_bindPoint, m_handle );
}
