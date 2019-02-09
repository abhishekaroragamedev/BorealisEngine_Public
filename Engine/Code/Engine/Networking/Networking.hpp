#pragma once

#include "Engine/Tools/Command.hpp"

class Networking
{

public:
	static bool Startup();
	static bool Shutdown();

};

bool NetHostCommand( Command& hostCommand );
bool NetJoinCommand( Command& joinCommand );
bool NetworkTestConnectToIP( Command& connectCommand );
bool NetworkTestConnectToHost( Command& connectCommand );
bool NetworkHostTest( Command& hostCommand );
