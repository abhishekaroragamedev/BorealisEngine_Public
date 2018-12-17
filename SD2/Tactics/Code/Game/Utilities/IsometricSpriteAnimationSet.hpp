#pragma once

#include "Engine/Renderer/SpriteAnimationSet.hpp"
#include "Engine/Renderer/SpriteAnimationSetDefinition.hpp"
#include "ThirdParty/tinyXML2/tinyxml2.h"
#include <map>

enum IsometricSpriteDirection
{
	ISO_INVALID = -1,
	ISO_FRONT_LEFT,
	ISO_FRONT_RIGHT,
	ISO_BACK_LEFT,
	ISO_BACK_RIGHT,
	ISO_NUM_DIRECTIONS
};

constexpr char ISO_SPRITE_NAME_TOWARD_RIGHT_SUFFIX[] = "TR";
constexpr char ISO_SPRITE_NAME_AWAY_LEFT_SUFFIX[] = "AL";

class IsometricSpriteAnimationSetDefinition
{

	friend class IsometricSpriteAnimationSet;

public:
	IsometricSpriteAnimationSetDefinition( const tinyxml2::XMLElement& isoSpriteAnimSetElement );
	~IsometricSpriteAnimationSetDefinition();

	SpriteAnimationSetDefinition* GetSpriteAnimationSetDefinition() const;

private:
	SpriteAnimationSetDefinition* m_spriteAnimSetDefinition = nullptr;

};

class IsometricSpriteAnimationSet	// Assumes each sprite animation name can be suffixed with TR/AL to indicate which direction/scale to use
{

public:
	IsometricSpriteAnimationSet( const IsometricSpriteAnimationSetDefinition& isoSpriteAnimSetDefinition );
	~IsometricSpriteAnimationSet();

	void Update( const Vector3& facingDirection );
	void SetAnimation( const std::string& animationName, const Vector3& facingDirection );	// Uses the current camera's transform to determine which version of the animation to use
	Sprite* GetCurrentSprite() const;
	SpriteAnimationSet* GetSpriteAnimationSet() const;

public:
	static IsometricSpriteDirection GetIsoSpriteDirectionForVector( const Vector3& actorForward );	// Uses the current camera's transform to compute this
	static std::string GetSuffixForFacingDirection( const Vector3& facingDirection );
	static std::string GetAnimationNameForFacingDirection( std::string& originalAnimName, const Vector3& facingDirection );
	static Vector2 GetScaleForFacingDirection( const Vector3& facingDirection );

private:
	SpriteAnimationSet* m_spriteAnimSet = nullptr;
	const IsometricSpriteAnimationSetDefinition* m_isoSpriteAnimSetDefinition = nullptr;

};
