#include "Game/GameCommon.hpp"
#include "Game/Entities/EnemyTank.hpp"

EnemyTank::EnemyTank( const Vector2& spawnLocation ) : Tank( spawnLocation )
{
	m_speed = ENEMY_TANK_SPEED;
	m_health = ENEMY_TANK_START_HEALTH;
	m_timeSinceLastDirectionChange = ENEMY_TANK_DIRECTION_CHANGE_TIME_SECONDS;		// So that the direction changes right away
	m_bulletCooldownTime = ENEMY_TANK_BULLET_FIRE_COOLDOWN_SECONDS;
	m_lastSeenPlayerTankLocation = Vector2( 0.0f, 0.0f );
	m_shouldPursue = false;

	m_tankTexture = g_renderer->CreateOrGetTexture( ENEMY_TANK_TEXTURE_PATH );
	m_tankTurretTexture = g_renderer->CreateOrGetTexture( ENEMY_TANK_TURRET_TEXTURE_PATH );
}

EnemyTank::~EnemyTank()
{

}

void EnemyTank::Update( float deltaSeconds )
{
	Tank::Update( deltaSeconds );
	if ( !m_isDead )
	{
		ComputeAIMove( deltaSeconds );
		Tank::Translate( deltaSeconds );
	}
}

void EnemyTank::ComputeAIMove( float deltaSeconds )
{
	m_timeSinceLastDirectionChange += deltaSeconds;

	PlayerTank* playerTank = g_theWorld->GetPlayerTank();
	Vector2 playerTankLocation = playerTank->GetLocation();
	Vector2 directionToPlayerTank = playerTankLocation - m_location;
	float distanceFromPlayer = directionToPlayerTank.NormalizeAndGetLength();
	if ( !playerTank->IsDead() && distanceFromPlayer <= ENEMY_TANK_MAX_SIGHT_DISTANCE && g_theWorld->GetCurrentMap()->HasLineOfSight( m_location, playerTankLocation ) )		// If the tank can see the player
	{
		m_goalOrientation = directionToPlayerTank.GetOrientationDegrees();
		Vector2 orientationDirection = Vector2::MakeDirectionAtDegrees( m_orientation );

		if ( DotProduct( orientationDirection, directionToPlayerTank ) >= ENEMY_TANK_AI_DRIVE_FORWARD_MIN_DOT_PRODUCT )		// Drive towards player
		{
			m_velocity = orientationDirection;

			if ( ( DotProduct( orientationDirection, directionToPlayerTank ) >= ENEMY_TANK_AI_FIRE_BULLET_MIN_DOT_PRODUCT ) && CanFireBullets() )		// Shoot at player
			{
				g_theWorld->GetCurrentMap()->SpawnBullet( EntityType::ENTITY_TYPE_ENEMY_TANK, m_location, m_tankTurretOrientation );
				m_tankTurretOffset = TANK_TURRET_MAX_OFFSET;
				m_timeSinceBulletFired += 0.01f;
			}
		}
		else
		{
			m_velocity = Vector2( 0.0f, 0.0f );
		}

		m_lastSeenPlayerTankLocation = playerTankLocation;		// For pursue mode
		m_shouldPursue = true;
	}
	else if ( m_shouldPursue )		// Pursue mode
	{
		Vector2 directionToPlayerTankLastLocation = m_lastSeenPlayerTankLocation - m_location;
		m_goalOrientation = directionToPlayerTankLastLocation.GetOrientationDegrees();
		Vector2 orientationDirection = Vector2::MakeDirectionAtDegrees( m_orientation );

		float distanceFromPlayerLastSeenLocation = directionToPlayerTankLastLocation.NormalizeAndGetLength();
		if ( distanceFromPlayerLastSeenLocation <= ( TANK_PHYSICAL_DISC_RADIUS_WORLD_UNITS ) )		// Condition to stop pursuing
		{
			m_shouldPursue = false;
		}
		else if ( DotProduct( orientationDirection, directionToPlayerTankLastLocation ) >= ENEMY_TANK_AI_DRIVE_FORWARD_MIN_DOT_PRODUCT )
		{
			m_velocity = orientationDirection;
		}
		else
		{
			m_velocity = Vector2( 0.0f, 0.0f );
		}
	}
	else		// Roam around randomly if the tank can't see the player
	{
		if ( m_timeSinceLastDirectionChange >= ENEMY_TANK_DIRECTION_CHANGE_TIME_SECONDS )
		{
			m_goalOrientation = GetRandomFloatInRange( -180.0f, 180.0f );
			m_timeSinceLastDirectionChange = 0.0f;
		}
		m_velocity = Vector2::MakeDirectionAtDegrees( m_orientation );
	}
	m_orientation = TurnToward( m_orientation, m_goalOrientation, ( ENEMY_TANK_TURN_SPEED_DEGREES_PER_SECOND * deltaSeconds ) );
	m_tankTurretOrientation = m_orientation;
}
