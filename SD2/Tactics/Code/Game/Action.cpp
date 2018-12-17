#include "Game/Action.hpp"
#include "Game/Actor.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Math/Trajectory.hpp"

#pragma region Action

Action::Action( Actor* actor, unsigned int waitTime /* = 0 */ )
	:	m_actor( actor ),
		m_waitTime( waitTime )
{

}

Action::~Action()
{

}

void Action::DecayWait( int wait )
{
	while ( m_waitTime > 0 && wait > 0 )
	{
		m_waitTime--;
		wait--;
	}
}

std::string Action::GetName() const
{
	return Action::GetName( GetType() );
}

ActionType Action::GetType() const
{
	return ActionType::ACTION_TYPE_INVALID;
}

bool Action::CanTick() const
{
	return ( m_waitTime == 0 );
}

bool Action::HasInitialized() const
{
	return m_hasInitialized;
}

std::string Action::GetName( ActionType actionType )
{
	switch( actionType )
	{
		case ActionType::ACTION_TYPE_INVALID : return "Invalid";
		case ActionType::WAIT : return "Wait";
		case ActionType::MOVE : return "Move";
		case ActionType::ATTACK : return "Attack";
		case ActionType::BOW : return "Bow";
		case ActionType::DEFEND : return "Defend";
		case ActionType::HEAL : return "Heal";
		case ActionType::CAST_FIRE : return "Fire";
		case ActionType::DAMAGE_HEAL_NUMBER : return "DamageHealNumber";
		case ActionType::DEATH : return "Death";
		default : return "Invalid";
	}
}

ActionType Action::GetType( const std::string& actionName )
{
	ActionType type = ActionType::ACTION_TYPE_INVALID;
	
	if ( actionName == "Wait" )
	{
		type = ActionType::WAIT;
	}
	else if ( actionName == "Move" )
	{
		type = ActionType::MOVE;
	}
	else if ( actionName == "Attack" )
	{
		type = ActionType::ATTACK;
	}
	else if ( actionName == "Bow" )
	{
		type = ActionType::BOW;
	}
	else if ( actionName == "Defend" )
	{
		type = ActionType::DEFEND;
	}
	else if ( actionName == "Heal" )
	{
		type = ActionType::HEAL;
	}
	else if ( actionName == "Fire" )
	{
		type = ActionType::CAST_FIRE;
	}
	else if ( actionName == "DamageHealNumber" )
	{
		type = ActionType::DAMAGE_HEAL_NUMBER;
	}
	else if ( actionName == "Death" )
	{
		type = ActionType::DEATH;
	}

	return type;
}

#pragma endregion

#pragma region DelayedAction

DelayedAction::DelayedAction( Actor* actor, const IntVector2& actionPosition, const IntRange& allowedRange, unsigned int waitTime )
	:	Action( actor, waitTime ),
		m_actionPosition( actionPosition ),
		m_allowedRange( allowedRange )
{

}

DelayedAction::~DelayedAction()
{

}

void DelayedAction::DecayWait( int wait )
{
	Action::DecayWait( wait );
	if ( !ActorOnlyMovedOrWaited() || !IsActorStillInRange() )
	{
		FailAction();
	}
}

void DelayedAction::FailAction()
{
	g_encounterGameState->RemoveFlyoutTextIfPresent( GetFlyoutTextKey() );
	FlyoutText blockFlyoutText = FlyoutText(
		"Action Failed...",
		( m_actor->GetOwningMap()->GetWorldPositionFrom2DMapPosition( m_actionPosition ) + Vector3::UP ),
		Rgba::RED,
		false,
		ACTOR_FLYOUT_TEXT_RENDER_TIME_SECONDS,
		g_renderer->CreateOrGetShaderProgram( OUTLINE_TEXT_SHADER_NAME ),
		0.5f
	);
	g_encounterGameState->AddFlyoutText( GetFailedFlyoutTextKey(), blockFlyoutText );
	g_encounterGameState->PopAction();	// Pops this DelayedAction
}

