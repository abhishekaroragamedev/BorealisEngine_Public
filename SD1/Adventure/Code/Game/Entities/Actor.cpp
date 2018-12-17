#include "Game/GameCommon.hpp"
#include "Game/Entities/Actor.hpp"
#include "Game/Entities/Item.hpp"
#include "Game/Entities/Player.hpp"
#include "Game/Entities/ProjectileDefinition.hpp"
#include "Engine/Math/MathUtils.hpp"

Actor::Actor( const std::string instanceName, const Vector2& position, ActorDefinition* actorDefinition, const Map& map )
	:	Entity( instanceName, position, actorDefinition, map ),
		m_actorDefinition( actorDefinition )
{
	InitializeStatsFromDefinition();
	InitializeInventoryFromDefinition();
	if ( m_actorDefinition->GetDialogue( ActorDialogueType::ACTOR_DIALOGUE_TYPE_SPAWN ) != nullptr )
	{
		g_theGame->PushDialogue( m_actorDefinition->GetDialogue( ActorDialogueType::ACTOR_DIALOGUE_TYPE_SPAWN ) );
	}
}

Actor::~Actor()
{

}

void Actor::InitializeStatsFromDefinition()
{
	m_baseStats[ StatID::STAT_HEALTH ] =  GetRandomIntInRange( m_actorDefinition->GetStatRange( StatID::STAT_HEALTH ) );
	m_baseStats[ StatID::STAT_STRENGTH ] = GetRandomIntInRange( m_actorDefinition->GetStatRange( StatID::STAT_STRENGTH ) );
	m_baseStats[ StatID::STAT_RESILIENCE ] = GetRandomIntInRange( m_actorDefinition->GetStatRange( StatID::STAT_RESILIENCE ) );
	m_baseStats[ StatID::STAT_AGILITY ] = GetRandomIntInRange( m_actorDefinition->GetStatRange( StatID::STAT_AGILITY ) );
	m_baseStats[ StatID::STAT_DEXTERITY ] = GetRandomIntInRange( m_actorDefinition->GetStatRange( StatID::STAT_DEXTERITY ) );
	m_baseStats[ StatID::STAT_AIM ] =  GetRandomIntInRange( m_actorDefinition->GetStatRange( StatID::STAT_AIM ) );
	m_baseStats[ StatID::STAT_ACCURACY ] = GetRandomIntInRange( m_actorDefinition->GetStatRange( StatID::STAT_ACCURACY ) );
	m_baseStats[ StatID::STAT_EVASIVENESS ] = GetRandomIntInRange( m_actorDefinition->GetStatRange( StatID::STAT_EVASIVENESS ) );
	m_baseStats[ StatID::STAT_POISON_USAGE ] = GetRandomIntInRange( m_actorDefinition->GetStatRange( StatID::STAT_POISON_USAGE ) );
	m_baseStats[ StatID::STAT_FIRE_USAGE ] = GetRandomIntInRange( m_actorDefinition->GetStatRange( StatID::STAT_FIRE_USAGE ) );
	m_baseStats[ StatID::STAT_ICE_USAGE ] = GetRandomIntInRange( m_actorDefinition->GetStatRange( StatID::STAT_ICE_USAGE ) );
	m_baseStats[ StatID::STAT_ELECTRICITY_USAGE ] = GetRandomIntInRange( m_actorDefinition->GetStatRange( StatID::STAT_ELECTRICITY_USAGE ) );

	m_stats[ StatID::STAT_HEALTH ] = m_baseStats[ StatID::STAT_HEALTH ];
	m_stats[ StatID::STAT_STRENGTH ] = m_baseStats[ StatID::STAT_STRENGTH ];
	m_stats[ StatID::STAT_RESILIENCE ] = m_baseStats[ StatID::STAT_RESILIENCE ];
	m_stats[ StatID::STAT_AGILITY ] = m_baseStats[ StatID::STAT_AGILITY ];
	m_stats[ StatID::STAT_DEXTERITY ] = m_baseStats[ StatID::STAT_DEXTERITY ];
	m_stats[ StatID::STAT_AIM ] =  m_baseStats[ StatID::STAT_AIM ];
	m_stats[ StatID::STAT_ACCURACY ] = m_baseStats[ StatID::STAT_ACCURACY ];
	m_stats[ StatID::STAT_EVASIVENESS ] = m_baseStats[ StatID::STAT_EVASIVENESS ];
	m_stats[ StatID::STAT_POISON_USAGE ] = m_baseStats[ StatID::STAT_POISON_USAGE ];
	m_stats[ StatID::STAT_FIRE_USAGE ] = m_baseStats[ StatID::STAT_FIRE_USAGE ];
	m_stats[ StatID::STAT_ICE_USAGE ] = m_baseStats[ StatID::STAT_ICE_USAGE ];
	m_stats[ StatID::STAT_ELECTRICITY_USAGE ] = m_baseStats[ StatID::STAT_ELECTRICITY_USAGE ];
}

void Actor::InitializeInventoryFromDefinition()
{
	for ( ActorDefaultItem defaultItem : m_actorDefinition->GetDefaultItems() )
	{
		if ( CheckRandomChance( defaultItem.m_chanceOfSpawning ) )
		{
			Item* newItem = m_map->SpawnItem( defaultItem.m_defaultItemDefinition, *m_map->GetTileAtWorldPosition( m_position ).GetTileDefinition() );
			AddItemToInventory( newItem );
		}
	}
}

