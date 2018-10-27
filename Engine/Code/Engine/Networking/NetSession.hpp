#pragma once

#include "Engine/Math/FloatRange.hpp"
#include "Engine/Networking/NetConnection.hpp"
#include "Engine/Networking/NetMessage.hpp"
#include <string>

constexpr uint8_t SESSION_MAX_CONNECTIONS = 32U;
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

	void RegisterMessage( const std::string& name, message_cb callback, unsigned int options = 0U );	// Registers non-indexed messages
	bool RegisterMessage( int messageIndex, const std::string& name, message_cb callback, unsigned int options = 0U );	// Registers indexed messages
	void AddBinding( unsigned int port, unsigned int portRange = 16U );
	NetConnection* AddConnection( uint8_t index, const NetAddressIPv4& addr );
	void SetSimLoss( float simLoss );
	void SetSimLag( float minLag, float maxLag );
	void SetSendRate( float sendRateHz );
	void SetHeartRate( float heartRateHz );

	uint8_t GetMyConnectionIndex() const;
	NetConnection* GetMyConnection() const;
	NetConnection* GetConnection( uint8_t index ) const;
	uint8_t GetNumConnections() const;
	uint8_t GetMessageIndex( const char* message ) const;
	UDPSocket* GetSocket() const;
	float GetSimLoss() const;
	FloatRange GetSimLag() const;
	float GetSendIntervalMilliseconds() const;
	float GetSendRate() const;

	void ProcessIncoming();
	void ProcessOutgoing();

private:
	void AddIncomingPacketToQueue( NetConnection* connection, NetPacket* packet );
	void ProcessIncomingPacketQueue();

private:
	NetConnection* GetConnectionWithAddress( const NetAddressIPv4& addr ) const;
	bool VerifyPacket( NetPacket& packet );

public:
	static NetSession* GetInstance();

private:
	UDPSocket* m_socket = nullptr;

	uint8_t m_myConnectionIndex = INVALID_CONNECTION_INDEX;
	NetConnection* m_connections[ SESSION_MAX_CONNECTIONS ] = {};

	float m_sendIntervalMS = 100.0f;

	std::map< std::string, NetMessageDefinition > m_messageHandlers;
	std::vector< DelayedPacket > m_incomingPacketQueue;

	// Simulation configuration
	float m_simLoss = 0.0f;
	FloatRange m_simLag;

};

enum NetCoreMessage : int
{

	NETMSG_PING = 0,
	NETMSG_PONG,
	NETMSG_HEARTBEAT,
	NETMSG_CORE_COUNT

};