bool DelayedAction::ActorOnlyMovedOrWaited() const
{
	return ( m_actor->GetLastAction() == ActionType::MOVE || m_actor->GetLastAction() == ActionType::WAIT );
}

bool DelayedAction::IsActorStillInRange() const
{
	IntVector2 displacement = m_actor->GetPosition() - m_actionPosition;
	int manhattanDistance = Abs( displacement.x ) + Abs( displacement.y );
	return ( manhattanDistance >= m_allowedRange.min && manhattanDistance <= m_allowedRange.max );
}

std::string DelayedAction::GetFailedFlyoutTextKey() const
{
	return ( "delay_failed_" + m_actor->GetName() );
}

std::string DelayedAction::GetFlyoutTextKey() const
{
	return ( Action::GetName( GetType() ) + "_" + m_actor->GetName() );
}

#pragma endregion

#pragma region MoveAction

MoveAction::MoveAction( Actor* actor, const IntVector2& movePosition, const std::vector<IntVector2>& path )
	:	Action( actor ),
		m_movePosition( movePosition ),
		m_path( path )
{
	m_actor->AddWaitTime( ACTOR_MOVE_WAIT_TIME );
	m_actor->DisableAction( ActionType::MOVE );
}

MoveAction::~MoveAction()
{

}

void MoveAction::Init()
{
	g_encounterGameState->SetCameraTargetLookAt( m_actor->GetRenderPosition() );
	m_actor->SetDestination( m_movePosition );
	m_actor->GetIsometricSpriteAnimationSet()->SetAnimation( "Walk", m_actor->GetFacingDirection() );
	
	m_hasInitialized = true;
}

void MoveAction::Tick()
{
	if (!m_hasInitialized)
	{
		Init();
	}

	if ( IsMoveComplete() )
	{
		m_actor->GetIsometricSpriteAnimationSet()->SetAnimation( "Idle", m_actor->GetFacingDirection() );
		m_actor->SetRenderPositionFromMapPosition();
		g_encounterGameState->PopAction();	// Pops this MoveAction
	}
	else
	{
		float lerpSpeed = BLOCK_SIZE_WORLD_UNITS * ( ACTOR_MOVE_ANIMATION_BLOCKS_PER_SECOND * GetMasterDeltaSecondsF() );
		m_pathProgress += lerpSpeed;

		TryUpdateNextPointOnPath();
		Vector3 initialPosition = GetPathPointWorldCoordinates( m_currentPathPoint );
		Vector3 finalPosition = GetPathPointWorldCoordinates( m_currentPathPoint + 1 );
		m_actor->LerpTo( initialPosition, finalPosition, Fraction( m_pathProgress ) );
		g_encounterGameState->SetCameraTargetLookAt( m_actor->GetRenderPosition(), false );
	}
}

ActionType MoveAction::GetType() const
{
	return ActionType::MOVE;
}

Vector3 MoveAction::GetPathPointWorldCoordinates( size_t pointIndex )
{
	if ( pointIndex >= m_path.size() )
	{
		pointIndex = m_path.size() - 1;
	}
	Map* map = m_actor->GetOwningMap();
	Vector3 point = map->GetWorldPositionOnSurfaceFrom2DMapPosition( m_path[ pointIndex ] );
	point = m_actor->GetHeightCorrectedPosition( point );
	return point;
}

void MoveAction::TryUpdateNextPointOnPath()
{
	if ( static_cast< int >( m_currentPathPoint ) < Integer( m_pathProgress ) )
	{
		m_currentPathPoint++;
	}
}

bool MoveAction::IsMoveComplete() const
{
	return ( m_pathProgress >= static_cast< float >( m_path.size() ) );
}

#pragma endregion

#pragma region AttackAction

