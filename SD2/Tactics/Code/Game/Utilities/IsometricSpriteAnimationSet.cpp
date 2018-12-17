#include "Game/GameCommon.hpp"
#include "Game/Utilities/IsometricSpriteAnimationSet.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/XmlUtilities.hpp"
#include "ThirdParty/tinyXML2/tinyxml2.h"

IsometricSpriteAnimationSetDefinition::IsometricSpriteAnimationSetDefinition( const tinyxml2::XMLElement& isoSpriteAnimSetElement )
{
	m_spriteAnimSetDefinition = new SpriteAnimationSetDefinition( *isoSpriteAnimSetElement.FirstChildElement(), *g_renderer, "IdleTR" );
}

IsometricSpriteAnimationSetDefinition::~IsometricSpriteAnimationSetDefinition()
{
	delete m_spriteAnimSetDefinition;
	m_spriteAnimSetDefinition = nullptr;
}

SpriteAnimationSetDefinition* IsometricSpriteAnimationSetDefinition::GetSpriteAnimationSetDefinition() const
{
	return m_spriteAnimSetDefinition;
}

IsometricSpriteAnimationSet::IsometricSpriteAnimationSet( const IsometricSpriteAnimationSetDefinition& isoSpriteAnimSetDefinition )
	:	m_isoSpriteAnimSetDefinition( &isoSpriteAnimSetDefinition )
{
	m_spriteAnimSet = new SpriteAnimationSet( *m_isoSpriteAnimSetDefinition->GetSpriteAnimationSetDefinition() );
}

IsometricSpriteAnimationSet::~IsometricSpriteAnimationSet()
{
	delete m_spriteAnimSet;
	m_spriteAnimSet = nullptr;
}

void IsometricSpriteAnimationSet::Update( const Vector3& facingDirection )
{
	std::string animName = m_spriteAnimSet->GetCurrentSpriteAnimation()->GetDefinition()->GetName();
	m_spriteAnimSet->SetCurrentSpriteAnimation( IsometricSpriteAnimationSet::GetAnimationNameForFacingDirection( animName, facingDirection ) );

	float deltaSeconds = g_encounterGameState->GetClock()->GetDeltaSecondsF();
	m_spriteAnimSet->Update( deltaSeconds );
}

void IsometricSpriteAnimationSet::SetAnimation( const std::string& animationName, const Vector3& facingDirection )
{
	m_spriteAnimSet->SetCurrentSpriteAnimation( animationName + IsometricSpriteAnimationSet::GetSuffixForFacingDirection( facingDirection ) );
}

Sprite* IsometricSpriteAnimationSet::GetCurrentSprite() const
{
	return m_spriteAnimSet->GetCurrentSpriteFromSpriteAnimation();
}

SpriteAnimationSet* IsometricSpriteAnimationSet::GetSpriteAnimationSet() const
{
	return m_spriteAnimSet;
}

IsometricSpriteDirection IsometricSpriteAnimationSet::GetIsoSpriteDirectionForVector( const Vector3& actorForward )
{
	Vector2 actorForwardXZ = Vector2( actorForward.x, actorForward.z );
	Vector2 cameraForwardXZ = Vector2( g_renderer->GetCurrentCamera()->GetForward().x, g_renderer->GetCurrentCamera()->GetForward().z );

	bool isBack = DotProduct( cameraForwardXZ, actorForwardXZ ) > 0.0f;

	Vector2 cameraRightXZ = Vector2( g_renderer->GetCurrentCamera()->GetForward().z, -g_renderer->GetCurrentCamera()->GetForward().x );

	bool isRight = DotProduct( cameraRightXZ, actorForwardXZ ) > 0.0f;

	if ( isBack )
	{
		if ( isRight )
		{
			return IsometricSpriteDirection::ISO_BACK_RIGHT;
		}
		else
		{
			return IsometricSpriteDirection::ISO_BACK_LEFT;
		}
	}
	else
	{
		if ( isRight )
		{
			return IsometricSpriteDirection::ISO_FRONT_RIGHT;
		}
		else
		{
			return IsometricSpriteDirection::ISO_FRONT_LEFT;
		}
	}
}

std::string IsometricSpriteAnimationSet::GetSuffixForFacingDirection( const Vector3& facingDirection )
{
	IsometricSpriteDirection direction = IsometricSpriteAnimationSet::GetIsoSpriteDirectionForVector( facingDirection );

	switch( direction )
	{
		case IsometricSpriteDirection::ISO_BACK_LEFT	:
		case IsometricSpriteDirection::ISO_BACK_RIGHT	:	return std::string( ISO_SPRITE_NAME_AWAY_LEFT_SUFFIX );
		case IsometricSpriteDirection::ISO_FRONT_LEFT	:
		case IsometricSpriteDirection::ISO_FRONT_RIGHT	:	return std::string( ISO_SPRITE_NAME_TOWARD_RIGHT_SUFFIX );
		default:	return "";
	}
}

std::string IsometricSpriteAnimationSet::GetAnimationNameForFacingDirection( std::string& originalAnimName, const Vector3& facingDirection )
{
	if ( originalAnimName.find( "AL" ) > 0 )
	{
		originalAnimName = originalAnimName.substr( 0, originalAnimName.find( "AL" ) );
	}
	if ( originalAnimName.find( "TR" ) > 0 )
	{
		originalAnimName = originalAnimName.substr( 0, originalAnimName.find( "TR" ) );
	}

	return ( originalAnimName + IsometricSpriteAnimationSet::GetSuffixForFacingDirection( facingDirection ) );
}

Vector2 IsometricSpriteAnimationSet::GetScaleForFacingDirection( const Vector3& facingDirection )
{
	IsometricSpriteDirection direction = IsometricSpriteAnimationSet::GetIsoSpriteDirectionForVector( facingDirection );

	switch( direction )
	{
		case IsometricSpriteDirection::ISO_FRONT_LEFT	:
		case IsometricSpriteDirection::ISO_BACK_RIGHT	:	return Vector2( -1.0f, 1.0f );
		case IsometricSpriteDirection::ISO_BACK_LEFT	:
		case IsometricSpriteDirection::ISO_FRONT_RIGHT	:	return Vector2::ONE;
		default:	return Vector2::ZERO;
	}
}
