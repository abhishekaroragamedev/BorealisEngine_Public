#include "Engine/Math/Disc2.hpp"

Disc2::Disc2( const Disc2& copyFrom )
{
	center = copyFrom.center;
	radius = copyFrom.radius;
}

Disc2::Disc2( float initialX, float initialY, float initialRadius )
{
	center = Vector2( initialX, initialY );
	radius = initialRadius;
}

Disc2::Disc2( const Vector2& initialCenter, float initialRadius )
{
	center = initialCenter;
	radius = initialRadius;
}

void Disc2::StretchToIncludePoint( float x, float y )
{
	Vector2 displacement = Vector2( x, y ) - center;
	float distance = displacement.GetLength();

	if ( distance > radius )
	{
		radius = distance;
	}
}

void Disc2::StretchToIncludePoint( const Vector2& point )
{
	Vector2 displacement = point - center;
	float distance = displacement.GetLength();

	if ( distance > radius )
	{
		radius = distance;
	}
}

void Disc2::AddPaddingToRadius( float paddingRadius )
{
	radius += paddingRadius;
}

void Disc2::Translate( const Vector2& translation )
{
	center += translation;
}

void Disc2::Translate( float translationX, float translationY )
{
	center += Vector2( translationX, translationY );
}

bool Disc2::IsPointInside( float x, float y ) const
{
	return ( GetDistance( Vector2( x, y ), center ) <= radius );
}

bool Disc2::IsPointInside( const Vector2& point ) const
{
	return ( GetDistance( point, center ) <= radius );
}

void Disc2::operator+=( const Vector2& translation )
{
	center += translation;
}

void Disc2::operator-=( const Vector2& antiTranslation )
{
	center -= antiTranslation;
}

Disc2 Disc2::operator+( const Vector2& translation ) const
{
	Disc2 disc2Value = Disc2( *this );
	disc2Value.Translate( translation );
	return disc2Value;
}

Disc2 Disc2::operator-( const Vector2& antiTranslation ) const
{
	Disc2 disc2Value = Disc2( *this );
	disc2Value.Translate( ( -1.0f * antiTranslation ) );
	return disc2Value;
}

/* STANDALONE FUNCTIONS */

bool DoDiscsOverlap( const Disc2& a, const Disc2& b )
{
	return ( GetDistanceSquared( a.center, b.center ) <= ( ( a.radius + b.radius ) * ( a.radius + b.radius ) ) );
}

bool DoDiscsOverlap( const Vector2& aCenter, float aRadius,	const Vector2& bCenter, float bRadius )
{
	return ( GetDistanceSquared( aCenter, bCenter ) <= ( ( aRadius + bRadius ) * ( aRadius + bRadius ) ) );
}

const Disc2 Interpolate( const Disc2& start, const Disc2& end, float fractionTowardEnd )
{
	Vector2 interpolatedCenter = Interpolate( start.center, end.center, fractionTowardEnd );
	float interpolatedRadius = Interpolate( start.radius, end.radius, fractionTowardEnd );

	return Disc2( interpolatedCenter, interpolatedRadius );
}
