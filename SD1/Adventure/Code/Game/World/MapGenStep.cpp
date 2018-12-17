#include "Game/World/Map.hpp"
#include "Game/World/MapGenStep.hpp"
#include "Game/World/Tile.hpp"
#include "Game/World/TileDefinition.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/XmlUtilities.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <vector>

MapGenStep::MapGenStep( const tinyxml2::XMLElement& genStepXmlElement )
{
	m_name = std::string( genStepXmlElement.Name() );
	m_numIterations = ParseXmlAttribute( genStepXmlElement, NUM_ITERATIONS_XML_ATTRIBUTE_NAME, m_numIterations );
	m_chanceToRun = ParseXmlAttribute( genStepXmlElement, CHANCE_TO_RUN_XML_ATTRIBUTE_NAME, m_chanceToRun );
}

MapGenStep::~MapGenStep()
{

}

MapGenStep* MapGenStep::CreateMapGenStep( const tinyxml2::XMLElement& genStepXmlElement )
{
	if ( std::string( genStepXmlElement.Name() ) == std::string( GEN_STEP_FILL_AND_EDGE_NAME ) )
	{
		return new MapGenStep_FillAndEdge( genStepXmlElement );
	}
	else if ( std::string( genStepXmlElement.Name() ) == std::string( GEN_STEP_FROM_FILE_NAME ) )
	{
		return new MapGenStep_FromFile( genStepXmlElement );
	}
	else if ( std::string( genStepXmlElement.Name() ) == std::string( GEN_STEP_MUTATE_NAME ) )
	{
		return new MapGenStep_Mutate( genStepXmlElement );
	}
	else if ( std::string( genStepXmlElement.Name() ) == std::string( GEN_STEP_SPAWN_ACTOR_NAME ) )
	{
		return new MapGenStep_SpawnActor( genStepXmlElement );
	}
	else if ( std::string( genStepXmlElement.Name() ) == std::string( GEN_STEP_SPAWN_ITEM_NAME ) )
	{
		return new MapGenStep_SpawnItem( genStepXmlElement );
	}
	
	return nullptr;
}

MapGenStep_FillAndEdge::MapGenStep_FillAndEdge( const tinyxml2::XMLElement& genStepXmlElement ) : MapGenStep( genStepXmlElement )
{
	m_fillTileDefinition = ParseXmlAttribute( genStepXmlElement, FILL_TILE_XML_ATTRIBUTE_NAME, m_fillTileDefinition );
	m_edgeTileDefinition = ParseXmlAttribute( genStepXmlElement, EDGE_TILE_XML_ATTRIBUTE_NAME, m_edgeTileDefinition );
	m_edgeThickness = ParseXmlAttribute( genStepXmlElement, EDGE_THICKNESS_XML_ATTRIBUTE_NAME, m_edgeThickness );
	ASSERT_OR_DIE( m_edgeThickness > 0, "MapGenStep_FillAndEdge constructor - edgeThickness for map gen step FillAndEdge needs to be at least 1" );
}

MapGenStep_FillAndEdge::~MapGenStep_FillAndEdge()
{

}

void MapGenStep_FillAndEdge::Run( Map& map ) const
{
	if ( CheckRandomChance( m_chanceToRun ) )
	{
		for ( int iteration = 0; iteration < GetRandomIntInRange( m_numIterations.min, m_numIterations.max ); iteration++ )
		{
			for ( std::vector< Tile >::iterator tileIterator = map.m_tiles.begin(); tileIterator != map.m_tiles.end(); tileIterator++ )
			{
				if ( map.IsEdgeTile( *tileIterator, m_edgeThickness ) )
				{
					tileIterator->ChangeTileType( *m_edgeTileDefinition );
				}
				else
				{
					tileIterator->ChangeTileType( *m_fillTileDefinition );
				}
			}
		}
	}
}

MapGenStep_FromFile::MapGenStep_FromFile( const tinyxml2::XMLElement& genStepXmlElement ) : MapGenStep( genStepXmlElement )
{
	std::string imageFilePath = ParseXmlAttribute( genStepXmlElement, IMAGE_FILE_PATH_XML_ATTRIBUTE_NAME, "" );
	m_mapLayoutImage = new Image( imageFilePath );
}

MapGenStep_FromFile::~MapGenStep_FromFile()
{
	delete m_mapLayoutImage;
	m_mapLayoutImage = nullptr;
}

