#include "Engine/Renderer/SpriteSheet.hpp"

SpriteSheet::SpriteSheet( const Texture& texture, int tilesWide /* = 0 */, int tilesHigh /* = 0 */, bool usesSprites /* = false */, bool flipY /* = false */ )
	:	m_spriteSheetTexture( texture ),
		m_usesSprites( usesSprites ),
		m_flipY( flipY )
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
	if ( m_usesSprites )
	{
		return AABB2::ZERO;
	}

	Vector2 textureMins = Vector2( ( static_cast<float>( spriteCoordinates.x ) / static_cast<float>( m_spriteLayout.x ) ), ( static_cast<float>( spriteCoordinates.y ) / static_cast<float>( m_spriteLayout.y ) ) );
	Vector2 textureMaxs = Vector2( ( static_cast<float>( spriteCoordinates.x + 1 ) / static_cast<float>( m_spriteLayout.x ) ), ( static_cast<float>( spriteCoordinates.y + 1 ) / static_cast<float>( m_spriteLayout.y ) ) );
	return AABB2( textureMins, textureMaxs );
}

AABB2 SpriteSheet::GetTextureCoordinatesForSpriteIndex( int spriteIndex ) const
{
	if ( !m_usesSprites )
	{
		IntVector2 spriteCoordinates = IntVector2( ( spriteIndex % m_spriteLayout.x ), ( spriteIndex / m_spriteLayout.x ) );
		return GetTextureCoordinatesForSpriteCoordinates( spriteCoordinates );
	}
	else
	{
		return m_sprites[ spriteIndex ]->GetUVs();
	}
}

Sprite* SpriteSheet::GetSprite( int spriteIndex ) const
{
	if ( !m_usesSprites )
	{
		return nullptr;
	}
	else
	{
		return m_sprites[ spriteIndex ];
	}
}

int SpriteSheet::GetNumSprites() const
{
	if ( !m_usesSprites )
	{
		return ( m_spriteLayout.x * m_spriteLayout.y );
	}
	else
	{
		return static_cast< int >( m_sprites.size() );
	}
}

bool SpriteSheet::UsesSprites() const
{
	return m_usesSprites;
}

int SpriteSheet::AddSprite( Sprite* sprite )
{
	if ( m_flipY )
	{
		sprite->m_UVs.mins.y = sprite->m_texture->GetDimensionsF().y - sprite->m_UVs.mins.y;
		sprite->m_UVs.maxs.y = sprite->m_texture->GetDimensionsF().y - sprite->m_UVs.maxs.y;
	}

	m_usesSprites = true;
	m_sprites.push_back( sprite );
	return static_cast< int >( m_sprites.size() - 1 );
}
