#pragma once

#include "Game/Entities/Asteroid.hpp"
#include "Game/Entities/Bullet.hpp"
#include "Game/Entities/PlayerShip.hpp"

constexpr int PLAYER_SHIP_MAX_COUNT = 5;
constexpr int ASTEROID_MAX_COUNT = 1000;
constexpr int BULLET_MAX_COUNT = 1000;

class TheGame
{

public:
	TheGame();
	~TheGame();

public:
	void Update( float& deltaSeconds );
	void SpawnLargeAsteroid();
	bool DestroyAsteroid( int index, bool isCalledFromCheatKeypressOrDestructor );
	void Render() const;
	int GetAsteroidCount() const;

private:
	void InitializeEntityArrays();
	void SpawnShip( PlayerIndex playerIndex );
	void SpawnBullet( int playerShipIndex );
	void SpawnMediumAsteroid( const Vector2& spawnLocation );
	void SpawnSmallAsteroid( const Vector2& spawnLocation );
	bool DetectAndHandleAsteroidCollisionWithBullets( int asteroidIndex );
	bool DetectAndHandleAsteroidCollisionWithPlayer( int asteroidIndex, int playerIndex );
	void HandleKeyboardInput( float& out_deltaSeconds );
	void HandleXboxControllerInputs();
	void StartNewWaveIfNoAsteroidsAreLeft();
	void SpawnAsteroidsAfterLargerAsteroidDeath( float asteroidRadius, const Vector2& spawnLocation );
	void UpdateAllObjects( float deltaSeconds );
	bool DestroyBullet( int index );
	bool DestroyShip( PlayerIndex playerIndex );
	void DeleteAllObjects();
	void RenderAllObjects() const;

private:
	const float CHEAT_DELTA_SECONDS_SLOWDOWN_MULTIPLIER = 0.2f;
	const int NUM_ASTEROIDS_SPAWN_ON_DEATH = 2;
	const int WAVE_INITIAL_ASTEROIDS_COUNT_MINIMUM = 4;
	const int WAVE_ASTEROIDS_COUNT_MAX_VARIATION = 2;
	const int WAVE_ASTEROIDS_COUNT_INCREMENT = 5;

	int m_activeAsteroidCount;
	int m_nextWaveAsteroidCount;
	int m_bulletCount;
	bool m_developerModeEnabled;
	Asteroid* m_asteroids[ ASTEROID_MAX_COUNT ];
	PlayerShip* m_playerShips[ PLAYER_SHIP_MAX_COUNT ];
	Bullet* m_bullets[ BULLET_MAX_COUNT ];
};
