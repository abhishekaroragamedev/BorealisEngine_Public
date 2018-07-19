#pragma once

#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/IntRange.hpp"
#include "Engine/Math/IntVector2.hpp"
#include "Engine/Math/IntVector3.hpp"
#include "Engine/Math/Vector2.hpp"
#include "ThirdParty/tinyXML2/tinyxml2.h"
#include <string>
#include <vector>

int ParseXmlAttribute( const tinyxml2::XMLElement& element, const char* attributeName, int defaultValue );
unsigned int ParseXmlAttribute( const tinyxml2::XMLElement& element, const char* attributeName, unsigned int defaultValue );
char ParseXmlAttribute( const tinyxml2::XMLElement& element, const char* attributeName, char defaultValue );
bool ParseXmlAttribute( const tinyxml2::XMLElement& element, const char* attributeName, bool defaultValue );
float ParseXmlAttribute( const tinyxml2::XMLElement& element, const char* attributeName, float defaultValue );
Rgba ParseXmlAttribute( const tinyxml2::XMLElement& element, const char* attributeName, const Rgba& defaultValue );
Vector2 ParseXmlAttribute( const tinyxml2::XMLElement& element, const char* attributeName, const Vector2& defaultValue );
Vector3 ParseXmlAttribute( const tinyxml2::XMLElement& element, const char* attributeName, const Vector3& defaultValue );
Vector4 ParseXmlAttribute( const tinyxml2::XMLElement& element, const char* attributeName, const Vector4& defaultValue );
IntRange ParseXmlAttribute( const tinyxml2::XMLElement& element, const char* attributeName, const IntRange& defaultValue );
FloatRange ParseXmlAttribute( const tinyxml2::XMLElement& element, const char* attributeName, const FloatRange& defaultValue );
IntVector2 ParseXmlAttribute( const tinyxml2::XMLElement& element, const char* attributeName, const IntVector2& defaultValue );
IntVector3 ParseXmlAttribute( const tinyxml2::XMLElement& element, const char* attributeName, const IntVector3& defaultValue );
AABB2 ParseXmlAttribute( const tinyxml2::XMLElement& element, const char* attributeName, const AABB2& defaultValue );
AABB3 ParseXmlAttribute( const tinyxml2::XMLElement& element, const char* attributeName, const AABB3& defaultValue );
std::string ParseXmlAttribute( const tinyxml2::XMLElement& element, const char* attributeName, const std::string& defaultValue );
std::string ParseXmlAttribute( const tinyxml2::XMLElement& element, const char* attributeName, const char* defaultValue );
std::vector< int > ParseXmlAttribute( const tinyxml2::XMLElement& element, const char* attributeName, std::vector< int > defaultValue );
