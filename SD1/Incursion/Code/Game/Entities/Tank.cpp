#include "Game/GameCommon.hpp"
#include "Tank.hpp"

Tank::Tank( const Vector2& spawnLocation )
{
	m_location = spawnLocation;
	m_velocity = Vector2( 0.0f, 0.0f );
	m_orientation = 0.0f;
	m_tankTurretOrientation = 0.0f;
	m_tankTurretOffset = 0.0f;
	m_rotationSpeed = 0.0f;
	m_discCollider = Disc2( m_location, TANK_PHYSICAL_DISC_RADIUS_WORLD_UNITS );
	m_isDead = false;
	m_timeSinceBulletFired = 0.0f;

	PopulateDeveloperModeCirclesVertices( TANK_COSMETIC_DISC_RADIUS_WORLD_UNITS, TANK_PHYSICAL_DISC_RADIUS_WORLD_UNITS );		// TODO: Revise these radii based on requirements
}

Tank::~Tank()
{
	m_tankTexture = nullptr;
	m_tankTurretTexture = nullptr;
}

void Tank::Update( float deltaSeconds )
{
	MarkForDeathIfHealthIsZero();
	UpdateBulletFireCooldown( deltaSeconds );
	RestoreTurretToCenterPosition( deltaSeconds );
}

void Tank::UpdateBulletFireCooldown( float deltaSeconds )
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

void Tank::RestoreTurretToCenterPosition( float deltaSeconds )
{
	if ( m_tankTurretOffset > 0.0f )
	{
		m_tankTurretOffset -= TANK_TURRET_RECOIL_MAX_RECOVERY_PER_SECOND * deltaSeconds;
		m_tankTurretOffset = ClampFloat( m_tankTurretOffset, 0.0f, TANK_TURRET_MAX_OFFSET );
	}
}

void Tank::Render( bool developerModeEnabled ) const
{
	Rgba tankColor = Rgba( RGBA_MAX, RGBA_MAX, RGBA_MAX, RGBA_MAX );
	AABB2 tankBounds = AABB2( Vector2( 0.0f, 0.0f ), TANK_COSMETIC_DISC_RADIUS_WORLD_UNITS, TANK_COSMETIC_DISC_RADIUS_WORLD_UNITS );

	g_renderer->DrawTexturedAABB( tankBounds, *m_tankTexture, Vector2( 0.0f, 0.0f ), Vector2( 1.0f, 1.0f ), tankColor );

	g_renderer->PushMatrix();
	g_renderer->Rotate( -m_orientation, 0.0f, 0.0f, 1.0f );		// Undo the tank rotation here, because the player needs to know what angle the turret will face without having to add the tank's angle
	g_renderer->Rotate( m_tankTurretOrientation, 0.0f, 0.0f, 1.0f );
	
	Vector2 turretDisplacementVector = Vector2( -1, 0 );	// Since the reference forward direction is in the positive x-axis
	tankBounds.mins += m_tankTurretOffset * turretDisplacementVector;
	tankBounds.maxs += m_tankTurretOffset * turretDisplacementVector;

	g_renderer->DrawTexturedAABB( tankBounds, *m_tankTurretTexture, Vector2(0.0f, 0.0f), Vector2( 1.0f, 1.0f ), tankColor );
	g_renderer->PopMatrix();

	if ( developerModeEnabled )
	{
		RenderDeveloperMode();
	}
}


void Tank::Translate( float deltaSeconds )
{
	Entity::Translate( deltaSeconds );
	m_discCollider.center = m_location;
}

void Tank::TranslateFromCollision( const Vector2& locationCorrection )
{
	m_discCollider.center += locationCorrection;
	m_location = m_discCollider.center;
}

Disc2 Tank::GetDiscCollider() const
{
	return m_discCollider;
}

float Tank::GetTurretOrientationDegrees() const
{
	return m_tankTurretOrientation;
}

bool Tank::CanFireBullets() const
{
	return ( IsFloatEqualTo( m_timeSinceBulletFired, 0.0f ) );
}
