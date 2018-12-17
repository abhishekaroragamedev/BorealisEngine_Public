#include "Game/Entities/PortalDefinition.hpp"
#include "Engine/Core/XmlUtilities.hpp"

std::map< std::string, PortalDefinition* > PortalDefinition::s_definitions;

PortalDefinition::PortalDefinition( const tinyxml2::XMLElement& portalDefinitionElement ) : EntityDefinition( portalDefinitionElement )
{
	PopulateDataFromXml( portalDefinitionElement );
}

PortalDefinition::~PortalDefinition()
{

}

void PortalDefinition::PopulateDataFromXml( const tinyxml2::XMLElement& portalDefinitionElement )
{
	EntityDefinition::PopulateDataFromXml( portalDefinitionElement );
	s_definitions[ m_name ] = this;
}

PortalDefinition* ParseXmlAttribute( const tinyxml2::XMLElement& element, const char* attributeName, PortalDefinition* defaultValue )
{
	const char* portalDefinitionText = element.Attribute( attributeName );
	if ( portalDefinitionText == nullptr )
	{
		return defaultValue;
	}

	std::string portalDefinitionKey = std::string( portalDefinitionText );
	if ( PortalDefinition::s_definitions.find( portalDefinitionKey ) == PortalDefinition::s_definitions.end() )
	{
		return defaultValue;
	}

	return PortalDefinition::s_definitions[ portalDefinitionKey ];
}
