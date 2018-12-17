#pragma once

#include "Game/Entities/ActorDefinition.hpp"
#include "Game/Entities/ItemDefinition.hpp"
#include "Game/Entities/Entity.hpp"
#include "Game/Entities/Stats.hpp"
#include "Engine/Math/Vector2.hpp"
#include <map>
#include <string>

class Item;

class Actor : public Entity
{

	friend class Map;

public:
	~Actor();

	void Update( float deltaSeconds ) override;
	virtual void Render( float renderAlpha, bool developerModeEnabled ) const override;
	void RenderHealthInfo( float renderAlpha ) const;
	virtual void Damage( int damageStats[ StatID::NUM_STATS ] );
	void ApplyKnockbackTranslation( const Vector2& knockback );
	bool IsDead() const;
	bool IsPlayer() const;
	bool CanAttack() const;
	ActorDefinition* GetActorDefinition() const;
	int GetStat( StatID statID ) const;
	void GetStats( int ( &out_statArray )[ StatID::NUM_STATS ] ) const;
	int GetBaseStat( StatID statID ) const;
	Vector2 GetFacingDirection() const;
	Item** GetEquipment();
	void AddStatModifiers( const int* statModifiers );
	void RemoveStatModifiers( const int* statModifiers );
	Item* GetItemAtIndex( int inventoryIndex );
	Item* GetEquippedItem( EquipSlot equipSlot );
	void AddItemToInventory( Item* itemToAdd );
	void DropItem( Item* itemToDrop );
	void EquipItem( Item* itemToEquip );
	void UnequipItem( Item* itemToUnequip );
	void UseItem( Item* itemToUse );
	bool IsAttacking() const;
	float GetFrictionPenalty( float initialSpeed, float deltaSeconds ) const;

protected:
	explicit Actor( const std::string instanceName, const Vector2& position, ActorDefinition* actorDefinition, const Map& map );

	void InitializeStatsFromDefinition();
	void InitializeInventoryFromDefinition();
	void CheckPlayerProximityForDialogue();
	void DoPhysicalDamage( int damageStrengthStat );
	void DoPoisonDamage( float deltaSeconds );
	void DoFireDamage( float deltaSeconds );
	void AddStatusEffect( int damageStats[ StatID::NUM_STATS ], StatID typeOfDamage );
	bool ShouldEvadeAttack() const;
	bool ShouldMeleeAttackSucceed() const;
	void SetFinalProjectileVelocityAfterAimStatComputation( Vector2& projectileVelocity );
	void UpdateAllStatusEffectInformation( float deltaSeconds );
	void UpdateStatusEffectDurationForTag( float deltaSeconds, const std::string& tagName );
	virtual void PerformAction();
	void SetAnimationState();
	std::string GetDirectionalAnimationNameBasedOnAttackingState( const std::string& movementAnimationName, const std::string& meleeAnimationName, const std::string& shootingAnimationName, const std::string& castingAnimationName );
	void Attack( EquipSlot slotToUse );
	virtual void HandleActorDeath();
	void TryAddItemAtCurrentLocationToInventory();
	void UpdateTimeBasedVariables( float deltaSeconds );
	void Translate( float deltaSeconds ) override;
	void ApplyFrictionOrStop( float deltaSeconds );
	void RenderEquipment( const Rgba& renderTint, float renderAlpha ) const;
	void RenderDamageOrAttackString( float renderAlpha, float secondsToConsider, const std::string& stringToRender ) const;

protected:
	float m_secondsSinceLastAttackAttempt = 9999.0f;			// To let actors fire arrows immediately
	float m_secondsSinceDirectionChange = 9999.0f;		// To make actors reorient themselves immediately
	float m_secondsSinceLastDamageAttempt = 0.0f;
	ActorDefinition* m_actorDefinition = nullptr;
	float m_speed = 0.0f;
	bool m_isBeingKnocked = false;
	int m_baseStats[ StatID::NUM_STATS ];
	int m_stats[ StatID::NUM_STATS ];
	std::string m_attackStringToRender = "";
	std::string m_damageStringToRender = "";
	Vector2 m_facingDirection = Vector2::DOWN;
	Item* m_equippedItems[ EquipSlot::NUM_SLOTS ];
	float m_hasPlayerComeWithinProximity = false;
	EquipSlot m_currentAttackSlot = EquipSlot::EQUIP_SLOT_NONE;
	std::map< std::string, float > m_statusEffectTagToRemainingDurationMap;

};
