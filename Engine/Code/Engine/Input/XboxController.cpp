#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <Xinput.h>
#pragma comment( lib, "xinput9_1_0" )

#include "Engine/Input/XboxController.hpp"

XboxController::XboxController()
{
	m_isConnected = false;
	m_controllerId = -1;
	m_leftTriggerValueNormalized = 0.0f;
	m_rightTriggerValueNormalized = 0.0f;
	m_vibrationStateLeftMotor = 0;
	m_vibrationStateRightMotor = 0;
	ResetKeyStates();
}

XboxController::XboxController( int controllerId )
{
	m_isConnected = false;
	m_controllerId = controllerId;
	m_leftTriggerValueNormalized = 0.0f;
	m_rightTriggerValueNormalized = 0.0f;
	m_vibrationStateLeftMotor = 0;
	m_vibrationStateRightMotor = 0;
	ResetKeyStates();
}

XboxController::~XboxController()
{

}

void XboxController::BeginFrame()
{
	SetControllerState();
}

void XboxController::EndFrame()
{
	if ( m_isConnected )
	{
		ResetKeyStates();
	}
}

AnalogJoystick& XboxController::GetJoystick( int index )
{
	return m_xboxAnalogSticks[ index ];
}

bool XboxController::IsKeyDown( const unsigned char keyCode ) const
{
	return m_keyState[ keyCode ].m_isDown;
}

bool XboxController::WasKeyJustPressed( const unsigned char keyCode ) const
{
	return m_keyState[ keyCode ].m_wasJustPressed;
}

bool XboxController::WasKeyJustReleased( const unsigned char keyCode ) const
{
	return m_keyState[ keyCode ].m_wasJustReleased;
}

int XboxController::GetControllerId() const
{
	return m_controllerId;
}

bool XboxController::IsConnected() const
{
	return m_isConnected;
}

float XboxController::GetLeftTriggerValue() const
{
	return m_leftTriggerValueNormalized;
}

float XboxController::GetRightTriggerValue() const
{
	return m_rightTriggerValueNormalized;
}

bool XboxController::VibrateController( unsigned short leftMotorValue, unsigned short rightMotorValue )
{
	XINPUT_VIBRATION vibrationState;
	memset( &vibrationState, 0, sizeof( vibrationState ) );

	vibrationState.wLeftMotorSpeed = leftMotorValue;
	vibrationState.wRightMotorSpeed = rightMotorValue;
	m_vibrationStateLeftMotor = leftMotorValue;
	m_vibrationStateRightMotor = rightMotorValue;

	return XInputSetState( m_controllerId, &vibrationState );
}

bool XboxController::StopControllerVibration()
{
	XINPUT_VIBRATION vibrationState;
	memset( &vibrationState, 0, sizeof( vibrationState ) );

	vibrationState.wLeftMotorSpeed = 0;
	vibrationState.wRightMotorSpeed = 0;
	m_vibrationStateLeftMotor = 0;
	m_vibrationStateRightMotor = 0;

	return XInputSetState( m_controllerId, &vibrationState );
}

bool XboxController::StopControllerVibrationLeftMotor()
{
	XINPUT_VIBRATION vibrationState;
	memset( &vibrationState, 0, sizeof( vibrationState ) );

	vibrationState.wLeftMotorSpeed = 0;
	vibrationState.wRightMotorSpeed = m_vibrationStateRightMotor;
	m_vibrationStateLeftMotor = 0;

	return XInputSetState( m_controllerId, &vibrationState );
}

bool XboxController::StopControllerVibrationRightMotor()
{
	XINPUT_VIBRATION vibrationState;
	memset( &vibrationState, 0, sizeof( vibrationState ) );

	vibrationState.wLeftMotorSpeed = m_vibrationStateLeftMotor;
	vibrationState.wRightMotorSpeed = 0;
	m_vibrationStateRightMotor = 0;

	return XInputSetState( m_controllerId, &vibrationState );
}

void XboxController::SetControllerId( int controllerId )
{
	m_controllerId = controllerId;
}

