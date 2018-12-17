#pragma once

#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/XmlUtilities.hpp"
#include "ThirdParty/tinyXML2/tinyxml2.h"
#include <string>
#include <vector>

class Tags
{

public:
	Tags();
	Tags( const std::string& commaSeparatedTagNames );
	Tags( const Tags& copyTags );

	void SetOrRemoveTags( const std::string& commaSeparatedTagNames );
	bool HasTags( const std::string& commaSeparatedTagNames ) const;
	bool HasTags( const Tags& tags ) const;

protected:
	void SetTag( const std::string& tagName );
	void RemoveTag( const std::string& tagName );
	bool HasTag( const std::string& tagName ) const;

protected:
	std::vector< std::string > m_tags;

};

Tags ParseXmlAttribute( const tinyxml2::XMLElement& element, const char* attributeName, Tags defaultValue );
