#include "Game/GameCommon.hpp"
#include "Game/Entities/Entity.hpp"

Entity::Entity()
{

}

Entity::~Entity()
{

}

float Entity::GetOrientationDegrees() const
{
	return m_orientation;
}

Vector2 Entity::GetLocation() const
{
	return m_location;
}

Vector2 Entity::GetVelocity() const
{
	return m_velocity;
}

void Entity::Update( float deltaSeconds )
{
	Translate( deltaSeconds );
	Rotate( deltaSeconds );
}

void Entity::Render() const
{

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
