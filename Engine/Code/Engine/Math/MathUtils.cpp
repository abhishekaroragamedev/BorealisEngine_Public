#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/IntRange.hpp"
#include "Engine/Math/MathUtils.hpp"


#pragma region NumberUtils

bool IsFloatEqualTo( const float number, const float numberCompared )
{
	constexpr float epsilon = 0.001f;
	float difference = number - numberCompared;
	return ( difference < epsilon && difference > -epsilon );
}

int Max( const int a, const int b )
{
	if ( a >= b )
	{
		return a;
	}
	else
	{
		return b;
	}
}

int Min( const int a, const int b )
{
	if ( a <= b )
	{
		return a;
	}
	else
	{
		return b;
	}
}

float Max( const float a, const float b )
{
	if ( b > a )
	{
		return b;
	}
	else
	{
		return a;
	}
}

float Min( const float a, const float b )
{
	if ( b < a )
	{
		return b;
	}
	else
	{
		return a;
	}
}


#pragma endregion

#pragma region AngleUtils

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

float GetAngularDisplacement( float startDegrees, float endDegrees )
{
	float angularDisplacement = endDegrees - startDegrees;

	while ( angularDisplacement > 180 )
	{
		angularDisplacement -= 360;
	}
	while ( angularDisplacement < -180 )
	{
		angularDisplacement += 360;
	}

	return angularDisplacement;
}

float TurnToward( float currentDegrees, float goalDegrees, float maxTurnDegrees )
{
		float newAngle = currentDegrees;
		float angularDisplacement = GetAngularDisplacement( currentDegrees, goalDegrees );

		if( angularDisplacement > 0 )
		{
			newAngle += ( angularDisplacement < maxTurnDegrees )? ( goalDegrees - newAngle ) : maxTurnDegrees ;
		}
		else if ( angularDisplacement < 0 )
		{
			newAngle -= ( ( -angularDisplacement ) < maxTurnDegrees )? ( newAngle - goalDegrees ) : maxTurnDegrees ;
		}

		return newAngle;
}

#pragma endregion

#pragma region RoundClampRangeUtils

int RoundToNearestInt( const float inValue )
{
	int intValueRounded = static_cast<int>( inValue );
	if ( intValueRounded < 0.0f )
	{
		intValueRounded -= 1;
	}

	float differenceFromLowerIntValue = inValue - static_cast<float>( intValueRounded );

	if ( IsFloatEqualTo( differenceFromLowerIntValue, 0.50f ) || ( differenceFromLowerIntValue > 0.50f ) )
	{
		intValueRounded += 1;
	}

	return intValueRounded;
}

int ClampInt( const int inValue, const int minInclusive, const int maxInclusive )
{
	if ( inValue < minInclusive )
	{
		return minInclusive;
	}
	else if ( inValue > maxInclusive )
	{
		return maxInclusive;
	}

	return inValue;
}

float ClampFloat( const float inValue, const float minInclusive, const float maxInclusive )
{
	if ( inValue < minInclusive )
	{
		return minInclusive;
	}
	else if ( inValue > maxInclusive )
	{
		return maxInclusive;
	}
	
	return inValue;
}

float ClampFloatZeroToOne( const float inValue )
{
	if ( inValue < 0.0f )
	{
		return 0.0f;
	}
	else if ( inValue > 1.0f )
	{
		return 1.0f;
	}

	return inValue;
}

float ClampFloatNegativeOneToOne( const float inValue )
{
	if ( inValue < -1.0f )
	{
		return -1.0f;
	}
	else if ( inValue > 1.0f )
	{
		return 1.0f;
	}

	return inValue;
}

