#include "Game/TheGame.hpp"
#include "Game/GameCommon.hpp"
#include "Game/TacticsNetMessage.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/XmlUtilities.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Networking/NetMessage.hpp"
#include "Engine/Networking/NetSession.hpp"
#include "Engine/Tools/DevConsole.hpp"
#include "ThirdParty/tinyXML2/tinyxml2.h"

TheGame::TheGame()
{
	m_defaultFont = g_renderer->CreateOrGetBitmapFont( DEFAULT_FONT_NAME );
	InitializeGameStates();

	NetSession::GetInstance()->RegisterMessage( NETMSG_GAME_JOIN_ACCEPT, "game_join_accept", OnGameJoinAccept );
	// All actions use same channel since they must be executed in a sequence
	NetSession::GetInstance()->RegisterMessage( NETMSG_MOVE, "net_move", OnNetMove, ( NETMSG_OPTION_RELIABLE | NETMSG_OPTION_IN_ORDER ) );
	NetSession::GetInstance()->RegisterMessage( NETMSG_ATTACK, "net_attack", OnNetAttack, ( NETMSG_OPTION_RELIABLE | NETMSG_OPTION_IN_ORDER ) );
	NetSession::GetInstance()->RegisterMessage( NETMSG_BOW, "net_bow", OnNetBow, ( NETMSG_OPTION_RELIABLE | NETMSG_OPTION_IN_ORDER ) );
	NetSession::GetInstance()->RegisterMessage( NETMSG_HEAL, "net_heal", OnNetHeal, ( NETMSG_OPTION_RELIABLE | NETMSG_OPTION_IN_ORDER ) );
	NetSession::GetInstance()->RegisterMessage( NETMSG_DEFEND, "net_defend", OnNetDefend, ( NETMSG_OPTION_RELIABLE | NETMSG_OPTION_IN_ORDER ) );
	NetSession::GetInstance()->RegisterMessage( NETMSG_FIRE, "net_fire", OnNetCastFire, ( NETMSG_OPTION_RELIABLE | NETMSG_OPTION_IN_ORDER ) );
	NetSession::GetInstance()->RegisterMessage( NETMSG_WAIT, "net_wait", OnNetWait, ( NETMSG_OPTION_RELIABLE | NETMSG_OPTION_IN_ORDER ) );
}

void TheGame::InitializeGameStates()
{
	m_gameStates[ GameStateType::STATE_LOADING ] = new LoadingGameState();
	m_gameStates[ GameStateType::STATE_MENU ] = new MenuGameState();
	g_encounterGameState = new EncounterGameState();
	m_gameStates[ GameStateType::STATE_ENCOUNTER ] = g_encounterGameState;
	
	m_currentGameState = GameStateType::STATE_LOADING;
	m_gameStates[ m_currentGameState ]->Enter();
}

TheGame::~TheGame()
{
	g_encounterGameState = nullptr;

	delete m_itemsSpriteSheet;
	m_itemsSpriteSheet = nullptr;
	delete m_terrainSpriteSheet;
	m_terrainSpriteSheet = nullptr;

	DeleteGameStates();
	DeleteEncounterDefinitions();
	DeleteBlockDefinitions();
	DeleteActorDefinitions();
}

SpriteSheet* TheGame::GetTerrainTextureAtlas() const
{
	return m_terrainSpriteSheet;
}

SpriteSheet* TheGame::GetItemsTextureAtlas() const
{
	return m_itemsSpriteSheet;
}

void TheGame::DeleteGameStates()
{
	for ( int gameState = GameStateType::STATE_LOADING; gameState < GameStateType::NUM_STATES; gameState++ )
	{
		delete m_gameStates[ gameState ];
		m_gameStates[ gameState ] = nullptr;
	}
}

void TheGame::DeleteActorDefinitions()
{
	for ( std::map< std::string, ActorDefinition* >::iterator actorIterator = ActorDefinition::s_definitions.begin(); actorIterator != ActorDefinition::s_definitions.end(); actorIterator++ )
	{
		delete actorIterator->second;
		actorIterator->second = nullptr;
	}
}

void TheGame::DeleteBlockDefinitions()
{
	for ( std::map< std::string, BlockDefinition* >::iterator blockIterator = BlockDefinition::s_definitions.begin(); blockIterator != BlockDefinition::s_definitions.end(); blockIterator++ )
	{
		delete blockIterator->second;
		blockIterator->second = nullptr;
	}
}

void TheGame::DeleteEncounterDefinitions()
{
	for ( std::map< std::string, EncounterDefinition* >::iterator encounterIterator = EncounterDefinition::s_definitions.begin(); encounterIterator != EncounterDefinition::s_definitions.end(); encounterIterator++ )
	{
		delete encounterIterator->second;
		encounterIterator->second = nullptr;
	}
}

