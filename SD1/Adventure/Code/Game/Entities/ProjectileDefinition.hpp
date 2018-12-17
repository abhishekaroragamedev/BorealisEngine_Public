#pragma once

#include "Game/Entities/EntityDefinition.hpp"
#include "ThirdParty/tinyXML2/tinyxml2.h"
#include <map>
#include <string>

enum ProjectileAttackActorSet
{
	PROJECTILE_ACTOR_SET_TYPE_NONE = -1,
	PROJECTILE_ACTOR_SET_DAMAGE_OTHER_ACTORS,
	PROJECTILE_ACTOR_SET_DAMAGE_OTHER_FACTIONS,
	PROJECTILE_ACTOR_SET_DAMAGE_SPECIFIC_FACTION,
	NUM_PROJECTILE_ACTOR_SET_TYPES
};

class ProjectileDefinition: public EntityDefinition
{

	friend class TheGame;

public:
	~ProjectileDefinition();

private:
	explicit ProjectileDefinition( const tinyxml2::XMLElement& projectileDefinitionElement );

public:
	int GetBaseStat( StatID statID ) const;
	float GetKnockbackFraction() const;
	ProjectileAttackActorSet GetAttackActorSet() const;
	std::string GetVulnerableFactionName() const;
	float GetLifetime() const;

private:
	void PopulateDataFromXml( const tinyxml2::XMLElement& projectileDefinitionElement ) override;
	void PopulateStatsFromXml( const tinyxml2::XMLElement& statsElement );

public:
	static std::map< std::string, ProjectileDefinition* > s_definitions;
	static ProjectileAttackActorSet GetAttackTypeFromAttackTypeName( const std::string& attackTypeName );
	static std::string GetAttackTypeNameFromAttackType( ProjectileAttackActorSet attackType );

private:
	static constexpr char ATTACK_XML_NODE_NAME[] = "Attack";
	static constexpr char STATS_XML_NODE_NAME[] = "Stats";
	static constexpr char LIFETIME_XML_ATTRIBUTE_NAME[] = "lifetime";
	static constexpr char ACTOR_SET_XML_ATTRIBUTE_NAME[] = "actorSet";
	static constexpr char ACTOR_SET_FACTION_XML_ATTRIBUTE_NAME[] = "faction";
	static constexpr char HEALTH_XML_ATTRIBUTE_NAME[] = "health";
	static constexpr char STRENGTH_XML_ATTRIBUTE_NAME[] = "strength";
	static constexpr char RESILIENCE_XML_ATTRIBUTE_NAME[] = "resilience";
	static constexpr char AGILITY_XML_ATTRIBUTE_NAME[] = "agility";
	static constexpr char DEXTERITY_XML_ATTRIBUTE_NAME[] = "dexterity";
	static constexpr char POISON_USAGE_XML_ATTRIBUTE_NAME[] = "poison";
	static constexpr char FIRE_USAGE_XML_ATTRIBUTE_NAME[] = "fire";
	static constexpr char ICE_USAGE_XML_ATTRIBUTE_NAME[] = "ice";
	static constexpr char ELECTRICITY_USAGE_XML_ATTRIBUTE_NAME[] = "electricity";
	static constexpr char KNOCKBACK_XML_ATTRIBUTE_NAME[] = "knockback";

private:
	int m_baseStats[ StatID::NUM_STATS ];
	float m_knockbackFraction = 0.0f;
	ProjectileAttackActorSet m_actorSet = ProjectileAttackActorSet::PROJECTILE_ACTOR_SET_TYPE_NONE;
	std::string m_vulnerableFactionName = "";
	float m_lifetime = 0.0f;

};

ProjectileDefinition* ParseXmlAttribute( const tinyxml2::XMLElement& element, const char* attributeName, ProjectileDefinition* defaultValue );
