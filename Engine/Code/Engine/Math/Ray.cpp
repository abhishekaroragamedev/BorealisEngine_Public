#include "Engine/Math/Ray.hpp"

Vector3 Ray3::Evaluate( float distance ) const
{
	Vector3 point = m_start + ( distance * m_direction );
	return point;
}
