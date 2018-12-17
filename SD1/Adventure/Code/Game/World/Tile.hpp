#pragma once

#include "Game/Tags.hpp"
#include "Game/World/TileDefinition.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/IntVector2.hpp"
#include "Engine/Math/Vector2.hpp"
#include <string>

constexpr float TILE_SIDE_LENGTH_WORLD_UNITS = 1.0f;

class TileExtraInfo
{

	friend class Tile;

public:
	TileExtraInfo();
	TileExtraInfo( const std::string& commaSeparatedTags );
	TileExtraInfo( const Tags& defaultTags );

	Tags GetTagSet() const;

private:
	Tags m_tags;

};

class Tile
{

public:
	explicit Tile( const TileDefinition& tileDefinition, const IntVector2& tileCoordinates );
	explicit Tile( const std::string tileDefinitionName, const IntVector2& tileCoordinates );
	~Tile();

public:
	void ChangeTileType( const TileDefinition& newTileType );
	void ChangeTileType( const std::string& newTileTypeName );
	TileExtraInfo GetExtraInfoAndInstantiateIfNeeded();		// Will instantiate Extra Info if called for the first time
	AABB2 GetTileWorldBounds() const;
	IntVector2 GetTileCoordinates() const;
	TileDefinition* GetTileDefinition() const;

private:
	IntVector2 m_tileCoordinates = IntVector2::ZERO;
	TileDefinition* m_tileDefinition = nullptr;
	TileExtraInfo* m_tileExtraInfo = nullptr;

};
