#include "Game/GameCommon.hpp"
#include "Game/World/Adventure.hpp"

VictoryConditionEntity::VictoryConditionEntity( Entity* entity, VictoryConditionType victoryConditionType )
	:	m_entity( entity ),
		m_victoryConditionType( victoryConditionType )
{

}

Adventure::Adventure( AdventureDefinition* adventureDefinition )
	:	m_adventureDefinition( adventureDefinition )
{
	PopulateFromDefinition();
	g_theGame->PushDialogue( m_adventureDefinition->GetDialogue( AdventureDialogueType::ADVENTURE_DIALOGUE_TYPE_START ) );
}

Adventure::~Adventure()
{
	DeleteVictoryConditions();
	DeleteMaps();
}

void Adventure::PopulateFromDefinition()
{
	std::map< PortalToSpawn*, Portal* > spawnedPortalByBlueprintId;		// Will be used to set reciprocal portals

	for ( MapToGenerate* mapToGenerate : m_adventureDefinition->GetMapsToGenerate() )		// Populate Maps and Victory Conditions (whenever a special entityDef is found)
	{
		Map* newMap = new Map( mapToGenerate->m_name, mapToGenerate->m_mapDefinition, *this );
		TileDefinition* defaultTileType = mapToGenerate->m_mapDefinition->GetDefaultTileDefinition();
		for ( ActorToSpawn* actorToSpawn : mapToGenerate->m_actorsToSpawn )
		{
			TileDefinition* tileTypeToSpawnOn = actorToSpawn->m_onTileType;
			if ( tileTypeToSpawnOn == nullptr )
			{
				tileTypeToSpawnOn = defaultTileType;
			}
			newMap->SpawnActor( actorToSpawn->m_actorDefinition, *tileTypeToSpawnOn );

			for ( VictoryCondition* victoryCondition : m_adventureDefinition->GetVictoryConditions() )
			{
				if ( victoryCondition->m_victoryConditionType == VictoryConditionType::HAVE_DIED && victoryCondition->m_entityDefinitionName == actorToSpawn->m_actorDefinition->GetName() )
				{
					m_victoryConditions.push_back( new VictoryConditionEntity( newMap->m_actors.back(), victoryCondition->m_victoryConditionType ) );
				}
			}
		}
		for ( ItemToSpawn* itemToSpawn : mapToGenerate->m_itemsToSpawn )
		{
			TileDefinition* tileTypeToSpawnOn = itemToSpawn->m_onTileType;
			if ( tileTypeToSpawnOn == nullptr )
			{
				tileTypeToSpawnOn = defaultTileType;
			}
			newMap->SpawnItem( itemToSpawn->m_itemDefinition, *tileTypeToSpawnOn );

			for ( VictoryCondition* victoryCondition : m_adventureDefinition->GetVictoryConditions() )
			{
				if ( victoryCondition->m_victoryConditionType == VictoryConditionType::HAVE_ITEM && victoryCondition->m_entityDefinitionName == itemToSpawn->m_itemDefinition->GetName() )
				{
					m_victoryConditions.push_back( new VictoryConditionEntity( newMap->m_items.back(), victoryCondition->m_victoryConditionType ) );
				}
			}
		}

		for ( PortalToSpawn* portalToSpawn : mapToGenerate->m_portalsToSpawn )
		{
			TileDefinition* tileTypeToSpawnOn = portalToSpawn->m_onTileType;
			if ( tileTypeToSpawnOn == nullptr )
			{
				tileTypeToSpawnOn = defaultTileType;
			}
			Portal* newPortal = newMap->SpawnPortal( portalToSpawn->m_portalDefinition, *tileTypeToSpawnOn );
			spawnedPortalByBlueprintId[ portalToSpawn ] = newPortal;

			for ( VictoryCondition* victoryCondition : m_adventureDefinition->GetVictoryConditions() )
			{
				if ( victoryCondition->m_entityDefinitionName == portalToSpawn->m_portalDefinition->GetName() )
				{
					m_victoryConditions.push_back( new VictoryConditionEntity( newMap->m_portals.back(), victoryCondition->m_victoryConditionType ) );
				}
			}
		}

		m_maps[ newMap->GetName() ] = newMap;
	}

	for ( MapToGenerate* mapToGenerate : m_adventureDefinition->GetMapsToGenerate() )		// Populate reciprocal portals
	{
		for ( PortalToSpawn* portalToSpawn : mapToGenerate->m_portalsToSpawn )
		{
			TileDefinition* tileTypeToSpawnOn = portalToSpawn->m_toTileType;
			if ( tileTypeToSpawnOn == nullptr )
			{
				tileTypeToSpawnOn = m_maps[ portalToSpawn->m_toMapName ]->GetMapDefinition()->GetDefaultTileDefinition();
			}
			Portal* newReciprocalPortal = m_maps[ portalToSpawn->m_toMapName ]->SpawnPortal( portalToSpawn->m_reciprocalPortalDefinition, *tileTypeToSpawnOn );
			newReciprocalPortal->SetReciprocal( spawnedPortalByBlueprintId[ portalToSpawn ] );
			spawnedPortalByBlueprintId[ portalToSpawn ]->SetReciprocal( newReciprocalPortal );

			for ( VictoryCondition* victoryCondition : m_adventureDefinition->GetVictoryConditions() )
			{
				if ( victoryCondition->m_entityDefinitionName == portalToSpawn->m_portalDefinition->GetName() )
				{
					m_victoryConditions.push_back( new VictoryConditionEntity( m_maps[ portalToSpawn->m_toMapName ]->m_portals.back(), victoryCondition->m_victoryConditionType ) );
				}
			}
		}
	}

	// Initialize state based on StartCondition
	m_currentMap = m_maps[ m_adventureDefinition->GetStartCondition()->m_startMapName ];
	m_currentMap->SpawnPlayer( *m_adventureDefinition->GetStartCondition()->m_startTileDefinition );
	m_currentMap->InitializeMapForLoad();
}

