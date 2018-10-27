#pragma once

#include "Engine/Core/BytePacker.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Networking/NetConnection.hpp"

typedef bool ( *message_cb )( NetMessage&, NetConnection& );

#define MESSAGE_DEFINITION_NO_INDEX -1

enum NetMessageOption	:	unsigned int
{
	NETMSG_OPTION_CONNECTIONLESS	= BIT_SHIFT( 0 ),
	NETMSG_OPTION_RELIABLE			= BIT_SHIFT( 1 ),
	NUM_NETMSG_OPTIONS
};

class NetMessageDefinition
{

public:
	NetMessageDefinition( const std::string& name, message_cb callback, unsigned int options = 0U, int messageIndex = MESSAGE_DEFINITION_NO_INDEX )
		:	m_name( name ),
			m_callback( callback ),
			m_options( options ),
			m_messageIndex( messageIndex )
	{

	}

public:
	std::string m_name = "";
	message_cb m_callback = nullptr;
	unsigned int m_options = 0U;
	int m_messageIndex = MESSAGE_DEFINITION_NO_INDEX;

};

struct NetMessageHeader
{

public:
	uint16_t m_messageLength = 0U;
	uint8_t m_messageIndex = 0U;

};

/*
	MESSAGE FORMAT
	[uint16_t messageLength]
	[uint8_t messageIndex]
	[payload]...
*/
class NetMessage	:	public BytePacker
{

public:
	NetMessage( bool writeHeader = true );
	explicit NetMessage( uint8_t messageIndex, bool writeHeader = true );
	NetMessage( const NetMessage& copy );
	~NetMessage();

	bool WriteHeader( const NetMessageHeader& header );
	bool ReadHeader( NetMessageHeader* out_header );

	size_t ReadFloat( float* out_value );
	size_t ReadInt( int* out_value );
	bool WriteFloat( float value );
	bool WriteInt( int value );

	bool WriteBytes( size_t byteCount, const void* data ) override;
	bool WriteString( const char* str ) override;
	size_t WriteSize( size_t size ) override;

private:
	void UpdateMessageLength( uint16_t bytesAdded );

public:
	// Redundant info
	const NetMessageDefinition* m_definition;
	uint8_t m_definitionIndex = 0U;
	std::string m_definitionName = "";

};
