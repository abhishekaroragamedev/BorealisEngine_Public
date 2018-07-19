#pragma once
#include "Engine/Math/Vector2.hpp"
#include <vector>

typedef bool ( *windows_message_handler_cb )( unsigned int msg, size_t wparam, size_t lparam ); 

constexpr char GAME_WINDOW_CLASS_NAME[] = "GameWindow";

class Window
{

private:
	Window( int width, int height, const char* APP_NAME );
	~Window();

public:
	// Register a function callback to the Window.  Any time Windows processing a 
	// message, this callback should forwarded the information, along with the
	// supplied user argument. 
	void RegisterHandler( windows_message_handler_cb cb ); 

	// Allow users to unregister (not needed for this Assignment, but I 
	// like having cleanup methods). 
	void UnregisterHandler( windows_message_handler_cb cb ); 

	// This is safely castable to an HWND
	void* GetHandle() const;

	std::vector< windows_message_handler_cb > GetMessageHandlers() const;

private:
	void* m_hwnd; // intptr_t  

				  // When the WindowsProcedure is called - let all listeners get first crack at the message
				  // Giving us better code modularity. 
	std::vector< windows_message_handler_cb > m_listeners; 
	int m_width = 0;
	int m_height = 0;
	float m_aspect = 0.0f;

	static Vector2 s_aspectMultipliers;

public:
	static Window* CreateAndGetInstance( int width, int height, const char* APP_NAME );
	static Window* GetInstance();
	static void DeleteInstance();
	static int GetWidth();		// The Client Width of the Window
	static int GetHeight();		// The Client Height of the Window
	static float GetWidthF();		// The Client Height of the Window
	static float GetHeightF();		// The Client Height of the Window
	static float GetAspect();
	static Vector2 GetAspectMultipliers();
};
