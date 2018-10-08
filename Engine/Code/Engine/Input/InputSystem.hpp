#pragma once
#include "Engine/Input/XboxController.hpp"

constexpr int KEYBOARD_NUM_STATES = 256;
constexpr int NUM_XBOXCONTROLLERS = 4;

enum MouseDeltaMode
{
	MOUSE_MODE_INVALID = -1,
	MOUSE_MODE_ABSOLUTE,
	MOUSE_MODE_RELATIVE
};

enum MouseButton : int
{
	MOUSE_BUTTON_INVALID = -1,
	LEFT_MOUSE_BUTTON,
	MIDDLE_MOUSE_BUTTON,
	RIGHT_MOUSE_BUTTON,
	NUM_MOUSE_BUTTONS
};

class InputSystem
{

public:
	InputSystem();
	~InputSystem();

public:
	void BeginFrame();
	void EndFrame();
	void HandleKeyDown( unsigned int keyCode );
	void HandleKeyUp( unsigned int keyCode );
	void HandleMouseButtonDown( unsigned int wParam );
	void HandleMouseButtonUp( MouseButton button );
	void LockMouseToScreen( bool lock );
	void ShowCursor( bool show );
	void SetMouseScreenPosition( const Vector2& desktopPosition );
	void SetMousePosition( const Vector2& clientPosition );
	void AddMouseWheelDelta( float delta );
	void SetMouseDeltaMode( MouseDeltaMode mode );
	void RecomputeClientRect();	// Resets the window rect and calls any mouse functions that may change with the new rect
	void SetWindowActive( bool active );

public:
	XboxController& GetController( int controllerId );
	Vector2 GetMouseClientPosition() const;
	Vector2 GetMouseClientDelta() const;
	float GetMouseWheelDelta() const;
	bool IsKeyDown( unsigned char keyCode ) const;
	bool WasKeyJustPressed( unsigned char keyCode ) const;
	bool WasKeyJustReleased( unsigned char keyCode ) const;
	bool IsMouseButtonDown( MouseButton button ) const;
	bool WasMouseButtonJustPressed( MouseButton button ) const;
	bool WasMouseButtonJustReleased( MouseButton button ) const;
	bool IsCursorVisible() const;
	MouseDeltaMode GetMouseDeltaMode() const;
	MouseButton GetMouseButtonForWParam( unsigned int wParam ) const;

public:
	static const unsigned char KEYBOARD_ESCAPE;
	static const unsigned char KEYBOARD_RETURN;
	static const unsigned char KEYBOARD_SPACE;
	static const unsigned char KEYBOARD_BACKSPACE;
	static const unsigned char KEYBOARD_DELETE;
	static const unsigned char KEYBOARD_TAB;
	static const unsigned char KEYBOARD_TILDE;
	static const unsigned char KEYBOARD_UP_ARROW;
	static const unsigned char KEYBOARD_DOWN_ARROW;
	static const unsigned char KEYBOARD_LEFT_ARROW;
	static const unsigned char KEYBOARD_RIGHT_ARROW;
	static const unsigned char KEYBOARD_PAGE_UP;
	static const unsigned char KEYBOARD_PAGE_DOWN;
	static const unsigned char KEYBOARD_SHIFT;
	static const unsigned char KEYBOARD_CTRL_A;
	static const unsigned char KEYBOARD_CTRL_C;
	static const unsigned char KEYBOARD_CTRL_V;
	static const unsigned char KEYBOARD_CTRL_X;
	static const unsigned char KEYBOARD_LEFT_SQUARE_BRACKET;
	static const unsigned char KEYBOARD_RIGHT_SQUARE_BRACKET;
	static const unsigned char KEYBOARD_F1;
	static const unsigned char KEYBOARD_F2;
	static const unsigned char KEYBOARD_F3;
	static const unsigned char KEYBOARD_F4;
	static const unsigned char KEYBOARD_F5;
	static const unsigned char KEYBOARD_F6;
	static const unsigned char KEYBOARD_F7;
	static const unsigned char KEYBOARD_F8;
	static const unsigned char KEYBOARD_F9;
	static const unsigned char KEYBOARD_F10;
	static const unsigned char KEYBOARD_F11;
	static const unsigned char KEYBOARD_F12;
	static const unsigned char KEYBOARD_A;
	static const unsigned char KEYBOARD_B;
	static const unsigned char KEYBOARD_C;
	static const unsigned char KEYBOARD_D;
	static const unsigned char KEYBOARD_E;
	static const unsigned char KEYBOARD_F;
	static const unsigned char KEYBOARD_G;
	static const unsigned char KEYBOARD_H;
	static const unsigned char KEYBOARD_I;
	static const unsigned char KEYBOARD_J;
	static const unsigned char KEYBOARD_K;
	static const unsigned char KEYBOARD_L;
	static const unsigned char KEYBOARD_M;
	static const unsigned char KEYBOARD_N;
	static const unsigned char KEYBOARD_O;
	static const unsigned char KEYBOARD_P;
	static const unsigned char KEYBOARD_Q;
	static const unsigned char KEYBOARD_R;
	static const unsigned char KEYBOARD_S;
	static const unsigned char KEYBOARD_T;
	static const unsigned char KEYBOARD_U;
	static const unsigned char KEYBOARD_V;
	static const unsigned char KEYBOARD_W;
	static const unsigned char KEYBOARD_X;
	static const unsigned char KEYBOARD_Y;
	static const unsigned char KEYBOARD_Z;
	static const unsigned char KEYBOARD_0;
	static const unsigned char KEYBOARD_1;
	static const unsigned char KEYBOARD_2;
	static const unsigned char KEYBOARD_3;
	static const unsigned char KEYBOARD_4;
	static const unsigned char KEYBOARD_5;
	static const unsigned char KEYBOARD_6;
	static const unsigned char KEYBOARD_7;
	static const unsigned char KEYBOARD_8;
	static const unsigned char KEYBOARD_9;

	static InputSystem* GetInstance();

protected:
	void InitializeXboxControllers();
	void ResetKeyStates();
	void SetMouseFramePositions();
	void ResetMouseFramePositionsToCenter();

	Vector2 GetCenterOfClientWindow() const;

protected:
	
	struct KeyButtonState m_keyState[ KEYBOARD_NUM_STATES ];
	struct KeyButtonState m_mouseState[ MouseButton::NUM_MOUSE_BUTTONS ];
	XboxController m_xboxControllers[ NUM_XBOXCONTROLLERS ];

	Vector2 m_mousePositionLastFrame = Vector2::ZERO;
	Vector2 m_mousePositionThisFrame = Vector2::ZERO;
	MouseDeltaMode m_mouseMode = MouseDeltaMode::MOUSE_MODE_ABSOLUTE;
	bool m_isWindowActive = true;
	bool m_mouseLockedThisFrame = false;
	float m_mouseWheelDelta = 0.0f;
	bool m_isCursorVisible = true;

private:
	void RunMessagePump();

};
