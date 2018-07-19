#include "Game/GameCommon.hpp"
#include "Game/World/TheWorld.hpp"

TheWorld::TheWorld()
{
	m_terrainTexture = g_renderer->CreateOrGetTexture( std::string( TERRAIN_TEXTURE_NAME ) );
	m_explosionTexture = g_renderer->CreateOrGetTexture( std::string( EXPLOSION_TEXTURE_NAME ) );
	m_terrainSpritesheet = new SpriteSheet( *m_terrainTexture, TERRAIN_SPRITE_SHEET_NUM_SPRITES_HORIZONTAL, TERRAIN_SPRITE_SHEET_NUM_SPRITES_VERTICAL );
	m_explosionSpritesheet = new SpriteSheet( *m_explosionTexture, EXPLOSION_SPRITE_SHEET_NUM_SPRITES_HORIZONTAL, EXPLOSION_SPRITE_SHEET_NUM_SPRITES_VERTICAL );
	m_currentMap = nullptr;
	m_playerTank = new PlayerTank( Vector2( 0.0f, 0.0f ) );
	m_gamePaused = false;
	m_hasPlayerWon = false;
	m_deltaSecondsMultiplier = 1.0f;
	m_playerSoundMuteFraction = m_playerTank->GetFractionOfHealthRemaining();
	m_playerHealthPulseFraction = m_playerTank->GetFractionOfDamageDone();
	m_framesSinceWorldInitialized = 0;

	CreateTileDefinitions();
	CreateMapOne();
	CreateMapTwo();
	CreateMapThree();

	InitializeTheWorld();
}

TheWorld::~TheWorld()
{
	DeleteAllMapReferences();
	DeleteTileDefinitions();
	delete m_terrainSpritesheet;
	delete m_explosionSpritesheet;
	delete m_playerTank;
	m_terrainSpritesheet = nullptr;
	m_explosionSpritesheet = nullptr;
	m_terrainTexture = nullptr;
	m_explosionTexture = nullptr;
	m_currentMap = nullptr;		// Since deleting all the map references will also delete the current map, we don't need to invoke delete on it again
	m_playerTank = nullptr;
}

void TheWorld::InitializeTheWorld()
{
	m_gamePaused = false;
	m_hasPlayerWon = false;
	m_deltaSecondsMultiplier = 1.0f;
	m_framesSinceWorldInitialized = 0;
	m_playerSoundMuteFraction = m_playerTank->GetFractionOfHealthRemaining();
	m_playerHealthPulseFraction = m_playerTank->GetFractionOfDamageDone();
	m_playerTank->Respawn();
	LoadMap( MAP_ONE_NAME );
}

void TheWorld::FlushSoundsPlaying()
{
	for ( std::vector<SoundPlaybackID>::iterator soundIterator = m_playbackIdsOfSoundsPlaying.begin(); soundIterator != m_playbackIdsOfSoundsPlaying.end(); soundIterator++ )
	{
		g_audioSystem->StopSound( *soundIterator );
	}

	m_playbackIdsOfSoundsPlaying.clear();
}

void TheWorld::AddSoundPlaying( SoundPlaybackID newSound )
{
	m_playbackIdsOfSoundsPlaying.push_back( newSound );
}

void TheWorld::SetCurrentMap( std::string mapName )
{
	m_currentMap = m_mapsByName[ mapName ];
}

Map* TheWorld::GetCurrentMap() const
{
	return m_currentMap;
}

PlayerTank* TheWorld::GetPlayerTank() const
{
	return m_playerTank;
}

SpriteSheet* TheWorld::GetExplosionSpritesheet() const
{
	return m_explosionSpritesheet;
}

bool TheWorld::IsGamePaused() const
{
	return m_gamePaused;
}

bool TheWorld::HasPlayerWon() const
{
	return m_hasPlayerWon;
}

float TheWorld::GetPlayerSoundMuteFraction() const
{
	return SmoothStart4( m_playerSoundMuteFraction );
}

float TheWorld::GetPlayerHealthPulseFraction() const
{
	return SmoothStop3( m_playerHealthPulseFraction );
}

