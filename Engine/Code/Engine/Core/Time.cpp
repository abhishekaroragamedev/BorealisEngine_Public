//-----------------------------------------------------------------------------------------------
// Time.cpp
//	

//-----------------------------------------------------------------------------------------------
#include "Engine/Core/Time.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stdint.h>

// Suggestion from MSFT for working with FILETIME struct: https://support.microsoft.com/en-us/help/188768/info-working-with-the-filetime-structure
#define SECOND ((uint64_t) 10000000)
#define MILLISECOND (SECOND / 1000)
#define MINUTE (60 * SECOND)
#define HOUR   (60 * MINUTE)
#define DAY    (24 * HOUR)

//-----------------------------------------------------------------------------------------------
double InitializeTime( LARGE_INTEGER& out_initialTime )
{
	LARGE_INTEGER countsPerSecond;
	QueryPerformanceFrequency( &countsPerSecond );
	QueryPerformanceCounter( &out_initialTime );
	return( 1.0 / static_cast< double >( countsPerSecond.QuadPart ) );
}


//-----------------------------------------------------------------------------------------------
double GetCurrentTimeSeconds()
{
	static LARGE_INTEGER s_initialTime;
	static double s_secondsPerCount = InitializeTime( s_initialTime );
	LARGE_INTEGER currentCount;
	QueryPerformanceCounter( &currentCount );
	LONGLONG elapsedCountsSinceInitialTime = currentCount.QuadPart - s_initialTime.QuadPart;

	double currentSeconds = static_cast< double >( elapsedCountsSinceInitialTime ) * s_secondsPerCount;
	return currentSeconds;
}

float GetCurrentTimeSecondsF()
{
	static LARGE_INTEGER s_initialTime;
	static double s_secondsPerCount = InitializeTime( s_initialTime );
	LARGE_INTEGER currentCount;
	QueryPerformanceCounter( &currentCount );
	LONGLONG elapsedCountsSinceInitialTime = currentCount.QuadPart - s_initialTime.QuadPart;

	float currentSeconds = static_cast< float >( elapsedCountsSinceInitialTime ) * static_cast< float >( s_secondsPerCount );
	return currentSeconds;
}

double GetCurrentTimeMilliseconds()
{
	return ( GetCurrentTimeSeconds() * 1000.0 );
}

float GetCurrentTimeMillisecondsF()
{
	return ( GetCurrentTimeSecondsF() * 1000.0f );
}

uint64_t GetPerformanceCounter()
{
	LARGE_INTEGER smolLarg;
	QueryPerformanceCounter( &smolLarg );
	return static_cast< uint64_t >( smolLarg.QuadPart );
}

class TimeSystem
{

public:
	TimeSystem()
	{
		LARGE_INTEGER smolLarg;
		QueryPerformanceFrequency( &smolLarg );
		m_frequency = static_cast< uint64_t >( smolLarg.QuadPart );
		m_secondsPerCount = 1.0f / static_cast< double >( m_frequency );
	}

public:
	uint64_t m_frequency = 0;
	double m_secondsPerCount = 0.0f;

};

static TimeSystem g_timeSystem;	// static - will get created before WinMain

double PerformanceCounterToSeconds( const uint64_t HPC )
{
	return static_cast< double >( HPC * g_timeSystem.m_secondsPerCount );
}

uint64_t SecondsToPerformanceCount( float seconds )
{
	return static_cast< uint64_t >( static_cast< double >( seconds ) / g_timeSystem.m_secondsPerCount );
}

std::string GetTimestamp()
{
	SYSTEMTIME localTime = { 0 };
	GetLocalTime( &localTime );
	return Stringf( "%02d:%02d:%02d %02d/%02d/%04d", localTime.wHour, localTime.wMinute, localTime.wSecond, localTime.wMonth, localTime.wDay, localTime.wYear );
}

std::string GetTimestampForFilename()
{
	SYSTEMTIME localTime = { 0 };
	GetLocalTime( &localTime );
	return Stringf( "%04d%02d%02d_%02d%02d%02d", localTime.wYear, localTime.wMonth, localTime.wDay, localTime.wHour, localTime.wMinute, localTime.wSecond );
}

FILETIME GetCurrentFileTime()
{
	SYSTEMTIME localTime = { 0 };
	GetLocalTime( &localTime );

	FILETIME fileTime;
	SystemTimeToFileTime( &localTime, &fileTime );
	return fileTime;
}

// Approach used in https://support.microsoft.com/en-us/help/188768/info-working-with-the-filetime-structure
void IncrementFileTime( FILETIME* fileTime, float milliSeconds )
{
	ULONGLONG ftLong;
	ftLong = ( ( ( ULONGLONG ) fileTime->dwHighDateTime ) << 32 ) + fileTime->dwLowDateTime;
	ftLong += ( ( ULONGLONG ) milliSeconds ) * MILLISECOND;

	fileTime->dwLowDateTime = ( DWORD )( ftLong & 0xFFFFFFFF );
	fileTime->dwHighDateTime = ( DWORD )( ftLong >> 32 );
}
