#include "Engine/Core/Clock.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Networking/NetConnection.hpp"
#include "Engine/Networking/NetMessage.hpp"
#include "Engine/Networking/NetSession.hpp"
#include "Engine/Networking/UDPSocket.hpp"
#include "Engine/Tools/DevConsole.hpp"
#include "Game/EngineBuildPreferences.hpp"

#if !defined( ENGINE_DISABLE_NETWORKING )

NetConnection::NetConnection( const NetAddressIPv4& addr, NetSession* session, unsigned int connectionIndex )
	:	m_address( addr ),
		m_session( session ),
		m_connectionIndex( connectionIndex )
{
	m_sendStopwatch.SetTimer( m_sendIntervalMS * 0.001f );
	m_heartbeatStopwatch.SetTimer( m_heartbeatIntervalMS * 0.001f );
}

NetConnection::NetConnection( const NetConnection& copy )
	:	m_session( copy.m_session ),
		m_address( copy.m_address ),
		m_connectionIndex( copy.m_connectionIndex )
{
	m_sendStopwatch.SetTimer( m_sendIntervalMS * 0.001f );
	m_heartbeatStopwatch.SetTimer( m_heartbeatIntervalMS * 0.001f );
}

void NetConnection::SetSendRate( float sendRateHz )
{
	sendRateHz = Max( sendRateHz, 0.01f );
	m_sendIntervalMS = 1000.0f / sendRateHz;

	float longerInterval = Max( m_sendIntervalMS, m_session->GetSendIntervalMilliseconds() );
	m_sendStopwatch.SetTimer( longerInterval * 0.001f );
}

void NetConnection::SetHeartRate( float heartRateHz )
{
	heartRateHz = Max( heartRateHz, 0.01f );
	m_heartbeatIntervalMS = 1000.0f / heartRateHz;

	m_heartbeatStopwatch.SetTimer( m_heartbeatIntervalMS * 0.001f );
}

float NetConnection::GetSendIntervalMilliseconds() const
{
	return m_sendIntervalMS;
}

float NetConnection::GetSendRate() const
{
	return ( 1000.0f / m_sendIntervalMS );
}

float NetConnection::GetTimeSinceLastSendMS() const
{
	return m_msSinceLastSend;
}

float NetConnection::GetTimeSinceLastReceiveMS() const
{
	return m_msSinceLastReceive;
}

float NetConnection::GetRTT() const
{
	return m_rttMS;
}

float NetConnection::GetLoss() const
{
	return m_loss;
}

uint16_t NetConnection::GetNextAck() const
{
	return m_nextAckToSend;
}

uint16_t NetConnection::GetLastReceivedAck() const
{
	return m_lastReceivedAck;
}

std::string NetConnection::GetPreviousAcks() const
{
	std::string wordString = "";
	for ( uint16_t flagPow = 0U; flagPow < 16U; flagPow++ )
	{
		uint16_t flag = 1U << flagPow;
		std::string bitString = ( m_previousAcksBitfield & flag )? "1" : "0";
		wordString = bitString + wordString;
	}
	return wordString;
}

void NetConnection::Send( const NetMessage& message )
{
	NetMessage* newMessage = new NetMessage( message );
	// LogPrintf( "Message queued: %s", message.m_buffer );
	m_outboundMessages.push_back( newMessage );
}

bool NetConnection::SendImmediate( const NetMessage& message )
{
	NetPacket* packet = new NetPacket( static_cast< uint8_t >( m_connectionIndex ), false );
	NetPacketHeader header;
	header.m_senderConnectionIndex = m_connectionIndex;
	header.m_ack = m_nextAckToSend;
	header.m_lastReceivedAck = m_lastReceivedAck;
	header.m_previousAcksBitfield = m_previousAcksBitfield;
	packet->WriteHeader( header );

	bool success = packet->WriteMessage( message );
	if ( success )
	{
		success = success && ( m_session->GetSocket()->SendTo( m_address, packet->m_buffer, packet->GetWrittenByteCount() ) > 0U );
	}
	
	delete packet;

	m_nextAckToSend++;
	if ( m_nextAckToSend == PACKET_ACK_INVALID )
	{
		m_nextAckToSend = 0U;
	}

	m_msSinceLastSend = 0.0f;

	return success;
}

