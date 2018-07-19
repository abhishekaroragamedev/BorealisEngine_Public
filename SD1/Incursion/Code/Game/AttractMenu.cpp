#include "Game/AttractMenu.hpp"
#include "Game/GameCommon.hpp"

AttractMenu::AttractMenu()
{
	m_titleTexture = g_renderer->CreateOrGetTexture( TITLE_TEXT_PATH );
	m_loadingTexture = g_renderer->CreateOrGetTexture( LOADING_TEXT_PATH );
	m_instructionTexture = g_renderer->CreateOrGetTexture( INSTRUCTION_TEXT_PATH );
	m_gameStartTexture = g_renderer->CreateOrGetTexture( START_PROMPT_TEXT_PATH );
	m_authorNameTexture = g_renderer->CreateOrGetTexture( AUTHOR_NAME_TEXT_PATH );
	m_framesSinceAttractMenuCreated = 0;
	m_gameStartTextAlpha = 0.0f;
	m_gameStartTextAlphaChangeDirection = 1.0f;

	InitializeAttractMode();
}

AttractMenu::~AttractMenu()
{
	m_titleTexture = nullptr;
	m_loadingTexture = nullptr;
	m_instructionTexture = nullptr;
	m_gameStartTexture = nullptr;
	m_authorNameTexture = nullptr;
}

void AttractMenu::Update( float deltaSeconds )
{
	HandleKeyboardInput();
	HandleXboxControllerInput();

	UpdateGameStartTextAlpha( deltaSeconds );
	
	m_framesSinceAttractMenuCreated++;
}

void AttractMenu::Render() const
{
	RenderBackground();
	RenderInstructions();
	RenderAuthorName();
	RenderTitle();
	RenderLoadingText();
	RenderStartPrompt();
}

void AttractMenu::InitializeAttractMode()
{
	AABB2 screenBounds = AABB2( Vector2( 0.0f, 0.0f ), Vector2( ( SCREEN_HEIGHT * CLIENT_ASPECT ), SCREEN_HEIGHT ) );		// The game may have changed the ortho for the camera
	g_renderer->SetOrtho( screenBounds.mins, screenBounds.maxs );

	SoundPlaybackID loadMenuSound = g_audioSystem->PlaySound( g_audioSystem->CreateOrGetSound( AUDIO_MENU_LOAD ) );
	m_playbackIdsOfSoundsPlaying.push_back( loadMenuSound );

	SoundPlaybackID menuMusic = g_audioSystem->PlaySound( g_audioSystem->CreateOrGetSound( AUDIO_MUSIC_ATTRACT ), true, AUDIO_MUSIC_VOLUME );
	m_playbackIdsOfSoundsPlaying.push_back( menuMusic );
}

void AttractMenu::HandleKeyboardInput() const
{

}

void AttractMenu::HandleXboxControllerInput() const
{

}

void AttractMenu::FlushSoundsPlaying()
{
	for ( std::vector<SoundPlaybackID>::iterator soundIterator = m_playbackIdsOfSoundsPlaying.begin(); soundIterator != m_playbackIdsOfSoundsPlaying.end(); soundIterator++ )
	{
		g_audioSystem->StopSound( *soundIterator );
	}

	m_playbackIdsOfSoundsPlaying.clear();
}

void AttractMenu::UpdateGameStartTextAlpha( float deltaSeconds )
{
	if ( m_gameStartTextAlphaChangeDirection > 0.0f )
	{
		m_gameStartTextAlpha += deltaSeconds;
	}
	else
	{
		m_gameStartTextAlpha -= deltaSeconds;
	}

	if ( m_gameStartTextAlpha >= 1.0f )
	{
		m_gameStartTextAlphaChangeDirection = -1.0f;
	}
	else if ( m_gameStartTextAlpha <= 0.0f )
	{
		m_gameStartTextAlphaChangeDirection = 1.0f;
	}
}

