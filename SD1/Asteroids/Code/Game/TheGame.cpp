#include "Game/TheGame.hpp"

TheGame::TheGame()
{
	InitializeEntityArrays();
	m_bulletCount = 0;
	m_activeAsteroidCount = 0;
	m_nextWaveAsteroidCount = WAVE_INITIAL_ASTEROIDS_COUNT_MINIMUM;
	m_developerModeEnabled = false;

	SpawnShip( PlayerIndex::KEYBOARD );
}

TheGame::~TheGame()
{
	DeleteAllObjects();
}

void TheGame::InitializeEntityArrays()
{
	for ( int i = 0; i < PLAYER_SHIP_MAX_COUNT; i++ )
	{
		m_playerShips[ i ] = nullptr;
	}

	for ( int j = 0; j < ASTEROID_MAX_COUNT; j++ )
	{
		m_asteroids[ j ] = nullptr;
	}

	for ( int k = 0; k < BULLET_MAX_COUNT; k++ )
	{
		m_bullets[ k ] = nullptr;
	}
}

void TheGame::Update( float& deltaSeconds )
{
	HandleKeyboardInput( deltaSeconds );
	HandleXboxControllerInputs();
	UpdateAllObjects( deltaSeconds );
	StartNewWaveIfNoAsteroidsAreLeft();
}

void TheGame::Render() const
{
	RenderAllObjects();
}

void TheGame::UpdateAllObjects( float deltaSeconds )
{
	for ( int i = 0; i < PLAYER_SHIP_MAX_COUNT; i++ )		// Update Players
	{
		if ( m_playerShips[ i ] != nullptr )
		{
			m_playerShips[ i ]->Update( deltaSeconds );
		}
	}

	for ( int j = 0; j < ASTEROID_MAX_COUNT; j++ )		// Update Asteroids
	{
		if ( m_asteroids[ j ] != nullptr )
		{
			m_asteroids[ j ]->Update( deltaSeconds );

			bool asteroidDestroyed = false;

			for ( int k = 0; k < PLAYER_SHIP_MAX_COUNT; k++ )
			{
				if ( !asteroidDestroyed && m_playerShips[ k ] != NULL && !m_playerShips[ k ]->IsDead() )
				{
					asteroidDestroyed = DetectAndHandleAsteroidCollisionWithPlayer( j, k );
				}
			}

			if ( !asteroidDestroyed )
			{
				asteroidDestroyed = DetectAndHandleAsteroidCollisionWithBullets( j );
			}
		}
	}

	for ( int k = 0; k < BULLET_MAX_COUNT; k++ )		// Update Bullets
	{
		if ( m_bullets[ k ] != nullptr )
		{
			if ( m_bullets[ k ]->GetTimeLeftToLive() < 0.0f )
			{
				DestroyBullet( k );
			}
			else 
			{
				m_bullets[ k ]->Update( deltaSeconds );
			}
		}
	}
}

void TheGame::RenderAllObjects() const
{
	for ( int i = 0; i < PLAYER_SHIP_MAX_COUNT; i++ )		// Render Players
	{
		if ( m_playerShips[ i ] != nullptr )
		{
			m_playerShips[ i ]->Render( m_developerModeEnabled );
		}
	}

	for ( int j = 0; j < ASTEROID_MAX_COUNT; j++ )		// Render Asteroids
	{
		if ( m_asteroids[ j ] != nullptr )
		{
			m_asteroids[ j ]->Render( m_developerModeEnabled );
		}
	}

	for ( int k = 0; k < BULLET_MAX_COUNT; k++ )		// Render Bullets
	{
		if ( m_bullets[ k ] != nullptr )
		{
			m_bullets[ k ]->Render( m_developerModeEnabled );
		}
	}
}

int TheGame::GetAsteroidCount() const
{
	return m_activeAsteroidCount;
}

void TheGame::StartNewWaveIfNoAsteroidsAreLeft()
{
	if ( m_activeAsteroidCount <= 0 )
	{
		int numberOfAsteroidsToSpawn = m_nextWaveAsteroidCount + ( rand() % WAVE_ASTEROIDS_COUNT_MAX_VARIATION );

		for ( int i = 0; i < numberOfAsteroidsToSpawn; i++ )
		{
			SpawnLargeAsteroid();
		}

		m_nextWaveAsteroidCount += WAVE_ASTEROIDS_COUNT_INCREMENT;
	}
}

void TheGame::SpawnShip( PlayerIndex playerShipIndex )
{
	if ( m_playerShips[ playerShipIndex ] == nullptr )
	{
		m_playerShips[ playerShipIndex ] = new PlayerShip( playerShipIndex );
	}
}

