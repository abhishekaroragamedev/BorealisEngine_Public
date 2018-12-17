#include "Game/GameCommon.hpp"
#include "Game/World/Tile.hpp"

TileExtraInfo::TileExtraInfo()
{

}

TileExtraInfo::TileExtraInfo( const std::string& commaSeparatedTags )
{
	m_tags = Tags( commaSeparatedTags );
}

TileExtraInfo::TileExtraInfo( const Tags& defaultTags )
{
	m_tags = Tags( defaultTags );
}

Tags TileExtraInfo::GetTagSet() const
{
	return m_tags;
}

Tile::Tile( const TileDefinition& tileDefinition, const IntVector2& tileCoordinates )
{
	m_tileDefinition = const_cast<TileDefinition*>( &tileDefinition );
	m_tileCoordinates = IntVector2( tileCoordinates );
}

Tile::Tile( const std::string tileDefinitionName, const IntVector2& tileCoordinates )
{
	m_tileDefinition = TileDefinition::s_definitions[ tileDefinitionName ];
	m_tileCoordinates = IntVector2( tileCoordinates );
}

Tile::~Tile()
{
	if ( m_tileExtraInfo != nullptr )
	{
		delete m_tileExtraInfo;
		m_tileExtraInfo = nullptr;
	}
}

void Tile::ChangeTileType( const TileDefinition& newTileType )
{
	m_tileDefinition = const_cast<TileDefinition*>( &newTileType );
}

void Tile::ChangeTileType( const std::string& newTileTypeName )
{
	m_tileDefinition = TileDefinition::s_definitions[ newTileTypeName ];
}

TileExtraInfo Tile::GetExtraInfoAndInstantiateIfNeeded()
{
	if ( m_tileExtraInfo == nullptr )
	{
		m_tileExtraInfo = new TileExtraInfo;
	}
	return *m_tileExtraInfo;
}

AABB2 Tile::GetTileWorldBounds() const		// Assumes the world originates at the origin, and grows in the positive x and y directions
{
	Vector2 tileCoordinatesAsVector2 = Vector2( static_cast<float>( m_tileCoordinates.x ), static_cast<float>( m_tileCoordinates.y ) );

	Vector2 tileWorldBoundsMins = Vector2( ( tileCoordinatesAsVector2.x * TILE_SIDE_LENGTH_WORLD_UNITS ), ( tileCoordinatesAsVector2.y * TILE_SIDE_LENGTH_WORLD_UNITS ) );
	Vector2 tileWorldBoundsMaxs = Vector2( ( ( tileCoordinatesAsVector2.x + 1.0f ) * TILE_SIDE_LENGTH_WORLD_UNITS ), ( ( tileCoordinatesAsVector2.y + 1.0f ) * TILE_SIDE_LENGTH_WORLD_UNITS ) );
	return AABB2( tileWorldBoundsMins, tileWorldBoundsMaxs );
}

IntVector2 Tile::GetTileCoordinates() const
{
	return m_tileCoordinates;
}

TileDefinition* Tile::GetTileDefinition() const
{
	return m_tileDefinition;
}
