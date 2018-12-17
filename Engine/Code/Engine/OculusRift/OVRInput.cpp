#include "Engine/OculusRift/OVRContext.hpp"
#include "Engine/OculusRift/OVRHelper.hpp"
#include "Engine/OculusRift/OVRInput.hpp"
#include "Engine/Tools/DevConsole.hpp"

OVRInput::OVRInput( ovrSession session )
	:	m_session( session )
{

}

OVRInput::~OVRInput()
{
	StopAllControllerVibrations();
}

#pragma region BeginFrame

void OVRInput::BeginFrame()
{
	UpdateTransforms();
	UpdateMovementInformation();
	SetInputStateForFrame();
	UpdateButtonState();
	UpdateGestureState();
	UpdateTriggerValues();
	UpdateJoystickValues();
}

void OVRInput::UpdateTransforms()
{
	double displayMidpointSeconds = ovr_GetPredictedDisplayTime( m_session, OVRContext::GetFrameIndex() );
	ovrTrackingState currentTrackingState = ovr_GetTrackingState( m_session, displayMidpointSeconds, ovrTrue );

	if ( currentTrackingState.StatusFlags && ( ovrStatus_OrientationTracked | ovrStatus_PositionTracked ) )
	{
		ovrPoseStatef handPoses[ 2 ] = {
			currentTrackingState.HandPoses[ OVRControllerType::OVR_TOUCH_CONTROLLER_LEFT ],
			currentTrackingState.HandPoses[ OVRControllerType::OVR_TOUCH_CONTROLLER_RIGHT ]
		};

		ovrVector3f ovrLeftHandPosition = handPoses[ OVRControllerType::OVR_TOUCH_CONTROLLER_LEFT ].ThePose.Position;
		Vector3 leftHandPosition = GetEngineVector3FromOVRVector3( ovrLeftHandPosition );
		ovrVector3f ovrRightHandPosition = handPoses[ OVRControllerType::OVR_TOUCH_CONTROLLER_RIGHT ].ThePose.Position;
		Vector3 rightHandPosition = GetEngineVector3FromOVRVector3( ovrRightHandPosition );

		ovrQuatf ovrLeftHandOrientation = handPoses[ OVRControllerType::OVR_TOUCH_CONTROLLER_LEFT ].ThePose.Orientation;
		Vector3 leftHandEuler = GetEngineEulerFromOVRQuaternion( ovrLeftHandOrientation );
		ovrQuatf ovrRightHandOrientation = handPoses[ OVRControllerType::OVR_TOUCH_CONTROLLER_RIGHT ].ThePose.Orientation;
		Vector3 rightHandEuler = GetEngineEulerFromOVRQuaternion( ovrRightHandOrientation );

		m_controllerTransform[ OVRControllerType::OVR_TOUCH_CONTROLLER_LEFT ].SetLocalPosition( leftHandPosition );
		m_controllerTransform[ OVRControllerType::OVR_TOUCH_CONTROLLER_LEFT ].SetLocalEuler( leftHandEuler );
		m_controllerTransform[ OVRControllerType::OVR_TOUCH_CONTROLLER_RIGHT ].SetLocalPosition( rightHandPosition );
		m_controllerTransform[ OVRControllerType::OVR_TOUCH_CONTROLLER_RIGHT ].SetLocalEuler( rightHandEuler );

		m_controllerConnected[ OVRControllerType::OVR_TOUCH_CONTROLLER_LEFT ] = true;
		m_controllerConnected[ OVRControllerType::OVR_TOUCH_CONTROLLER_RIGHT ] = true;
	}
	else
	{
		m_controllerConnected[ OVRControllerType::OVR_TOUCH_CONTROLLER_LEFT ] = false;
		m_controllerConnected[ OVRControllerType::OVR_TOUCH_CONTROLLER_RIGHT ] = false;
		ConsolePrintf( Rgba::YELLOW, "OVRInput::UpdateTransform() - Position/Orientation not tracked." );
	}
}