void TheGame::SpawnLargeAsteroid()
{
	if (  m_activeAsteroidCount < ASTEROID_MAX_COUNT )
	{
		m_asteroids[ m_activeAsteroidCount ] = new Asteroid( ASTEROID_RADIUS_LARGE );
		m_activeAsteroidCount++;
	}
}

void TheGame::SpawnMediumAsteroid( const Vector2& spawnLocation )
{
	if (  m_activeAsteroidCount < ASTEROID_MAX_COUNT )
	{
		m_asteroids[ m_activeAsteroidCount ] = new Asteroid( ASTEROID_RADIUS_MEDIUM, spawnLocation );
		m_activeAsteroidCount++;
	}
}

void TheGame::SpawnSmallAsteroid( const Vector2& spawnLocation )
{
	if (  m_activeAsteroidCount < ASTEROID_MAX_COUNT )
	{
		m_asteroids[ m_activeAsteroidCount ] = new Asteroid( ASTEROID_RADIUS_SMALL, spawnLocation );
		m_activeAsteroidCount++;
	}
}

void TheGame::SpawnAsteroidsAfterLargerAsteroidDeath( float asteroidRadius, const Vector2& spawnLocation )		// A larger asteroid should break into two smaller asteroids
{
	if ( IsFloatEqualTo( asteroidRadius, ASTEROID_RADIUS_LARGE ) )
	{
		for ( int i = 0; i < NUM_ASTEROIDS_SPAWN_ON_DEATH; i++ )
		{
			SpawnMediumAsteroid( spawnLocation );
		}
	}
	else if ( IsFloatEqualTo( asteroidRadius, ASTEROID_RADIUS_MEDIUM ) )
	{
		for ( int j = 0; j < NUM_ASTEROIDS_SPAWN_ON_DEATH; j++ )
		{
			SpawnSmallAsteroid( spawnLocation );
		}
	}
}

void TheGame::SpawnBullet( int playerShipIndex )
{
	if ( m_playerShips[ playerShipIndex ] != NULL && !m_playerShips[ playerShipIndex ]->IsDead() && ( m_bulletCount < BULLET_MAX_COUNT ) )
	{
		m_bullets[ m_bulletCount ] = new Bullet( m_playerShips[ playerShipIndex ]->GetBulletSpawnLocation(), m_playerShips[ playerShipIndex ]->GetBulletSpawnVelocity() );
		m_bulletCount++;
	}

}

bool TheGame::DestroyShip( PlayerIndex playerIndex )
{
	if ( m_playerShips[ playerIndex ] != nullptr )
	{
		delete m_playerShips[ playerIndex ];
		m_playerShips[ playerIndex ] = nullptr;
		return true;
	}

	return false;
}

bool TheGame::DestroyAsteroid( int index, bool isCalledFromCheatKeypressOrDestructor )
{
	if ( ( index >= 0 && ( index < m_activeAsteroidCount ) ) && ( m_asteroids[ index ] != nullptr ) )
	{
		float radiusOfAsteroidsToSpawn = m_asteroids[ index ]->GetRadius();
		Vector2 asteroidSplitLocation = m_asteroids[ index ]->GetLocation();

		delete m_asteroids[ index ];

		m_activeAsteroidCount--;		// Fill the destroyed asteroid's address slot in the array with the last asteroid's address so that the array maintains contiguous memory usage
		m_asteroids[ index ] = m_asteroids[ m_activeAsteroidCount ];
		m_asteroids[ m_activeAsteroidCount ] = nullptr;

		if ( !isCalledFromCheatKeypressOrDestructor )
		{
			SpawnAsteroidsAfterLargerAsteroidDeath( radiusOfAsteroidsToSpawn, asteroidSplitLocation );
		}

		return true;
	}

	return false;
}

bool TheGame::DestroyBullet( int index )
{
	if ( ( index >= 0 && ( index < m_bulletCount ) ) && ( m_bullets[ index ] != nullptr ) )
	{
		delete m_bullets[ index ];

		m_bulletCount--;		// Fill the destroyed bullet's address slot in the array with the last bullet's address so that the array maintains contiguous memory usage
		m_bullets[ index ] = m_bullets[ m_bulletCount ];
		m_bullets[ m_bulletCount ] = nullptr;

		return true;
	}

	return false;
}

