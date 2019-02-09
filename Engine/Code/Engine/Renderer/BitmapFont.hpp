#pragma once

#include "Engine/Math/AABB2.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include <string>

constexpr int BITMAP_FONT_SPRITESHEET_WIDTH_TILES = 16;
constexpr int BITMAP_FONT_SPRITESHEET_HEIGHT_TILES = 16;
constexpr char BITMAP_FONT_PATH[] = "Data/Fonts/";
constexpr char BITMAP_FONT_EXTENSION[] = ".png";

class BitmapFont
{

	friend class Renderer;

public:
	AABB2 GetUVsForGlyph( int glyphUnicode ) const;
	float GetGlyphAspect( int glyphUnicode ) const;
	float GetCharacterWidth( float cellHeight, float aspectScale ) const;
	float GetStringWidth( const std::string& asciiText, float cellHeight, float aspectScale ) const;
	const Texture* GetTexture() const;

private:
	explicit BitmapFont( const SpriteSheet& spriteSheet, float baseAspect );

private:
	const SpriteSheet&	m_spriteSheet;
	float m_baseAspect = 1.0f;

};
