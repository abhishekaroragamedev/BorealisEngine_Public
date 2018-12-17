#include "Engine/Core/Clock.hpp"
#include "Engine/Networking/NetConnection.hpp"
#include "Engine/Networking/NetSession.hpp"
#include "Engine/Networking/UDPSocket.hpp"
#include "Engine/Tools/DevConsole.hpp"
#include "Game/EngineBuildPreferences.hpp"

#if !defined( ENGINE_DISABLE_NETWORKING )

NetConnection::NetConnection( const NetConnectionInfo& info, NetSession* session )
	:	m_address( info.m_address ),
		m_session( session ),
		m_connectionIndex( info.m_connectionIndex )
{
	size_t idLength = Min( strlen( info.m_id ), CONNECTION_MAX_ID_LENGTH - 1U );
	memcpy( m_id, info.m_id, idLength );
	m_id[ idLength ] = '\0'; 

	m_sendIntervalMS = Max( m_sendIntervalMS, m_session->GetSendIntervalMilliseconds() );
	m_sendStopwatch.SetClock( m_session->GetNetClock() );
	m_sendStopwatch.SetTimer( m_sendIntervalMS * 0.001f );
	m_heartbeatStopwatch.SetClock( m_session->GetNetClock() );
	m_heartbeatStopwatch.SetTimer( m_heartbeatIntervalMS * 0.001f );
}

NetConnection::NetConnection( const NetAddressIPv4& addr, NetSession* session, unsigned int connectionIndex )
	:	m_address( addr ),
		m_session( session ),
		m_connectionIndex( connectionIndex )
{
	m_sendIntervalMS = Max( m_sendIntervalMS, m_session->GetSendIntervalMilliseconds() );
	m_sendStopwatch.SetClock( m_session->GetNetClock() );
	m_sendStopwatch.SetTimer( m_sendIntervalMS * 0.001f );
	m_heartbeatStopwatch.SetClock( m_session->GetNetClock() );
	m_heartbeatStopwatch.SetTimer( m_heartbeatIntervalMS * 0.001f );
}

NetConnection::NetConnection( const NetConnection& copy )
	:	m_session( copy.m_session ),
		m_address( copy.m_address ),
		m_connectionIndex( copy.m_connectionIndex )
{
	m_sendIntervalMS = Max( m_sendIntervalMS, m_session->GetSendIntervalMilliseconds() );
	m_sendStopwatch.SetClock( m_session->GetNetClock() );
	m_sendStopwatch.SetTimer( m_sendIntervalMS * 0.001f );
	m_heartbeatStopwatch.SetClock( m_session->GetNetClock() );
	m_heartbeatStopwatch.SetTimer( m_heartbeatIntervalMS * 0.001f );
}

