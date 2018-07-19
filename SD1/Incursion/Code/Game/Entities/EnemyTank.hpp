#pragma once

#include "Engine/Math/Disc2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/Entities/Tank.hpp"
#include "Game/World/Tile.hpp"

class EnemyTank : public Tank
{

	friend class TheEntityFactory;

private:
	EnemyTank( const Vector2& spawnLocation );
	~EnemyTank();

public:
	void Update( float deltaSeconds ) override;

private:
	void ComputeAIMove( float deltaSeconds );

private:
	const int ENEMY_TANK_START_HEALTH = 5;
	const float ENEMY_TANK_MAX_SIGHT_DISTANCE = 5.0f * TILE_SIDE_LENGTH_WORLD_UNITS;
	const float ENEMY_TANK_DIRECTION_CHANGE_TIME_SECONDS = 2.0f;
	const float ENEMY_TANK_TURN_SPEED_DEGREES_PER_SECOND = 90.0f;
	const float ENEMY_TANK_SPEED = TILE_SIDE_LENGTH_WORLD_UNITS / 2.0f;
	const float ENEMY_TANK_AI_DRIVE_FORWARD_MIN_DOT_PRODUCT = 0.707f;		// cos 45
	const float ENEMY_TANK_AI_FIRE_BULLET_MIN_DOT_PRODUCT = 0.996f;		// cos 5
	const float ENEMY_TANK_BULLET_FIRE_COOLDOWN_SECONDS = 0.5f;
	const std::string ENEMY_TANK_TEXTURE_PATH = "Data/Images/NemesisTankBase.png";
	const std::string ENEMY_TANK_TURRET_TEXTURE_PATH = "Data/Images/NemesisTankTop.png";

	float m_timeSinceLastDirectionChange;
	float m_goalOrientation;
	Vector2 m_lastSeenPlayerTankLocation;
	bool m_shouldPursue;
};
