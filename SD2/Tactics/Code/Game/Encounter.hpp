#pragma once

#include "Game/Actor.hpp"
#include "Game/World/Map.hpp"
#include "Engine/Core/XmlUtilities.hpp"
#include "Engine/Math/IntVector2.hpp"
#include "ThirdParty/tinyXML2/tinyxml2.h"
#include <string>
#include <vector>

constexpr int MAX_WAIT_TIME = 999999;
constexpr float VISIBLE_CURSOR_RENDER_ALPHA = 0.6f;
constexpr float HIDDEN_CURSOR_RENDER_ALPHA = 0.25f;
constexpr float HIDDEN_CURRENT_CURSOR_RENDER_ALPHA = 0.4f;

struct EncounterMapDefinitionStruct
{

public:
	EncounterMapDefinitionStruct( const tinyxml2::XMLElement& mapElement );

public:
	std::string m_fileName = "";
	unsigned int m_maxHeight = 8U;

};

struct EncounterActorDefinitionStruct
{

public:
	EncounterActorDefinitionStruct( const tinyxml2::XMLElement& actorElement );

public:
	const ActorDefinition* m_definition = nullptr;
	Team m_team = Team::TEAM_INVALID;
	std::string m_name = "";
	IntVector2 m_startPosition = IntVector2::ZERO;

};

class EncounterDefinition
{

	friend class Encounter;

public:
	EncounterDefinition( const tinyxml2::XMLElement& encounterDefinitionElement );
	~EncounterDefinition();

public:
	static std::map< std::string, EncounterDefinition* > s_definitions;

private:
	EncounterMapDefinitionStruct* m_mapDefinition = nullptr;
	std::vector< EncounterActorDefinitionStruct* > m_actorDefinitions;
	std::string m_name = "";

};

class Encounter
{

	friend class EncounterGameState;

public:
	Encounter( const std::string& firstMapToLoad, unsigned int maxHeight );
	Encounter( const EncounterDefinition& encounterDefinition );
	Encounter( const std::string& encounterDefinitionName );
	~Encounter();

public:
	bool LoadMap( const std::string& fileName, unsigned int maxHeight = 8U );
	bool AddActor( const ActorDefinition* actorDefinition, Team team, const std::string& name, const IntVector2& coordinates );
	void DestroyAllActors();

	Team CheckGameOverAndGetWinningTeam() const;
	Map* GetMap() const;
	unsigned int GetActorCount()const;
	Actor* GetActor( unsigned int actorIndex ) const;
	Actor* GetActorWithName( const std::string& name ) const;
	std::vector< Actor* > GetActors() const;
	Actor* GetActorAtPosition( const IntVector2& position ) const;
	bool CanPositionBeMovedInto( const IntVector2& position ) const;
	Actor* AdvanceTimeToNextTurnAndGetNextActor();
	std::vector< IntVector2 > GetAttackableTiles( const Actor* attacker ) const;
	std::vector< IntVector2 > GetOccupiedTilesInRange( const IntVector2& origin, const IntRange& range, const IntRange& verticalRange ) const;
	std::vector< IntVector2 > GetOccupiedTilesInRange( const Actor* attacker, const IntRange& range, const IntRange& verticalRange ) const;

private:
	void LoadActorsFromDefinition();
	void DecayAllActorsWaitTime();	// Updates each actor's wait according to its action speed, and sets the wait to the minimum wait value if below that value
	void PushCurrentActorToBack();

private:
	const EncounterDefinition* m_definition = nullptr;
	Map* m_map = nullptr;
	std::vector< Actor* > m_actors;

};
