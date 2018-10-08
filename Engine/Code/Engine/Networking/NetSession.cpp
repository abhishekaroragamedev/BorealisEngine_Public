#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Networking/NetPacket.hpp"
#include "Engine/Networking/NetSession.hpp"
#include "Engine/Networking/UDPSocket.hpp"
#include "Engine/Tools/DevConsole.hpp"

/*
	NET SESSION COMMANDS
*/
bool NetSimLossCommand( Command& command )
{
	if ( command.GetName() == "net_sim_loss" )
	{
		std::string amountStr = command.GetNextString();
		if ( amountStr == "" )
		{
			ConsolePrintf( Rgba::RED, "ERROR: net_sim_loss: No loss amount entered." );
			return false;
		}

		float amount = 0.0f;
		try
		{
			amount = stof( amountStr );
		}
		catch ( std::invalid_argument& args )
		{
			UNUSED( args );
			ConsolePrintf( Rgba::RED, "ERROR: net_sim_loss: Invalid loss amount entered. Enter a number between 0.0 and 1.0." );
			return false;
		}

		if ( amount < 0.0f || amount > 1.0f )
		{
			ConsolePrintf( Rgba::YELLOW, "WARNING: net_sim_loss: Clamping amount to [0.0, 1.0]." );
		}

		NetSession::GetInstance()->SetSimLoss( amount );
		return true;
	}

	return false;
}

bool NetSimLagCommand( Command& command )
{
	if ( command.GetName() == "net_sim_lag" )
	{
		std::string minMsString = command.GetNextString();
		if ( minMsString == "" )
		{
			ConsolePrintf( Rgba::RED, "ERROR: net_sim_lag: No min amount entered." );
			return false;
		}

		std::string maxMsString = command.GetNextString();
		if ( maxMsString == "" )
		{
			ConsolePrintf( Rgba::RED, "ERROR: net_sim_lag: No max amount entered." );
			return false;
		}

		float min = 0.0f;
		try
		{
			min = stof( minMsString );
		}
		catch ( std::invalid_argument& args )
		{
			UNUSED( args );
			ConsolePrintf( Rgba::RED, "ERROR: net_sim_lag: Invalid min lag entered. Enter a positive floating number." );
			return false;
		}
		if ( min < 0.0f )
		{
			ConsolePrintf( Rgba::RED, "ERROR: net_sim_lag: Invalid min lag entered. Enter a positive floating number." );
			return false;
		}

		float max = 0.0f;
		try
		{
			max = stof( maxMsString );
		}
		catch ( std::invalid_argument& args )
		{
			UNUSED( args );
			ConsolePrintf( Rgba::RED, "ERROR: net_sim_lag: Invalid max lag entered. Enter a positive floating number." );
			return false;
		}
		if ( max < 0.0f )
		{
			ConsolePrintf( Rgba::RED, "ERROR: net_sim_lag: Invalid max lag entered. Enter a positive floating number." );
			return false;
		}

		NetSession::GetInstance()->SetSimLag( min, max );

		return true;
	}

	return false;
}

/*
	NET SESSION
*/

NetSession* g_theSession = nullptr;
std::vector< NetMessageDefinition > g_messageDefinitions;

/* static */
NetSession* NetSession::GetInstance()
{
	if ( g_theSession == nullptr )
	{
		g_theSession = new NetSession();
	}
	return g_theSession;
}

NetSession::NetSession()
	:	m_simLag( 0.0f, 0.0f )
{
	if ( g_theSession == nullptr )
	{
		CommandRegister( "net_sim_loss", NetSimLossCommand, "Sets the internal network loss probability to a value between 0.0 and 1.0." );
		CommandRegister( "net_sim_lag", NetSimLagCommand, "Sets the internal network lag range to a provided min, max millisecond value." );
	}

	m_socket = new UDPSocket();
	g_theSession = this;
}

NetSession::~NetSession()
{
	CommandUnregister( "net_sim_loss" );
	CommandUnregister( "net_sim_lag" );

	delete m_socket;
	m_socket = nullptr;

	g_theSession = nullptr;
}

void NetSession::RegisterMessage( const std::string& name, message_cb callback )
{
	NetMessageDefinition newDefinition = NetMessageDefinition( name, callback );

	// Insertion sort in alphabetical order
	size_t insertionIndex = 0U;
	for ( insertionIndex = 0U; insertionIndex < g_messageDefinitions.size(); insertionIndex++ )
	{
		if ( g_messageDefinitions[ insertionIndex ].m_name.compare( newDefinition.m_name ) >= 0 )
		{
			break;
		}
	}

	TODO( "Verify that this doesn't break" );
	g_messageDefinitions.insert( ( g_messageDefinitions.begin() + insertionIndex ), newDefinition );
}

