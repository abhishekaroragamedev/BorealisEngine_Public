#pragma once

#include "Game/Entities/Entity.hpp"
#include "Game/World/AdventureDefinition.hpp"
#include "Game/World/Map.hpp"
#include <map>
#include <string>
#include <vector>

struct VictoryConditionEntity
{

public:
	VictoryConditionEntity( Entity* entity, VictoryConditionType victoryConditionType );

public:
	Entity* m_entity = nullptr;
	VictoryConditionType m_victoryConditionType = VictoryConditionType::NO_VICTORY_CONDITION;

};

class Adventure
{

public:
	Adventure( AdventureDefinition* adventureDefinition );
	~Adventure();

public:
	void Update( float deltaSeconds );
	void Render( float renderAlpha, bool developerModeEnabled ) const;
	void LoadMapAndMovePlayer( const Map& mapToLoad, Player* playerEntity );
	bool HasPlayerWon() const;
	AdventureDefinition* GetAdventureDefinition() const;
	Map* GetCurrentMap() const;

private:
	void PopulateFromDefinition();
	void DeleteMaps();
	void DeleteVictoryConditions();
	bool IsVictoryConditionSatisfied( const VictoryConditionEntity& victoryCondition ) const;

private:
	AdventureDefinition* m_adventureDefinition = nullptr;
	std::vector< VictoryConditionEntity* > m_victoryConditions;
	std::map< std::string, Map* > m_maps;
	Map* m_currentMap = nullptr;


};
