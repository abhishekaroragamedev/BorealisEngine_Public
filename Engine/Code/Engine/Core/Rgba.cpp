#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Tools/DevConsole.hpp"
#include <string>
#include <typeinfo>

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

const Rgba Rgba::NO_COLOR = Rgba( 0, 0, 0, 0 );

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

Rgba::Rgba( const Vector4& colorAsVec4 )
{
	float red = ClampFloat( colorAsVec4.x, 0.0f, 1.0f );
	r = static_cast< unsigned char >( red * RGBA_MAX_F );

	float green = ClampFloat( colorAsVec4.y, 0.0f, 1.0f );
	g = static_cast< unsigned char >( green * RGBA_MAX_F );

	float blue = ClampFloat( colorAsVec4.z, 0.0f, 1.0f );
	b = static_cast< unsigned char >( blue * RGBA_MAX_F );

	float alpha = ClampFloat( colorAsVec4.w, 0.0f, 1.0f );
	a = static_cast< unsigned char >( alpha * RGBA_MAX_F );
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

void Rgba::SetFromText( const std::string& text )
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

Vector3 Rgba::GetRGBAsFloats() const
{
	float normalizedRed = 0.0f;	float normalizedGreen = 0.0f;	float normalizedBlue = 0.0f;	float normalizedAlpha = 0.0f;
	GetAsFloats( normalizedRed, normalizedGreen, normalizedBlue, normalizedAlpha );
	UNUSED( normalizedAlpha );
	return Vector3( normalizedRed, normalizedGreen, normalizedBlue );
}

Vector4 Rgba::GetAsFloats() const
{
	float normalizedRed = 0.0f;	float normalizedGreen = 0.0f;	float normalizedBlue = 0.0f;	float normalizedAlpha = 0.0f;
	GetAsFloats( normalizedRed, normalizedGreen, normalizedBlue, normalizedAlpha );
	return Vector4( normalizedRed, normalizedGreen, normalizedBlue, normalizedAlpha );
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

Rgba Rgba::GetWithAlpha( float alphaAsFraction ) const
{
	Rgba color = *this;
	color.a = static_cast< unsigned char >( alphaAsFraction * static_cast< float >( 255 ) );
	return color;
}

Rgba Rgba::GetInverse() const
{
	unsigned char max = static_cast< unsigned char >( RGBA_MAX );
	return Rgba( ( max - r ), ( max - g ), ( max - b ), a );
}

bool Rgba::operator==( const Rgba& colorToCompare ) const
{
	return (
		r == colorToCompare.r &&
		g == colorToCompare.g &&
		b == colorToCompare.b &&
		a == colorToCompare.a
	);
}

bool Rgba::operator!=( const Rgba& colorToCompare ) const
{
	return (
		r != colorToCompare.r ||
		g != colorToCompare.g ||
		b != colorToCompare.b ||
		a != colorToCompare.a
	);
}

/* static */
Vector3 Rgba::GetRGBNormalized( const Rgba& color )
{
	return color.GetRGBAsFloats();
}

/* static */
Vector4 Rgba::GetRGBANormalized( const Rgba& color )
{
	return color.GetAsFloats();
}

/* static */
bool Rgba::AreColorsEqualExceptAlpha( const Rgba& firstColor, const Rgba& secondColor )
{
	return (
		( firstColor.r == secondColor.r ) &&
		( firstColor.g == secondColor.g ) &&
		( firstColor.b == secondColor.b )
		);
}

/* static */
bool Rgba::IsValidString( const std::string& rgbaAsString )
{
	if ( rgbaAsString.empty() )
	{
		ConsolePrintf( Rgba::RED, "ERROR: Rgba::IsValidString: Empty Rgba string provided" );
		return false;
	}

	TokenizedString tokenizedString = TokenizedString( rgbaAsString, "," );
	std::vector< std::string > tokens = tokenizedString.GetTokens();

	if ( tokens.size() < 3 || tokens.size() > 4 )
	{
		ConsolePrintf( Rgba::RED, "ERROR: Rgba::IsValidString: Invalid Rgba string provided." );
		return false;
	}

	try
	{
		int dummyCast = stoi( tokens[ 0 ] );
		dummyCast = stoi( tokens[ 1 ] );
		dummyCast = stoi( tokens[ 2 ] );

		if ( tokens.size() == 4 )
		{
			dummyCast = stoi( tokens[ 3 ] );
		}

		UNUSED( dummyCast );
	}
	catch ( std::invalid_argument& invalidArgument )
	{
		UNUSED( invalidArgument );
		ConsolePrintf( Rgba::RED, "ERROR: Rgba::IsValidString: Invalid Rgba string provided." );
		return false;
	}

	return true;
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
