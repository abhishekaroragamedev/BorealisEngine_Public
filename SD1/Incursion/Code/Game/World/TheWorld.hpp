#pragma once

#include <map>
#include <string>
#include <vector>
#include "Engine/Math/MathUtils.hpp"
#include "Game/World/Map.hpp"

constexpr char MAP_ONE_NAME[] = "Map One";
constexpr char MAP_TWO_NAME[] = "Map Two";
constexpr char MAP_THREE_NAME[] = "Map Three";

constexpr int MAP_ONE_HORIZONTAL_TILE_COUNT = 25;
constexpr int MAP_ONE_VERTICAL_TILE_COUNT = 25;
constexpr float MAP_ONE_SOLID_TILE_CHANCE = 0.1f;
constexpr float MAP_ONE_LIQUID_TILE_CHANCE = 0.05f;
constexpr int MAP_ONE_NUM_ENEMY_TANKS = 10;
constexpr int MAP_ONE_NUM_ENEMY_TURRETS = 10;

constexpr int MAP_TWO_HORIZONTAL_TILE_COUNT = 50;
constexpr int MAP_TWO_VERTICAL_TILE_COUNT = 25;
constexpr float MAP_TWO_SOLID_TILE_CHANCE = 0.05f;
constexpr float MAP_TWO_LIQUID_TILE_CHANCE = 0.15f;
constexpr int MAP_TWO_NUM_ENEMY_TANKS = 25;
constexpr int MAP_TWO_NUM_ENEMY_TURRETS = 25;

constexpr int MAP_THREE_HORIZONTAL_TILE_COUNT = 50;
constexpr int MAP_THREE_VERTICAL_TILE_COUNT = 50;
constexpr float MAP_THREE_SOLID_TILE_CHANCE = 0.02f;
constexpr float MAP_THREE_LIQUID_TILE_CHANCE = 0.15f;
constexpr int MAP_THREE_NUM_ENEMY_TANKS = 45;
constexpr int MAP_THREE_NUM_ENEMY_TURRETS = 45;

constexpr int TERRAIN_SPRITE_SHEET_NUM_SPRITES_HORIZONTAL = 8;
constexpr int TERRAIN_SPRITE_SHEET_NUM_SPRITES_VERTICAL = 8;
constexpr int EXPLOSION_SPRITE_SHEET_NUM_SPRITES_HORIZONTAL = 5;
constexpr int EXPLOSION_SPRITE_SHEET_NUM_SPRITES_VERTICAL = 5;

class TheWorld
{

public:
	TheWorld();
	~TheWorld();

public:
	void SetCurrentMap( std::string mapName );
	void Update( float deltaSeconds );
	void InitializeTheWorld();
	void FlushSoundsPlaying();
	void AddSoundPlaying( size_t newSound );
	void Render() const;
	Map* GetCurrentMap() const;
	PlayerTank* GetPlayerTank() const;
	SpriteSheet* GetExplosionSpritesheet() const;
	bool IsGamePaused() const;
	bool HasPlayerWon() const;
	float GetPlayerSoundMuteFraction() const;
	float GetPlayerHealthPulseFraction() const;

private:
	void HandleKeyboardInput( float& out_deltaSeconds );
	void HandleXboxControllerInput( float& out_deltaSeconds );
	void HandlePauseKeyPress();
	void UpdatePlayerSoundMuteFraction();
	void UpdatePlayerHealthPulseFraction();
	void LoadNextMap();
	void CreateTileDefinitions();
	void CreateMapOne();
	void CreateMapTwo();
	void CreateMapThree();
	void LoadMap( std::string mapName );
	void DeleteTileDefinitions();
	void DeleteAllMapReferences();

private:
	const float CHEAT_DELTA_SECONDS_SLOWDOWN_MULTIPLIER = 0.1f;
	const float CHEAT_DELTA_SECONDS_SPEEDUP_MULTIPLIER = 2.0f;
	const float PLAYER_SOUND_MUTE_FRACTION_MAX_UPDATE = 0.2f;		// If the amount the player can hear changes drastically, it's not good. So ease it up
	const float PLAYER_HEALTH_PULSE_FRACTION_MAX_UPDATE = 0.5f;		// Same thing with the player's health; it should continuously update; not discretely

	std::map< std::string, Map* > m_mapsByName;
	std::map< std::string, int > m_numEnemyTanksByMapName;
	std::map< std::string, int > m_numEnemyTurretsByMapName;
	std::vector<size_t> m_playbackIdsOfSoundsPlaying;
	Map* m_currentMap;
	Texture* m_terrainTexture;
	Texture* m_explosionTexture;
	SpriteSheet* m_terrainSpritesheet;
	SpriteSheet* m_explosionSpritesheet;
	PlayerTank* m_playerTank;
	bool m_gamePaused;
	bool m_developerModeEnabled;
	bool m_hasPlayerWon;
	float m_deltaSecondsMultiplier;
	float m_playerSoundMuteFraction;
	float m_playerHealthPulseFraction;
	int m_framesSinceWorldInitialized;

};
