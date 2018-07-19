#include "Game/GameCommon.hpp"
#include "Game/Entities/EnemyTurret.hpp"

EnemyTurret::EnemyTurret( const Vector2& spawnLocation )
{
	m_location = spawnLocation;
	m_velocity = Vector2( 0.0f, 0.0f );
	m_orientation = 0.0f;
	m_turretOrientation = 0.0f;
	m_turretOffset = 0.0f;
	m_currentTurnDirectionWanderMode = 1.0f;
	m_speed = 0.0f;
	m_rotationSpeed = 0.0f;
	m_health = ENEMY_TURRET_START_HEALTH;
	m_bulletCooldownTime = ENEMY_TURRET_BULLET_FIRE_COOLDOWN_SECONDS;
	m_timeSinceBulletFired = 0.0f;
	m_discCollider = Disc2( m_location, TURRET_PHYSICAL_DISC_RADIUS_WORLD_UNITS );
	m_isDead = false;

	m_turretBaseTexture = g_renderer->CreateOrGetTexture( ENEMY_TURRET_BASE_TEXTURE_PATH );
	m_turretTexture = g_renderer->CreateOrGetTexture( ENEMY_TURRET_TEXTURE_PATH );

	PopulateDeveloperModeCirclesVertices( TURRET_COSMETIC_DISC_RADIUS_WORLD_UNITS, TURRET_PHYSICAL_DISC_RADIUS_WORLD_UNITS );
}

EnemyTurret::~EnemyTurret()
{

}

void EnemyTurret::Update( float deltaSeconds )
{
	MarkForDeathIfHealthIsZero();
	UpdateBulletFireCooldown( deltaSeconds );
	if ( !m_isDead )
	{
		UpdateLineOfSightRaycast();
		ComputeAIMove( deltaSeconds );
	}

	RestoreTurretToCenterPosition( deltaSeconds );
}

void EnemyTurret::Render( bool developerModeEnabled ) const
{
	Rgba turretColor = Rgba( RGBA_MAX, RGBA_MAX, RGBA_MAX, RGBA_MAX );
	AABB2 turretBounds = AABB2( Vector2( 0.0f, 0.0f ), TURRET_COSMETIC_DISC_RADIUS_WORLD_UNITS, TURRET_COSMETIC_DISC_RADIUS_WORLD_UNITS );

	Vector2 lineOfSightStartPoint = Vector2( 0.0f, 0.0f );
	Vector2 lineOfSightEndPoint = m_lineOfSightRaycastForFrame.m_impactPosition - m_location;	// Transformed to local coordinates
	float lengthOfLine = (lineOfSightEndPoint - lineOfSightStartPoint).GetLength();
	Rgba lineOfSightStartColor = Rgba( RGBA_MAX, 0, 0, RGBA_MAX );
	Rgba lineOfSightEndColor = Rgba( RGBA_MAX, 0, 0, static_cast<unsigned char>( RGBA_MAX - ( RGBA_MAX * ( lengthOfLine / ENEMY_TURRET_MAX_SIGHT_DISTANCE ) ) ) );	// Feather the Raycast alpha by the ratio of the actual raycast length to the full range of sight length
	g_renderer->DrawLine( lineOfSightStartPoint, lineOfSightEndPoint, lineOfSightStartColor, lineOfSightEndColor, TURRET_LINE_OF_SIGHT_THICKNESS );

	g_renderer->DrawTexturedAABB( turretBounds, *m_turretBaseTexture, Vector2( 0.0f, 0.0f ), Vector2( 1.0f, 1.0f ), turretColor );

	g_renderer->PushMatrix();
	g_renderer->Rotate( -m_orientation, 0.0f, 0.0f, 1.0f );		// Undo the turret rotation here, because the player needs to know what angle the turret will face without having to add the turret's angle
	g_renderer->Rotate( m_turretOrientation, 0.0f, 0.0f, 1.0f );

	Vector2 turretDisplacementVector = Vector2( -1, 0 );	// Since the reference forward direction is in the positive x-axis
	turretBounds.mins += m_turretOffset * turretDisplacementVector;
	turretBounds.maxs += m_turretOffset * turretDisplacementVector;

	g_renderer->DrawTexturedAABB( turretBounds, *m_turretTexture, Vector2(0.0f, 0.0f), Vector2( 1.0f, 1.0f ), turretColor );

	g_renderer->PopMatrix();

	if ( developerModeEnabled )
	{
		RenderDeveloperMode();
	}
}

