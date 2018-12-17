#pragma once

#include "Game/Entities/EntityDefinition.hpp"
#include "Game/Entities/ItemDefinition.hpp"
#include "Engine/Math/Vector2.hpp"
#include "ThirdParty/tinyXML2/tinyxml2.h"
#include <map>
#include <string>

class LootDefinition
{

	friend class TheGame;

public:
	~LootDefinition();

private:
	explicit LootDefinition( const tinyxml2::XMLElement& lootDefinitionElement );

public:
	std::string GetName() const;
	std::vector< ItemDefinition* > GetItemDefinitions() const;

private:
	void PopulateDataFromXml( const tinyxml2::XMLElement& lootDefinitionElement );

public:
	static std::map< std::string, LootDefinition* > s_definitions;

private:
	static constexpr char LOOT_NAME_XML_ATTRIBUTE_NAME[] = "name";
	static constexpr char ITEM_TYPE_XML_ATTRIBUTE_NAME[] = "type";

private:
	std::string m_name = "";
	std::vector< ItemDefinition* > m_itemDefinitions;

};

LootDefinition* ParseXmlAttribute( const tinyxml2::XMLElement& element, const char* attributeName, LootDefinition* defaultValue );