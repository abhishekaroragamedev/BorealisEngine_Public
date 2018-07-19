#include "Game/GameCommon.hpp"
#include "Game/Entities/Bullet.hpp"

Bullet::Bullet( const Vector2& spawnLocation, const Vector2& spawnVelocity )
{
	m_location = spawnLocation;
	m_velocity = spawnVelocity;
	m_orientation = spawnVelocity.GetOrientationDegrees();
	m_speed = BULLET_SPEED;
	m_isDead = false;
	m_bulletTexture = g_renderer->CreateOrGetTexture( BULLET_TEXTURE_PATH );
	m_renderColor = Rgba( RGBA_MAX, RGBA_MAX, RGBA_MAX, RGBA_MAX );
	m_collisionDisc = Disc2( m_location, BULLET_PHYSICAL_DISC_RADIUS_WORLD_UNITS );

	PopulateDeveloperModeCirclesVertices( BULLET_COSMETIC_DISC_RADIUS_WORLD_UNITS, BULLET_PHYSICAL_DISC_RADIUS_WORLD_UNITS );
}

Bullet::~Bullet()
{
	m_bulletTexture = nullptr;
}

void Bullet::Update( float deltaSeconds )
{
	MarkForDeathIfHealthIsZero();
	if ( !m_isDead )
	{
		Translate( deltaSeconds );
	}
}

void Bullet::Render( bool developerModeEnabled ) const
{
	AABB2 bulletBounds = AABB2( Vector2( 0.0f, 0.0f ), BULLET_COSMETIC_DISC_RADIUS_WORLD_UNITS, BULLET_COSMETIC_DISC_RADIUS_WORLD_UNITS );
	g_renderer->DrawTexturedAABB( bulletBounds, *m_bulletTexture, Vector2( 0.0f, 0.0f ), Vector2( 1.0f, 1.0f ), m_renderColor );

	if ( developerModeEnabled )
	{
		RenderDeveloperMode();
	}
}

void Bullet::RedirectBullet( const Vector2& newVelocity )
{
	m_velocity = newVelocity;
	m_orientation = m_velocity.GetOrientationDegrees();
	DoDamage();
}

void Bullet::Destroy()
{
	m_health = 0;
}

Vector2 Bullet::GetNextPosition( float deltaSeconds ) const
{
	return ( m_location + ( ( m_speed * m_velocity ) *  deltaSeconds ) );
}

Disc2 Bullet::GetDiscCollider() const
{
	return m_collisionDisc;
}

void Bullet::Translate( float deltaSeconds )
{
	Entity::Translate( deltaSeconds );
	m_collisionDisc.center = m_location;
}
