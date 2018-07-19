#pragma once

#include "Engine/Math/Disc2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/Entities/Entity.hpp"
#include "Game/Physics/RaycastResult2D.hpp"
#include "Game/World/Tile.hpp"

class EnemyTurret : public Entity
{

	friend class TheEntityFactory;

private:
	EnemyTurret( const Vector2& spawnLocation );
	~EnemyTurret();

public:
	void Update( float deltaSeconds ) override;
	void Render( bool developerModeEnabled ) const override;
	float GetTurretOrientationDegrees() const;
	Disc2 GetDiscCollider() const;

private:
	void ComputeAIMove( float deltaSeconds );
	void RestoreTurretToCenterPosition( float deltaSeconds );
	bool CanFireBullets() const;
	void UpdateBulletFireCooldown( float deltaSeconds );
	void UpdateLineOfSightRaycast();

private:
	const int ENEMY_TURRET_START_HEALTH = 7;
	const float ENEMY_TURRET_MAX_SIGHT_DISTANCE = 20.0f * TILE_SIDE_LENGTH_WORLD_UNITS;
	const float ENEMY_TURRET_TURN_SPEED_DEGREES_PER_SECOND = 30.0f;
	const float ENEMY_TURRET_SPEED = TILE_SIDE_LENGTH_WORLD_UNITS / 2.0f;
	const float ENEMY_TURRET_AI_FIRE_BULLET_MIN_DOT_PRODUCT = 0.996f;		// cos 5
	const float ENEMY_TURRET_BULLET_FIRE_COOLDOWN_SECONDS = 1.0f;
	const std::string ENEMY_TURRET_BASE_TEXTURE_PATH = "Data/Images/EnemyTurretBase.png";
	const std::string ENEMY_TURRET_TEXTURE_PATH = "Data/Images/EnemyTurretTop.png";
	const float TURRET_COSMETIC_DISC_RADIUS_WORLD_UNITS = 0.5f * TILE_SIDE_LENGTH_WORLD_UNITS;
	const float TURRET_PHYSICAL_DISC_RADIUS_WORLD_UNITS = 0.4f * TILE_SIDE_LENGTH_WORLD_UNITS;
	const float TURRET_LINE_OF_SIGHT_THICKNESS = 2.0f;
	const float TURRET_WANDER_MODE_MAX_TURN_DEGREES = 10.0f;
	const float TURRET_MAX_OFFSET = 0.15f * TILE_SIDE_LENGTH_WORLD_UNITS;
	const float TURRET_RECOIL_MAX_RECOVERY_PER_SECOND = 0.15f * TILE_SIDE_LENGTH_WORLD_UNITS;

	Disc2 m_discCollider;
	float m_turretOrientation;
	float m_turretOffset;
	float m_timeSinceBulletFired;
	float m_bulletCooldownTime;
	Texture* m_turretTexture;
	Texture* m_turretBaseTexture;
	float m_goalOrientation;
	float m_currentTurnDirectionWanderMode;
	RaycastResult2D m_lineOfSightRaycastForFrame;
};
