#pragma once

#include "Engine/Core/BytePacker.hpp"

struct NetMessageHeader;
class NetMessage;

struct NetPacketHeader
{

public:
	uint8_t m_senderConnectionIndex = 0U;
	uint8_t m_unreliableCount = 0U;

};

/*
	PACKET FORMAT
	[uint8_t senderConnectionIndex]
	[uint8_t messageCount]
	[message0]
	[message1]
	...
*/
class NetPacket	:	public BytePacker
{

public:
	NetPacket( uint8_t connectionIndex, bool writeHeader = true );

public:
	bool WriteHeader( const NetPacketHeader& header );
	bool ReadHeader( NetPacketHeader* out_header );

	bool ReadMessageHeader( NetMessageHeader* out_header );

	bool WriteMessage( const NetMessage& message );	// Increment message count as well
	bool ReadMessage( size_t readSize, NetMessage* out_message );

private:
	void IncrementUnreliableCount();

};
