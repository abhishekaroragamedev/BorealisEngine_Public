#include "Game/World/AdventureDefinition.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/XmlUtilities.hpp"

MapToGenerate::~MapToGenerate()
{
	for ( std::vector< ActorToSpawn* >::iterator actorToSpawnIterator = m_actorsToSpawn.begin(); actorToSpawnIterator != m_actorsToSpawn.end(); actorToSpawnIterator++ )
	{
		delete *actorToSpawnIterator;
		*actorToSpawnIterator = nullptr;
	}
	for ( std::vector< ItemToSpawn* >::iterator itemToSpawnIterator = m_itemsToSpawn.begin(); itemToSpawnIterator != m_itemsToSpawn.end(); itemToSpawnIterator++ )
	{
		delete *itemToSpawnIterator;
		*itemToSpawnIterator = nullptr;
	}
	for ( std::vector< PortalToSpawn* >::iterator portalToSpawnIterator = m_portalsToSpawn.begin(); portalToSpawnIterator != m_portalsToSpawn.end(); portalToSpawnIterator++ )
	{
		delete *portalToSpawnIterator;
		*portalToSpawnIterator = nullptr;
	}
}

VictoryCondition::VictoryCondition( const std::string& entityDefinitionName, VictoryConditionType victoryConditionType )
	:	m_entityDefinitionName( entityDefinitionName ),
		m_victoryConditionType( victoryConditionType )
{

}

std::map< std::string, AdventureDefinition* > AdventureDefinition::s_definitions;

std::vector< AdventureDefinition* > AdventureDefinition::GetDefinitionsAsVector()
{
	std::vector< AdventureDefinition* > adventureDefinitions;

	for ( std::map< std::string, AdventureDefinition* >::iterator mapIterator = AdventureDefinition::s_definitions.begin(); mapIterator != AdventureDefinition::s_definitions.end(); mapIterator++ )
	{
		adventureDefinitions.push_back( mapIterator->second );
	}

	return adventureDefinitions;
}

AdventureDefinition::AdventureDefinition( const tinyxml2::XMLElement& adventureDefinitionElement )
{
	PopulateFromXml( adventureDefinitionElement );
	s_definitions[ m_name ] = this;
}

AdventureDefinition::~AdventureDefinition()
{
	delete m_startCondition;
	m_startCondition = nullptr;

	for ( std::vector< MapToGenerate* >::iterator mapToGenerateIterator = m_mapsToGenerate.begin(); mapToGenerateIterator != m_mapsToGenerate.end(); mapToGenerateIterator++ )
	{
		delete *mapToGenerateIterator;
		*mapToGenerateIterator = nullptr;
	}
	for ( std::vector< VictoryCondition* >::iterator victoryConditionIterator = m_victoryConditions.begin(); victoryConditionIterator != m_victoryConditions.end(); victoryConditionIterator++ )
	{
		delete *victoryConditionIterator;
		*victoryConditionIterator = nullptr;
	}
	for ( int adventureDialogueIndex = 0; adventureDialogueIndex < AdventureDialogueType::NUM_ADVENTURE_DIALOGUE_TYPES; adventureDialogueIndex++ )
	{
		delete m_adventureDialogues[ adventureDialogueIndex ];
		m_adventureDialogues[ adventureDialogueIndex ] = nullptr;
	}
}

