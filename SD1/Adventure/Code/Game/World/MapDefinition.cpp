#include "Game/World/MapDefinition.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/XmlUtilities.hpp"

std::map< std::string, MapDefinition* > MapDefinition::s_definitions;

MapDefinition::MapDefinition( const tinyxml2::XMLElement& mapDefinitionElement )
{
	PopulateDataFromXml( mapDefinitionElement );
}

MapDefinition::~MapDefinition()
{
	for ( std::vector< MapGenStep* >::iterator mapGenStepIterator = m_mapGenSteps.begin(); mapGenStepIterator != m_mapGenSteps.end(); mapGenStepIterator++ )
	{
		delete *mapGenStepIterator;		// Deallocate memory for MapGenSteps
		*mapGenStepIterator = nullptr;
	}
}

std::string MapDefinition::GetName() const
{
	return m_name;
}

TileDefinition* MapDefinition::GetDefaultTileDefinition() const
{
	return m_defaultTile;
}

IntRange MapDefinition::GetWidthRange() const
{
	return m_widthRange;
}

IntRange MapDefinition::GetHeightRange() const
{
	return m_heightRange;
}

const std::vector< MapGenStep* > MapDefinition::GetMapGenSteps() const
{
	return m_mapGenSteps;
}

void MapDefinition::PopulateDataFromXml( const tinyxml2::XMLElement& mapDefinitionElement )
{
	m_name = ParseXmlAttribute( mapDefinitionElement, NAME_XML_ATTRIBUTE_NAME, m_name );
	m_defaultTile = ParseXmlAttribute( mapDefinitionElement, DEFAULT_TILE_XML_ATTRIBUTE_NAME, m_defaultTile );
	m_widthRange = ParseXmlAttribute( mapDefinitionElement, WIDTH_XML_ATTRIBUTE_NAME, m_widthRange );
	m_heightRange = ParseXmlAttribute( mapDefinitionElement, HEIGHT_XML_ATTRIBUTE_NAME, m_heightRange );

	ASSERT_OR_DIE( !mapDefinitionElement.NoChildren(), "MapDefinition::PopulateDataFromXml - <MapDefinition> node must have a <GenerationSteps> node, but no children found. Aborting..." );
	PopulateMapGenStepsFromXml( *mapDefinitionElement.FirstChildElement() );

	MapDefinition::s_definitions[ m_name ] = this;
}

void MapDefinition::PopulateMapGenStepsFromXml( const tinyxml2::XMLElement& generationStepsElement )
{
	ASSERT_OR_DIE( !generationStepsElement.NoChildren(), "MapDefinition::PopulateMapGenStepsFromXml - <GenerationSteps> node must have at least one child Generation Step node, but no children found. Aborting..." );
	for ( const tinyxml2::XMLElement* generationStepElement = generationStepsElement.FirstChildElement(); generationStepElement != nullptr; generationStepElement = generationStepElement->NextSiblingElement() )
	{
		m_mapGenSteps.push_back( MapGenStep::CreateMapGenStep( *generationStepElement ) );
	}
}

MapDefinition* ParseXmlAttribute( const tinyxml2::XMLElement& element, const char* attributeName, MapDefinition* defaultValue )
{
	const char* mapDefinitionText = element.Attribute( attributeName );
	if ( mapDefinitionText == nullptr )
	{
		return defaultValue;
	}

	std::string mapDefinitionKey = std::string( mapDefinitionText );
	if ( MapDefinition::s_definitions.find( mapDefinitionKey ) == MapDefinition::s_definitions.end() )
	{
		return defaultValue;
	}

	return MapDefinition::s_definitions[ mapDefinitionKey ];
}
