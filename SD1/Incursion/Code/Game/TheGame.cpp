#include "Game/TheGame.hpp"
#include "Game/GameCommon.hpp"

TheGame::TheGame()
{
	g_theWorld = new TheWorld();
	m_gameState = GameState::STATE_ATTRACT;
	m_nextState = GameState::STATE_NONE;
	m_attractMenu = new AttractMenu();
	m_victoryScreen = new VictoryScreen();
	m_isQuitting = false;
}

TheGame::~TheGame()
{
	delete g_theWorld;
	g_theWorld = nullptr;

	delete m_attractMenu;
	m_attractMenu = nullptr;

	delete m_victoryScreen;
	m_victoryScreen = nullptr;
}

void TheGame::Update( float deltaSeconds )
{
	HandleKeyboardInput();
	HandleXboxControllerInput();

	if ( m_nextState != GameState::STATE_NONE )
	{
		TransitionToNextState();

		if ( g_inputSystem->GetController( 0 ).IsConnected() )
		{
			g_inputSystem->GetController( 0 ).StopControllerVibration();
		}
	}

	switch ( m_gameState )
	{
		case GameState::STATE_ATTRACT:
		{
			m_attractMenu->Update( deltaSeconds );
			break;
		}
		case GameState::STATE_GAME:
		{
			g_theWorld->Update( deltaSeconds );
			UpdateNextStateIfPlayerHasWonGame();
			break;
		}
		case GameState::STATE_VICTORY:
		{
			g_theWorld->Update( deltaSeconds );
			m_victoryScreen->Update( deltaSeconds );
			break;
		}

		default:
			break;
	}
}

void TheGame::Render() const
{
	g_renderer->ClearScreen( Rgba::BLACK );

	switch ( m_gameState )
	{
		case GameState::STATE_ATTRACT:
		{
			m_attractMenu->Render();
			break;
		}
		case GameState::STATE_GAME:
		{
			g_theWorld->Render();
			break;
		}
		case GameState::STATE_VICTORY:
		{
			g_theWorld->Render();		// Final game screen will be rendered in Victory mode, but not updated
			m_victoryScreen->Render();
			break;
		}

	default:
		break;
	}
}

bool TheGame::IsQuitting() const
{
	return m_isQuitting;
}

void TheGame::HandleKeyboardInput()
{
	if ( ( m_gameState == GameState::STATE_ATTRACT ) && ( m_attractMenu->GetFramesSinceAttractModeFirstLoaded() > 1 ) && g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_SPACE ) )
	{
		m_nextState = GameState::STATE_GAME;
	}
	else if ( ( m_gameState == GameState::STATE_GAME ) && g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_ESCAPE ) )
	{
		if ( g_theWorld->IsGamePaused() || g_theWorld->GetPlayerTank()->IsDeadAndCanRespawn() )		// Either the pause or death screen is displaying
		{
			m_nextState = GameState::STATE_ATTRACT;
		}
	}

	if ( ( m_gameState == GameState::STATE_ATTRACT ) && g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_ESCAPE ) )
	{
		m_isQuitting = true;
	}

	if ( ( m_gameState == GameState::STATE_VICTORY ) && ( m_victoryScreen->GetTimeSinceLastInit() > VICTORY_SCREEN_DISABLE_INPUT_SECONDS ) )
	{
		if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_P ) || g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_ESCAPE ) )
		{
			m_nextState = GameState::STATE_ATTRACT;
		}
	}
}

void TheGame::HandleXboxControllerInput()
{
	if ( g_inputSystem->GetController( 0 ).IsConnected() )
	{
		if ( ( m_gameState == GameState::STATE_ATTRACT ) && ( m_attractMenu->GetFramesSinceAttractModeFirstLoaded() > 1 ) && g_inputSystem->GetController( 0 ).WasKeyJustPressed( InputXboxControllerMappings::GAMEPAD_START )  )
		{
			m_nextState = GameState::STATE_GAME;
		}
		else if ( ( m_gameState == GameState::STATE_ATTRACT ) && g_inputSystem->GetController( 0 ).WasKeyJustPressed( InputXboxControllerMappings::GAMEPAD_SELECT ) )
		{
			m_isQuitting = true;
		}
		else if ( ( m_gameState == GameState::STATE_GAME ) && g_inputSystem->GetController( 0 ).WasKeyJustPressed( InputXboxControllerMappings::GAMEPAD_SELECT ) )
		{
			if ( g_theWorld->IsGamePaused() || g_theWorld->GetPlayerTank()->IsDeadAndCanRespawn() )		// Either the pause or death screen is displaying
			{
				m_nextState = GameState::STATE_ATTRACT;
			}
		}

		if ( ( m_gameState == GameState::STATE_VICTORY ) && ( m_victoryScreen->GetTimeSinceLastInit() > VICTORY_SCREEN_DISABLE_INPUT_SECONDS ) )
		{
			if ( g_inputSystem->GetController( 0 ).WasKeyJustPressed( InputXboxControllerMappings::GAMEPAD_START ) || g_inputSystem->GetController( 0 ).WasKeyJustPressed( InputXboxControllerMappings::GAMEPAD_SELECT ) )
			{
				m_nextState = GameState::STATE_ATTRACT;
			}
		}
	}
}

void TheGame::UpdateNextStateIfPlayerHasWonGame()
{
	if ( m_gameState == GameState::STATE_GAME && g_theWorld->HasPlayerWon() )
	{
		m_nextState = GameState::STATE_VICTORY;
	}
}

void TheGame::TransitionToNextState()
{
	if ( m_nextState == GameState::STATE_ATTRACT )
	{
		if ( m_gameState == GameState::STATE_GAME )
		{
			g_theWorld->FlushSoundsPlaying();
		}
		else if ( m_gameState == GameState::STATE_VICTORY )
		{
			m_victoryScreen->FlushSoundsPlaying();
		}
		m_attractMenu->InitializeAttractMode();
	}
	else if ( m_nextState == GameState::STATE_GAME && m_gameState == GameState::STATE_ATTRACT )
	{
		m_attractMenu->FlushSoundsPlaying();
		g_audioSystem->PlaySound( g_audioSystem->CreateOrGetSound( AUDIO_START_GAME ) );
		g_theWorld->InitializeTheWorld();
	}
	else if ( m_nextState == GameState::STATE_VICTORY && m_gameState == GameState::STATE_GAME )
	{
		g_theWorld->FlushSoundsPlaying();
		m_victoryScreen->InitializeVictoryScreen();
	}

	m_gameState = m_nextState;
	m_nextState = GameState::STATE_NONE;
}
