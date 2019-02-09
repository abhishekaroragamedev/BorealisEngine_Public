#pragma once

struct TextureAsyncLoadData
{
public:
	Texture* m_textureToLoad = nullptr;
	const char* m_filename = nullptr;
	unsigned int m_mipLevels = 1U;
};

void CreateTextureFromFileAsync( void* args );
Image LoadFlippedImageBlocking( const char* filename );
