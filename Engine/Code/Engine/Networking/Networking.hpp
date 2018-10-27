#pragma once

#include "Engine/Tools/Command.hpp"

#pragma comment( lib, "ws2_32.lib" )	// Winsock libraries

class Networking
{

public:
	static bool Startup();
	static bool Shutdown();

};

bool NetworkTestConnectToIP( Command& connectCommand );
bool NetworkTestConnectToHost( Command& connectCommand );
bool NetworkHostTest( Command& hostCommand );
