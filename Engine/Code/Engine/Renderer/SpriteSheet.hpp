#pragma once

#include "Engine/Math/AABB2.hpp"
#include "Engine/Renderer/Sprite.hpp"
#include "Engine/Renderer/Texture.hpp"
#include <string>
#include <vector>

class SpriteSheet
{
public:
	explicit SpriteSheet( Texture& texture, int tilesWide = 0, int tilesHigh = 0, bool usesSprites = false, bool flipY = false );
	~SpriteSheet();

public:
	Texture* GetTexture() const;
	AABB2 GetTextureCoordinatesForSpriteCoordinates( const IntVector2& spriteCoordinates ) const;	// Gives defined value only if UsesSprites() returns false
	AABB2 GetTextureCoordinatesForSpriteIndex( int spriteIndex ) const;
	Sprite* GetSprite( int spriteIndex ) const;
	int GetNumSprites() const;
	bool UsesSprites() const;
	int AddSprite( Sprite* sprite );

private:
	Texture& m_spriteSheetTexture;
	IntVector2 m_spriteLayout = IntVector2::ZERO;
	bool m_usesSprites = false;
	bool m_flipY = false;
	std::vector< Sprite* > m_sprites;

};
