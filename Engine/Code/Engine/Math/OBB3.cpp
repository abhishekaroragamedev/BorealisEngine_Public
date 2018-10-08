#include "Engine/Math/Matrix44.hpp"
#include "Engine/Math/OBB3.hpp"

OBB3::OBB3()
{

}

OBB3::OBB3( const OBB3& copy )
{
	m_center = copy.m_center;
	m_right = copy.m_right;
	m_up = copy.m_up;
	m_forward = copy.m_forward;
}

/* explicit */
OBB3::OBB3( const Vector3& center, const Vector3& right, const Vector3& up, const Vector3& forward )
	:	m_center( center ),
		m_right( right ),
		m_up( up ),
		m_forward( forward )
{

}

OBB3::~OBB3()
{

}

/*
Order Visualized:
  7___6
 /   /|
3___2 |
| 4-|-5
|/  |/
0___1
*/
bool OBB3::GetCorners( Vector3 (&out_corners)[ 8 ] ) const
{
	out_corners[ 0 ] = m_center - m_right - m_up - m_forward;
	out_corners[ 1 ] = m_center + m_right - m_up - m_forward;
	out_corners[ 2 ] = m_center + m_right + m_up - m_forward;
	out_corners[ 3 ] = m_center - m_right + m_up - m_forward;
	out_corners[ 4 ] = m_center - m_right - m_up + m_forward;
	out_corners[ 5 ] = m_center + m_right - m_up + m_forward;
	out_corners[ 6 ] = m_center + m_right + m_up + m_forward;
	out_corners[ 7 ] = m_center - m_right + m_up + m_forward;
	return true;
}

bool OBB3::IsPointInside( const Vector3& point ) const
{
	Vector3 corners[ 8 ];
	GetCorners( corners );

	Matrix44 basisInverse = Matrix44( m_right.GetNormalized(), m_up.GetNormalized(), m_forward.GetNormalized(), m_center );
	basisInverse.Invert();

	Vector3 mins = basisInverse.TransformPosition( corners[ 0 ] );
	Vector3 maxs = basisInverse.TransformPosition( corners[ 6 ] );
	Vector3 localPoint = basisInverse.TransformPosition( point );

	AABB3 aabb = AABB3( mins, maxs );
	bool containsPoint = aabb.IsPointInside( localPoint );
	return containsPoint;
}