void TheWorld::Update( float deltaSeconds )
{
	HandleKeyboardInput( deltaSeconds );
	HandleXboxControllerInput( deltaSeconds );

	bool hasPlayerCompletedLevel = m_currentMap->Update( deltaSeconds );

	if ( hasPlayerCompletedLevel )
	{
		LoadNextMap();
	}

	UpdatePlayerSoundMuteFraction();
	UpdatePlayerHealthPulseFraction();

	m_framesSinceWorldInitialized++;
}

void TheWorld::Render() const
{
	m_currentMap->Render( m_developerModeEnabled );
}

void TheWorld::UpdatePlayerSoundMuteFraction()
{
	float targetHearingLevel = m_playerTank->GetFractionOfHealthRemaining();
	m_playerSoundMuteFraction =  Interpolate( m_playerSoundMuteFraction, targetHearingLevel, PLAYER_SOUND_MUTE_FRACTION_MAX_UPDATE );
}

void TheWorld::UpdatePlayerHealthPulseFraction()
{
	float targetDamageConveyanceLevel = m_playerTank->GetFractionOfDamageDone();

	if ( targetDamageConveyanceLevel > m_playerHealthPulseFraction )
	{
		m_playerHealthPulseFraction = Interpolate( m_playerHealthPulseFraction, targetDamageConveyanceLevel, SmoothStop4( PLAYER_HEALTH_PULSE_FRACTION_MAX_UPDATE ) );		// Show damage immediately
	}
	else
	{
		m_playerHealthPulseFraction = Interpolate( m_playerHealthPulseFraction, targetDamageConveyanceLevel, SmoothStart4( PLAYER_HEALTH_PULSE_FRACTION_MAX_UPDATE ) );		// Show recovery gradually
	}
}

void TheWorld::HandleKeyboardInput( float& out_deltaSeconds )
{
	if ( ( m_framesSinceWorldInitialized > 0 ) && ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_P ) || (!IsGamePaused() && g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_ESCAPE ) ) ) )		// Pause/Unpause Game; Waiting for at least one frame since the game starts with the same keys, and this may pause the game immediately on start
	{
		HandlePauseKeyPress();
	}
	else if  ( m_deltaSecondsMultiplier > 0.0f )		// Slow down/speed up time, if the game isn't already paused
	{
		if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_T ) )
		{
			m_deltaSecondsMultiplier = CHEAT_DELTA_SECONDS_SLOWDOWN_MULTIPLIER;
		}
		else if( g_inputSystem->WasKeyJustReleased( InputSystem::KEYBOARD_T ) )
		{
			m_deltaSecondsMultiplier = 1.0f;
		}
		else if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_Y ) )
		{
			m_deltaSecondsMultiplier = CHEAT_DELTA_SECONDS_SPEEDUP_MULTIPLIER;
		}
		else if ( g_inputSystem->WasKeyJustReleased( InputSystem::KEYBOARD_Y ) )
		{
			m_deltaSecondsMultiplier = 1.0f;
		}
	}

	out_deltaSeconds *= m_deltaSecondsMultiplier;

	if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_F1 ))
	{
		if (m_developerModeEnabled)
		{
			m_developerModeEnabled = false;
		}
		else
		{
			m_developerModeEnabled = true;
		}
	}

	if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_1 ) && ( m_currentMap != m_mapsByName[ MAP_ONE_NAME ] ) )		// Map loading cheats
	{
		LoadMap( MAP_ONE_NAME );
	}
	else if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_2 ) && ( m_currentMap != m_mapsByName[ MAP_TWO_NAME ] ) )
	{
		LoadMap( MAP_TWO_NAME );
	}
	else if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_3 ) && ( m_currentMap != m_mapsByName[ MAP_THREE_NAME ] ) )
	{
		LoadMap( MAP_THREE_NAME );
	}
}