AttackAction::AttackAction( Actor* actor, const IntVector2& attackPosition, bool isBlocked, bool isCritical )
	:	Action( actor ),
		m_attackPosition( attackPosition ),
		m_blocked(isBlocked),
		m_critical(isCritical)
{
	m_actor->AddWaitTime( ACTOR_ATTACK_WAIT_TIME );
	m_actor->DisableAction( ActionType::ATTACK );
}

AttackAction::~AttackAction()
{

}

void AttackAction::Init()
{
	m_actor->TurnToward(m_attackPosition);
	m_actor->GetIsometricSpriteAnimationSet()->SetAnimation( "Attack", m_actor->GetFacingDirection() );

	m_hasInitialized = true;
}

void AttackAction::Tick()
{
	if (!m_hasInitialized)
	{
		Init();
	}

	if ( HasActorStoppedAnimation( m_actor ) && HasActorStoppedAnimation( GetTargetActor() ) )
	{
		if (!m_blocked)
		{
			if (m_critical)
			{
				m_critMultiplier = ACTOR_ATTACK_CRIT_MULTIPLIER;
				FlyoutText blockFlyoutText = FlyoutText(
					"x2!",
					( GetTargetActor()->GetRenderPosition() + Vector3::UP ),
					Rgba::RED,
					false,
					ACTOR_FLYOUT_TEXT_RENDER_TIME_SECONDS,
					g_renderer->CreateOrGetShaderProgram( OUTLINE_TEXT_SHADER_NAME ),
					0.5f
				);
				g_encounterGameState->AddFlyoutText( GetCritFlyoutTextKey(), blockFlyoutText );
			}
			GetTargetActor()->GetIsometricSpriteAnimationSet()->SetAnimation( "Hit", GetTargetActor()->GetFacingDirection() );
			GetTargetActor()->Damage( GetDamage() );
			g_encounterGameState->PushActionTo( new DamageHealNumberAction( GetTargetActor(), GetDamage() ), 1 );
			if ( GetTargetActor()->GetHealth() == 0 )
			{
				g_encounterGameState->PushActionTo( new DeathAction( GetTargetActor() ), 2 );
			}
		}
		else
		{
			FlyoutText blockFlyoutText = FlyoutText(
				"Blocked!",
				( GetTargetActor()->GetRenderPosition() + Vector3::UP ),
				Rgba::WHITE,
				false,
				ACTOR_FLYOUT_TEXT_RENDER_TIME_SECONDS,
				g_renderer->CreateOrGetShaderProgram( OUTLINE_TEXT_SHADER_NAME ),
				0.5f
			);
			g_encounterGameState->AddFlyoutText( GetBlockFlyoutTextKey(), blockFlyoutText );
		}
		g_encounterGameState->SetRenderFlag( EncounterRenderFlag::RENDER_FLAG_FLYOUT_TEXT );
		GetTargetActor()->SetAugmentedBlockChance( 0.0f );	// As the Target Actor's block effect now wears off
		g_encounterGameState->PopAction();		// Pops this AttackAction
	}
}

Actor* AttackAction::GetTargetActor() const
{
	return g_encounterGameState->GetCurrentEncounter()->GetActorAtPosition( m_attackPosition );
}

int AttackAction::GetDamage() const
{
	return ( m_actor->GetStats().m_strength * m_critMultiplier );
}

bool AttackAction::HasActorStoppedAnimation( const Actor* actor ) const
{
	std::string currentAnimationName = actor->GetIsometricSpriteAnimationSet()->GetSpriteAnimationSet()->GetCurrentSpriteAnimation()->GetDefinition()->GetName();

	// The animation set will revert to Idle once both the Attack and Hit animations are done, as they are marked as PLAY_TO_END
	return ( currentAnimationName.find( "Idle" ) != std::string::npos );
}

ActionType AttackAction::GetType() const
{
	return ActionType::ATTACK;
}

std::string AttackAction::GetBlockFlyoutTextKey() const
{
	return ( "block_" + GetTargetActor()->GetName() );
}

