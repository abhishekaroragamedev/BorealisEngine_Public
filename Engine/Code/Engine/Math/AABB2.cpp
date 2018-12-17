#include "Engine/Math/AABB2.hpp"

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
