#include "Engine/Math/CubicSpline.hpp"

CubicSpline2D::CubicSpline2D( const Vector2* positionsArray, int numPoints, const Vector2* velocitiesArray )
{
	AppendPoints( positionsArray, numPoints, velocitiesArray );
}

void CubicSpline2D::AppendPoint( const Vector2& position, const Vector2& velocity )
{
	m_positions.push_back( position );
	m_velocities.push_back( velocity );
}

void CubicSpline2D::AppendPoints( const Vector2* positionsArray, int numPoints, const Vector2* velocitiesArray )
{
	for ( int pointIndex = 0; pointIndex < numPoints; pointIndex++ )
	{
		m_positions.push_back( *( positionsArray + pointIndex ) );

		if ( velocitiesArray != nullptr )
		{
			m_velocities.push_back( *( velocitiesArray + pointIndex ) );
		}
		else
		{
			m_velocities.push_back( Vector2::ZERO );
		}
	}
}

void CubicSpline2D::InsertPoint( int insertBeforeIndex, const Vector2& position, const Vector2& velocity )
{
	m_positions.insert( ( m_positions.begin() + insertBeforeIndex ), position );
	m_velocities.insert( ( m_velocities.begin() + insertBeforeIndex ), velocity );
}

void CubicSpline2D::RemovePoint( int pointIndex )
{
	m_positions.erase( m_positions.begin() + pointIndex );
	m_velocities.erase( m_velocities.begin() + pointIndex );
}

void CubicSpline2D::RemoveAllPoints()
{
	m_positions.clear();
	m_velocities.clear();
}

void CubicSpline2D::SetPoint( int pointIndex, const Vector2& newPosition, const Vector2& newVelocity )
{
	m_positions[ pointIndex ] = newPosition;
	m_velocities[ pointIndex ] = newVelocity;
}

void CubicSpline2D::SetPosition( int pointIndex, const Vector2& newPosition )
{
	m_positions[ pointIndex ] = newPosition;
}

void CubicSpline2D::SetVelocity( int pointIndex, const Vector2& newVelocity )
{
	m_velocities[ pointIndex ] = newVelocity;
}

void CubicSpline2D::SetCardinalVelocities( float tension, const Vector2& startVelocity, const Vector2& endVelocity )
{
	m_velocities[ 0 ] = startVelocity;
	m_velocities[ m_velocities.size() - 1 ] = endVelocity;
	for ( int velocityIndex = 1; velocityIndex < static_cast< int >( m_velocities.size() - 1 ); velocityIndex++ )
	{
		m_velocities[ velocityIndex ] = ( m_positions[ velocityIndex + 1 ] - m_positions[ velocityIndex - 1 ] ) * ( 1.0f - tension ) * 0.5f;
	}
}

int CubicSpline2D::GetNumPoints() const
{
	return (int) m_positions.size();
}

const Vector2 CubicSpline2D::GetPosition( int pointIndex ) const
{
	return m_positions[ pointIndex ];
}

const Vector2 CubicSpline2D::GetVelocity( int pointIndex ) const
{
	return m_velocities[ pointIndex ];
}

int CubicSpline2D::GetPositions( std::vector<Vector2>& out_positions ) const
{
	out_positions = std::vector< Vector2 >( m_positions );
	return static_cast< int >( m_positions.size() );
}

int CubicSpline2D::GetVelocities( std::vector<Vector2>& out_velocities ) const
{
	out_velocities = std::vector< Vector2 >( m_velocities );
	return static_cast< int >( m_velocities.size() );
}

Vector2 CubicSpline2D::EvaluateAtCumulativeParametric( float t ) const
{
	if ( IsFloatEqualTo( t, static_cast< float >( m_positions.size() - 1 ) ) )
	{
		return m_positions[ m_positions.size() - 1 ];
	}

	int startIndexForSubSpline = static_cast< int >( t );
	int endIndexForSubSpline = startIndexForSubSpline + 1;
	float remainingTForSubSpline = t - static_cast< float >( startIndexForSubSpline );

	return EvaluateCubicHermite< Vector2 >( m_positions[ startIndexForSubSpline ], m_velocities[ startIndexForSubSpline ], m_positions[ endIndexForSubSpline ], m_velocities[ endIndexForSubSpline ], remainingTForSubSpline );
}

Vector2 CubicSpline2D::EvaluateAtNormalizedParametric( float t ) const
{
	if ( IsFloatEqualTo( t, 1.0f ) )
	{
		return m_positions[ m_positions.size() - 1 ];
	}

	int startIndexForSubSpline = static_cast< int >( t * static_cast< float >( m_positions.size() - 1 ) );
	int endIndexForSubSpline = startIndexForSubSpline + 1;
	float remainingTForSubSpline = ( t * static_cast< float >( m_positions.size() - 1 ) ) - static_cast< float >( startIndexForSubSpline );

	return EvaluateCubicHermite< Vector2 >( m_positions[ startIndexForSubSpline ], m_velocities[ startIndexForSubSpline ], m_positions[ endIndexForSubSpline ], m_velocities[ endIndexForSubSpline ], remainingTForSubSpline );
}
