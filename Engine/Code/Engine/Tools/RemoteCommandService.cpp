#include "Engine/Core/BytePacker.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Logger.hpp"
#include "Engine/Networking/NetAddress.hpp"
#include "Engine/Networking/TCPSocket.hpp"
#include "Engine/Tools/DevConsole.hpp"
#include "Engine/Tools/RemoteCommandService.hpp"

#if !defined( ENGINE_DISABLE_NETWORKING )

RemoteCommandService* g_theRemoteCommandService = nullptr;

/* THREADED */
ThreadSafePrimitive< bool > g_rcsShouldQuit;
void RemoteCommandServerWork( void* params )
{
	g_rcsShouldQuit.Set( false );
	TCPSocket* serverSocket = reinterpret_cast< TCPSocket* >( params );

	while( !g_rcsShouldQuit.Get() )
	{
		// Accept connections
		TCPSocket* otherSocket = serverSocket->Accept();
		if ( otherSocket != nullptr )
		{
			NetAddressIPv4 clientAddr = otherSocket->m_netAddress;
			std::string clientAddrStr = clientAddr.ToString();
			LogPrintf( "[SERVER] RemoteCommandService: Client connected: %s.", clientAddrStr.c_str() );

			g_theRemoteCommandService->m_clients.Push( otherSocket );
		}

		// Validate existing connections
		g_theRemoteCommandService->ValidateAndUpdateSocketsToClients();

		ThreadSleep( 1U );
	}
}

void RemoteCommandClientWork( void* params )
{
	g_rcsShouldQuit.Set( false );
	TCPSocket* clientSocket = reinterpret_cast< TCPSocket* >( params );

	while( !g_rcsShouldQuit.Get() )
	{
		bool isSocketValid = ( !g_theRemoteCommandService->m_socket.Get()->IsClosed() );
		if ( !isSocketValid )
		{
			g_rcsShouldQuit.Set( true );
		}
		else
		{
			g_theRemoteCommandService->UpdateSocketToHost();
		}

		ThreadSleep( 1U );
	}
}

void EchoConsoleLog( const std::string& logOutput, void* socketInfo )
{
	TCPSocket* socket = reinterpret_cast< TCPSocket* >( socketInfo );

	bool isEcho = true;
	BytePacker message = BytePacker( Endianness::BIG_ENDIAN, ( BYTEPACKER_OWNS_MEMORY | BYTEPACKER_CAN_GROW ) );
	message.WriteBytes( sizeof( bool ), &isEcho );
	message.WriteString( logOutput.c_str() );

	size_t messageLength = message.GetWrittenByteCount();
	if ( messageLength > 0xffff )	// Length of an unsigned short
	{
		LogErrorf( "EchoConsoleLog(): Message length too long to fit into an unsigned short." );
		return;
	}

	uint16_t usLength = static_cast< uint16_t >( messageLength );
	ToEndianness( sizeof( usLength ), &usLength, Endianness::BIG_ENDIAN );

	socket->Send( &usLength, sizeof( usLength ) );
	socket->Send( message.m_buffer, messageLength );
}

RemoteCommandService::RemoteCommandService()
{
	m_socket.Set( nullptr );
	m_mode.Set( RCSMode::RCS_MODE_INVALID );
	m_shouldProcessEcho.Set( true );
}

RemoteCommandService::~RemoteCommandService()
{
	TryShutdown();
}

bool RemoteCommandService::TryShutdown()
{
	if ( g_theRemoteCommandService->GetMode() == RCSMode::RCS_MODE_SERVER )
	{
		g_theRemoteCommandService->ShutdownServer();
		return true;
	}
	else if ( g_theRemoteCommandService->GetMode() == RCSMode::RCS_MODE_CLIENT )
	{
		g_theRemoteCommandService->ShutdownClient();
		return true;
	}

	return false;
}

