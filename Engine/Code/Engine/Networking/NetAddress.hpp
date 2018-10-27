#pragma once

#include "Engine/Tools/Command.hpp"
#include <string>

#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
#endif
#include <WinSock2.h>	// Must be defined before <windows.h>
#include <WS2tcpip.h>	// Only needed for IPv6 Support
#include <windows.h>

struct NetAddressIPv4
{

public:
	NetAddressIPv4();
	NetAddressIPv4( const sockaddr* socketAddress );
	NetAddressIPv4( const char* ipv4String );
	NetAddressIPv4( const NetAddressIPv4& copy );

	bool operator==( const NetAddressIPv4& toCompare ) const;

	bool ToSockAddr( sockaddr* out_address, size_t* out_addressLength ) const;
	bool FromSockAddr( const sockaddr* socketAddress );

	std::string ToString() const;
	std::string GetAddressAsString() const;
	std::string GetPortAsString() const;
	void FromString( const char* ipv4String );
	void FromString( const std::string& ipv4String );

	bool IsSet() const;

public:
	static bool IsValid( const char* ipv4 );
	static NetAddressIPv4 GetLocal();
	static NetAddressIPv4 GetBindableLocal( uint16_t port );	// Get a local address that's not in use

public:
	unsigned char m_addressIPv4[ 4 ] = {
		0, 0, 0, 0
	};
	uint16_t m_port = 0U;

};

bool GetAddressForHost( sockaddr* out, int* out_addrlen, const char* hostName, const char* service );

bool NetworkLogAddressForHost( Command& listHostAddressCommand );
bool NetworkLogIPv4Address( Command& listAddressCommand );
