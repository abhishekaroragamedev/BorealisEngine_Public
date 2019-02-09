#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Networking/NetMessage.hpp"
#include "Engine/Networking/NetPacket.hpp"
#include "Engine/Networking/UDPSocket.hpp"
#include "Game/EngineBuildPreferences.hpp"

#if !defined( ENGINE_DISABLE_NETWORKING )

NetPacket::NetPacket( uint8_t connectionIndex, bool writeHeader /* = true */ )
	:	BytePacker( PACKET_MTU, LITTLE_ENDIAN, BYTEPACKER_OWNS_MEMORY ) // Not allowed to grow, since it is a fixed size packet
{
	if ( writeHeader )
	{
		NetPacketHeader header;
		header.m_senderConnectionIndex = connectionIndex;
		WriteHeader( header );
	}
}

bool NetPacket::WriteHeader( const NetPacketHeader& header )
{
	bool success = true;
	success = success && WriteBytes( sizeof( uint8_t ), &header.m_senderConnectionIndex );
	success = success && WriteBytes( sizeof( uint16_t ), &header.m_ack );
	success = success && WriteBytes( sizeof( uint16_t ), &header.m_lastReceivedAck );
	success = success && WriteBytes( sizeof( uint16_t ), &header.m_previousAcksBitfield );
	success = success && WriteBytes( sizeof( uint8_t ), &header.m_messageCount );
	return success;
}

bool NetPacket::ReadHeader( NetPacketHeader* out_header )
{
	bool success = true;
	success = success && ReadBytes( &out_header->m_senderConnectionIndex, sizeof( uint8_t ) );
	success = success && ReadBytes( &out_header->m_ack, sizeof( uint16_t ) );
	success = success && ReadBytes( &out_header->m_lastReceivedAck, sizeof( uint16_t ) );
	success = success && ReadBytes( &out_header->m_previousAcksBitfield, sizeof( uint16_t ) );
	success = success && ReadBytes( &out_header->m_messageCount, sizeof( uint8_t ) );
	return success;
}

bool NetPacket::ReadMessageHeader( NetMessageHeader* out_header )
{
	size_t bytesRead = 0U;

	bool success = ReadBytes( &out_header->m_messageLength, sizeof( uint16_t ) );
	success = success && ReadBytes( &out_header->m_messageIndex, sizeof( uint8_t ) );
	
	bytesRead = 3U;

	if ( bytesRead < NetMessage::GetHeaderNumBytes( ( void* )out_header ) )
	{
		success = success && ReadBytes( &out_header->m_reliableID, sizeof( uint16_t ) );
	}

	bytesRead = 5U;
	if ( bytesRead < NetMessage::GetHeaderNumBytes( ( void* )out_header ) )
	{
		success = success && ReadBytes( &out_header->m_sequenceID, sizeof( uint16_t ) );
	}

	return success;
}

bool NetPacket::WriteMessage( const NetMessage& message )	// Increment message count as well
{
	bool success = WriteBytes( message.GetWrittenByteCount(), message.m_buffer );
	if ( success )
	{
		IncrementMessageCount();
	}
	return success;
}

bool NetPacket::ReadMessage( size_t readSize, NetMessage* out_message )
{
	return ReadBytes( out_message->m_buffer, readSize );
}

void NetPacket::IncrementMessageCount()
{
	if ( m_buffer != nullptr )
	{
		uint8_t* msgCountPtr = ( uint8_t* )( m_buffer + 7U );
		*msgCountPtr = *msgCountPtr + 1U;
	}
}

#endif	// #if !defined( ENGINE_DISABLE_NETWORKING )
