#pragma once

#include "Engine/Networking/NetAddress.hpp"
#include "Engine/Networking/Socket.hpp"

#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
#endif
#include <WinSock2.h>	// Must be defined before <windows.h>
#include <WS2tcpip.h>	// Only needed for IPv6 Support
#include <windows.h>

// class test
#define GAME_PORT 10084
#define ETHERNET_MTU 1500  // maximum transmission unit - determined by hardware part of OSI model.
// 1500 is the MTU of EthernetV2, and is the minimum one - so we use it;
#define PACKET_MTU (ETHERNET_MTU - 40 - 8) // 48 bytes - packet header

// IPv4 Header Size: 20B
// IPv6 Header Size: 40B
// TCP Header Size: 20B-60B
// UDP Header Size: 8B
// Ethernet: 28B, but MTU is already adjusted for it
// so packet size is 1500 - 40 - 8 => 1452B (why?)

#define MESSAGE_MTU 128

class UDPSocket	:	public Socket
{

public:
	UDPSocket();
	~UDPSocket();

public:
	bool Bind( const NetAddressIPv4& addr, uint16_t portRange = 0U );
	size_t SendTo( const NetAddressIPv4& addr, const void* data, const size_t byteCount );
	size_t ReceiveFrom( NetAddressIPv4* out_addr, void* out_buffer, const size_t maxReadSize );

};
