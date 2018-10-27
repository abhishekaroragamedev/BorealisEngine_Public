#pragma once

#include <string>

// Forward declaration
struct _FILETIME;
typedef _FILETIME FILETIME;

typedef unsigned long long uint64_t;

//-----------------------------------------------------------------------------------------------
double GetCurrentTimeSeconds();
float GetCurrentTimeSecondsF();
double GetCurrentTimeMilliseconds();
float GetCurrentTimeMillisecondsF();
uint64_t GetPerformanceCounter();
double PerformanceCounterToSeconds( const uint64_t HPC );
uint64_t SecondsToPerformanceCount( float seconds );

std::string GetTimestamp();
std::string GetTimestampForFilename();
FILETIME GetCurrentFileTime();
void IncrementFileTime( FILETIME* fileTime, float milliSeconds );