void RemoteCommandService::Update()
{
	if ( m_mode.Get() == RCSMode::RCS_MODE_INVALID && IsFloatLesserThanOrEqualTo( m_cooldownTimeCurrent, 0.0f ) )
	{
		LogPrintf( "RemoteCommandService: Not in valid state! Trying to join local server..." );

		NetAddressIPv4 localAddr = NetAddressIPv4::GetBindableLocal( static_cast< uint16_t >( RCS_DEFAULT_PORT_NUMBER ) );
		bool success = StartClient( localAddr );
		if ( !success )
		{
			LogPrintf( "RemoteCommandService: Failed to join local server! Trying to create local server at default port..." );
			success = StartServer();
			if ( !success )
			{
				LogPrintf( "RemoteCommandService: Failed to create local server! Will try again after %.2f seconds...", RCS_RETRY_TIME_SECONDS );
				m_cooldownTimeCurrent = RCS_RETRY_TIME_SECONDS;
			}
		}
	}

	if ( m_cooldownTimeCurrent > 0.0f )
	{
		m_cooldownTimeCurrent -= GetMasterDeltaSecondsF();
	}
}

bool RemoteCommandService::StartServer( unsigned int port /* = RCS_DEFAULT_PORT_NUMBER */ )
{
	if ( !IsRunning() )
	{
		bool success = InitializeSocket( port );
		return success;
	}
	return false;
}

bool RemoteCommandService::InitializeSocket( unsigned int port )
{
	TCPSocket* newSocket = new TCPSocket();
	m_socket.Set( newSocket );

	bool success = m_socket.Get()->Listen(
		static_cast< uint16_t >( port ),
		32
	);

	if ( success )
	{
		m_workerThread = ThreadCreate( RemoteCommandServerWork, m_socket.Get() );
		m_mode.Set( RCSMode::RCS_MODE_SERVER );
		LogPrintf( "[ SERVER ] RemoteCommandService: Listening on port %u.", port );
	}
	else
	{
		delete m_socket.Get();
		m_socket.Set( nullptr );
		LogErrorf( "RemoteCommandService: Failed to start on port %u.", port );
	}

	return success;
}

void RemoteCommandService::ShutdownServer()
{
	if ( IsRunning() )
	{
		g_theRemoteCommandService->CloseClientSockets();

		LogPrintf( "[CLIENT] RemoteCommandService: Attempting to shut down server..." );
		m_mode.Set( RCSMode::RCS_MODE_INVALID );

		g_rcsShouldQuit.Set( true );
		ThreadJoin( m_workerThread );
		m_workerThread = nullptr;

		m_socket.Get()->Close();
		delete m_socket.Get();
		m_socket.Set( nullptr );

		LogPrintf( "RemoteCommandService: Server shut down gracefully." );
	}
}

bool RemoteCommandService::StartClient( const NetAddressIPv4& serverAddr )
{
	TCPSocket* newSocket = new TCPSocket();
	m_socket.Set( newSocket );
	bool success = m_socket.Get()->Connect( serverAddr );
	if ( success )
	{
		m_workerThread = ThreadCreate( RemoteCommandClientWork, &m_socket );
		m_mode.Set( RCSMode::RCS_MODE_CLIENT );
		std::string serverAddrStr = serverAddr.ToString();
		LogPrintf( "[CLIENT] RemoteCommandService: Successfully joined %s as client.", serverAddrStr.c_str() );
	}
	else
	{
		delete m_socket.Get();
		m_socket.Set( nullptr );
		std::string serverAddrStr = serverAddr.ToString();
		LogPrintf( "RemoteCommandService: Could not join %s as client.", serverAddrStr.c_str() );
	}

	return success;
}

void RemoteCommandService::ShutdownClient()
{
	if ( IsRunning() )
	{
		LogPrintf( "[CLIENT] RemoteCommandService: Attempting to shut down client..." );
		m_mode.Set( RCSMode::RCS_MODE_INVALID );

		g_rcsShouldQuit.Set( true );
		ThreadJoin( m_workerThread );
		m_workerThread = nullptr;

		m_socket.Get()->Close();
		delete m_socket.Get();
		m_socket.Set( nullptr );

		LogPrintf( "RemoteCommandService: Client shut down gracefully." );
	}
}

void RemoteCommandService::ValidateAndUpdateSocketsToClients()
{
	if ( m_mode.Get() == RCSMode::RCS_MODE_SERVER )
	{
		for ( unsigned int clientIndex = 0U; clientIndex < m_clients.Size(); clientIndex++ )
		{
			TCPSocket* currentSocket = nullptr;
			bool success = m_clients.Get( clientIndex, &currentSocket );
			if ( !success )
			{
				// clientIndex is >= m_clients.size()
				break;
			}
			if ( success )
			{
				if ( currentSocket->IsClosed() )
				{
					m_clients.Erase( currentSocket );
					currentSocket->Close();
					delete currentSocket;
					currentSocket = nullptr;
				}
				else
				{
					ProcessConnection( currentSocket );
				}
			}
		}
	}
}