float GetFractionInRange( const float inValue, const float inStart, const float inEnd )
{
	float rangeDifference = inEnd - inStart;
	float inValueDifferenceFromStart = inValue - inStart;

	return ( inValueDifferenceFromStart / rangeDifference );
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

float Interpolate( const float start, const float end, const float fractionTowardEnd )
{
	float rangeDifference = end - start;
	float fractionOfRangeDifferenceToApply = fractionTowardEnd * rangeDifference;

	return ( start + fractionOfRangeDifferenceToApply );
}

int Interpolate( int start, int end, float fractionTowardEnd )
{
	float startFloat = static_cast<float>( start );
	float endFloat = static_cast<float>( end );
	float interpolatedFloat = Interpolate( startFloat, endFloat, fractionTowardEnd );

	return RoundToNearestInt( interpolatedFloat );
}

unsigned char Interpolate( unsigned char start, unsigned char end, float fractionTowardEnd )
{
	float startFloat = static_cast<float>( start );
	float endFloat = static_cast<float>( end );
	float interpolatedFloat = Interpolate( startFloat, endFloat, fractionTowardEnd );

	int nearestInt = RoundToNearestInt( interpolatedFloat );

	return static_cast<unsigned char>( nearestInt );
}

#pragma endregion

#pragma region BitflagUtils

bool AreBitsSet( unsigned char bitFlags8, unsigned char flagsToCheck )
{
	unsigned char bitsSetInFlag = bitFlags8 & flagsToCheck;
	return ( bitsSetInFlag == flagsToCheck );
}

bool AreBitsSet( unsigned int bitFlags32, unsigned int flagsToCheck )
{
	unsigned int bitsSetInFlag = bitFlags32 & flagsToCheck;
	return ( bitsSetInFlag == flagsToCheck );
}

void SetBits( unsigned char& bitFlags8, unsigned char flagsToSet )
{
	bitFlags8 |= flagsToSet;
}

void SetBits( unsigned int& bitFlags32, unsigned int flagsToSet )
{
	bitFlags32 |= flagsToSet;
}

void ClearBits( unsigned char& bitFlags8, unsigned char flagToClear )
{
	unsigned char flagToClearNegative = ~flagToClear;
	bitFlags8 &= flagToClearNegative;
}

void ClearBits( unsigned int& bitFlags32, unsigned int flagToClear )
{
	unsigned int flagToClearNegative = ~flagToClear;
	bitFlags32 &= flagToClearNegative;
}

#pragma endregion

#pragma region RandomUtils

float GetRandomFloatZeroToOne()
{
	return static_cast<float> ( rand() ) * ( 1.f / static_cast<float> (RAND_MAX) );
}

int GetRandomIntInRange( const int minInclusive, const int maxInclusive )
{
	int difference = maxInclusive - minInclusive;
	return ( rand() % ( difference + 1 ) ) + minInclusive;		// Use "difference + 1" below since the max number is included in the range
}

int GetRandomIntInRange( const IntRange& intRange )
{
	int difference = intRange.max - intRange.min;
	return ( rand() % ( difference + 1 ) ) + intRange.min;		// Use "difference + 1" below since the max number is included in the range
}

float GetRandomFloatInRange( const float minInclusive, const float maxInclusive )		// Use a combination of the above methods to achieve this
{
	float difference = maxInclusive - minInclusive;
	float fractionOfDifference = difference * ( static_cast<float> ( rand() ) / static_cast<float> ( RAND_MAX ) );

	return  minInclusive + fractionOfDifference;
}

float GetRandomFloatInRange( const FloatRange& floatRange )
{
	float difference = floatRange.max - floatRange.min;
	float fractionOfDifference = difference * ( static_cast<float> ( rand() ) / static_cast<float> ( RAND_MAX ) );

	return  floatRange.min + fractionOfDifference;
}

int GetRandomIntLessThan( const int maxNotInclusive )
{
	return rand() % maxNotInclusive;
}

bool CheckRandomChance( const float chanceOfSuccess )
{
	float randomFraction = static_cast<float> ( rand() ) / static_cast<float> ( RAND_MAX );		// This value will be in the range [0.0f, 1.0f]
	return ( ( randomFraction < chanceOfSuccess ) || IsFloatEqualTo( randomFraction, chanceOfSuccess ) );
}

#pragma endregion

#pragma region EasingFunctions

float SmoothStart2( float t )
{
	return ( t * t );
}

float SmoothStart3( float t )
{
	return ( t * ( t * t ) );
}

float SmoothStart4( float t )
{
	return ( t * ( t * ( t * t ) ) );
}

float SmoothStop2( float t )
{
	float oneMinusT = 1 - t;
	float oneMinusTSquare = oneMinusT * oneMinusT;

	return ( 1 - oneMinusTSquare );
}

float SmoothStop3( float t )
{
	float oneMinusT = 1 - t;
	float oneMinusTCube = oneMinusT * ( oneMinusT * oneMinusT );

	return ( 1 - oneMinusTCube );
}

float SmoothStop4( float t )
{
	float oneMinusT = 1 - t;
	float oneMinusTPowerFour = oneMinusT * ( oneMinusT * ( oneMinusT * oneMinusT ) );

	return ( 1 - oneMinusTPowerFour );
}

float SmoothStep3( float t )
{
	return BlendFloat( SmoothStop3( t ), SmoothStart3( t ), t );
}

#pragma endregion

#pragma region BlendFunctions

float BlendFloat( float a, float b, float t )
{
	if ( t < 0.0f )
	{
		return b;
	}
	else if ( t > 1.0f )
	{
		return a;
	}
	else
	{
		return ( ( t * a ) + ( ( 1-t ) * b ) );
	}
}

#pragma endregion
