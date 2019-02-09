#include "Engine/Core/Image.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "ThirdParty/stb/stb_image.h"

Image::Image()
{

}

Image::Image( const std::string& imageFilePath )
{
	int numComponents = 0;
	int numComponentsRequested = 0;

	unsigned char* imageData = stbi_load( imageFilePath.c_str(), &m_dimensions.x, &m_dimensions.y, &numComponents, numComponentsRequested );
	SetTexelsFromImageData( imageData, numComponents );
	stbi_image_free( imageData );
}

Image::Image( const Image& copy )
	:	m_dimensions( copy.m_dimensions )
{
	for ( Rgba texel : copy.m_texels )
	{
		m_texels.push_back( texel );
	}
}

IntVector2 Image::GetDimensions() const
{
	return m_dimensions;
}

std::vector< Rgba > Image::GetTexels() const
{
	return m_texels;
}

Rgba Image::GetTexel( int x, int y ) const
{
	int texelIndex1D = ( m_dimensions.x * y ) + x;
	return m_texels[ texelIndex1D ];
}

const Rgba* Image::GetTexelReference( int x, int y ) const
{
	int texelIndex1D = ( m_dimensions.x * y ) + x;
	return &m_texels[ texelIndex1D ];
}

void Image::SetTexel( int x, int y, const Rgba& color )
{
	int texelIndex1D = ( m_dimensions.x * y ) + x;
	m_texels[ texelIndex1D ] = color;
}

void Image::SetDimensions( int x, int y )
{
	size_t targetDimensions = static_cast< size_t >( x ) * static_cast< size_t >( y );
	while ( m_texels.size() > targetDimensions )
	{
		m_texels.pop_back();
	}

	Rgba colorStamp = Rgba();
	while ( m_texels.size() < targetDimensions )
	{
		m_texels.push_back( colorStamp );
	}

	m_dimensions.x = x;
	m_dimensions.y = y;
}

void Image::SetTexelsFromImageData( unsigned char* imageData, int numComponents )
{
	ASSERT_OR_DIE( ( numComponents == 3 || numComponents == 4 ), "Image::SetTexelsFromImageData - Texel color format neither identified as RGB or RGBA." );

	unsigned char* currentImagePointer = imageData;
	unsigned char numComponentsAsUnsignedChar = static_cast<unsigned char>( numComponents );

	for ( int currentTexelY = 0; currentTexelY < m_dimensions.y; currentTexelY++ )
	{
		for ( int currentTexelX = 0; currentTexelX < m_dimensions.x; currentTexelX++ )
		{
			Rgba currentTexelColor = Rgba( *currentImagePointer, *( currentImagePointer + 1 ), *( currentImagePointer + 2 ), RGBA_MAX );
			if ( numComponents == 4 )
			{
				currentTexelColor.a = *( currentImagePointer + 3 );
			}

			currentImagePointer += numComponentsAsUnsignedChar;		// Go to the next RGB/RGBA texel

			m_texels.push_back( currentTexelColor );
		}
	}
}
