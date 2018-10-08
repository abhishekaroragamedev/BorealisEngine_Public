#include "Engine/Core/Logger.hpp"
#include "Engine/Networking/Socket.hpp"

Socket::Socket()
{
	m_netAddress = NetAddressIPv4::GetLocal();
}

Socket::~Socket()
{

}

void Socket::Close()
{
	::closesocket( m_handle );
	m_handle = INVALID_SOCKET;
}

bool Socket::IsClosed()
{
	if ( m_handle == INVALID_SOCKET )
	{
		return true;
	}

	u_long nonBlocking = 1;
	::ioctlsocket( m_handle, FIONBIO, &nonBlocking );

	char* dummy = reinterpret_cast< char* >( malloc( 1 ) );
	int receivedSize = ::recv( m_handle, dummy, 1, MSG_PEEK );
	free( dummy );

	if ( receivedSize <= 0 && HasFatalError() )
	{
		Close();
		return true;
	}

	return false;
}

bool Socket::HasFatalError() const
{
	int lastError = WSAGetLastError();
	bool hasError = !( lastError == WSAEWOULDBLOCK || lastError == WSAEMSGSIZE || lastError == WSAECONNRESET );
	if ( hasError )
	{
		LogErrorf( "Socket::HasFatalError(): Socket operation failed with error error %d.", lastError );
	}

	return hasError;
}
