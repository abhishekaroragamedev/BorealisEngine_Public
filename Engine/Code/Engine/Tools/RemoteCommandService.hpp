#pragma once

#include "Engine/Core/Threading.hpp"
#include "Engine/Core/ThreadSafeTypes.hpp"
#include <string>

struct NetAddressIPv4;
class BytePacker;
class Command;
class TCPSocket;

constexpr unsigned int RCS_DEFAULT_PORT_NUMBER = 29283U;
constexpr unsigned int RCS_MAX_CLIENTS = 32U;
constexpr float RCS_RETRY_TIME_SECONDS = 3.0f;

enum RCSMode
{
	RCS_MODE_INVALID = -1,
	RCS_MODE_SERVER,
	RCS_MODE_CLIENT
};

class RemoteCommandService
{

private:
	RemoteCommandService();
	~RemoteCommandService();

public:
	bool IsRunning();
	std::string GetAddressIPv4String();
	unsigned int GetMaxNumClients() const;
	unsigned int GetNumClients();
	std::string GetClientAddressIPv4String( unsigned int clientIndex );
	RCSMode GetMode();

	void SetMode( RCSMode mode );

private:
	bool StartServer( unsigned int port = RCS_DEFAULT_PORT_NUMBER );
	void ShutdownServer();
	bool InitializeSocket( unsigned int port );
	bool StartClient( const NetAddressIPv4& serverAddr );
	void ShutdownClient();
	bool TryShutdown();

	void Update();

	void ValidateAndUpdateSocketsToClients();
	void UpdateSocketToHost();
	void CloseClientSockets();

	bool SendCommand( int clientIndex, bool isEcho, const std::string& commandStr );
	bool BroadcastCommand( const std::string& commandStr );
	void ProcessConnection( TCPSocket* socket );
	void ProcessMessage( TCPSocket* socket, BytePacker* payload );

	void SetEchoState( bool echoState );

public:
	static RemoteCommandService* GetInstance();

	friend void RemoteCommandServerWork( void* params );
	friend void RemoteCommandClientWork( void* params );
	friend void RemoteCommandServiceStartup();
	friend void RemoteCommandServiceUpdate();
	friend void RemoteCommandServiceShutdown();
	friend bool RCSHostCommand( Command& command );
	friend bool RCSJoinCommand( Command& command );
	friend bool RCSSendCommand( Command& command );
	friend bool RCSAllCommand( Command& command );
	friend bool RCSBroadcastCommand( Command& command );
	friend bool RCSEchoStateCommand( Command& command );
	friend void EchoConsoleLog( const std::string& logOutput, void* socketInfo );

private:
	// Handled only by main thread
	ThreadHandle m_workerThread = nullptr;
	float m_cooldownTimeCurrent = 0.0f;

	// Handled by main, server and client thread(s)
	ThreadSafePrimitive< TCPSocket* > m_socket;
	ThreadSafeVector< TCPSocket* > m_clients;
	ThreadSafePrimitive< RCSMode > m_mode;
	ThreadSafePrimitive< bool > m_shouldProcessEcho;

};

void RemoteCommandServiceStartup();
void RemoteCommandServiceUpdate();
void RemoteCommandServiceShutdown();

bool RCSHostCommand( Command& command );
bool RCSJoinCommand( Command& command );
bool RCSSendCommand( Command& command );
bool RCSAllCommand( Command& command );
bool RCSBroadcastCommand( Command& command );
bool RCSEchoStateCommand( Command& command );
