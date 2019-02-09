#pragma once

#include "Engine/Math/MathUtils.hpp"

class FloatRange
{

public:
	explicit FloatRange( float initialMin, float initialMax );
	explicit FloatRange( float initialMinMax );

	void SetFromText( const char* text );
	float GetRandomInRange() const;
	float Size() const;

public:
	float min = 0.0f;
	float max = 0.0f;

};

bool DoRangesOverlap( const FloatRange& a, const FloatRange& b );

const FloatRange Interpolate( const FloatRange& start, const FloatRange& end, float fractionTowardEnd );
