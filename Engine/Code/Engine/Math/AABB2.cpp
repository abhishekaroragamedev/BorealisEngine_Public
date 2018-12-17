#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/AABB2.hpp"

const AABB2 AABB2::ZERO = AABB2( Vector2( 0.0f, 0.0f ), Vector2( 0.0f, 0.0f ) );
const AABB2 AABB2::ONE = AABB2( Vector2( 1.0f, 1.0f ), Vector2( 1.0f, 1.0f ) );
const AABB2 AABB2::ZERO_TO_ONE = AABB2( Vector2( 0.0f, 0.0f ), Vector2( 1.0f, 1.0f ) );
const AABB2 AABB2::MINUS_ONE_TO_ONE = AABB2( Vector2( -1.0f , -1.0f ), Vector2( 1.0f, 1.0f ) );

AABB2::AABB2( const AABB2& copy )
{
	mins = copy.mins;
	maxs = copy.maxs;
}

AABB2::AABB2( float minX, float minY, float maxX, float maxY )
{
	mins = Vector2( minX, minY );
	maxs = Vector2( maxX, maxY );
}

AABB2::AABB2( const Vector2& initMins, const Vector2& initMaxs )
{
	mins = initMins;
	maxs = initMaxs;
}

AABB2::AABB2( const Vector2& center, float radiusX, float radiusY )		// Incircle implementation
{
	mins = Vector2( ( center.x - radiusX ),  ( center.y - radiusY ) );
	maxs = Vector2( ( center.x + radiusX ),  ( center.y + radiusY ) );
}

void AABB2::StretchToIncludePoint( float x, float y )
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
}

void AABB2::StretchToIncludePoint( const Vector2& point )
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
}

void AABB2::AddPaddingToSides( float xPaddingRadius, float yPaddingRadius )
{
	Vector2 padding = Vector2( xPaddingRadius, yPaddingRadius );

	mins -= padding;
	maxs += padding;
}

void AABB2::Translate( const Vector2& translation )
{
	mins += translation;
	maxs += translation;
}

void AABB2::Translate( float translationX, float translationY )
{
	Vector2 translation = Vector2( translationX, translationY );

	mins += translation;
	maxs += translation;
}

void AABB2::SetFromText( const char* text )
{
	TokenizedString tokenizedString = TokenizedString( text, "," );
	std::vector< std::string > tokens = tokenizedString.GetTokens();

	ASSERT_OR_DIE( tokens.size() == 4, "AABB2::SetFromText - serialized AABB2 requires 4 parameters, but the number of parameters provided is not 4. Aborting..." );

	std::string minsXPortion = tokens[ 0 ];
	std::string minsYPortion = tokens[ 1 ];
	std::string maxsXPortion = tokens[ 2 ];
	std::string maxsYPortion = tokens[ 3 ];

	mins.x = static_cast<float>( atof( minsXPortion.c_str() ) );
	mins.y = static_cast<float>( atof( minsYPortion.c_str() ) );
	maxs.x = static_cast<float>( atof( maxsXPortion.c_str() ) );
	maxs.y = static_cast<float>( atof( maxsYPortion.c_str() ) );
}


bool AABB2::IsPointInside( float x, float y ) const
{
	bool isXInside = ( x >= mins.x && x <= maxs.x );
	bool isYInside = ( y >= mins.y && y <= maxs.y );

	return ( isXInside && isYInside );
}

bool AABB2::IsPointInside( const Vector2& point ) const
{
	bool isXInside = ( point.x >= mins.x && point.x <= maxs.x );
	bool isYInside = ( point.y >= mins.y && point.y <= maxs.y );

	return ( isXInside && isYInside );
}

Vector2 AABB2::GetDimensions() const
{
	return ( maxs - mins );
}

Vector2 AABB2::GetCenter() const
{
	return ( ( maxs + mins ) / 2.0f );
}

Vector2 AABB2::GetMinXMaxY() const
{
	return Vector2( mins.x, maxs.y );
}

Vector2 AABB2::GetMaxXMinY() const
{
	return Vector2( maxs.x, mins.y );
}

void AABB2::operator=( const AABB2& copyFrom )
{
	mins = copyFrom.mins;
	maxs = copyFrom.maxs;
}

void AABB2::operator+=(const Vector2& translation)
{
	mins += translation;
	maxs += translation;
}

void AABB2::operator-=( const Vector2& antiTranslation )
{
	mins -= antiTranslation;
	maxs -= antiTranslation;
}

void AABB2::operator*=( const Vector2& upScaling )
{
	mins *= upScaling;
	maxs *= upScaling;
}

void AABB2::operator/=( const Vector2& downScaling )
{
	mins /= downScaling;
	maxs /= downScaling;
}

AABB2 AABB2::operator+( const Vector2& translation ) const
{
	AABB2 aabb2Value = AABB2( *this );
	aabb2Value.Translate( translation );
	return aabb2Value;
}

