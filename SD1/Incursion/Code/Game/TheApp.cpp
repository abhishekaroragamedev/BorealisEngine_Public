#include "Game/GameCommon.hpp"
#include "TheApp.hpp"

TheApp::TheApp()
{
	g_audioSystem = new AudioSystem();
	g_renderer = new Renderer();
	g_inputSystem = new InputSystem();

	m_audioLoaded = false;

	g_theGame = new TheGame();
}

TheApp::~TheApp()
{
	delete g_theGame;
	g_theGame = 0;

	delete g_inputSystem;
	g_inputSystem = 0;

	delete g_renderer;
	g_renderer = 0;

	delete g_audioSystem;
	g_audioSystem = 0;
}

//-----------------------------------------------------------------------------------------------
// One "frame" of the game.  Generally: Input, Update, Render.  We call this 60+ times per second.
//
void TheApp::RunFrame()
{
	g_audioSystem->BeginFrame();
	g_inputSystem->BeginFrame();
	g_renderer->BeginFrame( (SCREEN_HEIGHT * CLIENT_ASPECT), SCREEN_HEIGHT );

	Update();
	Render();

	if ( !m_audioLoaded )		// Load after first render so that the user sees something before a lag
	{
		LoadSoundsForApp();
	}

	g_inputSystem->EndFrame();
	g_renderer->EndFrame();
	g_audioSystem->EndFrame();
}

void TheApp::Update()
{
	float deltaSeconds = ComputeDeltaSeconds();
	g_theGame->Update( deltaSeconds );
}

void TheApp::Render()
{
	g_theGame->Render();
}

bool TheApp::IsQuitting() const
{
	return g_theGame->IsQuitting();
}

void TheApp::LoadSoundsForApp()
{
	std::vector<std::string> soundFilenames = GetAllAudioFilenames();

	for ( std::string soundFilename : soundFilenames )
	{
		g_audioSystem->CreateOrGetSound( soundFilename );
	}

	m_audioLoaded = true;
}

float TheApp::ComputeDeltaSeconds() const
{
	static double s_timeLastUpdate = GetCurrentTimeSeconds();

	double timeNow = GetCurrentTimeSeconds();
	float deltaSeconds = static_cast<float> ( timeNow - s_timeLastUpdate );
	s_timeLastUpdate = timeNow;

	return deltaSeconds;
}
