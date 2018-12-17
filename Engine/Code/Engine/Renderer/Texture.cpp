//-----------------------------------------------------------------------------------------------
// Texture.cpp
//
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Threading.hpp"
#include "Engine/Renderer/Framebuffer.hpp"
#include "Engine/Renderer/GLFunctionBinding.hpp"
#include "Engine/Renderer/GLUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "ThirdParty/stb/stb_image.h"
#include <mutex>

// Must do this as a blocking call since STBI's flip state must be consistent throughout for each image loaded (multithreaded)
SpinLock g_stbiFlipSpinLock;
Image LoadFlippedImageBlocking( const char* filename )
{
	g_stbiFlipSpinLock.Enter();

	/* BEGIN CRITICAL SECTION */
	stbi_set_flip_vertically_on_load( 1 );
	Image image = Image( filename );
	stbi_set_flip_vertically_on_load( 0 );
	/* END CRITICAL SECTION */

	g_stbiFlipSpinLock.Leave();

	return image;
}

struct TextureAsyncLoadData
{
public:
	Texture* m_textureToLoad = nullptr;
	const char* m_filename = nullptr;
	unsigned int m_mipLevels = 1U;
};

void CreateTextureFromFileAsync( void* args )
{
	TextureAsyncLoadData* loadData = reinterpret_cast< TextureAsyncLoadData* >( args );

	Image image = LoadFlippedImageBlocking( loadData->m_filename );

	TextureAsyncLoadRequest* loadInfo = new TextureAsyncLoadRequest( image, loadData->m_textureToLoad, loadData->m_mipLevels );
	Renderer::GetInstance()->AddToTextureLoadQueue( loadInfo );

	delete loadData->m_filename;
	delete loadData;
}

//-----------------------------------------------------------------------------------------------
// Called only by the Renderer to create render targets.
Texture::Texture()
{

}

// Called only by the Renderer.  Use renderer->CreateOrGetTexture() to instantiate textures.
//
Texture::Texture( const std::string& filePath, unsigned int mipLevels /* = 1U */, bool loadAsync /* = true */ )
	: m_textureID( 0 )
	, m_dimensions( 0, 0 )
{
	CreateFromFile( filePath, mipLevels, loadAsync );
}

Texture::~Texture()
{
	DeleteGPUSide();
}

void Texture::DeleteGPUSide()
{
	if ( IsValid() )
	{
		switch( m_type )
		{
			case TextureType::TEXTURE_TYPE_2D				:	glBindTexture( GL_TEXTURE_2D, 0 );	break;
			case TextureType::TEXTURE_TYPE_2D_MULTISAMPLE	:	glBindTexture( GL_TEXTURE_2D_MULTISAMPLE, 0 );	break;
		}
		glDeleteTextures( 1, &m_textureID );
		m_textureID = 0U;
		m_type = TextureType::TEXTURE_TYPE_INVALID;
	}
}

void Texture::CreateFromImage( const Image& image, unsigned int mipLevels /*= 1U*/ )
{
	ASSERT_RECOVERABLE( ( mipLevels > 0U ), "Texture::CreateFromImage() - Invalid mip level passed in. Defaulting to 1." );
	if ( mipLevels < 1U )
	{
		mipLevels = 1U;
	}

	m_dimensions = image.GetDimensions();

	// Tell OpenGL that our pixel data is single-byte aligned
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

	// Ask OpenGL for an unused texName (ID number) to use for this texture
	glGenTextures( 1, (GLuint*) &m_textureID );

	// Tell OpenGL to bind (set) this as the currently active texture
	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, m_textureID );

	GLenum bufferFormat = GL_RGBA; // the format our source pixel data is in; any of: GL_RGB, GL_RGBA, GL_LUMINANCE, GL_LUMINANCE_ALPHA, ...
	GLenum internalFormat = GL_RGBA8; // the format we want the texture to be on the card; allows us to translate into a different texture format as we upload to OpenGL

	glTexStorage2D( GL_TEXTURE_2D,
		mipLevels,               // number of levels (mip-layers)
		internalFormat, // how is the memory stored on the GPU
		m_dimensions.x, m_dimensions.y ); // dimensions

	glTexSubImage2D( GL_TEXTURE_2D,
		0,             // mip layer we're copying to
		0, 0,          // offset
		m_dimensions.x, m_dimensions.y, // dimensions
		bufferFormat,      // which channels exist in the CPU buffer
		GL_UNSIGNED_BYTE,     // how are those channels stored
		&( image.GetTexels()[ 0 ] ) ); // cpu buffer to copy;

	if ( mipLevels > 1U )
	{
		glGenerateMipmap( GL_TEXTURE_2D );
	}

	m_numMipLevels = mipLevels;
	m_type = TextureType::TEXTURE_TYPE_2D;
	m_numMSAASamples = 0U;

	GL_CHECK_ERROR();
}

