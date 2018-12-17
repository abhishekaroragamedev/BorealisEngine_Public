#pragma once

#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "ThirdParty/tinyXML2/tinyxml2.h"
#include <vector>

enum SpriteAnimMode
{
	SPRITE_ANIM_MODE_INVALID = -1,
	SPRITE_ANIM_MODE_PLAY_TO_END,
	SPRITE_ANIM_MODE_LOOPING,
	SPRITE_ANIM_MODE_PINGPONG,
	NUM_SPRITE_ANIM_MODES

};

class SpriteAnimationDefinition
{

public:
	explicit SpriteAnimationDefinition( const SpriteSheet& spriteSheet, float defaultFps, const SpriteAnimMode& playbackMode, const std::vector< int >& spriteIndices );
	explicit SpriteAnimationDefinition( const tinyxml2::XMLElement& spriteAnimElement, float defaultFps, Renderer& renderer );

public:
	~SpriteAnimationDefinition();

public:
	float GetDurationSeconds() const;
	float GetSingleSpriteDurationSeconds() const;
	int GetStartSpriteIndex() const;
	int GetEndSpriteIndex() const;
	float GetOrientationOffset() const;
	std::vector< int > GetSpriteIndices() const;
	int GetNumberOfSpritesInAnimation() const;
	SpriteAnimMode GetPlaybackMode() const;
	const SpriteSheet* GetSpriteSheet() const;
	const Texture* GetTexture() const;
	std::string GetName() const;

private:
	void PopulateSpriteSheetInfo( const tinyxml2::XMLElement& spriteAnimElement, Renderer& renderer );
	void SetDurationForEachSpriteSeconds();

private:
	static constexpr char NAME_XML_ATTRIBUTE_NAME[] = "name";
	static constexpr char SPRITE_SHEET_XML_ATTRIBUTE_NAME[] = "spriteSheet";
	static constexpr char SPRITE_LAYOUT_XML_ATTRIBUTE_NAME[] = "spriteLayout";
	static constexpr char PLAYBACK_MODE_ATTRIBUTE_NAME[] = "playbackMode";
	static constexpr char SPRITE_FPS_XML_ATTRIBUTE_NAME[] = "fps";
	static constexpr char SPRITE_INDICES_XML_ATTRIBUTE_NAME[] = "spriteIndices";
	static constexpr char SPRITE_ORIENTATION_OFFSET_XML_ATTRIBUTE_NAME[] = "orientationOffset";

private:
	float m_durationSeconds = 0.0f;
	float m_singleSpriteDurationSeconds = 0.0f;
	float m_fps = 1.0f;
	float m_orientationOffsetDegrees = 0.0f;		// The offset at which the sprite for this entity is oriented (0 means the sprite is aligned with the +x axis)
	std::string m_name = "";
	std::vector< int > m_spriteIndices;
	const SpriteSheet* m_spriteSheet = nullptr;
	SpriteAnimMode m_playbackMode = SpriteAnimMode::SPRITE_ANIM_MODE_PLAY_TO_END;

};
