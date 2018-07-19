#pragma once

#include "Game/Entities/EnemyBullet.hpp"
#include "Game/Entities/EnemyTank.hpp"
#include "Game/Entities/EnemyTurret.hpp"
#include "Game/Entities/Explosion.hpp"
#include "Game/Entities/PlayerBullet.hpp"
#include "Game/Entities/PlayerTank.hpp"

class TheEntityFactory
{

public:
	TheEntityFactory();
	~TheEntityFactory();

public:
	Entity* CreateEntityOfType( EntityType type, const Vector2& location, const Vector2& velocity ) const;
	Explosion* CreateExplosion( const Vector2& location, float size, float durationSeconds ) const;

};