void OVRInput::UpdateMovementInformation()
{
	double displayMidpointSeconds = ovr_GetPredictedDisplayTime( m_session, OVRContext::GetFrameIndex() );
	ovrTrackingState currentTrackingState = ovr_GetTrackingState( m_session, displayMidpointSeconds, ovrTrue );

	if ( currentTrackingState.StatusFlags && ( ovrStatus_OrientationTracked | ovrStatus_PositionTracked ) )
	{
		ovrPoseStatef handPoses[ 2 ] = {
			currentTrackingState.HandPoses[ OVRControllerType::OVR_TOUCH_CONTROLLER_LEFT ],
			currentTrackingState.HandPoses[ OVRControllerType::OVR_TOUCH_CONTROLLER_RIGHT ]
		};

		for ( int controllerIndex = 0; controllerIndex < OVRControllerType::NUM_OVR_CONTROLLER_TYPES; controllerIndex++ )
		{
			ovrVector3f ovrLinearVelocity = handPoses[ controllerIndex ].LinearVelocity;
			Vector3 linearVelocity = GetEngineVector3FromOVRVector3( ovrLinearVelocity );
			m_controllerLinearVelocity[ controllerIndex ] = linearVelocity;

			ovrVector3f ovrLinearAcceleration = handPoses[ controllerIndex ].LinearAcceleration;
			Vector3 linearAcceleration = GetEngineVector3FromOVRVector3( ovrLinearAcceleration );
			m_controllerLinearAcceleration[ controllerIndex ] = linearAcceleration;

			ovrVector3f ovrAngularVelocity = handPoses[ controllerIndex ].AngularVelocity;
			Vector3 angularVelocity = GetEngineVector3FromOVRVector3( ovrAngularVelocity );
			angularVelocity = Vector3(
				ConvertRadiansToDegrees( angularVelocity.x ),
				ConvertRadiansToDegrees( angularVelocity.y ),
				ConvertRadiansToDegrees( angularVelocity.z )
			);
			m_controllerAngularVelocity[ controllerIndex ] = angularVelocity;

			ovrVector3f ovrAngularAcceleration = handPoses[ controllerIndex ].AngularAcceleration;
			Vector3 angularAcceleration = GetEngineVector3FromOVRVector3( ovrAngularAcceleration );
			angularAcceleration = Vector3(
				ConvertRadiansToDegrees( angularAcceleration.x ),
				ConvertRadiansToDegrees( angularAcceleration.y ),
				ConvertRadiansToDegrees( angularAcceleration.z )
			);
			m_controllerAngularAcceleration[ controllerIndex ] = angularAcceleration;
		}
	}
	else
	{
		m_controllerConnected[ OVRControllerType::OVR_TOUCH_CONTROLLER_LEFT ] = false;
		m_controllerConnected[ OVRControllerType::OVR_TOUCH_CONTROLLER_RIGHT ] = false;
		ConsolePrintf( Rgba::YELLOW, "OVRInput::UpdateMovementInformation() - Position/Orientation not tracked." );
	}
}

void OVRInput::SetInputStateForFrame()
{
	m_ovrResult = ovr_GetInputState(
		m_session,
		ovrControllerType_LTouch,
		&m_ovrLeftInputState
	);
	if ( m_ovrResult == ovrSuccess )
	{
		m_controllerConnected[ OVRControllerType::OVR_TOUCH_CONTROLLER_LEFT ] = true;
	}
	else
	{
		m_controllerConnected[ OVRControllerType::OVR_TOUCH_CONTROLLER_LEFT ] = false;
		ConsolePrintf( Rgba::YELLOW, "OVRInput::UpdateButtonState() - Unable to obtain input from Left touch controller." );
	}

	m_ovrResult = ovr_GetInputState(
		m_session,
		ovrControllerType_RTouch,
		&m_ovrRightInputState
	);
	if ( m_ovrResult == ovrSuccess )
	{
		m_controllerConnected[ OVRControllerType::OVR_TOUCH_CONTROLLER_RIGHT ] = true;
	}
	else
	{
		m_controllerConnected[ OVRControllerType::OVR_TOUCH_CONTROLLER_RIGHT ] = false;
		ConsolePrintf( Rgba::YELLOW, "OVRInput::UpdateButtonState() - Unable to obtain input from Right touch controller." );
	}
}

void OVRInput::UpdateButtonState()
{
	if ( m_ovrResult == ovrSuccess )
	{
		for ( ovrButton_ button : OVR_LEFT_BUTTON_TYPES )
		{
			HandleButtonStateForOVRButton( m_ovrLeftInputState.Buttons, button );
		}
		for ( ovrButton_ button : OVR_RIGHT_BUTTON_TYPES )
		{
			HandleButtonStateForOVRButton( m_ovrRightInputState.Buttons, button );
		}
	}
}

