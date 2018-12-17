#pragma once

#include "Game/Action.hpp"
#include "Game/Encounter.hpp"
#include "Game/EncounterMode.hpp"
#include "Game/GameStates/GameState.hpp"
#include "Game/World/Map.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Math/IntVector3.hpp"
#include "Engine/Networking/NetConnection.hpp"
#include "Engine/Renderer/OrbitCamera.hpp"
#include "Engine/Tools/Command.hpp"
#include <deque>
#include <stack>

constexpr char FULLSCREEN_GRAYSCALE_SHADER_NAME[] = "FS_Grayscale";
constexpr char OUTLINE_TEXT_SHADER_NAME[] = "TextEdgeDetection";
constexpr char DAMAGE_NUMBER_SHADER_NAME[] = "DamageNumber";

constexpr float ORBIT_CAMERA_MIN_RADIUS = 10.0f;
constexpr float ORBIT_CAMERA_MAX_RADIUS = 20.0f;
constexpr float ORBIT_CAMERA_MIN_AZIMUTH_DEGREES = 10.0f;
constexpr float ORBIT_CAMERA_MAX_AZIMUTH_DEGREES = 45.0f;
constexpr float ORBIT_CAMERA_NEAR_Z = -500.0f;
constexpr float ORBIT_CAMERA_FAR_Z = 500.0f;
constexpr float ORBIT_CAMERA_ROTATION_SPEED_PER_SECOND = 75.0f;
constexpr float ORBIT_CAMERA_ZOOM_SPEED_PER_SECOND = 10.0f;
constexpr float ORBIT_CAMERA_LERP_TO_LOOK_AT_SECONDS = 0.4f;

constexpr float ENCOUNTER_MENU_RENDER_ALPHA = 0.5f;
constexpr float PAUSE_MENU_EFFECT_BLEND_TIME_SECONDS = 1.0f;

constexpr float ACTOR_STATUS_BOX_GAP_HEIGHT = 0.25f;
constexpr float ACTOR_STATUS_BOX_WIDTH = 3.0f;
constexpr float ACTOR_STATUS_BOX_HEIGHT = 2.0f;

constexpr float PROJECTILE_SIZE = 0.3f;

struct ShaderFloatUniform;
class NetMessage;

enum EncounterRenderFlag	: unsigned int
{
	RENDER_FLAG_MAP = 1,
	RENDER_FLAG_ACTORS = 2,
	RENDER_FLAG_TILES = 4,
	RENDER_FLAG_PROJECTILE = 8,
	RENDER_FLAG_MENU = 16,
	RENDER_FLAG_TURNINFO = 32,
	RENDER_FLAG_FLYOUT_TEXT = 64,
	RENDER_FLAG_VICTORY = 128,
	RENDER_FLAG_WAITING_FOR_OPPONENT = 256
};

enum EncounterNetConnectionState
{
	ENC_CONN_DISCONNECTED,
	ENC_CONN_CONNECTED,
	ENC_CONN_ERROR
};

struct FlyoutText
{

public:
	FlyoutText( const std::string& text, const Vector3& location, const Rgba& color, bool shouldPersist, float duration = 0.0f, ShaderProgram* shader = nullptr, float cellHeight = 0.25f );

public:
	std::string m_text = "";
	Vector3 m_location = Vector3::ZERO;
	Rgba m_color = Rgba::WHITE;
	float m_duration = 0.0f;
	ShaderProgram* m_shader = nullptr;
	std::vector< ShaderFloatUniform > m_shaderParams;
	float m_cellHeight = 0.0f;
	bool m_shouldPersist = false;
	bool m_enabled = true;

};

class EncounterGameState : public GameState
{

public:
	EncounterGameState();
	~EncounterGameState();

	void Enter() override;
	void Exit() override;
	void Update() override;
	void Render() const override;

	void PushDelayedAction( DelayedAction* action );
	void PushAction( Action* action );
	void PushActionTo( Action* action, int positionInQueue );
	void PushMode( EncounterMode* mode );
	void PopAction();
	void PopMode();
	void ClearActionQueue();
	void ClearModeStack();

	void SetCameraTargetLookAt( const Vector3& target, bool interpolate = true );
	void LoadEncounter( const EncounterDefinition& encounterDefinition );
	void SetNetworkingProperties( bool isNetworked, bool isHost );
	void SetRenderFlags( unsigned int bits );	// Resets render flags to the specified value
	void SetRenderFlag( EncounterRenderFlag flag );	// Sets a single flag over the existing value
	void RemoveRenderFlag( EncounterRenderFlag flag );	// Removes a single flag over the existing value
	void AdvanceTimeForActiveActions( int waitAmount );
	void SetCurrentCursorPosition( const IntVector2& cursorPosition );
	void SetSelectableTiles( const std::vector< IntVector2 >& selectableTiles );
	void SetProjectilePosition( const Vector3& position );
	void SetTurnNumber( unsigned int turnNumber );
	void SetCurrentActor( Actor* currentActor );
	void SetCurrentlySelectedAction( ActionType actionType );
	void AddFlyoutText( const std::string& key, const FlyoutText& flyoutText );
	void EnableFlyoutText( const std::string& key );
	void DisableFlyoutText( const std::string& key );
	void RemoveFlyoutTextIfPresent( const std::string& key );

