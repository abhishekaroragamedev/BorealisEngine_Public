#include "Engine/Math/MathUtils.hpp"

bool IsFloatEqualTo( const float number, const float numberCompared )
{
	constexpr float epsilon = 0.001f;
	float difference = number - numberCompared;
	return ( difference < epsilon && difference > -epsilon );
}

float ConvertRadiansToDegrees( const float radians )
{
	return ( radians * ( 180.0f / PI ) );
}

float ConvertDegreesToRadians( const float degrees )
{
	return ( degrees * ( PI / 180.0f ) );
}

float CosDegrees ( const float degrees )
{
	float radians = ConvertDegreesToRadians( degrees );
	return cosf(radians);
}

float SinDegrees ( const float degrees )
{
	float radians = ConvertDegreesToRadians( degrees );
	return sinf( radians );
}

float ATan2Degrees( const float y, const float x )
{
	float aTan2Radians = atan2f( y, x );
	return ConvertRadiansToDegrees( aTan2Radians );
}

float ClampFloat( float inValue, const float minInclusive, const float maxInclusive )
{
	if ( inValue < minInclusive )
	{
		inValue = minInclusive;
	}
	else if ( inValue > maxInclusive )
	{
		inValue = maxInclusive;
	}
	
	return inValue;
}

float RangeMapFloat( const float inValue, const float inStart, const float inEnd, const float outStart, const float outEnd )
{
	float inRange = inEnd - inStart;

	if ( IsFloatEqualTo( inRange, 0.0f ) )
	{
		return ( ( outStart + outEnd ) * 0.5f );
	}

	float inRelativeToStart = inValue - inStart;
	
	float outRange = outEnd - outStart;
	float fractionIntoRange = inRelativeToStart / inRange;
	float outRelativeToStart = fractionIntoRange * outRange;

	return ( outStart + outRelativeToStart );
}

float GetRandomFloatZeroToOne()
{
	return static_cast<float> ( rand() ) * ( 1.f / static_cast<float> (RAND_MAX) );
}

int GetRandomIntInRange( const int minInclusive, const int maxInclusive )
{
	int difference = maxInclusive - minInclusive;
	return ( rand() % ( difference + 1 ) ) + minInclusive;		// Use "difference + 1" below since the max number is included in the range
}

float GetRandomFloatInRange( const float minInclusive, const float maxInclusive )		// Use a combination of the above methods to achieve this
{
	float difference = maxInclusive - minInclusive;
	float fractionOfDifference = difference * ( static_cast<float> ( rand() ) / static_cast<float> ( RAND_MAX ) );

	return  minInclusive + fractionOfDifference;
}

int GetRandomIntLessThan( const int maxNotInclusive )
{
	return rand() % maxNotInclusive;
}