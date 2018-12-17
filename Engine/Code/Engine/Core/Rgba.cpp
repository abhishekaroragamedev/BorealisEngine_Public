#include "Engine/Core/Rgba.hpp"

Rgba::Rgba()
{

}

Rgba::Rgba( unsigned char redByte, unsigned char greenByte, unsigned char blueByte, unsigned char alphaByte = RGBA_MAX )
{
	r = redByte;
	g = greenByte;
	b = blueByte;
	a = alphaByte;
}

void Rgba::SetAsBytes( unsigned char redByte, unsigned char greenByte, unsigned char blueByte, unsigned char alphaByte = RGBA_MAX )
{
	r = redByte;
	g = greenByte;
	b = blueByte;
	a = alphaByte;
}

void Rgba::SetAsFloats( float normalizedRed, float normalizedGreen, float normalizedBlue, float normalizedAlpha = 1.0f )
{
	r = static_cast<unsigned char> ( normalizedRed ) * 255;
	g = static_cast<unsigned char> ( normalizedGreen ) * 255;
	b = static_cast<unsigned char> ( normalizedBlue ) * 255;
	a = static_cast<unsigned char> ( normalizedAlpha ) * 255;
}

void Rgba::GetAsFloats( float& out_normalizedRed, float& out_normalizedGreen, float& out_normalizedBlue, float& out_normalizedAlpha ) const
{
	out_normalizedRed = ( static_cast<float> ( r ) / static_cast<float> ( RGBA_MAX ) );
	out_normalizedGreen = ( static_cast<float> ( g ) / static_cast<float> ( RGBA_MAX ) );
	out_normalizedBlue = ( static_cast<float> ( b ) / static_cast<float> ( RGBA_MAX ) );
	out_normalizedAlpha = ( static_cast<float> ( a ) / static_cast<float> ( RGBA_MAX ) );
}

void Rgba::ScaleRGB( float rgbScale )
{
	r *= static_cast<unsigned char> ( rgbScale );
	g *= static_cast<unsigned char> ( rgbScale );
	b *= static_cast<unsigned char> ( rgbScale );
}

void Rgba::ScaleAlpha( float alphaScale )
{
	a *= static_cast<unsigned char> ( alphaScale );
}
