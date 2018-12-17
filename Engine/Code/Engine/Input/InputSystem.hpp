#pragma once
#include "Engine/Input/XboxController.hpp"

constexpr int KEYBOARD_NUM_STATES = 256;
constexpr int NUM_XBOXCONTROLLERS = 4;

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
	XboxController& GetController( int controllerId );

public:
	bool IsKeyDown( unsigned char keyCode ) const;
	bool WasKeyJustPressed( unsigned char keyCode ) const;
	bool WasKeyJustReleased( unsigned char keyCode ) const;

public:
	static const unsigned char KEYBOARD_ESCAPE;
	static const unsigned char KEYBOARD_RETURN;
	static const unsigned char KEYBOARD_SPACE;
	static const unsigned char KEYBOARD_TAB;
	static const unsigned char KEYBOARD_UP_ARROW;
	static const unsigned char KEYBOARD_DOWN_ARROW;
	static const unsigned char KEYBOARD_LEFT_ARROW;
	static const unsigned char KEYBOARD_RIGHT_ARROW;
	static const unsigned char KEYBOARD_SHIFT;
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

protected:
	void InitializeXboxControllers();
	void ResetKeyStates();

protected:
	
	struct KeyButtonState m_keyState[ KEYBOARD_NUM_STATES ];
	XboxController m_xboxControllers[ NUM_XBOXCONTROLLERS ];

private:
	void RunMessagePump();

};