void AdventureDefinition::PopulateFromXml( const tinyxml2::XMLElement& adventureDefinitionElement )
{
	m_name = ParseXmlAttribute( adventureDefinitionElement, NAME_XML_ATTRIBUTE_NAME, m_name );
	m_title = ParseXmlAttribute( adventureDefinitionElement, TITLE_XML_ATTRIBUTE_NAME, m_title );

	ASSERT_OR_DIE( !adventureDefinitionElement.NoChildren(), "AdventureDefinition::PopulateFromXml - <AdventureDefinition> node has no child nodes - expected <StartConditions>, <VictoryConditions> and at least one <Map> child node. Aborting..." );

	for ( const tinyxml2::XMLElement* adventureDefSubElement = adventureDefinitionElement.FirstChildElement(); adventureDefSubElement != nullptr; adventureDefSubElement = adventureDefSubElement->NextSiblingElement() )
	{
		if ( std::string( adventureDefSubElement->Name() ) == std::string( START_CONDITION_XML_NODE_NAME ) )
		{
			m_startCondition = new StartCondition;
			m_startCondition->m_startMapName = ParseXmlAttribute( *adventureDefSubElement, START_MAP_XML_ATTRIBUTE_NAME, m_startCondition->m_startMapName );
			m_startCondition->m_startTileDefinition = ParseXmlAttribute( *adventureDefSubElement, ON_TILE_TYPE_XML_ATTRIBUTE_NAME, m_startCondition->m_startTileDefinition );
		}
		else if ( std::string( adventureDefSubElement->Name() ) == std::string( VICTORY_CONDITION_XML_NODE_NAME ) )
		{
			VictoryConditionType victoryConditionType = VictoryConditionType::NO_VICTORY_CONDITION;
			std::string victoryConditionEntityDefName = "";
			const char* haveDiedValue = adventureDefSubElement->Attribute( HAVE_DIED_XML_ATTRIBUTE_NAME );
			if ( haveDiedValue != nullptr )
			{
				victoryConditionType = VictoryConditionType::HAVE_DIED;
				victoryConditionEntityDefName = std::string( haveDiedValue );

				ASSERT_OR_DIE( ( ActorDefinition::s_definitions.find( victoryConditionEntityDefName ) != ActorDefinition::s_definitions.end() ), "AdventureDefinition::PopulateFromXml - VictoryCondition - Invalid or no EntityDefinition name found. Aborting..." );
			}
			const char* haveItemValue = adventureDefSubElement->Attribute( HAVE_ITEM_XML_ATTRIBUTE_NAME );
			if ( haveItemValue != nullptr )
			{
				victoryConditionType = VictoryConditionType::HAVE_ITEM;
				victoryConditionEntityDefName = std::string( haveItemValue );

				ASSERT_OR_DIE( ( ItemDefinition::s_definitions.find( victoryConditionEntityDefName ) != ItemDefinition::s_definitions.end() ), "AdventureDefinition::PopulateFromXml - VictoryCondition - Invalid or no EntityDefinition name found. Aborting..." );
			}

			ASSERT_OR_DIE( victoryConditionEntityDefName != "", "AdventureDefinition::PopulateFromXml - VictoryCondition - No EntityDefinition name found. Aborting..." );
			ASSERT_OR_DIE( victoryConditionType != VictoryConditionType::NO_VICTORY_CONDITION, "AdventureDefinition::PopulateFromXml - VictoryCondition - Invalid or no Victory Condition type found. Aborting..." );

			VictoryCondition* newVictoryCondition = new VictoryCondition( victoryConditionEntityDefName, victoryConditionType );
			m_victoryConditions.push_back( newVictoryCondition );
		}
		else if ( std::string( adventureDefSubElement->Name() ) == std::string( MAP_XML_NODE_NAME ) )
		{
			MapToGenerate* mapToGenerate = new MapToGenerate;
			mapToGenerate->m_name = ParseXmlAttribute( *adventureDefSubElement, NAME_XML_ATTRIBUTE_NAME, mapToGenerate->m_name );
			mapToGenerate->m_mapDefinition = ParseXmlAttribute( *adventureDefSubElement, MAP_DEFINITION_XML_ATTRIBUTE_NAME, mapToGenerate->m_mapDefinition );

			ASSERT_OR_DIE( mapToGenerate->m_mapDefinition != nullptr, "AdventureDefinition::PopulateFromXml - Map - Invalid or no MapDefinition name found. Aborting..." );

			for ( const tinyxml2::XMLElement* mapSubElement = adventureDefSubElement->FirstChildElement(); mapSubElement != nullptr; mapSubElement = mapSubElement->NextSiblingElement() )
			{
				if ( std::string( mapSubElement->Name() ) == std::string( ACTOR_XML_NODE_NAME ) )
				{
					ActorToSpawn* actorToSpawn = new ActorToSpawn;
					actorToSpawn->m_actorDefinition = ParseXmlAttribute( *mapSubElement, TYPE_XML_ATTRIBUTE_NAME, actorToSpawn->m_actorDefinition );
					actorToSpawn->m_onTileType = ParseXmlAttribute( *mapSubElement, ON_TILE_TYPE_XML_ATTRIBUTE_NAME, actorToSpawn->m_onTileType );
					mapToGenerate->m_actorsToSpawn.push_back( actorToSpawn );
				}
				else if ( std::string( mapSubElement->Name() ) == std::string( ITEM_XML_NODE_NAME ) )
				{
					ItemToSpawn* itemToSpawn = new ItemToSpawn;
					itemToSpawn->m_itemDefinition = ParseXmlAttribute( *mapSubElement, TYPE_XML_ATTRIBUTE_NAME, itemToSpawn->m_itemDefinition );
					itemToSpawn->m_onTileType = ParseXmlAttribute( *mapSubElement, ON_TILE_TYPE_XML_ATTRIBUTE_NAME, itemToSpawn->m_onTileType );
					mapToGenerate->m_itemsToSpawn.push_back( itemToSpawn );
				}
				else if ( std::string( mapSubElement->Name() ) == std::string( PORTAL_XML_NODE_NAME ) )
				{
					PortalToSpawn* portalToSpawn = new PortalToSpawn;
					portalToSpawn->m_portalDefinition = ParseXmlAttribute( *mapSubElement, TYPE_XML_ATTRIBUTE_NAME, portalToSpawn->m_portalDefinition );
					portalToSpawn->m_onTileType = ParseXmlAttribute( *mapSubElement, ON_TILE_TYPE_XML_ATTRIBUTE_NAME, portalToSpawn->m_onTileType );
					portalToSpawn->m_toMapName = ParseXmlAttribute( *mapSubElement, TO_MAP_XML_ATTRIBUTE_NAME, portalToSpawn->m_toMapName );
					portalToSpawn->m_reciprocalPortalDefinition = ParseXmlAttribute( *mapSubElement, RECIPROCAL_TYPE_XML_ATTRIBUTE_NAME, portalToSpawn->m_reciprocalPortalDefinition );
					portalToSpawn->m_toTileType = ParseXmlAttribute( *mapSubElement, TO_TILE_TYPE_XML_ATTRIBUTE_NAME, portalToSpawn->m_toTileType );
					mapToGenerate->m_portalsToSpawn.push_back( portalToSpawn );
				}
			}

			m_mapsToGenerate.push_back( mapToGenerate );
		}
		else if ( std::string( adventureDefSubElement->Name() ) == std::string( DIALOGUE_XML_NODE_NAME ) )
		{
			PopulateDialoguesFromXml( *adventureDefSubElement );
		}
	}
}

