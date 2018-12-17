#include "Game/Entities/Bullet.hpp"

Bullet::Bullet( const Vector2& location, const Vector2& velocity )
{
	m_location = location;
	m_velocity = velocity;

	m_cosmeticDisc2 = Disc2( m_location, BULLET_RADIUS );
	m_physicalDisc2 = Disc2( m_location, BULLET_RADIUS );

	m_orientation = 0.0f;
	m_speed = BULLET_SPEED;
	m_timeLived = 0.0f;

	PopulateVertices();
	PopulateDeveloperModeCirclesVertices();
}

Bullet::~Bullet()
{

}

float Bullet::GetTimeLeftToLive() const
{
	return ( BULLET_LIFE_SECONDS - m_timeLived );
}

void Bullet::Update( float deltaSeconds )
{
	Entity::Update( deltaSeconds );
	m_timeLived += deltaSeconds;
}

void Bullet::Render( bool developerModeEnabled ) const
{
	g_renderer->ChangeRenderColor( Rgba( RGBA_MAX, 0, 0, RGBA_MAX ) );
	g_renderer->DrawPolygon( m_location, m_orientation, m_vertices, BULLET_CIRCLE_NUMSIDES, RPolygonType::REGULAR );
	g_renderer->ChangeRenderColor( Rgba( RGBA_MAX, RGBA_MAX, RGBA_MAX, RGBA_MAX ) );

	Entity::Render( developerModeEnabled );
}

void Bullet::PopulateVertices()
{
	float theta = 0.0f;
	float deltaTheta = 360.0f / static_cast<float>( BULLET_CIRCLE_NUMSIDES );

	for ( int i = 0; i < BULLET_CIRCLE_NUMSIDES; i++ )
	{
		float cosTheta = CosDegrees( theta );
		float sinTheta = SinDegrees( theta );
		m_vertices[ i ] = Vector2( ( BULLET_RADIUS * cosTheta ), ( BULLET_RADIUS * sinTheta ) );

		theta += deltaTheta;
	}
}