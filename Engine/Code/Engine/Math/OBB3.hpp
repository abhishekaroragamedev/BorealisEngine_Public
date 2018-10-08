#pragma once

#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/Vector3.hpp"

class OBB3
{

public:
	OBB3();
	~OBB3();
	OBB3( const OBB3& copy );

	explicit OBB3( const Vector3& center, const Vector3& right, const Vector3& up, const Vector3& forward );

	bool GetCorners( Vector3 (&out_corners)[ 8 ] ) const;
	bool IsPointInside( const Vector3& point ) const;

public:
	Vector3 m_center = Vector3::ZERO;
	Vector3 m_right = Vector3::RIGHT;		// Magnitude is half-size
	Vector3 m_up = Vector3::UP;				// Magnitude is half-size
	Vector3 m_forward = Vector3::FORWARD;	// Magnitude is half-size

};
