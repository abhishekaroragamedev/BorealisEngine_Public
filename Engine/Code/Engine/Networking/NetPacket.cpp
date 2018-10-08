#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Networking/NetMessage.hpp"
#include "Engine/Networking/NetPacket.hpp"
#include "Engine/Networking/UDPSocket.hpp"

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
	return WriteBytes( sizeof( NetPacketHeader ), &header );
}

bool NetPacket::ReadHeader( NetPacketHeader* out_header )
{
	return ReadBytes( out_header, sizeof( NetPacketHeader ) );
}

bool NetPacket::ReadMessageHeader( NetMessageHeader* out_header )
{
	return ReadBytes( out_header, 3U );
}

bool NetPacket::WriteMessage( const NetMessage& message )	// Increment message count as well
{
	bool success = WriteBytes( message.GetWrittenByteCount(), message.m_buffer );
	if ( success )
	{
		IncrementUnreliableCount();
	}
	return success;
}

bool NetPacket::ReadMessage( size_t readSize, NetMessage* out_message )
{
	return ReadBytes( out_message->m_buffer, readSize );
}

void NetPacket::IncrementUnreliableCount()
{
	NetPacketHeader* header = reinterpret_cast< NetPacketHeader* >( m_buffer );
	if ( header != nullptr )
	{
		header->m_unreliableCount++;
	}
}
