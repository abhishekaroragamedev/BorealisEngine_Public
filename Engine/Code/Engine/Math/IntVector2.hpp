#pragma once

#include "Engine/Math/MathUtils.hpp"

class Vector2;

class IntVector2
{

public:
	IntVector2();
	explicit IntVector2( int xCoordinate, int yCoordinate );
	~IntVector2();

public:
	const IntVector2 operator+( const IntVector2& vectorToAdd ) const;
	const IntVector2 operator-( const IntVector2& vectorToSubtract ) const;
	const bool operator==( const IntVector2& vectorToCompare ) const;

public:
	void SetFromText( const char* text );

public:
	static const IntVector2 ZERO;
	static const IntVector2 ONE;
	static const IntVector2 STEP_NORTH;
	static const IntVector2 STEP_SOUTH;
	static const IntVector2 STEP_WEST;
	static const IntVector2 STEP_EAST;
	static const IntVector2 STEP_NORTHWEST;
	static const IntVector2 STEP_NORTHEAST;
	static const IntVector2 STEP_SOUTHWEST;
	static const IntVector2 STEP_SOUTHEAST;

public:
	int x = 0;
	int y = 0;

};

const IntVector2 Interpolate( const IntVector2& start, const IntVector2& end, float fractionTowardEnd );

IntVector2 ConvertVector2ToIntVector2( const Vector2& vecToConvert );
