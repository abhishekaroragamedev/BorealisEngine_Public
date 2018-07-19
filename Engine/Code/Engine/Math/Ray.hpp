#pragma once

#include "Engine/Math/Vector3.hpp"

struct Ray3
{

public:
	Ray3();
	Ray3( const Vector3& start, const Vector3& direction )
		:	m_start( start ),
			m_direction( direction )
	{

	}

	Vector3 Evaluate( float distance ) const;

	Vector3 m_start = Vector3::ZERO;
	Vector3 m_direction = Vector3::FORWARD;

};
