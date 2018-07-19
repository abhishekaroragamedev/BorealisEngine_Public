#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Core/Image.hpp"

BitmapFont::BitmapFont( const SpriteSheet& spriteSheet, float baseAspect ) : m_spriteSheet( spriteSheet )
{
	m_baseAspect = baseAspect;
}

AABB2 BitmapFont::GetUVsForGlyph( int glyphUnicode ) const
{
	return m_spriteSheet.GetTextureCoordinatesForSpriteIndex( glyphUnicode );		// Assumes that the sprite index is the same as the glyph's unicode value
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
