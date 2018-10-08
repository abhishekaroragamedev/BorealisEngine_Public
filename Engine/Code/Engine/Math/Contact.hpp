#pragma once

#include "Engine/Math/Vector3.hpp"

struct Contact3
{

public:
	Contact3();
	Contact3( const Vector3& position, const Vector3& normal )
		:	m_position( position ),
			m_normal( normal )
	{

	}

	Vector3 m_position = Vector3::ZERO;
	Vector3 m_normal = Vector3::UP;

};
