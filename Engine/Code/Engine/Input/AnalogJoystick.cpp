#include "Engine/Input/AnalogJoystick.hpp"

AnalogJoystick::AnalogJoystick()
{
	m_rawCartesianPosition = Vector2( 0.0f, 0.0f );
	m_correctedCartesianPosition = Vector2( 0.0f, 0.0f );
	m_rawPolarMagnitude = 0.0f;
	m_correctedPolarMagnitude = 0.0f;
	m_rawPolarAngleDegrees = 0.0f;
	m_correctedPolarAngleDegrees = 0.0f;
}

AnalogJoystick::~AnalogJoystick()
{

}

void AnalogJoystick::BeginFrame( JoystickState& joystickState )
{
	NormalizeJoystickState( joystickState );
	UpdateRawState( joystickState );
	ApplyDeadzoneCorrectionsAndStoreCorrectedState();
}

void AnalogJoystick::EndFrame()
{

}

Vector2 AnalogJoystick::GetPosition() const
{
	return m_correctedCartesianPosition;
}

float AnalogJoystick::GetMagnitude() const
{
	return m_correctedPolarMagnitude;
}

float AnalogJoystick::GetAngleDegrees() const
{
	return m_correctedPolarAngleDegrees;
}

void AnalogJoystick::NormalizeJoystickState( JoystickState& out_joystickState ) const
{
	out_joystickState.sThumbX = RangeMapFloat( out_joystickState.sThumbX, JOYSTICK_MIN_VALUE, JOYSTICK_MAX_VALUE, -1.0f, 1.0f );
	out_joystickState.sThumbY = RangeMapFloat( out_joystickState.sThumbY, JOYSTICK_MIN_VALUE, JOYSTICK_MAX_VALUE, -1.0f, 1.0f );
}

void AnalogJoystick::ApplyDeadzoneCorrectionsAndStoreCorrectedState()
{
	if ( m_rawPolarMagnitude <= INNER_DEADZONE_RADIUS_FRACTION )						// Stick is within inner deadzone
	{
		m_correctedCartesianPosition = Vector2( 0.0f, 0.0f );
		m_correctedPolarMagnitude = 0.0f;
		m_correctedPolarAngleDegrees = 0.0f;
	}
	else if ( m_rawPolarMagnitude >= ( 1.0f - OUTER_DEADZONE_RADIUS_FRACTION ) )		// Stick is outside outer deadzone
	{
		m_correctedPolarMagnitude = 1.0f;
		m_correctedPolarAngleDegrees = m_rawPolarAngleDegrees;
		m_correctedCartesianPosition = Vector2( m_correctedPolarMagnitude, m_correctedPolarAngleDegrees );
		m_correctedCartesianPosition.ConvertToCartestian();
	}
	else																		// Stick is in between inner and outer deadzones
	{
		m_correctedPolarMagnitude = m_rawPolarMagnitude;
		m_correctedPolarAngleDegrees = m_rawPolarAngleDegrees;
		m_correctedCartesianPosition = m_rawCartesianPosition;
	}
}

void AnalogJoystick::UpdateRawState( JoystickState joystickState )
{
	m_rawCartesianPosition = Vector2( joystickState.sThumbX, joystickState.sThumbY );

	Vector2 rawPolarCoordinates = Vector2( m_rawCartesianPosition );
	rawPolarCoordinates.ConvertToPolar();

	m_rawPolarMagnitude = rawPolarCoordinates.x;
	m_rawPolarAngleDegrees = rawPolarCoordinates.y;
}
