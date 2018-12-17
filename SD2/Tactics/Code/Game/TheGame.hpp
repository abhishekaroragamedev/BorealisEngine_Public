#pragma once

#include "Game/GameStates/EncounterGameState.hpp"
#include "Game/GameStates/GameState.hpp"
#include "Game/GameStates/LoadingGameState.hpp"
#include "Game/GameStates/MenuGameState.hpp"
#include "Game/Entities/Entity.hpp"
#include "Game/World/BlockDefinition.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include <iterator>
#include <vector>

constexpr char DEFAULT_FONT_NAME[] = "SquirrelFixedFont";
constexpr char ITEMS_TEXTURE_PATH[] = "Data/Images/Items_13x14.png";
constexpr char TERRAIN_TEXTURE_PATH[] = "Data/Images/Terrain_32x32.png";
constexpr char GAME_CONFIG_XML_PATH[] = "Data/GameConfig.xml";
constexpr char ACTOR_DEFINITIONS_XML_PATH[] = "Data/Definitions/ActorDefinitions.xml";
constexpr char ENCOUNTER_DEFINITIONS_XML_PATH[] = "Data/Definitions/EncounterDefinitions.xml";

constexpr float XBOX_ANALOG_CURSOR_MOVE_COOLDOWN_SECONDS = 0.2f;

class TheGame
{

public:
	TheGame();
	~TheGame();

public:
	void Update();
	void LoadResources();
	void Render() const;
	SpriteSheet* GetTerrainTextureAtlas() const;
	SpriteSheet* GetItemsTextureAtlas() const;
	GameState* GetGameStateInstance( GameStateType stateType );
	GameStateType GetCurrentGameState() const;
	void ChangeGameState( GameStateType nextGameState );
	void LoadEncounter( const EncounterDefinition& encounterDefinition, bool isNetworked=false, bool isHost=false );

	inline bool CanJoin() const { return m_canJoin; }
	void SetCanJoin( bool canJoin );

private:
	void InitializeGameStates();
	BlockDefinition* RegisterBlockType( const std::string& name, const AABB2& tileTopUV = AABB2::ZERO_TO_ONE, const AABB2& tileSideUV = AABB2::ZERO_TO_ONE, const AABB2& tileBottomUV = AABB2::ZERO_TO_ONE );

	void HandleKeyboardInput();
	void HandleXboxControllerInputs();
	void LoadActorDefinitions();
	void LoadEncounterDefinitions();
	void DeleteGameStates();
	void DeleteActorDefinitions();
	void DeleteBlockDefinitions();
	void DeleteEncounterDefinitions();
	void TransitionToNextGameState();

private:
	BitmapFont* m_defaultFont = nullptr;
	GameState* m_gameStates[ GameStateType::NUM_STATES ];
	GameStateType m_currentGameState = GameStateType::STATE_INVALID;
	GameStateType m_nextGameState = GameStateType::STATE_INVALID;
	SpriteSheet* m_terrainSpriteSheet = nullptr;
	SpriteSheet* m_itemsSpriteSheet = nullptr;
	
	// Networking
	bool m_canJoin = false;

};
