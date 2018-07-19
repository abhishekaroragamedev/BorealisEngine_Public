#pragma once

#include "Engine/Math/Disc2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/Entities/Tank.hpp"
#include "Game/World/Tile.hpp"

constexpr float PLAYER_HEALTH_MAX_BORDER_FRACTION = 0.5f;

class PlayerTank : public Tank
{

	friend class TheEntityFactory;
	friend class TheWorld;

private:
	PlayerTank( const Vector2& spawnLocation );
	~PlayerTank();

public:
	void Update( float deltaSeconds ) override;
	void SetLocation( Vector2 newLocation );
	void DoDamage() override;
	bool IsDeadAndCanRespawn() const;
	bool WasDamagedThisFrame() const;
	float GetMaxHealthAsFloat() const;
	float GetFractionOfDamageDone() const;
	float GetFractionOfHealthRemaining() const;
	float GetTimeSinceDeath() const;
	bool IsGodModeEnabled() const;
	void Render( bool developerModeEnabled ) const override;
	void Respawn();

private:
	void UpdateTimeSinceLastHitAndRegenHealth( float deltaSeconds );
	void UpdateTimeSinceDeath( float deltaSeconds );
	void HandleKeyboardInput();
	void HandleXboxControllerInput( float deltaSeconds );
	void StopControllerVibrationIfTimeHasPassed();

private:
	const int PLAYER_TANK_START_HEALTH = 20;
	const int PLAYER_HEALTH_REGEN_PER_SECOND = 1;
	const float PLAYER_TANK_BULLET_XBOX_TRIGGER_THRESHOLD = 0.5f;
	const float PLAYER_TANK_TURN_SPEED_DEGREES_PER_SECOND = 90.0f;
	const float PLAYER_TANK_SPEED = TILE_SIDE_LENGTH_WORLD_UNITS;
	const float PLAYER_TANK_BULLET_FIRE_COOLDOWN_SECONDS = 0.1f;
	const float PLAYER_TANK_RESPAWN_TIME_SECONDS = 2.0f;
	const float PLAYER_TANK_REGEN_START_TIME_SECONDS = 3.0f;
	const float PLAYER_TANK_HIT_VIBRATION_TIME_SECONDS = 0.5f;
	const float PLAYER_TANK_DEATH_VIBRATION_TIME_SECONDS = 1.0f;
	const unsigned short PLAYER_TANK_FIRE_VIBRATION_RIGHT_MOTOR_VALUE = 20000;
	const unsigned short PLAYER_TANK_HIT_VIBRATION_LEFT_MOTOR_VALUE = 20000;
	const unsigned short PLAYER_TANK_DEATH_VIBRATION_MOTOR_VALUE = 25000;
	const std::string PLAYER_TANK_TEXTURE_PATH = "Data/Images/PlayerTankBase.png";
	const std::string PLAYER_TANK_TURRET_TEXTURE_PATH = "Data/Images/PlayerTankTop.png";
	
	float m_timeSinceDeath;
	float m_timeSinceLastHit;
	float m_timeSinceLastRegen;
	bool m_godModeEnabled;
	bool m_wasDamagedThisFrame;

};