void OVRInput::HandleButtonStateForOVRButton( unsigned int buttonState, ovrButton_ ovrButtonType )
{
	OVRButton buttonType = GetButtonTypeForOVRButtonType( ovrButtonType );
	if ( buttonState & ovrButtonType )
	{
		HandleButtonDown( buttonType );
	}
	else
	{
		HandleButtonUp( buttonType );
	}
}

void OVRInput::UpdateGestureState()
{
	if ( m_ovrResult == ovrSuccess )
	{
		for ( ovrTouch_ ovrTouch : OVR_LEFT_TOUCH_TYPES )
		{
			HandleGestureStateForOVRTouch( m_ovrLeftInputState.Touches, ovrTouch );
		}
		for ( ovrTouch_ ovrTouch : OVR_RIGHT_TOUCH_TYPES )
		{
			HandleGestureStateForOVRTouch( m_ovrRightInputState.Touches, ovrTouch );
		}
	}
}

void OVRInput::HandleGestureStateForOVRTouch( unsigned int touchState, ovrTouch_ ovrTouchType )
{
	OVRGesture gestureType = GetGestureTypeForOVRTouchType( ovrTouchType );
	if ( touchState & ovrTouchType )
	{
		ActivateGesture( gestureType );
	}
	else
	{
		DeactivateGesture( gestureType );
	}
}

void OVRInput::UpdateTriggerValues()
{
	if ( m_ovrResult == ovrSuccess )
	{
		m_indexTriggerStateNormalized[ OVRControllerType::OVR_TOUCH_CONTROLLER_LEFT ] = m_ovrLeftInputState.IndexTrigger[ OVRControllerType::OVR_TOUCH_CONTROLLER_LEFT ];
		m_indexTriggerStateNormalized[ OVRControllerType::OVR_TOUCH_CONTROLLER_RIGHT ] = m_ovrRightInputState.IndexTrigger[ OVRControllerType::OVR_TOUCH_CONTROLLER_RIGHT ];
		m_handTriggerStateNormalized[ OVRControllerType::OVR_TOUCH_CONTROLLER_LEFT ] = m_ovrLeftInputState.HandTrigger[ OVRControllerType::OVR_TOUCH_CONTROLLER_LEFT ];
		m_handTriggerStateNormalized[ OVRControllerType::OVR_TOUCH_CONTROLLER_RIGHT ] = m_ovrRightInputState.HandTrigger[ OVRControllerType::OVR_TOUCH_CONTROLLER_RIGHT ];
	}
}

void OVRInput::UpdateJoystickValues()
{
	if ( m_ovrResult == ovrSuccess )
	{
		ovrVector2f joystickPositionRaw = m_ovrLeftInputState.Thumbstick[ OVRControllerType::OVR_TOUCH_CONTROLLER_LEFT ];
		m_joystickPositionCartesian[ OVRControllerType::OVR_TOUCH_CONTROLLER_LEFT ] = GetEngineVector2FromOVRVector2( joystickPositionRaw );
		joystickPositionRaw = m_ovrRightInputState.Thumbstick[ OVRControllerType::OVR_TOUCH_CONTROLLER_RIGHT ];
		m_joystickPositionCartesian[ OVRControllerType::OVR_TOUCH_CONTROLLER_RIGHT ] = GetEngineVector2FromOVRVector2( joystickPositionRaw );
	}
}

#pragma endregion

#pragma region EndFrame

void OVRInput::EndFrame()
{
	ResetInputState();
}

void OVRInput::ResetInputState()
{
	for ( int buttonIndex = 0; buttonIndex < OVRButton::NUM_OVR_BUTTONS; buttonIndex++ )
	{
		m_buttonState[ buttonIndex ].m_wasJustPressed = false;
		m_buttonState[ buttonIndex ].m_wasJustPressed = false;
	}
	for ( int gestureIndex = 0; gestureIndex < OVRGesture::NUM_OVR_GESTURES; gestureIndex++ )
	{
		m_gestureState[ gestureIndex ].m_wasJustPressed = false;
		m_gestureState[ gestureIndex ].m_wasJustPressed = false;
	}
	for ( int controllerIndex = 0; controllerIndex < OVRControllerType::NUM_OVR_CONTROLLER_TYPES; controllerIndex++ )
	{
		m_indexTriggerStateNormalized[ controllerIndex ] = 0.0f;
		m_handTriggerStateNormalized[ controllerIndex ] = 0.0f;
		m_joystickPositionCartesian[ controllerIndex ] = Vector2::ZERO;
		m_joystickPositionPolar[ controllerIndex ] = Vector2::ZERO;
	}
}

