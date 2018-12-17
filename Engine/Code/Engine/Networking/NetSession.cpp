#include "Engine/Core/Clock.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Networking/NetPacket.hpp"
#include "Engine/Networking/NetSession.hpp"
#include "Engine/Networking/UDPSocket.hpp"
#include "Engine/Tools/DevConsole.hpp"
#include "Game/EngineBuildPreferences.hpp"

#if !defined( ENGINE_DISABLE_NETWORKING )

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

bool NetSessionSendRateCommand( Command& command )
{
	if ( command.GetName() == "net_session_send_rate" )
	{
		std::string freqStr = command.GetNextString();
		if ( freqStr == "" )
		{
			ConsolePrintf( Rgba::RED, "ERROR: net_set_session_send_rate: No frequency entered." );
			return false;
		}

		float frequency = 0.0f;
		try
		{
			frequency = stof( freqStr );
		}
		catch ( std::invalid_argument& args )
		{
			UNUSED( args );
			ConsolePrintf( Rgba::RED, "ERROR: net_set_session_send_rate: Invalid frequency entered. Enter a number above 0.01." );
			return false;
		}

		if ( frequency < 0.01f )
		{
			ConsolePrintf( Rgba::YELLOW, "WARNING: net_set_session_send_rate: Setting amount to 0.01f." );
		}

		NetSession::GetInstance()->SetSendRate( frequency );
		return true;
	}

	return false;
}

bool NetConnectionSendRateCommand( Command& command )
{
	if ( command.GetName() == "net_connection_send_rate" )
	{
		std::string connIndexStr = command.GetNextString();
		if ( connIndexStr == "" )
		{
			ConsolePrintf( Rgba::RED, "ERROR: net_set_connection_send_rate: No connection index entered." );
			return false;
		}

		int connIndex;
		try
		{
			connIndex = stoi( connIndexStr );
		}
		catch ( std::invalid_argument& args )
		{
			UNUSED( args );
			ConsolePrintf( Rgba::RED, "ERROR: net_set_connection_send_rate: Invalid connection index entered. Enter a whole number between 0 and %u.", ( SESSION_MAX_CONNECTIONS - 1U ) );
			return false;
		}

		if ( connIndex < 0 || static_cast< uint8_t >( connIndex ) >= SESSION_MAX_CONNECTIONS )
		{
			ConsolePrintf( Rgba::RED, "ERROR: net_set_connection_send_rate: Invalid connection index entered. Enter a whole number between 0 and %u.", ( SESSION_MAX_CONNECTIONS - 1U ) );
			return false;
		}

		std::string freqStr = command.GetNextString();
		if ( freqStr == "" )
		{
			ConsolePrintf( Rgba::RED, "ERROR: net_set_connection_send_rate: No frequency entered." );
			return false;
		}

		float frequency = 0.0f;
		try
		{
			frequency = stof( freqStr );
		}
		catch ( std::invalid_argument& args )
		{
			UNUSED( args );
			ConsolePrintf( Rgba::RED, "ERROR: net_set_connection_send_rate: Invalid frequency entered. Enter a number above 0.01." );
			return false;
		}

		if ( frequency < 0.01f )
		{
			ConsolePrintf( Rgba::YELLOW, "WARNING: net_set_connection_send_rate: Setting amount to 0.01f." );
		}
		frequency = Max( frequency, 0.01f );

		NetConnection* connection = NetSession::GetInstance()->GetConnection( static_cast< uint8_t >( connIndex ) );
		if ( connection == nullptr )
		{
			ConsolePrintf( Rgba::RED, "ERROR: net_set_connection_send_rate: No valid connection for the provided index." );
			return false;
		}

		connection->SetSendRate( frequency );
		return true;
	}

	return false;
}

bool NetSetHeartRateCommand( Command& command )
{
	if ( command.GetName() == "net_heart_rate" )
	{
		std::string freqStr = command.GetNextString();
		if ( freqStr == "" )
		{
			ConsolePrintf( Rgba::RED, "ERROR: net_set_heart_rate: No heart rate entered." );
			return false;
		}

		float freq = 0.0f;
		try
		{
			freq = stof( freqStr );
		}
		catch ( std::invalid_argument& args )
		{
			UNUSED( args );
			ConsolePrintf( Rgba::RED, "ERROR: net_set_heart_rate: Invalid frequency entered. Enter a number greater than 0.01." );
			return false;
		}

		if ( freq < 0.01f )
		{
			ConsolePrintf( Rgba::YELLOW, "WARNING: net_set_heart_rate: Setting frequency to 0.01." );
		}

		NetSession::GetInstance()->SetHeartRate( freq );
		return true;
	}

	return false;
}

/*
	NET SESSION MESSAGES
*/
bool OnPing( NetMessage& message, NetConnection& connection )
{
	char* str = new char[ MESSAGE_MTU ];
	size_t sizeRead = message.ReadString( str, MESSAGE_MTU );
	str[ sizeRead ] = '\0';

	ConsolePrintf( "Received ping from %s: %s", connection.m_address.ToString().c_str(), str );

	uint8_t messageIndex = NetSession::GetInstance()->GetMessageIndex( "pong" );

	NetMessage pong( messageIndex );
	if ( connection.m_connectionIndex != SESSION_MAX_CONNECTIONS )
	{
		// Push into send queue and wait for next net tick
		connection.Send( pong );
	}
	else
	{
		// Not really a connection. Send immediately, as this "connection" will be deleted right after this callback.
		connection.SendImmediate( pong );
	}

	delete[] str;

	return true;
}