void AdventureDefinition::PopulateDialoguesFromXml( const tinyxml2::XMLElement& dialogueElement )
{
	for ( const tinyxml2::XMLElement* dialogueSubElement = dialogueElement.FirstChildElement(); dialogueSubElement != nullptr; dialogueSubElement = dialogueSubElement->NextSiblingElement() )
	{
		if ( std::string( dialogueSubElement->Name() ) == std::string( DIALOGUE_START_XML_NODE_NAME ) )
		{
			m_adventureDialogues[ AdventureDialogueType::ADVENTURE_DIALOGUE_TYPE_START ] = ParseXmlAttribute( *dialogueSubElement, nullptr );
		}
		else if ( std::string( dialogueSubElement->Name() ) == std::string( DIALOGUE_VICTORY_XML_NODE_NAME ) )
		{
			m_adventureDialogues[ AdventureDialogueType::ADVENTURE_DIALOGUE_TYPE_VICTORY ] = ParseXmlAttribute( *dialogueSubElement, nullptr );
		}
		else if ( std::string( dialogueSubElement->Name() ) == std::string( DIALOGUE_DEFEAT_XML_NODE_NAME ) )
		{
			m_adventureDialogues[ AdventureDialogueType::ADVENTURE_DIALOGUE_TYPE_DEFEAT ] = ParseXmlAttribute( *dialogueSubElement, nullptr );
		}
	}
}

std::string AdventureDefinition::GetName() const
{
	return m_name;
}

std::string AdventureDefinition::GetTitle() const
{
	return m_title;
}

StartCondition* AdventureDefinition::GetStartCondition() const
{
	return m_startCondition;
}

std::vector< MapToGenerate* > AdventureDefinition::GetMapsToGenerate() const
{
	return m_mapsToGenerate;
}

std::vector< VictoryCondition* > AdventureDefinition::GetVictoryConditions() const
{
	return m_victoryConditions;
}

Dialogue* AdventureDefinition::GetDialogue( AdventureDialogueType dialogueType ) const
{
	return m_adventureDialogues[ dialogueType ];
}