void XboxController::SetControllerState()
{
	XINPUT_STATE controllerState;
	memset( &controllerState, 0, sizeof( controllerState ) );

	DWORD errorStatus = XInputGetState( m_controllerId, &controllerState );

	if ( errorStatus == ERROR_SUCCESS )
	{
		m_isConnected = true;

		WORD buttonState = controllerState.Gamepad.wButtons;
		SetButtonStates( buttonState );

		BYTE leftTriggerState = controllerState.Gamepad.bLeftTrigger;
		BYTE rightTriggerState = controllerState.Gamepad.bRightTrigger;
		SetTriggerStates( leftTriggerState, rightTriggerState );

		SHORT leftStickX = controllerState.Gamepad.sThumbLX;
		SHORT leftStickY = controllerState.Gamepad.sThumbLY;
		SHORT rightStickX = controllerState.Gamepad.sThumbRX;
		SHORT rightStickY = controllerState.Gamepad.sThumbRY;
		JoystickState leftJoystickState = ConstructLeftJoystickState( leftStickX, leftStickY );
		JoystickState rightJoystickState = ConstructRightJoystickState( rightStickX, rightStickY );
		m_xboxAnalogSticks[ XBOXCONTROLLER_JOYSTICK_LEFT_INDEX ].BeginFrame( leftJoystickState );
		m_xboxAnalogSticks[ XBOXCONTROLLER_JOYSTICK_RIGHT_INDEX ].BeginFrame( rightJoystickState );

		SetButtonStatesFromAnalogStates();
	}
	else
	{
		m_isConnected = false;
	}
}

void XboxController::SetButtonStates( unsigned short buttonStateBitFlag )
{
	WORD buttonStateBitMask = 1;

	for ( int i = 0; i < 14; i++ )	// 14 instead of NUM_BUTTONS because of added analog buttons
	{
		if ( i == 10 )
		{
			buttonStateBitMask *= 4;		// This is required because the A, B, X and Y buttons map from 4096 to 32768 instead of from 1024 to 8192
		}

		if ( ( buttonStateBitFlag & buttonStateBitMask ) > 0 )
		{
			if ( !m_keyState[ i ].m_isDown )
			{
				m_keyState[ i ].m_wasJustPressed = true;
			}
			m_keyState[ i ].m_isDown = true;
		}
		else
		{
			if ( m_keyState[ i ].m_isDown )
			{
				m_keyState[ i ].m_wasJustReleased = true;
			}
			m_keyState[ i ].m_isDown = false;
		}

		buttonStateBitMask *= 2;
	}
}

void XboxController::SetTriggerStates( unsigned char leftTriggerValue, unsigned char rightTriggerValue )
{
	m_leftTriggerValueNormalized = RangeMapFloat( static_cast<float> ( leftTriggerValue ), 0, 255, 0, 1 );
	m_rightTriggerValueNormalized = RangeMapFloat( static_cast<float> ( rightTriggerValue ), 0, 255, 0, 1 );
}

void XboxController::SetButtonStatesFromAnalogStates()
{
	SetTriggerButtonStatesFromAnalogStates();
	SetStickButtonStatesFromAnalogStates();
}

void XboxController::SetTriggerButtonStatesFromAnalogStates()
{
	SetButtonStateFromAnalogStateAndThreshold( InputXboxControllerMappings::TRIGGER_L, m_leftTriggerValueNormalized, XBOXCONTROLLER_TRIGGER_OFF_HIGHEST, XBOXCONTROLLER_TRIGGER_ON_LOWEST );
	SetButtonStateFromAnalogStateAndThreshold( InputXboxControllerMappings::TRIGGER_R, m_rightTriggerValueNormalized, XBOXCONTROLLER_TRIGGER_OFF_HIGHEST, XBOXCONTROLLER_TRIGGER_ON_LOWEST );
}

