#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Logger.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Networking/NetAddress.hpp"
#include "Engine/Tools/DevConsole.hpp"
#include "Game/EngineBuildPreferences.hpp"

#if !defined( ENGINE_DISABLE_NETWORKING )

NetAddressIPv4::NetAddressIPv4()
{

}

NetAddressIPv4::NetAddressIPv4( const sockaddr* socketAddress )
{
	FromSockAddr( socketAddress );
}

NetAddressIPv4::NetAddressIPv4( const char* ipv4String )
{
	FromString( ipv4String );
}

NetAddressIPv4::NetAddressIPv4( const NetAddressIPv4& copy )
{
	for ( unsigned int i = 0; i < 4U; i++ )
	{
		m_addressIPv4[ i ] = copy.m_addressIPv4[ i ];
	}
	m_port = copy.m_port;
}

bool NetAddressIPv4::operator==( const NetAddressIPv4& toCompare ) const
{
	return (
		( m_addressIPv4[ 0 ] == toCompare.m_addressIPv4[ 0 ] ) &&
		( m_addressIPv4[ 1 ] == toCompare.m_addressIPv4[ 1 ] ) &&
		( m_addressIPv4[ 2 ] == toCompare.m_addressIPv4[ 2 ] ) &&
		( m_addressIPv4[ 3 ] == toCompare.m_addressIPv4[ 3 ] ) &&
		( m_port == toCompare.m_port )
	);
}

bool NetAddressIPv4::ToSockAddr( sockaddr* out_address, size_t* out_addressLength ) const
{
	sockaddr_in ipv4;
	ipv4.sin_family = AF_INET;
	ipv4.sin_addr.S_un.S_un_b.s_b1 = m_addressIPv4[ 0 ];
	ipv4.sin_addr.S_un.S_un_b.s_b2 = m_addressIPv4[ 1 ];
	ipv4.sin_addr.S_un.S_un_b.s_b3 = m_addressIPv4[ 2 ];
	ipv4.sin_addr.S_un.S_un_b.s_b4 = m_addressIPv4[ 3 ];
	ipv4.sin_port = ::htons( m_port );	// Little Endian to Big Endian

	*out_addressLength = sizeof( sockaddr_in );
	memcpy( out_address, &ipv4, sizeof( sockaddr_in ) );

	return true;
}

bool NetAddressIPv4::FromSockAddr( const sockaddr* socketAddress )
{
	if ( socketAddress->sa_family != AF_INET )
	{
		return false;
	}

	const sockaddr_in* ipv4 = reinterpret_cast< const sockaddr_in* >( socketAddress );
	char ipv4AsString[ 256 ];
	::inet_ntop( ipv4->sin_family, &( ipv4->sin_addr ), ipv4AsString, 256 );

	memcpy( m_addressIPv4, &ipv4->sin_addr.S_un.S_addr, sizeof( unsigned int ) );
	m_port = ::ntohs( ipv4->sin_port );	// Big Endian to Little Endian

	return true;
}

std::string NetAddressIPv4::ToString() const
{
	return ( GetAddressAsString() + ":" + GetPortAsString() );
}

std::string NetAddressIPv4::GetAddressAsString() const
{
	std::string addressString = std::string(
		std::to_string( m_addressIPv4[ 0 ] ) + "." +
		std::to_string( m_addressIPv4[ 1 ] ) + "." +
		std::to_string( m_addressIPv4[ 2 ] ) + "." +
		std::to_string( m_addressIPv4[ 3 ] )
	);
	return addressString;
}

std::string NetAddressIPv4::GetPortAsString() const
{
	return std::to_string( m_port );
}

void NetAddressIPv4::FromString( const char* ipv4String )
{
	FromString( std::string( ipv4String ) );
}

void NetAddressIPv4::FromString( const std::string& ipv4String )
{
	TokenizedString ipv4Tokenized = TokenizedString( ipv4String, "." );
	if ( ipv4Tokenized.GetTokens().size() != 4 )
	{
		LogErrorf( "NetAddressIPv4::NetAddressIPv4(): Invalid IPv4 address: %s. Struct will remain unset.", ipv4String );
		return;
	}

	m_addressIPv4[ 0 ] = static_cast< unsigned char >( stoi( ipv4Tokenized.GetTokens()[ 0 ] ) );
	m_addressIPv4[ 1 ] = static_cast< unsigned char >( stoi( ipv4Tokenized.GetTokens()[ 1 ] ) );
	m_addressIPv4[ 2 ] = static_cast< unsigned char >( stoi( ipv4Tokenized.GetTokens()[ 2 ] ) );

	TokenizedString ipv4PortTokenized = TokenizedString( ipv4Tokenized.GetTokens()[ 3 ], ":" );
	m_addressIPv4[ 3 ] = static_cast< unsigned char >( stoi( ipv4PortTokenized.GetTokens()[ 0 ] ) );
	if ( ipv4PortTokenized.GetTokens().size() > 1 )
	{
		m_port = static_cast< uint16_t >( stoi( ipv4PortTokenized.GetTokens()[ 1 ] ) );
	}
}

