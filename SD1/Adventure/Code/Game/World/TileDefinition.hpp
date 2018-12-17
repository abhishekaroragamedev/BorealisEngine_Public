#pragma once

#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/AABB2.hpp"
#include "ThirdParty/tinyXML2/tinyxml2.h"
#include <map>
#include <string>

class ActorDefinition;
class ItemDefinition;

class TileDefinition
{

	friend class TheGame;

private:
	explicit TileDefinition( const tinyxml2::XMLElement& tileDefinitionElement );

public:
	std::string GetName() const;
	AABB2 GetBaseSpriteUvs() const;
	AABB2 GetOverlaySpriteUvs() const;
	Rgba GetBaseSpriteTint() const;
	Rgba GetOverlaySpriteTint() const;
	Rgba GetMapTexelColor() const;
	bool AllowsSight() const;
	bool AllowsWalking() const;
	bool AllowsFlying() const;
	bool AllowsSwimming() const;
	bool PermitsActorOfType( const ActorDefinition& actorDefinition ) const;
	bool PermitsItemOfType( const ItemDefinition& itemDefinition ) const;

	float GetFrictionMultiplier() const;

private:
	void PopulateDataFromXml( const tinyxml2::XMLElement& tileDefinitionElement );

private:
	static constexpr char NAME_XML_ATTRIBUTE_NAME[] = "name";
	static constexpr char BASE_SPRITE_COORDS_XML_ATTRIBUTE_NAME[] = "baseSpriteCoords";
	static constexpr char OVERLAY_SPRITE_COORDS_XML_ATTRIBUTE_NAME[] = "overlaySpriteCoords";
	static constexpr char BASE_SPRITE_TINT_XML_ATTRIBUTE_NAME[] = "baseSpriteTint";
	static constexpr char OVERLAY_SPRITE_TINT_XML_ATTRIBUTE_NAME[] = "overlaySpriteTint";
	static constexpr char MAP_TEXEL_COLOR_ATTRIBUTE_NAME[] = "mapTexelColor";
	static constexpr char ALLOWS_SIGHT_XML_ATTRIBUTE_NAME[] = "allowsSight";
	static constexpr char ALLOWS_WALKING_XML_ATTRIBUTE_NAME[] = "allowsWalking";
	static constexpr char ALLOWS_FLYING_XML_ATTRIBUTE_NAME[] = "allowsFlying";
	static constexpr char ALLOWS_SWIMMING_XML_ATTRIBUTE_NAME[] = "allowsSwimming";
	static constexpr char FRICTION_MULTIPLIER_XML_ATTRIBUTE_NAME[] = "frictionMultiplier";

public:
	static bool IsMapTexelColorValid( const Rgba& color );
	static TileDefinition* GetTileDefinitionForTexelColor( const Rgba& color );
	static std::map< std::string, TileDefinition* >	s_definitions;

private:
	std::string m_name = "";
	AABB2 m_baseSpriteUvs;
	AABB2 m_overlaySpriteUvs;
	Rgba m_baseSpriteTint = Rgba::WHITE;
	Rgba m_overlaySpriteTint = Rgba::WHITE;
	Rgba m_mapTexelColor = Rgba::WHITE;
	bool m_allowsSight = true;		// Default values are based on how frequent these values should ideally occur in the tile definitions of this game
	bool m_allowsWalking = true;
	bool m_allowsFlying = true;
	bool m_allowsSwimming = false;
	float m_frictionMultiplier = 1.0f;

};

TileDefinition* ParseXmlAttribute( const tinyxml2::XMLElement& element, const char* attributeName, TileDefinition* defaultValue );
