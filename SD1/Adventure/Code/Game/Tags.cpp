#include "Game/Tags.hpp"

Tags::Tags()
{

}

Tags::Tags( const std::string& commaSeparatedTagNames )
{
	SetOrRemoveTags( commaSeparatedTagNames );
}

Tags::Tags( const Tags& copyTags )
{
	m_tags = copyTags.m_tags;
}

void Tags::SetOrRemoveTags( const std::string& commaSeparatedTagNames )
{
	TokenizedString tagsTokenized = TokenizedString( commaSeparatedTagNames, "," );

	for ( std::string tag : tagsTokenized.GetTokens() )
	{
		if ( tag[ 0 ] != '!' && !HasTag( tag ) )
		{
			SetTag( tag );
		}
		else if ( tag[ 0 ] == '!' && HasTag( tag.substr( 1 ) ) )
		{
			RemoveTag( tag.substr( 1 ) );
		}
	}
}

bool Tags::HasTags( const std::string& commaSeparatedTagNames ) const
{
	TokenizedString tagsTokenized = TokenizedString( commaSeparatedTagNames, "," );

	for ( std::string tag : tagsTokenized.GetTokens() )
	{
		if ( tag[ 0 ] != '!' && !HasTag( tag ) )
		{
			return false;
		}
		else if ( tag[ 0 ] == '!' && HasTag( tag.substr( 1 ) ) )
		{
			return false;
		}
	}

	return true;
}

bool Tags::HasTags( const Tags& tags ) const
{
	for ( std::string tag : tags.m_tags )
	{
		if ( tag[ 0 ] != '!' && !HasTag( tag ) )
		{
			return false;
		}
		else if ( tag[ 0 ] == '!' && HasTag( tag.substr( 1 ) ) )
		{
			return false;
		}
	}

	return true;
}

void Tags::SetTag( const std::string& tagName )
{
	if ( !HasTag( tagName ) )
	{
		m_tags.push_back( tagName );
	}
}

void Tags::RemoveTag( const std::string& tagName )
{
	std::vector< std::string >::iterator tagIterator = std::find( m_tags.begin(), m_tags.end(), tagName );
	if ( tagIterator != m_tags.end() )
	{
		m_tags.erase( tagIterator );
	}
}

bool Tags::HasTag( const std::string& tagName ) const
{
	return ( std::find( m_tags.begin(), m_tags.end(), tagName ) != m_tags.end() );
}

Tags ParseXmlAttribute( const tinyxml2::XMLElement& element, const char* attributeName, Tags defaultValue )
{
	Tags tagsToReturn = defaultValue;

	const char* tagText = element.Attribute( attributeName );
	if ( tagText != nullptr )
	{
		tagsToReturn = Tags( std::string( tagText ) );
	}

	return tagsToReturn;
}