bool NetAddressIPv4::IsSet() const
{
	return (
		( m_port != 0U ) ||
		( m_addressIPv4[ 0 ] != 0 ) ||
		( m_addressIPv4[ 1 ] != 0 ) ||
		( m_addressIPv4[ 2 ] != 0 ) ||
		( m_addressIPv4[ 3 ] != 0 )
	);
}

/* static */
bool NetAddressIPv4::IsValid( const char* ipv4 )
{
	TokenizedString ipv4Tokenized = TokenizedString( ipv4, "." );
	if ( ipv4Tokenized.GetTokens().size() != 4 )
	{
		return false;
	}

	try
	{
		int number = stoi( ipv4Tokenized.GetTokens()[ 0 ] );
		if ( number < 0 || number > 255 )
		{
			return false;
		}
	}
	catch ( std::invalid_argument& args )
	{
		UNUSED( args );
		return false;
	}
	try
	{
		int number = stoi( ipv4Tokenized.GetTokens()[ 1 ] );
		if ( number < 0 || number > 255 )
		{
			return false;
		}
	}
	catch ( std::invalid_argument& args )
	{
		UNUSED( args );
		return false;
	}
	try
	{
		int number = stoi( ipv4Tokenized.GetTokens()[ 2 ] );
		if ( number < 0 || number > 255 )
		{
			return false;
		}
	}
	catch ( std::invalid_argument& args )
	{
		UNUSED( args );
		return false;
	}

	TokenizedString ipv4PortTokenized = TokenizedString( ipv4Tokenized.GetTokens()[ 3 ], ":" );
	try
	{
		int number = stoi( ipv4PortTokenized.GetTokens()[ 0 ] );
		if ( number < 0 || number > 255 )
		{
			return false;
		}
	}
	catch ( std::invalid_argument& args )
	{
		UNUSED( args );
		return false;
	}

	if ( ipv4PortTokenized.GetTokens().size() > 1 )
	{
		try
		{
			int port = stoi( ipv4PortTokenized.GetTokens()[ 1 ] );
			UNUSED( port );
		}
		catch ( std::invalid_argument& args )
		{
			UNUSED( args );
			return false;
		}
	}

	return true;
}

/* static */
bool NetAddressIPv4::AreSameMachine( const NetAddressIPv4& left, const NetAddressIPv4& right )
{
	return (
			( left.m_addressIPv4[ 0 ] == right.m_addressIPv4[ 0 ] ) &&
			( left.m_addressIPv4[ 1 ] == right.m_addressIPv4[ 1 ] ) &&
			( left.m_addressIPv4[ 2 ] == right.m_addressIPv4[ 2 ] ) &&
			( left.m_addressIPv4[ 3 ] == right.m_addressIPv4[ 3 ] )
		);
}

/* static */
NetAddressIPv4 NetAddressIPv4::GetLocal()
{
	char hostName[ 256 ];
	if ( ::gethostname( hostName, 256 ) == SOCKET_ERROR )
	{
		LogErrorf( "NetAddressIPv4::GetLocal(): Could not get local IPv4 address. Returned value is blank." );
		return NetAddressIPv4();
	}
	const char* service = "80";

	addrinfo hints;	// Helps the system filter down to the addresses we care about when we talk to it
	memset( &hints, 0, sizeof( hints ) );
	hints.ai_family = AF_INET;				// IPv4 Addresses
	hints.ai_socktype = SOCK_STREAM;		// TCP socket (SOCK_DGRAM for UDP)
	hints.ai_flags = 0;						// hints.ai_flags |= AI_NUMERICHOST; // Will speed up the search so it won't have to lookup

	addrinfo* searchResult = nullptr;	// Obtain a linked list from the system
	int status = ::getaddrinfo(
		hostName,
		service,
		&hints,
		&searchResult
	);

	addrinfo* iterator = searchResult;
	while( iterator != nullptr )
	{
		if ( iterator->ai_family == AF_INET )
		{
			NetAddressIPv4 ipv4 = NetAddressIPv4( iterator->ai_addr );
			return ipv4;
		}

		iterator = iterator->ai_next;
	}

	LogErrorf( "NetAddressIPv4::GetLocal(): Could not find addresses for host with name \"%s\".", hostName );

	::freeaddrinfo( searchResult );

	return NetAddressIPv4();
}

