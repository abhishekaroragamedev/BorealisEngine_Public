#pragma once

#include "Game/Entities/Actor.hpp"
#include "Game/Entities/ActorDefinition.hpp"
#include "Engine/Math/Vector2.hpp"
#include <string>

class Player : public Actor
{

	friend class Map;

public:
	~Player();

	void SetMap( const Map* newMap );		// The Player can be moved between Maps
	void Respawn();
	void HandleActorDeath() override;
	bool IsInvincible() const;
	void Damage( int damageStats[ StatID::STAT_STRENGTH ] ) override;

private:
	explicit Player( const std::string instanceName, const Vector2& position, ActorDefinition* actorDefinition, const Map& map );

	void PerformAction() override;
	void HandleKeyboardInput();
	void HandleXboxControllerInput();

private:
	bool m_isInvincible = false;

};
