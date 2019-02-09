//-----------------------------------------------------------------------------------------------
// Texture.hpp
//
#pragma once
#include "Engine/Core/Image.hpp"
#include "Engine/Math/IntVector2.hpp"
#include "Engine/Renderer/RendererTypes.hpp"
#include <string>
#include <map>

enum TextureFormat 
{
	TEXTURE_FORMAT_INVALID = -1,
	TEXTURE_FORMAT_RGBA8,
	TEXTURE_FORMAT_RGBA32,
	TEXTURE_FORMAT_D24S8, 
};

//---------------------------------------------------------------------------
class Texture
{
	friend class ForwardRenderingPath;
	friend class FrameBuffer;
	friend class Renderer; // Textures are managed by a Renderer instance
	friend class Mesh;
	friend class OVRHeadset;

public:
	IntVector2 GetDimensions() const;
	Vector2 GetDimensionsF() const;
	TextureType GetType() const;
	bool UsesMipMaps() const;
	unsigned int GetNumMipLevels() const;
	unsigned int GetNumMSAASamples() const;

private:
	Texture();								// For use as a render target
	Texture( const std::string& filePath, unsigned int mipLevels = 1U, bool loadAsync = true, TextureFormat format = TEXTURE_FORMAT_RGBA8 ); // Use renderer->CreateOrGetTexture() instead!

public:
	~Texture();

	void DeleteGPUSide();

private:
	void CreateFromImage( const Image& image, unsigned int mipLevels = 1U, TextureFormat format = TextureFormat::TEXTURE_FORMAT_RGBA8 );
	void CreateFromFile( const std::string& imageFilePath, unsigned int mipLevels = 1U, bool loadAsync = true, TextureFormat format = TextureFormat::TEXTURE_FORMAT_RGBA8 );
	bool CreateRenderTarget( unsigned int width, unsigned int height, TextureFormat format );
	bool CreateMultiSampleTarget( unsigned int width, unsigned int height, TextureFormat format, unsigned int numSamples );
	inline bool IsValid() const { return ( m_textureID != 0U ); };

private:
	unsigned int m_textureID = 0U;
	IntVector2 m_dimensions = IntVector2::ZERO;
	TextureFormat m_format = TextureFormat::TEXTURE_FORMAT_RGBA8;
	TextureType m_type = TextureType::TEXTURE_TYPE_INVALID;
	unsigned int m_numMipLevels = 1U;
	unsigned int m_numMSAASamples = 0U;
};

void TextureFormatToGLFormats( unsigned int* internalFormat, unsigned int* channels, unsigned int* pixelLayout, TextureFormat format );