void NetConnection::ProcessOutgoing()
{
	UpdateTrackedTime();
	UpdateHeartbeat();

	if ( m_sendStopwatch.CheckAndReset( m_sendIntervalMS * 0.001f ) )
	{
		NetPacket* packet = new NetPacket( static_cast< uint8_t >( m_connectionIndex ), false );
		NetPacketHeader header;
		header.m_senderConnectionIndex = m_connectionIndex;
		header.m_ack = m_nextAckToSend;
		header.m_lastReceivedAck = m_lastReceivedAck;
		header.m_previousAcksBitfield = m_previousAcksBitfield;
		packet->WriteHeader( header );

		/* TrackedPacket* trackedPacket = */ AddTrackedPacket( m_nextAckToSend );
		// LogPrintf( "Tracking packet: %u", m_nextAckToSend );

		for ( size_t messageIndex = 0U; messageIndex < m_outboundMessages.size(); messageIndex++ )
		{
			NetMessage* message = m_outboundMessages[ messageIndex ];
			bool success = packet->WriteMessage( *message );
			if ( success )
			{
				delete message;
				m_outboundMessages.erase( m_outboundMessages.begin() + messageIndex );
				messageIndex--;
			}
			else
			{
				break;
			}
		}

		m_session->GetSocket()->SendTo( m_address, packet->m_buffer, packet->GetWrittenByteCount() );
		// LogPrintf( "Sent packet: %u to %s", m_nextAckToSend, m_address.GetAddressAsString().c_str() );
		delete packet;

		m_nextAckToSend++;
		if ( m_nextAckToSend == PACKET_ACK_INVALID )
		{
			m_nextAckToSend = 0U;
		}

		m_msSinceLastSend = 0.0f;
	}
}

void NetConnection::OnReceivePacket( NetPacket* packet )
{
	m_msSinceLastReceive = 0.0f;

	NetPacketHeader packetHeader;
	if ( packet->ReadHeader( &packetHeader ) )
	{
		// Update received packet information with their ACK number
		if ( packetHeader.m_ack != PACKET_ACK_INVALID )
		{
			uint16_t difference = packetHeader.m_ack - m_lastReceivedAck;

			if ( difference == 0U )
			{
				ConsolePrintf( Rgba::RED, "NetConnection::OnReceivePacket(): Duplicate packet with ack number %u received. Aborting...", packetHeader.m_ack );
				return;
			}

			if ( difference < ( 0xffff / 2U ) )		// Got a higher ACK than the current m_lastReceivedAck
			{
				m_lastReceivedAck = packetHeader.m_ack;

				m_previousAcksBitfield <<= difference;
				if ( difference <= 16U )	// Since we only have 16 bits of history
				{
					uint16_t oldHighestAckBit = 1U << ( difference - 1U );
					m_previousAcksBitfield |= oldHighestAckBit;
				}
			}
			else	// Out of order
			{
				difference = m_lastReceivedAck - packetHeader.m_ack;
				if ( difference <= 16U )
				{
					uint16_t newAckBit = 1U << ( difference - 1U );
					if ( m_previousAcksBitfield & newAckBit )
					{
						ConsolePrintf( Rgba::RED, "NetConnection::OnReceivePacket(): Duplicate ack for %u received through bitfield. %u was highest received ack. Aborting...", packetHeader.m_ack, m_lastReceivedAck );
					}
					else
					{
						m_previousAcksBitfield |= newAckBit;
					}
				}
			}
		}

		// Update our trackers with their received packet information
		if ( packetHeader.m_lastReceivedAck != PACKET_ACK_INVALID )
		{
			ConfirmReceivePacket( packetHeader.m_lastReceivedAck );
			for ( uint16_t flagPow = 0U; flagPow < 16U; flagPow++ )
			{
				uint16_t flag = 1 << flagPow;
				if ( packetHeader.m_previousAcksBitfield & flag )
				{
					uint16_t previousAck = packetHeader.m_lastReceivedAck - ( flagPow + 1U );
					if ( previousAck == PACKET_ACK_INVALID )
					{
						previousAck--;
					}
					ConfirmReceivePacket( previousAck );
				}
			}
		}
	}
}

