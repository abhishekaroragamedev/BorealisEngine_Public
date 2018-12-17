#include "Game/GameCommon.hpp"
#include "Game/GameStates/MenuGameState.hpp"
#include "Engine/Core/Window.hpp"
#include "Engine/Tools/DevConsole.hpp"

MenuGameState::MenuGameState()
{
	
}

MenuGameState::~MenuGameState()
{
	
}

void MenuGameState::Enter()
{
	ConsolePrintf( Rgba::CYAN, "Entered Menu State." );
	m_camera = new Camera( g_renderer->GetDefaultColorTarget() );
	m_camera->SetProjectionOrtho( 2.0f, CLIENT_ASPECT, -1.0f, 1.0f );

	CommandRegister( "load_encounter", LoadEncounterCommand, "TACTICS: Load an Encounter from a known EncounterDefinition." );
}

void MenuGameState::Exit()
{
	delete m_camera;
	m_camera = nullptr;
	ConsolePrintf( Rgba::MAGENTA, "Leaving Menu State." );
}

void MenuGameState::Update()
{
	m_timeInState += GetMasterDeltaSecondsF();
	TryJoinGame();
	HandleInput();
}

void MenuGameState::TryJoinGame()
{
	if (g_theGame->CanJoin())
	{
		g_theGame->LoadEncounter( *EncounterDefinition::s_definitions[ "Fortress" ], true, false );
	}
}

void MenuGameState::HandleInput()
{
	if ( !DevConsole::GetInstance()->IsOpen() )
	{
		HandleXboxControllerInput();
		HandleKeyboardInput();
	}
}

void MenuGameState::HandleXboxControllerInput()
{
	static float s_timeSinceAnalogInput = XBOX_ANALOG_CURSOR_MOVE_COOLDOWN_SECONDS;
	s_timeSinceAnalogInput += GetMasterDeltaSecondsF();

	if ( !DevConsole::GetInstance()->IsOpen() && ( s_timeSinceAnalogInput > XBOX_ANALOG_CURSOR_MOVE_COOLDOWN_SECONDS ) && g_inputSystem->GetController( 0 ).IsConnected() )
	{
		if ( g_inputSystem->GetController( 0 ).GetJoystick( 0 ).GetPosition().y > 0.0f )
		{
			int itemIndex = ( m_selectedMenuItem == 0 )? (NUM_GAME_MENU_ITEMS - 1) : (m_selectedMenuItem - 1);
			m_selectedMenuItem = static_cast<GameMenuItem>(itemIndex);
			s_timeSinceAnalogInput = 0.0f;
		}
		if ( g_inputSystem->GetController( 0 ).GetJoystick( 0 ).GetPosition().y < 0.0f )
		{
			m_selectedMenuItem = static_cast<GameMenuItem>( (m_selectedMenuItem + 1) % NUM_GAME_MENU_ITEMS );
			s_timeSinceAnalogInput = 0.0f;
		}
		if ( g_inputSystem->GetController( 0 ).WasKeyJustPressed( InputXboxControllerMappings::GAMEPAD_A ) )
		{
			if ( m_selectedMenuItem == GameMenuItem::PLAY )
			{
				g_theGame->LoadEncounter( *EncounterDefinition::s_definitions[ "Fortress" ] );
			}
			else if ( m_selectedMenuItem == GameMenuItem::HOST )
			{
				g_theGame->LoadEncounter( *EncounterDefinition::s_definitions[ "Fortress" ], true, true );
			}
			else
			{
				g_theApp->ForceQuit();
			}
		}
	}
}

void MenuGameState::HandleKeyboardInput()
{
	if ( !DevConsole::GetInstance()->IsOpen() )
	{
		if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_UP_ARROW ) )
		{
			int itemIndex = ( m_selectedMenuItem == 0 )? (NUM_GAME_MENU_ITEMS - 1) : (m_selectedMenuItem - 1);
			m_selectedMenuItem = static_cast<GameMenuItem>(itemIndex);
		}
		if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_DOWN_ARROW ) )
		{
			m_selectedMenuItem = static_cast<GameMenuItem>( (m_selectedMenuItem + 1) % NUM_GAME_MENU_ITEMS );
		}
		if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_RETURN ) )
		{
			if ( m_selectedMenuItem == GameMenuItem::PLAY )
			{
				g_theGame->LoadEncounter( *EncounterDefinition::s_definitions[ "Fortress" ] );
			}
			else if ( m_selectedMenuItem == GameMenuItem::HOST )
			{
				g_theGame->LoadEncounter( *EncounterDefinition::s_definitions[ "Fortress" ], true, true );
			}
			else
			{
				g_theApp->ForceQuit();
			}
		}
	}
}

