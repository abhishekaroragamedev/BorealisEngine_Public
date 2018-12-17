#include "Game/Entities/ActorDefinition.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/XmlUtilities.hpp"

ActorDefaultItem::ActorDefaultItem( ItemDefinition* itemDefinition, float chanceOfSpawning )
{
	m_defaultItemDefinition = itemDefinition;
	m_chanceOfSpawning = chanceOfSpawning;
}

ActorLoot::ActorLoot( LootDefinition* lootDefinition, float chanceOfSpawning )
{
	m_lootDefinition = lootDefinition;
	m_chanceOfSpawning = chanceOfSpawning;
}

std::map< std::string, ActorDefinition* > ActorDefinition::s_definitions;

ActorDefinition::ActorDefinition( const tinyxml2::XMLElement& actorDefinitionElement ) : EntityDefinition( actorDefinitionElement )
{
	PopulateDataFromXml( actorDefinitionElement );
}

ActorDefinition::~ActorDefinition()
{
	
}

std::string ActorDefinition::GetFactionName() const
{
	return m_factionName;
}

IntRange ActorDefinition::GetStatRange( StatID statID ) const
{
	return IntRange( m_minStats[ statID ], m_maxStats[ statID ] );
}

bool ActorDefinition::IgnoresActorPhysics() const
{
	return m_ignoresActorPhysics;
}

float ActorDefinition::GetPhysicsMass() const
{
	return m_physicsMass;
}

std::vector< ActorDefaultItem > ActorDefinition::GetDefaultItems() const
{
	return m_defaultItems;
}

std::vector< ActorLoot > ActorDefinition::GetLootTypes() const
{
	return m_lootTypes;
}

Dialogue* ActorDefinition::GetDialogue( ActorDialogueType dialogueType ) const
{
	return m_actorDialogues[ dialogueType ];
}

float ActorDefinition::GetProximityDialogueRadius() const
{
	return m_proximityDialogueRadius;
}

void ActorDefinition::PopulateDataFromXml( const tinyxml2::XMLElement& actorDefinitionElement )
{
	EntityDefinition::PopulateDataFromXml( actorDefinitionElement );
	m_factionName = ParseXmlAttribute( actorDefinitionElement, FACTION_NAME_XML_ATTRIBUTE_NAME, m_factionName );

	if ( !actorDefinitionElement.NoChildren() )
	{
		for ( const tinyxml2::XMLElement* actorDefSubElement = actorDefinitionElement.FirstChildElement(); actorDefSubElement != nullptr; actorDefSubElement = actorDefSubElement->NextSiblingElement() )
		{
			if ( std::string( actorDefSubElement->Name() ) == std::string( MIN_STATS_XML_NODE_NAME ) )
			{
				PopulateMinStatsFromXml( *actorDefSubElement );
			}
			else if ( std::string( actorDefSubElement->Name() ) == std::string( MAX_STATS_XML_NODE_NAME ) )
			{
				PopulateMaxStatsFromXml( *actorDefSubElement );
			}
			else if ( std::string( actorDefSubElement->Name() ) == std::string( PHYSICS_XML_NODE_NAME ) )
			{
				m_ignoresActorPhysics = ParseXmlAttribute( *actorDefSubElement, IGNORES_ACTOR_PHYSICS_XML_ATTRIBUTE_NAME, m_ignoresActorPhysics );
				m_physicsMass = ParseXmlAttribute( *actorDefSubElement, PHYSICS_MASS_XML_ATTRIBUTE_NAME, m_physicsMass );
			}
			else if ( std::string( actorDefSubElement->Name() ) == std::string( DEFAULT_ITEMS_XML_NODE_NAME ) )
			{
				for ( const tinyxml2::XMLElement* defaultItemElement = actorDefSubElement->FirstChildElement(); defaultItemElement != nullptr; defaultItemElement = defaultItemElement->NextSiblingElement() )
				{
					std::string itemDefName = ParseXmlAttribute( *defaultItemElement, ITEM_NAME_XML_ATTRIBUTE_NAME, "" );
					ASSERT_OR_DIE( ItemDefinition::s_definitions.find( itemDefName ) != ItemDefinition::s_definitions.end(), "ActorDefinition::PopulateDataFromXml - Could not find item definition with the provided name. Aborting..." );
					ItemDefinition* defaultItemDefinition = ItemDefinition::s_definitions[ itemDefName ];
					float chanceOfSpawning = ParseXmlAttribute( *defaultItemElement, ITEM_CHANCE_XML_ATTRIBUTE_NAME, 1.0f );		// If no chance is specified, the item will surely spawn
					
					ActorDefaultItem defaultItemForActor = ActorDefaultItem( defaultItemDefinition, chanceOfSpawning );
					m_defaultItems.push_back( defaultItemForActor );
				}
			}
			else if ( std::string( actorDefSubElement->Name() ) == std::string( LOOT_TO_DROP_XML_NODE_NAME ) )
			{
				for ( const tinyxml2::XMLElement* lootElement = actorDefSubElement->FirstChildElement(); lootElement != nullptr; lootElement = lootElement->NextSiblingElement() )
				{
					std::string lootDefName = ParseXmlAttribute( *lootElement, LOOT_TYPE_XML_ATTRIBUTE_NAME, "" );
					ASSERT_OR_DIE( LootDefinition::s_definitions.find( lootDefName ) != LootDefinition::s_definitions.end(), "ActorDefinition::PopulateDataFromXml - Could not find loot definition with the provided name. Aborting..." );
					LootDefinition* lootDefinition = LootDefinition::s_definitions[ lootDefName ];
					float chanceOfSpawning = ParseXmlAttribute( *lootElement, LOOT_CHANCE_XML_ATTRIBUTE_NAME, 1.0f );		// If no chance is specified, the loot will certainly spawn
					ActorLoot defaultItemForActor = ActorLoot( lootDefinition, chanceOfSpawning );
					m_lootTypes.push_back( defaultItemForActor );
				}
			}
			else if ( std::string( actorDefSubElement->Name() ) == std::string( DIALOGUE_XML_NODE_NAME ) )
			{
				PopulateDialoguesFromXml( *actorDefSubElement );
			}
		}
	}

	s_definitions[ m_name ] = this;
}

