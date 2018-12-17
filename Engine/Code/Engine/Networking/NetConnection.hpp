#pragma once

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Stopwatch.hpp"
#include "Engine/Networking/NetAddress.hpp"
#include "Engine/Networking/NetMessage.hpp"
#include "Engine/Networking/NetPacket.hpp"

class NetSession;

constexpr uint8_t INVALID_CONNECTION_INDEX = 255U;
constexpr uint16_t MAX_TRACKED_PACKETS = 128U;
constexpr uint16_t RELIABLE_WINDOW = 32U;
constexpr size_t CONNECTION_MAX_ID_LENGTH = 16U;
constexpr float CONNECTION_DEFAULT_TIMEOUT_MS = 10000.0f;

constexpr float UNCONFIRMED_RELIABLE_RESEND_INTERVAL_MS = 100.0f;

enum ConnectionState : uint8_t
{
	CONNECTION_CONNECTED,
	CONNECTION_DISCONNECTED,
	CONNECTION_JOINING,
	CONNECTION_READY,
	NUM_CONNECTION_STATES
};

struct TrackedPacket
{

public:
	void Invalidate()
	{
		m_ack = PACKET_ACK_INVALID;
		m_sendTimeMS = 0.0f;

		for ( uint8_t reliableIDIndex = 0U; reliableIDIndex < m_numReliableIDs; reliableIDIndex++ )
		{
			m_sentReliableIDs[ reliableIDIndex ] = 0U;
		}
		m_numReliableIDs = 0U;
	}
	void AddReliableID( uint16_t reliableID )
	{
		m_sentReliableIDs[ m_numReliableIDs ] = reliableID;
		m_numReliableIDs++;
	}
	bool CanAddReliable() const
	{
		return ( m_numReliableIDs < PACKET_MAX_RELIABLES );
	}
	void operator=( const TrackedPacket& assign )
	{
		m_sendTimeMS = assign.m_sendTimeMS;
		m_ack = assign.m_ack;
		m_numReliableIDs = assign.m_numReliableIDs;

		for ( uint8_t reliableIDIndex = 0U; reliableIDIndex < m_numReliableIDs; reliableIDIndex++ )
		{
			m_sentReliableIDs[ reliableIDIndex ] = assign.m_sentReliableIDs[ reliableIDIndex ];
		}
	}

public:
	float m_sendTimeMS = 0.0f;
	uint16_t m_ack = PACKET_ACK_INVALID;
	uint16_t m_sentReliableIDs[ PACKET_MAX_RELIABLES ] = {};
	uint8_t m_numReliableIDs = 0U;

};

struct NetConnectionInfo
{

public:
	size_t SetID( const char* id )
	{
		size_t sizeCopied = Min( strlen( id ), CONNECTION_MAX_ID_LENGTH - 1U );
		memcpy( m_id, id, sizeCopied );
		m_id[ sizeCopied ] = '\0';
		return sizeCopied;
	}

public:
	NetAddressIPv4 m_address;
	char m_id[ CONNECTION_MAX_ID_LENGTH ] = {};
	uint8_t m_connectionIndex = 0U;

};

class NetConnection
{

	friend class NetSession;

public:
	NetConnection( const NetConnectionInfo& info, NetSession* session );
	NetConnection( const NetAddressIPv4& addr, NetSession* session, unsigned int connectionIndex );
	NetConnection( const NetConnection& copy );
	~NetConnection();

	void ProcessOutgoing();
	void Disconnect();

	void Send( const NetMessage& message );
	bool SendImmediate( const NetMessage& message );

	void OnReceivePacket( NetPacket* packet );
	void OnReceiveMessage( const NetMessage& message );

	size_t SetID( const char* newID );
	void SetState( ConnectionState newState );
	void SetSendRate( float sendRateHz );
	void SetHeartRate( float heartRateHz );

