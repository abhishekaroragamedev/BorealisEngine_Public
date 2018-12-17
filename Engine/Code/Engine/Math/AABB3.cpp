#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/AABB3.hpp"

const AABB3 AABB3::ZERO = AABB3( Vector3::ZERO, Vector3::ZERO );
const AABB3 AABB3::ONE = AABB3( Vector3::ONE, Vector3::ONE );
const AABB3 AABB3::ZERO_TO_ONE = AABB3( Vector3::ZERO, Vector3::ONE );
const AABB3 AABB3::MINUS_ONE_TO_ONE = AABB3( ( -1.0f * Vector3::ONE ), Vector3::ONE );

AABB3::AABB3( const AABB3& copy )
{
	mins = copy.mins;
	maxs = copy.maxs;
}

AABB3::AABB3( float minX, float minY, float minZ, float maxX, float maxY, float maxZ )
{
	mins = Vector3( minX, minY, minZ );
	maxs = Vector3( maxX, maxY, maxZ );
}

AABB3::AABB3( const Vector3& initMins, const Vector3& initMaxs )
{
	mins = initMins;
	maxs = initMaxs;
}

AABB3::AABB3( const Vector3& center, float radiusX, float radiusY, float radiusZ )		// Incircle implementation
{
	mins = Vector3( ( center.x - radiusX ),  ( center.y - radiusY ), ( center.z - radiusZ ) );
	maxs = Vector3( ( center.x + radiusX ),  ( center.y + radiusY ), ( center.z + radiusZ ) );
}

void AABB3::StretchToIncludePoint( float x, float y, float z )
{
	if ( x < mins.x )
	{
		mins.x = x;
	}
	else if ( x > maxs.x )
	{
		maxs.x = x;
	}

	if ( y < mins.y )
	{
		mins.y = y;
	}
	else if ( y > maxs.y )
	{
		maxs.y = y;
	}

	if ( z < mins.z )
	{
		mins.z = z;
	}
	else if ( z > maxs.z )
	{
		maxs.z = z;
	}
}

void AABB3::StretchToIncludePoint( const Vector3& point )
{
	if ( point.x < mins.x )
	{
		mins.x = point.x;
	}
	else if ( point.x > maxs.x )
	{
		maxs.x = point.x;
	}

	if ( point.y < mins.y )
	{
		mins.y = point.y;
	}
	else if ( point.y > maxs.y )
	{
		maxs.y = point.y;
	}

	if ( point.z < mins.z )
	{
		mins.z = point.z;
	}
	else if ( point.z > maxs.z )
	{
		maxs.z = point.z;
	}
}

void AABB3::AddPaddingToSides( float xPaddingRadius, float yPaddingRadius, float zPaddingRadius )
{
	Vector3 padding = Vector3( xPaddingRadius, yPaddingRadius, zPaddingRadius );

	mins -= padding;
	maxs += padding;
}

void AABB3::Translate( const Vector3& translation )
{
	mins += translation;
	maxs += translation;
}

void AABB3::Translate( float translationX, float translationY, float translationZ )
{
	Vector3 translation = Vector3( translationX, translationY, translationZ );

	mins += translation;
	maxs += translation;
}

void AABB3::SetFromText( const char* text )
{
	TokenizedString tokenizedString = TokenizedString( text, "," );
	std::vector< std::string > tokens = tokenizedString.GetTokens();

	ASSERT_OR_DIE( tokens.size() == 6, "AABB3::SetFromText - serialized AABB3 requires 6 parameters, but the number of parameters provided is not 6. Aborting..." );

	std::string minsXPortion = tokens[ 0 ];
	std::string minsYPortion = tokens[ 1 ];
	std::string minsZPortion = tokens[ 2 ];
	std::string maxsXPortion = tokens[ 3 ];
	std::string maxsYPortion = tokens[ 4 ];
	std::string maxsZPortion = tokens[ 5 ];

	mins.x = static_cast<float>( atof( minsXPortion.c_str() ) );
	mins.y = static_cast<float>( atof( minsYPortion.c_str() ) );
	mins.z = static_cast<float>( atof( minsZPortion.c_str() ) );
	maxs.x = static_cast<float>( atof( maxsXPortion.c_str() ) );
	maxs.y = static_cast<float>( atof( maxsYPortion.c_str() ) );
	maxs.z = static_cast<float>( atof( maxsZPortion.c_str() ) );
}