std::string AttackAction::GetCritFlyoutTextKey() const
{
	return ( "crit_" + GetTargetActor()->GetName() );
}

#pragma endregion

#pragma region BowAction

BowAction::BowAction( Actor* actor, const IntVector2& attackPosition, bool isBlocked, bool isCritical )
	:	Action( actor ),
		m_attackPosition( attackPosition ),
		m_blocked(isBlocked),
		m_critical(isCritical)
{
	m_actor->AddWaitTime( ACTOR_BOW_WAIT_TIME );
	m_actor->DisableAction( ActionType::BOW );
}

BowAction::~BowAction()
{

}

void BowAction::ComputeLaunchVelocity()
{
	Vector3 displacementVector = m_actor->GetOwningMap()->GetWorldPositionOnSurfaceFrom2DMapPosition( m_attackPosition ) - m_actor->GetOwningMap()->GetWorldPositionOnSurfaceFrom2DMapPosition( m_actor->GetPosition() );
	Vector2 horizontalDisplacement = Vector2( displacementVector.x, displacementVector.z );
	float horizontalDistance = horizontalDisplacement.GetLength();
	float verticalDistance = displacementVector.y;
	float minimumLaunchSpeed = Trajectory::GetMinimumLaunchSpeed( ACTOR_BOW_ACTION_GRAVITY_MAGNITUDE, horizontalDistance );
	float maxHeight = Trajectory::GetMaxHeight( ACTOR_BOW_ACTION_GRAVITY_MAGNITUDE, minimumLaunchSpeed, horizontalDistance );
	m_launchVelocity = Trajectory::GetLaunchVelocity( ACTOR_BOW_ACTION_GRAVITY_MAGNITUDE, maxHeight, horizontalDistance, verticalDistance );
	m_launchDirection = horizontalDisplacement.GetNormalized();
}

void BowAction::Init()
{
	m_actor->TurnToward(m_attackPosition);
	g_encounterGameState->SetCameraTargetLookAt( m_actor->GetRenderPosition() );

	ComputeLaunchVelocity();
	m_actor->GetIsometricSpriteAnimationSet()->SetAnimation( "Attack", m_actor->GetFacingDirection() );

	m_hasInitialized = true;
}

void BowAction::Tick()
{
	if (!m_hasInitialized)
	{
		Init();
	}

	if ( HasActorStoppedBowAnimation() && !m_hit )
	{
		Vector2 projectilePosition2D = Trajectory::Evaluate( ACTOR_BOW_ACTION_GRAVITY_MAGNITUDE, m_launchVelocity, m_bowProgress );
		Vector3 projectilePosition3D = m_actor->GetRenderPosition() + Vector3( ( projectilePosition2D.x * m_launchDirection.x ), projectilePosition2D.y, ( projectilePosition2D.x * m_launchDirection.y ) );
		g_encounterGameState->SetProjectilePosition( projectilePosition3D );
		g_encounterGameState->SetCameraTargetLookAt( projectilePosition3D, false );
		g_encounterGameState->SetRenderFlag( EncounterRenderFlag::RENDER_FLAG_PROJECTILE );

		IntVector3 destinationBlock = m_actor->GetOwningMap()->Get3DMapPositionFrom2DMapPosition( m_attackPosition );
		if ( m_actor->GetOwningMap()->IsInBlock( destinationBlock, projectilePosition3D ) )
		{
			m_hit = true;
			TryDamageTargetActor();
		}
		else if ( m_actor->GetOwningMap()->IsInSolidBlock( projectilePosition3D ) )
		{
			m_hit = true;
			AddMissFlyoutText();
		}

		m_bowProgress += g_encounterGameState->GetClock()->GetDeltaSecondsF();
	}
	else if ( m_hit )
	{
		g_encounterGameState->RemoveRenderFlag( EncounterRenderFlag::RENDER_FLAG_PROJECTILE );
		g_encounterGameState->PopAction();	// Pops this BowAction
	}
}

