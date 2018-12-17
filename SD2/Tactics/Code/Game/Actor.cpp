#include "Game/Actor.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"

Stats::Stats()
{

}

Stats::Stats( const Stats& copy )
	:	m_maxHealth( copy.m_maxHealth ),
		m_moveSpeed( copy.m_moveSpeed ),
		m_jumpHeight( copy.m_jumpHeight ),
		m_strength( copy.m_strength ),
		m_actionSpeed( copy.m_actionSpeed ),
		m_height( copy.m_height )
{
	
}

Stats::Stats( int maxHealth, int moveSpeed, int jumpHeight, int strength, int actionSpeed, int height )
	:	m_maxHealth( maxHealth ),
		m_moveSpeed( moveSpeed ),
		m_jumpHeight( jumpHeight ),
		m_strength( strength ),
		m_actionSpeed( actionSpeed ),
		m_height( height )
{

}

Stats::Stats( const tinyxml2::XMLElement& statsXML )
{
	m_maxHealth = ParseXmlAttribute( statsXML, "maxHealth", m_maxHealth );
	m_moveSpeed = ParseXmlAttribute( statsXML, "moveSpeed", m_moveSpeed );
	m_jumpHeight = ParseXmlAttribute( statsXML, "jumpHeight", m_jumpHeight );
	m_strength = ParseXmlAttribute( statsXML, "strength", m_strength );
	m_actionSpeed = ParseXmlAttribute( statsXML, "actionSpeed", m_actionSpeed );
	m_height = ParseXmlAttribute( statsXML, "height", m_height );
}

TurnBasedAction::TurnBasedAction( ActionType actionType )
	:	m_actionType( actionType )
{

}

std::string GetTeamNameFromTeam( Team team )
{
	switch( team )
	{
		case Team::TEAM_RED			:	return "Red";
		case Team::TEAM_BLUE		:	return "Blue";
		default						:	return "Invalid";
	}
}

Team GetTeamFromTeamName( const std::string& teamName )
{
	if ( teamName == "red" || teamName == "Red" || teamName == "r" || teamName == "R" )
	{
		return Team::TEAM_RED;
	}
	else if ( teamName == "blue" || teamName == "Blue" || teamName == "b" || teamName == "B" )
	{
		return Team::TEAM_BLUE;
	}
	else
	{
		return Team::TEAM_INVALID;
	}
}

std::map< std::string, ActorDefinition* > ActorDefinition::s_definitions;

ActorDefinition::ActorDefinition( const tinyxml2::XMLElement& actorDefinitionElement )
{
	PopulateFromXml( actorDefinitionElement );
}

ActorDefinition::~ActorDefinition()
{
	delete m_isoSpriteAnimSetDefinition;
	m_isoSpriteAnimSetDefinition = nullptr;
}

void ActorDefinition::PopulateFromXml( const tinyxml2::XMLElement& actorDefinitionElement )
{
	m_name = ParseXmlAttribute( actorDefinitionElement, "name", m_name );

	for ( const tinyxml2::XMLElement* childElement = actorDefinitionElement.FirstChildElement(); childElement != nullptr; childElement = childElement->NextSiblingElement() )
	{
		std::string name = std::string( childElement->Name() );
		if ( name == "Stats" )
		{
			m_baseStats = Stats( *childElement );
		}
		else if ( name == "Actions" )
		{
			for ( const tinyxml2::XMLElement* childActionElement = childElement->FirstChildElement(); childActionElement != nullptr; childActionElement = childActionElement->NextSiblingElement() )
			{
				ActionType actionType = Action::GetType( std::string( childActionElement->Name() ) );
				m_actionTypes.push_back( actionType );
			}
		}
		else if ( name == "IsoSpriteAnimSet" )
		{
			m_isoSpriteAnimSetDefinition = new IsometricSpriteAnimationSetDefinition( *childElement );
		}
	}

	s_definitions[ m_name ] = this;
}

IsometricSpriteAnimationSetDefinition* ActorDefinition::GetIsometricSpriteAnimationSetDefinition() const
{
	return m_isoSpriteAnimSetDefinition;
}

Stats ActorDefinition::GetBaseStats() const
{
	return m_baseStats;
}