void EnemyTurret::RestoreTurretToCenterPosition( float deltaSeconds )
{
	if ( m_turretOffset > 0.0f )
	{
		m_turretOffset -= TURRET_RECOIL_MAX_RECOVERY_PER_SECOND * deltaSeconds;
		m_turretOffset = ClampFloat( m_turretOffset, 0.0f, TURRET_MAX_OFFSET );
	}
}

void EnemyTurret::UpdateLineOfSightRaycast()
{
	m_lineOfSightRaycastForFrame = g_theWorld->GetCurrentMap()->Raycast( m_location, Vector2::MakeDirectionAtDegrees( m_turretOrientation ), ENEMY_TURRET_MAX_SIGHT_DISTANCE );
}

void EnemyTurret::ComputeAIMove( float deltaSeconds )
{
	PlayerTank* playerTank = g_theWorld->GetPlayerTank();
	Vector2 playerTankLocation = playerTank->GetLocation();
	Vector2 directionToPlayerTank = playerTankLocation - m_location;
	float distanceFromPlayer = directionToPlayerTank.NormalizeAndGetLength();
	if ( !playerTank->IsDead() && distanceFromPlayer <= ENEMY_TURRET_MAX_SIGHT_DISTANCE && g_theWorld->GetCurrentMap()->HasLineOfSight( m_location, playerTankLocation ) )		// If the turret can see the player
	{
		m_goalOrientation = directionToPlayerTank.GetOrientationDegrees();
		Vector2 turretOrientationDirection = Vector2::MakeDirectionAtDegrees( m_turretOrientation );

		if ( ( DotProduct( turretOrientationDirection, directionToPlayerTank ) >= ENEMY_TURRET_AI_FIRE_BULLET_MIN_DOT_PRODUCT ) && CanFireBullets() )		// Shoot at player
		{
			g_theWorld->GetCurrentMap()->SpawnBullet( EntityType::ENTITY_TYPE_ENEMY_TURRET, m_location, m_turretOrientation );
			m_turretOffset = TURRET_MAX_OFFSET;
			m_timeSinceBulletFired += 0.01f;
		}
		m_turretOrientation = TurnToward( m_turretOrientation, m_goalOrientation, ( ENEMY_TURRET_TURN_SPEED_DEGREES_PER_SECOND * deltaSeconds ) );
	}
	else		// Wander mode - turn about the player's last known location
	{
		float targetOrientationDegrees = m_goalOrientation + ( m_currentTurnDirectionWanderMode * TURRET_WANDER_MODE_MAX_TURN_DEGREES );
		m_turretOrientation = TurnToward( m_turretOrientation, targetOrientationDegrees, ( ENEMY_TURRET_TURN_SPEED_DEGREES_PER_SECOND * deltaSeconds ) );

		if ( IsFloatEqualTo( m_turretOrientation, targetOrientationDegrees ) )
		{
			m_currentTurnDirectionWanderMode *= -1.0f;		// Turn in the opposite direction
		}
	}
}

float EnemyTurret::GetTurretOrientationDegrees() const
{
	return m_turretOrientation;
}

bool EnemyTurret::CanFireBullets() const
{
	return ( IsFloatEqualTo( m_timeSinceBulletFired, 0.0f ) );
}

Disc2 EnemyTurret::GetDiscCollider() const
{
	return m_discCollider;
}

void EnemyTurret::UpdateBulletFireCooldown( float deltaSeconds )
{
	if ( m_timeSinceBulletFired >= m_bulletCooldownTime )
	{
		m_timeSinceBulletFired = 0.0f;
	}
	else if ( m_timeSinceBulletFired > 0.0f )
	{
		m_timeSinceBulletFired += deltaSeconds;
	}
}