void BowAction::TryDamageTargetActor()
{
	if (!m_blocked)
	{
		if (m_critical)
		{
			m_critMultiplier = ACTOR_ATTACK_CRIT_MULTIPLIER;
			FlyoutText blockFlyoutText = FlyoutText(
				"x2!",
				( GetTargetActor()->GetRenderPosition() + Vector3::UP ),
				Rgba::RED,
				false,
				ACTOR_FLYOUT_TEXT_RENDER_TIME_SECONDS,
				g_renderer->CreateOrGetShaderProgram( OUTLINE_TEXT_SHADER_NAME ),
				0.5f
			);
			g_encounterGameState->AddFlyoutText( GetCritFlyoutTextKey(), blockFlyoutText );
		}
		GetTargetActor()->GetIsometricSpriteAnimationSet()->SetAnimation( "Hit", GetTargetActor()->GetFacingDirection() );
		GetTargetActor()->Damage( GetDamage() );
		g_encounterGameState->PushActionTo( new DamageHealNumberAction( GetTargetActor(), GetDamage() ), 1 );
		if ( GetTargetActor()->GetHealth() == 0 )
		{
			g_encounterGameState->PushActionTo( new DeathAction( GetTargetActor() ), 2 );
		}
	}
	else
	{
		FlyoutText blockFlyoutText = FlyoutText(
			"Blocked!",
			( GetTargetActor()->GetRenderPosition() + Vector3::UP ),
			Rgba::WHITE,
			false,
			ACTOR_FLYOUT_TEXT_RENDER_TIME_SECONDS,
			g_renderer->CreateOrGetShaderProgram( OUTLINE_TEXT_SHADER_NAME ),
			0.5f
		);
		g_encounterGameState->AddFlyoutText( GetBlockFlyoutTextKey(), blockFlyoutText );
	}
	g_encounterGameState->SetRenderFlag( EncounterRenderFlag::RENDER_FLAG_FLYOUT_TEXT );
	GetTargetActor()->SetAugmentedBlockChance( 0.0f );	// As the Target Actor's block effect now wears off
}

void BowAction::AddMissFlyoutText()
{
	FlyoutText missFlyoutText = FlyoutText(
		"Miss!",
		( GetTargetActor()->GetRenderPosition() + Vector3::UP ),
		Rgba::WHITE,
		false,
		ACTOR_FLYOUT_TEXT_RENDER_TIME_SECONDS,
		g_renderer->CreateOrGetShaderProgram( OUTLINE_TEXT_SHADER_NAME ),
		0.5f
	);
	g_encounterGameState->AddFlyoutText( GetMissFlyoutTextKey(), missFlyoutText );
}

bool BowAction::HasActorStoppedBowAnimation() const
{
	return ( m_actor->GetIsometricSpriteAnimationSet()->GetSpriteAnimationSet()->GetCurrentSpriteAnimation()->GetDefinition()->GetName().find( "Idle" ) != std::string::npos );
}

Actor* BowAction::GetTargetActor() const
{
	return g_encounterGameState->GetCurrentEncounter()->GetActorAtPosition( m_attackPosition );
}

int BowAction::GetDamage() const
{
	return ( m_actor->GetStats().m_strength * m_critMultiplier );
}

ActionType BowAction::GetType() const
{
	return ActionType::BOW;
}

std::string BowAction::GetBlockFlyoutTextKey() const
{
	return ( "block_" + GetTargetActor()->GetName() );
}

std::string BowAction::GetCritFlyoutTextKey() const
{
	return ( "crit_" + GetTargetActor()->GetName() );
}

std::string BowAction::GetMissFlyoutTextKey() const
{
	return ( "miss_" + GetTargetActor()->GetName() );
}

#pragma endregion

#pragma region HealAction