void Texture::CreateFromFile( const std::string& imageFilePath, unsigned int mipLevels /* = 1U */, bool loadAsync /* = true */ )
{
	if ( loadAsync )
	{
		Texture* defaultTexture = Renderer::GetInstance()->GetDefaultTexture();
		if ( defaultTexture == nullptr )
		{
			ERROR_AND_DIE( "ERROR: Texture::CreateFromFile(): Cannot load Default Texture async! Aborting..." );
		}

		const char* filename = imageFilePath.c_str();
		char* filenameCopy = new char[ strlen( filename ) + 1 ];
		memcpy( filenameCopy, filename, strlen( filename ) );
		filenameCopy[ strlen( filename ) ] = '\0';

		TextureAsyncLoadData* loadData = new TextureAsyncLoadData();
		loadData->m_textureToLoad = this;
		loadData->m_filename = filenameCopy;
		loadData->m_mipLevels = mipLevels;

		ThreadCreateAndDetach( CreateTextureFromFileAsync, loadData );
		m_textureID = defaultTexture->m_textureID;
		m_dimensions = defaultTexture->m_dimensions;
		m_numMipLevels = defaultTexture->m_numMipLevels;
		m_numMSAASamples = 0U;
		m_type = defaultTexture->m_type;
		m_format = defaultTexture->m_format;
	}
	else
	{
		Image imageFromFile = LoadFlippedImageBlocking( imageFilePath.c_str() );
		CreateFromImage( imageFromFile, mipLevels );
	}
}

bool Texture::CreateRenderTarget( unsigned int width, unsigned int height, TextureFormat format )
{
	m_dimensions.x = static_cast< int >( width );  
	m_dimensions.y = static_cast< int >( height ); 

	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
	glGenTextures( 1, ( GLuint* ) &m_textureID ); 
	
	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, m_textureID );

	GLenum internalFormat;
	GLenum channels;
	GLenum pixelLayout;
	TextureFormatToGLFormats( &internalFormat, &channels, &pixelLayout, format );

	glTexStorage2D( GL_TEXTURE_2D,
		1,               // number of levels (mip-layers)
		internalFormat, // how is the memory stored on the GPU
		m_dimensions.x, m_dimensions.y ); // dimensions

	m_format = format; // I save the format with the texture
					// for sanity checking.

					// great, success

	m_type = TextureType::TEXTURE_TYPE_2D;
	m_numMSAASamples = 0U;

	return GLSucceeded();
}

bool Texture::CreateMultiSampleTarget( unsigned int width, unsigned int height, TextureFormat format, unsigned int numSamples )
{
	glEnable( GL_MULTISAMPLE );

	m_dimensions.x = static_cast< int >( width );
	m_dimensions.y = static_cast< int >( height );

	glGenTextures( 1, ( GLuint* ) &m_textureID );

	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D_MULTISAMPLE, m_textureID );

	GLenum internalFormat;
	GLenum channels;
	GLenum pixelLayout;
	TextureFormatToGLFormats( &internalFormat, &channels, &pixelLayout, format );
	GL_CHECK_ERROR();
	glTexImage2DMultisample( GL_TEXTURE_2D_MULTISAMPLE, numSamples, internalFormat, width, height, false );
	GL_CHECK_ERROR();
	m_format = format;
	m_type = TextureType::TEXTURE_TYPE_2D_MULTISAMPLE;
	m_numMSAASamples = numSamples;

	return GLSucceeded();
}

IntVector2 Texture::GetDimensions() const
{
	return m_dimensions;
}

Vector2 Texture::GetDimensionsF() const
{
	return ConvertIntVector2ToVector2( m_dimensions );
}

TextureType Texture::GetType() const
{
	return m_type;
}

bool Texture::UsesMipMaps() const
{
	return ( m_numMipLevels > 1U );
}

unsigned int Texture::GetNumMipLevels() const
{
	return m_numMipLevels;
}

unsigned int Texture::GetNumMSAASamples() const
{
	return m_numMSAASamples;
}

void TextureFormatToGLFormats( GLenum* internalFormat, GLenum* channels, GLenum* pixelLayout, TextureFormat format )
{
	switch ( format )
	{
		case TextureFormat::TEXTURE_FORMAT_RGBA8	:
		{
			*internalFormat = GL_RGBA8;
			*channels = GL_RGBA;
			*pixelLayout = GL_UNSIGNED_BYTE;
			break;
		}
		case TextureFormat::TEXTURE_FORMAT_D24S8	:
		{
			*internalFormat = GL_DEPTH24_STENCIL8;
			*channels = GL_DEPTH_STENCIL;
			*pixelLayout = GL_UNSIGNED_INT_24_8;
			break;
		}
		default	:
		{
			break;
		}
	}
}