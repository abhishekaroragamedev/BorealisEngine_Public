#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Networking/NetMessage.hpp"
#include "Engine/Networking/UDPSocket.hpp"
#include "Game/EngineBuildPreferences.hpp"

#if !defined( ENGINE_DISABLE_NETWORKING )

NetMessage::NetMessage( bool writeHeader /* = true */ )
	:	BytePacker( MESSAGE_MTU, LITTLE_ENDIAN, BYTEPACKER_OWNS_MEMORY )
{
	if ( writeHeader )
	{
		NetMessageHeader header = NetMessageHeader();
		WriteHeader( header );
	}
}

/* explicit */
NetMessage::NetMessage( uint8_t messageIndex, bool writeHeader /* = true */ )
	:	BytePacker( MESSAGE_MTU, LITTLE_ENDIAN, BYTEPACKER_OWNS_MEMORY )
{
	if ( writeHeader )
	{
		NetMessageHeader header = NetMessageHeader();
		header.m_messageIndex = messageIndex;
		WriteHeader( header );
	}
}

NetMessage::NetMessage( const NetMessage& copy )
{
	m_options = copy.m_options;
	m_endianness = copy.m_endianness;
	m_bufferSize = copy.GetReadableByteCount();
	m_buffer = new char[ m_bufferSize ];
	memcpy( m_buffer, copy.m_buffer, m_bufferSize );
	m_readHead = m_buffer + ( copy.m_readHead - copy.m_buffer );
	m_writeHead = m_buffer + ( copy.m_writeHead - copy.m_buffer );
}

NetMessage::~NetMessage()
{

}

bool NetMessage::WriteHeader( const NetMessageHeader& header )
{
	// Avoid incrementing size by 3; increment by 1 only since the size shouldn't include the uint16_t in the header
	bool success = BytePacker::WriteBytes( 3, &header );	// sizeof( NetMessageHeader ) will return 4 to pad to nearest 16-bit boundary
	if ( success )
	{
		UpdateMessageLength( 1U );
	}
	return success;
}

bool NetMessage::ReadHeader( NetMessageHeader* out_header )
{
	return ReadBytes( out_header, 3 );	// sizeof( NetMessageHeader ) will return 4 to pad to nearest 16-bit boundary
}

size_t NetMessage::ReadFloat( float* out_value )
{
	return ReadBytes( out_value, sizeof( float ) );
}

size_t NetMessage::ReadInt( int* out_value )
{
	return ReadBytes( out_value, sizeof( int ) );
}

bool NetMessage::WriteFloat( float value )
{
	return WriteBytes( 4, &value );
}

bool NetMessage::WriteInt( int value )
{
	return WriteBytes( 4, &value );
}

bool NetMessage::WriteBytes( size_t byteCount, const void* data ) /* override */
{
	if ( BytePacker::WriteBytes( byteCount, data ) )
	{
		UpdateMessageLength( static_cast< uint16_t >( byteCount ) );
		return true;
	}
	return false;
}

bool NetMessage::WriteString( const char* str ) /* override */
{
	if ( BytePacker::WriteString( str ) )
	{
		UpdateMessageLength( static_cast< uint16_t >( strlen( str ) ) );
		return true;
	}
	return false;
}

size_t NetMessage::WriteSize( size_t size ) /* override */
{
	size_t writtenSize = BytePacker::WriteSize( size );
	if ( writtenSize > 0U )
	{
		UpdateMessageLength( static_cast< uint16_t >( writtenSize ) );
		return true;
	}
	return false;
}

void NetMessage::UpdateMessageLength( uint16_t bytesAdded )
{
	NetMessageHeader* header = reinterpret_cast< NetMessageHeader* >( m_buffer );
	if ( header != nullptr )
	{
		header->m_messageLength += bytesAdded;
	}
}

#endif	// #if !defined( ENGINE_DISABLE_NETWORKING )