	void SendActionMessage( NetMessage& message );
	void InvokeMoveAction( Actor* movingActor, const IntVector2& targetPos );
	void InvokeAttackAction( Actor* attackingActor, const IntVector2& targetPos, bool isBlocked, bool isCritical );
	void InvokeBowAction( Actor* firingActor, const IntVector2& targetPos, bool isBlocked, bool isCritical );
	void InvokeHealAction( Actor* healingActor, const IntVector2& targetPos );
	void InvokeDefendAction( Actor* defendingActor );
	void InvokeCastFireAction( Actor* firingActor, const IntVector2& targetPos );
	void InvokeWaitAction( Actor* waitingActor );

	Encounter* GetCurrentEncounter() const;
	Actor* GetCurrentActor() const;
	OrbitCamera* GetOrbitCamera() const;
	Clock* GetClock() const;
	unsigned int GetRenderFlags() const;
	bool IsActionQueueEmpty() const;
	bool IsModeStackEmpty() const;
	bool IsCurrentActorIdle() const;
	bool IsNetworked() const;
	bool IsConnected() const;
	bool IsInErrorState() const;
	bool CanClientPlayThisTurn( Actor* currentActor ) const;
	FlyoutText* GetFlyoutText( const std::string& key );

private:
	// Networking
	bool ShouldWaitForNetwork();
	void TrySendJoinAcceptMessage();
	bool TryHandleDisconnect();
	
	void HandlePauseInput();
	void HandleDebugInput();
	void MoveDelayedActionsToActionQueueIfReady();
	void UpdateLookAt();
	void UpdateActors();
	void UpdateFlyoutText();
	void RenderEncounterMenu() const;
	void RenderVisibleSelectableTilesAndCursor() const;
	void RenderHiddenSelectableTilesAndCursor() const;
	void RenderActors() const;
	void RenderHighlightedActorInfo() const;
	void RenderProjectile() const;
	void RenderVictoryScreen() const;
	void RenderFlyoutText() const;
	void RenderTurnInfo() const;
	void RenderWaitingForOpponentMessage() const;
	void RenderPauseEffectsAndMenu() const;
	void RenderNetSessionOverlay() const;
	void RenderWaitingForConnectionMessage() const;
	void RenderConnectionErrorMessage() const;
	void PrintToDebugUI( const std::string& text, const Vector2& textMins ) const;
	Rgba GetCurrentCursorColor( bool getHiddenColors = false ) const;

private:
	OrbitCamera* m_orbitCamera = nullptr;
	Camera* m_debugUICamera = nullptr;
	Clock* m_encounterClock = nullptr;
	Encounter* m_encounter = nullptr;
	std::deque< Action* > m_actionQueue;
	std::vector< DelayedAction* > m_delayedActionQueue;
	std::stack< EncounterMode* > m_modeStack;
	bool m_popModeNextFrame = false;
	Vector3 m_currentLookAt = Vector3::ZERO;
	Vector3 m_targetLookAt = Vector3::ZERO;
	float m_lookAtLerpSecondsElapsed = 0.0f;
	Sprite* m_projectileSprite = nullptr;

	// Dumb members to be externally set for rendering purposes
	Actor* m_currentActor = nullptr;
	IntVector2 m_currentCursorPosition = IntVector2::ZERO;
	Vector3 m_projectilePosition = Vector3::ZERO;
	std::vector< IntVector2 > m_selectableTiles;
	std::vector< ActionType > m_availableActions;
	ActionType m_currentlySelectedAction;
	unsigned int m_renderFlags = 0;
	unsigned int m_turnNumber = 0;
	std::map< std::string, FlyoutText* > m_flyoutTextByKey;

	// Networking
	bool m_isNetworked = false;
	bool m_isHost = false;
	bool m_sessionOverlayVisible = false;
	EncounterNetConnectionState m_connectionState = ENC_CONN_DISCONNECTED;
	uint8_t m_otherConnectionIndex = INVALID_CONNECTION_INDEX;

};

bool AddActorCommand( Command& command );
bool DamageActorCommand( Command& command );
bool KillActorCommand( Command& command );