ActorClass ActorDefinition::GetClass() const
{
	return m_class;
}

std::vector< ActionType > ActorDefinition::GetAvailableActions() const
{
	return m_actionTypes;
}

std::string ActorDefinition::GetName() const
{
	return m_name;
}

Actor::Actor( const ActorDefinition& definition, Team team, const std::string& name, Map* map, const IntVector2& position /* = IntVector2::ZERO */ )
	:	m_definition( &definition ),
		m_team( team ),
		m_name( name ),
		m_owningMap( map ),
		m_position( position )
{
	SetStats( m_definition->m_baseStats );
	InitializeActions();
	SetRenderPositionFromMapPosition();
	m_isometricSpriteAnimSet = new IsometricSpriteAnimationSet( *m_definition->GetIsometricSpriteAnimationSetDefinition() );
	m_isometricSpriteAnimSet->SetAnimation( "Idle", m_facingDirection );
}

Actor::Actor( const std::string& actorDefinitionName, Team team, const std::string& name, Map* map, const IntVector2& position /* = IntVector2::ZERO */ )
	:	m_definition( ActorDefinition::s_definitions[ actorDefinitionName ] ),
		m_team( team ),
		m_name( name ),
		m_owningMap( map ),
		m_position( position )
{
	SetStats( m_definition->m_baseStats );
	InitializeActions();
	SetRenderPositionFromMapPosition();
	m_isometricSpriteAnimSet = new IsometricSpriteAnimationSet( *m_definition->GetIsometricSpriteAnimationSetDefinition() );
	m_isometricSpriteAnimSet->SetAnimation( "Idle", m_facingDirection );
}

Actor::~Actor()
{

}

void Actor::InitializeActions()
{
	for ( ActionType actionType : m_definition->GetAvailableActions() )
	{
		m_actions.push_back( TurnBasedAction( actionType ) );
	}
}

void Actor::SetRenderPositionFromMapPosition()
{
	m_renderPosition = m_owningMap->GetWorldPositionOnSurfaceFrom2DMapPosition( m_position );
	m_renderPosition = GetHeightCorrectedPosition( m_renderPosition );
}

void Actor::Update()
{
	g_renderer->SetCamera( g_encounterGameState->GetOrbitCamera() );	// The IsoSpriteAnimSet needs this set here to decide which sprite to render
	m_isometricSpriteAnimSet->Update( m_facingDirection );
	UpdateTrackingFlyoutText();
}

void Actor::UpdateTrackingFlyoutText()
{
	if ( m_trackingFlyoutTextKey != "" && g_encounterGameState->GetFlyoutText( m_trackingFlyoutTextKey ) != nullptr )
	{
		g_encounterGameState->GetFlyoutText( m_trackingFlyoutTextKey )->m_location = GetRenderPosition() + Vector3::UP;
	}
}

void Actor::Render()
{
	if ( !IsDead() )
	{
		g_renderer->UseShaderProgram( DEFAULT_SHADER_NAME );
		g_renderer->DrawSprite( m_isometricSpriteAnimSet->GetCurrentSprite(), m_renderPosition, g_renderer->GetCurrentCamera()->GetRight(), static_cast< float >( m_stats.m_height ), Rgba::WHITE, IsometricSpriteAnimationSet::GetScaleForFacingDirection( m_facingDirection ) );
	}
}

void Actor::SetDestination( const IntVector2& nextPosition )
{
	m_position = nextPosition;
}

void Actor::LerpTo( const Vector3& initialPosition, const Vector3& nextPosition, float lerpAmount )
{
	if ( nextPosition != initialPosition )
	{
		SetFacingDirection( nextPosition - initialPosition );
	}

	// Raw lerp the Vector3
	Vector3 intermediatePosition = Interpolate( initialPosition, nextPosition, lerpAmount );

	// Correct the height to match the map height
	IntVector2 mapPosition = m_owningMap->Get2DMapPositionFromWorldPosition( intermediatePosition );
	intermediatePosition.y =  m_owningMap->GetWorldPositionOnSurfaceFrom2DMapPosition( mapPosition ).y;
	intermediatePosition = GetHeightCorrectedPosition( intermediatePosition );

	m_renderPosition = intermediatePosition;
}