bool OnPong( NetMessage& message, NetConnection& connection )
{
	UNUSED( message );
	ConsolePrintf( "Received pong from %s", connection.m_address.ToString().c_str() );
	return true;
}

bool OnHeartbeat( NetMessage& message, NetConnection& connection )
{
	if ( !NetSession::GetInstance()->IsHost() && connection.IsHost() )
	{
		float hostTimeMS = 0.0f;
		message.ReadFloat( &hostTimeMS );
		NetSession::GetInstance()->SetClientTimeFromHostTime( hostTimeMS );
	}
	
	return true;
}

bool OnJoinRequest( NetMessage& message, NetConnection& connection )
{
	LogTaggedPrintf( "OnJoinRequest", "Request received from %s", connection.m_address.ToString().c_str() );

	if ( !NetSession::GetInstance()->GetMyConnection() || !NetSession::GetInstance()->GetMyConnection()->IsHost() )
	{
		NetMessage joinDeny( static_cast< uint8_t >( NETMSG_JOIN_DENY ) );

		uint8_t errorCode = static_cast< uint8_t >( SESSION_ERROR_JOIN_DENIED_NOT_HOST );
		joinDeny.WriteBytes( 1, &errorCode );
		
		connection.SendImmediate( joinDeny );	// Connectionless

		LogTaggedPrintf( "OnJoinRequest", "Join denied - not host." );
		return false;
	}
	if ( NetSession::GetInstance()->GetNumBoundConnections() >= SESSION_MAX_CONNECTIONS )
	{
		NetMessage joinDeny( static_cast< uint8_t >( NETMSG_JOIN_DENY ) );

		uint8_t errorCode = static_cast< uint8_t >( SESSION_ERROR_JOIN_DENIED_FULL );
		joinDeny.WriteBytes( 1, &errorCode );

		connection.SendImmediate( joinDeny );	// Connectionless

		LogTaggedPrintf( "OnJoinRequest", "Join denied - session full." );
		return false;
	}
	if ( NetSession::GetInstance()->ConnectionExistsForAddress( connection.m_address ) )
	{
		// Ignore
		LogTaggedPrintf( "OnJoinRequest", "Join ignored - connection exists." );
		return false;
	}

	NetConnection* newConnection = NetSession::GetInstance()->CreateConnection( connection.m_address );
	if ( newConnection == nullptr )
	{
		NetMessage joinDeny( static_cast< uint8_t >( NETMSG_JOIN_DENY ) );

		uint8_t errorCode = static_cast< uint8_t >( SESSION_ERROR_COULD_NOT_BIND );
		joinDeny.WriteBytes( 1, &errorCode );

		connection.SendImmediate( joinDeny );	// Connectionless

		LogTaggedPrintf( "OnJoinRequest", "Join denied - could not bind." );
		return false;
	}

	NetConnectionInfo info;
	message.ReadConnectionInfo( &info );
	newConnection->SetID( info.m_id );

	NetConnectionInfo newInfo = newConnection->GetInfo();
	float hostTimeMS = NetSession::GetInstance()->GetNetTimeMS();

	NetMessage joinAccept( static_cast< uint8_t >( NETMSG_JOIN_ACCEPT ) );
	joinAccept.WriteConnectionInfo( newInfo );
	joinAccept.WriteFloat( hostTimeMS );
	newConnection->Send( joinAccept );
	LogTaggedPrintf( "OnJoinRequest", "Join accepted." );

	NetMessage joinFinished( static_cast< uint8_t >( NETMSG_JOIN_FINISHED ) );
	NetConnectionInfo hostInfo = NetSession::GetInstance()->GetHostConnection()->GetInfo();
	joinFinished.WriteConnectionInfo( hostInfo );
	newConnection->Send( joinFinished );
	LogTaggedPrintf( "OnJoinRequest", "Join finished." );

	return true;
}

bool OnJoinDeny( NetMessage& message, NetConnection& connection )
{
	SessionError errorCode;
	message.ReadBytes( &errorCode, 1U );
	NetSession::GetInstance()->SetError( errorCode );

	NetConnection* actualConnection = NetSession::GetInstance()->GetConnectionWithAddress( connection.m_address );
	if ( actualConnection )
	{
		actualConnection->Disconnect();
	}

	LogTaggedPrintf( "OnJoinDeny", "Join denied: %u.", static_cast< uint8_t >( errorCode ) );
	return true;
}

bool OnJoinAccept( NetMessage& message, NetConnection& connection )
{
	NetConnectionInfo info;
	float hostTimeMS = 0.0f;
	message.ReadConnectionInfo( &info );
	message.ReadFloat( &hostTimeMS );
	
	NetSession::GetInstance()->BindConnection( info.m_connectionIndex, NetSession::GetInstance()->GetMyConnection() );	// The connection is already in the all_connections list
	NetSession::GetInstance()->SetClientTimeFromHostTime( hostTimeMS, true );
	
	LogTaggedPrintf( "OnJoinAccept", "Join finished." );
	return true;
}