AABB2 AABB2::operator-( const Vector2& antiTranslation ) const
{
	AABB2 aabb2Value = AABB2( *this );
	aabb2Value.Translate( ( -1.0f * antiTranslation ) );
	return aabb2Value;
}

/* STANDALONE FUNCTIONS */

bool DoAABBsOverlap( const AABB2& a, const AABB2& b )
{
	bool overlapFound = false;

	int numberOfExtremities = 4;
	Vector2 aExtremities[ 4 ] = { a.mins, Vector2( a.mins.x, a.maxs.y ), Vector2( a.maxs.x, a.mins.y ), a.maxs };
	Vector2 bExtremities[ 4 ] = { b.mins, Vector2( b.mins.x, b.maxs.y ), Vector2( b.maxs.x, b.mins.y ), b.maxs };
	
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

bool DoesDiscAndAABBOverlap( const Disc2& disc, const AABB2& aabb )
{
	Vector2 displacementBetweenCenters = disc.center - aabb.GetCenter();
	Vector2 absoluteXAndYBetweenCenters = Vector2( fabsf( displacementBetweenCenters.x ), fabsf( displacementBetweenCenters.y ) );
	float aabbHalfWidth = ( aabb.maxs.x - aabb.mins.x ) / 2.0f;
	float aabbHalfHeight = ( aabb.maxs.y - aabb.mins.y ) / 2.0f;

	if	(	absoluteXAndYBetweenCenters.x >= ( disc.radius + aabbHalfWidth ) ||		// Eliminates any point not within the larger, concentric AABB with dimensions ( width + radius ), ( height + radius )
			absoluteXAndYBetweenCenters.y >= ( disc.radius + aabbHalfHeight )
		)
	{
		return false;
	}

	if	(	absoluteXAndYBetweenCenters.x <= aabbHalfWidth ||		// Covers any circle center not right outside the AABB's corners
			absoluteXAndYBetweenCenters.y <= aabbHalfHeight
		)
	{
		return true;
	}

	float distanceOfDiscCenterFromAABBCornerAsHypotenuse =	( ( absoluteXAndYBetweenCenters.x - aabbHalfWidth ) * ( absoluteXAndYBetweenCenters.x - aabbHalfWidth ) ) +
		( ( absoluteXAndYBetweenCenters.y - aabbHalfHeight ) * ( absoluteXAndYBetweenCenters.y - aabbHalfHeight ) );
	float radiusSquared = disc.radius * disc.radius;

	if ( distanceOfDiscCenterFromAABBCornerAsHypotenuse <= radiusSquared )
	{
		return true;
	}

	return false;
}

bool DoesDiscAndAABBOverlap( const Vector2& discCenter, float discRadius, const AABB2& aabb )
{
	Vector2 displacementBetweenCenters = discCenter - aabb.GetCenter();
	Vector2 absoluteXAndYBetweenCenters = Vector2( fabsf( displacementBetweenCenters.x ), fabsf( displacementBetweenCenters.y ) );
	float aabbHalfWidth = ( aabb.maxs.x - aabb.mins.x ) / 2.0f;
	float aabbHalfHeight = ( aabb.maxs.y - aabb.mins.y ) / 2.0f;

	if	(	absoluteXAndYBetweenCenters.x >= ( discRadius + aabbHalfWidth ) ||		// Eliminates any point not within the larger, concentric AABB with dimensions ( width + radius ), ( height + radius )
			absoluteXAndYBetweenCenters.y >= ( discRadius + aabbHalfHeight )
		)
	{
		return false;
	}

	if	(	absoluteXAndYBetweenCenters.x <= aabbHalfWidth ||		// Covers any circle center not right outside the AABB's corners
			absoluteXAndYBetweenCenters.y <= aabbHalfHeight
		)
	{
		return true;
	}

	float distanceOfDiscCenterFromAABBCornerAsHypotenuse =	( ( absoluteXAndYBetweenCenters.x - aabbHalfWidth ) * ( absoluteXAndYBetweenCenters.x - aabbHalfWidth ) ) +
		( ( absoluteXAndYBetweenCenters.y - aabbHalfHeight ) * ( absoluteXAndYBetweenCenters.y - aabbHalfHeight ) );
	float radiusSquared = discRadius * discRadius;

	if ( distanceOfDiscCenterFromAABBCornerAsHypotenuse <= radiusSquared )
	{
		return true;
	}

	return false;
}

const AABB2 Interpolate( const AABB2& start, const AABB2& end, float fractionTowardEnd )
{
	Vector2 interpolatedMins = Interpolate( start.mins, end.mins, fractionTowardEnd );
	Vector2 interpolatedMaxs = Interpolate( start.maxs, end.maxs, fractionTowardEnd );

	return AABB2( interpolatedMins, interpolatedMaxs );
}