void TheWorld::HandleXboxControllerInput( float& out_deltaSeconds )
{
	if ( g_inputSystem->GetController( 0 ).IsConnected() )
	{
		if ( ( m_framesSinceWorldInitialized > 0 ) && ( g_inputSystem->GetController( 0 ).WasKeyJustPressed( InputXboxControllerMappings::GAMEPAD_START ) || (!IsGamePaused() && g_inputSystem->GetController( 0 ).WasKeyJustPressed( InputXboxControllerMappings::GAMEPAD_SELECT ) ) ) )		// Pause/Unpause Game
		{
			HandlePauseKeyPress();
		}

		out_deltaSeconds *= m_deltaSecondsMultiplier;
	}
}

void TheWorld::HandlePauseKeyPress()
{
	PlayerTank* playerTank = g_theWorld->GetPlayerTank();

	if ( !playerTank->IsDead() )   // Toggle game pause state
	{
		if ( IsFloatEqualTo( m_deltaSecondsMultiplier, 0.0f ) )
		{
			m_gamePaused = false;
			m_deltaSecondsMultiplier = 1.0f;		// Unpause the game
			SoundPlaybackID unpauseSound = g_audioSystem->PlaySound( g_audioSystem->CreateOrGetSound( AUDIO_UNPAUSE ) );
			m_playbackIdsOfSoundsPlaying.push_back( unpauseSound );
		}
		else
		{
			m_gamePaused = true;
			m_deltaSecondsMultiplier = 0.0f;		// Pause the game
			SoundPlaybackID pauseSound = g_audioSystem->PlaySound( g_audioSystem->CreateOrGetSound( AUDIO_PAUSE ) );
			m_playbackIdsOfSoundsPlaying.push_back( pauseSound );

			if ( g_inputSystem->GetController( 0 ).IsConnected() )
			{
				g_inputSystem->GetController( 0 ).StopControllerVibration();
			}
		}
	}
}

void TheWorld::CreateTileDefinitions()
{
	for ( int tileDefinitionIterator = 0; tileDefinitionIterator < TerrainType::NUM_TILE_TYPES; tileDefinitionIterator++ )
	{
		TileDefinition::s_tileDefinitions[ tileDefinitionIterator ] = new TileDefinition( static_cast< TerrainType >( tileDefinitionIterator ), *m_terrainSpritesheet );
	}
}

void TheWorld::LoadMap( std::string mapName )
{
	if ( m_currentMap != nullptr )
	{
		m_currentMap->UnloadMap();
	}

	FlushSoundsPlaying();

	m_currentMap = m_currentMap = m_mapsByName[ mapName ];

	m_currentMap->SpawnPlayerTank( m_playerTank );
	m_currentMap->SpawnEnemyTanks( m_numEnemyTanksByMapName[ mapName ] );
	m_currentMap->SpawnEnemyTurrets( m_numEnemyTurretsByMapName[ mapName ] );

	if ( m_currentMap == m_mapsByName[ MAP_ONE_NAME ] )
	{
		SoundPlaybackID music = g_audioSystem->PlaySound( g_audioSystem->CreateOrGetSound( AUDIO_MUSIC_LEVEL_1 ), true, AUDIO_MUSIC_VOLUME );
		m_playbackIdsOfSoundsPlaying.push_back( music );
	}
	else if ( m_currentMap == m_mapsByName[ MAP_TWO_NAME ] )
	{
		SoundPlaybackID music = g_audioSystem->PlaySound( g_audioSystem->CreateOrGetSound( AUDIO_MUSIC_LEVEL_2 ), true, AUDIO_MUSIC_VOLUME );
		m_playbackIdsOfSoundsPlaying.push_back( music );
	}
	else if ( m_currentMap == m_mapsByName[ MAP_THREE_NAME ] )
	{
		SoundPlaybackID music = g_audioSystem->PlaySound( g_audioSystem->CreateOrGetSound( AUDIO_MUSIC_LEVEL_3 ), true, AUDIO_MUSIC_VOLUME );
		m_playbackIdsOfSoundsPlaying.push_back( music );
	}
}

