#include "Engine/Core/Blackboard.hpp"
#include "Engine/Core/XmlUtilities.hpp"

Blackboard::Blackboard()
{

}

Blackboard::~Blackboard()
{

}

void Blackboard::PopulateFromXmlElementAttributes( const tinyxml2::XMLElement& element )
{
	for ( const tinyxml2::XMLAttribute* attribute = element.FirstAttribute(); attribute != nullptr; attribute = attribute->Next() )
	{
		const char* attributeName = attribute->Name();
		m_keyValuePairs[ std::string( attributeName ) ] = ParseXmlAttribute( element, attributeName, "" );
	}

	if ( !element.NoChildren() )
	{
		for ( const tinyxml2::XMLElement* childElement = element.FirstChildElement(); childElement != nullptr; childElement = childElement->NextSiblingElement() )
		{
			PopulateFromXmlElementAttributes( *childElement );
		}
	}
}

void Blackboard::SetValue( const std::string& keyName, const std::string& newValue )
{
	m_keyValuePairs[ keyName ] = newValue;
}

bool Blackboard::GetValue( const std::string& keyName, bool defaultValue ) const
{
	if ( m_keyValuePairs.find( keyName ) != m_keyValuePairs.end() )
	{
		if ( m_keyValuePairs.at( keyName ) == "true" )
		{
			return true;
		}
		else if ( m_keyValuePairs.at( keyName ) == "false" )
		{
			return false;
		}
		else
		{
			return defaultValue;
		}
	}
	else
	{
		return defaultValue;
	}
}

int Blackboard::GetValue( const std::string& keyName, int defaultValue ) const
{
	if ( m_keyValuePairs.find( keyName ) != m_keyValuePairs.end() )
	{
		int stringAsInt = std::stoi( m_keyValuePairs.at( keyName ) );
		return stringAsInt;
	}
	else
	{
		return defaultValue;
	}
}

float Blackboard::GetValue( const std::string& keyName, float defaultValue ) const
{
	if ( m_keyValuePairs.find( keyName ) != m_keyValuePairs.end() )
	{
		float stringAsFloat = std::stof( m_keyValuePairs.at( keyName ) );
		return stringAsFloat;
	}
	else
	{
		return defaultValue;
	}
}

std::string Blackboard::GetValue( const std::string& keyName, std::string defaultValue ) const
{
	if ( m_keyValuePairs.find( keyName ) != m_keyValuePairs.end() )
	{
		return m_keyValuePairs.at( keyName );
	}
	else
	{
		return defaultValue;
	}
}

std::string Blackboard::GetValue( const std::string& keyName, const char* defaultValue ) const
{
	if ( m_keyValuePairs.find( keyName ) != m_keyValuePairs.end() )
	{
		return m_keyValuePairs.at( keyName );
	}
	else
	{
		return std::string( defaultValue );
	}
}

Rgba Blackboard::GetValue( const std::string& keyName, const Rgba& defaultValue ) const
{
	if ( m_keyValuePairs.find( keyName ) != m_keyValuePairs.end() )
	{
		Rgba rgbaValue = Rgba( defaultValue );
		rgbaValue.SetFromText( m_keyValuePairs.at( keyName ).c_str() );
		return rgbaValue;
	}
	else
	{
		return defaultValue;
	}
}

Vector2 Blackboard::GetValue( const std::string& keyName, const Vector2& defaultValue ) const
{
	if ( m_keyValuePairs.find( keyName ) != m_keyValuePairs.end() )
	{
		Vector2 vectorValue = Vector2( defaultValue );
		vectorValue.SetFromText( m_keyValuePairs.at( keyName ).c_str() );
		return vectorValue;
	}
	else
	{
		return defaultValue;
	}
}

Vector3 Blackboard::GetValue( const std::string& keyName, const Vector3& defaultValue ) const
{
	if ( m_keyValuePairs.find( keyName ) != m_keyValuePairs.end() )
	{
		Vector3 vectorValue = Vector3( defaultValue );
		vectorValue.SetFromText( m_keyValuePairs.at( keyName ).c_str() );
		return vectorValue;
	}
	else
	{
		return defaultValue;
	}
}

Vector4 Blackboard::GetValue( const std::string& keyName, const Vector4& defaultValue ) const
{
	if ( m_keyValuePairs.find( keyName ) != m_keyValuePairs.end() )
	{
		Vector4 vectorValue = Vector4( defaultValue );
		vectorValue.SetFromText( m_keyValuePairs.at( keyName ).c_str() );
		return vectorValue;
	}
	else
	{
		return defaultValue;
	}
}

IntVector2 Blackboard::GetValue( const std::string& keyName, const IntVector2& defaultValue ) const
{
	if ( m_keyValuePairs.find( keyName ) != m_keyValuePairs.end() )
	{
		IntVector2 intVectorValue = IntVector2( defaultValue );
		intVectorValue.SetFromText( m_keyValuePairs.at( keyName ).c_str() );
		return intVectorValue;
	}
	else
	{
		return defaultValue;
	}
}

IntVector3 Blackboard::GetValue( const std::string& keyName, const IntVector3& defaultValue ) const
{
	if ( m_keyValuePairs.find( keyName ) != m_keyValuePairs.end() )
	{
		IntVector3 intVectorValue = IntVector3( defaultValue );
		intVectorValue.SetFromText( m_keyValuePairs.at( keyName ).c_str() );
		return intVectorValue;
	}
	else
	{
		return defaultValue;
	}
}

AABB2 Blackboard::GetValue( const std::string& keyName, const AABB2& defaultValue ) const
{
	if ( m_keyValuePairs.find( keyName ) != m_keyValuePairs.end() )
	{
		AABB2 aabb2Value = AABB2( defaultValue );
		aabb2Value.SetFromText( m_keyValuePairs.at( keyName ).c_str() );
		return aabb2Value;
	}
	else
	{
		return defaultValue;
	}
}

AABB3 Blackboard::GetValue( const std::string& keyName, const AABB3& defaultValue ) const
{
	if ( m_keyValuePairs.find( keyName ) != m_keyValuePairs.end() )
	{
		AABB3 aabb3Value = AABB3( defaultValue );
		aabb3Value.SetFromText( m_keyValuePairs.at( keyName ).c_str() );
		return aabb3Value;
	}
	else
	{
		return defaultValue;
	}
}

FloatRange Blackboard::GetValue( const std::string& keyName, const FloatRange& defaultValue ) const
{
	if ( m_keyValuePairs.find( keyName ) != m_keyValuePairs.end() )
	{
		FloatRange floatRangeValue = FloatRange( defaultValue );
		floatRangeValue.SetFromText( m_keyValuePairs.at( keyName ).c_str() );
		return floatRangeValue;
	}
	else
	{
		return defaultValue;
	}
}

IntRange Blackboard::GetValue( const std::string& keyName, const IntRange& defaultValue ) const
{
	if ( m_keyValuePairs.find( keyName ) != m_keyValuePairs.end() )
	{
		IntRange intRangeValue = IntRange( defaultValue );
		intRangeValue.SetFromText( m_keyValuePairs.at( keyName ).c_str() );
		return intRangeValue;
	}
	else
	{
		return defaultValue;
	}
}
