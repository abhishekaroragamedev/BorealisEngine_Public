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

#pragma region NumberUtils
bool IsFloatEqualTo( const float number, const float numberCompared );
int Max( const int a, const int b );
int Min( const int a, const int b );
float Max( const float a, const float b );
float Min( const float a, const float b );
#pragma endregion

#pragma region AngleUtils
float ConvertRadiansToDegrees( const float radians );
float ConvertDegreesToRadians( const float degrees );
float CosDegrees( const float degrees );
float SinDegrees( const float degrees );
float ATan2Degrees( const float y, const float x );
float GetAngularDisplacement( float startDegrees, float endDegrees );
float TurnToward( float currentDegrees, float goalDegrees, float maxTurnDegrees );
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
