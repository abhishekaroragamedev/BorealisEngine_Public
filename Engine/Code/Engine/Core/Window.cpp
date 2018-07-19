#include "Engine/Core/Window.hpp"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

//-----------------------------------------------------------------------------------------------
// Handles Windows (Win32) messages/events; i.e. the OS is trying to tell us something happened.
// This function is called by Windows whenever we ask it for notifications
//
LRESULT CALLBACK WindowsMessageHandlingProcedure(HWND hWnd, UINT wmMessageCode, WPARAM wParam, LPARAM lParam )
{
	bool runDefault = true;

	Window* currentWindow = Window::GetInstance();
	if ( currentWindow != nullptr )
	{
		// If unhandled, Windows should run its default handler for the message
		for ( windows_message_handler_cb messageHandler : currentWindow->GetMessageHandlers() )
		{
			runDefault = messageHandler( wmMessageCode, wParam, lParam ) && runDefault;
		}
	}

	if ( runDefault )
	{
		// Send back to Windows any unhandled/unconsumed messages we want other apps to see (e.g. play/pause in music apps, etc.)
		return DefWindowProc( hWnd, wmMessageCode, wParam, lParam );
	}
	else
	{
		return false;
	}
}

static Window *g_window = nullptr; // Instance Pointer; 

Vector2 Window::s_aspectMultipliers = Vector2::ONE;

Window* Window::CreateAndGetInstance( int width, int height, const char* APP_NAME )
{
	g_window = new Window( width, height, APP_NAME );
	return g_window;
}

Window* Window::GetInstance()
{
	return g_window;
}

void Window::DeleteInstance()
{
	delete g_window;
	g_window = nullptr;
}

int Window::GetWidth()
{
	return GetInstance()->m_width;
}

int Window::GetHeight()
{
	return GetInstance()->m_height;
}

float Window::GetWidthF()
{
	return static_cast< float >( GetInstance()->m_width );
}

float Window::GetHeightF()
{
	return static_cast< float >( GetInstance()->m_height );
}

float Window::GetAspect()
{
	return GetInstance()->m_aspect;
}

Vector2 Window::GetAspectMultipliers()
{
	return Window::s_aspectMultipliers;
}

Window::Window( int width, int height, const char* APP_NAME )
{
	m_aspect = static_cast< float >( width ) / static_cast< float >( height );

	s_aspectMultipliers = Vector2( m_aspect, 1.0f );

	HINSTANCE applicationInstanceHandle = GetModuleHandle( NULL );
	// Define a window style/class
	WNDCLASSEX windowClassDescription;
	memset( &windowClassDescription, 0, sizeof( windowClassDescription ) );
	windowClassDescription.cbSize = sizeof( windowClassDescription );
	windowClassDescription.style = CS_OWNDC; // Redraw on move, request own Display Context
	windowClassDescription.lpfnWndProc = ( WindowsMessageHandlingProcedure ); // Register our Windows message-handling function
	windowClassDescription.hInstance = applicationInstanceHandle;
	windowClassDescription.hIcon = nullptr;
	windowClassDescription.hCursor = nullptr;
	windowClassDescription.lpszClassName = ( LPCWSTR ) &GAME_WINDOW_CLASS_NAME;
	RegisterClassEx( &windowClassDescription );

	// #SD1ToDo: Add support for fullscreen mode (requires different window style flags than windowed mode)
	const DWORD windowStyleFlags = WS_CAPTION | WS_BORDER | WS_THICKFRAME | WS_SYSMENU | WS_OVERLAPPED;
	const DWORD windowStyleExFlags = WS_EX_APPWINDOW;

	// Get desktop rect, dimensions, aspect
	RECT desktopRect;
	HWND desktopWindowHandle = GetDesktopWindow();
	GetClientRect( desktopWindowHandle, &desktopRect );
	float desktopWidth = (float)( desktopRect.right - desktopRect.left );
	float desktopHeight = (float)( desktopRect.bottom - desktopRect.top );
	float desktopAspect = desktopWidth / desktopHeight;

	// Calculate maximum client size (as some % of desktop size)
	constexpr float maxClientFractionOfDesktop = 0.90f;
	float clientWidth = desktopWidth * maxClientFractionOfDesktop;
	float clientHeight = desktopHeight * maxClientFractionOfDesktop;
	float clientAspect = static_cast< float >( width ) / static_cast< float >( height );
	if( clientAspect > desktopAspect )
	{
		// Client window has a wider aspect than desktop; shrink client height to match its width
		clientHeight = clientWidth / clientAspect;
	}
	else
	{
		// Client window has a taller aspect than desktop; shrink client width to match its height
		clientWidth = clientHeight * clientAspect;
	}

	// Calculate client rect bounds by centering the client area
	float clientMarginX = 0.5f * (desktopWidth - clientWidth);
	float clientMarginY = 0.5f * (desktopHeight - clientHeight);
	RECT clientRect;
	clientRect.left = (int) clientMarginX;
	clientRect.right = clientRect.left + (int) clientWidth;
	clientRect.top = (int) clientMarginY;
	clientRect.bottom = clientRect.top + (int) clientHeight;

	m_width = static_cast< int >( clientWidth );			// The actual width of the drawable space in the window
	m_height = static_cast< int >( clientHeight );		// The actual height of the drawable space in the window

	// Calculate the outer dimensions of the physical window, including frame et. al.
	RECT windowRect = clientRect;
	AdjustWindowRectEx( &windowRect, windowStyleFlags, FALSE, windowStyleExFlags );

	WCHAR windowTitle[ 1024 ];
	MultiByteToWideChar( GetACP(), 0, APP_NAME, -1, windowTitle, sizeof( windowTitle ) / sizeof( windowTitle[ 0 ] ) );
	m_hwnd = ( void* ) new HWND( CreateWindowEx(
		windowStyleExFlags,
		windowClassDescription.lpszClassName,
		windowTitle,
		windowStyleFlags,
		windowRect.left,
		windowRect.top,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		nullptr,
		nullptr,
		applicationInstanceHandle,
		nullptr ) );

	ShowWindow( *( ( HWND* ) m_hwnd ), SW_SHOW );
	SetForegroundWindow( *( ( HWND* ) m_hwnd ) );
	SetFocus( *( ( HWND* ) m_hwnd ) );

	HCURSOR cursor = LoadCursor( nullptr, IDC_ARROW );
	SetCursor( cursor );

	g_window = this;
}

Window::~Window()
{
	g_window = nullptr;
	DestroyWindow( *( reinterpret_cast< HWND* >( ( m_hwnd ) ) ) );
	UnregisterClass( ( ( LPCWSTR ) &GAME_WINDOW_CLASS_NAME ), GetModuleHandle( NULL ) );
}

void* Window::GetHandle() const
{
	return m_hwnd;
}

std::vector< windows_message_handler_cb > Window::GetMessageHandlers() const
{
	return m_listeners;
}

void Window::RegisterHandler( windows_message_handler_cb cb )
{
	m_listeners.push_back( cb );
}

void Window::UnregisterHandler( windows_message_handler_cb cb )
{
	std::vector< windows_message_handler_cb >::iterator foundCbIterator = std::find( m_listeners.begin(), m_listeners.end(), cb );
	if ( foundCbIterator != m_listeners.end() )
	{
		m_listeners.erase( foundCbIterator );
	}
}
