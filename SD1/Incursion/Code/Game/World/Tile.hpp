#pragma once

#include "Engine/Math/IntVector2.hpp"
#include "Game/World/TileDefinition.hpp"

constexpr float TILE_SIDE_LENGTH_WORLD_UNITS = 1.0f;

class Tile
{

public:
	explicit Tile( TerrainType tileType, int x, int y );
	~Tile();

public:
	IntVector2 GetTileCoordinates() const;
	TerrainType GetTerrainType() const;
	TileDefinition GetTileDefinition() const;

private:
	IntVector2 m_tileCoordinates;
	TileDefinition& m_tileDefinition;

};