void NetConnection::ConfirmReceivePacket( uint16_t ack )
{
	TrackedPacket packet = PeekTrackedPacket( ack );
	if ( packet.m_ack == PACKET_ACK_INVALID || packet.m_ack != ack )
	{
		// Tracked packet was dropped earlier
		return;
	}

	RemoveTrackedPacket( ack );
	// LogPrintf( "NetConnection::ConfirmReceivePacket(): Connection received packet: %u", ack );

	float currentMilliseconds = GetCurrentTimeMillisecondsF();
	m_rttMS = currentMilliseconds - packet.m_sendTimeMS;	// TODO: Lerp toward this instead of hard-setting it
}

void NetConnection::ConfirmLossPacket( uint16_t ack )
{
	TrackedPacket packet = PeekTrackedPacket( ack );
	if ( packet.m_ack == PACKET_ACK_INVALID || packet.m_ack != ack )
	{
		return;
	}
	else
	{
		RemoveTrackedPacket( ack );
		// LogPrintf( "NetConnection::ConfirmLossPacket(): Connection lost packet: %u", ack );
		m_numPacketsLostInWindow += 1.0f;
	}
}

TrackedPacket* NetConnection::AddTrackedPacket( uint16_t ack )
{
	m_numPacketsTracked += 1.0f;

	uint16_t trackedIndex = ack % MAX_TRACKED_PACKETS;
	if ( trackedIndex == ( MAX_TRACKED_PACKETS - 1 ) )
	{
		UpdateLoss();
	}
	if ( m_trackedPackets[ trackedIndex ].m_ack != PACKET_ACK_INVALID )
	{
		ConfirmLossPacket( m_trackedPackets[ trackedIndex ].m_ack );
	}

	m_trackedPackets[ trackedIndex ].m_ack = ack;
	m_trackedPackets[ trackedIndex ].m_sendTimeMS = GetCurrentTimeMillisecondsF();
	return &m_trackedPackets[ trackedIndex ];
}

TrackedPacket NetConnection::PeekTrackedPacket( uint16_t ack )
{
	TrackedPacket packet;
	uint16_t trackedIndex = ack % MAX_TRACKED_PACKETS;

	TrackedPacket trackedPacket = m_trackedPackets[ trackedIndex ];
	if ( trackedPacket.m_ack == ack )
	{
		packet.m_ack = trackedPacket.m_ack;
		packet.m_sendTimeMS = trackedPacket.m_sendTimeMS;
	}

	return packet;
}

TrackedPacket NetConnection::RemoveTrackedPacket( uint16_t ack )
{
	TrackedPacket packet;
	uint16_t trackedIndex = ack % MAX_TRACKED_PACKETS;

	TrackedPacket trackedPacket = m_trackedPackets[ trackedIndex ];
	if ( trackedPacket.m_ack == ack )
	{
		packet.m_ack = trackedPacket.m_ack;
		packet.m_sendTimeMS = trackedPacket.m_sendTimeMS;
		m_trackedPackets[ trackedIndex ].Invalidate();
	}

	return packet;
}

void NetConnection::UpdateTrackedTime()
{
	float deltaMS = GetMasterDeltaSecondsF() * 1000.0f;
	m_msSinceLastSend += deltaMS;
	m_msSinceLastReceive += deltaMS;
}

void NetConnection::UpdateHeartbeat()
{
	if ( m_heartbeatStopwatch.CheckAndReset( m_heartbeatIntervalMS * 0.001f ) )
	{
		uint8_t messageIndex = m_session->GetMessageIndex( "heartbeat" );
		NetMessage msg( messageIndex );
		Send( msg );
	}
}

void NetConnection::UpdateLoss()
{
	m_loss = m_numPacketsLostInWindow / static_cast< float >( MAX_TRACKED_PACKETS );
	m_numPacketsLostInWindow = 0.0f;
}

#endif	// #if !defined( ENGINE_DISABLE_NETWORKING )
