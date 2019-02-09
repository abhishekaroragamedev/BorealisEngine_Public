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
	:	r( static_cast< float >( redByte ) / RGBA_MAX_F ),
		g( static_cast< float >( greenByte ) / RGBA_MAX_F ),
		b( static_cast< float >( blueByte ) / RGBA_MAX_F ),
		a( static_cast< float >( alphaByte ) / RGBA_MAX_F )
{

}

Rgba::Rgba( const Vector4& colorAsVec4 )
{
	r = colorAsVec4.x;
	g = colorAsVec4.y;
	b = colorAsVec4.z;
	a = colorAsVec4.w;
}

Rgba::Rgba( const Rgba& copy )
	:	r( copy.r ),
		g( copy.g ),
		b( copy.b ),
		a( copy.a )
{

}

void Rgba::SetAsBytes( unsigned char redByte, unsigned char greenByte, unsigned char blueByte, unsigned char alphaByte = RGBA_MAX )
{
	r = static_cast< float >( redByte ) / RGBA_MAX_F;
	g = static_cast< float >( greenByte ) / RGBA_MAX_F;
	b = static_cast< float >( blueByte ) / RGBA_MAX_F;
	a = static_cast< float >( alphaByte ) / RGBA_MAX_F;
}

void Rgba::SetAsFloats( float normalizedRed, float normalizedGreen, float normalizedBlue, float normalizedAlpha = 1.0f )
{
	r =  normalizedRed;
	g =  normalizedGreen;
	b =  normalizedBlue;
	a =  normalizedAlpha;
}

void Rgba::SetFromText( const char* text )
{
	TokenizedString tokenizedString = TokenizedString( text, "," );
	std::vector< std::string > tokens = tokenizedString.GetTokens();

	unsigned char redByte = static_cast<unsigned char>( atoi( tokens[ 0 ].c_str() ) );
	unsigned char greenByte = static_cast<unsigned char>( atoi( tokens[ 1 ].c_str() ) );
	unsigned char blueByte = static_cast<unsigned char>( atoi( tokens[ 2 ].c_str() ) );
	unsigned char alphaByte = 255;

	if ( tokens.size() == 4 )
	{
		alphaByte = static_cast<unsigned char>( atoi( tokens[ 3 ].c_str() ) );
	}

	SetAsBytes( redByte, greenByte, blueByte, alphaByte );
}

void Rgba::SetFromText( const std::string& text )
{
	TokenizedString tokenizedString = TokenizedString( text, "," );
	std::vector< std::string > tokens = tokenizedString.GetTokens();

	unsigned char redByte = static_cast<unsigned char>( atoi( tokens[ 0 ].c_str() ) );
	unsigned char greenByte = static_cast<unsigned char>( atoi( tokens[ 1 ].c_str() ) );
	unsigned char blueByte = static_cast<unsigned char>( atoi( tokens[ 2 ].c_str() ) );
	unsigned char alphaByte = 255;

	if ( tokens.size() == 4 )
	{
		alphaByte = static_cast<unsigned char>( atoi( tokens[ 3 ].c_str() ) );
	}

	SetAsBytes( redByte, greenByte, blueByte, alphaByte );
}

void Rgba::GetAsFloats( float& out_normalizedRed, float& out_normalizedGreen, float& out_normalizedBlue, float& out_normalizedAlpha ) const
{
	out_normalizedRed = r;
	out_normalizedGreen = g;
	out_normalizedBlue = b;
	out_normalizedAlpha = a;
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
	r *= rgbScale;
	g *= rgbScale;
	b *= rgbScale;
}

void Rgba::ScaleAlpha( float alphaScale )
{
	a *= alphaScale;
}

Rgba Rgba::GetWithAlpha( float alphaAsFraction ) const
{
	Rgba color( *this );
	color.a = alphaAsFraction;
	return color;
}

Rgba Rgba::GetInverse() const
{
	Rgba inverse( *this );
	inverse.r = 1.0f - inverse.r;
	inverse.g = 1.0f - inverse.g;
	inverse.b = 1.0f - inverse.b;
	return inverse;
}

void Rgba::operator=( const Rgba& assign )
{
	r = assign.r;
	g = assign.g;
	b = assign.b;
	a = assign.a;
}

bool Rgba::operator==( const Rgba& colorToCompare ) const
{
	return (
		IsFloatEqualTo( r, colorToCompare.r ) &&
		IsFloatEqualTo( g, colorToCompare.g ) &&
		IsFloatEqualTo( b, colorToCompare.b ) &&
		IsFloatEqualTo( a, colorToCompare.a )
	);
}

bool Rgba::operator!=( const Rgba& colorToCompare ) const
{
	return (
		!IsFloatEqualTo( r, colorToCompare.r ) ||
		!IsFloatEqualTo( g, colorToCompare.g ) ||
		!IsFloatEqualTo( b, colorToCompare.b ) ||
		!IsFloatEqualTo( a, colorToCompare.a )
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
		IsFloatEqualTo( firstColor.r, secondColor.r ) &&
		IsFloatEqualTo( firstColor.g, secondColor.g ) &&
		IsFloatEqualTo( firstColor.b, secondColor.b )
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
		static_cast< unsigned char >( Interpolate( start.r, end.r, fractionTowardEnd ) * RGBA_MAX_F ),
		static_cast< unsigned char >( Interpolate( start.g, end.g, fractionTowardEnd ) * RGBA_MAX_F ),
		static_cast< unsigned char >( Interpolate( start.b, end.b, fractionTowardEnd ) * RGBA_MAX_F ),
		static_cast< unsigned char >( Interpolate( start.a, end.a, fractionTowardEnd ) * RGBA_MAX_F )
	);
}
