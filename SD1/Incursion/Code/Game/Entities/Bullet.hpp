#pragma once

#include "Engine/Core/Rgba.hpp"
#include "Game/Entities/Entity.hpp"
#include "Game/World/Tile.hpp"

class Bullet : public Entity
{

protected:
	Bullet( const Vector2& spawnLocation, const Vector2& spawnVelocity );		// Only allow subclasses to call this constructor
	~Bullet();

public:
	void Update( float deltaSeconds ) override;
	void RedirectBullet( const Vector2& newVelocity );
	void Destroy();
	void Render( bool developerModeEnabled ) const override;
	Disc2 GetDiscCollider() const;
	Vector2 GetNextPosition( float deltaSeconds ) const;

protected:
	void Translate( float deltaSeconds ) override;

protected:
	const float BULLET_COSMETIC_DISC_RADIUS_WORLD_UNITS = 0.05f * TILE_SIDE_LENGTH_WORLD_UNITS;
	const float BULLET_PHYSICAL_DISC_RADIUS_WORLD_UNITS = 0.005f * TILE_SIDE_LENGTH_WORLD_UNITS;
	const float BULLET_SPEED = TILE_SIDE_LENGTH_WORLD_UNITS * 5.0f;
	const std::string BULLET_TEXTURE_PATH = "Data/Images/Bullet.png";

	Disc2 m_collisionDisc;
	Texture* m_bulletTexture;
	Rgba m_renderColor;		// This must be set by a child class

};
