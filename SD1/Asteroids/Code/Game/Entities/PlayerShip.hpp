#pragma once

#include "Game/Entities/Entity.hpp"

constexpr float SHIP_RADIUS = 25.0f;
constexpr int SHIP_NUM_VERTICES = 5;
constexpr int SHIP_EXHAUST_NUM_VERTICES = 3;

enum PlayerIndex : unsigned char
{
	CONTROLLER_1,
	CONTROLLER_2,
	CONTROLLER_3,
	CONTROLLER_4,
	KEYBOARD,
	MAX_NUM_PLAYERS
};

class PlayerShip : public Entity
{

public:
	PlayerShip( PlayerIndex playerIndex );
	~PlayerShip();

public:
	void Update( float deltaSeconds );
	void Render( bool developerModeEnabled ) const;
	void MarkForDeathAndResetPositionAndOrientation();
	Vector2 GetBulletSpawnLocation() const;
	Vector2 GetBulletSpawnVelocity() const;
	bool IsDead() const;

private:
	void InitializeForSpawn();
	void PopulateShipVertices();
	void PopulateExhaustVertices();
	void Respawn();
	void UpdateDeathStateRenderInformation( float deltaSeconds );
	void UpdateDeathRenderAlpha( float deltaSeconds );
	void UpdateDeathFragmentDistanceMultiplier( float deltaSeconds );
	void UpdateDeathScreenShakeTheta( float deltaSeconds );
	void UpdateTimeSinceDeathForControllerVibration( float deltaSeconds );
	void AccelerateShip( float deltaSeconds );
	void HandleKeyboardInput( float deltaSeconds );
	void HandleXboxControllerInput( float deltaSeconds );
	Rgba GetShipColorFromPlayerIndex() const;
	void ShakeScreen() const;
	void DrawDeadShipFragments() const;
	void DrawShip() const;
	void DrawExhaust() const;

private:
	const float SHIP_SPEED = 150.0f;
	const float SHIP_PHYSICAL_TO_COSMETIC_RADIUS_RATIO = 0.6f;
	const float SHIP_ROTATION_SPEED = 300.0f;
	const float SHIP_START_ORIENTATION_DEGREES = 90.0f;
	const float SHIP_DEATH_RENDER_ALPHA_DECREMENT_PER_SECOND = 127.5f;
	const float SHIP_DEATH_FRAGMENT_DISTANCE_MULTIPLIER_INCREMENT_PER_SECOND = 20.0f;
	const float SHIP_DEATH_CONTROLLER_VIBRATION_SECONDS = 1.0f;
	const float SHIP_DEATH_SCREEN_SHAKE_MAGNITUDE = SCREEN_WIDTH / 50.0f;
	const float SHIP_DEATH_SCREEN_SHAKE_START_THETA_DEGREES = 90.0f;
	const float SHIP_DEATH_SCREEN_SHAKE_THETA_DECREMENT_PER_SECOND = 90.0f;
	const unsigned short SHIP_DEATH_CONTROLLER_VIBRATION_MOTOR_VALUE = 4000;
	const Vector2 SHIP_SPAWN_POSITION = Vector2( ( SCREEN_WIDTH / 2.0f ), ( SCREEN_HEIGHT / 2.0f ) );
	const Vector2 SHIP_ACCELERATION = Vector2( 2.0f, 0.0f );	
	const Vector2 SHIP_TIP = Vector2( SHIP_RADIUS, 0.0f );
	const Vector2 SHIP_LEFT_EXHAUST_PORT_LOCATION = Vector2( -SHIP_RADIUS / 4.5f, SHIP_RADIUS / 4.5f );
	const Vector2 SHIP_RIGHT_EXHAUST_PORT_LOCATION = Vector2( -SHIP_RADIUS / 4.5f, -SHIP_RADIUS / 4.5f );

private:
	Vector2 m_exhaustVertices[ SHIP_EXHAUST_NUM_VERTICES ];
	Vector2 m_shipVertices[ SHIP_NUM_VERTICES ];
	Vector2 m_acceleration;
	bool m_drawExhaust;
	bool m_isDead;
	float m_deathRenderAlpha;
	float m_deathFragmentDistanceMultiplier;
	float m_deathScreenShakeTheta;
	float m_timeSinceDeath;
	PlayerIndex m_playerIndex;

};
