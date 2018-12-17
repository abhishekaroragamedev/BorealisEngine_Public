#pragma once

#include "Game/Dialogue.hpp"
#include "Game/Entities/EntityDefinition.hpp"
#include "Game/Entities/ItemDefinition.hpp"
#include "Game/Entities/LootDefinition.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/IntRange.hpp"
#include "Engine/Math/Vector2.hpp"
#include "ThirdParty/tinyXML2/tinyxml2.h"
#include <map>
#include <string>

enum ActorDialogueType
{
	ACTOR_DIALOGUE_TYPES_NONE = -1,
	ACTOR_DIALOGUE_TYPE_SPAWN,
	ACTOR_DIALOGUE_TYPE_PROXIMITY,
	ACTOR_DIALOGUE_TYPE_DEATH,
	NUM_ACTOR_DIALOGUE_TYPES
};

struct ActorDefaultItem
{

public:
	explicit ActorDefaultItem( ItemDefinition* itemDefinition, float chanceOfSpawning );

public:
	ItemDefinition* m_defaultItemDefinition = nullptr;
	float m_chanceOfSpawning = 0.0;

};

struct ActorLoot
{

public:
	explicit ActorLoot( LootDefinition* lootDefinition, float chanceOfSpawning );

public:
	LootDefinition* m_lootDefinition = nullptr;
	float m_chanceOfSpawning = 0.0;

};

class ActorDefinition: public EntityDefinition
{

	friend class TheGame;

public:
	~ActorDefinition();

private:
	explicit ActorDefinition( const tinyxml2::XMLElement& actorDefinitionElement );

public:
	std::string GetFactionName() const;
	IntRange GetStatRange( StatID statID ) const;
	bool IgnoresActorPhysics() const;
	float GetPhysicsMass() const;
	std::vector< ActorDefaultItem > GetDefaultItems() const;
	std::vector< ActorLoot > GetLootTypes() const;
	Dialogue* GetDialogue( ActorDialogueType dialogueType ) const;
	float GetProximityDialogueRadius() const;

private:
	void PopulateDataFromXml( const tinyxml2::XMLElement& actorDefinitionElement ) override;
	void PopulateMinStatsFromXml( const tinyxml2::XMLElement& statsElement );
	void PopulateMaxStatsFromXml( const tinyxml2::XMLElement& statsElement );
	void PopulateDialoguesFromXml( const tinyxml2::XMLElement& dialogElement );

public:
	static std::map< std::string, ActorDefinition* > s_definitions;

private:
	static constexpr char MIN_STATS_XML_NODE_NAME[] = "MinStats";
	static constexpr char MAX_STATS_XML_NODE_NAME[] = "MaxStats";
	static constexpr char DEFAULT_ITEMS_XML_NODE_NAME[] = "DefaultItems";
	static constexpr char ITEM_XML_NODE_NAME[] = "Item";
	static constexpr char LOOT_TO_DROP_XML_NODE_NAME[] = "LootToDrop";
	static constexpr char LOOT_XML_NODE_NAME[] = "Loot";
	static constexpr char DIALOGUE_XML_NODE_NAME[] = "Dialogue";
	static constexpr char DIALOGUE_SPAWN_XML_NODE_NAME[] = "Spawn";
	static constexpr char DIALOGUE_PROXIMITY_XML_NODE_NAME[] = "Proximity";
	static constexpr char DIALOGUE_DEATH_XML_NODE_NAME[] = "Death";
	static constexpr char ITEM_NAME_XML_ATTRIBUTE_NAME[] = "name";
	static constexpr char ITEM_CHANCE_XML_ATTRIBUTE_NAME[] = "chance";
	static constexpr char LOOT_TYPE_XML_ATTRIBUTE_NAME[] = "type";
	static constexpr char LOOT_CHANCE_XML_ATTRIBUTE_NAME[] = "chance";
	static constexpr char IGNORES_ACTOR_PHYSICS_XML_ATTRIBUTE_NAME[] = "ignoresActorPhysics";
	static constexpr char PHYSICS_MASS_XML_ATTRIBUTE_NAME[] = "physicsMass";
	static constexpr char FACTION_NAME_XML_ATTRIBUTE_NAME[] = "faction";
	static constexpr char HEALTH_XML_ATTRIBUTE_NAME[] = "health";
	static constexpr char STRENGTH_XML_ATTRIBUTE_NAME[] = "strength";
	static constexpr char RESILIENCE_XML_ATTRIBUTE_NAME[] = "resilience";
	static constexpr char AGILITY_XML_ATTRIBUTE_NAME[] = "agility";
	static constexpr char DEXTERITY_XML_ATTRIBUTE_NAME[] = "dexterity";
	static constexpr char AIM_XML_ATTRIBUTE_NAME[] = "aim";
	static constexpr char ACCURACY_XML_ATTRIBUTE_NAME[] = "accuracy";
	static constexpr char EVASIVENESS_XML_ATTRIBUTE_NAME[] = "evasiveness";
	static constexpr char POISON_USAGE_XML_ATTRIBUTE_NAME[] = "poison";
	static constexpr char FIRE_USAGE_XML_ATTRIBUTE_NAME[] = "fire";
	static constexpr char ICE_USAGE_XML_ATTRIBUTE_NAME[] = "ice";
	static constexpr char ELECTRICITY_USAGE_XML_ATTRIBUTE_NAME[] = "electricity";
	static constexpr char DIALOGUE_PROXIMITY_UNITS_XML_ATTRIBUTE_NAME[] = "units";

private:
	std::string m_factionName = "";
	std::vector< ActorDefaultItem > m_defaultItems;
	std::vector< ActorLoot > m_lootTypes;
	int m_minStats[ StatID::NUM_STATS ];
	int m_maxStats[ StatID::NUM_STATS ];
	bool m_ignoresActorPhysics = false;
	float m_physicsMass = 0.0f;
	float m_proximityDialogueRadius = 0.0f;
	Dialogue* m_actorDialogues[ ActorDialogueType::NUM_ACTOR_DIALOGUE_TYPES ];

};

ActorDefinition* ParseXmlAttribute( const tinyxml2::XMLElement& element, const char* attributeName, ActorDefinition* defaultValue );
