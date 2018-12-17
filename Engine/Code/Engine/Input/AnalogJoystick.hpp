#pragma once

#include "Engine/Math/Vector2.hpp"

struct JoystickState
{
	float sThumbX;
	float sThumbY;
};

class AnalogJoystick
{
public:

	AnalogJoystick();
	~AnalogJoystick();

public:
	void BeginFrame( JoystickState& joystickState );
	void EndFrame();

public:
	Vector2 GetPosition() const;
	float GetMagnitude() const;
	float GetAngleDegrees() const;

protected:
	Vector2 m_rawCartesianPosition;
	Vector2 m_correctedCartesianPosition;
	float m_rawPolarMagnitude;
	float m_correctedPolarMagnitude;
	float m_rawPolarAngleDegrees;
	float m_correctedPolarAngleDegrees;

private:
	void NormalizeJoystickState( JoystickState& out_joystickState ) const;
	void ApplyDeadzoneCorrectionsAndStoreCorrectedState();
	void UpdateRawState( JoystickState joystickState );

private:
	const float JOYSTICK_MAX_VALUE = 32767.0f;
	const float JOYSTICK_MIN_VALUE = -32768.0f;
	const float INNER_DEADZONE_RADIUS_FRACTION = 0.3f;
	const float OUTER_DEADZONE_RADIUS_FRACTION = 0.1f;
	
};
