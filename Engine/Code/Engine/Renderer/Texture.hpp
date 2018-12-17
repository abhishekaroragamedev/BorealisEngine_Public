//-----------------------------------------------------------------------------------------------
// Texture.hpp
//
#pragma once
#include "Engine/Math/IntVector2.hpp"
#include <string>
#include <map>


//---------------------------------------------------------------------------
class Texture
{
	friend class Renderer; // Textures are managed by a Renderer instance

private:
	Texture( const std::string& imageFilePath ); // Use renderer->CreateOrGetTexture() instead!
	void PopulateFromData( unsigned char* imageData, const IntVector2& texelSize, int numComponents );

private:
	unsigned int m_textureID = 0;
	IntVector2 m_dimensions = IntVector2::ZERO;
};