void Actor::SetStats( const Stats& stats )
{
	m_stats = Stats( stats );
	m_health = m_stats.m_maxHealth;
}

void Actor::SetWaitTime( int waitTime )
{
	m_waitTime = waitTime;
}

void Actor::SetPosition( const IntVector2& position )
{
	m_position = position;
}

void Actor::SetFacingDirection( const Vector3& facingDirection )
{
	m_facingDirection = facingDirection;
}

void Actor::TurnToward( const IntVector2& position )
{
	if ( position == m_position )
	{
		return;
	}

	Vector3 targetPos = GetOwningMap()->GetWorldPositionFrom2DMapPosition( position );
	Vector3 myPos = GetOwningMap()->GetWorldPositionFrom2DMapPosition( m_position );
	SetFacingDirection( (targetPos - myPos).GetNormalized() );
}

void Actor::SetAugmentedBlockChance( float augmentedBlockChance )
{
	m_augmentedBlockChance = ClampFloat( augmentedBlockChance, 0.0f, 1.0f );
	if ( IsFloatEqualTo( m_augmentedBlockChance, 0.0f ) )
	{
		StopDefending();
	}
}

void Actor::StopDefending()
{
	g_encounterGameState->RemoveFlyoutTextIfPresent( "defend_" + GetName() );
	m_trackingFlyoutTextKey = "";
}

void Actor::SetTrackingFlyoutTextKey( const std::string& key )
{
	m_trackingFlyoutTextKey = key;
}

void Actor::SetLastAction( ActionType action )
{
	m_lastActionUsed = action;
}

void Actor::Damage( int amount )
{
	m_health -= amount;
	m_health = ClampInt( m_health, 0, m_stats.m_maxHealth );
}

void Actor::Heal( int amount )
{
	m_health += amount;
	m_health = ClampInt( m_health, 0, m_stats.m_maxHealth );
}

void Actor::AddWaitTime( int amount )
{
	m_waitTime += amount;
	m_lastWaitAmount = amount;
}

void Actor::AdvanceTime( int amount )
{
	m_waitTime -= amount;
	if ( m_waitTime < 0 )
	{
		m_waitTime = 0;
	}
}

void Actor::DecayWait()
{
	m_waitTime -= m_stats.m_actionSpeed;
	if ( m_waitTime < 0 )
	{
		m_waitTime = 0;
	}
}

void Actor::RefreshActionsForTurn()
{
	for ( unsigned int actionIndex = 0; actionIndex < m_actions.size(); actionIndex++ )
	{
		m_actions[ actionIndex ].m_performedThisTurn = false;
	}
}

void Actor::EnableAction( ActionType type )
{
	int actionIndex = GetActionIndex( type );
	if ( actionIndex > -1 )
	{
		m_actions[ actionIndex ].m_performedThisTurn = false;
	}
}

void Actor::DisableAction( ActionType type )
{
	int actionIndex = GetActionIndex( type );
	if ( actionIndex > -1 )
	{
		m_actions[ actionIndex ].m_performedThisTurn = true;
	}
}

void Actor::DisableAllActionsButWait()
{
	for ( unsigned int actionIndex = 0; actionIndex < m_actions.size(); actionIndex++ )
	{
		if ( m_actions[ actionIndex ].m_actionType != ActionType::WAIT )
		{
			m_actions[ actionIndex ].m_performedThisTurn = true;
		}
	}
}

void Actor::UndoLastWaitAddition()
{
	m_waitTime -= m_lastWaitAmount;
}

void Actor::MarkDead()
{
	m_health = 0;
	m_isDead = true;
}

bool Actor::IsDead() const
{
	return m_isDead;
}

Team Actor::GetTeam() const
{
	return m_team;
}

const ActorDefinition* Actor::GetDefinition() const
{
	return m_definition;
}

std::string Actor::GetName() const
{
	return m_name;
}

Stats Actor::GetStats() const
{
	return m_stats;
}

int Actor::GetHealth() const
{
	return m_health;
}

int Actor::GetWaitTime() const
{
	return m_waitTime;
}

