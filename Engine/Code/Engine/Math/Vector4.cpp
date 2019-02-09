#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Vector4.hpp"
#include "Engine/Tools/DevConsole.hpp"

const Vector4 Vector4::ZERO_POINT = Vector4( 0.0f, 0.0f, 0.0f, 1.0f );
const Vector4 Vector4::ZERO_DISPLACEMENT = Vector4( 0.0f, 0.0f, 0.0f, 0.0f );
const Vector4 Vector4::ONE_POINT = Vector4( 1.0f, 1.0f, 1.0f, 1.0f );
const Vector4 Vector4::ONE_DISPLACEMENT = Vector4( 1.0f, 1.0f, 1.0f, 0.0f );

Vector4::Vector4( float initialX, float initialY, float initialZ, float initialW/* = 1.0f*/ )
{
	x = initialX;
	y = initialY;
	z = initialZ;
	w = initialW;
}

Vector4::Vector4( const Vector3& vector3, float initialW/* = 1.0f*/ )
{
	x = vector3.x;
	y = vector3.y;
	z = vector3.z;
	w = initialW;
}

Vector4::Vector4( const Vector4& copyFrom )
{
	x = copyFrom.x;
	y = copyFrom.y;
	z = copyFrom.z;
	w = copyFrom.w;
}

Vector3 Vector4::GetVector3() const
{
	return Vector3( x, y, z );
}

const Vector4 Vector4::operator+( const Vector4& vecToAdd ) const
{
	return Vector4( ( x + vecToAdd.x ), ( y + vecToAdd.y ), ( z + vecToAdd.z ), ( w + vecToAdd.w ) );
}

const Vector4 Vector4::operator-( const Vector4& vecToSubtract ) const
{
	return Vector4( ( x - vecToSubtract.x ), ( y - vecToSubtract.y ), ( z - vecToSubtract.z ), ( w - vecToSubtract.w ) );
}

const Vector4 Vector4::operator*( float uniformScale ) const
{
	return Vector4( ( x * uniformScale ), ( y * uniformScale ), ( z * uniformScale ), ( w* uniformScale ) );
}

const Vector4 Vector4::operator/( float inverseScale ) const
{

	if ( IsFloatEqualTo( inverseScale, 0.0f ) )
	{
		return Vector4( x, y, z );
	}

	return Vector4( ( x / inverseScale ), ( y / inverseScale ), ( z / inverseScale ), ( w / inverseScale ) );
}

void Vector4::operator+=( const Vector4& vecToAdd )
{
	x += vecToAdd.x;
	y += vecToAdd.y;
	z += vecToAdd.z;
	w += vecToAdd.w;
}

void Vector4::operator-=( const Vector4& vecToSubtract )
{
	x -= vecToSubtract.x;
	y -= vecToSubtract.y;
	z -= vecToSubtract.z;
	w -= vecToSubtract.w;
}

void Vector4::operator*=( const float uniformScale )
{
	x *= uniformScale;
	y *= uniformScale;
	z *= uniformScale;
	w *= uniformScale;
}

void Vector4::operator/=( const float uniformDivisor )
{
	if ( !IsFloatEqualTo( uniformDivisor, 0.0f ) )
	{
		x /= uniformDivisor;
		y /= uniformDivisor;
		z /= uniformDivisor;
		w /= uniformDivisor;
	}
}

void Vector4::operator=( const Vector4& copyFrom )
{
	x = copyFrom.x;
	y = copyFrom.y;
	z = copyFrom.z;
	w = copyFrom.w;
}

bool Vector4::operator==( const Vector4& compare ) const
{
	return ( IsFloatEqualTo( x, compare.x ) && IsFloatEqualTo( y, compare.y ) && IsFloatEqualTo( z, compare.z ) && IsFloatEqualTo( w, compare.w ) );
}

bool Vector4::operator!=( const Vector4& compare ) const
{
	return ( !IsFloatEqualTo( x, compare.x ) || !IsFloatEqualTo( y, compare.y ) || !IsFloatEqualTo( z, compare.z ) || !IsFloatEqualTo( w, compare.w ) );
}

bool Vector4::operator>( const Vector4& compare ) const
{
	return ( ( x > compare.x ) && ( y > compare.y ) && ( z > compare.z ) && ( z > compare.z ) );
}

bool Vector4::operator>=( const Vector4& compare ) const
{
	return ( ( *this > compare ) || ( *this == compare ) );
}

bool Vector4::operator<( const Vector4& compare ) const
{
	return ( ( x < compare.x ) && ( y < compare.y ) && ( z < compare.z ) && ( w < compare.w ) );
}

bool Vector4::operator<=( const Vector4& compare ) const
{
	return ( ( *this < compare ) || ( *this == compare ) );
}

void Vector4::SetFromText( const char* text )
{
	SetFromText( std::string( text ) );
}

void Vector4::SetFromText( const std::string& text )
{
	TokenizedString tokenizedString = TokenizedString( text, "," );
	std::vector< std::string > tokens = tokenizedString.GetTokens();

	x = std::stof( tokens[ 0 ].c_str() );
	y = std::stof( tokens[ 1 ].c_str() );
	z = std::stof( tokens[ 2 ].c_str() );
	w = std::stof( tokens[ 3 ].c_str() );
}

/* static */
bool Vector4::IsValidString( const std::string& vec4AsString )
{
	if ( vec4AsString.empty() )
	{
		ConsolePrintf( Rgba::RED, "ERROR: Vector4::IsValidString: Empty Vector4 string provided" );
		return false;
	}

	TokenizedString tokenizedString = TokenizedString( vec4AsString, "," );
	std::vector< std::string > tokens = tokenizedString.GetTokens();

	if ( tokens.size() != 4 )
	{
		ConsolePrintf( Rgba::RED, "ERROR: Vector4::IsValidString: Invalid Vector4 string provided." );
		return false;
	}

	try
	{
		float dummyCast = stof( tokens[ 0 ] );
		dummyCast = stof( tokens[ 1 ] );
		dummyCast = stof( tokens[ 2 ] );
		dummyCast = stof( tokens[ 3 ] );

		UNUSED( dummyCast );
	}
	catch ( std::invalid_argument& invalidArgument )
	{
		UNUSED( invalidArgument );
		ConsolePrintf( Rgba::RED, "ERROR: Vector4::IsValidString: Invalid Vector4 string provided." );
		return false;
	}

	return true;
}

float DotProduct( const Vector4& a, const Vector4& b )
{
	return ( ( a.x * b.x ) + ( a.y * b.y ) + ( a.z * b.z ) + ( a.w * b.w ) );
}
