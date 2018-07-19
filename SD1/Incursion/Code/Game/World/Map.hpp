#pragma once

#include <vector>
#include <string>
#include <algorithm>
#include "Game/Camera.hpp"
#include "Game/Entities/EntityFactory.hpp"
#include "Game/Physics/RaycastResult2D.hpp"
#include "Game/World/Tile.hpp"

constexpr char TERRAIN_TEXTURE_NAME[] = "Data/Images/Terrain_8x8.png";
constexpr char EXPLOSION_TEXTURE_NAME[] = "Data/Images/Explosion_5x5.png";
constexpr int RAYCAST_STEPS_PER_TILE = 10;

constexpr float EXPLOSION_BULLET_STRIKE_SIZE_WORLD_UNITS = TILE_SIDE_LENGTH_WORLD_UNITS * 0.1f;
constexpr float EXPLOSION_BULLET_STRIKE_DURATION_SECONDS = 1.0f;
constexpr float EXPLOSION_BULLET_SPAWN_SIZE_WORLD_UNITS = TILE_SIDE_LENGTH_WORLD_UNITS * 0.05f;

constexpr float EXPLOSION_TANK_EXPLOSION_SIZE_WORLD_UNITS = TILE_SIDE_LENGTH_WORLD_UNITS * 0.6f;
constexpr float EXPLOSION_TANK_EXPLOSION_DURATION_SECONDS = 1.5f;

constexpr float EXPLOSION_TURRET_EXPLOSION_SIZE_WORLD_UNITS = TILE_SIDE_LENGTH_WORLD_UNITS * 0.9f;
constexpr float EXPLOSION_TURRET_EXPLOSION_DURATION_SECONDS = 1.5f;

constexpr float EXPLOSION_PLAYER_TANK_EXPLOSION_SIZE_WORLD_UNITS = TILE_SIDE_LENGTH_WORLD_UNITS * 1.1f;
constexpr float EXPLOSION_PLAYER_TANK_EXPLOSION_DURATION_SECONDS = 2.0f;

constexpr float BULLET_FIRE_VOLUME = 0.5f;

class Map
{

public:
	explicit Map( int horizontalTileWidth, int verticalTileWidth, SpriteSheet& terrainSpriteSheet );
	~Map();

public:
	bool Update( float deltaSeconds );
	void AddTile( const Tile& newTile );
	void SpawnPlayerTank(  PlayerTank* playerTank  );
	void UnloadMap();
	void SpawnEnemyTanks( int numberToSpawn );
	void SpawnEnemyTurrets( int numberToSpawn );
	void SpawnBullet( EntityType firingEntityType, const Vector2& spawnLocation, float spawnOrientation );
	Tile GetTileAt( int x, int y ) const;
	void Render( bool developerModeEnabled ) const;
	bool HasLineOfSight( const Vector2& startPosition, const Vector2& endPosition ) const;
	RaycastResult2D Raycast( const Vector2& startPosition, const Vector2& direction, float maxDistance ) const;
	AABB2 GetBoundsInWorldCoordinates() const;

private:
	bool HasPlayerCompletedLevel();
	void PreventCollisions( float deltaSeconds );
	void HandleCollisions();
	void HandleBulletCollisionsWithTanksAndTurrets();
	void HandleTankCollisionsWithTanks();
	void HandleTankCollisionsWithTurrets();
	void HandleTankCollisionWithWalls();
	void HandleBulletDeath( Bullet* bullet );
	void HandleEnemyTankDeath( EnemyTank* enemyTank );
	void HandleEnemyTurretDeath( EnemyTurret* enemyTurret );
	void HandlePlayerTankDeath( PlayerTank* playerTank );
	void RemoveEntitiesMarkedForDeath();
	void DeleteAllEntities();
	void UpdateLiquidTileUvOffset( float deltaSeconds );
	void ImproveRaycastEndPositionPrecision( RaycastResult2D& out_raycastResult ) const;
	Vector2 CorrectDiscAndAABBCollisionHeadOn( const Disc2& discCollider, const AABB2& aabbCollider );
	Vector2 CorrectDiscAndAABBCollisionDiagonal( const Disc2& discCollider, const AABB2& aabbCollider );
	Tile GetTileAtWorldPosition( const Vector2& worldPosition ) const;
	AABB2 GetTileWorldPosition( const Tile& tile ) const;

private:
	const float LIQUID_TILE_MIN_FLOW_SPEED_WORLD_UNITS_PER_SECOND = 0.01f;

	TheEntityFactory m_entityFactory;
	std::vector< Tile > m_tiles;
	std::vector< Entity* > m_entitiesByType[ EntityType::NUM_ENTITY_TYPES ];
	Camera* m_camera;
	int m_horizontalTileWidth;
	int m_verticalTileWidth;
	SpriteSheet& m_terrainSpriteSheet;
	float m_liquidTileUvOffset;
	float m_liquidTileFlowDirection;

};
