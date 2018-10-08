#pragma once

#include "Engine/Networking/UDPSocket.hpp"
#include "Engine/Tools/Command.hpp"

struct NetAddressIPv4;
class UDPSocket;

class UDPTest
{
public:

	bool Start();
	void Stop();

	void SendTo( const NetAddressIPv4& addr, const void* buffer, unsigned int byteCount );

	void Update();

public:
	// if you have multiple address, you can use multiple sockets
	// but you have to be clear on which one you're sending from; 
	UDPSocket* m_socket = nullptr;

};

bool UDPTestSendCommand( Command& command );

extern UDPTest g_udpTest;