int Actor::GetTurnOrder() const
{
	if ( g_encounterGameState->GetCurrentActor() == this )
	{
		return 1;
	}

	int actorWaitNextTurn = GetWaitTime() - GetStats().m_actionSpeed;
	int turnOrder = 2;		// 1 is always reserved for the current actor
	for ( Actor* actor : g_encounterGameState->GetCurrentEncounter()->GetActors() )
	{
		if ( actor != this && actor != g_encounterGameState->GetCurrentActor() )
		{
			if ( ( actor->GetWaitTime() - actor->GetStats().m_actionSpeed ) < actorWaitNextTurn )
			{
				turnOrder++;
			}
		}
	}
	return turnOrder;
}

Vector3 Actor::GetFacingDirection() const
{
	return m_facingDirection;
}

Vector3 Actor::GetAssailantCardinalFacingDirection( const Vector3& assailantPosition ) const
{
	Vector3 assailantFacingDirection = m_renderPosition - assailantPosition;

	std::vector< Vector3 > cardinalDirections = {
		Vector3::RIGHT,
		Vector3::FORWARD,
		( Vector3::RIGHT * -1.0f ),
		( Vector3::FORWARD * -1.0f )
	};

	Vector3 strongestDirection = Vector3::ZERO;
	float strongestDotProduct = 0.0f;

	for ( Vector3 cardinalDirection : cardinalDirections )
	{
		float dotProduct = DotProduct( assailantFacingDirection, cardinalDirection );
		if ( dotProduct > strongestDotProduct )
		{
			strongestDirection = cardinalDirection;
			strongestDotProduct = dotProduct;
		}
	}

	return strongestDirection;
}

IsometricSpriteAnimationSet* Actor::GetIsometricSpriteAnimationSet() const
{
	return m_isometricSpriteAnimSet;
}

Map* Actor::GetOwningMap() const
{
	return m_owningMap;
}

IntVector2 Actor::GetPosition() const
{
	return m_position;
}

Vector3 Actor::GetRenderPosition() const
{
	return m_renderPosition;
}

Rgba Actor::GetDrawColor() const
{
	switch ( m_team )
	{
		case Team::TEAM_RED:	return Rgba::RED; break;
		case Team::TEAM_BLUE:	return Rgba::BLUE; break;
		default:				return Rgba::WHITE; break;
	}
}

Vector3 Actor::GetHeightCorrectedPosition( const Vector3& position ) const
{
	TODO( "Reevaluate this formula" )
	return ( position + Vector3( 0.0f, BLOCK_SIZE_WORLD_UNITS, 0.0f) );
}

std::string Actor::GetStatusString() const
{
	std::string statusString = Stringf(
		 "Name: %s\n"
		 "Team: %s\n"
		 "Class: %s\n"
		 "Health: %d\n"
		 "Turn Order: %d",
			m_name.c_str(),
			GetTeamNameFromTeam( GetTeam() ).c_str(),
			m_definition->GetName().c_str(),
			m_health,
			GetTurnOrder()
	);

	Actor* encounterCurrentActor = g_encounterGameState->GetCurrentActor();
	if ( encounterCurrentActor != this ) // The Actor is being targeted
	{
		statusString.append( 
			Stringf(
				"\nHit chance: %f\n"
				"Crit Chance: %f",
				( ( 1.0f - GetBlockChance( GetAssailantCardinalFacingDirection( encounterCurrentActor->GetRenderPosition() ) ) ) * 100.0f ),
				( GetCritChance( GetAssailantCardinalFacingDirection( encounterCurrentActor->GetRenderPosition() ) ) * 100.0f )
			)
		);
	}

	return statusString;
}

ActionType Actor::GetLastAction() const
{
	return m_lastActionUsed;
}

float Actor::GetBlockChance( const Vector3& assailantFacingDirection ) const
{
	float backstabDotProduct = DotProduct( assailantFacingDirection, m_facingDirection );
	// No need to check the two Actors' relative positions for the above since the assailant will always face the Actor before an attack

	float blockChance = 0.0f;

	if ( IsFloatGreaterThanOrEqualTo( backstabDotProduct, 1.0f ) )	// Assailant is in behind the Actor
	{
		blockChance = ACTOR_BLOCK_CHANCE_BACK;
	}
	else if ( IsFloatLesserThanOrEqualTo( backstabDotProduct, -1.0f ) )	// Assailant is in front of the Actor
	{
		blockChance = ACTOR_BLOCK_CHANCE_FRONT;
	}
	else if ( IsFloatEqualTo( backstabDotProduct, 0.0f ) )	// Assailant is to the side of the Actor
	{
		blockChance = ACTOR_BLOCK_CHANCE_SIDE;
	}

	blockChance = ClampFloat( ( blockChance + m_augmentedBlockChance ), 0.0f, 1.0f );

	return blockChance;
}