bool OnJoinFinished( NetMessage& message, NetConnection& connection )
{
	connection.SetState( CONNECTION_READY );

	NetConnectionInfo hostInfo;
	message.ReadConnectionInfo( &hostInfo );
	connection.SetID( hostInfo.m_id );

	LogTaggedPrintf( "OnJoinFinished", "Join finished." );
	return true;
}

bool OnNewConnection( NetMessage& message, NetConnection& connection )
{
	UNUSED( message );
	UNUSED( connection );
	return true;
}

bool OnUpdateConnectionState( NetMessage& message, NetConnection& connection )
{
	ConnectionState newConnectionState;
	message.ReadBytes( &newConnectionState, 1U );
	connection.SetState( newConnectionState );
	LogTaggedPrintf( "OnUpdateConnectionState", "Updated state: %u.", static_cast< uint8_t >( newConnectionState ) );

	return true;
}

bool OnHangUp( NetMessage& message, NetConnection& connection )
{
	LogPrintf( "Connection %u hung up.", connection.m_connectionIndex );
	connection.Disconnect();
	return true;
}

/*
	NET SESSION GLOBALS
*/

NetSession* g_theSession = nullptr;

std::vector< NetMessageDefinition > g_messageDefinitions;
bool NetSessionMessagesCommand( Command& command )
{
	if ( command.GetName() == "net_session_messages" )
	{
		ConsolePrintf( "Registered session messages:" );
		ConsolePrintf( "%6s%30s", "INDEX", "MESSAGE" );
		for ( size_t index = 0U; index < g_messageDefinitions.size(); index++ )
		{
			ConsolePrintf(
				"%6d%30s",
				g_messageDefinitions[ index ].m_messageIndex,
				g_messageDefinitions[ index ].m_name.c_str()
			);
		}
		return true;
	}
	
	return false;
}

void CascadeUpdateMessageDefinitionList()
{
	std::vector< NetMessageDefinition > sortingArray;

	// Add non-indexed messages first
	for ( size_t messageDefIndex = 0U; messageDefIndex < g_messageDefinitions.size(); messageDefIndex++ )
	{
		if ( g_messageDefinitions[ messageDefIndex ].m_messageIndex <= MESSAGE_DEFINITION_NO_INDEX )
		{
			sortingArray.push_back( g_messageDefinitions[ messageDefIndex ] );
			g_messageDefinitions.erase( g_messageDefinitions.begin() + messageDefIndex );
			messageDefIndex--;
		}
	}

	// Then insert the remaining indexed messages in order
	for ( size_t indexedMessageDefIndex = 0U; indexedMessageDefIndex < g_messageDefinitions.size(); indexedMessageDefIndex++ )
	{
		size_t insertionIndex = Min( static_cast< size_t >( g_messageDefinitions[ indexedMessageDefIndex ].m_messageIndex ), ( sortingArray.size() ) );
		sortingArray.insert( ( sortingArray.begin() + insertionIndex ), g_messageDefinitions[ indexedMessageDefIndex ] );
		g_messageDefinitions.erase( g_messageDefinitions.begin() + indexedMessageDefIndex );
		indexedMessageDefIndex--;
	}

	// Finally, insert the sorted definitions back into the global array
	for ( size_t messageDefIndex = 0U; messageDefIndex < sortingArray.size(); messageDefIndex++ )
	{
		g_messageDefinitions.push_back( sortingArray[ messageDefIndex ] );
	}
}

/*
	NET SESSION CLASS
*/

/* static */
NetSession* NetSession::GetInstance()
{
	if ( g_theSession == nullptr )
	{
		g_theSession = new NetSession();
	}
	return g_theSession;
}

/* static */
void NetSession::ShutdownInstance()
{
	delete g_theSession;
	g_theSession = nullptr;
}

NetSession::NetSession()
	:	m_simLag( 0.0f, 0.0f )
{
	if ( g_theSession == nullptr )
	{
		CommandRegister( "net_sim_loss", NetSimLossCommand, "Sets the internal network loss probability to a value between 0.0 and 1.0." );
		CommandRegister( "net_sim_lag", NetSimLagCommand, "Sets the internal network lag range to a provided min, max millisecond value." );
		CommandRegister( "net_session_send_rate", NetSessionSendRateCommand, "Sets the flush frequency of the session." );
		CommandRegister( "net_connection_send_rate", NetConnectionSendRateCommand, "Sets the flush frequency of the connection at the specified index." );
		CommandRegister( "net_heart_rate", NetSetHeartRateCommand, "Sets the heartbeat interval of all connections to the specified frequency." );
		CommandRegister( "net_session_messages", NetSessionMessagesCommand, "Lists the messages currently registered with the session, and their reserved indices, if specified." );
	}

	RegisterMessage( NETMSG_PING, "ping", OnPing, NETMSG_OPTION_CONNECTIONLESS );
	RegisterMessage( NETMSG_PONG, "pong", OnPong, NETMSG_OPTION_CONNECTIONLESS );
	RegisterMessage( NETMSG_HEARTBEAT, "heartbeat", OnHeartbeat );
	RegisterMessage( NETMSG_JOIN_REQUEST, "join_request", OnJoinRequest, NETMSG_OPTION_CONNECTIONLESS );
	RegisterMessage( NETMSG_JOIN_DENY, "join_deny", OnJoinDeny );
	RegisterMessage( NETMSG_JOIN_ACCEPT, "join_accept", OnJoinAccept, ( NETMSG_OPTION_RELIABLE | NETMSG_OPTION_IN_ORDER ) );
	RegisterMessage( NETMSG_JOIN_FINISHED, "join_finished", OnJoinFinished, ( NETMSG_OPTION_RELIABLE | NETMSG_OPTION_IN_ORDER ) );
	RegisterMessage( NETMSG_NEW_CONNECTION, "new_connection", OnNewConnection, ( NETMSG_OPTION_RELIABLE | NETMSG_OPTION_IN_ORDER ) );
	RegisterMessage( NETMSG_UPDATE_CONNECTION_STATE, "update_connection_state", OnUpdateConnectionState, ( NETMSG_OPTION_RELIABLE | NETMSG_OPTION_IN_ORDER ) );
	RegisterMessage( NETMSG_HANGUP, "hangup", OnHangUp );

	m_socket = new UDPSocket();
	m_netClock = new Clock();
	m_netClock->SetParent( nullptr );
	g_theSession = this;
}

