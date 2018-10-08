#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Core/Image.hpp"

BitmapFont::BitmapFont( const SpriteSheet& spriteSheet, float baseAspect ) : m_spriteSheet( spriteSheet )
{
	m_baseAspect = baseAspect;
}

AABB2 BitmapFont::GetUVsForGlyph( int glyphUnicode ) const
{
	// Flip Y around because the UV origin is the bottom-left, while the spritesheet's "ASCII origin" is the top-left
	int indexX = glyphUnicode % BITMAP_FONT_SPRITESHEET_WIDTH_TILES;
	int indexY = glyphUnicode / BITMAP_FONT_SPRITESHEET_HEIGHT_TILES;
	indexY = ( BITMAP_FONT_SPRITESHEET_HEIGHT_TILES - 1 ) - indexY;
	return m_spriteSheet.GetTextureCoordinatesForSpriteIndex( ( BITMAP_FONT_SPRITESHEET_WIDTH_TILES * indexY ) + indexX );
}

float BitmapFont::GetGlyphAspect( int glyphUnicode ) const
{
	return m_baseAspect;
}

float BitmapFont::GetCharacterWidth( float cellHeight, float aspectScale ) const
{
	return ( cellHeight * ( aspectScale * m_baseAspect ) );
}

float BitmapFont::GetStringWidth( const std::string& asciiText, float cellHeight, float aspectScale ) const
{
	float stringWidthInCharacters = static_cast<float>( asciiText.length() );
	return ( stringWidthInCharacters * ( cellHeight * ( aspectScale * m_baseAspect ) ) );
}

const Texture* BitmapFont::GetTexture() const
{
	return m_spriteSheet.GetTexture();
}
