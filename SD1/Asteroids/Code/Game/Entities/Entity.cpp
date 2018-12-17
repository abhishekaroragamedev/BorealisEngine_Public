#include "Game/GameCommon.hpp"
#include "Game/Entities/Entity.hpp"

Entity::Entity()
{
}

Entity::~Entity()
{

}

void Entity::Update( float deltaSeconds )
{
	Translate( deltaSeconds );
	Rotate( deltaSeconds );
}

void Entity::Render( bool developerModeEnabled ) const
{
	if ( developerModeEnabled )
	{
		RenderDeveloperMode();
	}
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

Disc2 Entity::GetCosmeticDisc2() const
{
	return m_cosmeticDisc2;
}

Disc2 Entity::GetPhysicalDisc2() const
{
	return m_physicalDisc2;
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
	WrapAroundScreenIfBeyondBounds();

	m_cosmeticDisc2.center = m_location;		// Update collider discs
	m_physicalDisc2.center = m_location;
}

void Entity::PopulateDeveloperModeCirclesVertices()
{
	float theta = 0.0f;
	float deltaTheta = 360.0f / static_cast<float>( CIRCLE_NUM_SIDES );

	for ( int i = 0; i < CIRCLE_NUM_SIDES; i++ )
	{
		float cosTheta = CosDegrees( theta );
		float sinTheta = SinDegrees( theta );
		m_cosmeticVertices[ i ] = Vector2( ( m_cosmeticDisc2.radius * cosTheta ), ( m_cosmeticDisc2.radius * sinTheta ) );
		m_physicalVertices[ i ] = Vector2( ( m_physicalDisc2.radius * cosTheta ), ( m_physicalDisc2.radius * sinTheta ) );

		theta += deltaTheta;
	}
}

void Entity::WrapAroundScreenIfBeyondBounds()
{
	if ( IsEntityBeyondNegativeXBound() )
	{
		m_location = Vector2( SCREEN_WIDTH + m_cosmeticDisc2.radius, m_location.y );
	}
	else if ( IsEntityBeyondPositiveXBound() )
	{
		m_location = Vector2( -m_cosmeticDisc2.radius, m_location.y );
	}
	else if ( IsEntityBeyondNegativeYBound() )
	{
		m_location = Vector2( m_location.x, SCREEN_HEIGHT + m_cosmeticDisc2.radius );
	}
	else if ( IsEntityBeyondPositiveYBound() )
	{
		m_location = Vector2( m_location.x, -m_cosmeticDisc2.radius );
	}
}

bool Entity::IsEntityBeyondNegativeXBound() const
{
	return ( m_location.x < -m_cosmeticDisc2.radius );
}

bool Entity::IsEntityBeyondPositiveXBound() const
{
	return ( m_location.x > ( SCREEN_WIDTH + m_cosmeticDisc2.radius ) );
}


bool Entity::IsEntityBeyondNegativeYBound() const
{
	return ( m_location.y < -m_cosmeticDisc2.radius );
}

bool Entity::IsEntityBeyondPositiveYBound() const
{
	return ( m_location.y > ( SCREEN_HEIGHT + m_cosmeticDisc2.radius ) );
}

void Entity::RenderDeveloperMode() const
{
	RenderVelocityVector();
	RenderPhysicalCircle();
	RenderCosmeticCircle();
}

void Entity::RenderVelocityVector() const
{
	const float VELOCITY_VECTOR_LINE_THICKNESS = 1.5f;

	Vector2 velocityVector = m_location + ( m_speed * m_velocity );		// This vector should stretch from the center of the entity to the point where it will appear in the next second (not counting framerate changes owing to the cheat key(s))
	g_renderer->DrawLine( m_location, velocityVector, Rgba( RGBA_MAX, RGBA_MAX, 0, RGBA_MAX ), Rgba( RGBA_MAX, RGBA_MAX, 0, RGBA_MAX ), VELOCITY_VECTOR_LINE_THICKNESS );

	g_renderer->ChangeRenderColor( Rgba( RGBA_MAX, RGBA_MAX, RGBA_MAX, RGBA_MAX ) );
}

void Entity::RenderPhysicalCircle() const
{
	g_renderer->ChangeRenderColor( Rgba( 0, RGBA_MAX, RGBA_MAX, RGBA_MAX ) );
	g_renderer->DrawPolygon( m_location, m_orientation, m_physicalVertices, CIRCLE_NUM_SIDES, RPolygonType::BROKEN_LINES );
	g_renderer->ChangeRenderColor( Rgba( RGBA_MAX, RGBA_MAX, RGBA_MAX, RGBA_MAX ) );
}

void Entity::RenderCosmeticCircle() const
{
	g_renderer->ChangeRenderColor( Rgba( RGBA_MAX, 0, RGBA_MAX, RGBA_MAX ) );
	g_renderer->DrawPolygon( m_location, m_orientation, m_cosmeticVertices, CIRCLE_NUM_SIDES, RPolygonType::BROKEN_LINES );
	g_renderer->ChangeRenderColor( Rgba( RGBA_MAX, RGBA_MAX, RGBA_MAX, RGBA_MAX ) );
}
