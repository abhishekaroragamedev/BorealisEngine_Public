#include "Game/Encounter.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Math/IntRange.hpp"
#include "Engine/Tools/DevConsole.hpp"

EncounterMapDefinitionStruct::EncounterMapDefinitionStruct( const tinyxml2::XMLElement& mapElement )
{
	m_fileName = ParseXmlAttribute( mapElement, "fileName", m_fileName );
	m_maxHeight = static_cast< unsigned int >( ParseXmlAttribute( mapElement, "maxHeight", static_cast< int >( m_maxHeight ) ) );
}

EncounterActorDefinitionStruct::EncounterActorDefinitionStruct( const tinyxml2::XMLElement& actorElement )
{
	m_definition = ActorDefinition::s_definitions[ ParseXmlAttribute( actorElement, "definition", "" ) ];
	m_name = ParseXmlAttribute( actorElement, "name", m_name );
	m_team = GetTeamFromTeamName( ParseXmlAttribute( actorElement, "team", "" ) );
	m_startPosition = ParseXmlAttribute( actorElement, "position", m_startPosition );
}

std::map< std::string, EncounterDefinition* > EncounterDefinition::s_definitions;

EncounterDefinition::EncounterDefinition( const tinyxml2::XMLElement& encounterDefinitionElement )
{
	m_name = ParseXmlAttribute( encounterDefinitionElement, "name", m_name );
	for ( const tinyxml2::XMLElement* childElement = encounterDefinitionElement.FirstChildElement(); childElement != nullptr; childElement = childElement->NextSiblingElement() )
	{
		if ( std::string( childElement->Name() ) == "Map" )
		{
			m_mapDefinition = new EncounterMapDefinitionStruct( *childElement );
		}
		else if ( std::string( childElement->Name() ) == "Actors" )
		{
			for ( const tinyxml2::XMLElement* actorElement = childElement->FirstChildElement(); actorElement != nullptr; actorElement = actorElement->NextSiblingElement() )
			{
				m_actorDefinitions.push_back( new EncounterActorDefinitionStruct( *actorElement ) );
			}
		}
	}

	EncounterDefinition::s_definitions[ m_name ] = this;
}

EncounterDefinition::~EncounterDefinition()
{
	delete m_mapDefinition;
	m_mapDefinition = nullptr;

	for ( std::vector< EncounterActorDefinitionStruct* >::iterator actorDefStructIterator = m_actorDefinitions.begin(); actorDefStructIterator != m_actorDefinitions.end(); actorDefStructIterator++ )
	{
		delete *actorDefStructIterator;
		*actorDefStructIterator = nullptr;
	}
}

Encounter::Encounter( const std::string& firstMapToLoad, unsigned int maxHeight /*= 8U*/ )
{
	LoadMap( firstMapToLoad, maxHeight );
}

Encounter::Encounter( const EncounterDefinition& encounterDefinition )
	:	m_definition( &encounterDefinition )
{
	LoadMap( m_definition->m_mapDefinition->m_fileName, m_definition->m_mapDefinition->m_maxHeight );
	LoadActorsFromDefinition();
}

Encounter::Encounter( const std::string& encounterDefinitionName )
{
	m_definition = EncounterDefinition::s_definitions[ encounterDefinitionName ];
	LoadMap( m_definition->m_mapDefinition->m_fileName, m_definition->m_mapDefinition->m_maxHeight );
	LoadActorsFromDefinition();
}

Encounter::~Encounter()
{
	delete m_map;
	m_map = nullptr;
}

bool Encounter::LoadMap( const std::string& fileName, unsigned int maxHeight /*= 8U*/ )
{
	if ( m_map != nullptr )
	{
		delete m_map;
		m_map = nullptr;
	}
	m_map = new Map( fileName, maxHeight );
	return true;
}

void Encounter::LoadActorsFromDefinition()
{
	for ( EncounterActorDefinitionStruct* actorDefStruct : m_definition->m_actorDefinitions )
	{
		AddActor( actorDefStruct->m_definition, actorDefStruct->m_team, actorDefStruct->m_name, actorDefStruct->m_startPosition );
	}
}

