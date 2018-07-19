//-----------------------------------------------------------------------------------------------
// Time.hpp
//
#pragma once

#include <string>

typedef unsigned long long uint64_t;

//-----------------------------------------------------------------------------------------------
double GetCurrentTimeSeconds();
uint64_t GetPerformanceCounter();
double PerformanceCounterToSeconds( const uint64_t HPC );
uint64_t SecondsToPerformanceCount( float seconds );

std::string GetTimestamp();
std::string GetTimestampForFilename();
