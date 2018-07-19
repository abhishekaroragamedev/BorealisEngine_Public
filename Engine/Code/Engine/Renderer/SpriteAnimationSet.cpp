#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/SpriteAnimationSet.hpp"

SpriteAnimationSet::SpriteAnimationSet( const SpriteAnimationSetDefinition& spriteAnimationSetDefinition )
	:	m_spriteAnimationSetDefinition( &spriteAnimationSetDefinition )
{
	PopulateSpriteAnimations( spriteAnimationSetDefinition );
	m_currentSpriteAnimation = m_spriteAnimationsByName[ m_spriteAnimationSetDefinition->GetDefaultSpriteAnimationDefinitionName() ];
	m_currentSpriteAnimation->PlayOrResume();
}

SpriteAnimationSet::~SpriteAnimationSet()
{
	for ( std::map< std::string, SpriteAnimation* >::iterator mapIterator = m_spriteAnimationsByName.begin(); mapIterator != m_spriteAnimationsByName.end(); mapIterator++ )
	{
		delete mapIterator->second;
		mapIterator->second = nullptr;
	}
}

void SpriteAnimationSet::Update( float deltaSeconds )
{
	m_currentSpriteAnimation->Update( deltaSeconds );

	if ( m_currentSpriteAnimation->IsFinished() )
	{
		m_currentSpriteAnimation->Reset();
		m_currentSpriteAnimation->Pause();
		m_currentSpriteAnimation = m_spriteAnimationsByName[ m_spriteAnimationSetDefinition->GetDefaultSpriteAnimationDefinitionName() ];
		m_currentSpriteAnimation->PlayOrResume();
	}
}

SpriteAnimation* SpriteAnimationSet::GetCurrentSpriteAnimation() const
{
	return m_currentSpriteAnimation;
}

const SpriteAnimationSetDefinition* SpriteAnimationSet::GetSpriteAnimationSetDefinition() const
{
	return m_spriteAnimationSetDefinition;
}

void SpriteAnimationSet::SetCurrentSpriteAnimation( const std::string& nextSpriteAnimationName )
{
	ASSERT_OR_DIE( ( m_spriteAnimationsByName.find( nextSpriteAnimationName ) != m_spriteAnimationsByName.end() ), "SpriteAnimationSet::SetCurrentSpriteAnimation - no sprite animation with the provided name found. Aborting..." );
	m_currentSpriteAnimation = m_spriteAnimationsByName[ nextSpriteAnimationName ];
	m_currentSpriteAnimation->PlayOrResume();
}

void SpriteAnimationSet::PopulateSpriteAnimations( const SpriteAnimationSetDefinition& spriteAnimationSetDefinition )
{
	for ( std::map< std::string, SpriteAnimationDefinition* >::const_iterator mapIterator = m_spriteAnimationSetDefinition->m_spriteAnimationDefinitionsByName.begin(); mapIterator != m_spriteAnimationSetDefinition->m_spriteAnimationDefinitionsByName.end(); mapIterator++ )
	{
		m_spriteAnimationsByName[ mapIterator->first ] = new SpriteAnimation( mapIterator->second );
	}
}

AABB2 SpriteAnimationSet::GetFrameUVsFromSpriteAnimation( int frameIndex, const std::string& spriteAnimName )
{
	ASSERT_OR_DIE( ( m_spriteAnimationsByName.find( spriteAnimName ) != m_spriteAnimationsByName.end() ), "SpriteAnimationSet::GetFrameUVsFromSpriteAnimation - no sprite animation with the provided name found. Aborting..." );
	int actualSpriteIndexInSpriteSheet = m_spriteAnimationsByName[ spriteAnimName ]->GetDefinition()->GetSpriteIndices()[ frameIndex ];
	return m_spriteAnimationsByName[ spriteAnimName ]->GetDefinition()->GetSpriteSheet()->GetTextureCoordinatesForSpriteIndex( actualSpriteIndexInSpriteSheet );
}

const Texture* SpriteAnimationSet::GetTextureForSpriteAnimation( const std::string& spriteAnimName )
{
	ASSERT_OR_DIE( ( m_spriteAnimationsByName.find( spriteAnimName ) != m_spriteAnimationsByName.end() ), "SpriteAnimationSet::GetTextureForSpriteAnimation - no sprite animation with the provided name found. Aborting..." );
	return m_spriteAnimationsByName[ spriteAnimName ]->GetDefinition()->GetSpriteSheet()->GetTexture();
}