void ActorDefinition::PopulateDialoguesFromXml( const tinyxml2::XMLElement& dialogueElement )
{
	for ( const tinyxml2::XMLElement* dialogueSubElement = dialogueElement.FirstChildElement(); dialogueSubElement != nullptr; dialogueSubElement = dialogueSubElement->NextSiblingElement() )
	{
		if ( std::string( dialogueSubElement->Name() ) == std::string( DIALOGUE_SPAWN_XML_NODE_NAME ) )
		{
			m_actorDialogues[ ActorDialogueType::ACTOR_DIALOGUE_TYPE_SPAWN ] = ParseXmlAttribute( *dialogueSubElement, nullptr );
		}
		else if ( std::string( dialogueSubElement->Name() ) == std::string( DIALOGUE_PROXIMITY_XML_NODE_NAME ) )
		{
			m_actorDialogues[ ActorDialogueType::ACTOR_DIALOGUE_TYPE_PROXIMITY ] = ParseXmlAttribute( *dialogueSubElement, nullptr );
			m_proximityDialogueRadius = ParseXmlAttribute( *dialogueSubElement, DIALOGUE_PROXIMITY_UNITS_XML_ATTRIBUTE_NAME, m_proximityDialogueRadius );
		}
		else if ( std::string( dialogueSubElement->Name() ) == std::string( DIALOGUE_DEATH_XML_NODE_NAME ) )
		{
			m_actorDialogues[ ActorDialogueType::ACTOR_DIALOGUE_TYPE_DEATH ] = ParseXmlAttribute( *dialogueSubElement, nullptr );
		}
	}
}

