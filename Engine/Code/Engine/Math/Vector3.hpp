#pragma once

#include "Engine/Math/MathUtils.hpp"

class Vector2;

class Vector3
{

public:
	~Vector3() {}
	Vector3() {}
	Vector3( const Vector3& copyFrom );
	explicit Vector3( float initialX, float initialY, float initialZ );

public:
	float GetLength() const;
	float GetLengthSquared() const;
	float NormalizeAndGetLength();
	Vector3 GetNormalized() const;
	float GetProjectionInDirection( const Vector3& direction ) const;

public:																
	const Vector3 operator+( const Vector3& vecToAdd ) const;
	const Vector3 operator-( const Vector3& vecToSubtract ) const;
	const Vector3 operator*( float uniformScale ) const;
	const Vector3 operator/( float inverseScale ) const;
	void operator+=( const Vector3& vecToAdd );
	void operator-=( const Vector3& vecToSubtract );	
	void operator*=( const float uniformScale );
	void operator/=( const float uniformDivisor );
	void operator=( const Vector3& copyFrom );
	bool operator==( const Vector3& compare ) const;
	bool operator!=( const Vector3& compare ) const;

public:
	static const Vector3 ZERO;
	static const Vector3 ONE;

public:
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;

};

float DotProduct( const Vector3& a, const Vector3& b );

Vector3 ConvertVector2ToVector3( const Vector2& vector2ToConvert );
