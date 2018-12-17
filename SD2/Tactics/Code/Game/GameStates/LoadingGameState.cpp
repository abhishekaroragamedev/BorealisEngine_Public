#include "Game/GameCommon.hpp"
#include "Game/GameStates/LoadingGameState.hpp"
#include "Engine/Core/Window.hpp"
#include "Engine/Tools/DevConsole.hpp"

LoadingGameState::LoadingGameState()
{

}

LoadingGameState::~LoadingGameState()
{

}

void LoadingGameState::Enter()
{
	ConsolePrintf( Rgba::CYAN, "Entered Loading State." );
	m_camera = new Camera( g_renderer->GetDefaultColorTarget() );
	m_camera->SetProjectionOrtho( 2.0f, 1.0f, -1.0f, 1.0f );
}

void LoadingGameState::Exit()
{
	delete m_camera;
	m_camera = nullptr;
	ConsolePrintf( Rgba::MAGENTA, "Leaving Loading State." );
}

void LoadingGameState::Update()
{
	if ( IsFloatEqualTo( m_timeInState, 0.0f ) )	// Should only run for the first frame; game should load all resources (apart from the font used to render "Loading...") before the next frame
	{
		m_timeInState += GetMasterDeltaSecondsF();
		Render();
		g_theGame->LoadResources();
	}

	m_timeInState += GetMasterDeltaSecondsF();
	g_theGame->ChangeGameState( GameStateType::STATE_MENU );
}

void LoadingGameState::Render() const
{
	g_renderer->UseShaderProgram( DEFAULT_SHADER_NAME );

	g_renderer->SetCamera( m_camera );
	g_renderer->EnableDepth( DepthTestCompare::COMPARE_ALWAYS, false );
	g_renderer->ClearColor();
	g_renderer->DrawText2D( ( -1.0f * Window::GetAspectMultipliers() ), "Loading...", 0.1f, Rgba::WHITE, 1.0f, g_renderer->CreateOrGetBitmapFont( DEFAULT_FONT_NAME ) );
}
