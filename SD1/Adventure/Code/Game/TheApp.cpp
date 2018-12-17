#include "Game/GameCommon.hpp"
#include "TheApp.hpp"

TheApp::TheApp()
{
	PopulateBlackboardFromGameConfigXml();

	g_audioSystem = new AudioSystem();
	g_renderer = new Renderer();
	g_inputSystem = new InputSystem();

	m_isQuitting = false;

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
	g_inputSystem->BeginFrame();
	g_renderer->BeginFrame( ( SCREEN_HEIGHT * CLIENT_ASPECT ), SCREEN_HEIGHT );

	QuitIfRequired();

	Update();
	Render();

	g_inputSystem->EndFrame();
	g_renderer->EndFrame();
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

void TheApp::PopulateBlackboardFromGameConfigXml()
{
	tinyxml2::XMLDocument* gameConfigDocument = new tinyxml2::XMLDocument();
	gameConfigDocument->LoadFile( GAME_CONFIG_XML_FILE_PATH );

	g_gameConfigBlackboard.PopulateFromXmlElementAttributes( *gameConfigDocument->RootElement() );

	delete gameConfigDocument;
	gameConfigDocument = nullptr;
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
