#pragma once

#include "Engine/Math/IntVector3.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Renderer/Texture.hpp"

class Texture3D
{

	friend class Renderer;

public:
	Texture3D();
	~Texture3D();

	IntVector3 GetDimensions() const;
	Vector3 GetDimensionsF() const;
	bool UsesMipMaps() const;
	unsigned int GetNumMipLevels() const;
	inline bool IsValid() const { return ( m_textureID != 0U ); };

	void DeleteGPUSide();

	void CreateFromImages( unsigned int imageCount, const Image* images, unsigned int mipLevels = 1U, TextureFormat format = TextureFormat::TEXTURE_FORMAT_RGBA8 );
	void CreateFromFiles( unsigned int numFiles, const std::string* fileNames, unsigned int mipLevels = 1U, TextureFormat format = TextureFormat::TEXTURE_FORMAT_RGBA8 );
	void RegenerateMipMaps();	// Assumes mipCount was specified on creation

private:
	unsigned int m_textureID = 0U;
	IntVector3 m_dimensions = IntVector3::ZERO;
	TextureFormat m_format = TextureFormat::TEXTURE_FORMAT_RGBA8;
	unsigned int m_numMipLevels = 1U;

};
