#pragma once

#include "Engine/Core/StringUtils.hpp"
#include "Engine/Networking/NetAddress.hpp"
#include "Engine/Networking/Socket.hpp"
#include "Engine/Tools/DevConsole.hpp"

class BytePacker;

class TCPSocket	:	public Socket
{

public:
	TCPSocket();
	~TCPSocket();

	bool Listen( uint16_t port, unsigned int maxQueued, bool bindAll = true, bool blocking = false );
	TCPSocket* Accept();

	bool Connect( const NetAddressIPv4& netAddress );

	void Close() override;

	size_t Send( const void* data, const size_t dataByteSize );
	size_t Receive( void* buffer, const size_t maxByteSize );

	bool IsListening() const;
	BytePacker* GetBuffer();

public:
	BytePacker* m_buffer = nullptr;
	bool m_isListening = false;

};
