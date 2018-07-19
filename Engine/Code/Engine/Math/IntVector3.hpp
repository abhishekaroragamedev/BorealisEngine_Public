#pragma once

#include "Engine/Math/IntVector2.hpp"
#include "Engine/Math/MathUtils.hpp"

class Vector3;

class IntVector3
{

public:
	IntVector3();
	explicit IntVector3( int xCoordinate, int yCoordinate, int zCoordinate );
	explicit IntVector3( const IntVector2& intVector2, int zCoordinate );
	~IntVector3();

public:
	const IntVector3 operator+( const IntVector3& vectorToAdd ) const;
	const IntVector3 operator-( const IntVector3& vectorToSubtract ) const;
	const bool operator==( const IntVector3& vectorToCompare ) const;
	const bool operator>( const IntVector3& vectorToCompare ) const;
	const bool operator>=( const IntVector3& vectorToCompare ) const;
	const bool operator<( const IntVector3& vectorToCompare ) const;
	const bool operator<=( const IntVector3& vectorToCompare ) const;

public:
	void SetFromText( const char* text );

public:
	static const IntVector3 ZERO;
	static const IntVector3 ONE;

public:
	int x = 0;
	int y = 0;
	int z = 0;

};

const IntVector3 Interpolate( const IntVector3& start, const IntVector3& end, float fractionTowardEnd );

IntVector3 ConvertVector3ToIntVector3( const Vector3& vecToConvert );
