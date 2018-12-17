#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Logger.hpp"
#include "Engine/Networking/NetConnection.hpp"
#include "Engine/Networking/NetMessage.hpp"
#include "Engine/Networking/NetSession.hpp"
#include "Engine/Networking/UDPSocket.hpp"
#include "Game/EngineBuildPreferences.hpp"

#if !defined( ENGINE_DISABLE_NETWORKING )

/* explicit */
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
	m_definition = NetSession::GetInstance()->GetMessageDefinition( messageIndex );

	if ( writeHeader )
	{
		NetMessageHeader header = NetMessageHeader();
		header.m_messageIndex = messageIndex;
		WriteHeader( header );
	}
}

NetMessage::NetMessage( const NetMessage& copy )
{
	m_definition = copy.m_definition;
	m_reliableID = copy.m_reliableID;
	m_sequenceID = copy.m_sequenceID;
	m_lastSentTimeMS = copy.m_lastSentTimeMS;
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
	const char* headerBytes = reinterpret_cast< const char* >( &header );
	bool success = BytePacker::WriteBytes( 2U, headerBytes );					// uint16_t m_messageLength
	success = success && BytePacker::WriteBytes( 1U, ( headerBytes + 2U ) );	// uint8_t m_messageIndex

	NetMessageDefinition* messageDefinition = NetSession::GetInstance()->GetMessageDefinition( header.m_messageIndex );
	m_definition = messageDefinition;

	bool isReliable = IsReliable();
	bool isInOrder = IsInOrder();
	if ( isReliable )
	{
		success = success && BytePacker::WriteBytes( 2U, ( headerBytes + 3U ) );	// uint16_t m_reliableID
		if ( isInOrder )
		{
			success = success && BytePacker::WriteBytes( 2U, ( headerBytes + 5U ) );	// uint16_t m_sequenceID
		}
	}

	if ( success )
	{
		// The size shouldn't include uint16_t m_messageLength
		uint16_t messageLengthFromHeader = static_cast< uint16_t >( GetHeaderNumBytesMinusLength( isReliable, isInOrder ) );
		UpdateMessageLength( messageLengthFromHeader );
	}
	return success;
}

