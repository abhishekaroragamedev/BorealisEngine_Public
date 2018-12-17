#pragma once

#include "Game/Entities/ActorDefinition.hpp"
#include "Game/Entities/ItemDefinition.hpp"
#include "Game/World/TileDefinition.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Math/IntRange.hpp"
#include "ThirdParty/tinyXML2/tinyxml2.h"
#include <string>

class Map;

class MapGenStep
{

public:
	explicit MapGenStep( const tinyxml2::XMLElement& genStepXmlElement );
	~MapGenStep();

	virtual void Run( Map& map ) const = 0;

public:
	static MapGenStep* CreateMapGenStep( const tinyxml2::XMLElement& genStepXmlElement );

private:
	static constexpr char NAME_XML_ATTRIBUTE_NAME[] = "name";
	static constexpr char NUM_ITERATIONS_XML_ATTRIBUTE_NAME[] = "iterations";
	static constexpr char CHANCE_TO_RUN_XML_ATTRIBUTE_NAME[] = "chanceToRun";
	static constexpr char GEN_STEP_FILL_AND_EDGE_NAME[] = "FillAndEdge";
	static constexpr char GEN_STEP_FROM_FILE_NAME[] = "FromFile";
	static constexpr char GEN_STEP_MUTATE_NAME[] = "Mutate";
	static constexpr char GEN_STEP_SPAWN_ACTOR_NAME[] = "SpawnActor";
	static constexpr char GEN_STEP_SPAWN_ITEM_NAME[] = "SpawnItem";

protected:
	std::string m_name = "";
	IntRange m_numIterations = IntRange( 1, 1 );
	float m_chanceToRun = 1.0f;

};

class MapGenStep_FillAndEdge : public MapGenStep
{

public:
	explicit MapGenStep_FillAndEdge( const tinyxml2::XMLElement& genStepXmlElement );
	~MapGenStep_FillAndEdge();

	void Run( Map& map ) const override;

private:
	static constexpr char FILL_TILE_XML_ATTRIBUTE_NAME[] = "fillTile";
	static constexpr char EDGE_TILE_XML_ATTRIBUTE_NAME[] = "edgeTile";
	static constexpr char EDGE_THICKNESS_XML_ATTRIBUTE_NAME[] = "edgeThickness";

private:
	TileDefinition* m_fillTileDefinition = nullptr;
	TileDefinition* m_edgeTileDefinition = nullptr;
	int m_edgeThickness = 1;

};

class MapGenStep_FromFile : public MapGenStep
{

public:
	explicit MapGenStep_FromFile( const tinyxml2::XMLElement& genStepXmlElement );
	~MapGenStep_FromFile();

	void Run( Map& map ) const override;

private:
	static constexpr char IMAGE_FILE_PATH_XML_ATTRIBUTE_NAME[] = "imageFile";

private:
	Image* m_mapLayoutImage = nullptr;

};

class MapGenStep_Mutate : public MapGenStep
{

public:
	explicit MapGenStep_Mutate( const tinyxml2::XMLElement& genStepXmlElement );
	~MapGenStep_Mutate();

	void Run( Map& map ) const override;

private:
	static constexpr char OLD_TILE_XML_ATTRIBUTE_NAME[] = "oldTile";
	static constexpr char NEW_TILE_XML_ATTRIBUTE_NAME[] = "newTile";
	static constexpr char MUTATE_CHANCE_XML_ATTRIBUTE_NAME[] = "mutationChance";

private:
	TileDefinition* m_oldTileDefinition = nullptr;
	TileDefinition* m_newTileDefinition = nullptr;
	float m_mutationChance = 1.0f;

};

class MapGenStep_SpawnActor : public MapGenStep
{

public:
	explicit MapGenStep_SpawnActor( const tinyxml2::XMLElement& genStepXmlElement );
	~MapGenStep_SpawnActor();

	void Run( Map& map ) const override;

private:
	static constexpr char ACTOR_TYPE_XML_ATTRIBUTE_NAME[] = "type";
	static constexpr char ON_TILE_XML_ATTRIBUTE_NAME[] = "onTile";

private:
	ActorDefinition* m_actorToSpawn = nullptr;
	TileDefinition* m_spawnOnTile = nullptr;

};

class MapGenStep_SpawnItem : public MapGenStep
{

public:
	explicit MapGenStep_SpawnItem( const tinyxml2::XMLElement& genStepXmlElement );
	~MapGenStep_SpawnItem();

	void Run( Map& map ) const override;

private:
	static constexpr char ITEM_TYPE_XML_ATTRIBUTE_NAME[] = "type";
	static constexpr char ON_TILE_XML_ATTRIBUTE_NAME[] = "onTile";

private:
	ItemDefinition* m_itemToSpawn = nullptr;
	TileDefinition* m_spawnOnTile = nullptr;

};