void Adventure::Update( float deltaSeconds )
{
	if ( !HasPlayerWon() )
	{
		m_currentMap->Update( deltaSeconds );
	}
}

bool Adventure::IsVictoryConditionSatisfied( const VictoryConditionEntity& victoryCondition ) const
{
	bool isConditionSatisfied = false;

	switch( victoryCondition.m_victoryConditionType )
	{
		case VictoryConditionType::HAVE_DIED	:	isConditionSatisfied = static_cast< Actor* >( victoryCondition.m_entity )->IsDead();	break;
		case VictoryConditionType::HAVE_ITEM	:	isConditionSatisfied = ( static_cast< Item* >( victoryCondition.m_entity )->IsInInventory() && static_cast< Item* >( victoryCondition.m_entity )->GetHoldingActor() == m_currentMap->m_playerEntity ); break;
		default									:	break;
	}

	return isConditionSatisfied;
}

bool Adventure::HasPlayerWon() const
{
	bool areAllVictoryConditionsSatisfied = true;

	for ( std::vector< VictoryConditionEntity* >::const_iterator victoryConditionIterator = m_victoryConditions.begin(); victoryConditionIterator != m_victoryConditions.end(); victoryConditionIterator++ )
	{
		if ( !IsVictoryConditionSatisfied( **victoryConditionIterator ) )
		{
			areAllVictoryConditionsSatisfied = false;
			break;
		}
	}

	return areAllVictoryConditionsSatisfied;
}

Map* Adventure::GetCurrentMap() const
{
	return m_currentMap;
}

AdventureDefinition* Adventure::GetAdventureDefinition() const
{
	return m_adventureDefinition;
};

void Adventure::LoadMapAndMovePlayer( const Map& mapToLoad, Player* playerEntity )
{
	m_currentMap = const_cast< Map* >( &mapToLoad );
	g_theGame->PushDialogue( mapToLoad.GetName(), AABB2( 0.0f, 0.0f, 1.0f, 0.4f ), DialogueStyle::DIALOGUE_STYLE_NONE );
	playerEntity->SetMap( m_currentMap );
	m_currentMap->AddExistingPlayer( playerEntity );
	m_currentMap->InitializeMapForLoad();
}

void Adventure::Render( float renderAlpha, bool developerModeEnabled ) const
{
	m_currentMap->Render( renderAlpha, developerModeEnabled );
}

void Adventure::DeleteMaps()
{
	for ( std::map< std::string, Map* >::iterator mapIterator = m_maps.begin(); mapIterator != m_maps.end(); mapIterator++ )
	{
		delete mapIterator->second;
		mapIterator->second = nullptr;
	}
}

void Adventure::DeleteVictoryConditions()
{
	for ( std::vector< VictoryConditionEntity* >::iterator victoryConditionIterator = m_victoryConditions.begin(); victoryConditionIterator != m_victoryConditions.end(); victoryConditionIterator++ )
	{
		delete *victoryConditionIterator;
		*victoryConditionIterator = nullptr;
	}
}
