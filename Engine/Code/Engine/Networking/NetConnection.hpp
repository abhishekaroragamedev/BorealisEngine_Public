#pragma once

#include "Engine/Networking/NetAddress.hpp"

class NetMessage;
class NetSession;

struct NetConnection
{

public:
	NetConnection( const NetAddressIPv4& addr, NetSession* session, unsigned int connectionIndex );
	NetConnection( const NetConnection& copy );

	void Send( const NetMessage& message );
	void ProcessOutgoing();

public:
	NetAddressIPv4 m_address;
	NetSession* m_session = nullptr;
	std::vector< NetMessage* > m_outboundMessages; 
	unsigned int m_connectionIndex = 0U;

};