void Actor::Damage( int damageStats[ StatID::NUM_STATS ] )
{
	int healthBeforeDamage = GetStat( StatID::STAT_HEALTH );

	if ( !ShouldEvadeAttack() )
	{
		DoPhysicalDamage( damageStats[ StatID::STAT_STRENGTH ] );
		AddStatusEffect( damageStats, StatID::STAT_POISON_USAGE );
		AddStatusEffect( damageStats, StatID::STAT_FIRE_USAGE );
		AddStatusEffect( damageStats, StatID::STAT_ICE_USAGE );
		AddStatusEffect( damageStats, StatID::STAT_ELECTRICITY_USAGE );
	}

	if ( healthBeforeDamage != 0 )
	{
		int healthDifference = healthBeforeDamage - GetStat( StatID::STAT_HEALTH );
		if ( healthDifference == 0 )
		{
			m_damageStringToRender = "Miss";
		}
		else
		{
			m_damageStringToRender = std::to_string( healthDifference );
		}

		if ( GetStat( StatID::STAT_HEALTH ) == 0 )
		{
			HandleActorDeath();
		}
	}
	else
	{
		m_damageStringToRender = "Dead";
	}
	m_secondsSinceLastDamageAttempt = 0.0f;
}

bool Actor::ShouldEvadeAttack() const
{
	float chanceToEvade = g_gameConfigBlackboard.GetValue( "evasivenessBestChance", 0.5f );
	if ( GetStat( StatID::STAT_EVASIVENESS ) > 0 )
	{
		chanceToEvade -= chanceToEvade / static_cast< float >( GetStat( StatID::STAT_EVASIVENESS ) );
	}
	return CheckRandomChance( chanceToEvade );
}

bool Actor::ShouldMeleeAttackSucceed() const
{
	if ( m_currentAttackSlot == EquipSlot::EQUIP_SLOT_SPELL )
	{
		return false;
	}

	float chanceToAttack = g_gameConfigBlackboard.GetValue( "meleeAccuracyWorstChance", 0.25f );
	if ( GetStat( StatID::STAT_ACCURACY ) > 0 )
	{
		float remainingChance = 1.0f - chanceToAttack;
		float remainingChanceAfterAccuracy = remainingChance - ( remainingChance / static_cast< float >( GetStat( StatID::STAT_ACCURACY ) ) );
		chanceToAttack += remainingChanceAfterAccuracy;
	}
	return CheckRandomChance( chanceToAttack );
}

void Actor::DoPhysicalDamage( int damageStrengthStat )
{
	if ( damageStrengthStat <= 0 || IsDead() )
	{
		return;
	}

	if ( GetStat( StatID::STAT_RESILIENCE ) > 0 )
	{
		int damageDone = static_cast< int >( static_cast< float >( damageStrengthStat ) / static_cast< float >( GetStat( StatID::STAT_RESILIENCE ) ) );
		if ( damageDone == 0 )	// Do some bare minimum damage
		{
			damageDone = 1;
		}
		m_stats[ StatID::STAT_HEALTH ] -= damageDone;
		m_stats[ StatID::STAT_HEALTH ] = ClampInt( GetStat( StatID::STAT_HEALTH ), 0, GetBaseStat( StatID::STAT_HEALTH ) );
	}
	else
	{
		m_stats[ StatID::STAT_HEALTH ] = 0;
	}
}

void Actor::DoPoisonDamage( float deltaSeconds )
{  
	if ( IsDead() )
	{
		return;
	}

	static float s_secondsSinceLastPoisonDamage = 0.0f;
	s_secondsSinceLastPoisonDamage += deltaSeconds;

	if ( s_secondsSinceLastPoisonDamage > 1.0f )
	{
		if ( m_tags.HasTags( Entity::GetNameForStatID( StatID::STAT_POISON_USAGE ) ) )
		{
			m_stats[ StatID::STAT_HEALTH ] -= g_gameConfigBlackboard.GetValue( "poisonHealthDropPerSecond", 10 );
			m_stats[ StatID::STAT_HEALTH ] = ClampInt( GetStat( StatID::STAT_HEALTH ), 0, GetBaseStat( StatID::STAT_HEALTH ) );
			m_damageStringToRender = std::to_string( g_gameConfigBlackboard.GetValue( "poisonHealthDropPerSecond", 10 ) );
			m_secondsSinceLastDamageAttempt = 0.0f;
		}
		s_secondsSinceLastPoisonDamage = 0.0f;
	}
	
}

void Actor::DoFireDamage( float deltaSeconds )
{  
	if ( IsDead() )
	{
		return;
	}

	static float s_secondsSinceLastFireDamage = 0.0f;
	s_secondsSinceLastFireDamage += deltaSeconds;

	if ( s_secondsSinceLastFireDamage > 1.0f )
	{
		if ( m_tags.HasTags( Entity::GetNameForStatID( StatID::STAT_FIRE_USAGE ) ) )
		{
			m_stats[ StatID::STAT_HEALTH ] -= g_gameConfigBlackboard.GetValue( "fireHealthDropPerSecond", 10 );
			m_stats[ StatID::STAT_HEALTH ] = ClampInt( GetStat( StatID::STAT_HEALTH ), 0, GetBaseStat( StatID::STAT_HEALTH ) );
			m_damageStringToRender = std::to_string( g_gameConfigBlackboard.GetValue( "fireHealthDropPerSecond", 10 ) );
			m_secondsSinceLastDamageAttempt = 0.0f;
		}
		s_secondsSinceLastFireDamage = 0.0f;
	}

}

void Actor::AddStatusEffect( int damageStats[ StatID::NUM_STATS ], StatID typeOfDamage )
{
	if ( damageStats[ typeOfDamage ] <= 0 || IsDead() )
	{
		return;
	}

	float chanceOfEffect = g_gameConfigBlackboard.GetValue( "maxStatusEffectChance", 0.5f );

	if ( GetStat( typeOfDamage ) > 0 )
	{
		chanceOfEffect *= static_cast< float >( damageStats[ typeOfDamage ] ) / static_cast< float >( GetStat( typeOfDamage ) );
		chanceOfEffect = ClampFloat( chanceOfEffect, 0.0f, g_gameConfigBlackboard.GetValue( "maxStatusEffectChance", 0.5f ) );
	}

	if ( CheckRandomChance( chanceOfEffect ) )
	{
		m_tags.SetOrRemoveTags( Entity::GetNameForStatID( typeOfDamage ) );
	}
}

