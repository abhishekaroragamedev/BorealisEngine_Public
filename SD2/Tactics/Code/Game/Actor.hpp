#pragma once

#include "Game/Action.hpp"
#include "Game/Utilities/IsometricSpriteAnimationSet.hpp"
#include "Game/World/Map.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Core/XmlUtilities.hpp"
#include "Engine/Math/IntVector2.hpp"
#include "Engine/Math/Vector3.hpp"
#include "ThirdParty/tinyXML2/tinyxml2.h"
#include <string>

constexpr int ACTOR_MIN_WAIT_TIME = 30;
constexpr int ACTOR_MOVE_WAIT_TIME = 40;
constexpr int ACTOR_ATTACK_WAIT_TIME = 60;
constexpr int ACTOR_BOW_WAIT_TIME = 60;
constexpr int ACTOR_CAST_WAIT_TIME = 50;
constexpr int ACTOR_DEFEND_WAIT_TIME = 40;
constexpr int ACTOR_HEAL_WAIT_TIME = 40;
constexpr float ACTOR_SIZE_WORLD_UNITS = 0.5f;
constexpr float ACTOR_BLOCK_CHANCE_BACK = 0.0f;
constexpr float ACTOR_BLOCK_CHANCE_SIDE = 0.05f;
constexpr float ACTOR_BLOCK_CHANCE_FRONT = 0.2f;
constexpr float ACTOR_CRIT_CHANCE_BACK = 0.3f;
constexpr float ACTOR_CRIT_CHANCE_SIDE = 0.15f;
constexpr float ACTOR_CRIT_CHANCE_FRONT = 0.05f;
constexpr float ACTOR_DEFEND_ACTION_AUGMENTED_BLOCK_CHANCE = 0.5f;

struct Stats
{

public:
	Stats();
	Stats( const Stats& copy );
	Stats( int maxHealth, int moveSpeed, int jumpHeight, int strength, int actionSpeed, int height );
	Stats( const tinyxml2::XMLElement& statsXML );
	int m_maxHealth = 0;
	int m_moveSpeed = 0;
	int m_jumpHeight = 0;
	int m_strength = 0;
	int m_actionSpeed = 0;
	int m_height = 0;

};

struct TurnBasedAction
{

public:
	TurnBasedAction( ActionType actionType );

public:
	ActionType m_actionType = ActionType::ACTION_TYPE_INVALID;
	bool m_performedThisTurn = false;

};

enum Team
{
	TEAM_INVALID = -1,
	TEAM_RED,
	TEAM_BLUE,
	NUM_TEAMS
};

std::string GetTeamNameFromTeam( Team team );
Team GetTeamFromTeamName( const std::string& teamName );

enum PlayerType
{
	PLAYER_HUMAN,
	PLAYER_AI
};

enum ActorClass
{
	ACTOR_CLASS_INVALID = -1,
	ACTOR_CLASS_KNIGHT,
	ACTOR_CLASS_ARCHER,
	ACTOR_CLASS_MAGE
};

class ActorDefinition
{

	friend class Actor;

public:
	ActorDefinition( const tinyxml2::XMLElement& actorDefinitionElement );
	~ActorDefinition();

	IsometricSpriteAnimationSetDefinition* GetIsometricSpriteAnimationSetDefinition() const;
	Stats GetBaseStats() const;
	ActorClass GetClass() const;
	std::vector< ActionType > GetAvailableActions() const;
	std::string GetName() const;

private:
	void PopulateFromXml( const tinyxml2::XMLElement& actorDefinitionElement );

public:
	static std::map< std::string, ActorDefinition* > s_definitions;

private:
	std::string m_name = "";
	ActorClass m_class = ActorClass::ACTOR_CLASS_INVALID;
	Stats m_baseStats;
	std::vector< ActionType > m_actionTypes;
	IsometricSpriteAnimationSetDefinition* m_isoSpriteAnimSetDefinition = nullptr;

};

