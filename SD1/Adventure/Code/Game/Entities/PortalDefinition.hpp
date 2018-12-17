#pragma once

#include "Game/Entities/EntityDefinition.hpp"
#include "ThirdParty/tinyXML2/tinyxml2.h"
#include <map>
#include <string>

class PortalDefinition: public EntityDefinition
{

	friend class TheGame;

public:
	~PortalDefinition();

private:
	explicit PortalDefinition( const tinyxml2::XMLElement& portalDefinitionElement );

private:
	void PopulateDataFromXml( const tinyxml2::XMLElement& portalDefinitionElement ) override;

public:
	static std::map< std::string, PortalDefinition* > s_definitions;

};

PortalDefinition* ParseXmlAttribute( const tinyxml2::XMLElement& element, const char* attributeName, PortalDefinition* defaultValue );