void Actor::HandleActorDeath()
{
	// Drop droppable items in inventory
	for ( int slotIndex = 0; slotIndex < EquipSlot::NUM_SLOTS; slotIndex++ )
	{
		if ( m_equippedItems[ slotIndex ] != nullptr )
		{
			UnequipItem( GetEquippedItem( EquipSlot( slotIndex ) ) );
		}
	}
	if ( m_inventory.size() > 0 )
	{
		std::vector< Item* > droppableItems;
		std::vector< Item* > undroppableItems;
		
		for ( std::vector< Item* >::iterator itemIterator = m_inventory.begin(); itemIterator != m_inventory.end(); itemIterator++ )
		{
			if ( ( *itemIterator )->GetItemDefinition()->CanBeDropped() )
			{
				droppableItems.push_back( *itemIterator );
			}
			else
			{
				undroppableItems.push_back( *itemIterator );
			}
		}

		for ( std::vector< Item* >::iterator droppableItemIterator = droppableItems.begin(); droppableItemIterator != droppableItems.end(); droppableItemIterator++ )
		{
			DropItem( *droppableItemIterator );
		}
		for ( std::vector< Item* >::iterator undroppableItemIterator = undroppableItems.begin(); undroppableItemIterator != undroppableItems.end(); undroppableItemIterator++ )
		{
			m_map->DeleteItemPermanently( *undroppableItemIterator );
		}

		m_inventory.clear();
	}

	// Drop extra loot
	for ( ActorLoot actorLoot : m_actorDefinition->GetLootTypes() )
	{
		if ( CheckRandomChance( actorLoot.m_chanceOfSpawning ) )
		{
			std::vector< ItemDefinition* > spawnableItemDefinitions = actorLoot.m_lootDefinition->GetItemDefinitions();		// Assumes all Items specified in Loot are droppable/overrides their non-droppable nature
			if ( spawnableItemDefinitions.size() > 0 )
			{
				int spawnableItemIndex = GetRandomIntInRange( 0, ( spawnableItemDefinitions.size() - 1 ) );
				ItemDefinition* typeOfItemToSpawn = spawnableItemDefinitions[ spawnableItemIndex ];
				m_map->SpawnItem( typeOfItemToSpawn, m_position );
			}
		}
	}

	if ( m_actorDefinition->GetDialogue( ActorDialogueType::ACTOR_DIALOGUE_TYPE_DEATH ) != nullptr )
	{
		g_theGame->PushDialogue( m_actorDefinition->GetDialogue( ActorDialogueType::ACTOR_DIALOGUE_TYPE_DEATH ) );
	}
}

ActorDefinition* Actor::GetActorDefinition() const
{
	return m_actorDefinition;
}

bool Actor::IsDead() const
{
	return ( GetStat( StatID::STAT_HEALTH ) ==	0 );
}

bool Actor::IsPlayer() const
{
	if ( g_theGame->GetCurrentAdventure() == nullptr || g_theGame->GetCurrentAdventure()->GetCurrentMap() == nullptr )		// This happens before the game is initialized
	{
		return false;
	}
	return ( this == static_cast< Actor* >( g_theGame->GetCurrentAdventure()->GetCurrentMap()->GetPlayerEntity() ) );
}

int Actor::GetStat( StatID statID ) const
{
	int statValue = m_stats[ statID ];

	if ( statID == StatID::STAT_STRENGTH && m_tags.HasTags( Entity::GetNameForStatID( StatID::STAT_FIRE_USAGE ) ) )
	{
		statValue = static_cast< int >( static_cast< float >( statValue ) * g_gameConfigBlackboard.GetValue( "fireStrengthPenaltyMultiplier", 0.5f ) );
	}
	else if ( statID == StatID::STAT_DEXTERITY && m_tags.HasTags( Entity::GetNameForStatID( StatID::STAT_ICE_USAGE ) ) )
	{
		statValue = static_cast< int >( static_cast< float >( statValue ) * g_gameConfigBlackboard.GetValue( "iceDexterityPenaltyMultiplier", 0.5f ) );
	}
	else if ( statID == StatID::STAT_AGILITY && m_tags.HasTags( Entity::GetNameForStatID( StatID::STAT_ELECTRICITY_USAGE ) ) )
	{
		statValue = static_cast< int >( static_cast< float >( statValue ) * g_gameConfigBlackboard.GetValue( "electricityAgilityPenaltyMultiplier", 0.5f ) );
	}

	return statValue;
}

void Actor::GetStats( int ( &out_statArray )[ StatID::NUM_STATS ] ) const
{
	for ( int statID = 0; statID < StatID::NUM_STATS; statID++ )
	{
		out_statArray[ statID ] = GetStat( StatID( statID ) );
	}
}

int Actor::GetBaseStat( StatID statID ) const
{
	return m_baseStats[ statID ];
}

Vector2 Actor::GetFacingDirection() const
{
	return m_facingDirection;
}

Item** Actor::GetEquipment()
{
	return m_equippedItems;
}

bool Actor::IsAttacking() const
{
	return ( m_currentAttackSlot != EquipSlot::EQUIP_SLOT_NONE );
}

float Actor::GetFrictionPenalty( float initialSpeed, float deltaSeconds ) const
{
	float frictionMultiplierFromTile = m_map->GetTileAtWorldPosition( m_position ).GetTileDefinition()->GetFrictionMultiplier();
	float frictionMultiplierFromPlayer = m_entityDefinition->GetFrictionPerSecond() * deltaSeconds;
	float totalFrictionMultiplier = frictionMultiplierFromTile * frictionMultiplierFromPlayer;
	float totalFrictionPenalty = static_cast< float >( initialSpeed ) * totalFrictionMultiplier;
	return ClampFloat( totalFrictionPenalty, 0.0f, initialSpeed );
}