bool AABB3::IsPointInside( float x, float y, float z ) const
{
	bool isXInside = ( x >= mins.x && x <= maxs.x );
	bool isYInside = ( y >= mins.y && y <= maxs.y );
	bool isZInside = ( z >= mins.z && z <= maxs.z );

	return ( isXInside && isYInside && isZInside );
}

bool AABB3::IsPointInside( const Vector3& point ) const
{
	bool isXInside = ( point.x >= mins.x && point.x <= maxs.x );
	bool isYInside = ( point.y >= mins.y && point.y <= maxs.y );
	bool isZInside = ( point.z >= mins.z && point.z <= maxs.z );

	return ( isXInside && isYInside && isZInside );
}

bool AABB3::IsValid() const
{
	return ( maxs > mins );
}

Vector3 AABB3::GetDimensions() const
{
	return ( maxs - mins );
}

Vector3 AABB3::GetCenter() const
{
	return ( ( maxs + mins ) / 2.0f );
}

Vector3 AABB3::GetMinXMaxYMaxZ() const
{
	return Vector3( mins.x, maxs.y, maxs.z );
}

Vector3 AABB3::GetMaxXMinYMaxZ() const
{
	return Vector3( maxs.x, mins.y, maxs.z );
}

Vector3 AABB3::GetMaxXMaxYMinZ() const
{
	return Vector3( maxs.x, maxs.y, mins.z );
}

Vector3 AABB3::GetMinXMaxYMinZ() const
{
	return Vector3( mins.x, maxs.y, mins.z );
}

Vector3 AABB3::GetMaxXMinYMinZ() const
{
	return Vector3( maxs.x, mins.y, mins.z );
}

Vector3 AABB3::GetMinXMinYMaxZ() const
{
	return Vector3( mins.x, mins.y, maxs.z );
}

void AABB3::operator=( const AABB3& assigned )
{
	mins = assigned.mins;
	maxs = assigned.maxs;
}

void AABB3::operator+=(const Vector3& translation)
{
	mins += translation;
	maxs += translation;
}

void AABB3::operator-=( const Vector3& antiTranslation )
{
	mins -= antiTranslation;
	maxs -= antiTranslation;
}

void AABB3::operator*=( const Vector3& upScaling )
{
	mins *= upScaling;
	maxs *= upScaling;
}

void AABB3::operator/=( const Vector3& downScaling )
{
	mins /= downScaling;
	maxs /= downScaling;
}

AABB3 AABB3::operator+( const Vector3& translation ) const
{
	AABB3 AABB3Value = AABB3( *this );
	AABB3Value.Translate( translation );
	return AABB3Value;
}

AABB3 AABB3::operator-( const Vector3& antiTranslation ) const
{
	AABB3 AABB3Value = AABB3( *this );
	AABB3Value.Translate( ( -1.0f * antiTranslation ) );
	return AABB3Value;
}

/* STANDALONE FUNCTIONS */

bool DoAABBsOverlap( const AABB3& a, const AABB3& b )
{
	bool overlapFound = false;

	int numberOfExtremities = 8;
	Vector3 aExtremities[ 8 ] = { a.mins, a.GetMaxXMinYMinZ(), a.GetMinXMaxYMaxZ(), a.GetMinXMinYMaxZ(), a.GetMaxXMaxYMinZ(), a.GetMaxXMinYMaxZ(), a.GetMinXMaxYMinZ(), a.maxs };
	Vector3 bExtremities[ 8 ] = { b.mins, b.GetMaxXMinYMinZ(), b.GetMinXMaxYMaxZ(), b.GetMinXMinYMaxZ(), b.GetMaxXMaxYMinZ(), b.GetMaxXMinYMaxZ(), b.GetMinXMaxYMinZ(), b.maxs };

	for ( int extremityIndex = 0; extremityIndex < numberOfExtremities; extremityIndex++ )
	{
		overlapFound = false;

		if( a.IsPointInside( bExtremities[ extremityIndex ] ) )		// See if a contains any of b's endpoints
		{
			overlapFound = true;
			break;
		}

		if( b.IsPointInside( aExtremities[ extremityIndex ] ) )		// See if b contains any of a's endpoints
		{
			overlapFound = true;
			break;
		}
	}

	return overlapFound;
}

const AABB3 Interpolate( const AABB3& start, const AABB3& end, float fractionTowardEnd )
{
	Vector3 interpolatedMins = Interpolate( start.mins, end.mins, fractionTowardEnd );
	Vector3 interpolatedMaxs = Interpolate( start.maxs, end.maxs, fractionTowardEnd );

	return AABB3( interpolatedMins, interpolatedMaxs );
}
