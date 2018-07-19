#include "Game/GameCommon.hpp"
#include "Game/VictoryScreen.hpp"

VictoryScreen::VictoryScreen()
{
	m_timeSinceLastInit = 0.0f;
	m_victoryScreenAlpha = 0.0f;
	m_victoryScreenTexture = g_renderer->CreateOrGetTexture( VICTORY_SCREEN_TEXTURE_PATH );
}

VictoryScreen::~VictoryScreen()
{

}

void VictoryScreen::Update( float deltaSeconds )
{
	UpdateAlpha( deltaSeconds );

	m_timeSinceLastInit++;
}

void VictoryScreen::Render() const
{
	RenderVictoryScreen();
}

float VictoryScreen::GetTimeSinceLastInit() const
{
	return m_timeSinceLastInit;
}

void VictoryScreen::InitializeVictoryScreen()
{
	m_timeSinceLastInit = 0.0f;
	m_victoryScreenAlpha = 0.0f;
	
	SoundPlaybackID victorySound = g_audioSystem->PlaySound( g_audioSystem->CreateOrGetSound( AUDIO_VICTORY ) );
	m_playbackIdsOfSoundsPlaying.push_back( victorySound );
}

void VictoryScreen::FlushSoundsPlaying()
{
	for ( std::vector<SoundPlaybackID>::iterator soundIterator = m_playbackIdsOfSoundsPlaying.begin(); soundIterator != m_playbackIdsOfSoundsPlaying.end(); soundIterator++ )
	{
		g_audioSystem->StopSound( *soundIterator );
	}

	m_playbackIdsOfSoundsPlaying.clear();
}

void VictoryScreen::UpdateAlpha( float deltaSeconds )
{
	float alphaIncrement = deltaSeconds / VICTORY_SCREEN_FADE_IN_TIME_SECONDS;
	m_victoryScreenAlpha += alphaIncrement;
	m_victoryScreenAlpha = ClampFloat( m_victoryScreenAlpha, 0.0f, 1.0f );
}

void VictoryScreen::RenderVictoryScreen() const
{
	AABB2 screenBounds = AABB2( Vector2( 0.0f, 0.0f ), Vector2( ( SCREEN_HEIGHT * CLIENT_ASPECT ), SCREEN_HEIGHT ) );

	g_renderer->SetOrtho( screenBounds.mins, screenBounds.maxs );

	Rgba quadColor = Rgba( 0, 0, 0, static_cast<unsigned char>( ( m_victoryScreenAlpha * static_cast<float>( RGBA_MAX ) ) / 1.5f ) );
	g_renderer->DrawAABB( screenBounds, quadColor );

	Rgba textureColor = Rgba( RGBA_MAX, RGBA_MAX, RGBA_MAX, RGBA_MAX); //static_cast<unsigned char>( ( m_victoryScreenAlpha * static_cast<float>( RGBA_MAX ) ) ) );
	g_renderer->DrawTexturedAABB( screenBounds, *m_victoryScreenTexture, Vector2( 0.0f, 0.0f ), Vector2( 1.0f, 1.0f ), textureColor );
}