void Actor::AddStatModifiers( const int* statModifiers )
{
	for ( int statIDIndex = 0; statIDIndex < StatID::NUM_STATS; statIDIndex++ )
	{
		m_stats[ statIDIndex ] += statModifiers[ statIDIndex ];
	}
	if ( GetStat( StatID::STAT_HEALTH ) > GetBaseStat( StatID::STAT_HEALTH ) )
	{
		m_stats[ StatID::STAT_HEALTH ] = GetBaseStat( StatID::STAT_HEALTH );
	}
}

void Actor::RemoveStatModifiers( const int* statModifiers )
{
	for ( int statIDIndex = 0; statIDIndex < StatID::NUM_STATS; statIDIndex++ )
	{
		m_stats[ statIDIndex ] -= statModifiers[ statIDIndex ];
	}
}

void Actor::TryAddItemAtCurrentLocationToInventory()
{
	std::vector< Item* > itemsInContactWithActor = m_map->GetItemsInContactWithActor( *this );

	if ( itemsInContactWithActor.size() > 0 )
	{
		if ( IsPlayer() )
		{
			g_theGame->PushDialogue( itemsInContactWithActor[ 0 ]->GetItemDefinition()->GetDialogue( ItemDialogueType::ITEM_DIALOGUE_TYPE_TAKE ) );
		}
		AddItemToInventory( itemsInContactWithActor[ 0 ] ); // Only add one item at a time; the player needs to click again to add another item
	}
}

void Actor::AddItemToInventory( Item* itemToAdd )
{
	itemToAdd->AddToInventory( this );
	m_inventory.push_back( itemToAdd );

	if ( m_tags.HasTags( itemToAdd->GetItemDefinition()->GetActorNeedsTags() ) )
	{
		EquipSlot itemEquipSlot = itemToAdd->GetItemDefinition()->GetEquipSlot();
		if ( itemEquipSlot != EquipSlot::NUM_SLOTS && m_equippedItems[ itemEquipSlot ] == nullptr )		// Equip the item right away if the slot is free and the actor can equip it
		{
			EquipItem( itemToAdd );
		}
	}
}

Item* Actor::GetItemAtIndex( int inventoryIndex )
{
	if ( inventoryIndex > ( static_cast< int >( m_inventory.size() ) - 1 ) )
	{
		return nullptr;
	}
	return m_inventory[ inventoryIndex ];
}

Item* Actor::GetEquippedItem( EquipSlot equipSlot )
{
	return m_equippedItems[ equipSlot ];
}

void Actor::DropItem( Item* itemToDrop )
{
	if ( itemToDrop->GetItemDefinition()->GetEquipSlot() != EquipSlot::EQUIP_SLOT_NONE && m_equippedItems[ itemToDrop->GetItemDefinition()->GetEquipSlot() ] == itemToDrop )
	{
		UnequipItem( itemToDrop );
	}

	std::vector< Item* >::iterator itemIterator = std::find( m_inventory.begin(), m_inventory.end(), itemToDrop );
	if ( itemIterator != m_inventory.end() )
	{
		if ( IsPlayer() )
		{
			g_theGame->PushDialogue( itemToDrop->GetItemDefinition()->GetDialogue( ItemDialogueType::ITEM_DIALOGUE_TYPE_DROP ) );
		}
		itemToDrop->RemoveFromInventory( m_position, m_map );
		m_inventory.erase( itemIterator );
	}
}

void Actor::EquipItem( Item* itemToEquip )
{
	std::vector< Item* >::iterator itemIterator = std::find( m_inventory.begin(), m_inventory.end(), itemToEquip );
	if ( itemToEquip->GetItemDefinition()->GetEquipSlot() != EquipSlot::EQUIP_SLOT_NONE && itemIterator != m_inventory.end() && m_tags.HasTags( itemToEquip->GetItemDefinition()->GetActorNeedsTags() ) )
	{
		if ( IsPlayer() )
		{
			g_theGame->PushDialogue( itemToEquip->GetItemDefinition()->GetDialogue( ItemDialogueType::ITEM_DIALOGUE_TYPE_EQUIP ) );
		}
		Item* itemAlreadyInSlot = m_equippedItems[ itemToEquip->GetItemDefinition()->GetEquipSlot() ];
		m_equippedItems[ itemToEquip->GetItemDefinition()->GetEquipSlot() ] = itemToEquip;
		m_inventory.erase( itemIterator );
		if ( itemAlreadyInSlot != nullptr )
		{
			m_inventory.push_back( itemAlreadyInSlot );
			RemoveStatModifiers( itemAlreadyInSlot->GetItemDefinition()->GetStatModifiers() );
		}
		AddStatModifiers( itemToEquip->GetItemDefinition()->GetStatModifiers() );
	}
}

void Actor::UnequipItem( Item* itemToUnequip )
{
	if ( itemToUnequip->GetItemDefinition()->GetEquipSlot() != EquipSlot::EQUIP_SLOT_NONE && m_equippedItems[ itemToUnequip->GetItemDefinition()->GetEquipSlot() ] == itemToUnequip )
	{
		if ( IsPlayer() )
		{
			g_theGame->PushDialogue( itemToUnequip->GetItemDefinition()->GetDialogue( ItemDialogueType::ITEM_DIALOGUE_TYPE_UNEQUIP ) );
		}
		m_equippedItems[ itemToUnequip->GetItemDefinition()->GetEquipSlot() ] = nullptr;
		m_inventory.push_back( itemToUnequip );
		RemoveStatModifiers( itemToUnequip->GetItemDefinition()->GetStatModifiers() );
	}
}