void TheWorld::LoadNextMap()
{
	if ( m_currentMap == m_mapsByName[ MAP_ONE_NAME ] )
	{
		g_audioSystem->PlaySound( g_audioSystem->CreateOrGetSound( AUDIO_EXIT_MAP ) );
		//m_playbackIdsOfSoundsPlaying.push_back( exitSound );		// Don't push this, as we don't want it to get flushed while loading levels
		LoadMap( MAP_TWO_NAME );
	}
	else if ( m_currentMap == m_mapsByName[ MAP_TWO_NAME ] )
	{
		g_audioSystem->PlaySound( g_audioSystem->CreateOrGetSound( AUDIO_EXIT_MAP ) );
		//m_playbackIdsOfSoundsPlaying.push_back( exitSound );		// Don't push this, as we don't want it to get flushed while loading levels
		LoadMap( MAP_THREE_NAME );
	}
	else if ( m_currentMap == m_mapsByName[ MAP_THREE_NAME ] )
	{
		m_hasPlayerWon = true;
	}
}

void TheWorld::CreateMapOne()
{
	std::string mapOneName = static_cast<std::string>( MAP_ONE_NAME );
	m_mapsByName[ mapOneName ] = new Map( MAP_ONE_HORIZONTAL_TILE_COUNT, MAP_ONE_VERTICAL_TILE_COUNT, *m_terrainSpritesheet );

	std::vector<TerrainType> mapOneTileTypes;		// First store the terrain types so we can alter them afterwards if we need to, before creating the actual tiles
	int firstNonSolidOrLiquidTileIndex = 0;			// Where the player tank will spawn
	int lastNonSolidOrLiquidTileIndex = 0;			// Find this so that the terminal tile for the level can be placed at this index

	for ( int verticalTileIterator = 0; verticalTileIterator < MAP_ONE_VERTICAL_TILE_COUNT; verticalTileIterator++ )		// The map has concrete on the borders, and grass elsewhere
	{
		for ( int horizontalTileIterator = 0; horizontalTileIterator < MAP_ONE_HORIZONTAL_TILE_COUNT; horizontalTileIterator++ )		// For reference, tileIndex = j * MAP_HORIZONTAL_TILE_WIDTH + i;
		{
			if	(	( horizontalTileIterator == 0 ) || 
					( horizontalTileIterator == ( MAP_ONE_HORIZONTAL_TILE_COUNT - 1 ) ) ||
					( verticalTileIterator == 0 ) ||
					( verticalTileIterator == ( MAP_ONE_VERTICAL_TILE_COUNT - 1 ) )
				)
			{
				mapOneTileTypes.push_back( TerrainType::TERRAIN_TYPE_BRICK_6 );
			}
			else if ( CheckRandomChance( MAP_ONE_SOLID_TILE_CHANCE ) )		// A tile in the middle has a chance of being a stone
			{
				mapOneTileTypes.push_back( TerrainType::TERRAIN_TYPE_STONE_4 );
			}
			else if ( CheckRandomChance( MAP_ONE_LIQUID_TILE_CHANCE ) )
			{
				mapOneTileTypes.push_back( TerrainType::TERRAIN_TYPE_WATER_3 );
			}
			else
			{
				if ( firstNonSolidOrLiquidTileIndex == 0 )		// This is definitely invalid, since the 0th/first tile will always be a bounding, solid tile
				{
					firstNonSolidOrLiquidTileIndex = ( verticalTileIterator * MAP_ONE_HORIZONTAL_TILE_COUNT ) + horizontalTileIterator;
				}
				else
				{
					lastNonSolidOrLiquidTileIndex = ( verticalTileIterator * MAP_ONE_HORIZONTAL_TILE_COUNT ) + horizontalTileIterator;
				}
				mapOneTileTypes.push_back( TerrainType::TERRAIN_TYPE_GRASS_3 );
			}
		}
	}

	mapOneTileTypes[ lastNonSolidOrLiquidTileIndex ] = TerrainType::TERRAIN_TYPE_PSYCH_1;		// Make the last non-solid tile a terminal-type tile

	if ( TileDefinition::DoesTileTypeBlockMovement( mapOneTileTypes[ ( firstNonSolidOrLiquidTileIndex + 1 ) ] ) && TileDefinition::DoesTileTypeBlockMovement( mapOneTileTypes[ ( firstNonSolidOrLiquidTileIndex + MAP_ONE_HORIZONTAL_TILE_COUNT ) ] ) )		// This means the player can't get out! Not allowed!
	{
		mapOneTileTypes[ ( firstNonSolidOrLiquidTileIndex + 1 ) ] = TerrainType::TERRAIN_TYPE_GRASS_2;		// Handle this arbitrarily
	}
	if ( TileDefinition::DoesTileTypeBlockMovement( mapOneTileTypes[ ( lastNonSolidOrLiquidTileIndex - 1 ) ] ) && TileDefinition::DoesTileTypeBlockMovement( mapOneTileTypes[ ( lastNonSolidOrLiquidTileIndex - MAP_ONE_HORIZONTAL_TILE_COUNT ) ] ) )		// This means the player can't get to the terminal tile! Also not allowed!
	{
		mapOneTileTypes[ ( lastNonSolidOrLiquidTileIndex - 1 ) ] = TerrainType::TERRAIN_TYPE_GRASS_2;
	}

	for ( size_t tileIterator = 0; tileIterator < mapOneTileTypes.size(); tileIterator++ )
	{
		size_t horizontalIndex = tileIterator % MAP_ONE_HORIZONTAL_TILE_COUNT;
		size_t verticalIndex = tileIterator / MAP_ONE_HORIZONTAL_TILE_COUNT;
		Tile newTile = Tile( mapOneTileTypes[ tileIterator ], horizontalIndex, verticalIndex );
		m_mapsByName[ mapOneName ]->AddTile( newTile );
	}

	m_numEnemyTanksByMapName[ mapOneName ] = MAP_ONE_NUM_ENEMY_TANKS;
	m_numEnemyTurretsByMapName[ mapOneName ] = MAP_ONE_NUM_ENEMY_TURRETS;
}

