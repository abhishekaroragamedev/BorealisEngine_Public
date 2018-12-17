#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/Threading.hpp"
#include "Engine/Networking/NetAddress.hpp"
#include "Engine/Networking/Networking.hpp"
#include "Engine/Networking/NetSession.hpp"
#include "Engine/Networking/TCPSocket.hpp"
#include "Engine/Tools/DevConsole.hpp"
#include "Game/EngineBuildPreferences.hpp"

#if !defined( ENGINE_DISABLE_NETWORKING )

#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>	// Must be defined before <windows.h>
#include <WS2tcpip.h>	// Only needed for IPv6 Support
#include <windows.h>

/* static */
bool Networking::Startup()
{
	WORD version = MAKEWORD( 2, 2 );

	WSADATA data;									// Stores information about the system
	int32_t socketError = ::WSAStartup( version, &data );	// Initialize WinSockAPI (WSA)

	bool success = socketError == 0;
	ASSERT_RECOVERABLE( success, "ERROR: Networking::Startup(): Startup failed!" );

	CommandRegister( "host", NetHostCommand, "Host a session at the specified port." );
	CommandRegister( "join", NetJoinCommand, "Join a session at the specified IPv4 address." );
	CommandRegister( "net_address", NetworkLogIPv4Address, "Displays the local IPv4 address of the machine." );
	CommandRegister( "net_address_host", NetworkLogAddressForHost, "Displays the IPv4 address of the host for the port number provided." );
	CommandRegister( "net_connect_test", NetworkTestConnectToIP, "Connects to the IP and port number provided." );
	CommandRegister( "net_connect_host_test", NetworkTestConnectToHost, "Connects to the host and port number provided." );
	CommandRegister( "net_host_test", NetworkHostTest, "Hosts a server at the port number provided." );

	NetworkLogIPv4Address( Command( "net_address" ) );

	return success;
}

/* static */
bool Networking::Shutdown()
{
	CommandUnregister( "host" );
	CommandUnregister( "join" );
	CommandUnregister( "net_address" );
	CommandUnregister( "net_address_host" );
	CommandUnregister( "net_connect_test" );
	CommandUnregister( "net_connect_host_test" );
	CommandUnregister( "net_host_test" );

	::WSACleanup();
	return true;
}

bool NetHostCommand( Command& hostCommand )
{
	if ( hostCommand.GetName() == "host" )
	{
		int port = static_cast< int >( SESSION_DEFAULT_PORT );
		std::string portString = hostCommand.GetNextString();
		if ( portString != "" )
		{
			try
			{
				port = stoi( portString );
			}
			catch ( std::invalid_argument& args )
			{
				UNUSED( args );
				ConsolePrintf( Rgba::RED, "ERROR: host: invalid port number provided." );
				return false;
			}

			if ( port < 0 )
			{
				ConsolePrintf( Rgba::RED, "ERROR: host: invalid port number provided." );
				return false;
			}
		}

		int portRange = 16;
		std::string portRangeString = hostCommand.GetNextString();
		if ( portRangeString != "" )
		{
			try
			{
				portRange = stoi( portRangeString );
			}
			catch ( std::invalid_argument& args )
			{
				UNUSED( args );
				ConsolePrintf( Rgba::RED, "ERROR: host: invalid port range provided." );
				return false;
			}

			if ( portRange < 0 )
			{
				ConsolePrintf( Rgba::RED, "ERROR: host: invalid port range provided." );
				return false;
			}
		}

		NetSession::GetInstance()->Host( "Host", static_cast< uint16_t >( port ), static_cast< uint16_t >( portRange ) );
		return true;
	}

	return false;
}

bool NetJoinCommand( Command& joinCommand )
{
	if ( joinCommand.GetName() == "join" )
	{
		std::string ipString = joinCommand.GetNextString();
		if ( ipString == "" )
		{
			ConsolePrintf( Rgba::RED, "ERROR: join: No IPv4 address provided." );
			return false;
		}
		if ( !NetAddressIPv4::IsValid( ipString.c_str() ) )
		{
			ConsolePrintf( Rgba::RED, "ERROR: join: Invalid IPv4 address provided." );
			return false;
		}

		NetAddressIPv4 addr( ipString.c_str() );
		NetConnectionInfo connectionInfo;
		connectionInfo.m_address = addr;

		NetSession::GetInstance()->Join( "Client", connectionInfo );
		return true;
	}

	return false;
}

bool NetworkTestConnectToIP( Command& connectCommand )
{
	if ( connectCommand.GetName() == "net_connect_test" )
	{
		std::string ipv4String = connectCommand.GetNextString();
		if ( ipv4String == "" )
		{
			LogTaggedPrintf( "net_connect_test", "ERROR: net_connect: No IP and port number provided." );
			return false;
		}

		std::string messageString = connectCommand.GetNextString();
		if ( messageString == "" )
		{
			messageString = "Ping";
		}

		NetAddressIPv4 netAddress = NetAddressIPv4( ipv4String.c_str() );
		if ( !netAddress.IsSet() )
		{
			LogTaggedPrintf( "net_connect_test", "ERROR: net_connect: Invalid IP or port number provided." );
			return false;
		}

		TCPSocket tcpSocket;
		LogTaggedPrintf( "net_connect_test", "Attempting to connect to host \"%s.\"", ipv4String.c_str() );
		if ( tcpSocket.Connect( netAddress ) )
		{
			LogTaggedPrintf( "net_connect_test", "Successfully connected to host \"%s.\"", ipv4String.c_str() );

			LogTaggedPrintf( "net_connect_test", "Sending data to host.", ipv4String.c_str() );
			tcpSocket.Send( messageString.c_str(), messageString.size() );

			char payload[ 256 ];
			LogTaggedPrintf( "net_connect_test", "Waiting to receive data from host...", ipv4String.c_str() );
			tcpSocket.Receive( payload, 256 - 1U );
			LogTaggedPrintf( "net_connect_test", "Received: %s", payload );

			tcpSocket.Close();
		}
		else
		{
			LogTaggedPrintf( "net_connect_test", "ERROR: Could not connect." );
			return false;
		}

		return true;
	}
	else
	{
		return false;
	}
}