HealAction::HealAction( Actor* actor, const IntVector2& healPosition )
	:	DelayedAction( actor, healPosition, IntRange( 0, 3 ), HEAL_ACTION_WAIT ),
		m_healPosition( healPosition )
{
	m_actor->AddWaitTime( ACTOR_ATTACK_WAIT_TIME );
	m_actor->DisableAction( ActionType::HEAL );
}

HealAction::~HealAction()
{

}

void HealAction::Init()
{
	if ( m_actor != GetTargetActor() )
	{
		m_actor->SetFacingDirection( ( GetTargetActor()->GetRenderPosition() - m_actor->GetRenderPosition() ).GetNormalized() );
	}

	m_actor->GetIsometricSpriteAnimationSet()->SetAnimation( "Cast", m_actor->GetFacingDirection() );

	FlyoutText healFlyoutText = FlyoutText(
		"Heal",
		( m_actor->GetOwningMap()->GetWorldPositionFrom2DMapPosition( m_actionPosition ) + Vector3::UP ),
		Rgba::GREEN,
		true,
		0.0f,
		g_renderer->CreateOrGetShaderProgram( OUTLINE_TEXT_SHADER_NAME ),
		0.25f
	);
	g_encounterGameState->AddFlyoutText( GetFlyoutTextKey(), healFlyoutText );

	m_hasInitialized = true;
}

void HealAction::Tick()
{
	if (!m_hasInitialized)
	{
		Init();
	}

	if ( GetTargetActor() != nullptr )
	{
		GetTargetActor()->GetIsometricSpriteAnimationSet()->SetAnimation( "Jump", m_actor->GetFacingDirection() );
		g_encounterGameState->RemoveFlyoutTextIfPresent( GetFlyoutTextKey() );
		GetTargetActor()->Heal( GetHealth() );
		g_encounterGameState->PushActionTo( new DamageHealNumberAction( GetTargetActor(), GetHealth(), Rgba::GREEN ), 1 );
		g_encounterGameState->PopAction();		// Pops this HealAction
	}
	else
	{
		FailAction();
	}
}

Actor* HealAction::GetTargetActor() const
{
	return g_encounterGameState->GetCurrentEncounter()->GetActorAtPosition( m_healPosition );
}

int HealAction::GetHealth() const
{
	return ( m_actor->GetStats().m_strength );
}

ActionType HealAction::GetType() const
{
	return ActionType::HEAL;
}

#pragma endregion

#pragma region CastFireAction

CastFireAction::CastFireAction( Actor* actor, const IntVector2& castFirePosition )
	:	DelayedAction( actor, castFirePosition, IntRange( 0, 1 ), FIRE_ACTION_WAIT ),
		m_castFirePosition( castFirePosition )
{
	m_actor->AddWaitTime( ACTOR_CAST_WAIT_TIME );
	m_actor->DisableAction( ActionType::CAST_FIRE );
}

CastFireAction::~CastFireAction()
{

}

void CastFireAction::Init()
{
	if ( m_castFirePosition != m_actor->GetPosition() )
	{
		m_actor->SetFacingDirection( ( m_actor->GetOwningMap()->GetWorldPositionFrom2DMapPosition( m_actionPosition ) - m_actor->GetRenderPosition() ).GetNormalized() );
	}

	m_actor->GetIsometricSpriteAnimationSet()->SetAnimation( "Cast", m_actor->GetFacingDirection() );

	FlyoutText castFlyoutText = FlyoutText(
		"Fire",
		( m_actor->GetOwningMap()->GetWorldPositionFrom2DMapPosition( m_actionPosition ) + Vector3::UP ),
		Rgba::ORANGE,
		true,
		0.0f,
		g_renderer->CreateOrGetShaderProgram( OUTLINE_TEXT_SHADER_NAME ),
		0.25f
	);
	g_encounterGameState->AddFlyoutText( GetFlyoutTextKey(), castFlyoutText );

	m_hasInitialized = true;
}

