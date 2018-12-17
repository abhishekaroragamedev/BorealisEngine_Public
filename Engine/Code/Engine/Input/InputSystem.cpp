#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "Engine/Input/InputSystem.hpp"

const unsigned char InputSystem::KEYBOARD_ESCAPE = VK_ESCAPE;
const unsigned char InputSystem::KEYBOARD_RETURN = VK_RETURN;
const unsigned char InputSystem::KEYBOARD_SPACE = VK_SPACE;
const unsigned char InputSystem::KEYBOARD_TAB = VK_TAB;
const unsigned char InputSystem::KEYBOARD_UP_ARROW = VK_UP;
const unsigned char InputSystem::KEYBOARD_DOWN_ARROW = VK_DOWN;
const unsigned char InputSystem::KEYBOARD_LEFT_ARROW = VK_LEFT;
const unsigned char InputSystem::KEYBOARD_RIGHT_ARROW = VK_RIGHT;
const unsigned char InputSystem::KEYBOARD_SHIFT = VK_SHIFT;
const unsigned char InputSystem::KEYBOARD_F1 = VK_F1;
const unsigned char InputSystem::KEYBOARD_F2 = VK_F2;
const unsigned char InputSystem::KEYBOARD_F3 = VK_F3;
const unsigned char InputSystem::KEYBOARD_F4 = VK_F4;
const unsigned char InputSystem::KEYBOARD_F5 = VK_F5;
const unsigned char InputSystem::KEYBOARD_F6 = VK_F6;
const unsigned char InputSystem::KEYBOARD_F7 = VK_F7;
const unsigned char InputSystem::KEYBOARD_F8 = VK_F8;
const unsigned char InputSystem::KEYBOARD_F9 = VK_F9;
const unsigned char InputSystem::KEYBOARD_F10 = VK_F10;
const unsigned char InputSystem::KEYBOARD_F11 = VK_F11;
const unsigned char InputSystem::KEYBOARD_F12 = VK_F12;
const unsigned char InputSystem::KEYBOARD_A = 'A';
const unsigned char InputSystem::KEYBOARD_B = 'B';
const unsigned char InputSystem::KEYBOARD_C = 'C';
const unsigned char InputSystem::KEYBOARD_D = 'D';
const unsigned char InputSystem::KEYBOARD_E = 'E';
const unsigned char InputSystem::KEYBOARD_F = 'F';
const unsigned char InputSystem::KEYBOARD_G = 'G';
const unsigned char InputSystem::KEYBOARD_H = 'H';
const unsigned char InputSystem::KEYBOARD_I = 'I';
const unsigned char InputSystem::KEYBOARD_J = 'J';
const unsigned char InputSystem::KEYBOARD_K = 'K';
const unsigned char InputSystem::KEYBOARD_L = 'L';
const unsigned char InputSystem::KEYBOARD_M = 'M';
const unsigned char InputSystem::KEYBOARD_N = 'N';
const unsigned char InputSystem::KEYBOARD_O = 'O';
const unsigned char InputSystem::KEYBOARD_P = 'P';
const unsigned char InputSystem::KEYBOARD_Q = 'Q';
const unsigned char InputSystem::KEYBOARD_R = 'R';
const unsigned char InputSystem::KEYBOARD_S = 'S';
const unsigned char InputSystem::KEYBOARD_T = 'T';
const unsigned char InputSystem::KEYBOARD_U = 'U';
const unsigned char InputSystem::KEYBOARD_V = 'V';
const unsigned char InputSystem::KEYBOARD_W = 'W';
const unsigned char InputSystem::KEYBOARD_X = 'X';
const unsigned char InputSystem::KEYBOARD_Y = 'Y';
const unsigned char InputSystem::KEYBOARD_Z = 'Z';
const unsigned char InputSystem::KEYBOARD_0 = '0';
const unsigned char InputSystem::KEYBOARD_1 = '1';
const unsigned char InputSystem::KEYBOARD_2 = '2';
const unsigned char InputSystem::KEYBOARD_3 = '3';
const unsigned char InputSystem::KEYBOARD_4 = '4';
const unsigned char InputSystem::KEYBOARD_5 = '5';
const unsigned char InputSystem::KEYBOARD_6 = '6';
const unsigned char InputSystem::KEYBOARD_7 = '7';
const unsigned char InputSystem::KEYBOARD_8 = '8';
const unsigned char InputSystem::KEYBOARD_9 = '9';

InputSystem::InputSystem()
{
	InitializeXboxControllers();
}

InputSystem::~InputSystem()
{

}

void InputSystem::BeginFrame()
{
	RunMessagePump();

	for ( int i = 0; i < NUM_XBOXCONTROLLERS; i++ )
	{
		m_xboxControllers[ i ].BeginFrame();
	}
}

void InputSystem::EndFrame()
{
	ResetKeyStates();

	for ( int i = 0; i < NUM_XBOXCONTROLLERS; i++ )
	{
		m_xboxControllers[ i ].EndFrame();
	}
}

void InputSystem::HandleKeyDown( unsigned int keyCode )
{
	if ( !m_keyState[ keyCode ].m_isDown )
	{
		m_keyState[ keyCode ].m_wasJustPressed = true;
	}

	m_keyState[ keyCode ].m_isDown = true;
}

void InputSystem::HandleKeyUp( unsigned int keyCode )
{
	m_keyState[ keyCode ].m_isDown = false;
	m_keyState[ keyCode ].m_wasJustReleased = true;
}

bool InputSystem::IsKeyDown( unsigned char keyCode ) const
{
	return ( m_keyState[ keyCode ].m_isDown ); 
}

bool InputSystem::WasKeyJustPressed( unsigned char keyCode ) const
{
	return ( m_keyState[ keyCode ].m_wasJustPressed );
}

bool InputSystem::WasKeyJustReleased( unsigned char keyCode ) const
{
	return ( m_keyState[ keyCode ].m_wasJustReleased );
}

XboxController& InputSystem::GetController( int index )
{
	return m_xboxControllers[ index ];
}

void InputSystem::InitializeXboxControllers()
{
	for ( int i = 0; i < NUM_XBOXCONTROLLERS; i++ )
	{
		m_xboxControllers[ i ].SetControllerId( i );
	}
}

void InputSystem::ResetKeyStates()
{
	for ( int i = 0; i < KEYBOARD_NUM_STATES; i++ )
	{
		if ( m_keyState[ i ].m_wasJustPressed  )
		{
			m_keyState[ i ].m_wasJustPressed = false;
		}
		if ( m_keyState[ i ].m_wasJustReleased  )
		{
			m_keyState[ i ].m_wasJustReleased = false;
		}
	}
}

//-----------------------------------------------------------------------------------------------
// Processes all Windows messages (WM_xxx) for this app that have queued up since last frame.
// For each message in the queue, our WindowsMessageHandlingProcedure (or "WinProc") function
//	is called, telling us what happened (key up/down, minimized/restored, gained/lost focus, etc.)
//
void InputSystem::RunMessagePump()
{
	MSG queuedMessage;
	for( ;; )
	{
		const BOOL wasMessagePresent = PeekMessage( &queuedMessage, nullptr, 0, 0, PM_REMOVE );
		if( !wasMessagePresent )
		{
			break;
		}

		TranslateMessage( &queuedMessage );
		DispatchMessage( &queuedMessage ); // This tells Windows to call our "WindowsMessageHandlingProcedure" function
	}
}