#pragma endregion

#pragma region ButtonStateControls

void OVRInput::HandleButtonDown( OVRButton button )
{
	if ( !m_buttonState[ button ].m_isDown )
	{
		m_buttonState[ button ].m_wasJustPressed = true;
	}
	m_buttonState[ button ].m_wasJustReleased = false;
	m_buttonState[ button ].m_isDown = true;
}

void OVRInput::HandleButtonUp( OVRButton button )
{
	if ( m_buttonState[ button ].m_isDown )
	{
		m_buttonState[ button ].m_wasJustReleased = true;
	}
	m_buttonState[ button ].m_wasJustPressed = false;
	m_buttonState[ button ].m_isDown = false;
}

void OVRInput::ActivateGesture( OVRGesture gesture )
{
	if ( m_gestureState[ gesture ].m_isDown )
	{
		m_gestureState[ gesture ].m_wasJustReleased = true;
	}

	m_gestureState[ gesture ].m_isDown = false;
}

void OVRInput::DeactivateGesture( OVRGesture gesture )
{
	if ( m_gestureState[ gesture ].m_isDown )
	{
		m_gestureState[ gesture ].m_wasJustReleased = true;
	}

	m_gestureState[ gesture ].m_isDown = false;
}

void OVRInput::SetControllerVibration( OVRControllerType controllerType, float frequency, float amplitude )
{
	ovrControllerType controllerToSet = GetOVRControllerTypeForEngineControllerType( controllerType );
	
	frequency = ClampFloat( frequency, 0.0f, 1.0f );
	frequency = frequency * 2.0f;
	frequency = Floor( frequency );	// Only allowed values are 0.0f, 0.5f and 1.0f, so mapping the input value to one of these

	amplitude = ClampFloat( amplitude, 0.0f, 1.0f );
	
	ovr_SetControllerVibration( m_session, controllerToSet, frequency, amplitude );
}

void OVRInput::StopControllerVibration( OVRControllerType controllerType )
{
	SetControllerVibration( controllerType, 0.0f, 0.0f );
}

void OVRInput::StopAllControllerVibrations()
{
	StopControllerVibration( OVRControllerType::OVR_TOUCH_CONTROLLER_LEFT );
	StopControllerVibration( OVRControllerType::OVR_TOUCH_CONTROLLER_RIGHT );
}

#pragma endregion

#pragma region Getters

bool OVRInput::IsConnected( OVRControllerType controller ) const
{
	return m_controllerConnected[ controller ];
}

Transform OVRInput::GetControllerTransform( OVRControllerType controller ) const
{
	return m_controllerTransform[ controller ];
}

Transform* OVRInput::GetControllerTransformReference( OVRControllerType controller )
{
	return &m_controllerTransform[ controller ];
}

Vector3 OVRInput::GetContollerLinearVelocity( OVRControllerType controller ) const
{
	return m_controllerLinearVelocity[ controller ];
}

Vector3 OVRInput::GetContollerLinearAcceleration( OVRControllerType controller ) const
{
	return m_controllerLinearAcceleration[ controller ];
}

Vector3 OVRInput::GetContollerAngularVelocity( OVRControllerType controller ) const
{
	return m_controllerAngularVelocity[ controller ];
}

Vector3 OVRInput::GetContollerAngularAcceleration( OVRControllerType controller ) const
{
	return m_controllerAngularAcceleration[ controller ];
}

bool OVRInput::IsButtonPressed( OVRButton button ) const
{
	return m_buttonState[ button ].m_isDown;
}

bool OVRInput::WasButtonJustPressed( OVRButton button ) const
{
	return m_buttonState[ button ].m_wasJustPressed;
}

bool OVRInput::WasButtonJustReleased( OVRButton button ) const
{
	return m_buttonState[ button ].m_wasJustReleased;
}

bool OVRInput::IsGestureActive( OVRGesture gesture ) const
{
	return m_gestureState[ gesture ].m_isDown;
}

bool OVRInput::WasGestureJustActivated( OVRGesture gesture ) const
{
	return m_gestureState[ gesture ].m_wasJustPressed;
}