void CastFireAction::Tick()
{
	if (!m_hasInitialized)
	{
		Init();
	}

	bool actorHit = false;
	for ( Actor* targetActor : GetTargetActors() )
	{
		targetActor->GetIsometricSpriteAnimationSet()->SetAnimation( "Hit", targetActor->GetFacingDirection() );
		g_encounterGameState->RemoveFlyoutTextIfPresent( GetFlyoutTextKey() );
		targetActor->Damage( GetDamage() );
		g_encounterGameState->PushActionTo( new DamageHealNumberAction( targetActor, GetDamage(), Rgba::RED ), 1 );
		if ( targetActor->GetHealth() == 0 )
		{
			g_encounterGameState->PushActionTo( new DeathAction( targetActor ), 2 );
		}
		actorHit = true;
	}
	if ( !actorHit )
	{
		FailAction();
	}
	else
	{
		g_encounterGameState->PopAction();		// Pops this CastFireAction
	}
}

std::vector< Actor* > CastFireAction::GetTargetActors() const
{
	std::vector< Actor* > actorsInRange;
	for ( IntVector2 position : g_encounterGameState->GetCurrentEncounter()->GetOccupiedTilesInRange( m_actionPosition, IntRange( 0, 1 ), IntRange( -2, 2 ) ) )
	{
		Actor* actorAtPosition = g_encounterGameState->GetCurrentEncounter()->GetActorAtPosition( position );
		if ( actorAtPosition != nullptr )
		{
			actorsInRange.push_back( actorAtPosition );
		}
	}
	return actorsInRange;
	
}

int CastFireAction::GetDamage() const
{
	return ( m_actor->GetStats().m_strength );
}

ActionType CastFireAction::GetType() const
{
	return ActionType::CAST_FIRE;
}

#pragma endregion

#pragma region DefendAction

DefendAction::DefendAction( Actor* actor, float augmentedBlockChance )
	:	Action( actor ),
		m_augmentedBlockChance( augmentedBlockChance )
{
	m_actor->AddWaitTime( ACTOR_DEFEND_WAIT_TIME );
	m_actor->DisableAction( ActionType::DEFEND );
}

DefendAction::~DefendAction()
{

}

void DefendAction::Init()
{
	FlyoutText defendFlyoutText = FlyoutText(
		"Defending",
		( m_actor->GetRenderPosition() + Vector3::UP ),
		Rgba::WHITE,
		true,
		0.0f,
		g_renderer->CreateOrGetShaderProgram( OUTLINE_TEXT_SHADER_NAME ),
		0.25f
	);
	g_encounterGameState->AddFlyoutText( GetFlyoutTextKey(), defendFlyoutText );
	m_actor->SetAugmentedBlockChance( m_augmentedBlockChance );
	m_actor->SetTrackingFlyoutTextKey( GetFlyoutTextKey() );	// The "Defending" tag should follow the actor around

	m_hasInitialized = true;
}

void DefendAction::Tick()
{
	if (!m_hasInitialized)
	{
		Init();
	}
	g_encounterGameState->PopAction();	// Pops this DefendAction
}

ActionType DefendAction::GetType() const
{
	return ActionType::DEFEND;
}

std::string DefendAction::GetFlyoutTextKey() const
{
	return ( "defend_" + m_actor->GetName() );
}

#pragma endregion

#pragma region WaitAction

WaitAction::WaitAction( Actor* actor )
	:	Action( actor )
{
	
}

WaitAction::~WaitAction()
{

}

void WaitAction::Init()
{
	m_actor->RefreshActionsForTurn();
	m_hasInitialized = true;
}

void WaitAction::Tick()
{
	if (!m_hasInitialized)
	{
		Init();
	}
	g_encounterGameState->PopAction();
}

ActionType WaitAction::GetType() const
{
	return ActionType::WAIT;
}

#pragma endregion

#pragma region DamageHealNumberAction