void NetSession::AddBinding( unsigned int port, unsigned int portRange /* = 16U */ )
{
	NetAddressIPv4 toBind = NetAddressIPv4::GetBindableLocal( port );

	bool success = m_socket->Bind( toBind, port );

	unsigned int tryCount = portRange;
	while ( !success && tryCount > 0U )
	{
		port++;
		toBind = NetAddressIPv4::GetBindableLocal( port );
		success = m_socket->Bind( toBind, port );
		tryCount--;
	}

	if ( success )
	{
		ConsolePrintf( Rgba::GREEN, "NetSession::AddBinding(): Successfully bound to %s.", toBind.ToString().c_str() );
	}
	else
	{
		ConsolePrintf( Rgba::RED, "NetSession::AddBinding(): Binding failed - too many attempts." );
	}
}

NetConnection* NetSession::AddConnection( unsigned int index, const NetAddressIPv4& addr )
{
	if ( m_connections[ index ] != nullptr )
	{
		ConsolePrintf( Rgba::YELLOW, "NetSession::AddConnection(): Replacing connection at index %u.", index );
		m_connections[ index ]->m_address = addr;
		return m_connections[ index ];
	}
	m_connections[ index ] = new NetConnection( addr, this, index );
	m_numConnections = index + 1;
	ConsolePrintf( Rgba::GREEN, "NetSession::AddConnection(): Add success." );
	return m_connections[ index ];
}

void NetSession::SetSimLoss( float simLoss )
{
	m_simLoss = ClampFloat( simLoss, 0.0f, 1.0f );
}

void NetSession::SetSimLag( float minLag, float maxLag )
{
	m_simLag.min = minLag;
	m_simLag.max = Max( minLag, maxLag );
}

NetConnection* NetSession::GetConnection( unsigned int index ) const
{
	if ( index >= m_numConnections )
	{
		return nullptr;
	}
	return m_connections[ index ];
}

unsigned int NetSession::GetNumConnections() const
{
	return m_numConnections;
}

uint8_t NetSession::GetMessageIndex( const char* message ) const
{
	uint8_t messageIndex = UINT8_MAX;
	std::string messageStr = std::string( message );
	for ( size_t index = 0U; index < g_messageDefinitions.size(); index++ )
	{
		if ( g_messageDefinitions[ index ].m_name == messageStr )
		{
			messageIndex = static_cast< uint8_t >( index );
			break;
		}
	}
	return messageIndex;
}

UDPSocket* NetSession::GetSocket() const
{
	return m_socket;
}

NetConnection* NetSession::GetConnectionWithAddress( const NetAddressIPv4& addr ) const
{
	for ( unsigned int i = 0U; i < m_numConnections; i++ )
	{
		if ( m_connections[ i ]->m_address == addr )
		{
			return m_connections[ i ];
		}
	}
	return nullptr;
}

bool NetSession::VerifyPacket( NetPacket& packet )
{
	packet.ResetRead();

	if ( packet.GetReadableByteCount() < sizeof( NetPacketHeader ) )
	{
		ConsolePrintf( Rgba::RED, "NetSession::VerifyPacket(): Packet doesn't have enough bytes for a Packet header. Aborting..." );
		return false;
	}
	NetPacketHeader header;
	if ( !packet.ReadHeader( &header ) )
	{
		ConsolePrintf( Rgba::RED, "NetSession::VerifyPacket(): Invalid packet header. Aborting..." );
		return false;
	}

	if (
		( header.m_senderConnectionIndex != GetMyConnectionIndex() ) &&
		( header.m_senderConnectionIndex >= SESSION_MAX_CONNECTIONS || m_connections[ header.m_senderConnectionIndex ] == nullptr )
	)
	{
		ConsolePrintf( Rgba::RED, "NetSession::VerifyPacket(): Invalid connection index. Aborting..." );
		return false;
	}

	uint8_t numMessages = header.m_unreliableCount;
	while ( numMessages > 0U )
	{
		if ( packet.GetReadableByteCount() < 3U )	// sizeof( NetMessageHeader )
		{
			ConsolePrintf( Rgba::RED, "NetSession::VerifyPacket(): Expected message not present. Aborting..." );
			return false;
		}

		NetMessageHeader msgHeader;
		if ( !packet.ReadMessageHeader( &msgHeader ) )
		{
			ConsolePrintf( Rgba::RED, "NetSession::VerifyPacket(): Could not read message header. Aborting..." );
			return false;
		}

		if ( msgHeader.m_messageIndex >= static_cast< uint8_t >( g_messageDefinitions.size() ) )
		{
			ConsolePrintf( Rgba::RED, "NetSession::VerifyPacket(): Invalid message index. Aborting..." );
			return false;
		}

		uint16_t messageLength = msgHeader.m_messageLength - 1U;
		if ( packet.GetReadableByteCount() < messageLength )
		{
			ConsolePrintf( Rgba::RED, "NetSession::VerifyPacket(): Packet not large enough to hold message. Aborting..." );
			return false;
		}

		NetMessage unusedMessage;
		if ( !packet.ReadMessage( static_cast< size_t >( messageLength ), &unusedMessage ) )
		{
			ConsolePrintf( Rgba::RED, "NetSession::VerifyPacket(): Could not read message. Aborting..." );
			return false;
		}

		numMessages--;
	}

	if ( packet.GetReadHead() != packet.GetWriteHead() )
	{
		ConsolePrintf( Rgba::RED, "NetSession::VerifyPacket(): Packet length not matching from header counts. Aborting..." );
		return false;
	}

	packet.ResetRead();
	return true;
}