NetSession::~NetSession()
{
	Disconnect();

	CommandUnregister( "net_sim_loss" );
	CommandUnregister( "net_sim_lag" );
	CommandUnregister( "net_session_send_rate" );
	CommandUnregister( "net_connection_send_rate" );
	CommandUnregister( "net_heart_rate" );
	CommandUnregister( "net_session_messages" );

	delete m_netClock;
	m_netClock = nullptr;

	delete m_socket;
	m_socket = nullptr;

	g_theSession = nullptr;
}

void NetSession::RegisterMessage( const std::string& name, message_cb callback, unsigned int options /* = 0U */, unsigned int channelIndex /* = 0U */ )
{
	NetMessageDefinition newDefinition = NetMessageDefinition( name, callback, options, -1, channelIndex );

	// Insertion sort in alphabetical order
	size_t insertionIndex = 0U;
	for ( insertionIndex = 0U; insertionIndex < g_messageDefinitions.size(); insertionIndex++ )
	{
		if ( g_messageDefinitions[ insertionIndex ].m_messageIndex == insertionIndex )	// A reserved, fixed index message cannot be displaced
		{
			continue;
		}
		if ( g_messageDefinitions[ insertionIndex ].m_name.compare( newDefinition.m_name ) >= 0 )
		{
			break;
		}
	}

	g_messageDefinitions.insert( ( g_messageDefinitions.begin() + insertionIndex ), newDefinition );
	CascadeUpdateMessageDefinitionList();
}

bool NetSession::RegisterMessage( int messageIndex, const std::string& name, message_cb callback, unsigned int options /* = 0U */, unsigned int channelIndex /* = 0U */ )
{
	if ( messageIndex <= MESSAGE_DEFINITION_NO_INDEX )
	{
		ConsolePrintf( Rgba::RED, "ERROR: NetSession::RegisterMessage(): Invalid message index. If no message index is required, use the other overload." );
		return false;
	}

	NetMessageDefinition newDefinition = NetMessageDefinition( name, callback, options, messageIndex, channelIndex );
	if ( static_cast< int >( g_messageDefinitions.size() ) > messageIndex )
	{
		// Can be inserted directly unless a registered message uses the same index
		if ( g_messageDefinitions[ messageIndex ].m_messageIndex == messageIndex )
		{
			ConsolePrintf( Rgba::RED, "ERROR: NetSession::RegisterMessage(): A registered message already uses the index %d.", messageIndex );
			return false;
		}

		g_messageDefinitions.insert( ( g_messageDefinitions.begin() + static_cast< size_t >( messageIndex ) ), newDefinition );
	}
	else
	{
		// Make sure the new message is not inserted after another with a higher index
		size_t insertionIndex = 0U;
		for ( insertionIndex = 0U; insertionIndex < g_messageDefinitions.size(); insertionIndex++ )
		{
			if ( g_messageDefinitions[ insertionIndex ].m_messageIndex > messageIndex )
			{
				break;
			}
		}
		g_messageDefinitions.insert( ( g_messageDefinitions.begin() + insertionIndex ), newDefinition );	// The definition will eventually cascade into place when more messages are inserted
	}

	CascadeUpdateMessageDefinitionList();
	return true;
}

uint16_t NetSession::AddBinding( uint16_t port, uint16_t portRange /* = 16U */ )
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
		return port;
	}
	else
	{
		ConsolePrintf( Rgba::RED, "NetSession::AddBinding(): Binding failed - too many attempts." );
		return 0U;
	}
}

bool NetSession::RemoveBinding()
{
	m_socket->Close();
	return m_socket->IsClosed();
}

void NetSession::SetError( SessionError error, const char* errorStr /* = nullptr */ )
{
	m_error = error;
	if ( errorStr )
	{
		m_errorString = std::string( errorStr );
	}
}

void NetSession::ClearError()
{
	m_error = SESSION_OK;
	m_errorString = "";
}

