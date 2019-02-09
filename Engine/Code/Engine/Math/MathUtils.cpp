#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/IntRange.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Tools/DevConsole.hpp"

#pragma region NumberUtils

bool IsFloatEqualTo( const float number, const float numberCompared )
{
	constexpr float epsilon = 0.001f;
	float difference = number - numberCompared;
	return ( difference < epsilon && difference > -epsilon );
}

bool IsFloatGreaterThanOrEqualTo( const float number, const float numberCompared )
{
	constexpr float epsilon = 0.001f;
	float difference = number - numberCompared;
	return ( IsFloatEqualTo( difference, 0.0f ) || difference > 0.0f );
}

bool IsFloatLesserThanOrEqualTo( const float number, const float numberCompared )
{
	constexpr float epsilon = 0.001f;
	float difference = number - numberCompared;
	return ( IsFloatEqualTo( difference, 0.0f ) || difference < 0.0f );
}

bool IsDoubleEqualTo( const double number, const double numberCompared )
{
	constexpr double epsilon = 0.001;
	double difference = number - numberCompared;
	return ( difference < epsilon && difference > -epsilon );
}

bool IsDoubleGreaterThanOrEqualTo( const double number, const double numberCompared )
{
	constexpr double epsilon = 0.001;
	double difference = number - numberCompared;
	return ( IsDoubleEqualTo( difference, 0.0 ) || difference > 0.0 );
}

bool IsDoubleLesserThanOrEqualTo( const double number, const double numberCompared )
{
	constexpr double epsilon = 0.001;
	double difference = number - numberCompared;
	return ( IsDoubleEqualTo( difference, 0.0 ) || difference < 0.0 );
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

size_t Max( const size_t a, const size_t b )
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

size_t Min( const size_t a, const size_t b )
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

int Abs( int a )
{
	if ( a > 0 )
	{
		return a;
	}
	else
	{
		return -a;
	}
}

float Abs( float a )
{
	if ( a > 0.0f )
	{
		return a;
	}
	else
	{
		return -a;
	}
}

int Sign( int a )
{
	if ( a == 0 )
	{
		return 0;
	}
	else if ( a >= 0 )
	{
		return 1;
	}
	else
	{
		return -1;
	}
}

float Sign( float a )
{
	if ( IsFloatEqualTo( a, 0.0f ) )
	{
		return 0.0f;
	}
	else if ( a < 0.0f )
	{
		return -1.0f;
	}
	else
	{
		return 1.0f;
	}
}

int Integer( float a )
{
	return static_cast< int >( a );
}

float IntegerF( float a )
{
	return static_cast< float >( Integer( a ) );
}

float Fraction( float a )
{
	return ( a - floorf( a ) );
}

float Average( float a, float b )
{
	return ( ( a + b ) * 0.5f );
}

float Floor( float a )
{
	return floorf( a );
}

float Ceil( float a )
{
	return ceilf( a );
}

void FractionToBits( float fract, int numBits, char* out_bits )
{
	fract = Fraction( fract );

	char* currentBit = out_bits;

	while ( numBits > 0 )
	{
		fract *= 2.0f;

		*currentBit = ( Integer( fract ) )? '1' : '0';	// Can only be zero or one
		currentBit++;

		fract = Fraction( fract );

		numBits--;
	}
}

void ReverseBits( char* bits, int numBits, char* out_reverse )
{
	int bitIndex = 0;
	while ( bitIndex < numBits )
	{
		out_reverse[ numBits - bitIndex - 1 ] = bits[ bitIndex ];
		bitIndex++;
	}
}

float BitsToFraction( char* bits, int numBits )
{
	float fraction = 0.0f;

	float base = 1.0f;
	int bitIndex = 0;
	while ( bitIndex < numBits )
	{
		base *= 0.5f;

		int currentBit = static_cast< int >( bits[ bitIndex ] ) - 48;	// 48 is the ascii value of '0'
		fraction += base * static_cast< float >( currentBit );

		bitIndex++;
	}

	return fraction;
}

#pragma endregion

#pragma region AlgebraicUtils

bool SolveQuadratic( Vector2& out_solutions, float a, float b, float c )
{
	if ( IsFloatEqualTo( a, 0.0f ) && IsFloatEqualTo( b, 0.0f ) )
	{
		out_solutions = Vector2::ZERO;
		ConsolePrintf( Rgba::YELLOW, "WARNING: SolveQuadratic: Provided coefficents for a and b are both zero. Equation cannot be solved." );
		return false;
	}

	float discriminant = ( b * b ) - 4 * ( a * c );
	if ( IsFloatGreaterThanOrEqualTo( discriminant, 0.0f ) )
	{
		if ( IsFloatEqualTo( a, 0.0f ) && !IsFloatEqualTo( b, 0.0f ) )	// Only one root exists; the extra check for b being nonzero is just for readability; this is handled at the beginning of the function
		{
			float root = -c / b;
			out_solutions = Vector2( root, root );
		}
		else	// Handles both zero and nonzero determinant cases
		{
			float rootOfDiscriminant = sqrtf( discriminant );
			float denominator = 0.5f / a;
			float firstRoot = ( -b + rootOfDiscriminant ) * denominator;
			float secondRoot = ( -b - rootOfDiscriminant ) * denominator;
			out_solutions = Vector2( Min( firstRoot, secondRoot ), Max( firstRoot, secondRoot ) );
		}
		return true;
	}
	else	// No solutions exist
	{
		out_solutions = Vector2::ZERO;
		return false;
	}
}

#pragma endregion

#pragma region AngleUtils

float ConvertRadiansToDegrees( float radians )
{
	return ( radians * ( 180.0f / PI ) );
}

float ConvertDegreesToRadians( float degrees )
{
	return ( degrees * ( PI / 180.0f ) );
}

float CosDegrees ( float degrees )
{
	float radians = ConvertDegreesToRadians( degrees );
	return cosf(radians);
}

float SinDegrees ( float degrees )
{
	float radians = ConvertDegreesToRadians( degrees );
	return sinf( radians );
}

float TanDegrees( float degrees )
{
	float radians = ConvertDegreesToRadians( degrees );
	return tanf( radians );
}

float ASinDegrees( float sin )
{
	float aSinRadians = asinf( sin );
	return ConvertRadiansToDegrees( aSinRadians );
}

float ACosDegrees( float cos )
{
	float aCosRadians = acosf( cos );
	return ConvertRadiansToDegrees( aCosRadians );
}

float ATanDegrees( float tan )
{
	float aTanRadians = atanf( tan );
	return ConvertRadiansToDegrees( aTanRadians );
}

float ATan2Degrees( float y, float x )
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

#pragma region GeometricUtils

bool IsPointInSphere( const Vector3& point, const Vector3& center, float radius )
{
	float squareDistance = ( point - center ).GetLengthSquared();
	float squareRadius = radius * radius;
	return IsFloatLesserThanOrEqualTo( squareDistance, squareRadius );
}

bool DoSpheresIntersect( const Vector3& aCenter, float aRadius, const Vector3& bCenter, float bRadius )
{
	float squareDistanceBetweenCenters = ( aCenter - bCenter ).GetLengthSquared();
	
	float squareSumOfRadii = ( aRadius + bRadius );
	squareSumOfRadii *= squareSumOfRadii;

	return IsFloatLesserThanOrEqualTo( squareDistanceBetweenCenters, squareSumOfRadii );
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
	return ( ( randomFraction < chanceOfSuccess ) );
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
