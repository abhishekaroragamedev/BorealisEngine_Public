#pragma once

#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "ThirdParty/tinyXML2/tinyxml2.h"

constexpr char SPRITE_SHEET_DIRECTORY_PATH[] = "Data/Images/";

class Renderer;

class Sprite
{

	friend class SpriteSheet;

public:
	Sprite( Texture* texture, AABB2 UVs, float PPU, Vector2 pivot = Vector2::ZERO );
	Sprite( const tinyxml2::XMLElement& spriteElement, Renderer& renderer, Texture* texture = nullptr );
	~Sprite();

	Texture* GetTexture() const;
	AABB2 GetUVs() const;
	AABB2 GetFlippedUVs( const Vector2& flip ) const;
	float GetPPU() const;
	Vector2 GetPivot() const;
	Vector2 GetRenderDimensions() const;

private:
	Texture* m_texture = nullptr;
	AABB2 m_UVs = AABB2::ZERO_TO_ONE;
	float m_PPU = 0.0f;
	Vector2 m_pivot = Vector2::ZERO;

};
