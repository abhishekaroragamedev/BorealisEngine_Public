#pragma once

#include "Game/Entities/Bullet.hpp"

class EnemyBullet : public Bullet
{

	friend class TheEntityFactory;

private:
	EnemyBullet( const Vector2& spawnLocation, const Vector2& spawnVelocity );
	~EnemyBullet();

private:
	const int ENEMY_BULLET_START_HEALTH = 1;
	const float ENEMY_BULLET_SPAWN_OFFSET_FROM_TURRET = 0.4f * TILE_SIDE_LENGTH_WORLD_UNITS;

};
