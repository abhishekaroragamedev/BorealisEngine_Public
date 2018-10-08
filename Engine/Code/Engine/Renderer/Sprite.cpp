#include "Engine/Core/XmlUtilities.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Sprite.hpp"
#include "Engine/Renderer/Texture.hpp"

Sprite::Sprite( Texture* texture, AABB2 UVs, float PPU, Vector2 pivot /* = Vector2::ZERO */ )
	:	m_texture( texture ),
		m_UVs( UVs ),
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
		m_texture = renderer.CreateOrGetTexture( SPRITE_SHEET_DIRECTORY_PATH + ParseXmlAttribute( *spriteElement.Parent()->Parent()->ToElement(), "spriteSheet", "" ) );
	}
	else // The Sprite has its own texture
	{
		m_texture = renderer.CreateOrGetTexture( SPRITE_SHEET_DIRECTORY_PATH + ParseXmlAttribute( spriteElement, "texture", "" ) );
	}
	
	m_UVs = ParseXmlAttribute( spriteElement, "uv", m_UVs );
	if ( m_UVs.GetDimensions().x > 1.0f || m_UVs.GetDimensions().y > 1.0f )
	{
		m_UVs /= m_texture->GetDimensionsF();
	}
	
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
	return m_UVs;
}

AABB2 Sprite::GetFlippedUVs( const Vector2& flip ) const
{
	AABB2 UVs = m_UVs;
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
	AABB2 scaledUpUVs = m_UVs;
	scaledUpUVs *= m_texture->GetDimensionsF();
	return scaledUpUVs.GetDimensions();
}