void Actor::UseItem( Item* itemToUse )
{
	if ( itemToUse->GetItemDefinition()->GetEquipSlot() == EquipSlot::EQUIP_SLOT_NONE && m_tags.HasTags( itemToUse->GetItemDefinition()->GetUsageTags() ) )
	{
		if ( IsPlayer() )
		{
			g_theGame->PushDialogue( itemToUse->GetItemDefinition()->GetDialogue( ItemDialogueType::ITEM_DIALOGUE_TYPE_USE ) );
		}
		AddStatModifiers( itemToUse->GetItemDefinition()->GetStatModifiers() );
		std::vector< Item* >::iterator itemIterator = std::find( m_inventory.begin(), m_inventory.end(), itemToUse );
		if ( itemIterator != m_inventory.end() )
		{
			itemToUse->RemoveFromInventory( m_position, m_map );
			m_inventory.erase( itemIterator );
		}
		m_map->DeleteItemPermanently( itemToUse );
	}
}

void Actor::Update( float deltaSeconds )
{
	CheckPlayerProximityForDialogue();
	UpdateTimeBasedVariables( deltaSeconds );
	UpdateAllStatusEffectInformation( deltaSeconds );
	if ( !IsDead() )
	{
		PerformAction();
	}

	Translate( deltaSeconds );
	Rotate( deltaSeconds );
	SetAnimationState();
	if ( !IsDead() )
	{
		if ( !IsAttacking() )
		{
			m_spriteAnimationSet->GetCurrentSpriteAnimation()->Update( static_cast< float >( ( StatID::STAT_AGILITY ) ) * deltaSeconds );
		}
		else
		{
			m_spriteAnimationSet->GetCurrentSpriteAnimation()->Update( static_cast< float >( GetStat( StatID::STAT_DEXTERITY ) ) * deltaSeconds );
		}
	}
}

void Actor::UpdateTimeBasedVariables( float deltaSeconds )
{
	m_secondsSinceLastDamageAttempt += deltaSeconds;
	if ( m_secondsSinceLastDamageAttempt > g_gameConfigBlackboard.GetValue( "actorSecondsToShowAttackText", 0.5f ) )
	{
		m_damageStringToRender = "";
	}

	m_secondsSinceLastAttackAttempt += deltaSeconds;
	if ( m_secondsSinceLastAttackAttempt > g_gameConfigBlackboard.GetValue( "actorSecondsToShowAttackText", 0.5f ) )
	{
		m_attackStringToRender = "";
	}
	m_secondsSinceDirectionChange += deltaSeconds;
}

void Actor::UpdateAllStatusEffectInformation( float deltaSeconds )
{
	if ( GetStat( StatID::STAT_HEALTH ) < GetBaseStat( StatID::STAT_HEALTH ) )
	{
		m_tags.SetOrRemoveTags( "damaged" );
	}
	else
	{
		m_tags.SetOrRemoveTags( "!damaged" );
	}

	UpdateStatusEffectDurationForTag( deltaSeconds, "Poison" );
	UpdateStatusEffectDurationForTag( deltaSeconds, "Fire" );
	UpdateStatusEffectDurationForTag( deltaSeconds, "Ice" );
	UpdateStatusEffectDurationForTag( deltaSeconds, "Electricity" );

	if ( m_tags.HasTags( Entity::GetNameForStatID( StatID::STAT_POISON_USAGE ) ) )
	{
		DoPoisonDamage( deltaSeconds );
	}
	if ( m_tags.HasTags( Entity::GetNameForStatID( StatID::STAT_FIRE_USAGE ) ) )
	{
		DoFireDamage( deltaSeconds );
	}
}

void Actor::UpdateStatusEffectDurationForTag( float deltaSeconds, const std::string& tagName )
{
	float maxStatusEffectDuration = g_gameConfigBlackboard.GetValue( "maxStatusEffectDurationSeconds", 5.0f );

	if ( m_tags.HasTags( tagName ) )
	{
		if ( m_statusEffectTagToRemainingDurationMap.find( tagName ) == m_statusEffectTagToRemainingDurationMap.end() )
		{
			m_statusEffectTagToRemainingDurationMap[ tagName ] = maxStatusEffectDuration;

			StatID statusEffectResistanceStat = Entity::GetStatIDForName( tagName );
			if ( GetStat( statusEffectResistanceStat ) > 0 )
			{
				m_statusEffectTagToRemainingDurationMap[ tagName ] = maxStatusEffectDuration - ( maxStatusEffectDuration / GetStat( statusEffectResistanceStat ) );
			}
		}
		else
		{
			m_statusEffectTagToRemainingDurationMap[ tagName ] -= deltaSeconds;
			if ( m_statusEffectTagToRemainingDurationMap[ tagName ] <= 0.0f )
			{
				m_tags.SetOrRemoveTags( "!" + tagName );
				m_statusEffectTagToRemainingDurationMap.erase( tagName );
			}
		}
	}
}