bool TheGame::DetectAndHandleAsteroidCollisionWithBullets( int asteroidIndex )
{
	if ( m_asteroids[ asteroidIndex ] == nullptr )
	{
		return false;
	}

	for ( int i = 0; i < BULLET_MAX_COUNT; i++ )
	{
		if ( m_bullets[ i ] == nullptr )
		{
			break;
		}

		if ( DoDiscsOverlap( m_asteroids[ asteroidIndex ]->GetPhysicalDisc2(), m_bullets[ i ]->GetPhysicalDisc2() ) )
		{
			DestroyAsteroid( asteroidIndex, false );
			DestroyBullet( i );
			return true;
		}
	}

	return false;
}

bool TheGame::DetectAndHandleAsteroidCollisionWithPlayer( int asteroidIndex, int playerIndex )
{
	if ( m_asteroids[ asteroidIndex ] == nullptr )
	{
		return false;
	}

	if ( m_playerShips[ playerIndex ] == nullptr )
	{
		return false;
	}

	if ( DoDiscsOverlap( m_asteroids[ asteroidIndex ]->GetPhysicalDisc2(), m_playerShips[ playerIndex ]->GetPhysicalDisc2() ) )
	{
		DestroyAsteroid( asteroidIndex, false );
		m_playerShips[ playerIndex ]->MarkForDeathAndResetPositionAndOrientation();
		return true;
	}

	return false;
}

void TheGame::DeleteAllObjects()
{
	for ( int i = 0; i < PLAYER_SHIP_MAX_COUNT; i++ )		// Delete Players
	{
		DestroyShip( static_cast<PlayerIndex>( i ) );
	}

	for ( int j = 0; j < ASTEROID_MAX_COUNT; j++ )		// Delete Asteroids
	{
		DestroyAsteroid( j, true );
	}

	for ( int k = 0; k < BULLET_MAX_COUNT; k++ )		// Delete Bullets
	{
		DestroyBullet( k );
	}
}

void TheGame::HandleKeyboardInput( float& out_deltaSeconds )
{
	static float s_deltaSecondsMultiplier = 1.0f;		// Maintain a static variable since deltaSeconds is recomputed in TheApp every frame

	if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_P ) )		// Pause/Unpause Game
	{
		if ( IsFloatEqualTo( s_deltaSecondsMultiplier, 0.0f ) )
		{
			s_deltaSecondsMultiplier = 1.0f;		// Unpause the game
		}
		else
		{
			s_deltaSecondsMultiplier = 0.0f;		// Pause the game
		}
	}
	else if  ( s_deltaSecondsMultiplier > 0.0f )		// Slow down time, if the game isn't already paused
	{
		if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_T ) )
		{
			s_deltaSecondsMultiplier = CHEAT_DELTA_SECONDS_SLOWDOWN_MULTIPLIER;
		}
		else if( g_inputSystem->WasKeyJustReleased( InputSystem::KEYBOARD_T ) )
		{
			s_deltaSecondsMultiplier = 1.0f;
		}
	}

	out_deltaSeconds *= s_deltaSecondsMultiplier;

	if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_I ) )
	{
		SpawnLargeAsteroid();
	}

	if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_O ) )
	{
		if ( GetAsteroidCount() > 0 )
		{
			DestroyAsteroid( rand() % GetAsteroidCount(), true );
		}
	}

	if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_F1 ) )
	{
		if (m_developerModeEnabled)
		{
			m_developerModeEnabled = false;
		}
		else
		{
			m_developerModeEnabled = true;
		}
	}

	if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_SPACE ) )
	{
		SpawnBullet( PlayerIndex::KEYBOARD );
	}

}

void TheGame::HandleXboxControllerInputs()
{
	for ( int i = 0; i < ( PlayerIndex::MAX_NUM_PLAYERS - 1 ); i++ )
	{
		if ( m_playerShips[ i ] == nullptr && g_inputSystem->GetController( i ).IsConnected() )		// If this condition is satisfied, it means that the controller with index i just connected
		{
			SpawnShip( static_cast<PlayerIndex>( i ) );
		}
		else if ( m_playerShips[ i ] != nullptr && !g_inputSystem->GetController( i ).IsConnected() )		// If this condition is satisfied, it means that the controller with index i just disconnected
		{
			DestroyShip( static_cast<PlayerIndex>( i ) );
		}

		if ( m_playerShips[ i ] != nullptr && g_inputSystem->GetController( i ).IsConnected() )
		{
			if ( g_inputSystem->GetController( i ).WasKeyJustPressed( InputXboxControllerMappings::GAMEPAD_A ) )		// Fire a bullet from the ship that fired it
			{
				SpawnBullet( static_cast<PlayerIndex>( i ) );
			}
		}
	}
}