void RemoteCommandService::UpdateSocketToHost()
{
	if ( m_mode.Get() == RCSMode::RCS_MODE_CLIENT )
	{
		TCPSocket* socket = m_socket.Get();
		if ( !socket->IsClosed() )
		{
			ProcessConnection( socket );
		}
	}
}

void RemoteCommandService::CloseClientSockets()
{
	if ( m_mode.Get() == RCSMode::RCS_MODE_SERVER )
	{
		while ( m_clients.Size() > 0 )
		{
			TCPSocket* socket = nullptr;
			bool success = m_clients.Get( 0, &socket );
			if ( success )
			{
				m_clients.Erase( socket );
				socket->Close();
				delete socket;
			}
			else
			{
				break;
			}
		}
	}
}

bool RemoteCommandService::SendCommand( int clientIndex, bool isEcho, const std::string& commandStr )
{
	if ( !IsRunning() )
	{
		LogErrorf( "RemoteCommandService::SendCommand(): Cannot send message as RCS is in an invalid state." );
		return false;
	}

	TCPSocket* socket = nullptr;
	if ( m_mode.Get() == RCSMode::RCS_MODE_SERVER )
	{
		if ( clientIndex >= m_clients.Size() )
		{
			LogErrorf( "[SERVER] RemoteCommandService::SendCommand(): Provided client index is invalid." );
			return false;
		}

		if ( !m_clients.Get( clientIndex, &socket ) )
		{
			LogErrorf( "[SERVER] RemoteCommandService::SendCommand(): Could not send to client at provided index." );
			return false;
		}
	}
	else
	{
		socket = m_socket.Get();
	}

	BytePacker message = BytePacker( Endianness::BIG_ENDIAN, ( BYTEPACKER_OWNS_MEMORY | BYTEPACKER_CAN_GROW ) );
	message.WriteBytes( sizeof( bool ), &isEcho );
	message.WriteString( commandStr.c_str() );

	size_t messageLength = message.GetWrittenByteCount();
	if ( messageLength > 0xffff )	// Length of an unsigned short
	{
		LogErrorf( "RemoteCommandService::SendCommand(): Message length too long to fit into an unsigned short." );
		return false;
	}

	uint16_t usLength = static_cast< uint16_t >( messageLength );
	ToEndianness( sizeof( usLength ), &usLength, Endianness::BIG_ENDIAN );

	bool success = true;
	success = socket->Send( &usLength, sizeof( usLength ) );
	success = socket->Send( message.m_buffer, messageLength );

	return success;
}

bool RemoteCommandService::BroadcastCommand( const std::string& commandStr )
{
	if ( !IsRunning() )
	{
		LogErrorf( "RemoteCommandService::BroadcastCommand(): Cannot send message as RCS is in an invalid state." );
		return false;
	}

	bool success = true;

	if ( m_mode.Get() == RCSMode::RCS_MODE_SERVER )
	{
		for ( size_t clientIndex = 0; clientIndex < m_clients.Size(); clientIndex++ )
		{
			success = success && SendCommand( static_cast< int >( clientIndex ), false, commandStr );
			if ( !success )
			{
				LogErrorf( "ERROR: [SERVER] RemoteCommandService::BroadcastCommand(): Failed to broadcast." );
			}
		}
	}
	else
	{
		success = SendCommand( 0, false, commandStr );
		if ( !success )
		{
			LogErrorf( "ERROR: [CLIENT] RemoteCommandService::BroadcastCommand(): Failed to broadcast." );
		}
	}

	return success;
}

