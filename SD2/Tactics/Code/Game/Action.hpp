#pragma once

#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/IntRange.hpp"
#include "Engine/Math/IntVector2.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"
#include <string>
#include <vector>

constexpr int ACTOR_ATTACK_CRIT_MULTIPLIER = 2;
constexpr int HEAL_ACTION_WAIT = 100;
constexpr int FIRE_ACTION_WAIT = 150;
constexpr float ACTOR_MOVE_ANIMATION_BLOCKS_PER_SECOND = 2.0f;
constexpr float ACTOR_BOW_ACTION_GRAVITY_MAGNITUDE = 2.0f;
constexpr float ACTOR_FLYOUT_TEXT_RENDER_TIME_SECONDS = 2.0f;
constexpr float ACTOR_DEATH_ANIM_TIME_SECONDS = 2.0f;

class Actor;

enum ActionType
{
	ACTION_TYPE_INVALID = -1,
	WAIT,
	MOVE,
	ATTACK,
	BOW,
	DEFEND,
	HEAL,
	CAST_FIRE,
	DAMAGE_HEAL_NUMBER,
	DEATH
};

class Action
{

public:
	Action( Actor* actor, unsigned int waitTime = 0 );
	~Action();

	virtual void Init() = 0;
	virtual void Tick() = 0;
	virtual void DecayWait( int wait );

	std::string GetName() const;
	virtual ActionType GetType() const;
	bool CanTick() const;
	bool HasInitialized() const;

	static std::string GetName( ActionType actionType );
	static ActionType GetType( const std::string& actionName );

protected:
	Actor* m_actor = nullptr;
	unsigned int m_waitTime = 0;
	bool m_hasInitialized = false;

};

class DelayedAction : public Action
{

public:
	DelayedAction( Actor* actor, const IntVector2& actionPosition, const IntRange& allowedRange, unsigned int waitTime );
	~DelayedAction();

	virtual void Tick() override = 0;
	void DecayWait( int wait ) override;	// Calls Action::DecayWait(), but also tracks the Actor's position and last action

	virtual ActionType GetType() const override = 0;

protected:
	void FailAction();

	bool ActorOnlyMovedOrWaited() const;
	bool IsActorStillInRange() const;
	std::string GetFailedFlyoutTextKey() const;
	std::string GetFlyoutTextKey() const;	// Used to reference the flyout text spawned by the action

protected:
	IntVector2 m_actionPosition = IntVector2::ZERO;
	IntRange m_allowedRange;

};

class MoveAction : public Action
{

public:
	MoveAction( Actor* actor, const IntVector2& movePosition, const std::vector< IntVector2  >& path );
	~MoveAction();

	void Init() override;
	void Tick() override;

	ActionType GetType() const override;

private:
	void TryUpdateNextPointOnPath();
	Vector3 GetPathPointWorldCoordinates( size_t pointIndex );
	bool IsMoveComplete() const;

private:
	IntVector2 m_movePosition = IntVector2::ZERO;
	std::vector< IntVector2 > m_path;
	size_t m_currentPathPoint = 0;
	float m_pathProgress = 0.0f;

};

class AttackAction : public Action
{

public:
	AttackAction( Actor* actor, const IntVector2& attackPosition, bool isBlocked, bool isCritical );
	~AttackAction();

	void Init() override;
	void Tick() override;

	ActionType GetType() const override;

private:
	int GetDamage() const;
	Actor* GetTargetActor() const;
	bool HasActorStoppedAnimation( const Actor* actor ) const;
	std::string GetBlockFlyoutTextKey() const;
	std::string GetCritFlyoutTextKey() const;

private:
	IntVector2 m_attackPosition = IntVector2::ZERO;
	bool m_blocked = false;
	bool m_critical = false;
	int m_critMultiplier = 1;

};

class BowAction : public Action
{

public:
	BowAction( Actor* actor, const IntVector2& attackPosition, bool isBlocked, bool isCritical );
	~BowAction();

	void Init() override;
	void Tick() override;

	ActionType GetType() const override;

private:
	void ComputeLaunchVelocity();
	void TryDamageTargetActor();
	void AddMissFlyoutText();

	bool HasActorStoppedBowAnimation() const;
	int GetDamage() const;
	Actor* GetTargetActor() const;
	std::string GetBlockFlyoutTextKey() const;
	std::string GetCritFlyoutTextKey() const;
	std::string GetMissFlyoutTextKey() const;

private:
	IntVector2 m_attackPosition = IntVector2::ZERO;
	bool m_blocked = false;
	bool m_critical = false;
	int m_critMultiplier = 1;
	Vector2 m_launchVelocity = Vector2::ZERO;
	Vector2 m_launchDirection = Vector2::ZERO;
	float m_bowProgress = 0.0f;
	bool m_hit = false;
};

class DefendAction : public Action
{

public:
	DefendAction( Actor* actor, float augmentedBlockChance );
	~DefendAction();

	void Init() override;
	void Tick() override;

	ActionType GetType() const override;

	std::string GetFlyoutTextKey() const;

private:
	float m_augmentedBlockChance = 0.0f;

};

class HealAction : public DelayedAction
{

public:
	HealAction( Actor* actor, const IntVector2& healPosition );
	~HealAction();

	void Init() override;
	void Tick() override;

	ActionType GetType() const override;

private:
	int GetHealth() const;
	Actor* GetTargetActor() const;

private:
	IntVector2 m_healPosition = IntVector2::ZERO;

};

class CastFireAction : public DelayedAction
{

public:
	CastFireAction( Actor* actor, const IntVector2& castFirePosition );
	~CastFireAction();

	void Init() override;
	void Tick() override;

	ActionType GetType() const override;

private:
	int GetDamage() const;
	std::vector< Actor* > GetTargetActors() const;

private:
	IntVector2 m_castFirePosition = IntVector2::ZERO;

};

class WaitAction : public Action
{

public:
	WaitAction( Actor* actor );
	~WaitAction();

	void Init() override;
	void Tick() override;

	ActionType GetType() const override;

};

class DamageHealNumberAction : public Action
{

public:
	DamageHealNumberAction( Actor* damagedActor, int damageAmount, const Rgba renderColor = Rgba::RED );
	~DamageHealNumberAction();

	void Init() override;
	void Tick() override;

	ActionType GetType() const override;

private:
	std::string GetFlyoutTextKey() const;
	Rgba m_renderColor = Rgba::RED;

private:
	int m_damage = 0;
	float m_remainingAnimationTime = ACTOR_FLYOUT_TEXT_RENDER_TIME_SECONDS;

};

class DeathAction : public Action
{

public:
	DeathAction( Actor* deadActor );
	~DeathAction();

	void Init() override;
	void Tick() override;
	
	ActionType GetType() const override;

private:
	std::string GetFlyoutTextKey() const;

private:
	float m_remainingAnimationTime = ACTOR_DEATH_ANIM_TIME_SECONDS;

};
