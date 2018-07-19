#include "Game/GameCommon.hpp"
#include "Game/Entities/Entity.hpp"

float Entity::GetOrientationDegrees() const
{
	return m_orientation;
}

int Entity::GetHealth() const
{
	return m_health;
}

Vector2 Entity::GetLocation() const
{
	return m_location;
}

Vector2 Entity::GetVelocity() const
{
	return m_velocity;
}

bool Entity::IsDead() const
{
	return m_isDead;
}

void Entity::DoDamage()
{
	if ( m_health > 0 )
	{
		m_health--;
	}
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
	m_location = m_location + ( ( m_speed * m_velocity ) *  deltaSeconds );
}

void Entity::MarkForDeathIfHealthIsZero()
{
	if ( m_health == 0 )
	{
		m_isDead = true;
	}
}

void Entity::PopulateDeveloperModeCirclesVertices( float cosmeticRadius, float physicalRadius )
{
	float theta = 0.0f;
	float deltaTheta = 360.0f / static_cast<float>( CIRCLE_NUM_SIDES );

	for ( int i = 0; i < CIRCLE_NUM_SIDES; i++ )
	{
		float cosTheta = CosDegrees( theta );
		float sinTheta = SinDegrees( theta );

		// TODO: Revise the radii based on requirements
		m_cosmeticVertices[ i ] = Vector2( ( cosmeticRadius * cosTheta ), ( cosmeticRadius * sinTheta ) );
		m_physicalVertices[ i ] = Vector2( ( physicalRadius * cosTheta ), ( physicalRadius * sinTheta ) );

		theta += deltaTheta;
	}
}

void Entity::RenderDeveloperMode() const
{
	RenderPhysicalCircle();
	RenderCosmeticCircle();
}

void Entity::RenderPhysicalCircle() const
{
	g_renderer->DrawPolygon( Vector2( 0.0f, 0.0f ), 0.0f, m_physicalVertices, CIRCLE_NUM_SIDES, RPolygonType::BROKEN_LINES, Rgba( 0, RGBA_MAX, RGBA_MAX, RGBA_MAX ) );
	g_renderer->ChangeRenderColor( Rgba( RGBA_MAX, RGBA_MAX, RGBA_MAX, RGBA_MAX ) );
}

void Entity::RenderCosmeticCircle() const
{
	g_renderer->DrawPolygon( Vector2( 0.0f, 0.0f ), 0.0f, m_cosmeticVertices, CIRCLE_NUM_SIDES, RPolygonType::BROKEN_LINES, Rgba( RGBA_MAX, 0, RGBA_MAX, RGBA_MAX ) );
	g_renderer->ChangeRenderColor( Rgba( RGBA_MAX, RGBA_MAX, RGBA_MAX, RGBA_MAX ) );
}