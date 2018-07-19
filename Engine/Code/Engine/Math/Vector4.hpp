#pragma once

#include "Engine/Math/MathUtils.hpp"

class Vector3;

class Vector4
{

public:
	~Vector4() {}
	Vector4() {}
	Vector4( const Vector4& copyFrom );
	explicit Vector4( float initialX, float initialY, float initialZ, float initialW = 1.0f );
	explicit Vector4( const Vector3& vector3, float initialW = 1.0f );

public:
	Vector3 GetVector3() const;

	void SetFromText( const char* text );
	void SetFromText( const std::string& text );

public:																
	const Vector4 operator+( const Vector4& vecToAdd ) const;
	const Vector4 operator-( const Vector4& vecToSubtract ) const;
	const Vector4 operator*( float uniformScale ) const;
	const Vector4 operator/( float inverseScale ) const;
	void operator+=( const Vector4& vecToAdd );
	void operator-=( const Vector4& vecToSubtract );	
	void operator*=( const float uniformScale );
	void operator/=( const float uniformDivisor );
	void operator=( const Vector4& copyFrom );
	bool operator==( const Vector4& compare ) const;
	bool operator!=( const Vector4& compare ) const;
	bool operator>( const Vector4& compare ) const;
	bool operator>=( const Vector4& compare ) const;
	bool operator<( const Vector4& compare ) const;
	bool operator<=( const Vector4& compare ) const;

public:
	static bool IsValidString( const std::string& vec4AsString );

	static const Vector4 ZERO_POINT;
	static const Vector4 ZERO_DISPLACEMENT;
	static const Vector4 ONE_POINT;
	static const Vector4 ONE_DISPLACEMENT;

public:
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
	float w = 0.0f;

};

float DotProduct( const Vector4& a, const Vector4& b );
