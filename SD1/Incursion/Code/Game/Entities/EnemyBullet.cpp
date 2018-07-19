#include "Game/GameCommon.hpp"
#include "Game/Entities/EnemyBullet.hpp"

EnemyBullet::EnemyBullet( const Vector2& spawnLocation, const Vector2& spawnVelocity ) : Bullet( spawnLocation, spawnVelocity )
{
	m_health = ENEMY_BULLET_START_HEALTH;
	m_renderColor = Rgba( RGBA_MAX, 0, 0, RGBA_MAX );

	m_location = m_location + Vector2::MakeDirectionAtDegrees( m_orientation ) * ( ENEMY_BULLET_SPAWN_OFFSET_FROM_TURRET + BULLET_COSMETIC_DISC_RADIUS_WORLD_UNITS );	// Offset the spawned bullet from the center of the tank
}

EnemyBullet::~EnemyBullet()
{

};