bool NetworkTestConnectToHost( Command& connectCommand )
{
	if ( connectCommand.GetName() == "net_connect_host_test" )
	{
		std::string hostAndPortString = connectCommand.GetNextString();
		if ( hostAndPortString == "" )
		{
			LogTaggedPrintf( "net_connect_host_test", "ERROR: net_connect: No hostname and port number provided." );
			return false;
		}

		TokenizedString hostAndPortTokens = TokenizedString( hostAndPortString, ":" );
		std::string hostname = hostAndPortTokens.GetTokens()[ 0 ];
		std::string portString = "";
		if ( hostAndPortTokens.GetTokens().size() > 1 )
		{
			portString = hostAndPortTokens.GetTokens()[ 1 ];
		}
		else
		{
			LogTaggedPrintf( "net_connect_host_test", "Attempting to connect to host \"%s.\"", hostname.c_str() );
		}

		std::string messageString = connectCommand.GetNextString();
		if ( messageString == "" )
		{
			messageString = "Ping";
		}

		sockaddr ipv4;
		int addrLen = 0;
		GetAddressForHost( &ipv4, &addrLen, hostname.c_str(), portString.c_str() );
		NetAddressIPv4 netAddress = NetAddressIPv4( &ipv4 );

		TCPSocket tcpSocket;
		LogTaggedPrintf( "net_connect_host_test", "Attempting to connect to host \"%s.\"", hostAndPortString.c_str() );
		if ( tcpSocket.Connect( netAddress ) )
		{
			LogTaggedPrintf( "net_connect_host_test", "Successfully connected to host \"%s.\"", hostAndPortString.c_str() );

			LogTaggedPrintf( "net_connect_host_test", "Sending data to host.", hostAndPortString.c_str() );
			tcpSocket.Send( messageString.c_str(), messageString.size() );

			char payload[ 256 ];
			LogTaggedPrintf( "net_connect_host_test", "Waiting to receive data from host...", hostAndPortString.c_str() );
			tcpSocket.Receive( payload, 256 - 1U );
			LogTaggedPrintf( "net_connect_host_test", "Received: %s", payload );

			tcpSocket.Close();
		}
		else
		{
			LogTaggedPrintf( "net_connect_host_test", "ERROR: Could not connect." );
			return false;
		}

		return true;
	}
	else
	{
		return false;
	}
}

struct ServiceThreadData
{

public:
	TCPSocket* m_hostSocket = nullptr;
	TCPSocket* m_otherSocket = nullptr;

};

void ServiceThreadWork( void* socketData )
{
	ServiceThreadData* threadData = reinterpret_cast< ServiceThreadData* >( socketData );
	TCPSocket* host = threadData->m_hostSocket;
	TCPSocket* other = threadData->m_otherSocket;

	char buffer[ 1024 ];
	LogTaggedPrintf( "net_host_test", "Waiting to receive..." );
	size_t received = other->Receive( buffer, 1023 );
	if ( received > 0 )
	{
		//buffer[ received ] = '\0';
		LogTaggedPrintf( "net_host_test", "Received: %s", buffer );

		LogTaggedPrintf( "net_host_test", "Sending data." );
		other->Send( "Pong\0", 5 );
	}

	delete other;
	delete threadData;
}

void ServerWork( void* socketData )
{
	TCPSocket* host = reinterpret_cast< TCPSocket* >( socketData );
	while ( host->IsListening() )
	{
		TCPSocket* otherSocket = host->Accept();
		if ( otherSocket != nullptr )
		{
			LogTaggedPrintf( "net_host_test", "Connection accepted." );
			ServiceThreadData* threadData = new ServiceThreadData();
			threadData->m_hostSocket = host;
			threadData->m_otherSocket = otherSocket;
			ThreadCreateAndDetach( ServiceThreadWork, threadData );
		}
	}

	host->Close();
	delete host;
}

bool NetworkHostTest( Command& hostCommand )
{
	if ( hostCommand.GetName() == "net_host_test" )
	{
		std::string portNumber = hostCommand.GetNextString();
		uint16_t port;

		if ( portNumber == "" )
		{
			port = 12345;
			ConsolePrintf( Rgba::YELLOW, "net_host_test: Port number not provided. Defaulting to 12345." );
		}
		else
		{
			try
			{
				port = stoi( portNumber );
			}
			catch ( std::invalid_argument& argument )
			{
				UNUSED( argument );
				ConsolePrintf( Rgba::RED, "ERROR: net_host_test: Invalid port number provided." );
				return false;
			}
		}

		uint16_t maxQueued = 16;
		TCPSocket* host = new TCPSocket();
		if ( !host->Listen( port, maxQueued ) )
		{
			ConsolePrintf( Rgba::RED, "ERROR: net_host_test: Cannot host on the provided port %d.", port );
			return false;
		}

		ThreadCreateAndDetach( ServerWork, host );
		return true;
	}
	else
	{
		return false;
	}
}

#endif	// #if !defined( ENGINE_DISABLE_NETWORKING )