void MapGenStep_FromFile::Run( Map& map ) const
{
	IntVector2 imageDimensions = m_mapLayoutImage->GetDimensions();
	IntVector2 mapDimensions = map.GetDimensions();

	IntVector2 overlappingDimensions = IntVector2( mapDimensions );
	overlappingDimensions.x = Min( overlappingDimensions.x, imageDimensions.x );
	overlappingDimensions.y = Min( overlappingDimensions.y, imageDimensions.y );

	if ( CheckRandomChance( m_chanceToRun ) )
	{
		float rgbaMaxAsFloat = static_cast< float >( RGBA_MAX );

		for ( int iteration = 0; iteration < GetRandomIntInRange( m_numIterations.min, m_numIterations.max ); iteration++ )
		{
			for ( int y = 0; y < overlappingDimensions.y; y++ )
			{
				for ( int x = 0; x < overlappingDimensions.x; x++ )
				{
					Rgba texelAtCoordinate = m_mapLayoutImage->GetTexel( x, y );
					float chanceForRun = static_cast< float >( texelAtCoordinate.a ) / rgbaMaxAsFloat;		// This is how chance should be calculated

					if ( chanceForRun > 0.0f && CheckRandomChance( chanceForRun ) )
					{
						ASSERT_OR_DIE( TileDefinition::IsMapTexelColorValid( texelAtCoordinate ), Stringf( "MapGenStep_FromFile::Run - Texel at (%d, %d) is not a valid tile color, as specified in Tiles.xml. Aborting...", x, y) );
						
						TileDefinition* tileDefFromTexel = TileDefinition::GetTileDefinitionForTexelColor( texelAtCoordinate );
						map.m_tiles[ map.GetActualTileIndex( IntVector2( x, y ) ) ].ChangeTileType( tileDefFromTexel->GetName() );
					}
				}
			}
		}
	}
}

MapGenStep_Mutate::MapGenStep_Mutate( const tinyxml2::XMLElement& genStepXmlElement ) : MapGenStep( genStepXmlElement )
{
	std::string oldTileDefName = ParseXmlAttribute( genStepXmlElement, OLD_TILE_XML_ATTRIBUTE_NAME, "" );
	std::string newTileDefName = ParseXmlAttribute( genStepXmlElement, NEW_TILE_XML_ATTRIBUTE_NAME, "" );

	ASSERT_OR_DIE( ( oldTileDefName == "" || TileDefinition::s_definitions.find( oldTileDefName ) != TileDefinition::s_definitions.end() ), "MapGenStep_Mutate constructor - oldTileDefName must be either empty or a valid tile definition name. Aborting..." );
	ASSERT_OR_DIE( ( newTileDefName != "" && TileDefinition::s_definitions.find( newTileDefName ) != TileDefinition::s_definitions.end() ), "MapGenStep_Mutate constructor - newTileDefName cannot be either empty or an invalid tile definition name. Aborting..." );

	if ( oldTileDefName != "" )
	{
		m_oldTileDefinition = TileDefinition::s_definitions[ oldTileDefName ];
	}
	m_newTileDefinition = TileDefinition::s_definitions[ newTileDefName ];
	m_mutationChance = ParseXmlAttribute( genStepXmlElement, MUTATE_CHANCE_XML_ATTRIBUTE_NAME, m_mutationChance );
}

MapGenStep_Mutate::~MapGenStep_Mutate()
{
	m_oldTileDefinition = nullptr;
	m_newTileDefinition = nullptr;
}

void MapGenStep_Mutate::Run( Map& map ) const
{
	if ( CheckRandomChance( m_chanceToRun ) )
	{
		for ( int iteration = 0; iteration < GetRandomIntInRange( m_numIterations.min, m_numIterations.max ); iteration++ )
		{
			for ( std::vector< Tile >::iterator tileIterator = map.m_tiles.begin(); tileIterator != map.m_tiles.end(); tileIterator++ )
			{
				if ( m_oldTileDefinition == nullptr || tileIterator->GetTileDefinition() == m_oldTileDefinition )
				{
					if ( CheckRandomChance( m_mutationChance ) )
					{
						tileIterator->ChangeTileType( *m_newTileDefinition );
					}
				}
			}
		}
	}
}

MapGenStep_SpawnActor::MapGenStep_SpawnActor( const tinyxml2::XMLElement& genStepXmlElement ) : MapGenStep( genStepXmlElement )
{
	std::string actorDefName = ParseXmlAttribute( genStepXmlElement, ACTOR_TYPE_XML_ATTRIBUTE_NAME, "" );
	std::string tileDefName = ParseXmlAttribute( genStepXmlElement, ON_TILE_XML_ATTRIBUTE_NAME, "" );

	ASSERT_OR_DIE( ( actorDefName != "" && ActorDefinition::s_definitions.find( actorDefName ) != ActorDefinition::s_definitions.end() ), "MapGenStep_SpawnActor constructor - actorDefName must cannot be either empty or an invalid actor definition name. Aborting..." );
	ASSERT_OR_DIE( ( tileDefName == "" || TileDefinition::s_definitions.find( tileDefName ) != TileDefinition::s_definitions.end() ), "MapGenStep_SpawnActor constructor - tileDefName must be either empty or a valid tile definition name. Aborting..." );

	m_actorToSpawn = ActorDefinition::s_definitions[ actorDefName ];
	if ( tileDefName != "" )
	{
		m_spawnOnTile = TileDefinition::s_definitions[ tileDefName ];
	}
}