void TheWorld::CreateMapTwo()
{
	std::string mapTwoName = static_cast<std::string>( MAP_TWO_NAME );
	m_mapsByName[ mapTwoName ] = new Map( MAP_TWO_HORIZONTAL_TILE_COUNT, MAP_TWO_VERTICAL_TILE_COUNT, *m_terrainSpritesheet );

	std::vector<TerrainType> mapTwoTileTypes;
	int firstNonSolidOrLiquidTileIndex = 0;
	int lastNonSolidOrLiquidTileIndex = 0;

	for ( int verticalTileIterator = 0; verticalTileIterator < MAP_TWO_VERTICAL_TILE_COUNT; verticalTileIterator++ )
	{
		for ( int horizontalTileIterator = 0; horizontalTileIterator < MAP_TWO_HORIZONTAL_TILE_COUNT; horizontalTileIterator++ )
		{
			if	(	( horizontalTileIterator == 0 ) || 
				( horizontalTileIterator == ( MAP_TWO_HORIZONTAL_TILE_COUNT - 1 ) ) ||
				( verticalTileIterator == 0 ) ||
				( verticalTileIterator == ( MAP_TWO_VERTICAL_TILE_COUNT - 1 ) )
				)
			{
				mapTwoTileTypes.push_back( TerrainType::TERRAIN_TYPE_BRICK_1 );
			}
			else if ( CheckRandomChance( MAP_TWO_SOLID_TILE_CHANCE ) )
			{
				mapTwoTileTypes.push_back( TerrainType::TERRAIN_TYPE_STONE_7 );
			}
			else if ( CheckRandomChance( MAP_TWO_LIQUID_TILE_CHANCE ) )
			{
				mapTwoTileTypes.push_back( TerrainType::TERRAIN_TYPE_WATER_5 );
			}
			else
			{
				if ( firstNonSolidOrLiquidTileIndex == 0 )
				{
					firstNonSolidOrLiquidTileIndex = ( verticalTileIterator * MAP_TWO_HORIZONTAL_TILE_COUNT ) + horizontalTileIterator;
				}
				else
				{
					lastNonSolidOrLiquidTileIndex = ( verticalTileIterator * MAP_TWO_HORIZONTAL_TILE_COUNT ) + horizontalTileIterator;
				}
				mapTwoTileTypes.push_back( TerrainType::TERRAIN_TYPE_SAND_7 );
			}
		}
	}

	mapTwoTileTypes[ lastNonSolidOrLiquidTileIndex ] = TerrainType::TERRAIN_TYPE_PSYCH_1;		// Make the last non-solid tile a terminal-type tile

	if ( TileDefinition::DoesTileTypeBlockMovement( mapTwoTileTypes[ ( firstNonSolidOrLiquidTileIndex + 1 ) ] ) && TileDefinition::DoesTileTypeBlockMovement( mapTwoTileTypes[ ( firstNonSolidOrLiquidTileIndex + MAP_TWO_HORIZONTAL_TILE_COUNT ) ] ) )		// This means the player can't get out! Not allowed!
	{
		mapTwoTileTypes[ ( firstNonSolidOrLiquidTileIndex + 1 ) ] = TerrainType::TERRAIN_TYPE_SAND_7;		// Handle this arbitrarily
	}
	if ( TileDefinition::DoesTileTypeBlockMovement( mapTwoTileTypes[ ( lastNonSolidOrLiquidTileIndex - 1 ) ] ) && TileDefinition::DoesTileTypeBlockMovement( mapTwoTileTypes[ ( lastNonSolidOrLiquidTileIndex - MAP_TWO_HORIZONTAL_TILE_COUNT ) ] ) )		// This means the player can't get to the terminal tile! Also not allowed!
	{
		mapTwoTileTypes[ ( lastNonSolidOrLiquidTileIndex - 1 ) ] = TerrainType::TERRAIN_TYPE_SAND_7;
	}

	for ( size_t tileIterator = 0; tileIterator < mapTwoTileTypes.size(); tileIterator++ )
	{
		size_t horizontalIndex = tileIterator % MAP_TWO_HORIZONTAL_TILE_COUNT;
		size_t verticalIndex = tileIterator / MAP_TWO_HORIZONTAL_TILE_COUNT;
		Tile newTile = Tile( mapTwoTileTypes[ tileIterator ], horizontalIndex, verticalIndex );
		m_mapsByName[ mapTwoName ]->AddTile( newTile );
	}

	m_numEnemyTanksByMapName[ mapTwoName ] = MAP_TWO_NUM_ENEMY_TANKS;
	m_numEnemyTurretsByMapName[ mapTwoName ] = MAP_TWO_NUM_ENEMY_TURRETS;
}