void XboxController::SetStickButtonStatesFromAnalogStates()
{
	float lStickXPos = m_xboxAnalogSticks[ 0 ].GetPosition().x;
	float lStickYPos = m_xboxAnalogSticks[ 0 ].GetPosition().y;
	SetButtonStateFromAnalogStateAndThreshold( InputXboxControllerMappings::ANALOG_L_LEFT, lStickXPos, XBOXCONTROLLER_ANALOG_OFF_HIGHEST, XBOXCONTROLLER_ANALOG_ON_LOWEST, true );
	SetButtonStateFromAnalogStateAndThreshold( InputXboxControllerMappings::ANALOG_L_DOWN, lStickYPos, XBOXCONTROLLER_ANALOG_OFF_HIGHEST, XBOXCONTROLLER_ANALOG_ON_LOWEST, true );
	SetButtonStateFromAnalogStateAndThreshold( InputXboxControllerMappings::ANALOG_L_RIGHT, lStickXPos, XBOXCONTROLLER_ANALOG_OFF_HIGHEST, XBOXCONTROLLER_ANALOG_ON_LOWEST );
	SetButtonStateFromAnalogStateAndThreshold( InputXboxControllerMappings::ANALOG_L_UP, lStickYPos, XBOXCONTROLLER_ANALOG_OFF_HIGHEST, XBOXCONTROLLER_ANALOG_ON_LOWEST );

	float rStickXPos = m_xboxAnalogSticks[ 1 ].GetPosition().x;
	float rStickYPos = m_xboxAnalogSticks[ 1 ].GetPosition().y;
	SetButtonStateFromAnalogStateAndThreshold( InputXboxControllerMappings::ANALOG_R_LEFT, rStickXPos, XBOXCONTROLLER_ANALOG_OFF_HIGHEST, XBOXCONTROLLER_ANALOG_ON_LOWEST, true );
	SetButtonStateFromAnalogStateAndThreshold( InputXboxControllerMappings::ANALOG_R_DOWN, rStickYPos, XBOXCONTROLLER_ANALOG_OFF_HIGHEST, XBOXCONTROLLER_ANALOG_ON_LOWEST, true );
	SetButtonStateFromAnalogStateAndThreshold( InputXboxControllerMappings::ANALOG_R_RIGHT, rStickXPos, XBOXCONTROLLER_ANALOG_OFF_HIGHEST, XBOXCONTROLLER_ANALOG_ON_LOWEST );
	SetButtonStateFromAnalogStateAndThreshold( InputXboxControllerMappings::ANALOG_R_UP, rStickYPos, XBOXCONTROLLER_ANALOG_OFF_HIGHEST, XBOXCONTROLLER_ANALOG_ON_LOWEST );
}

void XboxController::SetButtonStateFromAnalogStateAndThreshold( InputXboxControllerMappings key, float currentValue, float offThreshold, float onThreshold, bool compareNegative /* = false */ )
{
	bool releaseCondition = ( compareNegative )? ( currentValue > -offThreshold ) : ( currentValue < offThreshold );
	bool pressCondition = ( compareNegative )? ( currentValue < -onThreshold ) : ( currentValue > onThreshold );

	if ( releaseCondition )
	{
		if ( m_keyState[ key ].m_isDown )
		{
			m_keyState[ key ].m_wasJustReleased = true;
		}
		m_keyState[ key ].m_isDown = false;
	}
	else if ( pressCondition )
	{
		if ( !m_keyState[ key ].m_isDown )
		{
			m_keyState[ key ].m_wasJustPressed = true;
		}
		m_keyState[ key ].m_isDown = true;
	}
}

JoystickState XboxController::ConstructLeftJoystickState( short lStickX, short lStickY ) const
{
	JoystickState joystickState;

	joystickState.sThumbX = lStickX;
	joystickState.sThumbY = lStickY;

	return joystickState;
}

JoystickState XboxController::ConstructRightJoystickState( short rStickX, short rStickY ) const
{
	JoystickState joystickState;

	joystickState.sThumbX = rStickX;
	joystickState.sThumbY = rStickY;

	return joystickState;
}

void XboxController::ResetKeyStates()
{
	for ( int i = 0; i < InputXboxControllerMappings::NUM_BUTTONS; i++ )
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
