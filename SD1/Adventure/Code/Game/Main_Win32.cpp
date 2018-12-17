#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cassert>
#include <crtdbg.h>
#include "Game/TheApp.hpp"
#include "Game/GameCommon.hpp"

HWND* g_windowHandle = nullptr;
HDC* g_deviceDisplayContext = nullptr;

void CreateOpenGLWindow( HINSTANCE applicationInstanceHandle, const float clientAspect, LPCCH APP_NAME, WNDPROC WindowsMessageHandlingProcedure )
{
	// Define a window style/class
	WNDCLASSEX windowClassDescription;
	memset( &windowClassDescription, 0, sizeof( windowClassDescription ) );
	windowClassDescription.cbSize = sizeof( windowClassDescription );
	windowClassDescription.style = CS_OWNDC; // Redraw on move, request own Display Context
	windowClassDescription.lpfnWndProc = ( WindowsMessageHandlingProcedure ); // Register our Windows message-handling function
	windowClassDescription.hInstance = GetModuleHandle( nullptr );
	windowClassDescription.hIcon = nullptr;
	windowClassDescription.hCursor = nullptr;
	windowClassDescription.lpszClassName = TEXT( "Simple Window Class" );
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

	// Calculate the outer dimensions of the physical window, including frame et. al.
	RECT windowRect = clientRect;
	AdjustWindowRectEx( &windowRect, windowStyleFlags, FALSE, windowStyleExFlags );

	WCHAR windowTitle[ 1024 ];
	MultiByteToWideChar( GetACP(), 0, APP_NAME, -1, windowTitle, sizeof( windowTitle ) / sizeof( windowTitle[ 0 ] ) );
	g_windowHandle = new HWND( CreateWindowEx(
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

	ShowWindow( *g_windowHandle, SW_SHOW );
	SetForegroundWindow( *g_windowHandle );
	SetFocus( *g_windowHandle );

	g_deviceDisplayContext = new HDC ( GetDC( *g_windowHandle ) );

	HCURSOR cursor = LoadCursor( nullptr, IDC_ARROW );
	SetCursor( cursor );

	PIXELFORMATDESCRIPTOR pixelFormatDescriptor;
	memset( &pixelFormatDescriptor, 0, sizeof( pixelFormatDescriptor ) );
	pixelFormatDescriptor.nSize = sizeof( pixelFormatDescriptor );
	pixelFormatDescriptor.nVersion = 1;
	pixelFormatDescriptor.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pixelFormatDescriptor.iPixelType = PFD_TYPE_RGBA;
	pixelFormatDescriptor.cColorBits = 24;
	pixelFormatDescriptor.cDepthBits = 24;
	pixelFormatDescriptor.cAccumBits = 0;
	pixelFormatDescriptor.cStencilBits = 8;

	int pixelFormatCode = ChoosePixelFormat( *g_deviceDisplayContext, &pixelFormatDescriptor );
	SetPixelFormat( *g_deviceDisplayContext, pixelFormatCode, &pixelFormatDescriptor );
	HGLRC openGlRenderingContext = wglCreateContext( *g_deviceDisplayContext );
	g_renderer = new Renderer();
	wglMakeCurrent( *g_deviceDisplayContext, openGlRenderingContext );

	g_renderer->InitializeRenderer();
}

//-----------------------------------------------------------------------------------------------
// Handles Windows (Win32) messages/events; i.e. the OS is trying to tell us something happened.
// This function is called by Windows whenever we ask it for notifications
//
LRESULT CALLBACK WindowsMessageHandlingProcedure(HWND hWnd, UINT wmMessageCode, WPARAM wParam, LPARAM lParam )
{
	switch( wmMessageCode )
	{
		// App close requested via "X" button, or right-click "Close Window" on task bar, or "Close" from system menu, or Alt-F4
	case WM_CLOSE:		
	{
		// Handles the same way as pressing escape
		g_inputSystem->HandleKeyDown( VK_ESCAPE );
		return 0; // "Consumes" this message (tells Windows "okay, we handled it")
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
		//			unsigned char asKey = (unsigned char) wParam;
		break;
	}
	}

	// Send back to Windows any unhandled/unconsumed messages we want other apps to see (e.g. play/pause in music apps, etc.)
	return DefWindowProc( hWnd, wmMessageCode, wParam, lParam );
}

//-----------------------------------------------------------------------------------------------
int WINAPI WinMain( HINSTANCE applicationInstanceHandle, HINSTANCE, LPSTR commandLineString, int )
{
	UNUSED( commandLineString );
	CreateOpenGLWindow( applicationInstanceHandle, CLIENT_ASPECT, ADVENTURE_APP_NAME, WindowsMessageHandlingProcedure );
	TheApp* g_theApp = new TheApp();

	while( !g_theApp->IsQuitting() )		// Program main loop; keep running frames until it's time to quit
	{
		g_theApp->RunFrame();
	}

	delete g_theApp;
	g_theApp = nullptr;

	return 0;
}
