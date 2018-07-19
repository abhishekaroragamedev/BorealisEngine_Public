#include "Game/World/Tile.hpp"

Tile::Tile( TerrainType tileType, int x, int y ): m_tileDefinition(*TileDefinition::s_tileDefinitions[ tileType ])
{
	m_tileCoordinates = IntVector2( x, y );
}

Tile::~Tile()
{

}

IntVector2 Tile::GetTileCoordinates() const
{
	return m_tileCoordinates;
}

TerrainType Tile::GetTerrainType() const
{
	return m_tileDefinition.m_terrainType;
}

TileDefinition Tile::GetTileDefinition() const
{
	return m_tileDefinition;
}