void RemoteCommandService::ProcessConnection( TCPSocket* socket )
{
	BytePacker* buffer = socket->GetBuffer();
	if ( buffer->m_buffer == nullptr )
	{
		buffer->TryInitAndGrow( 2 );
	}

	if ( buffer->GetWrittenByteCount() < 2 )
	{
		// Wait till the size of the message is received
		size_t read = socket->Receive( buffer->GetWriteHead(), 2 - buffer->GetWrittenByteCount() );
		buffer->AdvanceWriteHead( read );
	}

	bool isReadyToProcess = false;
	if ( buffer->GetWrittenByteCount() >= 2 )
	{
		uint16_t bufferLength = 0;
		if ( buffer->Peek< uint16_t >( &bufferLength ) )
		{
			FromEndianness( sizeof( bufferLength ), &bufferLength, Endianness::BIG_ENDIAN );
			size_t bytesNeeded = static_cast< size_t >( bufferLength ) + 2 - buffer->GetWrittenByteCount();
			if ( bytesNeeded > 0 )
			{
				size_t bytesAvailable = buffer->m_bufferSize - buffer->GetWrittenByteCount();
				if ( bytesNeeded > bytesAvailable )
				{
					buffer->TryInitAndGrow( bytesNeeded - bytesAvailable );
				}
				size_t read = socket->Receive( buffer->GetWriteHead(), bytesNeeded );
				buffer->AdvanceWriteHead( read );
				bytesNeeded -= read;
			}

			isReadyToProcess = ( bytesNeeded == 0 );
		}
	}

	if ( isReadyToProcess )
	{
		buffer->AdvanceReadHead( 2U );	// Get past the message size
		ProcessMessage( socket, buffer );
		buffer->Clear();
	}
}

void RemoteCommandService::ProcessMessage( TCPSocket* socket, BytePacker* payload )
{
	bool isEcho = true;
	payload->ReadBytes( &isEcho, sizeof( isEcho ) );

	size_t readSize = payload->GetReadableByteCount() + 1;
	char* command = new char[ readSize ];
	size_t read = payload->ReadString( command, readSize - 1 );
	command[ read ] = '\0';

	if ( read > 0 )
	{
		if ( isEcho )
		{
			if ( m_shouldProcessEcho.Get() )
			{
				std::string combinedString = std::string( "ECHO: " ) + std::string( command );
				ConsolePrintf( combinedString.c_str() );
			}
		}
		else
		{
			DevConsoleHook( EchoConsoleLog, socket );
			CommandRun( command, true );
			DevConsoleUnhook( EchoConsoleLog, socket );
		}
	}

	if ( readSize > 0 )
	{
		delete[] command;
	}
}

void RemoteCommandService::SetEchoState( bool echoState )
{
	m_shouldProcessEcho.Set( echoState );
}

bool RemoteCommandService::IsRunning()
{
	return ( m_socket.Get() != nullptr && m_workerThread != nullptr && m_mode.Get() != RCSMode::RCS_MODE_INVALID );
}

std::string RemoteCommandService::GetAddressIPv4String()
{
	if ( IsRunning() )
	{
		NetAddressIPv4 ipv4 = m_socket.Get()->m_netAddress;
		return ipv4.ToString();
	}
	else
	{
		return "NO_ADDR";
	}
}

unsigned int RemoteCommandService::GetMaxNumClients() const
{
	return RCS_MAX_CLIENTS;
}

unsigned int RemoteCommandService::GetNumClients()
{
	return static_cast< unsigned int >( m_clients.Size() );
}

std::string RemoteCommandService::GetClientAddressIPv4String( unsigned int clientIndex )
{
	if ( IsRunning() && clientIndex < m_clients.Size() )
	{
		TCPSocket* client = nullptr;
		bool success = m_clients.Get( clientIndex, &client );
		if ( success )
		{
			NetAddressIPv4 ipv4 = client->m_netAddress;
			return ipv4.ToString();
		}
	}

	return "NO_ADDR";
}

RCSMode RemoteCommandService::GetMode()
{
	return m_mode.Get();
}

void RemoteCommandService::SetMode( RCSMode mode )
{
	m_mode.Set( mode );
}

/* static */
RemoteCommandService* RemoteCommandService::GetInstance()
{
	return g_theRemoteCommandService;
}

/* STANDALONE FUNCTIONS */
void RemoteCommandServiceStartup()
{
	g_theRemoteCommandService = new RemoteCommandService();
	CommandRegister( "rc_host", RCSHostCommand, "Starts (or restarts) the Remote Command Service on the specified port.", true );
	CommandRegister( "rc_join", RCSJoinCommand, "Joins a Remote Command Service daemon running on the (IPv4) address specified.", true );
	CommandRegister( "rc_echo", RCSEchoStateCommand, "Enables or disabled Remote Command echo for this (client) instance.", true );
	CommandRegister( "rc", RCSSendCommand, "Sends a message to a specified client, or to host.", true );
	CommandRegister( "rca", RCSAllCommand, "Sends a message to all connected clients, and runs it locally.", true );
	CommandRegister( "rcb", RCSBroadcastCommand, "Sends a message to all connected clients, but doesn't run it locally.", true );
}