void MenuGameState::Render() const
{
	g_renderer->EnableDepth( DepthTestCompare::COMPARE_ALWAYS, false );

	g_renderer->UseShaderProgram( DEFAULT_SHADER_NAME );
	g_renderer->BindModelMatrixToCurrentShader( Matrix44::IDENTITY );

	g_renderer->ClearScreen( Rgba::BLACK );
	g_renderer->SetCamera( m_camera );
	g_renderer->ClearColor();
	g_renderer->DrawTextInBox2D( "Tactics", AABB2( ( Window::GetAspectMultipliers() * -1.0f ), Window::GetAspectMultipliers() ), 0.2f, Rgba::WHITE, 1.0f, g_renderer->CreateOrGetBitmapFont( DEFAULT_FONT_NAME ), TextDrawMode::TEXT_DRAW_OVERRUN, Vector2( 0.5f, 0.25f ) );
	
	Rgba playButtonColor = ( m_selectedMenuItem == GameMenuItem::PLAY )? Rgba::YELLOW : Rgba::WHITE;
	Rgba hostButtonColor = ( m_selectedMenuItem == GameMenuItem::HOST )? Rgba::YELLOW : Rgba::WHITE;
	Rgba quitButtonColor = ( m_selectedMenuItem == GameMenuItem::QUIT )? Rgba::YELLOW : Rgba::WHITE;

	g_renderer->DrawTextInBox2D( "Play", AABB2( ( Window::GetAspectMultipliers() * -1.0f ), Window::GetAspectMultipliers() ), 0.1f, playButtonColor, 1.0f, g_renderer->CreateOrGetBitmapFont( DEFAULT_FONT_NAME ), TextDrawMode::TEXT_DRAW_OVERRUN, Vector2( 0.5f, 0.65f ) );
	g_renderer->DrawTextInBox2D( "Host", AABB2( ( Window::GetAspectMultipliers() * -1.0f ), Window::GetAspectMultipliers() ), 0.1f, hostButtonColor, 1.0f, g_renderer->CreateOrGetBitmapFont( DEFAULT_FONT_NAME ), TextDrawMode::TEXT_DRAW_OVERRUN, Vector2( 0.5f, 0.75f ) );
	g_renderer->DrawTextInBox2D( "Quit", AABB2( ( Window::GetAspectMultipliers() * -1.0f ), Window::GetAspectMultipliers() ), 0.1f, quitButtonColor, 1.0f, g_renderer->CreateOrGetBitmapFont( DEFAULT_FONT_NAME ), TextDrawMode::TEXT_DRAW_OVERRUN, Vector2( 0.5f, 0.85f ) );
	
	g_renderer->DrawTextInBox2D( "by Abhishek Arora", AABB2( ( Window::GetAspectMultipliers() * -1.0f ), Window::GetAspectMultipliers() ), 0.1f, Rgba::WHITE, 1.0f, g_renderer->CreateOrGetBitmapFont( DEFAULT_FONT_NAME ), TextDrawMode::TEXT_DRAW_OVERRUN, Vector2( 1.0f, 1.0f ) );
}

bool LoadEncounterCommand( Command& command )
{
	static const char s_commandErrorString[] = "ERROR: load_encounter definitionname";

	if ( command.GetName() == "load_encounter" )
	{
		std::string encounterDefinitionName = command.GetNextString();
		if ( EncounterDefinition::s_definitions.find( encounterDefinitionName ) == EncounterDefinition::s_definitions.end() )
		{
			ConsolePrintf( Rgba::RED, "ERROR: load_encounter: Invalid encounter definition name provided." );
			ConsolePrintf( Rgba::RED, s_commandErrorString );
			return false;
		}
		EncounterDefinition* encounterDefinition = EncounterDefinition::s_definitions[ encounterDefinitionName ];
		g_theGame->LoadEncounter( *encounterDefinition );
		return true;
	}
	else
	{
		return false;
	}
}
