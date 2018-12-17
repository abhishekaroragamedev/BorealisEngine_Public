#pragma once

#include "Engine/Math/MathUtils.hpp"

//-----------------------------------------------------------------------------------------------
class Vector2
{

public:
	// Construction/Destruction
	~Vector2() {}											// destructor: do nothing (for speed)
	Vector2() {}											// default constructor: do nothing (for speed)
	Vector2( const Vector2& copyFrom );						// copy constructor (from another vec2)
	explicit Vector2( float initialX, float initialY );		// explicit constructor (from x, y)

public:
	void ConvertToPolar();		// Converts to a vector (R, theta), where R = magnitude and theta = orientation in degrees
	void ConvertToCartestian();
	void SetOrientationDegrees( float degrees );
	void RotateDegrees( float deltaDegrees );

public:
	float GetLength() const;
	float GetLengthSquared() const;
	float NormalizeAndGetLength();
	Vector2 GetNormalized() const;
	float GetOrientationDegrees() const;

public:																// Operators
	const Vector2 operator+( const Vector2& vecToAdd ) const;		// vec2 + vec2
	const Vector2 operator-( const Vector2& vecToSubtract ) const;	// vec2 - vec2
	const Vector2 operator*( float uniformScale ) const;			// vec2 * float
	const Vector2 operator/( float inverseScale ) const;			// vec2 / float
	void operator+=( const Vector2& vecToAdd );						// vec2 += vec2
	void operator-=( const Vector2& vecToSubtract );				// vec2 -= vec2
	void operator*=( const float uniformScale );					// vec2 *= float
	void operator/=( const float uniformDivisor );					// vec2 /= float
	void operator=( const Vector2& copyFrom );						// vec2 = vec2
	bool operator==( const Vector2& compare ) const;				// vec2 == vec2
	bool operator!=( const Vector2& compare ) const;				// vec2 != vec2

public:
	friend const Vector2 operator*( float uniformScale, const Vector2& vecToScale );	// float * vec2
	static Vector2 MakeDirectionAtDegrees( float degrees );

public: // NOTE: this is one of the few cases where we break both the "m_" naming rule AND the avoid-public-members rule
	float x;
	float y;
};


/* STANDALONE FUNCTIONS */

float GetDistance( const Vector2& a, const Vector2& b );

float GetDistanceSquared( const Vector2& a, const Vector2& b );

Vector2 RotateVector2( const Vector2& vecToRotate, float theta );

Vector2 GetMidPoint( const Vector2& a, const Vector2& b );
