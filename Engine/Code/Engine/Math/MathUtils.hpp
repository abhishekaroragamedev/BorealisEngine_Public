#pragma once

#include <math.h>
#include <stdlib.h>

constexpr float PI = 3.14159f;

bool IsFloatEqualTo( const float number, const float numberCompared );

float ConvertRadiansToDegrees( const float radians );

float ConvertDegreesToRadians( const float degrees );

float CosDegrees( const float degrees );

float SinDegrees( const float degrees );

float ATan2Degrees( const float y, const float x );

float ClampFloat( float inValue, const float minInclusive, const float maxInclusive );

float RangeMapFloat( const float inValue, const float inStart, const float inEnd, const float outStart, const float outEnd );

float GetRandomFloatZeroToOne();

int GetRandomIntInRange( const int minInclusive, const int maxInclusive );

float GetRandomFloatInRange( const float minInclusive, const float maxInclusive );

int GetRandomIntLessThan( const int maxNotInclusive );