NetConnection::~NetConnection()
{
	for ( NetMessage* message : m_unconfirmedReliables )
	{
		delete message;
	}

	for ( NetMessage* message : m_unsentReliables )
	{
		delete message;
	}

	for ( NetMessage* message : m_unsentUnreliables )
	{
		delete message;
	}

	for ( unsigned int channel = 0U; channel < NETWORKING_MAX_MESSAGE_CHANNELS; channel++ )
	{
		for ( NetMessage* message : m_channels[ channel ].m_outOfOrderMessages )
		{
			delete message;
		}
	}
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

uint16_t NetConnection::GetLastConfirmedAck() const
{
	return m_lastConfirmedAck;
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

size_t NetConnection::GetNumUnconfirmedReliables() const
{
	return m_unconfirmedReliables.size();
}

size_t NetConnection::GetNumUnsentReliables() const
{
	return m_unsentReliables.size();
}

size_t NetConnection::GetNumUnsentUnreliables() const
{
	return m_unsentUnreliables.size();
}

NetConnectionInfo NetConnection::GetInfo() const
{
	NetConnectionInfo info;
	info.m_address = m_address;
	info.m_connectionIndex = m_connectionIndex;
	info.SetID( m_id );
	return info;
}

bool NetConnection::IsMe() const
{
	return ( this == m_session->GetMyConnection() );
}

bool NetConnection::IsHost() const
{
	return ( m_state == CONNECTION_READY && m_connectionIndex == 0U );
}

bool NetConnection::IsClient() const
{
	return ( m_state == CONNECTION_CONNECTED && m_connectionIndex > 0U );
}

bool NetConnection::IsReliableConfirmed( uint16_t reliableID ) const
{
	bool found = false;

	for ( uint16_t confirmedReliableID : m_confirmedReliableIDs )
	{
		found = reliableID == confirmedReliableID;
		if ( found )
		{
			break;
		}
	}

	return found;
}

bool NetConnection::ShouldProcessMessage( const NetMessage& message ) const
{
	// If a reliable message has been confirmed as received, do not process it
	uint16_t forwardDifference = message.m_reliableID - m_lastReceivedReliableID;
	uint16_t reverseDifference = m_lastReceivedReliableID - message.m_reliableID;
	bool isUnreliableOrWithinWindow = !message.IsReliable() ||
			( forwardDifference <= RELIABLE_WINDOW ) ||
			( reverseDifference <= RELIABLE_WINDOW );

	bool isUnreliableOrUnconfirmed = !message.IsReliable() || !IsReliableConfirmed( message.m_reliableID );

	return ( isUnreliableOrWithinWindow && isUnreliableOrUnconfirmed );
}

void NetConnection::Send( const NetMessage& message )
{
	NetMessage* newMessage = new NetMessage( message );
	// LogPrintf( "Message queued: %s", message.m_buffer );

	if ( newMessage->IsReliable() )
	{
		m_unsentReliables.push_back( newMessage );
	}
	else
	{
		m_unsentUnreliables.push_back( newMessage );
	}
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

bool NetConnection::CanSendUnsentReliableMessage() const
{
	uint16_t oldestUnconfirmedReliableID = m_nextReliableIDToSend;
	uint16_t largestGap = 0U;
	for ( size_t reliableIndex = 0U; reliableIndex < m_unconfirmedReliables.size(); reliableIndex++ )
	{
		uint16_t gap = m_nextReliableIDToSend - m_unconfirmedReliables[ reliableIndex ]->m_reliableID;
		if ( gap > largestGap )
		{
			largestGap = gap;
			oldestUnconfirmedReliableID = m_unconfirmedReliables[ reliableIndex ]->m_reliableID;
		}
		if ( gap > RELIABLE_WINDOW )
		{
			break;
		}
	}

	return ( ( m_nextReliableIDToSend - oldestUnconfirmedReliableID ) <= RELIABLE_WINDOW );
}

bool NetConnection::CanFlushMessageFromQueue( NetMessage* message, TrackedPacket* trackedPacket, const std::vector<NetMessage *>& sentFromQueue ) const
{
	float currentTimeMS = m_session->GetNetTimeMS();

	bool canAddReliableMessage = !message->IsReliable() || trackedPacket->CanAddReliable();
	bool canSendUnconfirmedReliableMessage = ( sentFromQueue != m_unconfirmedReliables ) || ( currentTimeMS - message->m_lastSentTimeMS > UNCONFIRMED_RELIABLE_RESEND_INTERVAL_MS );
	bool canSendUnsentReliableMessage = ( sentFromQueue != m_unsentReliables ) || CanSendUnsentReliableMessage();

	return ( canAddReliableMessage && canSendUnconfirmedReliableMessage && canSendUnsentReliableMessage );
}

void NetConnection::HandleMessageBeforeFlush( NetMessage* message, TrackedPacket* trackedPacket, const std::vector<NetMessage *>& sentFromQueue )
{
	if ( message->IsReliable() )
	{
		uint16_t reliableID = message->GetReliableID();
		if ( sentFromQueue == m_unsentReliables )
		{
			reliableID = m_nextReliableIDToSend;
			message->SetReliableID( reliableID );
			m_nextReliableIDToSend++;

			if ( message->IsInOrder() )
			{
				unsigned int channel = message->m_definition->m_channelIndex;
				message->SetSequenceID( m_channels[ channel ].m_nextSequenceIDToSend );
				m_channels[ channel ].m_nextSequenceIDToSend++;
			}
		}

		trackedPacket->AddReliableID( reliableID );	// trackedPacket should not be null for a reliable packet from either queue
	}
}

void NetConnection::HandleFlushedMessage( NetMessage* message )
{
	if ( message->IsReliable() )
	{
		message->m_lastSentTimeMS = m_session->GetNetTimeMS();
		m_unconfirmedReliables.push_back( message );
	}
	else
	{
		delete message;
	}
}

size_t NetConnection::FlushMessagesFromQueue( NetPacket* packet, TrackedPacket* trackedPacket, std::vector< NetMessage* >& msgQueue )
{
	std::vector< NetMessage* > flushedMessages;

	size_t messageIndex;
	for ( messageIndex = 0U; messageIndex < msgQueue.size(); messageIndex++ )
	{
		NetMessage* message = msgQueue[ messageIndex ];
		if ( CanFlushMessageFromQueue( message, trackedPacket, msgQueue ) )
		{
			HandleMessageBeforeFlush( message, trackedPacket, msgQueue );
			bool success = packet->WriteMessage( *message );
			if ( success )
			{
				msgQueue.erase( msgQueue.begin() + messageIndex );
				flushedMessages.push_back( message );
				messageIndex--;
			}
			else
			{
				break;
			}
		}
	}

	for ( NetMessage* flushedMessage : flushedMessages )
	{
		HandleFlushedMessage( flushedMessage );	// Will push back into m_unconfirmedReliables if msgQueue is m_unconfirmedReliables
	}

	return messageIndex;
}

void NetConnection::ProcessOutgoing()
{
	UpdateTrackedTime();
	CheckAndHandleTimeout();

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

		TrackedPacket* trackedPacket = AddTrackedPacket( m_nextAckToSend );

		FlushMessagesFromQueue( packet, trackedPacket, m_unconfirmedReliables );
		FlushMessagesFromQueue( packet, trackedPacket, m_unsentReliables );
		FlushMessagesFromQueue( packet, nullptr, m_unsentUnreliables );

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

void NetConnection::Disconnect()
{
	m_state = CONNECTION_DISCONNECTED;
}

void NetConnection::HangUp()
{
	NetMessage hangup( static_cast< uint8_t >( NETMSG_HANGUP ) );
	Send( hangup );

	NetPacket* packet = new NetPacket( static_cast< uint8_t >( m_connectionIndex ), false );
	NetPacketHeader header;
	header.m_senderConnectionIndex = m_connectionIndex;
	header.m_ack = m_nextAckToSend;
	header.m_lastReceivedAck = m_lastReceivedAck;
	header.m_previousAcksBitfield = m_previousAcksBitfield;
	packet->WriteHeader( header );
	
	// We only care about unreliables when hanging up
	FlushMessagesFromQueue( packet, nullptr, m_unsentUnreliables );
	m_session->GetSocket()->SendTo( m_address, packet->m_buffer, packet->GetWrittenByteCount() );

	delete packet;
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

void NetConnection::OnReceiveMessage( const NetMessage& message )
{
	bool shouldProcess = !message.IsConnectionless();
	bool shouldProcessChannel = false;

	if ( message.IsReliable() )
	{
		m_confirmedReliableIDs.push_back( message.m_reliableID );

		// Update highest received reliable ID and ensure no IDs more than a window apart from it are present in the confirmed list
		uint16_t difference = message.m_reliableID - m_lastReceivedReliableID;
		if ( difference < ( 0xffff / 2U ) )
		{
			m_lastReceivedReliableID = message.m_reliableID;
		}
		for ( size_t reliableIndex = 0U; reliableIndex < m_confirmedReliableIDs.size(); reliableIndex++ )
		{
			uint16_t gap = m_lastReceivedReliableID - m_confirmedReliableIDs[ reliableIndex ];
			if ( gap > RELIABLE_WINDOW )
			{
				m_confirmedReliableIDs.erase( m_confirmedReliableIDs.begin() + reliableIndex );
				reliableIndex--;
			}
		}

		if ( message.IsInOrder() )
		{
			unsigned int channel = message.m_definition->m_channelIndex;
			NetMessageChannel& msgChannel = m_channels[ channel ];

			if ( message.GetSequenceID() == msgChannel.m_nextSequenceIDExpected )
			{
				msgChannel.m_nextSequenceIDExpected++;
				shouldProcessChannel = true;
			}
			else
			{
				NetMessage* messageCopy = new NetMessage( message );

				size_t insertionIndex = 0U;
				for ( size_t msgIndex = 0U; msgIndex < msgChannel.m_outOfOrderMessages.size(); msgIndex++ )
				{
					NetMessage* existingMessage = msgChannel.m_outOfOrderMessages[ msgIndex ];
					if ( messageCopy->GetSequenceID() < existingMessage->GetSequenceID() )
					{
						break;
					}
					insertionIndex++;
				}

				msgChannel.m_outOfOrderMessages.insert( ( msgChannel.m_outOfOrderMessages.begin() + insertionIndex ), messageCopy );
				shouldProcess = false;
			}
		}
	}

	if ( shouldProcess )
	{
		NetMessage messageCopy( message );
		ProcessMessage( messageCopy );
	}
	if ( message.IsInOrder() && shouldProcessChannel )
	{
		unsigned int channel = message.m_definition->m_channelIndex;
		NetMessageChannel& msgChannel = m_channels[ channel ];

		for ( size_t msgIndex = 0U; msgIndex < msgChannel.m_outOfOrderMessages.size(); msgIndex++ )
		{
			NetMessage* oldMessage = msgChannel.m_outOfOrderMessages[ msgIndex ];
			if ( oldMessage->GetSequenceID() == msgChannel.m_nextSequenceIDExpected )
			{
				ProcessMessage( *oldMessage );
				delete oldMessage;
				msgChannel.m_nextSequenceIDExpected++;

				msgChannel.m_outOfOrderMessages.erase( msgChannel.m_outOfOrderMessages.begin() + msgIndex );
				msgIndex--;
			}
		}
	}
}

void NetConnection::ProcessMessage( NetMessage& message )
{
	NetMessageDefinition* definition = message.m_definition;
	definition->m_callback( message, *this );
}

void NetConnection::ConfirmReceiveReliable( uint16_t reliableID )
{
	for ( size_t reliableIndex = 0U; reliableIndex < m_unconfirmedReliables.size(); reliableIndex++ )
	{
		if ( m_unconfirmedReliables[ reliableIndex ]->m_reliableID == reliableID )
		{
			delete m_unconfirmedReliables[ reliableIndex ];
			m_unconfirmedReliables.erase( m_unconfirmedReliables.begin() + reliableIndex );
			return;
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

	for ( uint8_t reliableIDIndex = 0U; reliableIDIndex < packet.m_numReliableIDs; reliableIDIndex++ )
	{
		ConfirmReceiveReliable( packet.m_sentReliableIDs[ reliableIDIndex ] );
	}

	RemoveTrackedPacket( ack );
	m_lastConfirmedAck = ack;
	// LogPrintf( "NetConnection::ConfirmReceivePacket(): Connection received packet: %u", ack );

	float currentMilliseconds = m_session->GetNetTimeMS();
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
	m_trackedPackets[ trackedIndex ].m_sendTimeMS = m_session->GetNetTimeMS();
	return &m_trackedPackets[ trackedIndex ];
}

TrackedPacket NetConnection::PeekTrackedPacket( uint16_t ack )
{
	TrackedPacket packet;
	uint16_t trackedIndex = ack % MAX_TRACKED_PACKETS;

	TrackedPacket trackedPacket = m_trackedPackets[ trackedIndex ];
	if ( trackedPacket.m_ack == ack )
	{
		packet = trackedPacket;
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
		packet = trackedPacket;
		m_trackedPackets[ trackedIndex ].Invalidate();
	}

	return packet;
}

void NetConnection::CheckAndHandleTimeout()
{
	if ( this != m_session->GetMyConnection() && m_msSinceLastReceive > CONNECTION_DEFAULT_TIMEOUT_MS )
	{
		LogErrorf( "Connection %u timed out. Deleting connection.", m_connectionIndex );
		Disconnect();
	}
}

void NetConnection::UpdateTrackedTime()
{
	float deltaMS = m_session->GetNetClock()->GetDeltaSecondsF() * 1000.0f;
	m_msSinceLastSend += deltaMS;
	m_msSinceLastReceive += deltaMS;
}

void NetConnection::UpdateHeartbeat()
{
	if ( m_heartbeatStopwatch.CheckAndReset( m_heartbeatIntervalMS * 0.001f ) )
	{
		uint8_t messageIndex = m_session->GetMessageIndex( "heartbeat" );
		NetMessage heartbeat( messageIndex );
		if ( m_session->IsHost() && !IsMe() )
		{
			float sentTime = m_session->GetNetTimeMS();
			heartbeat.WriteFloat( sentTime );
		}
		Send( heartbeat );
	}
}

void NetConnection::UpdateLoss()
{
	m_loss = m_numPacketsLostInWindow / static_cast< float >( MAX_TRACKED_PACKETS );
	m_numPacketsLostInWindow = 0.0f;
}

size_t NetConnection::SetID( const char* newID )
{
	size_t sizeCopied = Min( strlen( newID ), CONNECTION_MAX_ID_LENGTH - 1U );
	memcpy( m_id, newID, sizeCopied );
	m_id[ sizeCopied ] = '\0';
	return sizeCopied;
}

void NetConnection::SetState( ConnectionState newState )
{
	m_state = newState;
	if ( m_state == CONNECTION_CONNECTED && this == m_session->GetMyConnection() )
	{
		NetMessage updateConnState( static_cast< uint8_t >( NETMSG_UPDATE_CONNECTION_STATE ) );
		updateConnState.WriteBytes( 1U, &m_state );
		m_session->BroadcastMessage( updateConnState );
	}
}

#endif	// #if !defined( ENGINE_DISABLE_NETWORKING )
