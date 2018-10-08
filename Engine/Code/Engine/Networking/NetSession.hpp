#pragma once

#include "Engine/Math/FloatRange.hpp"
#include "Engine/Networking/NetConnection.hpp"
#include "Engine/Networking/NetMessage.hpp"
#include <string>

constexpr unsigned int SESSION_MAX_CONNECTIONS = 32;
constexpr uint8_t INVALID_CONNECTION_INDEX = 255U;

class NetPacket;
class UDPSocket;

struct DelayedPacket
{
	
public:
	DelayedPacket( NetConnection* connection, NetPacket* packet, FILETIME timestamp )
		:	m_connection( connection ),
			m_packet( packet ),
			m_timestamp( timestamp )
	{

	}
	DelayedPacket( const DelayedPacket& copy )
	{
		m_connection = copy.m_connection;
		m_packet = copy.m_packet;
		m_timestamp = copy.m_timestamp;
	}

public:
	FILETIME m_timestamp;
	NetConnection* m_connection;
	NetPacket* m_packet;

};

class NetSession
{

public:
	NetSession();
	~NetSession();

	void RegisterMessage( const std::string& name, message_cb callback );
	void AddBinding( unsigned int port, unsigned int portRange = 16U );
	NetConnection* AddConnection( unsigned int index, const NetAddressIPv4& addr );
	void SetSimLoss( float simLoss );
	void SetSimLag( float minLag, float maxLag );

	NetConnection* GetConnection( unsigned int index ) const;
	unsigned int GetNumConnections() const;
	uint8_t GetMessageIndex( const char* message ) const;
	UDPSocket* GetSocket() const;

	void ProcessIncoming();
	void ProcessOutgoing();

private:
	void AddIncomingPacketToQueue( NetConnection* connection, NetPacket* packet );
	void ProcessIncomingPacketQueue();

private:
	uint8_t GetMyConnectionIndex() const { return INVALID_CONNECTION_INDEX; }
	NetConnection* GetConnectionWithAddress( const NetAddressIPv4& addr ) const;
	bool VerifyPacket( NetPacket& packet );

public:
	static NetSession* GetInstance();

private:
	UDPSocket* m_socket = nullptr;

	NetConnection* m_connections[ SESSION_MAX_CONNECTIONS ] = {};
	unsigned int m_numConnections = 0U;

	// Simulation configuration
	float m_simLoss = 0.0f;
	FloatRange m_simLag;

	std::map< std::string, NetMessageDefinition > m_messageHandlers;
	std::vector< DelayedPacket > m_incomingPacketQueue;

};
