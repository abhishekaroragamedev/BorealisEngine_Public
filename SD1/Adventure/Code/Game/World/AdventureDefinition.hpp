#pragma once

#include "Game/Dialogue.hpp"
#include "Game/Entities/ActorDefinition.hpp"
#include "Game/Entities/EntityDefinition.hpp"
#include "Game/Entities/ItemDefinition.hpp"
#include "Game/Entities/PortalDefinition.hpp"
#include "Game/World/MapDefinition.hpp"
#include "Game/World/TileDefinition.hpp"
#include "ThirdParty/tinyXML2/tinyxml2.h"
#include <map>
#include <string>
#include <vector>

enum VictoryConditionType
{
	NO_VICTORY_CONDITION = -1,
	HAVE_DIED,
	HAVE_ITEM,
	NUM_VICTORY_CONDITION_TYPES
};

enum AdventureDialogueType
{
	ADVENTURE_DIALOGUE_TYPES_NONE = -1,
	ADVENTURE_DIALOGUE_TYPE_START,
	ADVENTURE_DIALOGUE_TYPE_VICTORY,
	ADVENTURE_DIALOGUE_TYPE_DEFEAT,
	NUM_ADVENTURE_DIALOGUE_TYPES
};

struct ActorToSpawn
{

public:
	ActorDefinition* m_actorDefinition = nullptr;
	TileDefinition* m_onTileType = nullptr;

};

struct ItemToSpawn
{

public:
	ItemDefinition* m_itemDefinition = nullptr;
	TileDefinition* m_onTileType = nullptr;

};

struct PortalToSpawn
{

public:
	PortalDefinition* m_portalDefinition = nullptr;
	TileDefinition* m_onTileType = nullptr;

	std::string m_toMapName = "";
	TileDefinition* m_toTileType = nullptr;
	PortalDefinition* m_reciprocalPortalDefinition = nullptr;

};

struct MapToGenerate
{

public:
	~MapToGenerate();

public:
	std::string m_name = "";
	MapDefinition* m_mapDefinition = nullptr;
	std::vector< ActorToSpawn* > m_actorsToSpawn;
	std::vector< ItemToSpawn* > m_itemsToSpawn;
	std::vector< PortalToSpawn* > m_portalsToSpawn;

};

struct StartCondition
{

public:
	std::string m_startMapName = "";
	TileDefinition* m_startTileDefinition = nullptr;

};

struct VictoryCondition
{

public:
	VictoryCondition( const std::string& entityDefinitionName, VictoryConditionType victoryConditionType );

public:
	std::string m_entityDefinitionName = "";
	VictoryConditionType m_victoryConditionType = VictoryConditionType::NO_VICTORY_CONDITION;

};

class AdventureDefinition
{

public:
	AdventureDefinition( const tinyxml2::XMLElement& adventureDefinitionElement );
	~AdventureDefinition();

	std::string GetName() const;
	std::string GetTitle() const;
	StartCondition* GetStartCondition() const;
	std::vector< MapToGenerate* > GetMapsToGenerate() const;
	std::vector< VictoryCondition* > GetVictoryConditions() const;
	Dialogue* GetDialogue( AdventureDialogueType dialogueType ) const;

private:
	void PopulateFromXml( const tinyxml2::XMLElement& adventureDefinitionElement );
	void PopulateDialoguesFromXml( const tinyxml2::XMLElement& dialogElement );

public:
	static std::map< std::string, AdventureDefinition* > s_definitions;
	static std::vector< AdventureDefinition* > GetDefinitionsAsVector();

private:
	static constexpr char START_CONDITION_XML_NODE_NAME[] = "StartCondition";
	static constexpr char VICTORY_CONDITION_XML_NODE_NAME[] = "VictoryCondition";
	static constexpr char DIALOGUE_XML_NODE_NAME[] = "Dialogue";
	static constexpr char DIALOGUE_START_XML_NODE_NAME[] = "Start";
	static constexpr char DIALOGUE_VICTORY_XML_NODE_NAME[] = "Victory";
	static constexpr char DIALOGUE_DEFEAT_XML_NODE_NAME[] = "Defeat";
	static constexpr char MAP_XML_NODE_NAME[] = "Map";
	static constexpr char ACTOR_XML_NODE_NAME[] = "Actor";
	static constexpr char ITEM_XML_NODE_NAME[] = "Item";
	static constexpr char PORTAL_XML_NODE_NAME[] = "Portal";
	static constexpr char NAME_XML_ATTRIBUTE_NAME[] = "name";
	static constexpr char TITLE_XML_ATTRIBUTE_NAME[] = "title";
	static constexpr char START_MAP_XML_ATTRIBUTE_NAME[] = "startMap";
	static constexpr char HAVE_DIED_XML_ATTRIBUTE_NAME[] = "haveDied";
	static constexpr char HAVE_ITEM_XML_ATTRIBUTE_NAME[] = "haveItem";
	static constexpr char MAP_DEFINITION_XML_ATTRIBUTE_NAME[] = "mapDefinition";
	static constexpr char TYPE_XML_ATTRIBUTE_NAME[] = "type";
	static constexpr char ON_TILE_TYPE_XML_ATTRIBUTE_NAME[] = "onTileType";
	static constexpr char TO_TILE_TYPE_XML_ATTRIBUTE_NAME[] = "toTileType";
	static constexpr char RECIPROCAL_TYPE_XML_ATTRIBUTE_NAME[] = "reciprocalType";
	static constexpr char TO_MAP_XML_ATTRIBUTE_NAME[] = "toMap";

private:
	std::string m_name = "";
	std::string m_title = "";
	std::vector< MapToGenerate* > m_mapsToGenerate;
	std::vector< VictoryCondition* > m_victoryConditions;
	StartCondition* m_startCondition = nullptr;
	Dialogue* m_adventureDialogues[ AdventureDialogueType::NUM_ADVENTURE_DIALOGUE_TYPES ];

};
