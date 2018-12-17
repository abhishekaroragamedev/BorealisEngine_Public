#pragma once

#include "Engine/Input/AnalogJoystick.hpp"
#include "Engine/Input/KeyButtonState.hpp"

constexpr int XBOXCONTROLLER_NUM_JOYSTICKS = 2;
constexpr int XBOXCONTROLLER_JOYSTICK_LEFT_INDEX = 0;
constexpr int XBOXCONTROLLER_JOYSTICK_RIGHT_INDEX = 1;

enum InputXboxControllerMappings
{
	DPAD_UP,
	DPAD_DOWN,
	DPAD_LEFT,
	DPAD_RIGHT,
	GAMEPAD_START,
	GAMEPAD_SELECT,
	ANALOG_CLICK_L,
	ANALOG_CLICK_R,
	GAMEPAD_LB,
	GAMEPAD_RB,
	GAMEPAD_A,
	GAMEPAD_B,
	GAMEPAD_X,
	GAMEPAD_Y,
	NUM_BUTTONS
};

class XboxController
{

public:
	XboxController();
	explicit XboxController( int controllerId );
	~XboxController();

public:
	void BeginFrame();
	void EndFrame();
	void SetControllerId( int controllerId );
	AnalogJoystick& GetJoystick( const int index );

public:
	bool IsKeyDown( const unsigned char keyCode ) const;
	bool WasKeyJustPressed( const unsigned char keyCode ) const;
	bool WasKeyJustReleased( const unsigned char keyCode ) const;
	int GetControllerId() const;
	bool IsConnected() const;
	float GetLeftTriggerValue() const;
	float GetRightTriggerValue() const;
	bool VibrateController( unsigned short leftMotorValue, unsigned short rightMotorValue ) const;
	bool StopControllerVibration() const;

protected:
	void SetControllerState();
	void SetButtonStates( unsigned short buttonState );
	void SetTriggerStates( unsigned char leftTriggerValue, unsigned char rightTriggerValue );
	void ResetKeyStates();

protected:
	JoystickState ConstructLeftJoystickState( short lStickX, short lStickY ) const;
	JoystickState ConstructRightJoystickState( short rStickX, short rStickY ) const;

protected:
	int m_controllerId;
	bool m_isConnected;
	float m_leftTriggerValueNormalized;
	float m_rightTriggerValueNormalized;
	struct KeyButtonState m_keyState[ InputXboxControllerMappings::NUM_BUTTONS ];
	AnalogJoystick m_xboxAnalogSticks[ XBOXCONTROLLER_NUM_JOYSTICKS ];

};
