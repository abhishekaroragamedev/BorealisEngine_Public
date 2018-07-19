#include "Game/World/TileDefinition.hpp"

TileDefinition* TileDefinition::s_tileDefinitions[ TerrainType::NUM_TILE_TYPES ];

TileDefinition::TileDefinition( TerrainType terrainType, const SpriteSheet& spriteSheet )
{
	m_terrainType = terrainType;
	m_terrainTextureBoundsOnSpriteSheet = spriteSheet.GetTextureCoordinatesForSpriteIndex( m_terrainType );
}

bool TileDefinition::IsTileTypeSolid( TerrainType terrainType )
{
	if	(	(	terrainType >= TerrainType::TERRAIN_TYPE_STONE_1 && terrainType <= TerrainType::TERRAIN_TYPE_BRICK_15 ) || 
				terrainType == TerrainType::TERRAIN_TYPE_METAL_1 ||
				terrainType == TerrainType::TERRAIN_TYPE_SOLID_1 || 
				terrainType == TerrainType::TERRAIN_TYPE_LAVA_1
		)
	{
		return true;
	}

	return false;
}

bool TileDefinition::IsTileTypeLiquid( TerrainType terrainType )
{
	if	(	terrainType == TerrainType::TERRAIN_TYPE_WATER_1 || 
			terrainType == TerrainType::TERRAIN_TYPE_WATER_2 || 
			terrainType == TerrainType::TERRAIN_TYPE_WATER_3 || 
			terrainType == TerrainType::TERRAIN_TYPE_WATER_4 || 
			terrainType == TerrainType::TERRAIN_TYPE_WATER_5 || 
			terrainType == TerrainType::TERRAIN_TYPE_LAVA_1 || 
			terrainType == TerrainType::TERRAIN_TYPE_LAVA_2 || 
			terrainType == TerrainType::TERRAIN_TYPE_LAVA_3
		)
	{
		return true;
	}

	return false;
}

bool TileDefinition::DoesTileTypeBlockMovement( TerrainType terrainType )
{
	return ( TileDefinition::IsTileTypeSolid( terrainType ) || TileDefinition::IsTileTypeLiquid( terrainType ) );
}

bool TileDefinition::IsSolid() const
{
	return TileDefinition::IsTileTypeSolid( m_terrainType );
}

bool TileDefinition::IsLiquid() const
{
	return TileDefinition::IsTileTypeLiquid( m_terrainType );
}

bool TileDefinition::DoesBlockMovement() const
{
	return ( IsSolid() || IsLiquid() );
}

bool TileDefinition::IsTerminalTile() const
{
	if	( m_terrainType == TerrainType::TERRAIN_TYPE_PSYCH_1 )
	{
		return true;
	}

	return false;
}

AABB2 TileDefinition::GetTerrainTextureBounds() const
{
	return m_terrainTextureBoundsOnSpriteSheet;
}