void RemoteCommandServiceUpdate()
{
	g_theRemoteCommandService->Update();
}

void RemoteCommandServiceShutdown()
{
	CommandUnregister( "rc_host" );
	CommandUnregister( "rc_join" );
	CommandUnregister( "rc_echo" );
	CommandUnregister( "rc" );
	CommandUnregister( "rca" );
	CommandUnregister( "rcb" );

	delete g_theRemoteCommandService;
	g_theRemoteCommandService = nullptr;
}

bool RCSHostCommand( Command& command )
{
	if ( command.GetName() == "rc_host" )
	{
		std::string portNumString = command.GetNextString();
		unsigned int portNumber = RCS_DEFAULT_PORT_NUMBER;
		if ( portNumString != "" )
		{
			try
			{
				int portNumInt = stoi( portNumString );
				if ( portNumInt >= 0 )
				{
					portNumber = static_cast< unsigned int >( portNumInt );
				}
				else
				{
					ConsolePrintf( Rgba::RED, "ERROR: rc_host: Port number cannot be negative." );
					return false;
				}
			}
			catch ( std::invalid_argument& arg )
			{
				UNUSED( arg );
				ConsolePrintf( Rgba::RED, "ERROR: rc_host: Invalid port number provided." );
				return false;
			}
		}
		else
		{
			ConsolePrintf( Rgba::YELLOW, "rc_host: No port number provided. Defaulting to %u.", RCS_DEFAULT_PORT_NUMBER );
		}

		g_theRemoteCommandService->TryShutdown();
		g_theRemoteCommandService->StartServer( portNumber );

		return true;
	}

	return false;
}

bool RCSJoinCommand( Command& command )
{
	if ( command.GetName() == "rc_join" )
	{
		NetAddressIPv4 netAddr;
		std::string netAddrStr = command.GetNextString();
		if ( netAddrStr != "" )
		{
			netAddr.FromString( netAddrStr );
			if ( !NetAddressIPv4::IsValid( netAddrStr.c_str() ) || !netAddr.IsSet() )
			{
				ConsolePrintf( Rgba::RED, "ERROR: rc_join: Invalid join address provided." );
				return false;
			}

			if ( netAddr.m_port == 0U )
			{
				netAddr.m_port = RCS_DEFAULT_PORT_NUMBER;
				ConsolePrintf( Rgba::YELLOW, "rc_join: No port number provided. Defaulting to %u.", RCS_DEFAULT_PORT_NUMBER );
			}
		}
		else
		{
			netAddr.m_addressIPv4[ 0 ] = 127;	netAddr.m_addressIPv4[ 1 ] = 0;	netAddr.m_addressIPv4[ 2 ] = 0;	netAddr.m_addressIPv4[ 3 ] = 1;
			netAddr.m_port = RCS_DEFAULT_PORT_NUMBER;
			ConsolePrintf( Rgba::YELLOW, "rc_join: No join address provided. Defaulting to 127.0.0.1:%u.", RCS_DEFAULT_PORT_NUMBER );
		}

		g_theRemoteCommandService->TryShutdown();
		bool success = g_theRemoteCommandService->StartClient( netAddr );
		if ( success )
		{
			ConsolePrintf( Rgba::GREEN, "rc_join: Successfully connected to %s as a client.", netAddrStr.c_str() );
		}
		else
		{
			ConsolePrintf( Rgba::RED, "rc_join: Could not join %s as a client.", netAddrStr.c_str() );
			return false;
		}

		return true;
	}

	return false;
}