SessionError NetSession::GetLastError( std::string* out_error /* = nullptr */ )
{
	SessionError error = m_error;
	if ( out_error )
	{
		*out_error = m_errorString;
	}
	ClearError();
	return error;
}

NetConnection* NetSession::CreateConnection( const NetAddressIPv4& addr )
{
	uint8_t bindableIndex = GetBindableConnectionIndex();
	if ( bindableIndex == SESSION_MAX_CONNECTIONS )
	{
		return nullptr;
	}

	NetConnectionInfo info;
	info.m_address = addr;
	info.m_connectionIndex = bindableIndex;
	
	return CreateConnection( info );
}

NetConnection* NetSession::CreateConnection( const NetConnectionInfo& info )
{
	NetConnection* connection = new NetConnection( info, this );
	m_allConnections.push_back( connection );
	if ( CanBindConnection( connection ) )
	{
		BindConnection( connection->m_connectionIndex, connection );
	}
	return connection;
}

void NetSession::DestroyConnection( NetConnection* connection )
{
	if ( connection != nullptr )
	{
		if ( m_myConnection == connection )
		{
			m_myConnection = nullptr;
		}
		if ( m_hostConnection == connection )
		{
			m_hostConnection = nullptr;
		}

		uint8_t connectionIndex = GetIndexForConnection( connection );
		m_boundConnections[ connectionIndex ] = nullptr;

		std::vector<NetConnection*>::iterator foundConnection = std::find( m_allConnections.begin(), m_allConnections.end(), connection );
		if ( foundConnection != m_allConnections.end() )
		{
			m_allConnections.erase( foundConnection );
		}

		delete connection;
	}
}

void NetSession::BindConnection( uint8_t connectionIndex, NetConnection* connection )
{
	m_boundConnections[ connectionIndex ] = connection;
	connection->m_connectionIndex = connectionIndex;
	connection->SetState( CONNECTION_CONNECTED );
}

bool NetSession::CanBindConnection( NetConnection* connection ) const
{
	return (
		( connection != nullptr ) &&
		( connection->m_connectionIndex < SESSION_MAX_CONNECTIONS ) &&
		( m_boundConnections[ connection->m_connectionIndex ] == nullptr )
	);
}

bool NetSession::ConnectionExistsForAddress( const NetAddressIPv4& addr ) const
{
	for ( NetConnection* connection : m_allConnections )
	{
		if ( connection->m_address == addr )
		{
			return true;
		}
	}

	return false;
}

uint8_t NetSession::GetBindableConnectionIndex() const
{
	for ( uint8_t connIndex = 0U; connIndex < SESSION_MAX_CONNECTIONS; connIndex++ )
	{
		if ( m_boundConnections[ connIndex ] == nullptr )
		{
			return connIndex;
		}
	}

	return SESSION_MAX_CONNECTIONS;
}

uint8_t NetSession::GetNumBoundConnections() const
{
	uint8_t numBound = 0U;

	for ( NetConnection* connection : m_boundConnections )
	{
		if ( connection != nullptr )
		{
			numBound++;
		}
	}

	return numBound;
}

NetConnectionInfo NetSession::GetConnectionInfo( uint8_t connectionIndex ) const
{
	NetConnectionInfo info;
	return m_boundConnections[ connectionIndex ]->GetInfo();
}

void NetSession::BroadcastMessage( NetMessage& message )
{
	for ( uint8_t connIndex = 0U; connIndex < SESSION_MAX_CONNECTIONS; connIndex++ )
	{
		NetConnection* connection = m_boundConnections[ connIndex ];
		if ( connection && connection != m_myConnection )
		{
			connection->Send( message );
		}
	}
}