bool Encounter::AddActor( const ActorDefinition* actorDefinition, Team team, const std::string& name, const IntVector2& coordinates )
{
	if ( !CanPositionBeMovedInto( coordinates ) )
	{
		ConsolePrintf( Rgba::RED, "ERROR: add_actor: An actor is already present at the provided coordinates. Try again." );
		return false;
	}
	m_actors.push_back( new Actor( *actorDefinition, team, name, m_map, coordinates ) );
	return true;
}

void Encounter::DecayAllActorsWaitTime()
{
	for ( std::vector< Actor* >::iterator actorIterator = m_actors.begin(); actorIterator != m_actors.end(); actorIterator++ )
	{
		Actor* currentActor = *actorIterator;
		currentActor->DecayWait();
		if ( currentActor->GetWaitTime() < ACTOR_MIN_WAIT_TIME )
		{
			currentActor->SetWaitTime( ACTOR_MIN_WAIT_TIME );
		}
	}
}

void Encounter::DestroyAllActors()
{
	for ( std::vector< Actor* >::iterator actorIterator = m_actors.begin(); actorIterator != m_actors.end(); actorIterator++ )
	{
		delete *actorIterator;
	}
	m_actors.clear();
}

Team Encounter::CheckGameOverAndGetWinningTeam() const
{
	int teamAliveTally[ Team::NUM_TEAMS ];
	teamAliveTally[ Team::TEAM_RED ] = 0;
	teamAliveTally[ Team::TEAM_BLUE ] = 0;

	for ( std::vector< Actor* >::const_iterator actorIterator = m_actors.begin(); actorIterator != m_actors.end(); actorIterator++ )
	{
		if ( !( *actorIterator )->IsDead() )
		{
			teamAliveTally[ ( *actorIterator )->GetTeam() ]++;
		}
	}

	int numTeamsSurviving = 0;
	Team winningTeam = Team::TEAM_INVALID;
	for ( int teamIndex = 0; teamIndex < Team::NUM_TEAMS; teamIndex++ )
	{
		if ( teamAliveTally[ teamIndex ] > 0 )
		{
			numTeamsSurviving++;
			winningTeam = static_cast< Team >( teamIndex );
		}
		if ( numTeamsSurviving > 1 )
		{
			winningTeam = Team::TEAM_INVALID;
			break;
		}
	}

	return winningTeam;
}

Map* Encounter::GetMap() const
{
	return m_map;
}

unsigned int Encounter::GetActorCount() const
{
	return static_cast< unsigned int >( m_actors.size() );
}

Actor* Encounter::GetActor( unsigned int actorIndex ) const
{
	return m_actors[ actorIndex ];
}

Actor* Encounter::GetActorWithName( const std::string& name ) const
{
	Actor* foundActor = nullptr;
	for ( Actor* actor : m_actors )
	{
		if ( actor->GetName() == name )
		{
			foundActor = actor;
			break;
		}
	}
	return foundActor;
}

std::vector< Actor* > Encounter::GetActors() const
{
	return m_actors;
}

Actor* Encounter::GetActorAtPosition( const IntVector2& position ) const
{
	Actor* actorAtPosition = nullptr;

	for ( std::vector< Actor* >::const_iterator actorIterator = m_actors.begin(); actorIterator != m_actors.end(); actorIterator++ )
	{
		if ( ( *actorIterator )->GetPosition() == position )
		{
			actorAtPosition = *actorIterator;
			break;
		}
	}

	return actorAtPosition;
}

bool Encounter::CanPositionBeMovedInto( const IntVector2& position ) const
{
	Actor* actorAtPosition = GetActorAtPosition( position );
	return ( actorAtPosition == nullptr || actorAtPosition->IsDead() );
}

Actor* Encounter::AdvanceTimeToNextTurnAndGetNextActor()
{
	PushCurrentActorToBack();
	DecayAllActorsWaitTime();

	Actor* actorWithLeastWait = nullptr;
	int leastWaitAmount = MAX_WAIT_TIME;

	for ( std::vector< Actor* >::iterator actorIterator = m_actors.begin(); actorIterator != m_actors.end(); actorIterator++ )
	{
		Actor* currentActor = *actorIterator;
		if ( !currentActor->IsDead() && currentActor->GetWaitTime() < leastWaitAmount )
		{
			actorWithLeastWait = *actorIterator;
			leastWaitAmount = actorWithLeastWait->GetWaitTime();
		}
	}

	if ( leastWaitAmount < MAX_WAIT_TIME )
	{
		for ( std::vector< Actor* >::iterator actorIterator = m_actors.begin(); actorIterator != m_actors.end(); actorIterator++ )
		{
			( *actorIterator )->AdvanceTime( leastWaitAmount );
		}

		g_encounterGameState->AdvanceTimeForActiveActions( leastWaitAmount );
	}

	return actorWithLeastWait;
}

