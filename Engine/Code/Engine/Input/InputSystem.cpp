#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Window.hpp"
#include "Engine/Input/InputSystem.hpp"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

static InputSystem* g_inputSystemInstance = nullptr;

const unsigned char InputSystem::KEYBOARD_ESCAPE = VK_ESCAPE;
const unsigned char InputSystem::KEYBOARD_RETURN = VK_RETURN;
const unsigned char InputSystem::KEYBOARD_SPACE = VK_SPACE;
const unsigned char InputSystem::KEYBOARD_BACKSPACE = VK_BACK;
const unsigned char InputSystem::KEYBOARD_DELETE = VK_DELETE;
const unsigned char InputSystem::KEYBOARD_TAB = VK_TAB;
const unsigned char InputSystem::KEYBOARD_TILDE = VK_OEM_3;
const unsigned char InputSystem::KEYBOARD_UP_ARROW = VK_UP;
const unsigned char InputSystem::KEYBOARD_DOWN_ARROW = VK_DOWN;
const unsigned char InputSystem::KEYBOARD_LEFT_ARROW = VK_LEFT;
const unsigned char InputSystem::KEYBOARD_RIGHT_ARROW = VK_RIGHT;
const unsigned char InputSystem::KEYBOARD_PAGE_UP = VK_PRIOR;
const unsigned char InputSystem::KEYBOARD_PAGE_DOWN = VK_NEXT;
const unsigned char InputSystem::KEYBOARD_SHIFT = VK_SHIFT;
const unsigned char InputSystem::KEYBOARD_CTRL_A = 1;
const unsigned char InputSystem::KEYBOARD_CTRL_C = 3;
const unsigned char InputSystem::KEYBOARD_CTRL_V = 22;
const unsigned char InputSystem::KEYBOARD_CTRL_X = 24;
const unsigned char InputSystem::KEYBOARD_LEFT_SQUARE_BRACKET = VK_OEM_4;
const unsigned char InputSystem::KEYBOARD_RIGHT_SQUARE_BRACKET = VK_OEM_6;
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
	ResetMouseFramePositionsToCenter();

	g_inputSystemInstance = this;
}

InputSystem::~InputSystem()
{

}

void InputSystem::BeginFrame()
{
	m_mouseWheelDelta = 0.0f;

	RunMessagePump();

	SetMouseFramePositions();
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

	m_mouseLockedThisFrame = false;
}

void InputSystem::LockMouseToScreen( bool lock )
{
	RECT lockRegion;

	if ( !lock )
	{
		::ClipCursor( nullptr );	// Unlocks the mouse
	}
	else
	{
		HWND* hWnd = reinterpret_cast< HWND* >( Window::GetInstance()->GetHandle() );

		RECT clientRect;
		::GetClientRect( *hWnd, &clientRect );
		
		POINT offset;
		offset.x = 0;
		offset.y = 0;
		::ClientToScreen( *hWnd, &offset );

		lockRegion = clientRect;
		::OffsetRect( &lockRegion, offset.x, offset.y );
		::ClipCursor( &lockRegion );

		ResetMouseFramePositionsToCenter();

		m_mouseLockedThisFrame = true;
	}
}

void InputSystem::ShowCursor( bool show )
{
	m_isCursorVisible = show;
	::ShowCursor( show );
}

void InputSystem::SetMouseScreenPosition( const Vector2& desktopPosition )
{
	::SetCursorPos( static_cast< int >( desktopPosition.x ), static_cast< int >( desktopPosition.y ) );
}

void InputSystem::SetMousePosition( const Vector2& clientPosition )
{
	HWND* hWnd = reinterpret_cast< HWND* >( Window::GetInstance()->GetHandle() );
	
	POINT screenPosition;
	screenPosition.x = static_cast< int >( clientPosition.x );
	screenPosition.y = static_cast< int >( clientPosition.y );
	::ClientToScreen( *hWnd, &screenPosition );
	::SetCursorPos( screenPosition.x, screenPosition.y );
}

void InputSystem::AddMouseWheelDelta( float delta )
{
	m_mouseWheelDelta += delta;
}

void InputSystem::SetMouseDeltaMode( MouseDeltaMode mode )
{
	m_mouseMode = mode;
}

void InputSystem::RecomputeClientRect()
{
	LockMouseToScreen( true );
}

void InputSystem::SetWindowActive( bool active )
{
	m_isWindowActive = active;
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

	// Handle special cases for CTRL+A, C, V and X
	if ( keyCode == InputSystem::KEYBOARD_A )
	{
		HandleKeyUp( InputSystem::KEYBOARD_CTRL_A );
	}
	else if ( keyCode == InputSystem::KEYBOARD_C )
	{
		HandleKeyUp( InputSystem::KEYBOARD_CTRL_C );
	}
	else if ( keyCode == InputSystem::KEYBOARD_V )
	{
		HandleKeyUp( InputSystem::KEYBOARD_CTRL_V );
	}
	else if ( keyCode == InputSystem::KEYBOARD_X )
	{
		HandleKeyUp( InputSystem::KEYBOARD_CTRL_X );
	}
}

