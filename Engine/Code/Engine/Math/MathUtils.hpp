#pragma once

#include <math.h>
#include <stdlib.h>

constexpr float PI = 3.14159f;
constexpr float ROOT_TWO = 1.4142857f;
constexpr float ROOT_THREE = 1.732142f;
constexpr float ONE_BY_ROOT_TWO = 0.70710678f;
constexpr float ONE_BY_THREE = 1.0f / 3.0f;

class FloatRange;
class IntRange;
class Vector2;
class Vector3;

#pragma region NumberUtils
bool IsFloatEqualTo( const float number, const float numberCompared );
bool IsFloatGreaterThanOrEqualTo( const float number, const float numberCompared );
bool IsFloatLesserThanOrEqualTo( const float number, const float numberCompared );
bool IsDoubleEqualTo( const double number, const double numberCompared );
bool IsDoubleGreaterThanOrEqualTo( const double number, const double numberCompared );
bool IsDoubleLesserThanOrEqualTo( const double number, const double numberCompared );
int Max( const int a, const int b );
int Min( const int a, const int b );
float Max( const float a, const float b );
float Min( const float a, const float b );
int Abs( int a );
float Abs( float a );
int Sign( int a );
float Sign( float a );
int Integer( float a );	// Returns the integral part of the float - ( i - 1 ) for negative numbers [just a static_cast]
float IntegerF( float a );	// Returns the integral part of the float - ( i - 1.0f ) for negative numbers
float Fraction( float a );	// Returns the fractional part of the float - ( 1.0f - f ) for negative numbers
float Average( float a, float b );
float Floor( float a );
float Ceil( float a );

template< typename T >
bool IsBetween( T a, T min, T max )
{
	return ( ( a >= min ) && ( a <= max ) );
}
#pragma endregion

#pragma region AlgebraicUtils
bool SolveQuadratic( Vector2& out_solutions, float a, float b, float c );	// ax^2 + bx + c = 0
#pragma endregion

#pragma region AngleUtils
float ConvertRadiansToDegrees( float radians );
float ConvertDegreesToRadians( float degrees );
float CosDegrees( float degrees );
float SinDegrees( float degrees );
float TanDegrees( float degrees );
float ASinDegrees( float sin );
float ACosDegrees( float cos );
float ATanDegrees( float tan );
float ATan2Degrees( float y, float x );
float GetAngularDisplacement( float startDegrees, float endDegrees );
float TurnToward( float currentDegrees, float goalDegrees, float maxTurnDegrees );
#pragma endregion

#pragma region GeometricUtils
bool IsPointInSphere( const Vector3& point, const Vector3& center, float radius );
bool DoSpheresIntersect( const Vector3& aCenter, float aRadius, const Vector3& bCenter, float bRadius );
#pragma endregion

#pragma region RoundClampRangeUtils
int RoundToNearestInt( const float inValue );
int ClampInt( const int inValue, const int minInclusive, const int maxInclusive );
float ClampFloat( const float inValue, const float minInclusive, const float maxInclusive );
float ClampFloatZeroToOne( const float inValue );
float ClampFloatNegativeOneToOne( const float inValue );
float GetFractionInRange( const float inValue, const float inStart, const float inEnd );
float RangeMapFloat( const float inValue, const float inStart, const float inEnd, const float outStart, const float outEnd );
float Interpolate( const float start, const float end, const float fractionTowardEnd );
int Interpolate( int start, int end, float fractionTowardEnd );
unsigned char Interpolate( unsigned char start, unsigned char end, float fractionTowardEnd );
#pragma endregion

#pragma region BitflagUtils
bool AreBitsSet( unsigned char bitFlags8, unsigned char flagsToCheck );
bool AreBitsSet( unsigned int bitFlags32, unsigned int flagsToCheck );
void SetBits( unsigned char& bitFlags8, unsigned char flagsToSet );
void SetBits( unsigned int& bitFlags32, unsigned int flagsToSet );
void ClearBits( unsigned char& bitFlags8, unsigned char flagToClear );
void ClearBits( unsigned int& bitFlags32, unsigned int flagToClear );
#pragma endregion

#pragma region RandomUtils
float GetRandomFloatZeroToOne();
int GetRandomIntInRange( const int minInclusive, const int maxInclusive );
int GetRandomIntInRange( const IntRange& intRange );
float GetRandomFloatInRange( const float minInclusive, const float maxInclusive );
float GetRandomFloatInRange( const FloatRange& floatRange );
int GetRandomIntLessThan( const int maxNotInclusive );
bool CheckRandomChance( const float chanceOfSuccess );
#pragma endregion

#pragma region EasingFunctions
float SmoothStart2( float t ); 
float SmoothStart3( float t );  
float SmoothStart4( float t );
float SmoothStop2( float t );
float SmoothStop3( float t );
float SmoothStop4( float t );
float SmoothStep3( float t );
#pragma endregion

#pragma region BlendFunctions
float BlendFloat( float a, float b, float t );		// Blends a fraction t of a with (1-t) of b
#pragma endregion
