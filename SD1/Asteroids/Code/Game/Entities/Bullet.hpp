#pragma once

#include "Game/Entities/Entity.hpp"

constexpr int BULLET_CIRCLE_NUMSIDES = 30;

class Bullet : public Entity
{

public:
	explicit Bullet( const Vector2& location, const Vector2& velocity );
	~Bullet();

public:
	void Update( float deltaSeconds );
	void Render( bool developerModeEnabled ) const;
	float GetTimeLeftToLive() const;

private:
	void PopulateVertices();

private:
	const float BULLET_LIFE_SECONDS = 2.0f;
	const float BULLET_SPEED = 350.0f;
	const float BULLET_RADIUS = 2.5f;
	float m_timeLived;
	Vector2 m_vertices[ BULLET_CIRCLE_NUMSIDES ];
};
