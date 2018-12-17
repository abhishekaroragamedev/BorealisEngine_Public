#include "Engine/Core/Rgba.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include <string>

const Rgba Rgba::WHITE = Rgba( RGBA_MAX, RGBA_MAX, RGBA_MAX, RGBA_MAX );

const Rgba Rgba::BLACK = Rgba( 0, 0, 0, RGBA_MAX );

const Rgba Rgba::RED = Rgba( RGBA_MAX, 0, 0, RGBA_MAX );

const Rgba Rgba::GREEN = Rgba( 0, RGBA_MAX, 0, RGBA_MAX );

const Rgba Rgba::BLUE = Rgba( 0, 0, RGBA_MAX, RGBA_MAX );

const Rgba Rgba::MAGENTA = Rgba( RGBA_MAX, 0, RGBA_MAX, RGBA_MAX );

const Rgba Rgba::YELLOW = Rgba( RGBA_MAX, RGBA_MAX, 0, RGBA_MAX );

const Rgba Rgba::CYAN = Rgba( 0, RGBA_MAX, RGBA_MAX, RGBA_MAX );

const Rgba Rgba::ORANGE = Rgba( RGBA_MAX, 165, 0, RGBA_MAX );

const Rgba Rgba::PURPLE = Rgba( 138, 43, 226, RGBA_MAX );

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

void Rgba::SetFromText( const char* text )
{
	TokenizedString tokenizedString = TokenizedString( text, "," );
	std::vector< std::string > tokens = tokenizedString.GetTokens();

	r = static_cast<unsigned char>( atoi( tokens[ 0 ].c_str() ) );
	g = static_cast<unsigned char>( atoi( tokens[ 1 ].c_str() ) );
	b = static_cast<unsigned char>( atoi( tokens[ 2 ].c_str() ) );
	a = 255;

	if ( tokens.size() == 4 )
	{
		a = static_cast<unsigned char>( atoi( tokens[ 3 ].c_str() ) );
	}
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

bool Rgba::AreColorsEqualExceptAlpha( const Rgba& firstColor, const Rgba& secondColor )
{
	return (
		( firstColor.r == secondColor.r ) &&
		( firstColor.g == secondColor.g ) &&
		( firstColor.b == secondColor.b )
	);
}

Rgba Rgba::GetWithAlpha( float alphaAsFraction ) const
{
	Rgba color = *this;
	color.a = static_cast< unsigned char >( alphaAsFraction * static_cast< float >( a ) );
	return color;
}

const Rgba Interpolate( const Rgba& start, const Rgba& end, float fractionTowardEnd )
{
	return Rgba(
		Interpolate( start.r, end.r, fractionTowardEnd ),
		Interpolate( start.g, end.g, fractionTowardEnd ),
		Interpolate( start.b, end.b, fractionTowardEnd ),
		Interpolate( start.a, end.a, fractionTowardEnd )
	);
}
