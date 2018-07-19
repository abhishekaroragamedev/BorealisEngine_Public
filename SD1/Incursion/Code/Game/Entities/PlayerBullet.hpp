#pragma once

#include "Game/Entities/Bullet.hpp"

class PlayerBullet : public Bullet
{

	friend class TheEntityFactory;

private:
	PlayerBullet( const Vector2& spawnLocation, const Vector2& spawnVelocity );
	~PlayerBullet();

private:
	const int PLAYER_BULLET_START_HEALTH = 3;
	const float PLAYER_BULLET_SPAWN_OFFSET_FROM_TURRET = 0.4f * TILE_SIDE_LENGTH_WORLD_UNITS;

};
