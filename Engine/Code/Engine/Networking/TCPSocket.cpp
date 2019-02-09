#include "Engine/Core/BytePacker.hpp"
#include "Engine/Core/Logger.hpp"
#include "Engine/Networking/NetCommon.hpp"
#include "Engine/Networking/TCPSocket.hpp"
#include "Game/EngineBuildPreferences.hpp"

#if !defined( ENGINE_DISABLE_NETWORKING )

TCPSocket::TCPSocket()
{
	m_buffer = new BytePacker( Endianness::BIG_ENDIAN, ( BYTEPACKER_OWNS_MEMORY | BYTEPACKER_CAN_GROW ) );
}

TCPSocket::~TCPSocket()
{
	delete m_buffer;
	m_buffer = nullptr;
}

bool TCPSocket::Listen( uint16_t port, unsigned int maxQueued, bool bindAll /* = true */, bool blocking /* = false */ )
{
	if ( !IsClosed() )
	{
		LogWarningf( "TCPSocket::Listen(): Cannot listen while socket is open." );
		return false;
	}

	m_netAddress = NetAddressIPv4::GetBindableLocal( port );
	sockaddr socketAddress;
	size_t socketAddressLength;
	m_netAddress.ToSockAddr( &socketAddress, &socketAddressLength );
	if ( bindAll )
	{
		sockaddr_in* socketIn = reinterpret_cast< sockaddr_in* >( &socketAddress );
		socketIn->sin_addr.S_un.S_addr = INADDR_ANY;
	}

	m_handle = ::socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	u_long nonBlocking = ( blocking )? 0 : 1;
	::ioctlsocket( m_handle, FIONBIO, &nonBlocking );
	int status = ::bind( m_handle, &socketAddress, static_cast< int >( socketAddressLength ) );
	if ( status == SOCKET_ERROR && HasFatalError() )
	{
		int error = ::WSAGetLastError();
		LogErrorf( "TCPSocket::Listen(): Socket bind failed with error %d.", error );
		Close();
		return false;
	}

	status = ::listen( m_handle, maxQueued );
	if ( status == SOCKET_ERROR )
	{
		int error = ::WSAGetLastError();
		LogErrorf( "TCPSocket::Listen(): Socket listen failed with error %d.", error );
		Close();
	}
	else
	{
		m_isListening = true;
		if ( bindAll )
		{
			LogTaggedPrintf( "TCPSocket::Listen()", "Binding to all available addresses, including 127.0.0.1." );
		}
		LogTaggedPrintf( "TCPSocket::Listen()", "Listening on %s.", m_netAddress.ToString().c_str() );
	}

	return ( status != SOCKET_ERROR );
}

TCPSocket* TCPSocket::Accept()
{
	sockaddr_storage otherSockAddr;
	int otherAddrLength = sizeof( otherSockAddr );

	TCPSocket* otherSocket = new TCPSocket();

	//LogTaggedPrintf( "TCPSocket::Accept()", "Waiting for connection..." );
	otherSocket->m_handle = ::accept(
		m_handle,
		reinterpret_cast< sockaddr* >( &otherSockAddr ),
		&otherAddrLength
	);
	otherSocket->m_netAddress = NetAddressIPv4( reinterpret_cast< sockaddr* >( &otherSockAddr ) );

	if ( otherSocket->m_handle == INVALID_SOCKET )
	{
		delete otherSocket;
		otherSocket = nullptr;
	}
	else
	{
		LogPrintf( "TCPSocket::Accept(): Connection accepted." );
	}

	return otherSocket;
}

bool TCPSocket::Connect( const NetAddressIPv4& netAddress )
{
	m_handle = ::socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	if ( m_handle == INVALID_SOCKET )
	{
		LogErrorf( "TCPSocket::Connect(): Could not open socket." );
	}

	sockaddr sockAddr;
	size_t addrLength;
	netAddress.ToSockAddr( &sockAddr, &addrLength );

	u_long nonBlocking = 0;
	::ioctlsocket( m_handle, FIONBIO, &nonBlocking );

	int status = ::connect(
		m_handle,
		&sockAddr,
		static_cast< int >( addrLength )
	);
	if ( status == SOCKET_ERROR )
	{
		LogErrorf( "TCPSocket::Connect(): Could not connect." );
		Close();
	}
	else
	{
		m_netAddress = netAddress;
	}

	return ( status != SOCKET_ERROR );
}

void TCPSocket::Close() /* override */
{
	Socket::Close();
	m_isListening = false;
}

size_t TCPSocket::Send( const void* data, const size_t dataByteSize )
{
	size_t sent = 0;

	u_long nonBlocking = 1;
	::ioctlsocket( m_handle, FIONBIO, &nonBlocking );

	int numBytesSent =  ::send(
		m_handle,
		reinterpret_cast< const char* >( data ),
		static_cast< int >( dataByteSize ),
		0
	);
	if ( numBytesSent != SOCKET_ERROR )
	{
		sent = static_cast< size_t >( numBytesSent );
	}
	else
	{
		LogErrorf( "TCPSocket::Send(): Could not send data." );
		Close();
	}

	return sent;
}

size_t TCPSocket::Receive( void* buffer, const size_t maxByteSize )
{
	size_t received = 0;

	u_long nonBlocking = 1;
	::ioctlsocket( m_handle, FIONBIO, &nonBlocking );

	int numBytesReceived = ::recv(
		m_handle,
		reinterpret_cast< char* >( buffer ),
		static_cast< int >( maxByteSize ),
		0
	);
	if ( numBytesReceived != SOCKET_ERROR )
	{
		received = static_cast< size_t >( numBytesReceived );
	}
	else if ( HasFatalError() )
	{
		LogErrorf( "TCPSocket::Receive(): Could not receive data." );
		Close();
	}

	return received;
}

bool TCPSocket::IsListening() const
{
	return m_isListening;
}

BytePacker* TCPSocket::GetBuffer()
{
	return m_buffer;
}

#endif	// #if !defined( ENGINE_DISABLE_NETWORKING )