MouseButton InputSystem::GetMouseButtonForWParam( unsigned int wParam ) const
{
	switch ( wParam )
	{
		case MK_LBUTTON	:	return MouseButton::LEFT_MOUSE_BUTTON;
		case MK_MBUTTON	:	return MouseButton::MIDDLE_MOUSE_BUTTON;
		case MK_RBUTTON	:	return MouseButton::RIGHT_MOUSE_BUTTON;
	}
	return MouseButton::MOUSE_BUTTON_INVALID;
}

void InputSystem::HandleMouseButtonDown( unsigned int wParam )
{
	MouseButton button = GetMouseButtonForWParam( wParam );
	if ( button != MouseButton::MOUSE_BUTTON_INVALID )
	{
		if ( !m_mouseState[ button ].m_isDown )
		{
			m_mouseState[ button ].m_wasJustPressed = true;
		}
		m_mouseState[ button ].m_isDown = true;
	}
}

void InputSystem::HandleMouseButtonUp( MouseButton button )
{
	if ( button != MouseButton::MOUSE_BUTTON_INVALID )
	{
		if ( m_mouseState[ button ].m_isDown )
		{
			m_mouseState[ button ].m_wasJustReleased = true;
		}
		m_mouseState[ button ].m_isDown = false;
	}
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

bool InputSystem::IsMouseButtonDown( MouseButton button ) const
{
	return ( m_mouseState[ button ].m_isDown ); 
}

bool InputSystem::WasMouseButtonJustPressed( MouseButton button ) const
{
	return ( m_mouseState[ button ].m_wasJustPressed ); 
}

bool InputSystem::WasMouseButtonJustReleased( MouseButton button ) const
{
	return ( m_mouseState[ button ].m_wasJustReleased ); 
}

bool InputSystem::IsCursorVisible() const
{
	return m_isCursorVisible;
}

MouseDeltaMode InputSystem::GetMouseDeltaMode() const
{
	return m_mouseMode;
}

XboxController& InputSystem::GetController( int index )
{
	return m_xboxControllers[ index ];
}

Vector2 InputSystem::GetMouseClientPosition() const
{
	HWND* hWnd = reinterpret_cast< HWND* >( Window::GetInstance()->GetHandle() );

	POINT mouseScreenPosition;
	::GetCursorPos( &mouseScreenPosition );
	::ScreenToClient( *hWnd, &mouseScreenPosition );

	POINT clientPosition = mouseScreenPosition;

	return Vector2( static_cast< float >( clientPosition.x ), static_cast< float >( clientPosition.y ) );
}

Vector2 InputSystem::GetMouseClientDelta() const
{
	return ( m_mousePositionThisFrame - m_mousePositionLastFrame );
}

float InputSystem::GetMouseWheelDelta() const
{
	return m_mouseWheelDelta;
}

void InputSystem::InitializeXboxControllers()
{
	for ( int i = 0; i < NUM_XBOXCONTROLLERS; i++ )
	{
		m_xboxControllers[ i ].SetControllerId( i );
	}
}

void InputSystem::SetMouseFramePositions()
{
	if ( m_isWindowActive )
	{
		if ( !m_mouseLockedThisFrame )	// When the window is refocused (the mouse is just locked), updating mouse positions doesn't make sense
		{
			m_mousePositionLastFrame = m_mousePositionThisFrame;
			m_mousePositionThisFrame = GetMouseClientPosition();
			if ( m_mouseMode == MouseDeltaMode::MOUSE_MODE_RELATIVE )
			{
				m_mousePositionLastFrame = GetCenterOfClientWindow();
				SetMousePosition( m_mousePositionLastFrame );
			}
		}
		else
		{
			ResetMouseFramePositionsToCenter();
		}
	}
}

void InputSystem::ResetMouseFramePositionsToCenter()
{
	m_mousePositionLastFrame = GetCenterOfClientWindow();
	m_mousePositionThisFrame = GetCenterOfClientWindow();
	SetMousePosition( GetCenterOfClientWindow() );
}

Vector2 InputSystem::GetCenterOfClientWindow() const
{
	HWND* hWnd = reinterpret_cast< HWND* >( Window::GetInstance()->GetHandle() );
	
	RECT clientRect;
	::GetClientRect( *hWnd, &clientRect );

	Vector2 centerOfRect = Vector2 (
		static_cast< float >( ( clientRect.right + clientRect.left ) / 2 ),
		static_cast< float >( ( clientRect.bottom + clientRect.top ) / 2 )
	);
	return centerOfRect;
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

	for ( int i = 0; i < MouseButton::NUM_MOUSE_BUTTONS; i++ )
	{
		if ( m_mouseState[ i ].m_wasJustPressed  )
		{
			m_mouseState[ i ].m_wasJustPressed = false;
		}
		if ( m_mouseState[ i ].m_wasJustReleased  )
		{
			m_mouseState[ i ].m_wasJustReleased = false;
		}
	}
}

/* static */
InputSystem* InputSystem::GetInstance()
{
	return g_inputSystemInstance;
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
