#include "TheApp.hpp"

TheApp::TheApp()
{
	g_renderer = new Renderer();
	g_inputSystem = new InputSystem();

	m_isQuitting = false;

	m_theGame = new TheGame();
}

TheApp::~TheApp()
{
	delete m_theGame;
	m_theGame = 0;

	delete g_inputSystem;
	g_inputSystem = 0;

	delete g_renderer;
	g_renderer = 0;
}

//-----------------------------------------------------------------------------------------------
// One "frame" of the game.  Generally: Input, Update, Render.  We call this 60+ times per second.
//
void TheApp::RunFrame()
{
	g_inputSystem->BeginFrame();
	g_renderer->BeginFrame( SCREEN_WIDTH, SCREEN_HEIGHT );

	QuitIfRequired();

	Update();
	Render();

	g_inputSystem->EndFrame();
	g_renderer->EndFrame();
}

void TheApp::Update()
{
	float deltaSeconds = ComputeDeltaSeconds();
	m_theGame->Update( deltaSeconds );
}

void TheApp::Render()
{
	m_theGame->Render();
}

bool TheApp::IsQuitting() const
{
	return m_isQuitting;
}

void TheApp::QuitIfRequired()
{
	if ( g_inputSystem->IsKeyDown( InputSystem::KEYBOARD_ESCAPE ) )
	{
		m_isQuitting = true;
	}
}

float TheApp::ComputeDeltaSeconds() const
{
	static double s_timeLastUpdate = GetCurrentTimeSeconds();

	double timeNow = GetCurrentTimeSeconds();
	float deltaSeconds = static_cast<float> ( timeNow - s_timeLastUpdate );
	s_timeLastUpdate = timeNow;

	return deltaSeconds;
}
