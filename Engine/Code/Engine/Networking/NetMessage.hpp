#pragma once

#include "Engine/Core/BytePacker.hpp"
#include "Engine/Core/EngineCommon.hpp"

struct NetConnectionInfo;
class NetConnection;
class NetMessage;

typedef bool ( *message_cb )( NetMessage&, NetConnection& );

#define MESSAGE_DEFINITION_NO_INDEX -1

enum NetMessageOption	:	unsigned int
{
	NETMSG_OPTION_CONNECTIONLESS	= BIT_SHIFT( 0 ),
	NETMSG_OPTION_RELIABLE			= BIT_SHIFT( 1 ),
	NETMSG_OPTION_IN_ORDER			= BIT_SHIFT( 2 ),
	NUM_NETMSG_OPTIONS
};

class NetMessageDefinition
{

public:
	NetMessageDefinition( const std::string& name, message_cb callback, unsigned int options = 0U, int messageIndex = MESSAGE_DEFINITION_NO_INDEX, unsigned int channelIndex = 0U )
		:	m_name( name ),
			m_callback( callback ),
			m_options( options ),
			m_messageIndex( messageIndex ),
			m_channelIndex( channelIndex )
	{

	}

	bool IsConnectionless() const
	{
		return ( ( m_options & NETMSG_OPTION_CONNECTIONLESS ) > 0U );
	}

	bool IsReliable() const
	{
		return ( ( m_options & NETMSG_OPTION_RELIABLE ) > 0U );
	}

	bool IsInOrder() const
	{
		return ( IsReliable() && ( ( m_options & NETMSG_OPTION_IN_ORDER ) > 0U ) );
	}

public:
	std::string m_name = "";
	message_cb m_callback = nullptr;
	unsigned int m_options = 0U;
	unsigned int m_channelIndex = 0U;
	int m_messageIndex = MESSAGE_DEFINITION_NO_INDEX;

};

struct NetMessageHeader
{

public:
	uint16_t m_messageLength = 0U;
	uint8_t m_messageIndex = 0U;
	uint16_t m_reliableID = 0U;
	uint16_t m_sequenceID = 0U;

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
	explicit NetMessage( bool writeHeader = true );
	explicit NetMessage( uint8_t messageIndex, bool writeHeader = true );
	NetMessage( const NetMessage& copy );
	~NetMessage();

	bool WriteHeader( const NetMessageHeader& header );
	bool ReadHeader( NetMessageHeader* out_header );

	size_t ReadFloat( float* out_value );
	size_t ReadInt( int* out_value );
	bool WriteFloat( float value );
	bool WriteInt( int value );

	bool WriteConnectionInfo( const NetConnectionInfo& info );
	bool ReadConnectionInfo( NetConnectionInfo* out_info );

	bool WriteBytes( size_t byteCount, const void* data ) override;
	bool WriteString( const char* str ) override;
	size_t WriteSize( size_t size ) override;

	template <typename T>
	size_t Read( T* out )
	{
		return ReadBytes( out, sizeof(T) );
	}
	template <typename T>
	bool Write(const T* data)
	{
		return WriteBytes( sizeof(T), data );
	}

	bool IsConnectionless() const;	// If a header has already been written, checks the MessageDefinition for the NETMSG_CONNECTIONLESS flag
	bool IsReliable() const;		// If a header has already been written, checks the MessageDefinition for the NETMSG_RELIABLE flag
	bool IsInOrder() const;			// If a header has already been written, checks the MessageDefinition for the NETMSG_INORDER flag

	uint16_t GetReliableID() const;	// Will return 0U for an unreliable
	bool SetReliableID( uint16_t reliableID );
	uint16_t GetSequenceID() const;
	bool SetSequenceID( uint16_t sequenceID );

public:
	static uint16_t GetHeaderNumBytes( bool isReliable, bool isInOrder );
	static uint16_t GetHeaderNumBytes( const NetMessageDefinition& messageDefinition );
	static uint16_t GetHeaderNumBytes( void* buffer );	// Get the number of bytes from a header starting at the provided location
	static uint16_t GetHeaderNumBytesMinusLength( bool isReliable, bool isInOrder );
	static uint16_t GetHeaderNumBytesMinusLength( const NetMessageDefinition& messageDefinition );
	static uint16_t GetHeaderNumBytesMinusLength( void* buffer );	// Get the number of bytes from a header starting at the provided location

private:
	void UpdateMessageLength( uint16_t bytesAdded );

public:
	// Redundant info
	NetMessageDefinition* m_definition = nullptr;
	uint16_t m_reliableID = 0U;
	uint16_t m_sequenceID = 0U;
	float m_lastSentTimeMS = 0.0f;

};

class NetMessageChannel
{

public:
	uint16_t m_nextSequenceIDToSend = 0U;				// Sending
	uint16_t m_nextSequenceIDExpected = 0U;				// Receiving
	std::vector< NetMessage* > m_outOfOrderMessages;	// Receiving

};
