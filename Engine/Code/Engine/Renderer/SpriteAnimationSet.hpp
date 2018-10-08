#pragma once

#include "Engine/Math/AABB2.hpp"
#include "Engine/Renderer/Sprite.hpp"
#include "Engine/Renderer/SpriteAnimation.hpp"
#include "Engine/Renderer/SpriteAnimationSetDefinition.hpp"
#include <string>
#include <map>

class SpriteAnimationSet
{

public:
	explicit SpriteAnimationSet( const SpriteAnimationSetDefinition& spriteAnimationSetDefinition );
	~SpriteAnimationSet();

public:
	void Update( float deltaSeconds );
	void SetCurrentSpriteAnimation( const std::string& nextSpriteAnimationName );	// Will not do anything if the provided name is that of the current playing animation
	SpriteAnimation* GetCurrentSpriteAnimation() const;
	const SpriteAnimationSetDefinition* GetSpriteAnimationSetDefinition() const;
	AABB2 GetFrameUVsFromSpriteAnimation( int frameIndex, const std::string& spriteAnimName );
	Sprite* GetCurrentSpriteFromSpriteAnimation();
	const Texture* GetTextureForSpriteAnimation( const std::string& spriteAnimName );

private:
	void PopulateSpriteAnimations( const SpriteAnimationSetDefinition& spriteAnimationSetDefinition );

private:
	static constexpr char SPRITE_ANIM_NAME_XML_ATTRIBUTE_NAME[] = "name";

private:
	std::map< std::string, SpriteAnimation* > m_spriteAnimationsByName;
	SpriteAnimation* m_currentSpriteAnimation = nullptr;
	const SpriteAnimationSetDefinition* m_spriteAnimationSetDefinition = nullptr;

};