MapGenStep_SpawnActor::~MapGenStep_SpawnActor()
{
	m_actorToSpawn = nullptr;
	m_spawnOnTile = nullptr;
}

void MapGenStep_SpawnActor::Run( Map& map ) const
{
	if ( CheckRandomChance( m_chanceToRun ) )
	{
		for ( int iteration = 0; iteration < GetRandomIntInRange( m_numIterations.min, m_numIterations.max ); iteration++ )
		{
			if ( m_spawnOnTile != nullptr )
			{
				map.SpawnActor( m_actorToSpawn, *m_spawnOnTile );
			}
			else
			{
				std::vector< Tile > eligibleTiles;
				for ( std::vector< Tile >::iterator tileIterator = map.m_tiles.begin(); tileIterator != map.m_tiles.end(); tileIterator++ )
				{
					if ( tileIterator->GetTileDefinition()->PermitsActorOfType( *m_actorToSpawn ) )
					{
						eligibleTiles.push_back( *tileIterator );
					}
				}
				if ( eligibleTiles.size() > 0 )
				{
					int tileIndexToSpawnOn = GetRandomIntInRange( 0, static_cast< int >( eligibleTiles.size() - 1 ) );
					map.SpawnActor( m_actorToSpawn, *eligibleTiles[ tileIndexToSpawnOn ].GetTileDefinition() );
				}
			}
		}
	}
}

MapGenStep_SpawnItem::MapGenStep_SpawnItem( const tinyxml2::XMLElement& genStepXmlElement ) : MapGenStep( genStepXmlElement )
{
	std::string itemDefName = ParseXmlAttribute( genStepXmlElement, ITEM_TYPE_XML_ATTRIBUTE_NAME, "" );
	std::string tileDefName = ParseXmlAttribute( genStepXmlElement, ON_TILE_XML_ATTRIBUTE_NAME, "" );

	ASSERT_OR_DIE( ( itemDefName != "" && ItemDefinition::s_definitions.find( itemDefName ) != ItemDefinition::s_definitions.end() ), "MapGenStep_SpawnItem constructor - itemDefName must cannot be either empty or an invalid item definition name. Aborting..." );
	ASSERT_OR_DIE( ( tileDefName == "" || TileDefinition::s_definitions.find( tileDefName ) != TileDefinition::s_definitions.end() ), "MapGenStep_SpawnItem constructor - tileDefName must be either empty or a valid tile definition name. Aborting..." );

	m_itemToSpawn = ItemDefinition::s_definitions[ itemDefName ];
	if ( tileDefName != "" )
	{
		m_spawnOnTile = TileDefinition::s_definitions[ tileDefName ];
	}
}

MapGenStep_SpawnItem::~MapGenStep_SpawnItem()
{
	m_itemToSpawn = nullptr;
	m_spawnOnTile = nullptr;
}

void MapGenStep_SpawnItem::Run( Map& map ) const
{
	if ( CheckRandomChance( m_chanceToRun ) )
	{
		for ( int iteration = 0; iteration < GetRandomIntInRange( m_numIterations.min, m_numIterations.max ); iteration++ )
		{
			if ( m_spawnOnTile != nullptr )
			{
				map.SpawnItem( m_itemToSpawn, *m_spawnOnTile );
			}
			else
			{
				std::vector< Tile > eligibleTiles;
				for ( std::vector< Tile >::iterator tileIterator = map.m_tiles.begin(); tileIterator != map.m_tiles.end(); tileIterator++ )
				{
					if ( tileIterator->GetTileDefinition()->PermitsItemOfType( *m_itemToSpawn ) )
					{
						eligibleTiles.push_back( *tileIterator );
					}
				}
				if ( eligibleTiles.size() > 0 )
				{
					int tileIndexToSpawnOn = GetRandomIntInRange( 0, static_cast< int >( eligibleTiles.size() - 1 ) );
					map.SpawnItem( m_itemToSpawn, *eligibleTiles[ tileIndexToSpawnOn ].GetTileDefinition() );
				}
			}
		}
	}
}
