#include "Game/Entities/EntityFactory.hpp"

TheEntityFactory::TheEntityFactory()
{

}

TheEntityFactory::~TheEntityFactory()
{

}

Entity* TheEntityFactory::CreateEntityOfType( EntityType type, const Vector2& location, const Vector2& velocity ) const
{
	switch( type )
	{
		case EntityType::ENTITY_TYPE_PLAYER_TANK:
		{
			return new PlayerTank( location );
		}
		case EntityType::ENTITY_TYPE_PLAYER_BULLET:
		{
			return new PlayerBullet( location, velocity );
		}
		case EntityType::ENTITY_TYPE_ENEMY_TANK:
		{
			return new EnemyTank( location );
		}
		case EntityType::ENTITY_TYPE_ENEMY_TURRET:
		{
			return new EnemyTurret( location );
		}
		case EntityType::ENTITY_TYPE_ENEMY_BULLET:
		{
			return new EnemyBullet( location, velocity );
		}
		default:
		{
			return nullptr;
		}
	}
}

Explosion* TheEntityFactory::CreateExplosion( const Vector2& location, float size, float durationSeconds ) const
{
	return new Explosion( location, size, durationSeconds );
}