void Actor::SetAnimationState()
{
	if ( IsAttacking() && m_spriteAnimationSet->GetCurrentSpriteAnimation()->IsFinished() )
	{
		m_currentAttackSlot = EquipSlot::EQUIP_SLOT_NONE;
		m_spriteAnimationSet->GetCurrentSpriteAnimation()->Reset();
	}

	Vector2 actorViewCardinalDirection = GetStrongestCardinalDirection( m_facingDirection );
	std::string currentAnimationName = m_spriteAnimationSet->GetCurrentSpriteAnimation()->GetDefinition()->GetName();
	std::string requiredAnimationName = "";

	if ( actorViewCardinalDirection == Vector2::UP )
	{
		requiredAnimationName = GetDirectionalAnimationNameBasedOnAttackingState( "MoveNorth", "MeleeNorth", "ShootNorth", "CastNorth" );
	}
	else if ( actorViewCardinalDirection == Vector2::DOWN )
	{
		requiredAnimationName = GetDirectionalAnimationNameBasedOnAttackingState( "MoveSouth", "MeleeSouth", "ShootSouth", "CastSouth" );
	}
	else if ( actorViewCardinalDirection == Vector2::RIGHT )
	{
		requiredAnimationName = GetDirectionalAnimationNameBasedOnAttackingState( "MoveEast", "MeleeEast", "ShootEast", "CastEast" );
	}
	else if ( actorViewCardinalDirection == Vector2::LEFT )
	{
		requiredAnimationName = GetDirectionalAnimationNameBasedOnAttackingState( "MoveWest", "MeleeWest", "ShootWest", "CastWest" );
	}
	else
	{
		requiredAnimationName = GetDirectionalAnimationNameBasedOnAttackingState( "Idle", "MeleeSouth", "ShootSouth", "CastSouth" );
	}

	if ( requiredAnimationName != "" && requiredAnimationName != currentAnimationName )
	{
		m_spriteAnimationSet->SetCurrentSpriteAnimation( requiredAnimationName );
	}
}

std::string Actor::GetDirectionalAnimationNameBasedOnAttackingState( const std::string& movementAnimationName, const std::string& meleeAnimationName, const std::string& shootingAnimationName, const std::string& castingAnimationName )
{
	std::string requiredAnimationName = "";

	if ( !IsAttacking() )
	{
		requiredAnimationName = movementAnimationName;
	}
	else
	{
		if ( m_currentAttackSlot == EquipSlot::EQUIP_SLOT_SPELL )
		{
			requiredAnimationName = castingAnimationName;
		}
		else if ( m_currentAttackSlot == EquipSlot::EQUIP_SLOT_WEAPON )
		{
			if ( m_equippedItems[ EquipSlot::EQUIP_SLOT_WEAPON ] == nullptr )
			{
				requiredAnimationName = meleeAnimationName;
			}
			else
			{
				switch( m_equippedItems[ EquipSlot::EQUIP_SLOT_WEAPON ]->GetItemDefinition()->GetWeaponType() )
				{
				case WeaponType::WEAPON_TYPE_PROJECTILE		:	requiredAnimationName = shootingAnimationName;	break;
				case WeaponType::WEAPON_TYPE_MELEE			:	requiredAnimationName = meleeAnimationName;		break;
				default										:	requiredAnimationName = meleeAnimationName;		break;
				}
			}
		}
	}

	return requiredAnimationName;
}

void Actor::CheckPlayerProximityForDialogue()
{
	if ( !m_hasPlayerComeWithinProximity && m_actorDefinition->GetDialogue( ActorDialogueType::ACTOR_DIALOGUE_TYPE_PROXIMITY ) != nullptr )
	{
		Vector2 playerPosition = m_map->GetPlayerEntity()->GetLocation();
		float distanceFromPlayerSquared = ( playerPosition - m_position ).GetLengthSquared();
		if ( distanceFromPlayerSquared <= ( m_actorDefinition->GetProximityDialogueRadius() * m_actorDefinition->GetProximityDialogueRadius() ) )
		{
			g_theGame->PushDialogue( m_actorDefinition->GetDialogue( ActorDialogueType::ACTOR_DIALOGUE_TYPE_PROXIMITY ) );
			m_hasPlayerComeWithinProximity = true;
		}
	}
}

void Actor::Render( float renderAlpha, bool developerModeEnabled ) const
{
	Rgba renderTint = Rgba::WHITE;
	if ( IsDead() )
	{
		renderAlpha *= 0.5f;
	}
	if ( GetStat( StatID::STAT_HEALTH ) < GetBaseStat( StatID::STAT_HEALTH ) && m_secondsSinceLastDamageAttempt < g_gameConfigBlackboard.GetValue( "actorSecondsToShowDamage", 0.5f ) )
	{
		renderTint = Rgba::RED;
	}
	else if ( m_tags.HasTags( GetNameForStatID( StatID::STAT_POISON_USAGE ) ) )
	{
		renderTint = Rgba::GREEN;
	}
	else if ( m_tags.HasTags( GetNameForStatID( StatID::STAT_FIRE_USAGE ) ) )
	{
		renderTint = Rgba::ORANGE;
	}
	else if ( m_tags.HasTags( GetNameForStatID( StatID::STAT_ICE_USAGE ) ) )
	{
		renderTint = Rgba::CYAN;
	}
	else if ( m_tags.HasTags( GetNameForStatID( StatID::STAT_ELECTRICITY_USAGE ) ) )
	{
		renderTint = Rgba::YELLOW;
	}

	g_renderer->PushMatrix();
	g_renderer->Translate( m_position.x, m_position.y, 0.0f );

	g_renderer->DrawTexturedAABB( m_entityDefinition->GetDrawBounds(), *m_spriteAnimationSet->GetCurrentSpriteAnimation()->GetDefinition()->GetTexture(), m_spriteAnimationSet->GetCurrentSpriteAnimation()->GetCurrentTexCoords().mins, m_spriteAnimationSet->GetCurrentSpriteAnimation()->GetCurrentTexCoords().maxs, renderTint.GetWithAlpha( renderAlpha ) );
	if ( developerModeEnabled )
	{
		m_entityDefinition->RenderDeveloperModeVertices( renderAlpha );
	}
	RenderEquipment( renderTint, renderAlpha );

	g_renderer->PopMatrix();
}