void Encounter::PushCurrentActorToBack()
{
	Actor* currentActor = g_encounterGameState->GetCurrentActor();

	if ( currentActor != nullptr )
	{
		for ( size_t actorIndex = 0; actorIndex < m_actors.size(); actorIndex++ )
		{
			if ( m_actors[ actorIndex ] == currentActor )
			{
				m_actors.erase( m_actors.begin() + actorIndex );
				m_actors.push_back( currentActor );
				break;
			}
		}
	}
}

std::vector< IntVector2 > Encounter::GetAttackableTiles( const Actor* attacker ) const
{
	std::vector< IntVector2 > attackableTiles;
	attackableTiles.push_back( attacker->GetPosition() );

	for ( IntVector2 neighboringTile : m_map->GetNeighboringTiles( attacker->GetPosition() ) )
	{
		Actor* actorAtNeighboringTile = GetActorAtPosition( neighboringTile );
		if ( actorAtNeighboringTile != nullptr && !actorAtNeighboringTile->IsDead() )
		{
			IntRange attackerReach = IntRange( ( m_map->GetHeight( attacker->GetPosition() ) - 1 ), ( m_map->GetHeight( attacker->GetPosition() ) + attacker->GetStats().m_height + 1 ) );
			IntRange actorReach = IntRange( m_map->GetHeight( actorAtNeighboringTile->GetPosition() ), ( m_map->GetHeight( actorAtNeighboringTile->GetPosition() ) + actorAtNeighboringTile->GetStats().m_height ) );
			if ( DoRangesOverlap( attackerReach, actorReach ) )
			{
				attackableTiles.push_back( neighboringTile );
			}
		}
	}

	return attackableTiles;
}

std::vector< IntVector2 > Encounter::GetOccupiedTilesInRange( const IntVector2& origin, const IntRange& range, const IntRange& verticalRange ) const
{
	std::vector< IntVector2 > tiles;

	for ( int x = 0; x < m_map->GetWidth(); x++ )
	{
		for ( int y = 0; y < m_map->GetDepth(); y++ )
		{
			IntVector2 tile = IntVector2( x, y );
			if ( !CanPositionBeMovedInto( tile ) )
			{
				IntVector2 distance = origin - tile;
				int manhattanDistance = Abs( distance.x ) + Abs( distance.y );
				if ( manhattanDistance >= range.min && manhattanDistance <= range.max )
				{
					int verticalDistance = m_map->GetHeight( tile ) - m_map->GetHeight( origin );
					if ( verticalDistance >= verticalRange.min && verticalDistance <= verticalRange.max )
					{
						tiles.push_back( tile );
					}
				}
			}
		}
	}

	return tiles;
}

std::vector< IntVector2 > Encounter::GetOccupiedTilesInRange( const Actor* attacker, const IntRange& range, const IntRange& verticalRange ) const
{
	std::vector< IntVector2 > tiles;
	
	for ( int x = 0; x < m_map->GetWidth(); x++ )
	{
		for ( int y = 0; y < m_map->GetDepth(); y++ )
		{
			IntVector2 tile = IntVector2( x, y );
			if ( !CanPositionBeMovedInto( tile ) )
			{
				IntVector2 distance = attacker->GetPosition() - tile;
				int manhattanDistance = Abs( distance.x ) + Abs( distance.y );
				if ( manhattanDistance >= range.min && manhattanDistance <= range.max )
				{
					int verticalDistance = m_map->GetHeight( tile ) - m_map->GetHeight( attacker->GetPosition() );
					if ( verticalDistance >= verticalRange.min && verticalDistance <= verticalRange.max )
					{
						tiles.push_back( tile );
					}
				}
			}
		}
	}

	return tiles;
}
