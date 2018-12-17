#pragma once

#include "Game/Dialogue.hpp"
#include "Game/Entities/Entity.hpp"
#include "Game/World/Adventure.hpp"
#include "Engine/Math/IntRange.hpp"
#include <deque>
#include <iterator>
#include <vector>

constexpr char SAMPLE_AUDIO_FILE_PATH[] = "Data/Audio/TestSound.mp3" ;

constexpr char GAME_CONFIG_XML_FILE_PATH[] = "Data/GameConfig.xml";
constexpr char ADVENTURE_XML_FILE_PATH[] = "Data/Definitions/Adventures.xml";
constexpr char TILE_XML_FILE_PATH[] = "Data/Definitions/Tiles.xml";
constexpr char MAP_XML_FILE_PATH[] = "Data/Definitions/Maps.xml";
constexpr char ACTOR_XML_FILE_PATH[] = "Data/Definitions/Actors.xml";
constexpr char ITEM_XML_FILE_PATH[] = "Data/Definitions/Items.xml";
constexpr char LOOT_XML_FILE_PATH[] = "Data/Definitions/Loot.xml";
constexpr char PROJECTILE_XML_FILE_PATH[] = "Data/Definitions/Projectiles.xml";
constexpr char PORTAL_XML_FILE_PATH[] = "Data/Definitions/Portals.xml";

constexpr char TILE_SPRITE_SHEET_FILE_PATH[] = "Data/Images/Terrain_32x32.png";
constexpr int TILE_SPRITE_SHEET_NUM_TILES_WIDE = 32;
constexpr int TILE_SPRITE_SHEET_NUM_TILES_HIGH = 32;

constexpr char FIXED_FONT_NAME[] = "SquirrelFixedFont";

enum GameState
{
	NONE = -1,
	ATTRACT,
	PLAYING,
	DIALOGUE,
	INVENTORY,
	PAUSED,
	DEFEAT,
	VICTORY,
	NUM_GAME_STATES
};

enum InventoryMenuSelectionState
{
	INVENTORY_MENU_STATE_NONE = -1,
	INVENTORY_MENU_STATE_INVENTORY,
	INVENTORY_MENU_STATE_EQUIPMENT,
	NUM_INVENTORY_MENU_STATES
};

class TheGame
{

public:
	TheGame();
	~TheGame();

public:
	void Update( float& deltaSeconds );
	void Render() const;
	void StartTransitionToState( GameState newGameState );
	void PushDialogue( Dialogue* newDialogue );
	void PushDialogue( const std::string& text, const AABB2& bounds, DialogueStyle dialogueStyle );
	Adventure* GetCurrentAdventure() const;

private:
	void InitializeTileSpriteSheet();
	void CreateTileDefinitionsFromXml();
	void CreateMapDefinitionsFromXml();
	void CreateActorDefinitionsFromXml();
	void CreateLootDefinitionsFromXml();
	void CreateItemDefinitionsFromXml();
	void CreateProjectileDefinitionsFromXml();
	void CreatePortalDefinitionsFromXml();
	void CreateAdventureDefinitionsFromXml();
	IntRange ComputeIndexRangeForListPaginationBasedOnGameState() const;
	void Update_Attract( float deltaSeconds );
	void Update_Playing( float deltaSeconds );
	void Update_Dialogue( float deltaSeconds );
	void Update_Inventory( float deltaSeconds );
	void Update_Paused( float deltaSeconds );
	void Update_Defeat( float deltaSeconds );
	void Update_Victory( float deltaSeconds );
	void UpdateStateVariables( float deltaSeconds );
	float GetRenderAlphaForState( GameState state ) const;
	void Render_Attract() const;
	void Render_Playing() const;
	void Render_Dialogue() const;
	void Render_Inventory() const;
	void Render_Paused() const;
	void Render_Defeat() const;
	void Render_Victory() const;
	void Render_HUD( float alphaFraction ) const;
	void RenderAdventureList( const AABB2& screenBounds, const Rgba& textColor, float lineHeight, float alphaFraction ) const;
	void RenderItemList( const AABB2& screenBounds, const Rgba& textColor, float lineHeight, float alphaFraction ) const;
	void RenderEquipmentList( const AABB2& screenBounds, const Rgba& textColor, float lineHeight, float alphaFraction ) const;
	void RenderStatList( const AABB2& screenBounds, const Rgba& textColor, float lineHeight, float alphaFraction ) const;
	Rgba GetLineRenderColorForStatListLine( StatID statID, const Player* playerEntity, const Rgba& defaultColor ) const;
	void RenderSelectedItemDescription( const AABB2& screenBounds, const Rgba& textColor, float lineHeight, float alphaFraction  ) const;
	void RenderSelectedItemInstructions( const AABB2& screenBounds, const Rgba& textColor, float lineHeight, float alphaFraction  ) const;
	void RenderSelectedItemBoundingBox( const std::string& currentLine, const AABB2& selectedItemBounds, float lineHeight, const Vector2& alignment, float alphaFraction ) const;
	void HandleDebugKeyboardInput( float& out_deltaSeconds ) const;
	void HandleStateTransitionKeyboardInput();
	void HandleStateTransitionXboxControllerInput();
	void HandleAttractScreenKeyboardInput();
	void HandleAttractScreenXboxControllerInput();
	void HandleInventoryScreenKeyboardInput();
	void HandleInventoryScreenXboxControllerInput();
	bool IsStateTransitioning() const;
	void RenderNameInBitmapFont() const;
	void DeleteAdventureDefinitions();
	void DeleteTileDefinitions();
	void DeleteMapDefinitions();
	void DeleteEntityDefinitions();
	void DeleteLootDefinitions();

private:
	Adventure* m_currentAdventure = nullptr;
	GameState m_currentGameState = GameState::ATTRACT;
	GameState m_transitionToState = GameState::NONE;
	GameState m_dialogueReturnToState = GameState::NONE;		// Can be either Inventory or Playing
	InventoryMenuSelectionState m_inventoryMenuState = InventoryMenuSelectionState::INVENTORY_MENU_STATE_INVENTORY;
	int m_menuSelectionIndex = 0;
	float m_numSecondsInCurrentGameState = 0.0f;
	float m_numSecondsIntoStateTransition = 0.0f;
	bool m_isFinishedTransitioning = false;
	bool m_developerModeEnabled = false;
	std::deque< Dialogue* > m_dialogueQueue;

};
