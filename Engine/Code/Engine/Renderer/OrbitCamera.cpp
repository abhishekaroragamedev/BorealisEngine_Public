#include "Engine/Renderer/OrbitCamera.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

OrbitCamera::OrbitCamera( float minRadius, float maxRadius, float minAzimuth, float maxAzimuth, Texture* colorTarget /*= nullptr*/, Texture* depthTarget /*= nullptr*/ )
		:	m_radiusRange( minRadius, maxRadius ),
			m_azimuthRange( minAzimuth, maxAzimuth ),
			m_radius( minRadius ),
			m_azimuth( minAzimuth ),
			Camera( colorTarget, depthTarget )
{
	RecomputeMVPFromPolarCoordinates();
}

OrbitCamera::OrbitCamera( const FloatRange& radiusRange, const FloatRange& azimuthRange, Texture* colorTarget /*= nullptr*/, Texture* depthTarget /*= nullptr*/ )
	:	m_radiusRange( radiusRange ),
		m_azimuthRange( azimuthRange ),
		m_radius( radiusRange.min ),
		m_azimuth( azimuthRange.min ),
		Camera( colorTarget, depthTarget )
{
	RecomputeMVPFromPolarCoordinates();
}

OrbitCamera::~OrbitCamera()
{

}

Vector3 OrbitCamera::GetTarget() const
{
	return m_target;
}

Vector3 OrbitCamera::GetPolarCoordinates() const
{
	return Vector3( m_radius, m_rotation, m_azimuth );
}

Vector3 OrbitCamera::GetCartesianRelativeCoordinates() const
{
	Vector3 cartesianCoordinates = GetPolarCoordinates();
	cartesianCoordinates.ConvertToCartesian();
	return cartesianCoordinates;
}

Vector3 OrbitCamera::GetCartesianAbsoluteCoordinates() const
{
	return ( m_target + GetCartesianRelativeCoordinates() );
}

void OrbitCamera::SetTarget( const Vector3& target )
{
	m_target = target;
	RecomputeMVPFromPolarCoordinates();
}

void OrbitCamera::SetSphericalCoordinates( float radius, float rotation, float azimuth )
{
	m_radius = radius;
	m_rotation = rotation;
	m_azimuth = azimuth;
	RecomputeMVPFromPolarCoordinates();
}

void OrbitCamera::AddToRadius( float amount )
{
	float initialRadius = m_radius;
	m_radius += amount;
	m_radius = ClampFloat( m_radius, m_radiusRange.min, m_radiusRange.max );

	if ( !IsFloatEqualTo( m_radius, initialRadius ) )
	{
		RecomputeMVPFromPolarCoordinates();
	}
}

void OrbitCamera::AddToRotation( float amount )
{
	m_rotation += amount;
	if ( m_rotation > 180.0f )
	{
		m_rotation -= 360.0f;
	}
	else if ( m_rotation < -180.0f )
	{
		m_rotation += 360.0f;
	}
	RecomputeMVPFromPolarCoordinates();
}

void OrbitCamera::AddToAzimuth( float amount )
{
	float initialAzimuth = m_azimuth;
	m_azimuth += amount;
	m_azimuth = ClampFloat( m_azimuth, m_azimuthRange.min, m_azimuthRange.max );
	
	if ( !IsFloatEqualTo( m_azimuth, initialAzimuth ) )
	{
		RecomputeMVPFromPolarCoordinates();
	}
}

void OrbitCamera::SnapToNextFixedAngle()
{
	float closestAngle = FIXED_ANGLES[ 0 ];
	float lowestAngularDisplacement = 360.0f;

	for ( float angle : FIXED_ANGLES )
	{
		float angularDisplacement = GetAngularDisplacement( m_rotation, angle );
		if ( ( angularDisplacement > 0.0f ) && ( angularDisplacement < lowestAngularDisplacement ) )
		{
			closestAngle = angle;
			lowestAngularDisplacement = angularDisplacement;
		}
	}

	m_rotation = closestAngle;
	RecomputeMVPFromPolarCoordinates();
}

void OrbitCamera::SnapToPreviousFixedAngle()
{
	float closestAngle = FIXED_ANGLES[ 0 ];
	float highestAngularDisplacement = -360.0f;

	for ( float angle : FIXED_ANGLES )
	{
		float angularDisplacement = GetAngularDisplacement( m_rotation, angle );
		if ( ( angularDisplacement < 0.0f ) && ( angularDisplacement > highestAngularDisplacement ) )
		{
			closestAngle = angle;
			highestAngularDisplacement = angularDisplacement;
		}
	}

	m_rotation = closestAngle;
	RecomputeMVPFromPolarCoordinates();
}

void OrbitCamera::RecomputeMVPFromPolarCoordinates()
{
	LookAt( GetCartesianAbsoluteCoordinates(), m_target );
	RecomputeOrthoFromRadiusChange();
}

void OrbitCamera::RecomputeOrthoFromRadiusChange()
{
	float aspect = m_projectionMatrix.jY / m_projectionMatrix.iX;
	m_projectionMatrix.jY = 2.0f / m_radius;		// Height
	m_projectionMatrix.iX = m_projectionMatrix.jY / aspect;		// Width
}