void TheWorld::CreateMapThree()
{
	std::string mapThreeName = static_cast<std::string>( MAP_THREE_NAME );
	m_mapsByName[ mapThreeName ] = new Map( MAP_THREE_HORIZONTAL_TILE_COUNT, MAP_THREE_VERTICAL_TILE_COUNT, *m_terrainSpritesheet );

	std::vector<TerrainType> mapThreeTileTypes;
	int firstNonSolidOrLiquidTileIndex = 0;
	int lastNonSolidOrLiquidTileIndex = 0;

	for ( int verticalTileIterator = 0; verticalTileIterator < MAP_THREE_VERTICAL_TILE_COUNT; verticalTileIterator++ )
	{
		for ( int horizontalTileIterator = 0; horizontalTileIterator < MAP_THREE_HORIZONTAL_TILE_COUNT; horizontalTileIterator++ )
		{
			if	(	( horizontalTileIterator == 0 ) || 
				( horizontalTileIterator == ( MAP_THREE_HORIZONTAL_TILE_COUNT - 1 ) ) ||
				( verticalTileIterator == 0 ) ||
				( verticalTileIterator == ( MAP_THREE_VERTICAL_TILE_COUNT - 1 ) )
				)
			{
				mapThreeTileTypes.push_back( TerrainType::TERRAIN_TYPE_BRICK_13 );
			}
			else if ( CheckRandomChance( MAP_THREE_SOLID_TILE_CHANCE ) )
			{
				mapThreeTileTypes.push_back( TerrainType::TERRAIN_TYPE_METAL_1 );
			}
			else if ( CheckRandomChance( MAP_THREE_LIQUID_TILE_CHANCE ) )
			{
				mapThreeTileTypes.push_back( TerrainType::TERRAIN_TYPE_LAVA_2 );
			}
			else
			{
				if ( firstNonSolidOrLiquidTileIndex == 0 )
				{
					firstNonSolidOrLiquidTileIndex = ( verticalTileIterator * MAP_THREE_HORIZONTAL_TILE_COUNT ) + horizontalTileIterator;
				}
				else
				{
					lastNonSolidOrLiquidTileIndex = ( verticalTileIterator * MAP_THREE_HORIZONTAL_TILE_COUNT ) + horizontalTileIterator;
				}
				mapThreeTileTypes.push_back( TerrainType::TERRAIN_TYPE_TILE_1 );
			}
		}
	}

	mapThreeTileTypes[ lastNonSolidOrLiquidTileIndex ] = TerrainType::TERRAIN_TYPE_PSYCH_1;		// Make the last non-solid tile a terminal-type tile

	if ( TileDefinition::DoesTileTypeBlockMovement( mapThreeTileTypes[ ( firstNonSolidOrLiquidTileIndex + 1 ) ] ) && TileDefinition::DoesTileTypeBlockMovement( mapThreeTileTypes[ ( firstNonSolidOrLiquidTileIndex + MAP_THREE_HORIZONTAL_TILE_COUNT ) ] ) )		// This means the player can't get out! Not allowed!
	{
		mapThreeTileTypes[ ( firstNonSolidOrLiquidTileIndex + 1 ) ] = TerrainType::TERRAIN_TYPE_TILE_1;		// Handle this arbitrarily
	}
	if ( TileDefinition::DoesTileTypeBlockMovement( mapThreeTileTypes[ ( lastNonSolidOrLiquidTileIndex - 1 ) ] ) && TileDefinition::DoesTileTypeBlockMovement( mapThreeTileTypes[ ( lastNonSolidOrLiquidTileIndex - MAP_THREE_HORIZONTAL_TILE_COUNT ) ] ) )		// This means the player can't get to the terminal tile! Also not allowed!
	{
		mapThreeTileTypes[ ( lastNonSolidOrLiquidTileIndex - 1 ) ] = TerrainType::TERRAIN_TYPE_TILE_1;
	}

	for ( size_t tileIterator = 0; tileIterator < mapThreeTileTypes.size(); tileIterator++ )
	{
		size_t horizontalIndex = tileIterator % MAP_THREE_HORIZONTAL_TILE_COUNT;
		size_t verticalIndex = tileIterator / MAP_THREE_HORIZONTAL_TILE_COUNT;
		Tile newTile = Tile( mapThreeTileTypes[ tileIterator ], horizontalIndex, verticalIndex );
		m_mapsByName[ mapThreeName ]->AddTile( newTile );
	}

	m_numEnemyTanksByMapName[ mapThreeName ] = MAP_THREE_NUM_ENEMY_TANKS;
	m_numEnemyTurretsByMapName[ mapThreeName ] = MAP_THREE_NUM_ENEMY_TURRETS;
}

void TheWorld::DeleteAllMapReferences()
{
	for ( std::map< std::string, Map* >::iterator mapIterator = m_mapsByName.begin(); mapIterator != m_mapsByName.end(); mapIterator++ )
	{
		delete mapIterator->second;
		mapIterator->second = nullptr;
	}
}

void TheWorld::DeleteTileDefinitions()
{
	for ( int tileDefinitionIterator = 0; tileDefinitionIterator < TerrainType::NUM_TILE_TYPES; tileDefinitionIterator++ )
	{
		delete TileDefinition::s_tileDefinitions[ tileDefinitionIterator ];
		TileDefinition::s_tileDefinitions[ tileDefinitionIterator ] = nullptr;
	}
}