void NetSession::ProcessIncoming()
{
	NetAddressIPv4 senderAddr;
	size_t maxSize = PACKET_MTU;
	char* buffer = new char[ maxSize ];

	size_t received = m_socket->ReceiveFrom(
		&senderAddr,
		buffer,
		maxSize
	);

	if ( received > 0U && CheckRandomChance( 1.0f - m_simLoss ) )
	{
		NetConnection* connection = GetConnectionWithAddress( senderAddr );
		if ( connection != nullptr )
		{
			// Extract packet header
			// For numMessages,
				// Extract message length
				// Extract message ID
				// Validate message ID
				// Extract payload
			NetPacket* packet = new NetPacket( connection->m_connectionIndex, false );
			packet->WriteBytes( received, buffer );

			AddIncomingPacketToQueue( connection, packet );
		}
		else
		{
			ConsolePrintf( Rgba::RED, "NetSession::ProcessIncoming(): Invalid connection index." );
		}
	}

	delete[] buffer;

	ProcessIncomingPacketQueue();
}

void NetSession::AddIncomingPacketToQueue( NetConnection* connection, NetPacket* packet )
{
	FILETIME fileTime = GetCurrentFileTime();
	IncrementFileTime( &fileTime, GetRandomFloatInRange( m_simLag.min, m_simLag.max ) );
	DelayedPacket delayedPacket( connection, packet, fileTime );

	// Insertion sort
	size_t insertionIndex = 0U;
	for ( size_t index = 0U; index < m_incomingPacketQueue.size(); index++ )
	{
		if ( CompareFileTime( &delayedPacket.m_timestamp, &m_incomingPacketQueue[ index ].m_timestamp ) <= 0 )
		{
			break;
		}
		insertionIndex++;
	}

	m_incomingPacketQueue.insert(
		( m_incomingPacketQueue.begin() + insertionIndex ),
		delayedPacket
	);
}

void NetSession::ProcessIncomingPacketQueue()
{
	if ( m_incomingPacketQueue.size() == 0U )
	{
		return;
	}

	FILETIME currentFileTime = GetCurrentFileTime();

	for ( size_t index = 0U; index < m_incomingPacketQueue.size(); index++ )
	{
		if ( CompareFileTime( &currentFileTime, &m_incomingPacketQueue[ index ].m_timestamp ) <= 0 )
		{
			NetPacket* packet = m_incomingPacketQueue[ index ].m_packet;
			NetConnection* connection = m_incomingPacketQueue[ index ].m_connection;
			if ( VerifyPacket( *packet ) )
			{
				NetPacketHeader packetHeader;
				if ( packet->ReadHeader( &packetHeader ) )
				{
					uint8_t senderIndex = packetHeader.m_senderConnectionIndex;
					uint8_t numMessages = packetHeader.m_unreliableCount;

					while( numMessages > 0U )
					{
						NetMessageHeader messageHeader;
						if ( !packet->ReadMessageHeader( &messageHeader ) )
						{
							ConsolePrintf( Rgba::RED, "NetSession::ProcessIncoming(): NetMessage: Invalid header." );
							break;
						}

						NetMessage message = NetMessage( false );
						packet->ReadMessage( ( messageHeader.m_messageLength - 1U ), &message );

						NetMessageDefinition& messageDefinition = g_messageDefinitions[ messageHeader.m_messageIndex ];
						messageDefinition.m_callback( message, *connection );
						numMessages--;
					}
				}
				else
				{
					ConsolePrintf( Rgba::RED, "NetSession::ProcessIncomingPacketQueue(): Invalid packet received: could not read header." );
				}
			}
			else
			{
				ConsolePrintf( Rgba::RED, "NetSession::ProcessIncomingPacketQueue(): NetPacket: Invalid packet." );
			}

			delete packet;

			// Fast removal
			m_incomingPacketQueue[ index ] = m_incomingPacketQueue[ m_incomingPacketQueue.size() - 1 ];
			m_incomingPacketQueue.erase( m_incomingPacketQueue.begin() + index );
			index--;
		}
	}
}

void NetSession::ProcessOutgoing()
{
	for ( unsigned int connectionIndex = 0U; connectionIndex < m_numConnections; connectionIndex++ )
	{
		m_connections[ connectionIndex ]->ProcessOutgoing();
	}
}
