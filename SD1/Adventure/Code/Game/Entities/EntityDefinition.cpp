#include "Game/GameCommon.hpp"
#include "Game/Entities/EntityDefinition.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/XmlUtilities.hpp"

EntityDefinition::EntityDefinition( const tinyxml2::XMLElement& entityDefinitionElement )
{
	PopulateDataFromXml( entityDefinitionElement );
	PopulatePhysicalDiscVertices();
}

EntityDefinition::~EntityDefinition()
{
	delete m_spriteAnimSetDefinition;
	m_spriteAnimSetDefinition = nullptr;
}

std::string EntityDefinition::GetName() const
{
	return m_name;
}

bool EntityDefinition::CanSee() const
{
	return m_canSee;
}

bool EntityDefinition::CanWalk() const
{
	return m_canWalk;
}

bool EntityDefinition::CanFly() const
{
	return m_canFly;
}

bool EntityDefinition::CanSwim() const
{
	return m_canSwim;
}

float EntityDefinition::GetPhysicsRadius() const
{
	return m_physicsRadius;
}

float EntityDefinition::GetFrictionPerSecond() const
{
	return m_frictionPerSecond;
}

AABB2 EntityDefinition::GetDrawBounds() const
{
	return m_drawBounds;
}

SpriteAnimationSetDefinition* EntityDefinition::GetSpriteAnimationSetDefinition() const
{
	return m_spriteAnimSetDefinition;
}

Tags EntityDefinition::GetDefaultTags() const
{
	return m_defaultTags;
}

const std::map< TileDefinition*, float > EntityDefinition::GetTileDefinitionToMovementPenaltyMap() const
{
	return m_movementPenaltiesByTileDefinition;
}

void EntityDefinition::PopulateDataFromXml( const tinyxml2::XMLElement& entityDefinitionElement )
{
	m_name = ParseXmlAttribute( entityDefinitionElement, NAME_XML_ATTRIBUTE_NAME, m_name );
	m_defaultTags = ParseXmlAttribute( entityDefinitionElement, TAGS_XML_ATTRIBUTE_NAME, m_defaultTags );

	if ( !entityDefinitionElement.NoChildren() )
	{
		for ( const tinyxml2::XMLElement* entityDefinitionSubElement = entityDefinitionElement.FirstChildElement(); entityDefinitionSubElement != nullptr; entityDefinitionSubElement = entityDefinitionSubElement->NextSiblingElement() )
		{
			if ( std::string( entityDefinitionSubElement->Name() ) == std::string( PHYSICS_XML_NODE_NAME ) )
			{
				m_physicsRadius = ParseXmlAttribute( *entityDefinitionSubElement, PHYSICS_RADIUS_XML_ATTRIBUTE_NAME, TILE_SIDE_LENGTH_WORLD_UNITS );
				m_drawBounds = ParseXmlAttribute( *entityDefinitionSubElement, DRAW_BOUNDS_XML_ATTRIBUTE_NAME, m_drawBounds );
				m_frictionPerSecond = ParseXmlAttribute( *entityDefinitionSubElement, FRICTION_XML_ATTRIBUTE_NAME, m_frictionPerSecond );
			}
			else if ( std::string( entityDefinitionSubElement->Name() ) == std::string( SPRITE_ANIM_SET_XML_NODE_NAME ) )
			{
				m_spriteAnimSetDefinition = new SpriteAnimationSetDefinition( *entityDefinitionSubElement, *g_renderer );
			}
			else if ( std::string( entityDefinitionSubElement->Name() ) == std::string( MOVEMENT_XML_NODE_NAME ) )
			{
				if ( !entityDefinitionSubElement->NoChildren() )
				{
					for ( const tinyxml2::XMLElement* entityDefMovementSubElement = entityDefinitionSubElement->FirstChildElement(); entityDefMovementSubElement != nullptr; entityDefMovementSubElement = entityDefMovementSubElement->NextSiblingElement() )
					{
						if ( std::string( entityDefMovementSubElement->Name() ) == std::string( SIGHT_XML_NODE_NAME ) )
						{
							m_canSee = true;
						}
						else if ( std::string( entityDefMovementSubElement->Name() ) == std::string( WALKING_XML_NODE_NAME ) )
						{
							m_canWalk = true;
						}
						else if ( std::string( entityDefMovementSubElement->Name() ) == std::string( FLYING_XML_NODE_NAME ) )
						{
							m_canFly = true;
						}
						else if ( std::string( entityDefMovementSubElement->Name() ) == std::string( SWIMMING_XML_NODE_NAME ) )
						{
							m_canSwim = true;
						}
						else if ( std::string( entityDefMovementSubElement->Name() ) == std::string( TILE_XML_NODE_NAME ) )
						{
							std::string tileDefinitionName = ParseXmlAttribute( *entityDefMovementSubElement, TILE_TYPE_XML_ATTRIBUTE_NAME, "" );
							if ( tileDefinitionName != "" )
							{
								ASSERT_OR_DIE( TileDefinition::s_definitions.find( tileDefinitionName ) != TileDefinition::s_definitions.end(), "EntityDefinition::PopulateDataFromXml - Provided tile definition name in MovementInfo tile penalties section is invalid. Aborting..." );
								float tilePenalty = ParseXmlAttribute( *entityDefMovementSubElement, TILE_PENALTY_XML_ATTRIBUTE_NAME, 0.0f );
								m_movementPenaltiesByTileDefinition[ TileDefinition::s_definitions[ tileDefinitionName ] ] = tilePenalty;
							}
						}
					}
				}
			}
		}
	}
}

void EntityDefinition::PopulatePhysicalDiscVertices()
{
	float theta = 0.0f;
	float thetaIncrement = 360.0f / static_cast< float >( CIRCLE_NUM_SIDES );

	for ( int circleVertexIterator = 0; circleVertexIterator < CIRCLE_NUM_SIDES; circleVertexIterator++ )
	{
		m_physicalDiscVertices[ circleVertexIterator ] = m_physicsRadius * Vector2( CosDegrees( theta ), SinDegrees( theta ) );
		theta += thetaIncrement;
	}
}

void EntityDefinition::RenderDeveloperModeVertices( float renderAlpha ) const
{
	g_renderer->DrawPolygon( Vector2::ZERO, 0.0f, m_physicalDiscVertices, CIRCLE_NUM_SIDES, RPolygonType::BROKEN_LINES, Rgba::MAGENTA.GetWithAlpha( renderAlpha ) );
}
