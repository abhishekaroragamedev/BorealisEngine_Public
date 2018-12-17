#pragma once

#include "Engine/Math/MathUtils.hpp"

class IntVector2;
class Vector3;

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
	void SetFromText( const char* text );

public:
	float GetLength() const;
	float GetLengthSquared() const;
	float NormalizeAndGetLength();
	Vector2 GetNormalized() const;
	float GetOrientationDegrees() const;
	float GetProjectionInDirection( const Vector2& direction ) const;

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

public:
	static const Vector2 ZERO;
	static const Vector2 ONE;
	static const Vector2 UP;
	static const Vector2 DOWN;
	static const Vector2 LEFT;
	static const Vector2 RIGHT;

public:
	float x = 0.0f;
	float y = 0.0f;
};


/* STANDALONE FUNCTIONS */

float GetDistance( const Vector2& a, const Vector2& b );

float GetDistanceSquared( const Vector2& a, const Vector2& b );

float DotProduct( const Vector2& a, const Vector2& b );

Vector2 RotateVector2( const Vector2& vecToRotate, float theta );

Vector2 RotateVector2RightAngle( const Vector2& vecToRotate, bool isClockwise );

Vector2 GetMidPoint( const Vector2& a, const Vector2& b );

Vector2 GetProjectedVector( const Vector2& vectorToProject, const Vector2& projectOnto );

const Vector2 GetTransformedIntoBasis( const Vector2& originalVector, const Vector2& newBasisI, const Vector2& newBasisJ );

const Vector2 GetTransformedOutOfBasis( const Vector2& vectorInBasis, const Vector2& oldBasisI, const Vector2& oldBasisJ );

void DecomposeVectorIntoBasis( const Vector2& originalVector, const Vector2& newBasisI, const Vector2& newBasisJ, Vector2& out_vectorAlongI, Vector2& out_vectorAlongJ );

Vector2 GetStrongestCardinalDirection( const Vector2& actualDirection );

const Vector2 Interpolate( const Vector2& start, const Vector2& end, float fractionTowardEnd );

Vector2 Reflect( const Vector2& vectorToReflect, const Vector2& reflectionNormal );

Vector2 ConvertIntVector2ToVector2( const IntVector2& intVecToConvert ); 

Vector2 ConvertVector3ToVector2( const Vector3& vector3ToConvert );
