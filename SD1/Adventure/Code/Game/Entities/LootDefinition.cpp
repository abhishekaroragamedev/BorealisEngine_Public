#include "Game/Entities/LootDefinition.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

std::map< std::string, LootDefinition* > LootDefinition::s_definitions;

LootDefinition::LootDefinition( const tinyxml2::XMLElement& lootDefinitionElement )
{
	PopulateDataFromXml( lootDefinitionElement );
}

LootDefinition::~LootDefinition()
{

}

std::string LootDefinition::GetName() const
{
	return m_name;
}

std::vector< ItemDefinition* > LootDefinition::GetItemDefinitions() const
{
	return m_itemDefinitions;
}

void LootDefinition::PopulateDataFromXml( const tinyxml2::XMLElement& lootDefinitionElement )
{
	m_name = ParseXmlAttribute( lootDefinitionElement, LOOT_NAME_XML_ATTRIBUTE_NAME, m_name );

	if ( !lootDefinitionElement.NoChildren() )
	{
		for ( const tinyxml2::XMLElement* lootDefSubElement = lootDefinitionElement.FirstChildElement(); lootDefSubElement != nullptr; lootDefSubElement = lootDefSubElement->NextSiblingElement() )
		{
			std::string itemDefName = ParseXmlAttribute( *lootDefSubElement, ITEM_TYPE_XML_ATTRIBUTE_NAME, "" );
			ASSERT_OR_DIE( ItemDefinition::s_definitions.find( itemDefName ) != ItemDefinition::s_definitions.end(), "LootDefinition::PopulateDataFromXml - could not find ItemDefinition with the name specified. Aborting..." );
			ItemDefinition* itemDefInLoot = ItemDefinition::s_definitions[ itemDefName ];
			m_itemDefinitions.push_back( itemDefInLoot );
		}
	}

	LootDefinition::s_definitions[ m_name ] = this;
}

LootDefinition* ParseXmlAttribute( const tinyxml2::XMLElement& element, const char* attributeName, LootDefinition* defaultValue )
{
	const char* lootDefinitionText = element.Attribute( attributeName );
	if ( lootDefinitionText == nullptr )
	{
		return defaultValue;
	}

	std::string lootDefinitionKey = std::string( lootDefinitionText );
	if ( LootDefinition::s_definitions.find( lootDefinitionKey ) == LootDefinition::s_definitions.end() )
	{
		return defaultValue;
	}

	return LootDefinition::s_definitions[ lootDefinitionKey ];
}
