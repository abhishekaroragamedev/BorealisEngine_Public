#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/XmlUtilities.hpp"
#include "Engine/Math/IntRange.hpp"
#include "Engine/Renderer/SpriteAnimation.hpp"

SpriteAnimation::SpriteAnimation( SpriteSheet& spriteSheet, float durationSeconds, SpriteAnimMode playbackMode, int startSpriteIndex, int endSpriteIndex )
{
	IntRange spriteIndexRange = IntRange( startSpriteIndex, endSpriteIndex );
	float fps = static_cast< float >( ( endSpriteIndex - startSpriteIndex ) + 1 ) / durationSeconds;
	m_spriteAnimDefinition = new SpriteAnimationDefinition( spriteSheet, fps, playbackMode, spriteIndexRange.GetAllIntsInRange() );
	m_wasInitializedFromDefinition = false;
	m_elapsedSeconds = 0.0f;
	m_isPlaying = false;
	m_isFinished = false;
	m_playDirection = 1;		// Play forward initially by default
	m_currentSpriteIndexRelative = 0;
}

SpriteAnimation::SpriteAnimation( SpriteSheet& spriteSheet, float durationSeconds, SpriteAnimMode playbackMode, std::vector< int > spriteIndices )
{
	float fps = static_cast< float >( spriteIndices.size() ) / durationSeconds;
	m_spriteAnimDefinition = new SpriteAnimationDefinition( spriteSheet, fps, playbackMode, spriteIndices );
	m_wasInitializedFromDefinition = false;
	m_elapsedSeconds = 0.0f;
	m_isPlaying = false;
	m_isFinished = false;
	m_playDirection = 1;		// Play forward initially by default
	m_currentSpriteIndexRelative = 0;
}

SpriteAnimation::SpriteAnimation( const SpriteAnimationDefinition* spriteAnimDefinition )
{
	m_spriteAnimDefinition = spriteAnimDefinition;
	m_wasInitializedFromDefinition = true;
	m_elapsedSeconds = 0.0f;
	m_isPlaying = false;
	m_isFinished = false;
	m_playDirection = 1;		// Play forward initially by default
	m_currentSpriteIndexRelative = 0;
}

SpriteAnimation::~SpriteAnimation()
{
	if ( !m_wasInitializedFromDefinition )
	{
		delete m_spriteAnimDefinition;
	}

	m_spriteAnimDefinition = nullptr;
}

void SpriteAnimation::Update( float deltaSeconds )
{
	if ( m_isPlaying && !m_isFinished )
	{
		m_elapsedSeconds += deltaSeconds;

		if ( m_elapsedSeconds > GetCurrentSpriteMaxDuration() )
		{
			switch ( m_spriteAnimDefinition->GetPlaybackMode() )		// Calculate the next sprite index based on the play mode
			{
				case SpriteAnimMode::SPRITE_ANIM_MODE_PLAY_TO_END:
				{
					m_currentSpriteIndexRelative += 1;
					if ( m_elapsedSeconds >= m_spriteAnimDefinition->GetDurationSeconds() )
					{
						m_currentSpriteIndexRelative = m_spriteAnimDefinition->GetEndSpriteIndex() - m_spriteAnimDefinition->GetStartSpriteIndex();
						m_isFinished = true;
						m_isPlaying = false;
						m_elapsedSeconds = 0.0f;
					}
				}
				break;

				case SpriteAnimMode::SPRITE_ANIM_MODE_LOOPING:
				{
					m_currentSpriteIndexRelative = ( m_currentSpriteIndexRelative + 1 ) % m_spriteAnimDefinition->GetNumberOfSpritesInAnimation();

					if ( m_currentSpriteIndexRelative == 0 )
					{
						m_elapsedSeconds = 0.0f;
					}
				}
				break;

				case SpriteAnimMode::SPRITE_ANIM_MODE_PINGPONG:
				{
					m_currentSpriteIndexRelative += m_playDirection;

					if ( m_currentSpriteIndexRelative == -1 )
					{
						m_playDirection = 1;
						m_currentSpriteIndexRelative++;
						m_elapsedSeconds = 0.0f;
					}
					else if ( m_currentSpriteIndexRelative == m_spriteAnimDefinition->GetNumberOfSpritesInAnimation() )
					{
						m_playDirection = -1;
						m_currentSpriteIndexRelative--;
						m_elapsedSeconds = 0.0f;
					}
				}
			}
		}
	}
}

const SpriteAnimationDefinition* SpriteAnimation::GetDefinition() const
{
	return m_spriteAnimDefinition;
}

AABB2 SpriteAnimation::GetCurrentTexCoords() const
{
	return m_spriteAnimDefinition->GetSpriteSheet()->GetTextureCoordinatesForSpriteIndex( m_spriteAnimDefinition->GetSpriteIndices()[ m_currentSpriteIndexRelative ] );
}

Sprite* SpriteAnimation::GetCurrentSprite() const
{
	return m_spriteAnimDefinition->GetSpriteSheet()->GetSprite( m_spriteAnimDefinition->GetSpriteIndices()[ m_currentSpriteIndexRelative ] );
}

void SpriteAnimation::Pause()
{
	m_isPlaying = false;
}

void SpriteAnimation::PlayOrResume()
{
	m_isPlaying = true;
}

void SpriteAnimation::Reset()
{
	m_currentSpriteIndexRelative = 0;
	m_elapsedSeconds = 0.0f;
	m_isFinished = false;
	m_isPlaying = true;
}

bool SpriteAnimation::IsFinished() const
{
	return m_isFinished;
}

bool SpriteAnimation::IsPlaying() const
{
	return m_isPlaying;
}

float SpriteAnimation::GetSecondsElapsed() const
{
	return m_elapsedSeconds;
}

float SpriteAnimation::GetSecondsRemaining() const
{
	return ( m_spriteAnimDefinition->GetDurationSeconds() - m_elapsedSeconds );
}

float SpriteAnimation::GetFractionElapsed() const
{
	return ( m_elapsedSeconds / m_spriteAnimDefinition->GetDurationSeconds() );
}

float SpriteAnimation::GetFractionRemaining() const
{
	return ( 1 - ( m_elapsedSeconds / m_spriteAnimDefinition->GetDurationSeconds() ) );
}

int SpriteAnimation::GetCurrentIndex() const
{
	return m_currentSpriteIndexRelative;
}

void SpriteAnimation::SetSecondsElapsed( float secondsElapsed )
{
	m_elapsedSeconds = secondsElapsed;
}

void SpriteAnimation::SetFractionElapsed( float fractionElapsed )
{
	m_elapsedSeconds = fractionElapsed * m_spriteAnimDefinition->GetDurationSeconds();
}

float SpriteAnimation::GetCurrentSpriteMaxDuration() const
{
	return ( m_spriteAnimDefinition->GetSingleSpriteDurationSeconds() * static_cast<float>( m_currentSpriteIndexRelative + 1 ) );
}
