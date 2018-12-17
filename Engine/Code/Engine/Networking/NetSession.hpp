#pragma once

#include "Engine/Math/FloatRange.hpp"
#include "Engine/Networking/NetConnection.hpp"
#include "Engine/Networking/NetMessage.hpp"
#include <string>

constexpr uint8_t SESSION_MAX_CONNECTIONS = 32U;
constexpr uint16_t SESSION_DEFAULT_PORT = 10084U;
constexpr uint16_t SESSION_DEFAULT_PORT_RANGE = 16U;
constexpr float JOIN_REQUEST_TIMEOUT_SECONDS = 0.1f;
constexpr float JOIN_TIMEOUT_SECONDS = 10.0f;
constexpr float NET_MAX_TIME_DILATION = 0.1f;

class Clock;
class NetPacket;
class UDPSocket;

enum SessionState
{
	SESSION_STATE_INVALID = -1,
	SESSION_DISCONNECTED,
	SESSION_BOUND,
	SESSION_CONNECTING,
	SESSION_JOINING,
	SESSION_READY,
	SESSION_CONNECTED,
	NUM_SESSION_STATES
};

enum SessionError : uint8_t
{
	SESSION_OK,
	SESSION_ERROR_COULD_NOT_BIND,
	SESSION_ERROR_USER_DISCONNECTED,
	SESSION_ERROR_JOIN_TIMEOUT,
	SESSION_ERROR_JOIN_DENIED,
	SESSION_ERROR_JOIN_DENIED_NOT_HOST,
	SESSION_ERROR_JOIN_DENIED_CLOSED,
	SESSION_ERROR_JOIN_DENIED_FULL
};

struct DelayedPacket
{
	
public:
	DelayedPacket( NetConnection* connection, NetPacket* packet, float receivedTimeMS )
		:	m_connection( connection ),
			m_packet( packet ),
			m_receiveTimeMS( receivedTimeMS )
	{

	}
	DelayedPacket( const DelayedPacket& copy )
	{
		m_connection = copy.m_connection;
		m_packet = copy.m_packet;
		m_receiveTimeMS = copy.m_receiveTimeMS;
	}

public:
	float m_receiveTimeMS = 0.0f;
	NetConnection* m_connection;
	NetPacket* m_packet;

};

class NetSession
{

public:
	NetSession();
	~NetSession();

	/* MESSAGE REGISTRATION */
	void RegisterMessage( const std::string& name, message_cb callback, unsigned int options = 0U, unsigned int channelIndex = 0U );	// Registers non-indexed messages
	bool RegisterMessage( int messageIndex, const std::string& name, message_cb callback, unsigned int options = 0U, unsigned int channelIndex = 0U );	// Registers indexed messages
	
	/* LIFE CYCLE */
	void Update();
	void ProcessIncoming();
	void ProcessOutgoing();

	/* HOST/JOIN */
	bool Host( const char* myID, uint16_t port, uint16_t portRange = SESSION_DEFAULT_PORT_RANGE );
	void Join( const char* myID, const NetConnectionInfo& hostInfo );
	void Disconnect();

	/* ERROR HANDLING */
	void SetError( SessionError error, const char* errorStr = nullptr );
	void ClearError();
	SessionError GetLastError( std::string* out_error = nullptr );	// Clears the last error after returning it

	/* CONNECTIONS */
	NetConnection* CreateConnection( const NetAddressIPv4& addr );
	NetConnection* CreateConnection( const NetConnectionInfo& info );
	void DestroyConnection( NetConnection* connection );
	void BindConnection( uint8_t connectionIndex, NetConnection* connection );
	void BroadcastMessage( NetMessage& message );
	bool CanBindConnection( NetConnection* connection ) const;
	bool ConnectionExistsForAddress( const NetAddressIPv4& addr ) const;
	uint8_t GetBindableConnectionIndex() const;
	uint8_t GetNumBoundConnections() const;
	NetConnectionInfo GetConnectionInfo( uint8_t connectionIndex ) const;

	/* NET CLOCK */
	void SetClientTimeFromHostTime( float hostTimeMS, bool setCurrentTime = false );

	/* SIMULATION SETTINGS */
	void SetSimLoss( float simLoss );
	void SetSimLag( float minLag, float maxLag );
	void SetSendRate( float sendRateHz );
	void SetHeartRate( float heartRateHz );

	/* GETTERS */
	bool IsHost() const;
	uint8_t GetMyConnectionIndex() const;
	uint8_t GetNumConnections() const;
	uint8_t GetMessageIndex( const char* messageName ) const;
	float GetSimLoss() const;
	float GetSendIntervalMilliseconds() const;
	float GetSendRate() const;
	float GetNetTimeMS() const;
	Clock* GetNetClock() const;
	FloatRange GetSimLag() const;
	UDPSocket* GetSocket() const;
	NetConnection* GetMyConnection() const;
	NetConnection* GetHostConnection() const;
	NetConnection* GetConnection( uint8_t index ) const;
	NetConnection* GetConnectionWithAddress( const NetAddressIPv4& addr ) const;
	NetMessageDefinition* GetMessageDefinition( uint8_t messageIndex ) const;
	NetMessageDefinition* GetMessageDefinition( const char* messageName ) const;

private:
	/* SETUP */
	uint16_t AddBinding( uint16_t port, uint16_t portRange = SESSION_DEFAULT_PORT_RANGE );
	bool RemoveBinding();

	/* LIFE CYCLE */
	void UpdateNetTime();

	/* MESSAGE PROCESSING */
	void AddIncomingPacketToQueue( NetConnection* connection, NetPacket* packet );
	void ProcessIncomingPacketQueue();

	/* GETTERS */
	uint8_t GetIndexForConnection( NetConnection* connection ) const;
	bool VerifyPacket( NetPacket& packet );

public:
	static NetSession* GetInstance();
	static void ShutdownInstance();

private:
	UDPSocket* m_socket = nullptr;

	std::vector< NetConnection* > m_allConnections;	// Linked list
	NetConnection* m_boundConnections[ SESSION_MAX_CONNECTIONS ] = {};

	NetConnection* m_myConnection = nullptr;	// Convenience pointer
	NetConnection* m_hostConnection = nullptr;	// Convenience pointer

	SessionState m_state = SESSION_DISCONNECTED;

	SessionError m_error = SESSION_OK;
	std::string m_errorString = "";

	float m_sendIntervalMS = 20.0f;

	/* NET TIME */
	Clock* m_netClock = nullptr;
	float m_lastReceivedHostTimeMS = 0.0f;
	float m_desiredClientTimeMS = 0.0f;
	float m_currentClientTimeMS = 0.0f;

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
	NETMSG_JOIN_REQUEST,
	NETMSG_JOIN_DENY,
	NETMSG_JOIN_ACCEPT,
	NETMSG_JOIN_FINISHED,
	NETMSG_NEW_CONNECTION,
	NETMSG_UPDATE_CONNECTION_STATE,
	NETMSG_HANGUP,
	NETMSG_CORE_COUNT

};