	float GetSendIntervalMilliseconds() const;
	float GetSendRate() const;
	float GetTimeSinceLastSendMS() const;
	float GetTimeSinceLastReceiveMS() const;
	float GetRTT() const;
	float GetLoss() const;
	uint16_t GetNextAck() const;
	uint16_t GetLastReceivedAck() const;
	uint16_t GetLastConfirmedAck() const;
	std::string GetPreviousAcks() const;	// Returns uint16_t as bitfield string
	size_t GetNumUnconfirmedReliables() const;
	size_t GetNumUnsentReliables() const;
	size_t GetNumUnsentUnreliables() const;
	NetConnectionInfo GetInfo() const;

	bool IsReliableConfirmed( uint16_t reliableID ) const;
	bool ShouldProcessMessage( const NetMessage& message ) const;

	bool IsMe() const;
	bool IsHost() const;
	bool IsClient() const;

	inline bool IsConnected() const		{ return m_state == CONNECTION_CONNECTED; }
	inline bool IsDisconnected() const	{ return m_state == CONNECTION_DISCONNECTED; }
	inline bool IsJoining() const		{ return m_state == CONNECTION_JOINING; }
	inline bool IsReady() const			{ return m_state == CONNECTION_READY; }

private:
	void UpdateTrackedTime();
	void UpdateHeartbeat();
	void UpdateLoss();

	void CheckAndHandleTimeout();

	void ProcessMessage( NetMessage& message );

	bool CanSendUnsentReliableMessage() const;
	bool CanFlushMessageFromQueue( NetMessage* message, TrackedPacket* trackedPacket, const std::vector< NetMessage* >& sentFromQueue ) const;
	void HandleMessageBeforeFlush( NetMessage* message, TrackedPacket* trackedPacket, const std::vector< NetMessage* >& sentFromQueue );
	void HandleFlushedMessage( NetMessage* message );
	size_t FlushMessagesFromQueue( NetPacket* packet, TrackedPacket* trackedPacket, std::vector< NetMessage* >& msgQueue );

	void HangUp();

	void ConfirmReceiveReliable( uint16_t reliableID );
	void ConfirmReceivePacket( uint16_t ack );
	void ConfirmLossPacket( uint16_t ack );
	TrackedPacket* AddTrackedPacket( uint16_t ack );
	TrackedPacket PeekTrackedPacket( uint16_t ack );
	TrackedPacket RemoveTrackedPacket( uint16_t ack );

public:
	NetAddressIPv4 m_address;
	NetSession* m_session = nullptr;
	unsigned int m_connectionIndex = INVALID_CONNECTION_INDEX;

private:

	ConnectionState m_state = CONNECTION_DISCONNECTED;
	char m_id[ CONNECTION_MAX_ID_LENGTH ] = {};

	// Connection properties
	float m_sendIntervalMS = 0.0f;
	StopWatch m_sendStopwatch;
	float m_heartbeatIntervalMS = 1000.0f;	// Arbitrary non-zero default value
	StopWatch m_heartbeatStopwatch;

	// Packet tracking
	uint16_t m_nextAckToSend = 0U;						// Ours
	TrackedPacket m_trackedPackets[ MAX_TRACKED_PACKETS ];
	uint16_t m_lastReceivedAck = PACKET_ACK_INVALID;	// Theirs
	uint16_t m_previousAcksBitfield = 0U;

	// Message tracking
	uint16_t m_nextReliableIDToSend = 0U;	// Ours
	std::vector< uint16_t > m_confirmedReliableIDs;	// Theirs
	uint16_t m_lastReceivedReliableID = 65530U;

	std::vector< NetMessage* > m_unsentUnreliables; 
	std::vector< NetMessage* > m_unsentReliables;
	std::vector< NetMessage* > m_unconfirmedReliables;

	NetMessageChannel m_channels[ NETWORKING_MAX_MESSAGE_CHANNELS ];

	// Statistics
	float m_msSinceLastSend = 0.0f;
	float m_msSinceLastReceive = 0.0f;
	float m_rttMS = 0.0f;
	float m_numPacketsTracked = 0.0f;
	float m_numPacketsLostInWindow = 0.0f;
	float m_loss = 0.0f;
	uint16_t m_lastConfirmedAck = 0U;

};