/* static */
NetAddressIPv4 NetAddressIPv4::GetBindableLocal( uint16_t port )
{
	char hostName[ 256 ];
	if ( ::gethostname( hostName, 256 ) == SOCKET_ERROR )
	{
		int errorCode = WSAGetLastError();
		LogErrorf( "NetAddressIPv4::GetLocal(): Could not get local IPv4 address. Returned value is blank. Error code: %d.", errorCode );
		return NetAddressIPv4();
	}
	std::string serviceStr = std::to_string( port );
	const char* service = serviceStr.c_str();

	addrinfo hints;	// Helps the system filter down to the addresses we care about when we talk to it
	memset( &hints, 0, sizeof( hints ) );
	hints.ai_family = AF_INET;				// IPv4 Addresses
	hints.ai_socktype = SOCK_STREAM;		// TCP socket (SOCK_DGRAM for UDP)
	hints.ai_flags = AI_PASSIVE;			// An address we can host on
											// hints.ai_flags |= AI_NUMERICHOST; // Will speed up the search so it won't have to lookup

	addrinfo* searchResult = nullptr;	// Obtain a linked list from the system
	int status = ::getaddrinfo(
		hostName,
		service,
		&hints,
		&searchResult
	);

	addrinfo* iterator = searchResult;
	while( iterator != nullptr )
	{
		if ( iterator->ai_family == AF_INET )
		{
			NetAddressIPv4 ipv4 = NetAddressIPv4( iterator->ai_addr );
			return ipv4;
		}

		iterator = iterator->ai_next;
	}

	LogErrorf( "NetAddressIPv4::GetLocal(): Could not find addresses for host with name \"%s\".", hostName );

	::freeaddrinfo( searchResult );

	return NetAddressIPv4();
}

bool GetAddressForHost( sockaddr* out, int* out_addrlen, const char* hostName, const char* service )
{
	addrinfo hints;	// Helps the system filter down to the addresses we care about when we talk to it
	memset( &hints, 0, sizeof( hints ) );
	hints.ai_family = AF_INET;				// IPv4 Addresses
	hints.ai_socktype = SOCK_STREAM;		// TCP socket (SOCK_DGRAM for UDP)
	hints.ai_flags = AI_PASSIVE;			// An address we can host on
											// hints.ai_flags |= AI_NUMERICHOST; // Will speed up the search so it won't have to lookup

	addrinfo* searchResult = nullptr;	// Obtain a linked list from the system
	int status = ::getaddrinfo(
		hostName,
		service,
		&hints,
		&searchResult
	);	// The port number provided could resolve to a different IP address - for example, google.com's FTP server vs. its HTTP server

	addrinfo* iterator = searchResult;
	while( iterator != nullptr )
	{
		if ( iterator->ai_family == AF_INET )
		{
			sockaddr_in* ipv4 = reinterpret_cast< sockaddr_in* >( iterator->ai_addr );

			*out_addrlen = sizeof( sockaddr_in );
			memcpy( out, ipv4, sizeof( sockaddr_in ) );

			::freeaddrinfo( searchResult );
			return true;
		}

		iterator = iterator->ai_next;
	}

	::freeaddrinfo( searchResult );
	return false;
}

void LogIPv4AddressesForHostNameAndService( const char* hostName, const char* service )
{
	addrinfo hints;	// Helps the system filter down to the addresses we care about when we talk to it
	memset( &hints, 0, sizeof( hints ) );
	hints.ai_family = AF_INET;				// IPv4 Addresses
	hints.ai_socktype = SOCK_STREAM;		// TCP socket (SOCK_DGRAM for UDP)
	hints.ai_flags = AI_PASSIVE;			// An address we can host on
											// hints.ai_flags |= AI_NUMERICHOST; // Will speed up the search so it won't have to lookup

	addrinfo* searchResult = nullptr;	// Obtain a linked list from the system
	int status = ::getaddrinfo(
		hostName,
		service,
		&hints,
		&searchResult
	);

	int found = 0;

	addrinfo* iterator = searchResult;
	while( iterator != nullptr )
	{
		if ( iterator->ai_family == AF_INET )
		{
			NetAddressIPv4 ipv4 = NetAddressIPv4( iterator->ai_addr );
			std::string ipv4String = ipv4.GetAddressAsString();
			LogTaggedPrintf( "Network", "Address[IPv4]: %s", ipv4String.c_str() );

			found++;
		}

		iterator = iterator->ai_next;
	}

	if ( !found )
	{
		LogTaggedPrintf( "Network", "LogIPv4: Could not find addresses for host with name \"%s\".", hostName );
	}

	::freeaddrinfo( searchResult );
}

bool NetworkLogAddressForHost( Command& listHostAddressCommand )
{
	if ( listHostAddressCommand.GetName() == "net_address_host" )
	{
		std::string hostName = listHostAddressCommand.GetNextString();
		if ( hostName == "" )
		{
			ConsolePrintf( Rgba::RED, "ERROR: net_address_host: No host name provided." );
			return false;
		}

		std::string service = listHostAddressCommand.GetNextString();
		if ( service == "" )
		{
			ConsolePrintf( Rgba::YELLOW, "net_address_host: No service port number provided. Defaulting to 12345." );
			service = "12345";
		}

		LogIPv4AddressesForHostNameAndService( hostName.c_str(), service.c_str() );

		return true;
	}
	else
	{
		return false;
	}
}

bool NetworkLogIPv4Address( Command& listAddressCommand )
{
	if ( listAddressCommand.GetName() == "net_address" )
	{
		char hostName[ 256 ];
		if ( ::gethostname( hostName, 256 ) == SOCKET_ERROR )
		{
			return false;
		}

		const char* service = "80";	// Port number

		LogIPv4AddressesForHostNameAndService( hostName, service );

		return true;
	}
	else
	{
		return false;
	}
}

#endif	// #if !defined( ENGINE_DISABLE_NETWORKING )