#include "Game/GameCommon.hpp"
#include "Game/Entities/Entity.hpp"
#include "Game/Entities/Item.hpp"
#include "Game/World/Map.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/XmlUtilities.hpp"
#include "Engine/Math/MathUtils.hpp"

Entity::Entity( const std::string instanceName, const Vector2& position, EntityDefinition* entityDefinition, const Map& map  )
	:	m_instanceName( instanceName ),
		m_position( position ),
		m_entityDefinition( entityDefinition ),
		m_map( const_cast< Map* >( &map ) )
{
	m_spriteAnimationSet = new SpriteAnimationSet( *m_entityDefinition->GetSpriteAnimationSetDefinition() );
	m_tags = Tags( m_entityDefinition->GetDefaultTags() );
}

Entity::~Entity()
{
	delete m_spriteAnimationSet;
	m_spriteAnimationSet = nullptr;

	m_map = nullptr;
}

void Entity::SetPosition( const Vector2& newPosition )
{
	m_position = newPosition;
}

std::string Entity::GetInstanceName() const
{
	return m_instanceName;
}

float Entity::GetOrientationDegrees() const
{
	return m_orientation;
}

Vector2 Entity::GetLocation() const
{
	return m_position;
}

Vector2 Entity::GetVelocity() const
{
	return m_velocity;
}

EntityDefinition* Entity::GetEntityDefinition() const
{
	return m_entityDefinition;
}

Map* Entity::GetMap() const
{
	return m_map;
}

Tags Entity::GetTagSet() const
{
	return m_tags;
}

std::vector< Item* > Entity::GetInventory() const
{
	return m_inventory;
}

void Entity::Update( float deltaSeconds )
{
	m_spriteAnimationSet->GetCurrentSpriteAnimation()->Update( deltaSeconds );
	Translate( deltaSeconds );
	Rotate( deltaSeconds );
}

void Entity::Render( float renderAlpha, bool developerModeEnabled ) const
{
	float angleToRotate =  m_orientation - m_spriteAnimationSet->GetCurrentSpriteAnimation()->GetDefinition()->GetOrientationOffset();

	g_renderer->PushMatrix();
	g_renderer->Translate( m_position.x, m_position.y, 0.0f );
	g_renderer->Rotate( angleToRotate, 0.0f, 0.0f, 1.0f );

	g_renderer->DrawTexturedAABB( m_entityDefinition->GetDrawBounds(), *m_spriteAnimationSet->GetCurrentSpriteAnimation()->GetDefinition()->GetTexture(), m_spriteAnimationSet->GetCurrentSpriteAnimation()->GetCurrentTexCoords().mins, m_spriteAnimationSet->GetCurrentSpriteAnimation()->GetCurrentTexCoords().maxs, Rgba::WHITE.GetWithAlpha( renderAlpha ) );
	if ( developerModeEnabled )
	{
		m_entityDefinition->RenderDeveloperModeVertices( renderAlpha );
	}

	g_renderer->PopMatrix();
}

void Entity::Rotate( float deltaSeconds )
{
	m_orientation = m_orientation + ( m_rotationSpeed * deltaSeconds );

	while (m_orientation > 360.0f)
	{
		m_orientation -= 360.0f;
	}
}

void Entity::Translate( float deltaSeconds )
{
	m_position += m_velocity *  deltaSeconds;
}

void Entity::ApplyCorrectiveTranslation( const Vector2& translation )
{
	m_position += translation;
}

StatID Entity::GetStatIDForName( const std::string& statIDName )
{
	if ( statIDName == "Health" )
	{
		return StatID::STAT_HEALTH;
	}
	else if ( statIDName == "Strength" )
	{
		return StatID::STAT_STRENGTH;
	}
	else if ( statIDName == "Resilience" )
	{
		return StatID::STAT_RESILIENCE;
	}
	else if ( statIDName == "Agility" )
	{
		return StatID::STAT_AGILITY;
	}
	else if ( statIDName == "Dexterity" )
	{
		return StatID::STAT_DEXTERITY;
	}
	else if ( statIDName == "Aim" )
	{
		return StatID::STAT_AIM;
	}
	else if ( statIDName == "Accuracy" )
	{
		return StatID::STAT_ACCURACY;
	}
	else if ( statIDName == "Evasiveness" )
	{
		return StatID::STAT_EVASIVENESS;
	}
	else if ( statIDName == "Poison" )
	{
		return StatID::STAT_POISON_USAGE;
	}
	else if ( statIDName == "Fire" )
	{
		return StatID::STAT_FIRE_USAGE;
	}
	else if ( statIDName == "Ice" )
	{
		return StatID::STAT_ICE_USAGE;
	}
	else if ( statIDName == "Electricity" )
	{
		return StatID::STAT_ELECTRICITY_USAGE;
	}
	else
	{
		return StatID::INVALID_STAT_ID;
	}
}

std::string Entity::GetNameForStatID( StatID statID )
{
	switch( statID )
	{
		case StatID::STAT_HEALTH:				return "Health";
		case StatID::STAT_STRENGTH:				return "Strength";
		case StatID::STAT_RESILIENCE:			return "Resilience";
		case StatID::STAT_AGILITY:				return "Agility";
		case StatID::STAT_DEXTERITY:			return "Dexterity";
		case StatID::STAT_AIM:					return "Aim";
		case StatID::STAT_ACCURACY:				return "Accuracy";
		case StatID::STAT_EVASIVENESS:			return "Evasiveness";
		case StatID::STAT_POISON_USAGE:			return "Poison";
		case StatID::STAT_FIRE_USAGE:			return "Fire";
		case StatID::STAT_ICE_USAGE:			return "Ice";
		case StatID::STAT_ELECTRICITY_USAGE:	return "Electricity";
		default:	return "";
	}
}
