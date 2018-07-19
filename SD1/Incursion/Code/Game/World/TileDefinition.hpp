#pragma once

#include "Engine/Math/AABB2.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"

enum TerrainType
{
	TERRAIN_TYPE_NONE = -1,
	TERRAIN_TYPE_GRASS_1,
	TERRAIN_TYPE_GRASS_2,
	TERRAIN_TYPE_GRASS_3,
	TERRAIN_TYPE_GRASS_4,
	TERRAIN_TYPE_GRASS_5,
	TERRAIN_TYPE_GRASS_6,
	TERRAIN_TYPE_GRASS_7,
	TERRAIN_TYPE_GRASS_8,
	TERRAIN_TYPE_GRASS_9,
	TERRAIN_TYPE_GRASS_10,
	TERRAIN_TYPE_GRASS_11,
	TERRAIN_TYPE_GRASS_12,
	TERRAIN_TYPE_SAND_1,
	TERRAIN_TYPE_SAND_2,
	TERRAIN_TYPE_SAND_3,
	TERRAIN_TYPE_SAND_4,
	TERRAIN_TYPE_SAND_5,
	TERRAIN_TYPE_SAND_6,
	TERRAIN_TYPE_SAND_7,
	TERRAIN_TYPE_SAND_8,
	TERRAIN_TYPE_SAND_9,
	TERRAIN_TYPE_SAND_10,
	TERRAIN_TYPE_SAND_11,
	TERRAIN_TYPE_SAND_12,
	TERRAIN_TYPE_STONE_1,
	TERRAIN_TYPE_STONE_2,
	TERRAIN_TYPE_STONE_3,
	TERRAIN_TYPE_STONE_4,
	TERRAIN_TYPE_STONE_5,
	TERRAIN_TYPE_STONE_6,
	TERRAIN_TYPE_STONE_7,
	TERRAIN_TYPE_STONE_8,
	TERRAIN_TYPE_BRICK_1,
	TERRAIN_TYPE_BRICK_2,
	TERRAIN_TYPE_BRICK_3,
	TERRAIN_TYPE_BRICK_4,
	TERRAIN_TYPE_BRICK_5,
	TERRAIN_TYPE_BRICK_6,
	TERRAIN_TYPE_BRICK_7,
	TERRAIN_TYPE_BRICK_8,
	TERRAIN_TYPE_BRICK_9,
	TERRAIN_TYPE_BRICK_10,
	TERRAIN_TYPE_BRICK_11,
	TERRAIN_TYPE_BRICK_12,
	TERRAIN_TYPE_BRICK_13,
	TERRAIN_TYPE_BRICK_14,
	TERRAIN_TYPE_BRICK_15,
	TERRAIN_TYPE_CRACKED_GROUND_1,
	TERRAIN_TYPE_WOOD_1,
	TERRAIN_TYPE_WOOD_2,
	TERRAIN_TYPE_TILE_1,
	TERRAIN_TYPE_TILE_2,
	TERRAIN_TYPE_WATER_1,
	TERRAIN_TYPE_METAL_1,
	TERRAIN_TYPE_WATER_2,
	TERRAIN_TYPE_SOLID_1,
	TERRAIN_TYPE_LAVA_1,
	TERRAIN_TYPE_PSYCH_1,
	TERRAIN_TYPE_TILE_3,
	TERRAIN_TYPE_WATER_3,
	TERRAIN_TYPE_WATER_4,
	TERRAIN_TYPE_WATER_5,
	TERRAIN_TYPE_LAVA_2,
	TERRAIN_TYPE_LAVA_3,
	NUM_TILE_TYPES
};

class TileDefinition
{

	friend class Tile;
	friend class TheWorld;

public:
	explicit TileDefinition( TerrainType terrainType, const SpriteSheet& spriteSheet );

public:
	static bool IsTileTypeSolid( TerrainType terrainType );
	static bool IsTileTypeLiquid( TerrainType terrainType );
	static bool DoesTileTypeBlockMovement( TerrainType terrainType );

public:
	bool IsSolid() const;		// Neither tanks nor bullets can pass over solid tiles.
	bool IsLiquid() const;		// Tanks cannot pass over liquid tiles, but bullets can.
	bool DoesBlockMovement() const;
	bool IsTerminalTile() const;		// A tile that can represent the end of the level.
	AABB2 GetTerrainTextureBounds() const;

private:
	static TileDefinition* s_tileDefinitions[ TerrainType::NUM_TILE_TYPES ];

private:
	TerrainType m_terrainType;
	AABB2 m_terrainTextureBoundsOnSpriteSheet;
	bool m_isTileSolid;

};
