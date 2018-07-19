#include "Engine/Core/XmlUtilities.hpp"

int ParseXmlAttribute( const tinyxml2::XMLElement& element, const char* attributeName, int defaultValue )
{
	return element.IntAttribute( attributeName, defaultValue );
}

char ParseXmlAttribute( const tinyxml2::XMLElement& element, const char* attributeName, char defaultValue )
{
	return static_cast<char>( element.IntAttribute( attributeName, static_cast<int>( defaultValue ) ) );		// TODO: Redo this right
}

bool ParseXmlAttribute( const tinyxml2::XMLElement& element, const char* attributeName, bool defaultValue )
{
	return element.BoolAttribute( attributeName, defaultValue );
}

float ParseXmlAttribute( const tinyxml2::XMLElement& element, const char* attributeName, float defaultValue )
{
	return element.FloatAttribute( attributeName, defaultValue );
}

Rgba ParseXmlAttribute( const tinyxml2::XMLElement& element, const char* attributeName, const Rgba& defaultValue )
{
	const char* rgbaText = element.Attribute( attributeName );
	if ( rgbaText == nullptr )
	{
		return defaultValue;
	}

	Rgba rgbaFromText = Rgba( defaultValue );
	rgbaFromText.SetFromText( rgbaText );
	return rgbaFromText;
}

Vector2 ParseXmlAttribute( const tinyxml2::XMLElement& element, const char* attributeName, const Vector2& defaultValue )
{
	const char* vectorText = element.Attribute( attributeName );
	if ( vectorText == nullptr )
	{
		return defaultValue;
	}

	Vector2 vectorFromText = Vector2( defaultValue );
	vectorFromText.SetFromText( vectorText );
	return vectorFromText;
}

IntRange ParseXmlAttribute( const tinyxml2::XMLElement& element, const char* attributeName, const IntRange& defaultValue )
{
	const char* intRangeText = element.Attribute( attributeName );
	if ( intRangeText == nullptr )
	{
		return defaultValue;
	}

	IntRange intRangeFromText = IntRange( defaultValue );
	intRangeFromText.SetFromText( intRangeText );
	return intRangeFromText;
}

FloatRange ParseXmlAttribute( const tinyxml2::XMLElement& element, const char* attributeName, const FloatRange& defaultValue )
{
	const char* floatRangeText = element.Attribute( attributeName );
	if ( floatRangeText == nullptr )
	{
		return defaultValue;
	}

	FloatRange floatRangeFromText = FloatRange( defaultValue );
	floatRangeFromText.SetFromText( floatRangeText );
	return floatRangeFromText;
}

IntVector2 ParseXmlAttribute( const tinyxml2::XMLElement& element, const char* attributeName, const IntVector2& defaultValue )
{
	const char* intVectorText = element.Attribute( attributeName );
	if ( intVectorText == nullptr )
	{
		return defaultValue;
	}

	IntVector2 intVectorFromText = IntVector2( defaultValue );
	intVectorFromText.SetFromText( intVectorText );
	return intVectorFromText;
}

AABB2 ParseXmlAttribute( const tinyxml2::XMLElement& element, const char* attributeName, const AABB2& defaultValue )
{
	const char* aabbText = element.Attribute( attributeName );
	if ( aabbText == nullptr )
	{
		return defaultValue;
	}

	AABB2 aabbFromText = AABB2( defaultValue );
	aabbFromText.SetFromText( aabbText );
	return aabbFromText;
}

std::string ParseXmlAttribute( const tinyxml2::XMLElement& element, const char* attributeName, const std::string& defaultValue )
{
	const char* value = element.Attribute( attributeName );

	if ( value == nullptr )
	{
		return defaultValue;
	}
	else
	{
		return std::string( value );
	}
}

std::string ParseXmlAttribute( const tinyxml2::XMLElement& element, const char* attributeName, const char* defaultValue = nullptr )
{
	const char* value = element.Attribute( attributeName );

	if ( value == nullptr )
	{
		return std::string( defaultValue );
	}
	else
	{
		return std::string( value );
	}
}

std::vector< int > ParseXmlAttribute( const tinyxml2::XMLElement& element, const char* attributeName, std::vector< int > defaultValue )
{
	std::vector< int > vectorOfInts = std::vector< int >( defaultValue );

	const char* value = element.Attribute( attributeName );
	if ( value != nullptr )
	{
		SetFromText( vectorOfInts, value );		// TODO: Add this method to IntRange file
	}

	return vectorOfInts;
}
