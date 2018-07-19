#pragma once

#include "Engine/Math/MathUtils.hpp"
#include <string>

class IntVector3;
class Vector2;
class Vector4;

class Vector3
{

public:
	~Vector3() {}
	Vector3() {}
	Vector3( const Vector3& copyFrom );
	explicit Vector3( float initialX, float initialY, float initialZ );
	explicit Vector3( const Vector4& vector4 );

public:
	bool IsEqual( const Vector3& compare ) const;
	float GetLength() const;
	float GetLengthSquared() const;
	float NormalizeAndGetLength();
	Vector3 GetNormalized() const;
	float GetProjectionInDirection( const Vector3& direction ) const;
	Vector3 GetComponentInDirection( const Vector3& direction ) const;
	void AddYaw( float degrees );		// Rotate about Y axis
	void AddPitch( float degrees );		// Rotate about X axis
	void AddRoll( float degrees );		// Rotate about Z axis
	void RotateAboutAxes( const Vector3& eulerAngles );	// Will add yaw, pitch and roll, in that order
	void ConvertToCartesian();
	void ConvertToPolar();		// Will convert Vector3 to the form ( radius (along X), rotation (in XZ plane), azimuth (in Y direction) )

	void SetFromText( const char* text );
	void SetFromText( const std::string& text );

public:																
	const Vector3 operator+( const Vector3& vecToAdd ) const;
	const Vector3 operator-( const Vector3& vecToSubtract ) const;
	const Vector3 operator*( float uniformScale ) const;
	const Vector3 operator/( float inverseScale ) const;
	const Vector3 operator*( const Vector3& nonUniformScale ) const;
	const Vector3 operator/( const Vector3& nonUniformDivisor ) const;
	void operator+=( const Vector3& vecToAdd );
	void operator-=( const Vector3& vecToSubtract );	
	void operator*=( const float uniformScale );
	void operator/=( const float uniformDivisor );
	void operator*=( const Vector3& nonUniformScale );
	void operator/=( const Vector3& nonUniformDivisor );
	void operator=( const Vector3& copyFrom );
	bool operator==( const Vector3& compare ) const;
	bool operator!=( const Vector3& compare ) const;
	bool operator>( const Vector3& compare ) const;
	bool operator>=( const Vector3& compare ) const;
	bool operator<( const Vector3& compare ) const;
	bool operator<=( const Vector3& compare ) const;

public:
	friend const Vector3 operator*( float uniformScale, const Vector3& vecToScale );

public:
	static Vector3 FromEuler( const Vector3& euler );
	static Vector3 ConvertToCartesian( const Vector3& polarVector );
	static Vector3 ConvertToPolar( const Vector3& cartesianVector );		// Will convert Vector3 to the form ( radius (along X), rotation (in XZ plane), azimuth (in Y direction) )
	static Vector3 ConvertToCartesian( float radius, float rotation, float azimuth );
	static Vector3 ConvertToPolar( float x, float y, float z );				// Will convert Vector3 to the form ( radius (along X), rotation (in XZ plane), azimuth (in Y direction) )
	static Vector3 GetRandomPointOnSphere( float radius = 1.0f );
	static bool IsValidString( const std::string& vec3AsString );

public:
	static const Vector3 ZERO;
	static const Vector3 ONE;
	static const Vector3 FORWARD;	// Z coordinate
	static const Vector3 UP;	// Y coordinate
	static const Vector3 RIGHT;	// X coordinate

public:
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;

};

float DotProduct( const Vector3& a, const Vector3& b );
Vector3 CrossProduct( const Vector3& a, const Vector3& b );

Vector3 ConvertIntVector3ToVector3( const IntVector3& intVecToConvert );
Vector3 ConvertVector2ToVector3( const Vector2& vector2ToConvert );

Vector3 Interpolate( const Vector3& start, const Vector3& end, float fractionTowardEnd );
Vector3 SLerp( const Vector3& start, const Vector3& end, float fractionTowardEnd );
Vector3 SLerpUnitVector( const Vector3& start, const Vector3& end, float fractionTowardEnd );
