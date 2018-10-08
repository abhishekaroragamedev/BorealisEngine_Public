#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Logger.hpp"
#include "Engine/Networking/UDPSocket.hpp"

UDPSocket::UDPSocket()
{

}

UDPSocket::~UDPSocket()
{

}

bool UDPSocket::Bind( const NetAddressIPv4& addr, uint16_t portRange /* = 0U */ )
{
	// create the socket 
	SOCKET mySocket = socket( AF_INET,	// IPv4 to send...
		SOCK_DGRAM,							// ...Datagrams... 
		IPPROTO_UDP );						// ...using UDP.

	ASSERT_RECOVERABLE( mySocket != INVALID_SOCKET, "UDPSocket::Bind(): Bind failed!" ); 
	if ( mySocket == INVALID_SOCKET )
	{
		LogErrorf( "UDPSocket::Bind(): Bind failed with error %d.", WSAGetLastError() );
		return false;
	}

	// TODO, try to bind all ports within the range.
	// Shown - just trying one; 

	NetAddressIPv4 bindAddr = NetAddressIPv4( addr.ToString().c_str() );
	bindAddr.m_port = portRange;

	sockaddr_storage sockAddr;
	size_t sockAddrLen;
	bindAddr.ToSockAddr( (sockaddr*)&sockAddr, &sockAddrLen );

	// try to bind - if it succeeds - great.  If not, try the next port in the range.
	int result = ::bind(
		mySocket,
		reinterpret_cast< sockaddr* >( &sockAddr ),
		static_cast< int >( sockAddrLen )
	);

	if ( result == 0 )
	{
		LogPrintf( "UDPSocket::Bind(): Successfully bound to %s.", bindAddr.ToString().c_str() );
		m_handle = mySocket; 
		m_netAddress = addr; 
		return true; 
	} 

	return false; 
}

size_t UDPSocket::SendTo( const NetAddressIPv4& addr, const void* data, const size_t byteCount )
{
	sockaddr_storage sockAddr;
	size_t addrLen;
	addr.ToSockAddr( reinterpret_cast< sockaddr* >( &sockAddr ), &addrLen );

	SOCKET socket = m_handle;
	int sent = ::sendto(
		socket,
		reinterpret_cast< const char* >( data ),
		static_cast< int >( byteCount ),
		0,
		reinterpret_cast< sockaddr* >( &sockAddr ),
		static_cast< int >( addrLen )
	);

	if ( sent > 0 )
	{
		ASSERT_RECOVERABLE( sent == byteCount, "ERROR: UDPSocket::SendTo(): Sent less bytes than expected." );
		return static_cast< size_t >( sent );
	}
	
	if ( HasFatalError() )
	{
		Close();
	}
	return 0U;
}

size_t UDPSocket::ReceiveFrom( NetAddressIPv4* out_addr, void* out_buffer, const size_t maxReadSize )
{
	if ( IsClosed() )
	{
		return 0U;
	}

	sockaddr_storage fromAddr;	// The sender
	int addrLen = sizeof( sockaddr_storage );
	SOCKET socket = m_handle;

	int received = ::recvfrom(
		socket,
		reinterpret_cast< char* >( out_buffer ),
		static_cast< int >( maxReadSize ),
		0,
		reinterpret_cast< sockaddr* >( &fromAddr ),
		&addrLen
	);

	if ( received > 0 )
	{
		out_addr->FromSockAddr( reinterpret_cast< sockaddr* >( &fromAddr ) );
		return received;
	}
	else
	{
		if ( HasFatalError() )
		{
			Close();
		}
	}

	return 0U;
}
