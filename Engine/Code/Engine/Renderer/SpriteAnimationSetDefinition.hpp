#pragma once

#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/SpriteAnimationDefinition.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/Texture.hpp"

class SpriteAnimationSetDefinition
{

	friend class SpriteAnimationSet;

public:
	explicit SpriteAnimationSetDefinition( const tinyxml2::XMLElement& spriteAnimSetElement, Renderer& renderer );
	~SpriteAnimationSetDefinition();

public:
	SpriteAnimationDefinition* GetSpriteAnimationDefinitionFromName( const std::string& spriteAnimationDefinitionName );
	SpriteAnimationDefinition* GetDefaultSpriteAnimationDefinition();
	std::string GetDefaultSpriteAnimationDefinitionName() const;

private:
	void PopulateSpriteAnimations( const tinyxml2::XMLElement& spriteAnimElement, Renderer& renderer );

private:
	static constexpr char SPRITE_LAYOUT_XML_ATTRIBUTE_NAME[] = "spriteLayout";
	static constexpr char SPRITE_FPS_XML_ATTRIBUTE_NAME[] = "fps";
	static constexpr char SPRITE_ANIM_DEF_NAME_XML_ATTRIBUTE_NAME[] = "name";

private:
	std::map< std::string, SpriteAnimationDefinition* > m_spriteAnimationDefinitionsByName;
	std::string m_defaultAnimName = "Idle";		// TODO: Add some way to modify this?
	float m_fps = 1.0f;

};