class Actor
{

public:
	Actor( const ActorDefinition& definition, Team team, const std::string& name, Map* map, const IntVector2& position = IntVector2::ZERO );
	Actor( const std::string& actorDefinitionName, Team team, const std::string& name, Map* map, const IntVector2& position = IntVector2::ZERO );
	~Actor();

public:
	void Update();
	void Render();
	void SetDestination( const IntVector2& nextPosition );	// Set by MoveActions
	void LerpTo( const Vector3& initialPosition, const Vector3& nextPosition, float lerpAmount );
	void SetRenderPositionFromMapPosition();	// Sets m_renderPosition to the center of the tile located at m_position
	void SetStats( const Stats& stats );
	void SetWaitTime( int waitTime );
	void SetPosition( const IntVector2& position );
	void SetFacingDirection( const Vector3& facingDirection );
	void TurnToward( const IntVector2& position );
	void SetAugmentedBlockChance( float augmentedBlockChance );
	void SetTrackingFlyoutTextKey( const std::string& key );
	void SetLastAction( ActionType action );
	void Damage( int amount );
	void Heal( int amount );
	void AddWaitTime( int amount );
	void AdvanceTime( int amount );
	void DecayWait();
	void RefreshActionsForTurn();
	void EnableAction( ActionType type );
	void DisableAction( ActionType type );
	void DisableAllActionsButWait();
	void UndoLastWaitAddition();
	void MarkDead();

	bool IsDead() const;
	Team GetTeam() const;
	const ActorDefinition* GetDefinition() const;
	std::string GetName() const;
	Map* GetOwningMap() const;
	IntVector2 GetPosition() const;
	Vector3 GetRenderPosition() const;
	Vector3 GetHeightCorrectedPosition( const Vector3& position ) const;
	Stats GetStats() const;
	int GetHealth() const;
	int GetWaitTime() const;
	int GetTurnOrder() const;
	ActionType GetLastAction() const;
	Vector3 GetFacingDirection() const;
	Vector3 GetAssailantCardinalFacingDirection( const Vector3& assailantPosition ) const;
	IsometricSpriteAnimationSet* GetIsometricSpriteAnimationSet() const;
	Rgba GetDrawColor() const;
	std::string GetStatusString() const;
	float GetBlockChance( const Vector3& assailantFacingDirection ) const;
	bool IsAttackBlocked( const Vector3& assailantFacingDirection ) const;	// Assailant will always turn towards the Actor before attacking
	float GetCritChance( const Vector3& assailantFacingDirection ) const;
	bool IsAttackCritical( const Vector3& assailantFacingDirection ) const;	// Assailant will always turn towards the Defender before attacking
	bool IsActionAvailableThisTurn( ActionType type ) const;
	std::vector< ActionType > GetActions() const;
	ActionType GetNextAvailableAction( ActionType currentAction ) const;
	ActionType GetPreviousAvailableAction( ActionType currentAction ) const;

private:
	void InitializeActions();
	void UpdateTrackingFlyoutText();
	void StopDefending();
	int GetActionIndex( ActionType type ) const;				// Returns -1 if ActionType is not found

private:
	Team m_team = Team::TEAM_INVALID;
	std::string m_name = "";
	Map* m_owningMap = nullptr;
	IsometricSpriteAnimationSet* m_isometricSpriteAnimSet = nullptr;
	Stats m_stats;
	const ActorDefinition* m_definition = nullptr;
	std::vector< TurnBasedAction > m_actions;
	std::string m_trackingFlyoutTextKey = "";	// Each Actor can have one flyout text string follow it around

	IntVector2 m_position;
	Vector3 m_renderPosition;
	Vector3 m_facingDirection = Vector3::FORWARD;
	int m_health = 0;
	int m_waitTime = 0;
	bool m_isDead = false;
	float m_augmentedBlockChance = 0.0f;
	ActionType m_lastActionUsed = ActionType::ACTION_TYPE_INVALID;
	int m_lastWaitAmount = 0;

};