void AttractMenu::RenderBackground() const
{
	AABB2 screenLeftHalf = AABB2( Vector2( 0.0f, 0.0f ), Vector2( ( ( SCREEN_HEIGHT * CLIENT_ASPECT ) / 2.0f ), SCREEN_HEIGHT ) );
	AABB2 screenRightHalf = AABB2( Vector2( ( ( SCREEN_HEIGHT * CLIENT_ASPECT ) / 2.0f ), 0.0f ), Vector2( ( SCREEN_HEIGHT * CLIENT_ASPECT ), SCREEN_HEIGHT ) );

	Rgba startColor = Rgba( 0, ( RGBA_MAX / 2 ), ( RGBA_MAX / 2 ), RGBA_MAX );
	Rgba endColor = Rgba( ( RGBA_MAX / 2 ), 0, 0, RGBA_MAX );

	g_renderer->DrawAABB( screenLeftHalf, startColor );
	g_renderer->DrawAABB( screenRightHalf, endColor );
}

void AttractMenu::RenderInstructions() const
{
	if ( m_framesSinceAttractMenuCreated > 1 )
	{
		AABB2 screenBounds = AABB2( Vector2( 0.0f, 0.0f ), Vector2( ( SCREEN_HEIGHT * CLIENT_ASPECT ), SCREEN_HEIGHT ) );
		g_renderer->DrawTexturedAABB( screenBounds, *m_instructionTexture, Vector2( 0.0f, 0.0f ), Vector2( 1.0f, 1.0f ), Rgba( RGBA_MAX, RGBA_MAX, RGBA_MAX, RGBA_MAX ) );
	}
}

void AttractMenu::RenderAuthorName() const
{
	if ( m_framesSinceAttractMenuCreated > 1 )
	{
		AABB2 screenBounds = AABB2( Vector2( 0.0f, 0.0f ), Vector2( ( SCREEN_HEIGHT * CLIENT_ASPECT ), SCREEN_HEIGHT ) );
		g_renderer->DrawTexturedAABB( screenBounds, *m_authorNameTexture, Vector2( 0.0f, 0.0f ), Vector2( 1.0f, 1.0f ), Rgba( RGBA_MAX, RGBA_MAX, RGBA_MAX, RGBA_MAX ) );
	}
}

void AttractMenu::RenderTitle() const
{
	if ( m_framesSinceAttractMenuCreated > 1 )
	{
		AABB2 screenBounds = AABB2( Vector2( 0.0f, 0.0f ), Vector2( ( SCREEN_HEIGHT * CLIENT_ASPECT ), SCREEN_HEIGHT ) );
		g_renderer->DrawTexturedAABB( screenBounds, *m_titleTexture, Vector2( 0.0f, 0.0f ), Vector2( 1.0f, 1.0f ), Rgba( RGBA_MAX, RGBA_MAX, RGBA_MAX, RGBA_MAX ) );
	}
}

void AttractMenu::RenderLoadingText() const
{
	if ( m_framesSinceAttractMenuCreated == 1 )		// The audio is loaded at the end of the first frame, so only display this till audio is loaded
	{
		AABB2 screenBounds = AABB2( Vector2( 0.0f, 0.0f ), Vector2( ( SCREEN_HEIGHT * CLIENT_ASPECT ), SCREEN_HEIGHT ) );
		g_renderer->DrawTexturedAABB( screenBounds, *m_loadingTexture, Vector2( 0.0f, 0.0f ), Vector2( 1.0f, 1.0f ), Rgba( RGBA_MAX, RGBA_MAX, RGBA_MAX, RGBA_MAX ) );
	}
}

void AttractMenu::RenderStartPrompt() const
{
	if ( m_framesSinceAttractMenuCreated > 1 )
	{
		AABB2 screenBounds = AABB2( Vector2( 0.0f, 0.0f ), Vector2( ( SCREEN_HEIGHT * CLIENT_ASPECT ), SCREEN_HEIGHT ) );

		float clampedAlpha = ClampFloat( m_gameStartTextAlpha, 0.0f, 1.0f );
		unsigned char textAlpha = static_cast<unsigned char>( clampedAlpha * static_cast<float>( RGBA_MAX ) );
		Rgba renderColor = Rgba( RGBA_MAX, RGBA_MAX, RGBA_MAX, textAlpha );
		g_renderer->DrawTexturedAABB( screenBounds, *m_gameStartTexture, Vector2( 0.0f, 0.0f ), Vector2( 1.0f, 1.0f ), renderColor );
	}
}

int AttractMenu::GetFramesSinceAttractModeFirstLoaded() const
{
	return m_framesSinceAttractMenuCreated;
}
