#include "Game/Entities/ProjectileDefinition.hpp"
#include "Engine/Core/XmlUtilities.hpp"

std::map< std::string, ProjectileDefinition* > ProjectileDefinition::s_definitions;

ProjectileAttackActorSet ProjectileDefinition::GetAttackTypeFromAttackTypeName( const std::string& attackTypeName )
{
	if ( attackTypeName == "otherActors" )
	{
		return ProjectileAttackActorSet::PROJECTILE_ACTOR_SET_DAMAGE_OTHER_ACTORS;
	}
	else if ( attackTypeName == "otherFactions" )
	{
		return ProjectileAttackActorSet::PROJECTILE_ACTOR_SET_DAMAGE_OTHER_FACTIONS;
	}
	else if ( attackTypeName == "faction" )
	{
		return ProjectileAttackActorSet::PROJECTILE_ACTOR_SET_DAMAGE_SPECIFIC_FACTION;
	}
	else
	{
		return ProjectileAttackActorSet::PROJECTILE_ACTOR_SET_TYPE_NONE;
	}
}

std::string ProjectileDefinition::GetAttackTypeNameFromAttackType( ProjectileAttackActorSet attackType )
{
	switch( attackType )
	{
		case ProjectileAttackActorSet::PROJECTILE_ACTOR_SET_DAMAGE_OTHER_ACTORS		:	return "otherActors";
		case ProjectileAttackActorSet::PROJECTILE_ACTOR_SET_DAMAGE_OTHER_FACTIONS		:	return "otherFactions";
		case ProjectileAttackActorSet::PROJECTILE_ACTOR_SET_DAMAGE_SPECIFIC_FACTION	:	return "faction";
		default																		:	return "";
	}
}

ProjectileDefinition::ProjectileDefinition( const tinyxml2::XMLElement& projectileDefinitionElement ) : EntityDefinition( projectileDefinitionElement )
{
	PopulateDataFromXml( projectileDefinitionElement );
}

ProjectileDefinition::~ProjectileDefinition()
{

}

int ProjectileDefinition::GetBaseStat( StatID statID ) const
{
	return m_baseStats[ statID ];
}

float ProjectileDefinition::GetKnockbackFraction() const
{
	return m_knockbackFraction;
}

ProjectileAttackActorSet ProjectileDefinition::GetAttackActorSet() const
{
	return m_actorSet;
}

std::string ProjectileDefinition::GetVulnerableFactionName() const
{
	return m_vulnerableFactionName;
}

float ProjectileDefinition::GetLifetime() const
{
	return m_lifetime;
}

void ProjectileDefinition::PopulateDataFromXml( const tinyxml2::XMLElement& projectileDefinitionElement )
{
	EntityDefinition::PopulateDataFromXml( projectileDefinitionElement );
	m_lifetime = ParseXmlAttribute( projectileDefinitionElement, LIFETIME_XML_ATTRIBUTE_NAME, m_lifetime );

	if ( !projectileDefinitionElement.NoChildren() )
	{
		for ( const tinyxml2::XMLElement* projectileDefinitionSubElement = projectileDefinitionElement.FirstChildElement(); projectileDefinitionSubElement != nullptr; projectileDefinitionSubElement = projectileDefinitionSubElement->NextSiblingElement() )
		{
			if ( std::string( projectileDefinitionSubElement->Name() ) == std::string( STATS_XML_NODE_NAME ) )
			{
				PopulateStatsFromXml( *projectileDefinitionSubElement );
			}
			else if ( std::string( projectileDefinitionSubElement->Name() ) == std::string( ATTACK_XML_NODE_NAME ) )
			{
				m_actorSet = ProjectileDefinition::GetAttackTypeFromAttackTypeName( ParseXmlAttribute( *projectileDefinitionSubElement, ACTOR_SET_XML_ATTRIBUTE_NAME, "" ) );
				if ( m_actorSet == ProjectileAttackActorSet::PROJECTILE_ACTOR_SET_DAMAGE_SPECIFIC_FACTION )
				{
					m_vulnerableFactionName = ParseXmlAttribute( *projectileDefinitionSubElement, ACTOR_SET_FACTION_XML_ATTRIBUTE_NAME, m_vulnerableFactionName );
				}
			}
			else if ( std::string( projectileDefinitionSubElement->Name() ) == std::string( PHYSICS_XML_NODE_NAME ) )
			{
				m_knockbackFraction = ParseXmlAttribute( *projectileDefinitionSubElement, KNOCKBACK_XML_ATTRIBUTE_NAME, m_knockbackFraction );
			}
		}
	}
	s_definitions[ m_name ] = this;
}

void ProjectileDefinition::PopulateStatsFromXml( const tinyxml2::XMLElement& statsElement )
{
	m_baseStats[ StatID::STAT_HEALTH ] = ParseXmlAttribute( statsElement, HEALTH_XML_ATTRIBUTE_NAME, 0 );
	m_baseStats[ StatID::STAT_STRENGTH ] = ParseXmlAttribute( statsElement, STRENGTH_XML_ATTRIBUTE_NAME, 0 );
	m_baseStats[ StatID::STAT_RESILIENCE ] = ParseXmlAttribute( statsElement, RESILIENCE_XML_ATTRIBUTE_NAME, 0 );
	m_baseStats[ StatID::STAT_AGILITY ] = ParseXmlAttribute( statsElement, AGILITY_XML_ATTRIBUTE_NAME, 0 );
	m_baseStats[ StatID::STAT_DEXTERITY ] = ParseXmlAttribute( statsElement, DEXTERITY_XML_ATTRIBUTE_NAME, 0 );
	m_baseStats[ StatID::STAT_POISON_USAGE ] = ParseXmlAttribute( statsElement, POISON_USAGE_XML_ATTRIBUTE_NAME, 0 );
	m_baseStats[ StatID::STAT_FIRE_USAGE ] = ParseXmlAttribute( statsElement, FIRE_USAGE_XML_ATTRIBUTE_NAME, 0 );
	m_baseStats[ StatID::STAT_ICE_USAGE ] = ParseXmlAttribute( statsElement, ICE_USAGE_XML_ATTRIBUTE_NAME, 0 );
	m_baseStats[ StatID::STAT_ELECTRICITY_USAGE ] = ParseXmlAttribute( statsElement, ELECTRICITY_USAGE_XML_ATTRIBUTE_NAME, 0 );
}

ProjectileDefinition* ParseXmlAttribute( const tinyxml2::XMLElement& element, const char* attributeName, ProjectileDefinition* defaultValue )
{
	const char* projectileDefinitionText = element.Attribute( attributeName );
	if ( projectileDefinitionText == nullptr )
	{
		return defaultValue;
	}

	std::string projectileDefinitionKey = std::string( projectileDefinitionText );
	if ( ProjectileDefinition::s_definitions.find( projectileDefinitionKey ) == ProjectileDefinition::s_definitions.end() )
	{
		return defaultValue;
	}

	return ProjectileDefinition::s_definitions[ projectileDefinitionKey ];
}