DamageHealNumberAction::DamageHealNumberAction( Actor* damagedActor, int damageAmount, const Rgba renderColor /* = Rgba::RED */ )
	:	Action( damagedActor ),
		m_damage( damageAmount ),
		m_renderColor( renderColor )
{
	
}

DamageHealNumberAction::~DamageHealNumberAction()
{

}

void DamageHealNumberAction::Init()
{
	g_encounterGameState->SetCameraTargetLookAt( m_actor->GetRenderPosition() );

	FlyoutText damageFlyoutText = FlyoutText(
		std::to_string( m_damage ),
		m_actor->GetRenderPosition() + Vector3::UP,
		m_renderColor,
		false,
		m_remainingAnimationTime,
		g_renderer->CreateOrGetShaderProgram( DAMAGE_NUMBER_SHADER_NAME )
	);
	damageFlyoutText.m_shaderParams.push_back( ShaderFloatUniform( "TOTALTIME", ACTOR_FLYOUT_TEXT_RENDER_TIME_SECONDS ) );
	damageFlyoutText.m_shaderParams.push_back( ShaderFloatUniform( "TIMEREMAINING", m_remainingAnimationTime ) );

	g_encounterGameState->AddFlyoutText( GetFlyoutTextKey(), damageFlyoutText );
	g_encounterGameState->SetRenderFlag( EncounterRenderFlag::RENDER_FLAG_FLYOUT_TEXT );

	m_hasInitialized = true;
}

void DamageHealNumberAction::Tick()
{
	if (!m_hasInitialized)
	{
		Init();
	}

	if ( m_remainingAnimationTime > 0.0f )
	{
		FlyoutText* damageFlyoutText = g_encounterGameState->GetFlyoutText( GetFlyoutTextKey() );
		if ( damageFlyoutText != nullptr )
		{
			damageFlyoutText->m_shaderParams[ 1 ].m_value = m_remainingAnimationTime; // Set "TIMEREMAINING"
		}

		m_remainingAnimationTime -= g_encounterGameState->GetClock()->GetDeltaSecondsF();
	}
	else
	{
		g_encounterGameState->PopAction();	// Pops this DamageNumberAction
	}
}

ActionType DamageHealNumberAction::GetType() const
{
	return ActionType::DAMAGE_HEAL_NUMBER;
}

std::string DamageHealNumberAction::GetFlyoutTextKey() const
{
	return ( "damage_" + m_actor->GetName() );
}

#pragma endregion

#pragma region DeathAction

DeathAction::DeathAction( Actor* deadActor )
	:	Action( deadActor )
{
	
}

DeathAction::~DeathAction()
{

}

void DeathAction::Init()
{
	m_actor->GetIsometricSpriteAnimationSet()->SetAnimation( "Dead", m_actor->GetFacingDirection() );

	FlyoutText deathFlyoutText = FlyoutText(
		"Dead",
		( m_actor->GetRenderPosition() + Vector3::UP ),
		Rgba::RED,
		false,
		m_remainingAnimationTime,
		g_renderer->CreateOrGetShaderProgram( OUTLINE_TEXT_SHADER_NAME )
	);
	g_encounterGameState->AddFlyoutText( GetFlyoutTextKey(), deathFlyoutText );
	g_encounterGameState->SetRenderFlag( EncounterRenderFlag::RENDER_FLAG_FLYOUT_TEXT );

	m_hasInitialized = true;
}

void DeathAction::Tick()
{
	if (!m_hasInitialized)
	{
		Init();
	}

	if ( m_remainingAnimationTime <= 0.0f )
	{
		m_actor->MarkDead();
		g_encounterGameState->PopAction();	// Pops this DeathAction
	}
	m_remainingAnimationTime -= g_encounterGameState->GetClock()->GetDeltaSecondsF();
}

ActionType DeathAction::GetType() const
{
	return ActionType::DEATH;
}

std::string DeathAction::GetFlyoutTextKey() const
{
	return ( "dead_" + m_actor->GetName() );
}

#pragma endregion