void Actor::RenderEquipment( const Rgba& renderTint, float renderAlpha ) const
{
	for ( int equipSlotIndex = 0; equipSlotIndex < EquipSlot::NUM_SLOTS; equipSlotIndex++ )
	{
		if ( equipSlotIndex != EquipSlot::EQUIP_SLOT_SPELL && m_equippedItems[ equipSlotIndex ] != nullptr )
		{
			std::string currentAnimationName = m_spriteAnimationSet->GetCurrentSpriteAnimation()->GetDefinition()->GetName();
			int currentSpriteIndex = m_spriteAnimationSet->GetCurrentSpriteAnimation()->GetCurrentIndex();

			m_equippedItems[ equipSlotIndex ]->RenderSprite( currentAnimationName, currentSpriteIndex, m_entityDefinition->GetDrawBounds(), renderTint, renderAlpha );
		}
	}
}

void Actor::RenderHealthInfo( float renderAlpha ) const
{
	g_renderer->PushMatrix();
	g_renderer->Translate( m_position.x, m_position.y, 0.0f );

	float healthBarLength = g_gameConfigBlackboard.GetValue( "healthBarLineLength", ( TILE_SIDE_LENGTH_WORLD_UNITS * 0.5f ) );

	Vector2 drawBoundsMaxY = Vector2( 0.0f, m_entityDefinition->GetDrawBounds().maxs.y );
	Vector2 healthBarStartPoint = drawBoundsMaxY + g_gameConfigBlackboard.GetValue( "healthBarLocalDrawLocation", Vector2::ZERO ) - ( 0.5f * Vector2( healthBarLength, healthBarLength ) );

	float currentHealthFraction = static_cast< float >( GetStat( StatID::STAT_HEALTH ) ) / static_cast<float>( GetBaseStat( StatID::STAT_HEALTH ) );
	Vector2 healthBarEndPoint = healthBarStartPoint + ( currentHealthFraction * ( healthBarLength * Vector2::RIGHT ) );

	Rgba healthBarColor = Rgba::GREEN;
	if ( currentHealthFraction < g_gameConfigBlackboard.GetValue( "healthBarYellowMinHealthFraction", 0.4f ) )
	{
		healthBarColor = Rgba::RED;
	}
	else if ( currentHealthFraction < g_gameConfigBlackboard.GetValue( "healthBarGreenMinHealthFraction", 0.8f ) )
	{
		healthBarColor = Rgba::YELLOW;
	}

	float healthBarThickness = g_gameConfigBlackboard.GetValue( "healthBarLineThickness", 2.0f );

	RenderDamageOrAttackString( renderAlpha, m_secondsSinceLastAttackAttempt, m_attackStringToRender );
	RenderDamageOrAttackString( renderAlpha, m_secondsSinceLastDamageAttempt, m_damageStringToRender );

	g_renderer->DrawLine( healthBarStartPoint, healthBarEndPoint, healthBarColor.GetWithAlpha( renderAlpha ), healthBarColor, healthBarThickness );

	g_renderer->PopMatrix();
}

void Actor::RenderDamageOrAttackString( float renderAlpha, float secondsToConsider, const std::string& stringToRender ) const		// Render coordinates are in local space
{
	if ( stringToRender != "" )
	{
		float maxYForText = g_gameConfigBlackboard.GetValue( "damageTextMaxYRelative", 0.5f );
		float timeForTextDisplay = g_gameConfigBlackboard.GetValue( "actorSecondsToShowAttackText", 0.5f );
		float timeElapsedFraction = secondsToConsider / timeForTextDisplay;

		float textAlpha = 1.0f - timeElapsedFraction;
		AABB2 textBounds = m_entityDefinition->GetDrawBounds();
		float textYDisplacement = timeElapsedFraction * maxYForText;
		Vector2 renderMins = Vector2( textBounds.GetCenter().x, textBounds.maxs.y );
		renderMins.y += textYDisplacement;

		Rgba renderColor = Rgba::RED;
		if ( stringToRender == "Miss" || stringToRender == "Dead" || stringToRender == "No Spell" )
		{
			renderColor = Rgba::WHITE;
		}
		renderColor = renderColor.GetWithAlpha( renderAlpha * textAlpha );

		g_renderer->DrawText2D( renderMins, stringToRender, 0.2f, renderColor, 1.0f, g_renderer->CreateOrGetBitmapFont( FIXED_FONT_NAME ) );
	}
}

void Actor::PerformAction()
{
	Player* playerEntity = m_map->GetPlayerEntity();
	Vector2 playerLocation = playerEntity->GetLocation();
	Vector2 directionToPlayer = playerLocation - m_position;
	float distanceFromPlayer = directionToPlayer.NormalizeAndGetLength();

	if ( !playerEntity->IsDead() && distanceFromPlayer <= g_gameConfigBlackboard.GetValue( "enemySightDistance", 1.0f ) && m_map->HasLineOfSight( m_position, playerLocation ) )		// If the tank can see the player
	{
		if ( !m_isBeingKnocked )
		{
			m_velocity = directionToPlayer;
			m_facingDirection = m_velocity;
			m_speed = static_cast< float >( GetStat( StatID::STAT_AGILITY ) );
		}

		if ( CanAttack() )		// Prefer spells
		{
			if ( m_equippedItems[ EquipSlot::EQUIP_SLOT_SPELL ] != nullptr && CheckRandomChance( g_gameConfigBlackboard.GetValue( "enemyAIChanceOfUsingSpell", 0.5f ) ) )
			{
				Attack( EquipSlot::EQUIP_SLOT_SPELL );
			}
			else
			{
				Attack( EquipSlot::EQUIP_SLOT_WEAPON );
			}
		}
	}
	else		// Roam around randomly if the actor can't see the player
	{
		if ( !m_isBeingKnocked && m_secondsSinceDirectionChange >= g_gameConfigBlackboard.GetValue( "enemyWanderReorientTime", 1.0f ) )
		{
			float newVelocityOrientation = GetRandomFloatInRange( -180.0f, 180.0f );
			m_secondsSinceDirectionChange = 0.0f;
			m_velocity = Vector2::MakeDirectionAtDegrees( newVelocityOrientation );
			m_facingDirection = m_velocity;
			m_speed = static_cast< float >( GetStat( StatID::STAT_AGILITY ) );
		}
	}
}