bool NetMessage::ReadHeader( NetMessageHeader* out_header )
{
	char* headerBytes = reinterpret_cast< char* >( out_header );
	bool success = ReadBytes( headerBytes, 2U );					// uint16_t m_messageLength
	success = success && ReadBytes( ( headerBytes + 2U ), 1U );		// uint8_t m_messageIndex

	uint8_t messageIndex = *( reinterpret_cast< uint8_t* >( headerBytes + 2U ) );
	NetMessageDefinition* messageDefinition = NetSession::GetInstance()->GetMessageDefinition( messageIndex );

	bool isReliable = messageDefinition->m_options & NETMSG_OPTION_RELIABLE;
	bool isInOrder = isReliable && ( messageDefinition->m_options & NETMSG_OPTION_IN_ORDER );
	if ( isReliable )
	{
		success = success && ReadBytes( ( headerBytes + 3U ), 2U );		// uint16_t m_reliableID
		if ( isInOrder )
		{
			success = success && ReadBytes( ( headerBytes + 5U ), 2U );	// uint16_t m_sequenceID
		}
	}
	
	return success;
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

bool NetMessage::WriteConnectionInfo( const NetConnectionInfo& info )
{
	bool success = WriteBytes( CONNECTION_MAX_ID_LENGTH, &info.m_id );
	success = success && WriteBytes( 1U, &info.m_connectionIndex );
	return success;
}

bool NetMessage::ReadConnectionInfo( NetConnectionInfo* out_info )
{
	bool success = ReadBytes( &out_info->m_id, CONNECTION_MAX_ID_LENGTH );
	success = success && ReadBytes( &out_info->m_connectionIndex, 1U );
	return success;
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

bool NetMessage::IsConnectionless() const
{
	return m_definition->IsConnectionless();
}

bool NetMessage::IsReliable() const
{
	return m_definition->IsReliable();
}

bool NetMessage::IsInOrder() const
{
	return m_definition->IsInOrder();
}

uint16_t NetMessage::GetReliableID() const
{
	return m_reliableID;
}

bool NetMessage::SetReliableID( uint16_t reliableID )
{
	if ( !IsReliable() )
	{
		LogErrorf( "NetMessage::SetReliableID(): Cannot set reliable ID on an unreliable. Aborting..." );
		return false;
	}

	m_reliableID = reliableID;
	
	if ( m_buffer != nullptr )
	{
		uint16_t* reliableIDPtr = reinterpret_cast< uint16_t* >( m_buffer + 3U );	// Skip over uint16_t m_messageLength and uint8_t m_messageIndex
		*reliableIDPtr = m_reliableID;
	}
	
	return true;
}

uint16_t NetMessage::GetSequenceID() const
{
	return m_sequenceID;
}

bool NetMessage::SetSequenceID( uint16_t sequenceID )
{
	if ( !IsInOrder() )
	{
		LogErrorf( "NetMessage::SetSequenceID(): Cannot set sequence ID on a non-in-order message. Aborting..." );
		return false;
	}

	m_sequenceID = sequenceID;

	if ( m_buffer != nullptr )
	{
		uint16_t* sequenceIDPtr = reinterpret_cast< uint16_t* >( m_buffer + 5U );	// Skip over uint16_t m_messageLength, uint8_t m_messageIndex and uint16_t m_reliableID
		*sequenceIDPtr = m_sequenceID;
	}

	return true;
}

void NetMessage::UpdateMessageLength( uint16_t bytesAdded )
{
	NetMessageHeader* header = reinterpret_cast< NetMessageHeader* >( m_buffer );
	if ( header != nullptr )
	{
		header->m_messageLength += bytesAdded;
	}
}

/* static */
uint16_t NetMessage::GetHeaderNumBytes( bool isReliable, bool isInOrder )
{
	if ( isReliable )
	{
		if ( isInOrder )
		{
			return 7U;
		}
		return 5U;
	}
	return 3U;
}

/* static */
uint16_t NetMessage::GetHeaderNumBytes( const NetMessageDefinition& messageDefinition )
{
	return GetHeaderNumBytes( messageDefinition.IsReliable(), messageDefinition.IsInOrder() );
}

/* static */
uint16_t NetMessage::GetHeaderNumBytes( void* buffer )
{
	uint8_t messageIndex = *( reinterpret_cast< uint8_t* >( buffer ) + 2U );	// Skip over uint16_t m_messageLength
	NetMessageDefinition* messageDefinition = NetSession::GetInstance()->GetMessageDefinition( messageIndex );
	return GetHeaderNumBytes( *messageDefinition );
}

/* static */
uint16_t NetMessage::GetHeaderNumBytesMinusLength( bool isReliable, bool isInOrder )
{
	if ( isReliable )
	{
		if ( isInOrder )
		{
			return 5U;
		}
		return 3U;
	}
	return 1U;
}

/* static */
uint16_t NetMessage::GetHeaderNumBytesMinusLength( const NetMessageDefinition& messageDefinition )
{
	return GetHeaderNumBytesMinusLength( messageDefinition.IsReliable(), messageDefinition.IsInOrder() );
}

/* static */
uint16_t NetMessage::GetHeaderNumBytesMinusLength( void* buffer )
{
	uint8_t messageIndex = *( reinterpret_cast< uint8_t* >( buffer ) + 2U );	// Skip over uint16_t m_messageLength
	NetMessageDefinition* messageDefinition = NetSession::GetInstance()->GetMessageDefinition( messageIndex );
	return GetHeaderNumBytesMinusLength( *messageDefinition );
}

#endif	// #if !defined( ENGINE_DISABLE_NETWORKING )
