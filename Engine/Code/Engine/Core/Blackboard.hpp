#pragma once
#include <map>
#include <string>
#include "Engine/Core/XmlUtilities.hpp"

class Blackboard
{

public:
	Blackboard();
	~Blackboard();

public:
	void PopulateFromXmlElementAttributes( const tinyxml2::XMLElement& element );
	void SetValue( const std::string& keyName, const std::string& newValue );
	bool GetValue( const std::string& keyName, bool defaultValue ) const;
	int GetValue( const std::string& keyName, int defaultValue ) const;
	float GetValue( const std::string& keyName, float defaultValue ) const;
	std::string GetValue( const std::string& keyName, std::string defaultValue ) const;
	std::string GetValue( const std::string& keyName, const char* defaultValue ) const;
	Rgba GetValue( const std::string& keyName, const Rgba& defaultValue ) const;
	Vector2 GetValue( const std::string& keyName, const Vector2& defaultValue ) const;
	Vector3 GetValue( const std::string& keyName, const Vector3& defaultValue ) const;
	Vector4 GetValue( const std::string& keyName, const Vector4& defaultValue ) const;
	IntVector2 GetValue( const std::string& keyName, const IntVector2& defaultValue ) const;
	IntVector3 GetValue( const std::string& keyName, const IntVector3& defaultValue ) const;
	AABB2 GetValue( const std::string& keyName, const AABB2& defaultValue ) const;
	AABB3 GetValue( const std::string& keyName, const AABB3& defaultValue ) const;
	FloatRange GetValue( const std::string& keyName, const FloatRange& defaultValue ) const;
	IntRange GetValue( const std::string& keyName, const IntRange& defaultValue ) const;

private:
	std::map<std::string, std::string> m_keyValuePairs;

};
