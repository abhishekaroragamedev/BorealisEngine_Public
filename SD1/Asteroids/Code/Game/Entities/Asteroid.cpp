#include "Game/Entities/Asteroid.hpp"

Asteroid::Asteroid( float asteroidRadius )
{
	m_velocity = Vector2( ( GetRandomFloatZeroToOne() - 0.5f ), ( GetRandomFloatZeroToOne() - 0.5f ) ).GetNormalized();		// The -0.5s are to convert the range [0, 1] to [-0.5, 0.5]
	m_speed = ASTEROID_SPEED;
	m_rotationSpeed = ( GetRandomFloatZeroToOne() + 0.5f ) * ASTEROID_ROTATION_SPEED;		// The +0.5 is to avoid very low rotation speeds, thus converting [0, 1] to [0.5, 1.5]

	m_radius = asteroidRadius;
	m_numSides = GetRandomIntInRange( ASTEROID_MIN_SIDES, ASTEROID_MAX_SIDES );
	m_vertices = new Vector2[ m_numSides ];
	PopulateVertices();

	m_location = ComputeAsteroidSpawnPosition( m_radius );
	
	m_cosmeticDisc2 = Disc2( m_location, m_radius );
	m_physicalDisc2 = Disc2( m_location, ( m_radius * ASTEROID_PHYSICAL_TO_COSMETIC_RADIUS_RATIO ) );

	PopulateDeveloperModeCirclesVertices();
}

Asteroid::Asteroid( float asteroidRadius, const Vector2& spawnLocation )
{
	m_velocity = Vector2( ( GetRandomFloatZeroToOne() - 0.5f ), ( GetRandomFloatZeroToOne() - 0.5f ) ).GetNormalized();		// The -0.5s are to convert the range [0, 1] to [-0.5, 0.5]
	m_speed = ASTEROID_SPEED;
	m_rotationSpeed = ( GetRandomFloatZeroToOne() + 0.5f ) * ASTEROID_ROTATION_SPEED;		// The +0.5 is to avoid very low rotation speeds, thus converting [0, 1] to [0.5, 1.5]

	m_radius = asteroidRadius;
	m_numSides = GetRandomIntInRange( ASTEROID_MIN_SIDES, ASTEROID_MAX_SIDES );
	m_vertices = new Vector2[ m_numSides ];
	PopulateVertices();

	m_location = spawnLocation;

	m_cosmeticDisc2 = Disc2( m_location, m_radius );
	m_physicalDisc2 = Disc2( m_location, ( m_radius * ASTEROID_PHYSICAL_TO_COSMETIC_RADIUS_RATIO ) );

	PopulateDeveloperModeCirclesVertices();
}

Asteroid::~Asteroid()
{
	delete[] m_vertices;
	m_vertices = nullptr;
}

void Asteroid::Update( float deltaSeconds )
{
	Entity::Update( deltaSeconds );
}

void Asteroid::Render( bool developerModeEnabled ) const
{
	g_renderer->DrawPolygon( m_location, m_orientation, m_vertices, m_numSides, RPolygonType::REGULAR );
	Entity::Render( developerModeEnabled );
}

float Asteroid::GetRadius() const
{
	return m_radius;
}

void Asteroid::PopulateVertices()		// Include noise computation; this is the final asteroid shape
{
	float theta = 0.0f;
	float deltaTheta = 360.0f / static_cast<float>( m_numSides );

	for ( int i = 0; i < m_numSides; i++ )
	{
		float fractionOfRadius = GetRandomFloatInRange( ASTEROID_NOISE_MINIMUM_RADIUS_FRACTION, ASTEROID_NOISE_MAXIMUM_RADIUS_FRACTION );

		float vertexMagnitude = m_radius *  fractionOfRadius;
		float vertexX = vertexMagnitude * CosDegrees( theta );
		float vertexY = vertexMagnitude * SinDegrees( theta );
		m_vertices[ i ] = Vector2(vertexX, vertexY);

		theta += deltaTheta;
	}
}

Vector2 Asteroid::ComputeAsteroidSpawnPosition( float radius ) const
{
	const int NUM_QUADRANTS = 4;

	int randomQuadrant = rand() % NUM_QUADRANTS;
	Vector2 spawnPosition;

	switch( randomQuadrant )
	{
		case 0:
		{
			spawnPosition = Vector2( ( SCREEN_WIDTH + radius ), GetRandomFloatInRange( 0.0f, SCREEN_HEIGHT ) );
			break;
		}

		case 1:
		{
			spawnPosition = Vector2( GetRandomFloatInRange( 0.0f, SCREEN_WIDTH ), ( SCREEN_HEIGHT + radius ) );
			break;
		}

		case 2:
		{
			spawnPosition = Vector2( ( -radius ), GetRandomFloatInRange( 0.0f, SCREEN_HEIGHT ) );
			break;
		}

		case 3:
		{
			spawnPosition = Vector2( GetRandomFloatInRange( 0.0f, SCREEN_WIDTH ), ( -radius ) );
			break;
		}

		default:
		{
			spawnPosition = Vector2( 0.0f, 0.0f );
			break;
		}
	}

	return spawnPosition;		// Dummy value
}
