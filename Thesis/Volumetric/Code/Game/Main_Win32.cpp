#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cassert>
#include <crtdbg.h>
#include "Game/TheApp.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Window.hpp"

//-----------------------------------------------------------------------------------------------
int WINAPI WinMain( HINSTANCE applicationInstanceHandle, HINSTANCE, LPSTR commandLineString, int )
{
	UNUSED( applicationInstanceHandle );
	UNUSED( commandLineString );
	g_theApp = new TheApp();

	while( !g_theApp->IsQuitting() )		// Program main loop; keep running frames until it's time to quit
	{
		g_theApp->RunFrame();
	}

	delete g_theApp;
	g_theApp = nullptr;

	return 0;
}
