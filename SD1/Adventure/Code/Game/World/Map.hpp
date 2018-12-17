#pragma once

#include "Game/Entities/Actor.hpp"
#include "Game/Entities/Item.hpp"
#include "Game/Entities/Player.hpp"
#include "Game/Entities/Portal.hpp"
#include "Game/Entities/Projectile.hpp"
#include "Game/World/MapDefinition.hpp"
#include "Game/World/RaycastResult2D.hpp"
#include "Game/World/Tile.hpp"
#include "Game/World/TileDefinition.hpp"
#include "Engine/Math/IntVector2.hpp"
#include "Engine/Math/Vector2.hpp"
#include "ThirdParty/tinyXML2/tinyxml2.h"
#include <string>
#include <vector>

class Adventure;

class Map
{
	
	friend class Adventure;
	friend class MapGenStep_FillAndEdge;
	friend class MapGenStep_FromFile;
	friend class MapGenStep_Mutate;
	friend class MapGenStep_SpawnActor;
	friend class MapGenStep_SpawnItem;

public:
	explicit Map( const std::string& mapName, const MapDefinition* mapDefinition, const Adventure& adventure );
	explicit Map( const std::string& mapName, const std::string& mapDefinitionName, const Adventure& adventure );
	~Map();

public:
	void InitializeMapForLoad();
	void Update( float deltaSeconds );
	void Render( float renderAlpha, bool developerModeEnabled ) const;
	std::string GetName() const;
	IntVector2 GetDimensions() const;
	Player* GetPlayerEntity() const;
	Vector2 GetDimensionsInWorldCoordinates() const;
	MapDefinition* GetMapDefinition() const;
	Tile GetTileAt( const IntVector2& coordinates ) const;
	IntVector2 GetTileCoordinatesForWorldPosition( const Vector2& worldCoordinates ) const;
	Tile GetTileAtWorldPosition( const Vector2& worldCoordinates ) const;
	std::vector< Item* > GetItemsInContactWithActor( const Actor& actor ) const;
	void AddExistingPlayer( const Player* playerEntity );
	void AddExistingItem( Item* playerEntity );
	Player* SpawnPlayer( const TileDefinition& onTileType );
	Actor* SpawnActor( const ActorDefinition* actorDefinition, const TileDefinition& onTileType );
	Item* SpawnItem( const ItemDefinition* itemDefinition, const TileDefinition& onTileType );
	Item* SpawnItem( const ItemDefinition* itemDefinition, const Vector2& spawnPosition );
	void RemoveItemFromMap( Item* itemToRemove );		// Usually called by an Item when it is dropped in a new map
	void DeleteItemPermanently( Item* itemToDelete );		// Usually called by an Actor when an Item is used
	Portal* SpawnPortal( const PortalDefinition* portalDefinition, const TileDefinition& onTileType );
	Projectile* SpawnProjectile( const ProjectileDefinition* projectileDefinition, Actor* attacker, WeaponType sourceWeaponType, const Vector2& spawnPosition, const Vector2& spawnVelocity, int statMultipliers[ StatID::NUM_STATS ] );
	bool HasLineOfSight( const Vector2& startPosition, const Vector2& endPosition ) const;
	RaycastResult2D Raycast( const Vector2& startPosition, const Vector2& direction, float maxDistance ) const;

private:
	Tile GetRandomUnpopulatedTileOfType( const TileDefinition& tileType );
	void PopulateTilesFromMapDefinitionAndMapGenSteps();
	void RemovePlayerEntityFromActorList();
	void UpdateEntities( float deltaSeconds );
	void HandleCollisions();
	void HandleActorCollisions();
	void BubbleSortEntityRenderVector();
	bool HasPlayerReachedAPortal();
	void HandleProjectileLifetimeAndCollisions();
	bool HandleCollisionBetweenEntityAndTileAtCoordinates( Entity& entity, const IntVector2& tileCoordinates );
	int GetActualTileIndex( const IntVector2& coordinates ) const;
	void RenderTiles( float renderAlpha ) const;
	void RenderEntities( float renderAlpha, bool developerModeEnabled ) const;
	bool IsEdgeTile( const Tile& tile, int edgeThickness ) const;
	void ImproveRaycastEndPositionPrecision( RaycastResult2D& out_raycastResult ) const;
	void DeleteEntities();

	friend void Entity::ApplyCorrectiveTranslation( const Vector2& translation );

private:
	std::string m_name = "";
	IntVector2 m_dimensions = IntVector2( 0, 0 );
	MapDefinition* m_mapDefinition = nullptr;
	std::vector< Actor* > m_actors;
	std::vector< Item* > m_items;
	std::vector< Portal* > m_portals;
	std::vector< Projectile* > m_projectiles;
	std::vector< Entity* > m_allEntitiesInDrawOrder;
	Player* m_playerEntity = nullptr;
	std::vector< Tile > m_tiles;
	Adventure& m_adventure;

};