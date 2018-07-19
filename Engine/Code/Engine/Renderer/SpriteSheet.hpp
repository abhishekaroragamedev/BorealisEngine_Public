#pragma once

#include "Engine/Math/AABB2.hpp"
#include "Engine/Renderer/Texture.hpp"

constexpr char SPRITE_SHEET_DIRECTORY_PATH[] = "Data/Images/";

class SpriteSheet
{
public:
	explicit SpriteSheet( const Texture& texture, int tilesWide, int tilesHigh );
	~SpriteSheet();

public:
	const Texture* GetTexture() const;
	AABB2 GetTextureCoordinatesForSpriteCoordinates( const IntVector2& spriteCoordinates ) const;
	AABB2 GetTextureCoordinatesForSpriteIndex( int spriteIndex ) const;
	int GetNumSprites() const;

private:
	const Texture& m_spriteSheetTexture;
	IntVector2 m_spriteLayout = IntVector2::ZERO;

};