void Actor::Attack( EquipSlot slotToUse )
{
	if ( GetStat( StatID::STAT_DEXTERITY ) == 0 )
	{
		return;
	}
	if ( slotToUse != EquipSlot::EQUIP_SLOT_WEAPON && slotToUse != EquipSlot::EQUIP_SLOT_SPELL )
	{
		return;
	}

	m_secondsSinceLastAttackAttempt = 0.0f;
	m_currentAttackSlot = slotToUse;
	int currentStats[ StatID::NUM_STATS ];
	GetStats( currentStats );

	Item* equippedWeaponOrSpell = GetEquippedItem( slotToUse );
	if ( equippedWeaponOrSpell != nullptr )
	{
		if ( equippedWeaponOrSpell->GetItemDefinition()->GetWeaponType() == WeaponType::WEAPON_TYPE_PROJECTILE )
		{
			Vector2 projectileVelocity = m_facingDirection;
			if ( projectileVelocity == Vector2::ZERO )
			{
				projectileVelocity = Vector2::DOWN;		// Since actors face "down" while in idle state
			}
			SetFinalProjectileVelocityAfterAimStatComputation( projectileVelocity );
			m_map->SpawnProjectile( equippedWeaponOrSpell->GetItemDefinition()->GetProjectileSpawned(), this, WeaponType::WEAPON_TYPE_PROJECTILE, m_position, projectileVelocity, currentStats );
		}
		else if ( equippedWeaponOrSpell->GetItemDefinition()->GetWeaponType() == WeaponType::WEAPON_TYPE_MELEE )
		{
			if ( ShouldMeleeAttackSucceed() )
			{
				Vector2 projectileVelocity = Vector2::ZERO;
				Vector2 projectileSpawnPosition = m_position + ( ( equippedWeaponOrSpell->GetItemDefinition()->GetProjectileSpawned()->GetPhysicsRadius() + m_actorDefinition->GetPhysicsRadius() ) * m_facingDirection );
				m_map->SpawnProjectile( equippedWeaponOrSpell->GetItemDefinition()->GetProjectileSpawned(), this, WeaponType::WEAPON_TYPE_MELEE, projectileSpawnPosition, projectileVelocity, currentStats );
				m_attackStringToRender = "";
			}
			else
			{
				m_attackStringToRender = "Miss";
			}
		}
	}
	else
	{
		if ( ShouldMeleeAttackSucceed() )
		{
			ProjectileDefinition* meleeHitboxDefinition = ProjectileDefinition::s_definitions[ "MeleeHitbox" ];
			Vector2 projectileVelocity = Vector2::ZERO;
			Vector2 projectileSpawnPosition = m_position + ( ( meleeHitboxDefinition->GetPhysicsRadius() + m_actorDefinition->GetPhysicsRadius() ) * m_facingDirection );
			m_map->SpawnProjectile( meleeHitboxDefinition, this, WeaponType::WEAPON_TYPE_MELEE, projectileSpawnPosition, projectileVelocity, currentStats );
			m_attackStringToRender = "";
		}
		else
		{
			if ( m_currentAttackSlot == EquipSlot::EQUIP_SLOT_SPELL )
			{
				m_attackStringToRender = "No Spell";
			}
			else
			{
				m_attackStringToRender = "Miss";
			}
		}
	}
}

void Actor::SetFinalProjectileVelocityAfterAimStatComputation( Vector2& projectileVelocity )
{
	float maxDeflectionDegrees = g_gameConfigBlackboard.GetValue( "projectileMaxDeflectionDegrees", 30.0f );
	float deflectionDegrees = maxDeflectionDegrees;
	if ( GetStat( StatID::STAT_AIM ) != 0 )
	{
		deflectionDegrees = deflectionDegrees / static_cast< float >( GetStat( StatID::STAT_AIM ) );
	}
	projectileVelocity.RotateDegrees( GetRandomFloatInRange( -deflectionDegrees, deflectionDegrees ) );
}

bool Actor::CanAttack() const
{
	if ( GetStat( StatID::STAT_DEXTERITY ) == 0 || IsAttacking() )
	{
		return false;
	}

	float attackCooldownSeconds = g_gameConfigBlackboard.GetValue( "actorAttackMaxCooldown", 1.0f ) / static_cast< float >( GetStat( StatID::STAT_DEXTERITY ) );
	return ( m_secondsSinceLastAttackAttempt > attackCooldownSeconds );
}

void Actor::Translate( float deltaSeconds )
{
	ApplyFrictionOrStop( deltaSeconds );
	m_position = m_position + ( ( m_speed * m_velocity ) *  deltaSeconds );
}

void Actor::ApplyKnockbackTranslation( const Vector2& knockback )
{
	if ( !IsFloatEqualTo( m_actorDefinition->GetPhysicsMass(), 0.0f ) )
	{
		Vector2 paddedKnockback = knockback / m_actorDefinition->GetPhysicsMass();
		m_velocity = paddedKnockback;
	}
	else
	{
		m_velocity = knockback;
	}

	if ( knockback != Vector2::ZERO )
	{
		m_speed = 1.0f;		// Let the projectile govern the speed
		m_isBeingKnocked = true;
	}
}

void Actor::ApplyFrictionOrStop( float deltaSeconds )
{
	if ( m_speed > 0.0f && !IsFloatEqualTo( m_speed, 0.0f ) )
	{
		m_speed -= GetFrictionPenalty( m_speed, deltaSeconds );
	}
	else
	{
		m_speed = 0.0f;
		m_velocity = Vector2::ZERO;
		m_facingDirection = Vector2::ZERO;
		m_isBeingKnocked = false;
	}
}
