#pragma once

#include "Engine/Math/MathUtils.hpp"
#include <string>

class Vector2;

class IntVector2
{

public:
	IntVector2();
	explicit IntVector2( int xCoordinate, int yCoordinate );
	IntVector2( const IntVector2& copy );
	~IntVector2();

public:
	const IntVector2 operator+( const IntVector2& vectorToAdd ) const;
	const IntVector2 operator-( const IntVector2& vectorToSubtract ) const;
	const bool operator==( const IntVector2& vectorToCompare ) const;
	const bool operator!=( const IntVector2& vectorToCompare ) const;
	const bool operator>( const IntVector2& vectorToCompare ) const;
	const bool operator>=( const IntVector2& vectorToCompare ) const;
	const bool operator<( const IntVector2& vectorToCompare ) const;
	const bool operator<=( const IntVector2& vectorToCompare ) const;
	void operator=( const IntVector2& assignedVector );

public:
	void SetFromText( const char* text );
	void SetFromText( const std::string& text );

public:
	static IntVector2 GetAsSigns( const IntVector2& intVector );	// Returns an IntVector2 with both x and y reduced to -1 or 1, depending on their sign

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
