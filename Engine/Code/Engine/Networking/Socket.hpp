#pragma once

#include "Engine/Networking/NetAddress.hpp"
#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
#endif
#include <WinSock2.h>	// Must be defined before <windows.h>
#include <WS2tcpip.h>	// Only needed for IPv6 Support
#include <windows.h>

class Socket
{
public:
	Socket();
	~Socket();

public:
	virtual void Close();

	bool IsClosed();

protected:
	bool HasFatalError() const;

public:
	SOCKET m_handle = INVALID_SOCKET;
	NetAddressIPv4 m_netAddress;

};
