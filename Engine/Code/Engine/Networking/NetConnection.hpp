#pragma once

#include "Engine/Core/Stopwatch.hpp"
#include "Engine/Networking/NetAddress.hpp"
#include "Engine/Networking/NetPacket.hpp"

class NetMessage;
class NetSession;

#define MAX_TRACKED_PACKETS 64U

struct TrackedPacket
{

public:
	void Invalidate()
	{
		m_ack = PACKET_ACK_INVALID;
		m_sendTimeMS = 0.0f;
	}

public:
	uint16_t m_ack = PACKET_ACK_INVALID;
	float m_sendTimeMS = 0.0f;

};

class NetConnection
{

public:
	NetConnection( const NetAddressIPv4& addr, NetSession* session, unsigned int connectionIndex );
	NetConnection( const NetConnection& copy );

	void SetSendRate( float sendRateHz );
	void SetHeartRate( float heartRateHz );

	void Send( const NetMessage& message );
	bool SendImmediate( const NetMessage& message );
	void ProcessOutgoing();
	void OnReceivePacket( NetPacket* packet );

	float GetSendIntervalMilliseconds() const;
	float GetSendRate() const;
	float GetTimeSinceLastSendMS() const;
	float GetTimeSinceLastReceiveMS() const;
	float GetRTT() const;
	float GetLoss() const;
	uint16_t GetNextAck() const;
	uint16_t GetLastReceivedAck() const;
	std::string GetPreviousAcks() const;	// Returns uint16_t as bitfield string

private:
	void UpdateTrackedTime();
	void UpdateHeartbeat();
	void UpdateLoss();

	void ConfirmReceivePacket( uint16_t ack );
	void ConfirmLossPacket( uint16_t ack );
	TrackedPacket* AddTrackedPacket( uint16_t ack );
	TrackedPacket PeekTrackedPacket( uint16_t ack );
	TrackedPacket RemoveTrackedPacket( uint16_t ack );

public:
	NetAddressIPv4 m_address;
	NetSession* m_session = nullptr;
	std::vector< NetMessage* > m_outboundMessages; 
	unsigned int m_connectionIndex = 0U;

private:
	// My ACK counter
	uint16_t m_nextAckToSend = 0U;
	TrackedPacket m_trackedPackets[ MAX_TRACKED_PACKETS ];

	// Their ACK counter
	uint16_t m_lastReceivedAck = PACKET_ACK_INVALID;
	uint16_t m_previousAcksBitfield = 0U;

	float m_msSinceLastSend = 0.0f;
	float m_msSinceLastReceive = 0.0f;
	float m_rttMS = 0.0f;
	float m_numPacketsTracked = 0.0f;

	float m_numPacketsLostInWindow = 0.0f;
	float m_loss = 0.0f;

	float m_sendIntervalMS = 100.0f;
	StopWatch m_sendStopwatch;
	float m_heartbeatIntervalMS = 1000.0f;	// Arbitrary non-zero default value
	StopWatch m_heartbeatStopwatch;

};
