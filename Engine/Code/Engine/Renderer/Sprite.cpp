#include "Engine/Core/XmlUtilities.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Sprite.hpp"
#include "Engine/Renderer/Texture.hpp"

Sprite::Sprite( Texture* texture, AABB2 UVs, float PPU, Vector2 pivot /* = Vector2::ZERO */ )
	:	m_texture( texture ),
		m_UVPixels( UVs ),
		m_PPU( PPU ),
		m_pivot( pivot )
{

}

Sprite::Sprite( const tinyxml2::XMLElement& spriteElement, Renderer& renderer, Texture* texture /* = nullptr */ )
{
	if ( texture != nullptr )
	{
		m_texture = texture;
	}
	else if		(	spriteElement.Parent() != nullptr &&			// <SpriteAnimSet> defines a shared texture
					spriteElement.Parent()->Parent() != nullptr &&
					spriteElement.Parent()->Parent()->ToElement() != nullptr &&
					ParseXmlAttribute( *spriteElement.Parent()->Parent()->ToElement(), "spriteSheet", "" ) != ""
				)	
	{
		m_texture = renderer.CreateOrGetTexture( SPRITE_SHEET_DIRECTORY_PATH + ParseXmlAttribute( *spriteElement.Parent()->Parent()->ToElement(), "spriteSheet", "" ), 1U, false ); // Sprite system doesn't support async load as it needs dimensions defined beforehand
	}
	else // The Sprite has its own texture
	{
		m_texture = renderer.CreateOrGetTexture( SPRITE_SHEET_DIRECTORY_PATH + ParseXmlAttribute( spriteElement, "texture", "" ), 1U, false ); // Sprite system doesn't support async load as it needs dimensions defined beforehand
	}
	
	m_UVPixels = ParseXmlAttribute( spriteElement, "uv", m_UVPixels );
	
	m_PPU = ParseXmlAttribute( spriteElement, "ppu", m_PPU );
	m_pivot = ParseXmlAttribute( spriteElement, "pivot", m_pivot );
}

Sprite::~Sprite()
{

}

Texture* Sprite::GetTexture() const
{
	return m_texture;
}

AABB2 Sprite::GetUVs() const
{
	Vector2 dimensions = m_texture->GetDimensionsF();
	return AABB2(
		(m_UVPixels.mins / dimensions),
		(m_UVPixels.maxs / dimensions)
	);
}

AABB2 Sprite::GetFlippedUVs( const Vector2& flip ) const
{
	AABB2 UVs = GetUVs();
	if ( flip.x < 0.0f )
	{
		float temp = UVs.mins.x;
		UVs.mins.x = UVs.maxs.x;
		UVs.maxs.x = temp;
	}
	if ( flip.y < 0.0f )
	{
		float temp = UVs.mins.y;
		UVs.mins.y = UVs.maxs.y;
		UVs.maxs.y = temp;
	}

	return UVs;
}

float Sprite::GetPPU() const
{
	return m_PPU;
}

Vector2 Sprite::GetPivot() const
{
	return m_pivot;
}

Vector2 Sprite::GetRenderDimensions() const
{
	return m_UVPixels.GetDimensions() / m_PPU;
}
