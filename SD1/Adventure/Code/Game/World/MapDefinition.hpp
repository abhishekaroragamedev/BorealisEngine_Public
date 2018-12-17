#pragma once

#include "Game/World/MapGenStep.hpp"
#include "Game/World/TileDefinition.hpp"
#include "Engine/Math/IntRange.hpp"
#include "Engine/Math/Vector2.hpp"
#include "ThirdParty/tinyXML2/tinyxml2.h"
#include <map>
#include <string>
#include <vector>

class MapDefinition		// TODO: Own MapGenSteps once the class is created
{

	friend class TheGame;

private:
	explicit MapDefinition( const tinyxml2::XMLElement& mapDefinitionElement );

public:
	~MapDefinition();

public:
	std::string GetName() const;
	TileDefinition* GetDefaultTileDefinition() const;
	IntRange GetWidthRange() const;
	IntRange GetHeightRange() const;
	const std::vector< MapGenStep* > GetMapGenSteps() const;

private:
	void PopulateDataFromXml( const tinyxml2::XMLElement& mapDefinitionElement );
	void PopulateMapGenStepsFromXml( const tinyxml2::XMLElement& generationStepsElement );

private:
	static constexpr char NAME_XML_ATTRIBUTE_NAME[] = "name";
	static constexpr char DEFAULT_TILE_XML_ATTRIBUTE_NAME[] = "defaultTile";
	static constexpr char WIDTH_XML_ATTRIBUTE_NAME[] = "width";
	static constexpr char HEIGHT_XML_ATTRIBUTE_NAME[] = "height";

public:
	static std::map< std::string, MapDefinition* >	s_definitions;

private:
	std::string m_name = "";
	TileDefinition* m_defaultTile = nullptr;
	IntRange m_widthRange = IntRange(0, 0);
	IntRange m_heightRange = IntRange(0, 0);
	std::vector< MapGenStep* > m_mapGenSteps;

};

MapDefinition* ParseXmlAttribute( const tinyxml2::XMLElement& element, const char* attributeName, MapDefinition* defaultValue );