bool OVRInput::WasGestureJustDeactivated( OVRGesture gesture ) const
{
	return m_gestureState[ gesture ].m_wasJustReleased;
}

float OVRInput::GetIndexTriggerValue( OVRControllerType controller ) const
{
	return m_indexTriggerStateNormalized[ controller ];
}

float OVRInput::GetHandTriggerValue( OVRControllerType controller ) const
{
	return m_handTriggerStateNormalized[ controller ];
}

Vector2 OVRInput::GetJoystickPositionCartesian( OVRControllerType controller ) const
{
	return m_joystickPositionCartesian[ controller ];
}

Vector2 OVRInput::GetJoystickPositionPolar( OVRControllerType controller ) const
{
	return m_joystickPositionPolar[ controller ];
}

unsigned int OVRInput::GetButtonInputStateRaw( OVRControllerType controller ) const
{
	switch( controller )
	{
		case OVRControllerType::OVR_TOUCH_CONTROLLER_LEFT	:	return m_ovrLeftInputState.Buttons;
		case OVRControllerType::OVR_TOUCH_CONTROLLER_RIGHT	:	return m_ovrRightInputState.Buttons;
		default												:	return 0U;
	}
}

unsigned int OVRInput::GetTouchInputStateRaw( OVRControllerType controller ) const
{
	switch( controller )
	{
		case OVRControllerType::OVR_TOUCH_CONTROLLER_LEFT	:	return m_ovrLeftInputState.Touches;
		case OVRControllerType::OVR_TOUCH_CONTROLLER_RIGHT	:	return m_ovrRightInputState.Touches;
		default												:	return 0U;
	}
}

OVRButton OVRInput::GetButtonTypeForOVRButtonType( ovrButton_ ovrButtonType )
{
	switch ( ovrButtonType )
	{
		case ovrButton_A		:	return OVRButton::OVR_BUTTON_A;
		case ovrButton_B		:	return OVRButton::OVR_BUTTON_B;
		case ovrButton_X		:	return OVRButton::OVR_BUTTON_X;
		case ovrButton_Y		:	return OVRButton::OVR_BUTTON_Y;
		case ovrButton_LThumb	:	return OVRButton::OVR_BUTTON_THUMB_L;
		case ovrButton_RThumb	:	return OVRButton::OVR_BUTTON_THUMB_R;
		case ovrButton_Enter	:	return OVRButton::OVR_BUTTON_ENTER;
		default					:	return OVRButton::OVR_BUTTON_INVALID;
	}
}

OVRGesture OVRInput::GetGestureTypeForOVRTouchType( ovrTouch_ ovrTouchType )
{
	switch ( ovrTouchType )
	{
		case ovrTouch_A					:	return OVRGesture::OVR_GESTURE_A;
		case ovrTouch_B					:	return OVRGesture::OVR_GESTURE_B;
		case ovrTouch_X					:	return OVRGesture::OVR_GESTURE_X;
		case ovrTouch_Y					:	return OVRGesture::OVR_GESTURE_Y;
		case ovrTouch_LThumb			:	return OVRGesture::OVR_GESTURE_THUMB_L;
		case ovrTouch_RThumb			:	return OVRGesture::OVR_GESTURE_THUMB_R;
		case ovrTouch_LThumbRest		:	return OVRGesture::OVR_GESTURE_THUMB_L_REST;
		case ovrTouch_RThumbRest		:	return OVRGesture::OVR_GESTURE_THUMB_R_REST;
		case ovrTouch_LThumbUp			:	return OVRGesture::OVR_GESTURE_THUMB_L_UP;
		case ovrTouch_RThumbUp			:	return OVRGesture::OVR_GESTURE_THUMB_R_UP;
		case ovrTouch_LIndexTrigger		:	return OVRGesture::OVR_GESTURE_INDEX_L_TRIGGER;
		case ovrTouch_RIndexTrigger		:	return OVRGesture::OVR_GESTURE_INDEX_R_TRIGGER;
		case ovrTouch_LIndexPointing	:	return OVRGesture::OVR_GESTURE_INDEX_L_POINTING;
		case ovrTouch_RIndexPointing	:	return OVRGesture::OVR_GESTURE_INDEX_R_POINTING;
		default							:	return OVRGesture::OVR_GESTURE_INVALID;
	}
}

#pragma endregion
