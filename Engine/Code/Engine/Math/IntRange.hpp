#pragma once

#include "Engine/Math/MathUtils.hpp"
#include <vector>

class IntRange
{

public:
	IntRange();
	explicit IntRange( int initialMin, int initialMax );
	explicit IntRange( int initialMinMax );

	void SetFromText( const char* text );
	std::vector< int > GetAllIntsInRange() const;
	int GetRandomInRange() const;
	int GetRangeMagnitude() const;
	bool Contains( int number ) const;

public:
	int min = 0;
	int max = 0;

};

bool DoRangesOverlap( const IntRange& a, const IntRange& b );

const IntRange Interpolate( const IntRange& start, const IntRange& end, float fractionTowardEnd );

void SetFromText( std::vector< int >& out_vectorOfInts, const char* text );