void TheGame::LoadResources()
{
	m_terrainSpriteSheet = new SpriteSheet( *g_renderer->CreateOrGetTexture( TERRAIN_TEXTURE_PATH ), 32, 32 );	// Terrain texture
	RegisterBlockType( "grass", m_terrainSpriteSheet->GetTextureCoordinatesForSpriteCoordinates( IntVector2( 22, 30 ) ), m_terrainSpriteSheet->GetTextureCoordinatesForSpriteCoordinates( IntVector2( 3, 28 ) ), m_terrainSpriteSheet->GetTextureCoordinatesForSpriteCoordinates( IntVector2( 25, 30 ) ) );

	m_itemsSpriteSheet = new SpriteSheet( *g_renderer->CreateOrGetTexture( ITEMS_TEXTURE_PATH ), 13, 14, true );

	tinyxml2::XMLDocument gameConfigDocument;
	gameConfigDocument.LoadFile( GAME_CONFIG_XML_PATH );
	g_gameConfigBlackboard.PopulateFromXmlElementAttributes( *gameConfigDocument.FirstChildElement() );

	LoadActorDefinitions();
	LoadEncounterDefinitions();
}

void TheGame::LoadActorDefinitions()
{
	tinyxml2::XMLDocument actorDefinitionsDocument;
	actorDefinitionsDocument.LoadFile( ACTOR_DEFINITIONS_XML_PATH );
	for ( tinyxml2::XMLElement* actorDefinitionElement = actorDefinitionsDocument.FirstChildElement()->FirstChildElement(); actorDefinitionElement != nullptr; actorDefinitionElement = actorDefinitionElement->NextSiblingElement() )
	{
		new ActorDefinition( *actorDefinitionElement );
	}
}

void TheGame::LoadEncounterDefinitions()
{
	tinyxml2::XMLDocument encounterDefinitionsDocument;
	encounterDefinitionsDocument.LoadFile( ENCOUNTER_DEFINITIONS_XML_PATH );
	for ( tinyxml2::XMLElement* encounterDefinitionElement = encounterDefinitionsDocument.FirstChildElement()->FirstChildElement(); encounterDefinitionElement != nullptr; encounterDefinitionElement = encounterDefinitionElement->NextSiblingElement() )
	{
		new EncounterDefinition( *encounterDefinitionElement );
	}
}

BlockDefinition* TheGame::RegisterBlockType( const std::string& name, const AABB2& tileTopUV /* = AABB2::ZERO_TO_ONE */, const AABB2& tileSideUV /* = AABB2::ZERO_TO_ONE */, const AABB2& tileBottomUV /* = AABB2::ZERO_TO_ONE */ )
{
	BlockDefinition* newBlockDefinition = new BlockDefinition( name, const_cast< Texture* >( m_terrainSpriteSheet->GetTexture() ), tileTopUV, tileSideUV, tileBottomUV );
	// Deletion is taken care of in TheGame::DeleteBlockDefinitions
	return newBlockDefinition;
}

GameState* TheGame::GetGameStateInstance( GameStateType stateType )
{
	return m_gameStates[ stateType ];
}

GameStateType TheGame::GetCurrentGameState() const
{
	return m_currentGameState;
}

void TheGame::Update()
{
	TransitionToNextGameState();
	if ( !IsDevConsoleOpen() )
	{
		HandleKeyboardInput();
		HandleXboxControllerInputs();
	}
	m_gameStates[ m_currentGameState ]->Update();
}

void TheGame::ChangeGameState( GameStateType nextGameState )
{
	m_nextGameState = nextGameState;
}

void TheGame::SetCanJoin( bool canJoin )
{
	m_canJoin = canJoin;
}

void TheGame::LoadEncounter( const EncounterDefinition& encounterDefinition, bool isNetworked/* =false */, bool isHost/* =false */ )
{
	switch( m_currentGameState )
	{
		case GameStateType::STATE_MENU:
		{
			g_encounterGameState->LoadEncounter( encounterDefinition );
			ChangeGameState( GameStateType::STATE_ENCOUNTER );
		}
		case GameStateType::STATE_ENCOUNTER:
		{
			ChangeGameState( GameStateType::STATE_MENU );
			TransitionToNextGameState();
			g_encounterGameState->LoadEncounter( encounterDefinition );
			g_encounterGameState->SetNetworkingProperties( isNetworked, isHost );
			ChangeGameState( GameStateType::STATE_ENCOUNTER );
		}
	}
}

void TheGame::TransitionToNextGameState()
{
	if ( ( m_nextGameState != GameStateType::STATE_INVALID ) && ( m_nextGameState != m_currentGameState ) )
	{
		m_gameStates[ m_currentGameState ]->Exit();
		m_currentGameState = m_nextGameState;
		m_gameStates[ m_currentGameState ]->Enter();
	}
}

void TheGame::Render() const
{
	m_gameStates[ m_currentGameState ]->Render();
}

void TheGame::HandleKeyboardInput()
{

}

void TheGame::HandleXboxControllerInputs()
{
	
}
