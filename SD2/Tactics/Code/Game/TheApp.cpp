#include "Game/GameCommon.hpp"
#include "Game/TheApp.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/Window.hpp"
#include "Engine/Networking/NetSession.hpp"
#include "Engine/Networking/Networking.hpp"
#include "Engine/Tools/DevConsole.hpp"
#include "Engine/Tools/Profiler/Profiler.hpp"
#include "Engine/Tools/RemoteCommandService.hpp"
#include <windows.h>

bool AppMessageHandler( UINT wmMessageCode, WPARAM wParam, LPARAM lParam )
{
	UNUSED( lParam );
	switch( wmMessageCode )
	{
		// App close requested via "X" button, or right-click "Close Window" on task bar, or "Close" from system menu, or Alt-F4
		case WM_CLOSE:		
		{
			g_theApp->ForceQuit();
			return false; // "Consumes" this message (tells Windows "okay, we handled it")
		}

		// Raw physical keyboard "key-was-just-depressed" event (case-insensitive, not translated)
		case WM_KEYDOWN:
		{
			unsigned char asKey = (unsigned char) wParam;
			g_inputSystem->HandleKeyDown( asKey );
			break;
		}

		// Raw physical keyboard "key-was-just-released" event (case-insensitive, not translated)
		case WM_KEYUP:
		{
			unsigned char asKey = (unsigned char) wParam;
			g_inputSystem->HandleKeyUp( asKey );
			break;
		}

		case WM_CHAR:		// Only for Ctrl+A, C, V and X
		{
			unsigned char asKey = ( unsigned char ) wParam;
			if ( asKey == InputSystem::KEYBOARD_CTRL_A || asKey == InputSystem::KEYBOARD_CTRL_C || asKey == InputSystem::KEYBOARD_CTRL_V || asKey == InputSystem::KEYBOARD_CTRL_X )
			{
				g_inputSystem->HandleKeyDown( asKey );
			}
			break;
		}
	}

	return true;
}

bool QuitCommand( Command& quitCommand )
{
	if ( quitCommand.GetName() == "quit" )
	{
		g_theApp->ForceQuit();
		return true;
	}
	return false;
}

TheApp::TheApp()
{
	m_isQuitting = false;

	LogSystemStartup();

	Window::CreateAndGetInstance( static_cast< int >( SCREEN_HEIGHT * CLIENT_ASPECT ), static_cast< int >( SCREEN_HEIGHT ), PROTOGAME_2D_APP_NAME );
	Window::GetInstance()->RegisterHandler( ( windows_message_handler_cb ) AppMessageHandler );

	ClockSystemStartup();

	g_renderer = new Renderer();
	DevConsole::CreateInstance( g_renderer );
	
	Networking::Startup();
	RemoteCommandServiceStartup();

	ProfilerStartup();

	g_audioSystem = new AudioSystem();
	g_inputSystem = new InputSystem();
	g_theGame = new TheGame();

	CommandRegister( "quit", QuitCommand, "Quits the application." );
}

TheApp::~TheApp()
{
	delete g_theGame;
	g_theGame = 0;

	ProfilerShutdown();

	RemoteCommandServiceShutdown();
	NetSession::ShutdownInstance();
	Networking::Shutdown();

	delete g_inputSystem;
	g_inputSystem = 0;

	delete g_renderer;
	g_renderer = 0;

	delete g_audioSystem;
	g_audioSystem = 0;

	DevConsole::DeleteInstance();
	Window::DeleteInstance();

	LogSystemShutdown();
}

//-----------------------------------------------------------------------------------------------
// One "frame" of the game.  Generally: Input, Update, Render.  We call this 60+ times per second.
//
void TheApp::RunFrame()
{
	ClockSystemBeginFrame();
	g_inputSystem->BeginFrame();
	g_renderer->BeginFrame( ( SCREEN_HEIGHT * CLIENT_ASPECT ), SCREEN_HEIGHT );

	Update();
	Render();

	g_inputSystem->EndFrame();
	g_renderer->EndFrame();

	Profiler::EndFrame();

	Sleep( 1 );
}

void TheApp::Update()
{
	RemoteCommandServiceUpdate();

	HandleKeyboardInput();
	if ( IsDevConsoleOpen() )
	{
		DevConsole::GetInstance()->Update( *g_inputSystem, *g_renderer );
	}

	NetSession::GetInstance()->Update();
	g_theGame->Update();
	NetSession::GetInstance()->ProcessOutgoing();
}

void TheApp::HandleKeyboardInput()
{
	if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_F5 ) )
	{
		g_renderer->TakeScreenshotAtEndOfFrame();
	}
	if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_F9 ) )
	{
		g_renderer->ReloadAllFileShaders();
	}
	if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_TILDE ) )
	{
		DevConsole::GetInstance()->ToggleOpenState();
	}
	if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_F11 ) )
	{
		if ( GetMasterDeltaSeconds() > 0.0f )
		{
			CommandRun( "clock_pause" );
		}
		else
		{
			CommandRun( "clock_reset" );
		}
	}
	if ( !DevConsole::GetInstance()->IsOpen() && g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_RIGHT_SQUARE_BRACKET ) )
	{
		std::string timeScaleCommand = "time_scale " + std::to_string( TIME_SCALE_HOTKEY_VALUES[ m_nextTimeScaleHotkeyIndex ] );
		CommandRun( timeScaleCommand.c_str() );
		m_nextTimeScaleHotkeyIndex = ( m_nextTimeScaleHotkeyIndex + 1 ) % NUM_TIME_SCALE_HOTKEY_VALUES;
	}
	if ( !DevConsole::GetInstance()->IsOpen() && g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_LEFT_SQUARE_BRACKET ) )
	{
		std::string timeScaleCommand = "time_scale " + std::to_string( TIME_SCALE_HOTKEY_VALUES[ m_nextTimeScaleHotkeyIndex ] );
		CommandRun( timeScaleCommand.c_str() );
		m_nextTimeScaleHotkeyIndex -= 1;
		if ( m_nextTimeScaleHotkeyIndex < 0 )
		{
			m_nextTimeScaleHotkeyIndex = NUM_TIME_SCALE_HOTKEY_VALUES - 1;
		}
	}

	if ( Profiler::GetInstance()->IsVisible() )
	{
		if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_V ) )
		{
			Profiler::GetInstance()->ToggleReportView();
		}
		if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_L ) )
		{
			Profiler::GetInstance()->ToggleSortMode();
		}
		if ( g_inputSystem->WasMouseButtonJustPressed( MouseButton::LEFT_MOUSE_BUTTON ) )
		{
			Profiler::GetInstance()->HandleMouseClickAtPosition( g_inputSystem->GetMouseClientPosition() );
		}
		if ( g_inputSystem->IsMouseButtonDown( MouseButton::LEFT_MOUSE_BUTTON ) )
		{
			Profiler::GetInstance()->HandleMouseDownAtPosition( g_inputSystem->GetMouseClientPosition() );
		}
		if ( g_inputSystem->WasMouseButtonJustPressed( MouseButton::RIGHT_MOUSE_BUTTON ) )
		{
			Profiler::GetInstance()->DeselectFrame();
		}
	}
}

void TheApp::Render()
{
	g_theGame->Render();

	Profiler::Render();

	if ( IsDevConsoleOpen() )
	{
		DevConsole::GetInstance()->Render( *g_renderer );
	}
}

bool TheApp::IsQuitting() const
{
	return m_isQuitting;
}

void TheApp::ForceQuit()
{
	m_isQuitting = true;
}