bool Actor::IsAttackBlocked( const Vector3& assailantFacingDirection ) const
{
	if ( CheckRandomChance( GetBlockChance( assailantFacingDirection ) ) )
	{
		return true;
	}
	else
	{
		return false;
	}
}

float Actor::GetCritChance( const Vector3& assailantFacingDirection ) const
{
	float backstabDotProduct = DotProduct( assailantFacingDirection, m_facingDirection );
	// No need to check the two Actors' relative positions for the above since the assailant will always face the Actor before an attack

	float critChance = 0.0f;

	if ( IsFloatGreaterThanOrEqualTo( backstabDotProduct, 1.0f ) )	// Assailant is in behind the Actor
	{
		critChance = ACTOR_CRIT_CHANCE_BACK;
	}
	else if ( IsFloatLesserThanOrEqualTo( backstabDotProduct, -1.0f ) )	// Assailant is in front of the Actor
	{
		critChance = ACTOR_CRIT_CHANCE_FRONT;
	}
	else if ( IsFloatEqualTo( backstabDotProduct, 0.0f ) )	// Assailant is to the side of the Actor
	{
		critChance = ACTOR_CRIT_CHANCE_SIDE;
	}

	return critChance;
}

bool Actor::IsAttackCritical( const Vector3& assailantFacingDirection ) const
{
	if ( CheckRandomChance( GetCritChance( assailantFacingDirection ) ) )
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool Actor::IsActionAvailableThisTurn( ActionType type ) const
{
	int actionIndex = GetActionIndex( type );
	return ( actionIndex != -1 && !m_actions[ actionIndex ].m_performedThisTurn );
}

std::vector< ActionType > Actor::GetActions() const
{
	std::vector< ActionType > actions;
	for ( TurnBasedAction action : m_actions )
	{
		actions.push_back( action.m_actionType );
	}
	return actions;
}

ActionType Actor::GetNextAvailableAction( ActionType currentAction ) const
{
	ActionType nextActionType = ActionType::WAIT;
	int actionIndex = GetActionIndex( currentAction );

	if ( actionIndex != -1 )
	{
		for ( int itemIndex = ( ( actionIndex + 1 ) % static_cast< int >( m_actions.size() ) ); itemIndex != actionIndex; itemIndex = ( ( itemIndex + 1 ) % static_cast< int >( m_actions.size() ) ) )
		{
			if ( IsActionAvailableThisTurn( m_actions[ itemIndex ].m_actionType ) )
			{
				nextActionType = m_actions[ itemIndex ].m_actionType;
				break;
			}
		}
	}

	return nextActionType;
}

ActionType Actor::GetPreviousAvailableAction( ActionType currentAction ) const
{
	ActionType previousActionType = ActionType::WAIT;
	int actionIndex = GetActionIndex( currentAction );

	if ( actionIndex != -1 )
	{
		for ( int itemIndex = ( ( actionIndex > 0 )? ( actionIndex - 1 ) : static_cast< int >( m_actions.size() - 1 ) ); itemIndex != actionIndex; itemIndex = ( ( itemIndex > 0 )? ( itemIndex - 1 ) : static_cast< int >( m_actions.size() - 1 ) ) )
		{
			if ( IsActionAvailableThisTurn( m_actions[ itemIndex ].m_actionType ) )
			{
				previousActionType = m_actions[ itemIndex ].m_actionType;
				break;
			}
		}
	}
	
	return previousActionType;
}

int Actor::GetActionIndex( ActionType type ) const
{
	int foundIndex = -1;
	for ( unsigned int actionIndex = 0; actionIndex < m_actions.size(); actionIndex++ )
	{
		if ( m_actions[ actionIndex ].m_actionType == type )
		{
			foundIndex = static_cast< int >( actionIndex );
			break;
		}
	}
	return foundIndex;
}
