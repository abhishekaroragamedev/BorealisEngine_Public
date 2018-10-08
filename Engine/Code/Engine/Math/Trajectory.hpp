#pragma once

#include "Engine/Math/Vector2.hpp"

class Trajectory	// A collection of static methods to evaluate various unknowns in parabolic trajectories
{

public:
	static Vector2 Evaluate( float gravity, Vector2 launchVelocity, float time );	// f(t) = Vector2[ ( v.x * t ), ( -0.5 * g * t + v.y * t ) ]
	static Vector2 Evaluate( float gravity, float launchSpeed, float launchAngle, float time );	// f(t) = Vector2[ ( s * cos( a ) * t ), ( -0.5 * g * t + s * sin( a ) * t ) ]
	static float GetMinimumLaunchSpeed( float gravity, float distance );		// Provides the launch speed required at the optimal angle of 45 degrees
	static bool GetLaunchAngles( Vector2& out_angles, float gravity, float launchSpeed, float distance, float heightDifference = 0.0f );	// Possible launch angle(s) given a target and launch speed
	static float GetMaxHeight( float gravity, float launchSpeed, float distance );	// Maximum height attainable by adjusting the angle with a fixed gravity, launchSpeed and distance
	static Vector2 GetLaunchVelocity( float gravity, float apexHeight, float distance, float targetHeight );	// Apex height must be greater than or equal to Target height

};