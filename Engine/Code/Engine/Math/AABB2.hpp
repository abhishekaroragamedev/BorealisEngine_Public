#pragma once

#include "Engine/Math/Disc2.hpp"

class AABB2
{

public:
	AABB2() {}
	~AABB2() {}
	AABB2( const AABB2& copy );
	
	explicit AABB2( float minX, float minY, float maxX, float maxY );
	explicit AABB2( const Vector2& mins, const Vector2& maxs );
	explicit AABB2( const Vector2& center, float radiusX, float radiusY );

public:
	void StretchToIncludePoint( float x, float y );
	void StretchToIncludePoint( const Vector2& point );
	void AddPaddingToSides( float xPaddingRadius, float yPaddingRadius );
	void Translate( const Vector2& translation );
	void Translate( float translationX, float translationY );
	void SetFromText( const char* text );

public:
	bool IsPointInside( float x, float y ) const;
	bool IsPointInside( const Vector2& point ) const;
	Vector2 GetDimensions() const;
	Vector2 GetCenter() const;
	Vector2 GetMinXMaxY() const;
	Vector2 GetMaxXMinY() const;

public:
	void operator=( const AABB2& copyFrom );
	void operator+=(const Vector2& translation);
	void operator-=( const Vector2& antiTranslation );
	void operator*=( const Vector2& upScaling );
	void operator/=( const Vector2& downScaling );
	AABB2 operator+( const Vector2& translation ) const;
	AABB2 operator-( const Vector2& antiTranslation ) const;

public:
	static const AABB2 ZERO;
	static const AABB2 ONE;
	static const AABB2 ZERO_TO_ONE;
	static const AABB2 MINUS_ONE_TO_ONE;

public:
	Vector2 mins = Vector2::ZERO;
	Vector2 maxs = Vector2::ZERO;

};

/* STANDALONE FUNCTIONS */

bool DoAABBsOverlap( const AABB2& a, const AABB2& b );

bool DoesDiscAndAABBOverlap( const Disc2& disc, const AABB2& aabb );

bool DoesDiscAndAABBOverlap( const Vector2& discCenter, float discRadius, const AABB2& aabb );

const AABB2 Interpolate( const AABB2& start, const AABB2& end, float fractionTowardEnd );