void ActorDefinition::PopulateMinStatsFromXml( const tinyxml2::XMLElement& statsElement )
{
	m_minStats[ StatID::STAT_HEALTH ] = ParseXmlAttribute( statsElement, HEALTH_XML_ATTRIBUTE_NAME, 0 );
	m_minStats[ StatID::STAT_STRENGTH ] = ParseXmlAttribute( statsElement, STRENGTH_XML_ATTRIBUTE_NAME, 0 );
	m_minStats[ StatID::STAT_RESILIENCE ] = ParseXmlAttribute( statsElement, RESILIENCE_XML_ATTRIBUTE_NAME, 0 );
	m_minStats[ StatID::STAT_AGILITY ] = ParseXmlAttribute( statsElement, AGILITY_XML_ATTRIBUTE_NAME, 0 );
	m_minStats[ StatID::STAT_DEXTERITY ] = ParseXmlAttribute( statsElement, DEXTERITY_XML_ATTRIBUTE_NAME, 0 );
	m_minStats[ StatID::STAT_AIM ] =  ParseXmlAttribute( statsElement, AIM_XML_ATTRIBUTE_NAME, 0 );
	m_minStats[ StatID::STAT_ACCURACY ] = ParseXmlAttribute( statsElement, ACCURACY_XML_ATTRIBUTE_NAME, 0 );
	m_minStats[ StatID::STAT_EVASIVENESS ] = ParseXmlAttribute( statsElement, EVASIVENESS_XML_ATTRIBUTE_NAME, 0 );
	m_minStats[ StatID::STAT_POISON_USAGE ] = ParseXmlAttribute( statsElement, POISON_USAGE_XML_ATTRIBUTE_NAME, 0 );
	m_minStats[ StatID::STAT_FIRE_USAGE ] = ParseXmlAttribute( statsElement, FIRE_USAGE_XML_ATTRIBUTE_NAME, 0 );
	m_minStats[ StatID::STAT_ICE_USAGE ] = ParseXmlAttribute( statsElement, ICE_USAGE_XML_ATTRIBUTE_NAME, 0 );
	m_minStats[ StatID::STAT_ELECTRICITY_USAGE ] = ParseXmlAttribute( statsElement, ELECTRICITY_USAGE_XML_ATTRIBUTE_NAME, 0 );
}

void ActorDefinition::PopulateMaxStatsFromXml( const tinyxml2::XMLElement& statsElement )
{
	m_maxStats[ StatID::STAT_HEALTH ] = ParseXmlAttribute( statsElement, HEALTH_XML_ATTRIBUTE_NAME, m_minStats[ StatID::STAT_HEALTH ] );
	m_maxStats[ StatID::STAT_STRENGTH ] = ParseXmlAttribute( statsElement, STRENGTH_XML_ATTRIBUTE_NAME, m_minStats[ StatID::STAT_STRENGTH ] );
	m_maxStats[ StatID::STAT_RESILIENCE ] = ParseXmlAttribute( statsElement, RESILIENCE_XML_ATTRIBUTE_NAME, m_minStats[ StatID::STAT_RESILIENCE ] );
	m_maxStats[ StatID::STAT_AGILITY ] = ParseXmlAttribute( statsElement, AGILITY_XML_ATTRIBUTE_NAME, m_minStats[ StatID::STAT_AGILITY ] );
	m_maxStats[ StatID::STAT_DEXTERITY ] = ParseXmlAttribute( statsElement, DEXTERITY_XML_ATTRIBUTE_NAME, m_minStats[ StatID::STAT_DEXTERITY ] );
	m_maxStats[ StatID::STAT_AIM ] =  ParseXmlAttribute( statsElement, AIM_XML_ATTRIBUTE_NAME, 0 );
	m_maxStats[ StatID::STAT_ACCURACY ] = ParseXmlAttribute( statsElement, ACCURACY_XML_ATTRIBUTE_NAME, 0 );
	m_maxStats[ StatID::STAT_EVASIVENESS ] = ParseXmlAttribute( statsElement, EVASIVENESS_XML_ATTRIBUTE_NAME, 0 );
	m_maxStats[ StatID::STAT_POISON_USAGE ] = ParseXmlAttribute( statsElement, POISON_USAGE_XML_ATTRIBUTE_NAME, 0 );
	m_maxStats[ StatID::STAT_FIRE_USAGE ] = ParseXmlAttribute( statsElement, FIRE_USAGE_XML_ATTRIBUTE_NAME, 0 );
	m_maxStats[ StatID::STAT_ICE_USAGE ] = ParseXmlAttribute( statsElement, ICE_USAGE_XML_ATTRIBUTE_NAME, 0 );
	m_maxStats[ StatID::STAT_ELECTRICITY_USAGE ] = ParseXmlAttribute( statsElement, ELECTRICITY_USAGE_XML_ATTRIBUTE_NAME, 0 );
}

ActorDefinition* ParseXmlAttribute( const tinyxml2::XMLElement& element, const char* attributeName, ActorDefinition* defaultValue )
{
	const char* actorDefinitionText = element.Attribute( attributeName );
	if ( actorDefinitionText == nullptr )
	{
		return defaultValue;
	}

	std::string actorDefinitionKey = std::string( actorDefinitionText );
	if ( ActorDefinition::s_definitions.find( actorDefinitionKey ) == ActorDefinition::s_definitions.end() )
	{
		return defaultValue;
	}

	return ActorDefinition::s_definitions[ actorDefinitionKey ];
}
