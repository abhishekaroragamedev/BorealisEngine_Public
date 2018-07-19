#include "Engine/Renderer/SpriteSheet.hpp"

SpriteSheet::SpriteSheet( const Texture& texture, int tilesWide, int tilesHigh ) : m_spriteSheetTexture( texture )
{
	m_spriteLayout = IntVector2( tilesWide, tilesHigh );
}

SpriteSheet::~SpriteSheet()
{

}

const Texture* SpriteSheet::GetTexture() const
{
	return ( &m_spriteSheetTexture );
}

AABB2 SpriteSheet::GetTextureCoordinatesForSpriteCoordinates( const IntVector2& spriteCoordinates ) const
{
	Vector2 textureMins = Vector2( ( static_cast<float>( spriteCoordinates.x ) / static_cast<float>( m_spriteLayout.x ) ), ( static_cast<float>( spriteCoordinates.y ) / static_cast<float>( m_spriteLayout.y ) ) );
	Vector2 textureMaxs = Vector2( ( static_cast<float>( spriteCoordinates.x + 1 ) / static_cast<float>( m_spriteLayout.x ) ), ( static_cast<float>( spriteCoordinates.y + 1 ) / static_cast<float>( m_spriteLayout.y ) ) );
	return AABB2( textureMins, textureMaxs );
}

AABB2 SpriteSheet::GetTextureCoordinatesForSpriteIndex( int spriteIndex ) const
{
	IntVector2 spriteCoordinates = IntVector2( ( spriteIndex % m_spriteLayout.x ), ( spriteIndex / m_spriteLayout.x ) );
	return GetTextureCoordinatesForSpriteCoordinates( spriteCoordinates );
}

int SpriteSheet::GetNumSprites() const
{
	return ( m_spriteLayout.x * m_spriteLayout.y );
}
