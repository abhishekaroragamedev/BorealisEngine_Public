#pragma once

#include "Engine/Input/KeyButtonState.hpp"
#include "Engine/Math/Transform.hpp"
#include "ThirdParty/OculusSDK/Include/OVR_CAPI.h"

enum OVRControllerType	:	int
{
	OVR_CONTROLLER_INVALID = -1,
	OVR_TOUCH_CONTROLLER_LEFT,
	OVR_TOUCH_CONTROLLER_RIGHT,
	NUM_OVR_CONTROLLER_TYPES
};

constexpr ovrButton_ OVR_LEFT_BUTTON_TYPES[ 4 ] = {
	ovrButton_X,
	ovrButton_Y,
	ovrButton_LThumb,
	ovrButton_Enter
};

constexpr ovrButton_ OVR_RIGHT_BUTTON_TYPES[ 3 ] = {
	ovrButton_A,
	ovrButton_B,
	ovrButton_RThumb
};

enum OVRButton	:	int
{
	OVR_BUTTON_INVALID = -1,
	OVR_BUTTON_A,
	OVR_BUTTON_B,
	OVR_BUTTON_X,
	OVR_BUTTON_Y,
	OVR_BUTTON_THUMB_L,
	OVR_BUTTON_THUMB_R,
	OVR_BUTTON_ENTER,
	NUM_OVR_BUTTONS
};

constexpr ovrTouch_ OVR_LEFT_TOUCH_TYPES[ 7 ] = {
	ovrTouch_X,
	ovrTouch_Y,
	ovrTouch_LThumb,
	ovrTouch_LThumbRest,
	ovrTouch_LThumbUp,
	ovrTouch_LIndexTrigger,
	ovrTouch_LIndexPointing
};

constexpr ovrTouch_ OVR_RIGHT_TOUCH_TYPES[ 7 ] = {
	ovrTouch_A,
	ovrTouch_B,
	ovrTouch_RThumb,
	ovrTouch_RThumbRest,
	ovrTouch_RThumbUp,
	ovrTouch_RIndexTrigger,
	ovrTouch_RIndexPointing
};

enum OVRGesture	:	int
{
	OVR_GESTURE_INVALID = -1,
	OVR_GESTURE_A,
	OVR_GESTURE_B,
	OVR_GESTURE_X,
	OVR_GESTURE_Y,
	OVR_GESTURE_THUMB_L,
	OVR_GESTURE_THUMB_R,
	OVR_GESTURE_THUMB_L_REST,
	OVR_GESTURE_THUMB_R_REST,
	OVR_GESTURE_THUMB_L_UP,
	OVR_GESTURE_THUMB_R_UP,
	OVR_GESTURE_INDEX_L_TRIGGER,
	OVR_GESTURE_INDEX_R_TRIGGER,
	OVR_GESTURE_INDEX_L_POINTING,
	OVR_GESTURE_INDEX_R_POINTING,
	NUM_OVR_GESTURES
};

class OVRInput
{

public:
	OVRInput( ovrSession session );
	~OVRInput();

	void BeginFrame();
	void EndFrame();

	void HandleButtonDown( OVRButton button );
	void HandleButtonUp( OVRButton button );
	void ActivateGesture( OVRGesture gesture );
	void DeactivateGesture( OVRGesture gesture );
	void SetControllerVibration( OVRControllerType controllerType, float frequency, float amplitude );	// Frequency = 0.0f or 1.0f, Amplitude = 0.0f to 1.0f
	void StopControllerVibration( OVRControllerType controllerType );
	void StopAllControllerVibrations();

	bool IsConnected( OVRControllerType controller ) const;
	Transform GetControllerTransform( OVRControllerType controller ) const;
	Transform* GetControllerTransformReference( OVRControllerType controller );
	Vector3 GetContollerLinearVelocity( OVRControllerType controller ) const;
	Vector3 GetContollerLinearAcceleration( OVRControllerType controller ) const;
	Vector3 GetContollerAngularVelocity( OVRControllerType controller ) const;
	Vector3 GetContollerAngularAcceleration( OVRControllerType controller ) const;
	bool IsButtonPressed( OVRButton button ) const;
	bool WasButtonJustPressed( OVRButton button ) const;
	bool WasButtonJustReleased( OVRButton button ) const;
	bool IsGestureActive( OVRGesture gesture ) const;
	bool WasGestureJustActivated( OVRGesture gesture ) const;
	bool WasGestureJustDeactivated( OVRGesture gesture ) const;
	float GetIndexTriggerValue( OVRControllerType controller ) const;	// [ 0.0f, 1.0f ]
	float GetHandTriggerValue( OVRControllerType controller ) const;	// [ 0.0f, 1.0f ]
	Vector2 GetJoystickPositionCartesian( OVRControllerType controller ) const;	// [ -1.0f, 1.0f ]
	Vector2 GetJoystickPositionPolar( OVRControllerType controller ) const;		// [ magnitude, degrees ]
	unsigned int GetButtonInputStateRaw( OVRControllerType controller ) const;
	unsigned int GetTouchInputStateRaw( OVRControllerType controller ) const;

private:
	void UpdateTransforms();
	void UpdateMovementInformation();
	void SetInputStateForFrame();
	void UpdateButtonState();
	void HandleButtonStateForOVRButton( unsigned int buttonState, ovrButton_ ovrButtonType );
	void UpdateGestureState();
	void HandleGestureStateForOVRTouch( unsigned int touchState, ovrTouch_ ovrTouchType );
	void UpdateTriggerValues();
	void UpdateJoystickValues();
	void ResetInputState();

	OVRButton GetButtonTypeForOVRButtonType( ovrButton_ ovrButtonType );
	OVRGesture GetGestureTypeForOVRTouchType( ovrTouch_ ovrTouchType );

private:
	// Oculus context properties
	ovrSession m_session;
	ovrResult m_ovrResult = ovrTrue;

	// Controller state
	bool m_controllerConnected[ OVRControllerType::NUM_OVR_CONTROLLER_TYPES ];
	Transform m_controllerTransform[ OVRControllerType::NUM_OVR_CONTROLLER_TYPES ];
	Vector3 m_controllerLinearVelocity[ OVRControllerType::NUM_OVR_CONTROLLER_TYPES ];
	Vector3 m_controllerLinearAcceleration[ OVRControllerType::NUM_OVR_CONTROLLER_TYPES ];
	Vector3 m_controllerAngularVelocity[ OVRControllerType::NUM_OVR_CONTROLLER_TYPES ];
	Vector3 m_controllerAngularAcceleration[ OVRControllerType::NUM_OVR_CONTROLLER_TYPES ];

	// Oculus Input state
	ovrInputState m_ovrLeftInputState;
	ovrInputState m_ovrRightInputState;

	// Engine Input state
	KeyButtonState m_buttonState[ OVRButton::NUM_OVR_BUTTONS ];
	KeyButtonState m_gestureState[ OVRGesture::NUM_OVR_GESTURES ];
	float m_indexTriggerStateNormalized[ OVRControllerType::NUM_OVR_CONTROLLER_TYPES ];
	float m_handTriggerStateNormalized[ OVRControllerType::NUM_OVR_CONTROLLER_TYPES ];
	Vector2 m_joystickPositionCartesian[ OVRControllerType::NUM_OVR_CONTROLLER_TYPES ];
	Vector2 m_joystickPositionPolar[ OVRControllerType::NUM_OVR_CONTROLLER_TYPES ];

};
