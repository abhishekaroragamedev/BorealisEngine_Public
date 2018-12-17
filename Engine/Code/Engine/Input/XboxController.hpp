#pragma once

#include "Engine/Input/AnalogJoystick.hpp"
#include "Engine/Input/KeyButtonState.hpp"

constexpr int XBOXCONTROLLER_NUM_JOYSTICKS = 2;
constexpr int XBOXCONTROLLER_JOYSTICK_LEFT_INDEX = 0;
constexpr int XBOXCONTROLLER_JOYSTICK_RIGHT_INDEX = 1;
constexpr float XBOXCONTROLLER_TRIGGER_OFF_HIGHEST = 0.45f;
constexpr float XBOXCONTROLLER_TRIGGER_ON_LOWEST = 0.55f;
constexpr float XBOXCONTROLLER_ANALOG_OFF_HIGHEST = 0.9f;
constexpr float XBOXCONTROLLER_ANALOG_ON_LOWEST = 0.95f;

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
	ANALOG_L_LEFT,
	ANALOG_L_DOWN,
	ANALOG_L_RIGHT,
	ANALOG_L_UP,
	ANALOG_R_LEFT,
	ANALOG_R_DOWN,
	ANALOG_R_RIGHT,
	ANALOG_R_UP,
	TRIGGER_L,
	TRIGGER_R,
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
	bool VibrateController( unsigned short leftMotorValue, unsigned short rightMotorValue );
	bool StopControllerVibration();
	bool StopControllerVibrationLeftMotor();
	bool StopControllerVibrationRightMotor();

protected:
	void SetControllerState();
	void SetButtonStates( unsigned short buttonState );
	void SetTriggerStates( unsigned char leftTriggerValue, unsigned char rightTriggerValue );
	void SetButtonStatesFromAnalogStates();
	void SetTriggerButtonStatesFromAnalogStates();
	void SetStickButtonStatesFromAnalogStates();
	void SetButtonStateFromAnalogStateAndThreshold( InputXboxControllerMappings key, float currentValue, float offThreshold, float onThreshold, bool compareNegative = false );
	void ResetKeyStates();

protected:
	JoystickState ConstructLeftJoystickState( short lStickX, short lStickY ) const;
	JoystickState ConstructRightJoystickState( short rStickX, short rStickY ) const;

protected:
	int m_controllerId = 0;
	bool m_isConnected = false;
	float m_leftTriggerValueNormalized = 0.0f;
	float m_rightTriggerValueNormalized = 0.0f;
	unsigned short m_vibrationStateLeftMotor = 0;
	unsigned short m_vibrationStateRightMotor = 0;
	struct KeyButtonState m_keyState[ InputXboxControllerMappings::NUM_BUTTONS ];
	AnalogJoystick m_xboxAnalogSticks[ XBOXCONTROLLER_NUM_JOYSTICKS ];

};
