#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/GLFunctionBinding.hpp"
#include "Engine/Renderer/GLUtils.hpp"
#include "Engine/Renderer/Texture3D.hpp"
#include "Engine/Renderer/TextureAsync.hpp"

Texture3D::Texture3D()
{

}

Texture3D::~Texture3D()
{
	DeleteGPUSide();
}

void Texture3D::DeleteGPUSide()
{
	if ( IsValid() )
	{
		glDeleteTextures( 1, &m_textureID );
		m_textureID = 0U;
	}
}

void Texture3D::CreateFromImages( unsigned int imageCount, const Image* images, unsigned int mipLevels /* = 1U */, TextureFormat format /* = TextureFormat::TEXTURE_FORMAT_RGBA8 */ )
{
	ASSERT_RECOVERABLE( ( mipLevels > 0U ), "Texture3D::CreateFromImage() - Invalid mip level passed in. Defaulting to 1." );
	if ( mipLevels < 1U )
	{
		mipLevels = 1U;
	}

	m_dimensions = IntVector3( images[ 0 ].GetDimensions(), static_cast< int >( imageCount ) );

	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
	glGenTextures( 1, reinterpret_cast< GLuint* >( &m_textureID ) );

	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_3D, m_textureID );

	GLenum bufferFormat;
	GLenum internalFormat;
	GLenum pixelLayout;

	TextureFormatToGLFormats( &internalFormat, &bufferFormat, &pixelLayout, format );

	glTexStorage3D(
		GL_TEXTURE_3D,
		mipLevels,
		internalFormat,
		m_dimensions.x, m_dimensions.y, m_dimensions.z
	);
	GL_CHECK_ERROR();

	for ( unsigned int imageIndex = 0; imageIndex < imageCount; imageIndex++ )
	{
		glTexSubImage3D(
			GL_TEXTURE_3D,
			0,
			0, 0, static_cast< GLint >( imageIndex ),
			m_dimensions.x, m_dimensions.y, 1,
			bufferFormat,
			GL_FLOAT,
			&( images[ imageIndex ].GetTexels()[ 0 ] )
		);
	}

	if ( mipLevels > 1U )
	{
		glGenerateMipmap( GL_TEXTURE_3D );
	}

	m_numMipLevels = mipLevels;
	m_format = format;

	GL_CHECK_ERROR();
}

void Texture3D::CreateFromFiles( unsigned int numFiles, const std::string* fileNames, unsigned int mipLevels /* = 1U */, TextureFormat format /* = TextureFormat::TEXTURE_FORMAT_RGBA8 */ )
{
	Image* images = new Image[ numFiles ];
	for ( unsigned int fileIndex = 0U; fileIndex < numFiles; fileIndex++ )
	{
		images[ fileIndex ] = LoadFlippedImageBlocking( fileNames[ fileIndex ].c_str() );
	}
	CreateFromImages( numFiles, images, mipLevels, format );

	delete[] images;
}

void Texture3D::RegenerateMipMaps()
{
	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_3D, m_textureID );
	glGenerateMipmap( GL_TEXTURE_3D );
	GL_CHECK_ERROR();
}

IntVector3 Texture3D::GetDimensions() const
{
	return m_dimensions;
}

Vector3 Texture3D::GetDimensionsF() const
{
	return ConvertIntVector3ToVector3( m_dimensions );
}

bool Texture3D::UsesMipMaps() const
{
	return ( m_numMipLevels > 1U );
}

unsigned int Texture3D::GetNumMipLevels() const
{
	return m_numMipLevels;
}
