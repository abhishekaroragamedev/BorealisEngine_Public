#include "Game/Entities/Explosion.hpp"
#include "Game/GameCommon.hpp"

Explosion::Explosion( const Vector2& spawnLocation, float size, float durationSeconds )
{
	m_location = Vector2( spawnLocation );
	m_size = size;
	m_explosionAnimation = new SpriteAnimation( *g_theWorld->GetExplosionSpritesheet(), durationSeconds, SpriteAnimMode::SPRITE_ANIM_MODE_PLAY_TO_END, 0, ( EXPLOSION_SPRITESHEET_NUM_SPRITES - 1 ) );
	m_explosionAnimation->PlayOrResume();
	
	SoundPlaybackID explosionSound = PlayRandomSoundFor( IncursionAudioType::EXPLOSION, ( g_theWorld->GetPlayerSoundMuteFraction() * ( size / TILE_SIDE_LENGTH_WORLD_UNITS ) ) );		// Mute the explosion progressively as the player takes damage
	Vector2 positionRelativeToPlayer = m_location - g_theWorld->GetPlayerTank()->GetLocation();
	positionRelativeToPlayer.x /= g_theWorld->GetCurrentMap()->GetBoundsInWorldCoordinates().maxs.x;
	g_audioSystem->SetSoundPlaybackBalance( explosionSound,  SmoothStop4( positionRelativeToPlayer.x ) );
	g_theWorld->AddSoundPlaying( explosionSound );

	m_velocity = Vector2( 0.0f ,0.0f );
	m_orientation = GetRandomFloatInRange( -180.0f, 180.0f );
	m_rotationSpeed = 0.0f;
	m_speed = 0.0f;
	m_health = 0;
	m_isDead = false;

	PopulateDeveloperModeCirclesVertices( m_size, m_size );
}

Explosion::~Explosion()
{
	delete m_explosionAnimation;
	m_explosionAnimation = nullptr;
}

void Explosion::Update( float deltaSeconds )
{
	if ( m_explosionAnimation->IsFinished() )
	{
		m_isDead = true;
	}
	else
	{
		m_explosionAnimation->Update( deltaSeconds );
	}
}

void Explosion::Render( bool developerModeEnabled ) const
{
	g_renderer->SetBlendMode( RendererBlendMode::ADDITIVE );

	Rgba explosionTint = Rgba( RGBA_MAX, RGBA_MAX, RGBA_MAX, RGBA_MAX );

	Vector2 bottomLeftDrawCoords = Vector2( -m_size, -m_size );
	Vector2 topRightDrawCoords = Vector2( m_size, m_size );

	AABB2 drawCoords = AABB2( bottomLeftDrawCoords, topRightDrawCoords );
	AABB2 currentTextureCoords = m_explosionAnimation->GetCurrentTexCoords();
	g_renderer->DrawTexturedAABB( drawCoords, *( g_theWorld->GetExplosionSpritesheet()->GetTexture() ), currentTextureCoords.mins, currentTextureCoords.maxs, explosionTint );

	g_renderer->SetBlendMode( RendererBlendMode::ALPHA );

	if ( developerModeEnabled )
	{
		//RenderDeveloperMode(); // TODO: See if you really want to do this
	}
}