bool RCSSendCommand( Command& rcsCommand )
{
	if ( rcsCommand.GetName() == "rc" )
	{
		int index = 0;
		std::string indexStr = rcsCommand.GetNextString();
		std::string commandStr = "";
		std::string partialCommandStr = "";
		if ( indexStr != "" )
		{
			try
			{
				index = stoi( indexStr );
				if ( index < 0 )
				{
					ConsolePrintf( Rgba::RED, "ERROR: rc: Invalid client index entered." );
					return false;
				}

				partialCommandStr = rcsCommand.GetNextString();
				while ( partialCommandStr != "" )
				{
					if ( commandStr != "" )
					{
						commandStr += " ";
					}
					commandStr += partialCommandStr;
					partialCommandStr = rcsCommand.GetNextString();
				}
				if ( commandStr == "" )
				{
					ConsolePrintf( Rgba::RED, "ERROR: rc: No command entered." );
					return false;
				}
			}
			catch ( std::invalid_argument& arg )
			{
				UNUSED( arg );
				ConsolePrintf( Rgba::YELLOW, "rc: No client index entered. Defaulting to 0." );
				partialCommandStr = indexStr;
				while ( partialCommandStr != "" )
				{
					if ( commandStr != "" )
					{
						commandStr += " ";
					}
					commandStr += partialCommandStr;
					partialCommandStr = rcsCommand.GetNextString();
				}
			}
		}
		else
		{
			ConsolePrintf( Rgba::RED, "ERROR: rc: No command entered." );
			return false;
		}

		if ( g_theRemoteCommandService->IsRunning() )
		{
			bool sendSuccess = g_theRemoteCommandService->SendCommand( index, false, commandStr );
			if ( !sendSuccess )
			{
				return false;
			}
		}
		else
		{
			ConsolePrintf( Rgba::RED, "ERROR: rc: Remote Command Service not running." );
			return false;
		}

		return true;
	}

	return false;
}

bool RCSAllCommand( Command& rcsCommand )
{
	if ( rcsCommand.GetName() == "rca" )
	{
		std::string commandStr = "";
		std::string partialCommandStr = rcsCommand.GetNextString();
		while ( partialCommandStr != "" )
		{
			if ( commandStr != "" )
			{
				commandStr += " ";
			}
			commandStr += partialCommandStr;
			partialCommandStr = rcsCommand.GetNextString();
		}
		if ( commandStr == "" )
		{
			ConsolePrintf( Rgba::RED, "ERROR: rca: No command entered." );
			return false;
		}

		if ( g_theRemoteCommandService->IsRunning() )
		{
			bool sendSuccess = g_theRemoteCommandService->BroadcastCommand( commandStr );
			if ( !sendSuccess )
			{
				return false;
			}
			else
			{
				CommandRun( commandStr.c_str() );
			}
		}
		else
		{
			ConsolePrintf( Rgba::RED, "ERROR: rca: Remote Command Service not running." );
			return false;
		}
		return true;
	}
	else
	{
		return false;
	}
}

bool RCSBroadcastCommand( Command& rcsCommand )
{
	if ( rcsCommand.GetName() == "rcb" )
	{
		std::string commandStr = "";
		std::string partialCommandStr = rcsCommand.GetNextString();
		while ( partialCommandStr != "" )
		{
			if ( commandStr != "" )
			{
				commandStr += " ";
			}
			commandStr += partialCommandStr;
			partialCommandStr = rcsCommand.GetNextString();
		}
		if ( commandStr == "" )
		{
			ConsolePrintf( Rgba::RED, "ERROR: rcb: No command entered." );
			return false;
		}

		if ( g_theRemoteCommandService->IsRunning() )
		{
			bool sendSuccess = g_theRemoteCommandService->BroadcastCommand( commandStr );
			if ( !sendSuccess )
			{
				return false;
			}
		}
		else
		{
			ConsolePrintf( Rgba::RED, "ERROR: rca: Remote Command Service not running." );
			return false;
		}
		return true;
	}
	else
	{
		return false;
	}
}

bool RCSEchoStateCommand( Command& rcsEchoCommand )
{
	if ( rcsEchoCommand.GetName() == "rc_echo" )
	{
		bool echoEnabled = true;
		std::string echoString = rcsEchoCommand.GetNextString();
		if ( echoString != "" )
		{
			if ( echoString == "Enabled" || echoString == "enabled" || echoString == "True" || echoString == "true" )
			{
				echoEnabled = true;	// Just for readability
			}
			else if ( echoString == "Disabled" || echoString == "disabled" || echoString == "False" || echoString == "false" )
			{
				echoEnabled = false;
			}
			else
			{
				ConsolePrintf( Rgba::RED, "ERROR: rc_echo: Invalid echo state command. Enter \"enabled\" or \"disabled\"." );
				return false;
			}
		}

		g_theRemoteCommandService->SetEchoState( echoEnabled );
		return true;
	}

	return false;
}

#endif	// #if !defined( ENGINE_DISABLE_NETWORKING )