void NetSession::SetClientTimeFromHostTime( float hostTimeMS, bool setCurrentTime /* = false */ )
{
	if ( hostTimeMS < m_lastReceivedHostTimeMS )
	{
		return;
	}

	m_lastReceivedHostTimeMS = hostTimeMS + ( m_hostConnection->GetRTT() * 0.5f );
	m_desiredClientTimeMS = m_lastReceivedHostTimeMS;
	if ( setCurrentTime )
	{
		m_currentClientTimeMS = m_desiredClientTimeMS;
	}
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

void NetSession::SetSendRate( float sendRateHz )
{
	sendRateHz = Max( sendRateHz, 0.01f );
	m_sendIntervalMS = 1000.0f / sendRateHz;
	for ( uint8_t connIndex = 0U; connIndex < SESSION_MAX_CONNECTIONS; connIndex++ )
	{
		NetConnection* connection = m_boundConnections[ connIndex ];
		if ( connection != nullptr )
		{
			connection->SetSendRate( connection->GetSendRate() );	// Update connection send rate as Max( Session::send_rate, Connection::send_rate )
		}
	}
}

void NetSession::SetHeartRate( float heartRateHz )
{
	heartRateHz = Max( heartRateHz, 0.01f );
	for ( uint8_t connIndex = 0U; connIndex < SESSION_MAX_CONNECTIONS; connIndex++ )
	{
		if ( m_boundConnections[ connIndex ] != nullptr )
		{
			m_boundConnections[ connIndex ]->SetHeartRate( heartRateHz );
		}
	}
}

bool NetSession::IsHost() const
{
	return ( m_hostConnection != nullptr && m_myConnection == m_hostConnection );
}

uint8_t NetSession::GetMyConnectionIndex() const
{
	return ( ( m_myConnection )? GetIndexForConnection( m_myConnection ) : SESSION_MAX_CONNECTIONS );
}

NetConnection* NetSession::GetMyConnection() const
{
	return m_myConnection;
}

NetConnection* NetSession::GetHostConnection() const
{
	return m_hostConnection;
}

NetConnection* NetSession::GetConnection( uint8_t index ) const
{
	if ( index >= SESSION_MAX_CONNECTIONS )
	{
		return nullptr;
	}
	return m_boundConnections[ index ];
}

uint8_t NetSession::GetNumConnections() const
{
	return SESSION_MAX_CONNECTIONS;
}

uint8_t NetSession::GetMessageIndex( const char* messageName ) const
{
	uint8_t messageIndex = UINT8_MAX;
	std::string messageStr = std::string( messageName );
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

NetMessageDefinition* NetSession::GetMessageDefinition( uint8_t messageIndex ) const
{
	if ( messageIndex >= static_cast< uint8_t >( g_messageDefinitions.size() ) )
	{
		return nullptr;
	}

	return &g_messageDefinitions[ messageIndex ];
}

NetMessageDefinition* NetSession::GetMessageDefinition( const char* messageName ) const
{
	uint8_t messageIndex = UINT8_MAX;
	std::string messageStr = std::string( messageName );
	for ( size_t index = 0U; index < g_messageDefinitions.size(); index++ )
	{
		if ( g_messageDefinitions[ index ].m_name == messageStr )
		{
			return &g_messageDefinitions[ index ];
		}
	}
	return nullptr;
}

UDPSocket* NetSession::GetSocket() const
{
	return m_socket;
}

FloatRange NetSession::GetSimLag() const
{
	return m_simLag;
}

float NetSession::GetSendIntervalMilliseconds() const
{
	return m_sendIntervalMS;
}

float NetSession::GetSendRate() const
{
	return ( 1000.0f / m_sendIntervalMS );
}

float NetSession::GetSimLoss() const
{
	return m_simLoss;
}

float NetSession::GetNetTimeMS() const
{
	if ( IsHost() )
	{
		TimeUnit totalTime = m_netClock->GetTotalTime();
		return static_cast< float >( totalTime.milliSeconds );
	}

	return m_currentClientTimeMS;
}

Clock* NetSession::GetNetClock() const
{
	return m_netClock;
}

uint8_t NetSession::GetIndexForConnection( NetConnection* connection ) const
{
	for ( uint8_t i = 0U; i < SESSION_MAX_CONNECTIONS; i++ )
	{
		if ( m_boundConnections[ i ] == connection )
		{
			return i;
		}
	}
	return SESSION_MAX_CONNECTIONS;
}

NetConnection* NetSession::GetConnectionWithAddress( const NetAddressIPv4& addr ) const
{
	for ( uint8_t i = 0U; i < SESSION_MAX_CONNECTIONS; i++ )
	{
		if ( m_boundConnections[ i ] != nullptr && m_boundConnections[ i ]->m_address == addr )
		{
			return m_boundConnections[ i ];
		}
	}
	return nullptr;
}

bool NetSession::VerifyPacket( NetPacket& packet )
{
	packet.ResetRead();

	if ( packet.GetReadableByteCount() < 8U )
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

	if ( header.m_senderConnectionIndex > SESSION_MAX_CONNECTIONS )	// m_senderConnectionIndex == SESSION_MAX_CONNECTIONS for connectionless messages
	{
		ConsolePrintf( Rgba::RED, "NetSession::VerifyPacket(): Invalid connection index. Aborting..." );
		return false;
	}

	uint8_t numMessages = header.m_messageCount;
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

		NetMessageDefinition* definition = GetMessageDefinition( msgHeader.m_messageIndex );
		uint16_t messageLength = msgHeader.m_messageLength - NetMessage::GetHeaderNumBytesMinusLength( *definition );
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

void NetSession::Update()
{
	UpdateNetTime();
	ProcessIncoming();

	if ( m_myConnection != nullptr )
	{
		if ( m_state == SESSION_CONNECTING )
		{
			static float s_joinRequestTimeout = 0.0f;
			static float s_joinTimeout = 0.0f;

			s_joinRequestTimeout += m_netClock->GetDeltaSecondsF();
			s_joinTimeout += m_netClock->GetDeltaSecondsF();

			if ( m_myConnection->IsConnected() )
			{
				m_state = SESSION_JOINING;
			}
			else
			{
				if ( s_joinTimeout > JOIN_TIMEOUT_SECONDS )
				{
					LogErrorf( "NetSession::Update(): join request to %s timed out. Reverting to disconnected state.", m_hostConnection->m_address.ToString().c_str() );
					m_error = SESSION_ERROR_JOIN_TIMEOUT;

					m_hostConnection->Disconnect();

					s_joinTimeout = 0.0f;
					s_joinRequestTimeout = 0.0f;
				}
				else if ( s_joinRequestTimeout > JOIN_REQUEST_TIMEOUT_SECONDS )
				{
					NetMessage joinRequest( static_cast<uint8_t>( NETMSG_JOIN_REQUEST ) );
					NetConnectionInfo info;
					info.SetID( m_myConnection->m_id );
					joinRequest.WriteConnectionInfo( info );

					m_hostConnection->Send( joinRequest );
					s_joinRequestTimeout = 0.0f;
				}
			}
		}
		else if ( m_state == SESSION_JOINING )
		{
			if ( m_myConnection->IsReady() )
			{
				m_myConnection->SetState( CONNECTION_CONNECTED );
				m_state = SESSION_CONNECTED;
			}
		}
	}

	// Clean up bound connections
 	for ( uint8_t connIndex = 0U; connIndex < SESSION_MAX_CONNECTIONS; connIndex++ )
 	{
 		NetConnection* connection = m_boundConnections[ connIndex ];
 		if ( connection && connection->IsDisconnected() )
 		{
 			if ( connection == m_hostConnection || connection == m_myConnection )
 			{
 				Disconnect();	// Fatal - disconnect session
 				break;
 			}

			DestroyConnection( connection );
			m_boundConnections[ connIndex ] = nullptr;
 		}
 	}
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

	while ( received > 0U )
	{
		uint16_t *ackPtr = ( uint16_t* )( buffer + 1 );
		if ( CheckRandomChance( m_simLoss ) )
		{
			// Packet dropped
			// LogPrintf( "NetSession::ProcessIncoming(): Lost packet %u.", *ackPtr );
		}
		else
		{
			NetConnection* connection = GetConnectionWithAddress( senderAddr );
			unsigned int connectionIndex = ( connection != nullptr )? connection->m_connectionIndex : SESSION_MAX_CONNECTIONS;
			if ( connection == nullptr )
			{
				connection = new NetConnection( senderAddr, this, connectionIndex );	// Will be removed after processing if connectionIndex == SESSION_MAX_CONNECTIONS
			}

			NetPacket* packet = new NetPacket(
				connectionIndex,
				false
			);
			packet->WriteBytes( received, buffer );

			AddIncomingPacketToQueue( connection, packet );
		}

		received = m_socket->ReceiveFrom(
			&senderAddr,
			buffer,
			maxSize
		);
	}

	delete[] buffer;

	ProcessIncomingPacketQueue();
}

void NetSession::AddIncomingPacketToQueue( NetConnection* connection, NetPacket* packet )
{
	float currentTimeMS = GetNetTimeMS();
	float packetReceiveTime = currentTimeMS + GetRandomFloatInRange( m_simLag.min, m_simLag.max );
	DelayedPacket delayedPacket( connection, packet, packetReceiveTime );

	// Insertion sort
	size_t insertionIndex = 0U;
	for ( size_t index = 0U; index < m_incomingPacketQueue.size(); index++ )
	{
		if ( m_incomingPacketQueue[ index ].m_receiveTimeMS > delayedPacket.m_receiveTimeMS )
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

	float currentMilliseconds = GetNetTimeMS();

	for ( size_t index = 0U; index < m_incomingPacketQueue.size(); index++ )
	{
		if ( IsFloatGreaterThanOrEqualTo( currentMilliseconds, m_incomingPacketQueue[ index ].m_receiveTimeMS ) )
		{
			NetPacket* packet = m_incomingPacketQueue[ index ].m_packet;
			NetConnection* connection = m_incomingPacketQueue[ index ].m_connection;

			if ( VerifyPacket( *packet ) )
			{
				if ( connection != nullptr )
				{
					connection->OnReceivePacket( packet );
				}
				
				packet->ResetRead();

				NetPacketHeader packetHeader;
				if ( packet->ReadHeader( &packetHeader ) )
				{
					uint8_t senderIndex = packetHeader.m_senderConnectionIndex;
					uint8_t numMessages = packetHeader.m_messageCount;

					while( numMessages > 0U )
					{
						NetMessageHeader messageHeader;
						if ( !packet->ReadMessageHeader( &messageHeader ) )
						{
							ConsolePrintf( Rgba::RED, "NetSession::ProcessIncoming(): NetMessage: Invalid header." );
							break;
						}

						NetMessageDefinition* definition = GetMessageDefinition( messageHeader.m_messageIndex );

						NetMessage message( messageHeader.m_messageIndex, false );
						message.m_reliableID = messageHeader.m_reliableID;
						message.m_sequenceID = messageHeader.m_sequenceID;
						packet->ReadMessage( ( messageHeader.m_messageLength - NetMessage::GetHeaderNumBytesMinusLength( *definition ) ), &message );

						if ( messageHeader.m_messageIndex < static_cast< uint8_t >( g_messageDefinitions.size() ) )
						{
							if ( connection->ShouldProcessMessage( message ) )
							{
								connection->OnReceiveMessage( message );

								NetMessageDefinition& messageDefinition = g_messageDefinitions[ messageHeader.m_messageIndex ];
								bool isConnectionValid = ( GetConnectionWithAddress( connection->m_address ) != nullptr ) || ( messageDefinition.m_options & NETMSG_OPTION_CONNECTIONLESS );
								if ( !isConnectionValid )
								{
									LogWarningf( "NetSession::ProcessIncomingPacketQueue(): Invalid connection for %s message. Skipping...", messageDefinition.m_name.c_str() );
								}
								else if ( messageDefinition.m_options & NETMSG_OPTION_CONNECTIONLESS ) // Only call connectionless messages' callbacks here - NetConnection::OnReceiveMessage() will take care of them otherwise
								{
									messageDefinition.m_callback( message, *connection );
								}
							}
						}
						else
						{
							LogErrorf( "NetSession::ProcessIncomingPacketQueue(): Invalid message index received. Aborting..." );
						}

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

			if ( connection != nullptr && connection->m_connectionIndex == SESSION_MAX_CONNECTIONS )
			{
				delete connection;	// Not an existing connection, but it was allocated in ProcessIncoming()
			}

			delete packet;

			// Fast removal
			m_incomingPacketQueue[ index ] = m_incomingPacketQueue[ m_incomingPacketQueue.size() - 1 ];
			m_incomingPacketQueue.erase( m_incomingPacketQueue.begin() + index );
			index--;
		}
	}
}

void NetSession::UpdateNetTime()
{
	m_netClock->BeginFrame();

	if ( IsHost() )
	{
		return;
	}

	float deltaSeconds = m_netClock->GetDeltaSecondsF();
	float deltaMS = deltaSeconds * 1000.0f;

	if ( IsFloatEqualTo( deltaMS, 0.0f ) )	// Divide by zero
	{
		return;
	}

	m_desiredClientTimeMS += deltaMS;
	float deltaMSRatio = Max( (m_desiredClientTimeMS - m_currentClientTimeMS), 0.0f ) / deltaMS;
	float deltaMSCurrent = ( m_currentClientTimeMS + deltaMS > m_desiredClientTimeMS )?
		( deltaMS * Max( deltaMSRatio, 1.0f - NET_MAX_TIME_DILATION ) ) :
		( deltaMS * Min( deltaMSRatio, 1.0f + NET_MAX_TIME_DILATION ) );

	m_currentClientTimeMS += deltaMSCurrent;
}

void NetSession::ProcessOutgoing()
{
	for ( uint8_t connectionIndex = 0U; connectionIndex < SESSION_MAX_CONNECTIONS; connectionIndex++ )
	{
		if ( m_boundConnections[ connectionIndex ] != nullptr )
		{
			m_boundConnections[ connectionIndex ]->ProcessOutgoing();
		}
	}
}

bool NetSession::Host( const char* myID, uint16_t port, uint16_t portRange /* = DEFAULT_PORT_RANGE */ )
{
	if ( m_state != SESSION_DISCONNECTED )
	{
		LogWarningf( "NetSession::Host(): attempting to host while not disconnected. Aborting..." );
		return false;
	}

	uint16_t boundPort = AddBinding( port, portRange );
	if ( boundPort == 0U )
	{
		SetError( SESSION_ERROR_COULD_NOT_BIND, "Unable to bind to socket" );
		LogWarningf( "NetSession::Host(): unable to bind to %u with range %u. Aborting...", port, portRange );
		return false;
	}

	NetConnectionInfo connInfo;
	connInfo.m_address = NetAddressIPv4::GetLocal();
	connInfo.m_address.m_port = boundPort;
	connInfo.m_connectionIndex = 0U;
	connInfo.SetID( myID );
	NetConnection* connection = CreateConnection( connInfo );
	
	m_myConnection = connection;
	m_hostConnection = connection;
	m_hostConnection->SetState( CONNECTION_READY );
	m_state = SESSION_READY;
	return true;
}

void NetSession::Join( const char* myID, const NetConnectionInfo& hostInfo )
{
	uint16_t boundPort = AddBinding( SESSION_DEFAULT_PORT, SESSION_DEFAULT_PORT_RANGE );
	if ( boundPort == 0U )
	{
		SetError( SESSION_ERROR_COULD_NOT_BIND, "Unable to bind to socket" );
		LogWarningf( "NetSession::Join(): unable to bind to %u with range %u. Aborting...", SESSION_DEFAULT_PORT, SESSION_DEFAULT_PORT_RANGE );
		return;
	}

	m_hostConnection = CreateConnection( hostInfo );

	NetConnectionInfo myInfo;
	myInfo.m_address = NetAddressIPv4::GetLocal();
	myInfo.m_address.m_port = boundPort;
	myInfo.SetID( myID );
	m_myConnection = CreateConnection( myInfo );	// Will not bind yet
	m_myConnection->SetState( CONNECTION_JOINING );

	m_state = SESSION_CONNECTING;
}

void NetSession::Disconnect()
{
	for ( uint8_t connIndex = 0U; connIndex < SESSION_MAX_CONNECTIONS; connIndex++ )
	{
		if ( m_boundConnections[ connIndex ] != nullptr )
		{
			if ( m_boundConnections[ connIndex ] != m_myConnection )
			{
				m_boundConnections[ connIndex ]->HangUp();
			}
			
			DestroyConnection( m_boundConnections[ connIndex ] );
			m_boundConnections[ connIndex ] = nullptr;
		}
	}
	m_allConnections.clear();

	m_myConnection = nullptr;
	m_hostConnection = nullptr;

	RemoveBinding();

	m_state = SESSION_DISCONNECTED;
}

#endif	// #if !defined( ENGINE_DISABLE_NETWORKING )
