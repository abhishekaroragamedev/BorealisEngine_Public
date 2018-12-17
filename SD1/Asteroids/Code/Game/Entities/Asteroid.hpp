#pragma once

#include "Game/Entities/Entity.hpp"

constexpr float ASTEROID_RADIUS_LARGE = ( SCREEN_HEIGHT / 10.0f );
constexpr float ASTEROID_RADIUS_MEDIUM = ( ASTEROID_RADIUS_LARGE / 2.0f );
constexpr float ASTEROID_RADIUS_SMALL = ( ASTEROID_RADIUS_MEDIUM / 2.0f );

class Asteroid : public Entity
{

public:
	explicit Asteroid( float asteroidRadius );
	explicit Asteroid( float asteroidRadius, const Vector2& spawnLocation );
	~Asteroid();

public:
	void Update( float deltaSeconds );
	void Render( bool developerModeEnabled ) const;
	float GetRadius() const;

private:
	void PopulateVertices();
	Vector2 ComputeAsteroidSpawnPosition( float radius ) const;

private:
	const int ASTEROID_MIN_SIDES = 15;
	const int ASTEROID_MAX_SIDES = 20;
	const float ASTEROID_PHYSICAL_TO_COSMETIC_RADIUS_RATIO = 0.8f;
	const float ASTEROID_SPEED = 25.0f;
	const float ASTEROID_ROTATION_SPEED = 70.0f;
	const float ASTEROID_NOISE_MINIMUM_RADIUS_FRACTION = 0.6f;
	const float ASTEROID_NOISE_MAXIMUM_RADIUS_FRACTION = 0.8f;
	int m_numSides;
	float m_radius;
	Vector2* m_vertices;		// This is a dynamic array, since the number of vertices can vary

};
