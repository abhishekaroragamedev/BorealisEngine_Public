#pragma once

#pragma comment( lib, "ws2_32.lib" )	// Winsock libraries

#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>	// Must be defined before <windows.h>
#include <WS2tcpip.h>	// Only needed for IPv6 Support
#include <windows.h>
