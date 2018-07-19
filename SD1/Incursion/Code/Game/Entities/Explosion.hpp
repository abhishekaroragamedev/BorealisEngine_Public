#pragma once

#include "Engine/Renderer/SpriteAnimation.hpp"
#include "Game/Entities/Entity.hpp"

class Explosion : public Entity
{

	friend class TheEntityFactory;

private:
	explicit Explosion( const Vector2& spawnLocation, float size, float durationSeconds );
	~Explosion();

public:
	void Update( float deltaSeconds ) override;
	void Render( bool developerModeEnabled ) const override;

private:
	const int EXPLOSION_SPRITESHEET_NUM_SPRITES = 25;

	SpriteAnimation* m_explosionAnimation;
	float m_size;
};