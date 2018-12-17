#include "Game/GameCommon.hpp"
#include "Game/Entities/ActorDefinition.hpp"
#include "Game/Entities/ItemDefinition.hpp"
#include "Game/World/TileDefinition.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/XmlUtilities.hpp"
#include "Engine/Math/IntVector2.hpp"

std::map< std::string, TileDefinition* > TileDefinition::s_definitions;

TileDefinition::TileDefinition( const tinyxml2::XMLElement& tileDefinitionElement )
{
	m_overlaySpriteTint = m_overlaySpriteTint.GetWithAlpha( 0.0f );		// Set overlay sprite tint to transparent initially, in case it's not present in the xml
	PopulateDataFromXml( tileDefinitionElement );
}

std::string TileDefinition::GetName() const
{
	return m_name;
}

AABB2 TileDefinition::GetBaseSpriteUvs() const
{
	return m_baseSpriteUvs;
}

AABB2 TileDefinition::GetOverlaySpriteUvs() const
{
	return m_overlaySpriteUvs;
}

Rgba TileDefinition::GetBaseSpriteTint() const
{
	return m_baseSpriteTint;
}

Rgba TileDefinition::GetOverlaySpriteTint() const
{
	return m_overlaySpriteTint;
}

Rgba TileDefinition::GetMapTexelColor() const
{
	return m_mapTexelColor;
}

bool TileDefinition::AllowsSight() const
{
	return m_allowsSight;
}

bool TileDefinition::AllowsWalking() const
{
	return m_allowsWalking;
}

bool TileDefinition::AllowsFlying() const
{
	return m_allowsFlying;
}

bool TileDefinition::AllowsSwimming() const
{
	return m_allowsSwimming;
}

float TileDefinition::GetFrictionMultiplier() const
{
	return m_frictionMultiplier;
}

bool TileDefinition::PermitsActorOfType( const ActorDefinition& actorDefinition ) const
{
	return ( ( m_allowsWalking && actorDefinition.CanWalk() ) || ( m_allowsSwimming && actorDefinition.CanSwim() ) || ( m_allowsFlying && actorDefinition.CanFly() ) );
}

bool TileDefinition::PermitsItemOfType( const ItemDefinition& itemDefinition ) const
{
	return ( ( m_allowsWalking && itemDefinition.CanWalk() ) || ( m_allowsSwimming && itemDefinition.CanSwim() ) || ( m_allowsFlying && itemDefinition.CanFly() ) );
}

void TileDefinition::PopulateDataFromXml( const tinyxml2::XMLElement& tileDefinitionElement )
{
	m_name = ParseXmlAttribute( tileDefinitionElement, NAME_XML_ATTRIBUTE_NAME, m_name );

	IntVector2 baseSpriteCoordinatesAsIntVector2 = ParseXmlAttribute( tileDefinitionElement, BASE_SPRITE_COORDS_XML_ATTRIBUTE_NAME, IntVector2::ZERO );
	m_baseSpriteUvs = g_tileSpriteSheet->GetTextureCoordinatesForSpriteCoordinates( baseSpriteCoordinatesAsIntVector2 );
	IntVector2 overlaySpriteCoordinatesAsIntVector2 = ParseXmlAttribute( tileDefinitionElement, OVERLAY_SPRITE_COORDS_XML_ATTRIBUTE_NAME, IntVector2::ZERO );
	m_overlaySpriteUvs = g_tileSpriteSheet->GetTextureCoordinatesForSpriteCoordinates( overlaySpriteCoordinatesAsIntVector2 );

	m_baseSpriteTint = ParseXmlAttribute( tileDefinitionElement, BASE_SPRITE_TINT_XML_ATTRIBUTE_NAME, m_baseSpriteTint );
	m_overlaySpriteTint = ParseXmlAttribute( tileDefinitionElement, OVERLAY_SPRITE_TINT_XML_ATTRIBUTE_NAME, m_overlaySpriteTint );
	m_mapTexelColor = ParseXmlAttribute( tileDefinitionElement, MAP_TEXEL_COLOR_ATTRIBUTE_NAME, m_mapTexelColor );
	m_allowsSight = ParseXmlAttribute( tileDefinitionElement, ALLOWS_SIGHT_XML_ATTRIBUTE_NAME, m_allowsSight );
	m_allowsWalking = ParseXmlAttribute( tileDefinitionElement, ALLOWS_WALKING_XML_ATTRIBUTE_NAME, m_allowsWalking );
	m_allowsFlying = ParseXmlAttribute( tileDefinitionElement, ALLOWS_FLYING_XML_ATTRIBUTE_NAME, m_allowsFlying );
	m_allowsSwimming = ParseXmlAttribute( tileDefinitionElement, ALLOWS_SWIMMING_XML_ATTRIBUTE_NAME, m_allowsSwimming );
	m_frictionMultiplier = ParseXmlAttribute( tileDefinitionElement, FRICTION_MULTIPLIER_XML_ATTRIBUTE_NAME, m_frictionMultiplier );

	TileDefinition::s_definitions[ m_name ] = this;
}

TileDefinition* ParseXmlAttribute( const tinyxml2::XMLElement& element, const char* attributeName, TileDefinition* defaultValue )
{
	const char* tileDefinitionText = element.Attribute( attributeName );
	if ( tileDefinitionText == nullptr )
	{
		return defaultValue;
	}

	std::string tileDefinitionKey = std::string( tileDefinitionText );
	if ( TileDefinition::s_definitions.find( tileDefinitionKey ) == TileDefinition::s_definitions.end() )
	{
		return defaultValue;
	}

	return TileDefinition::s_definitions[ tileDefinitionKey ];
}

bool TileDefinition::IsMapTexelColorValid( const Rgba& color )
{
	bool isColorValid = false;

	for ( std::map< std::string, TileDefinition* >::iterator tileDefIterator = s_definitions.begin(); tileDefIterator != s_definitions.end(); tileDefIterator++ )
	{
		if ( Rgba::AreColorsEqualExceptAlpha( color, tileDefIterator->second->m_mapTexelColor ) )
		{
			isColorValid = true;
			break;
		}
	}

	return isColorValid;
}

TileDefinition* TileDefinition::GetTileDefinitionForTexelColor( const Rgba& color )
{
	ASSERT_OR_DIE( TileDefinition::IsMapTexelColorValid( color ), "TileDefinition::GetTileDefinitionForTexelColor - Provided texel does not have not a valid tile color, as specified in Tiles.xml. Aborting...");

	for ( std::map< std::string, TileDefinition* >::iterator tileDefIterator = s_definitions.begin(); tileDefIterator != s_definitions.end(); tileDefIterator++ )
	{
		if ( Rgba::AreColorsEqualExceptAlpha( color, tileDefIterator->second->m_mapTexelColor ) )
		{
			return tileDefIterator->second;
		}
	}

	return nullptr;
}
