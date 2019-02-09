#pragma once

#include "Engine/Core/Image.hpp"
#include "Engine/Renderer/RendererTypes.hpp"
#include "Engine/Renderer/Texture.hpp"

class TextureCube
{

public:
	TextureCube();
	virtual ~TextureCube();

	void Cleanup();

	// Make a cube map from 6 images
	// +x -x +y -y +z -z
	bool MakeFromImages( const Image* images );

	// Todo - may want this to take tile offsets
	// into the image since I can't assume the same
	// plus sign format my demo one is in
	bool MakeFromImage( const Image& image );
	bool MakeFromImage( const char* filename, bool loadAsync = true ); 

	// Cube maps should be square on each face
	inline unsigned int GetWidth() const { return m_size; }
	inline unsigned int GetHeight() const { return m_size; }

	inline bool IsValid() const { return ( m_handle != 0U ); }

public:
	// GL handle
	inline unsigned int GetHandle() const { return m_handle; }

public:
	unsigned int m_size = 0U;	// Cube maps are equal length on all sizes
	TextureFormat m_format = TextureFormat::TEXTURE_FORMAT_INVALID; 
	// GL 
	unsigned int m_handle = 0U; 

};
