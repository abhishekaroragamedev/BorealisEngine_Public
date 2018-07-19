#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Tools/Profiler/Profiler.hpp"
#include "Engine/Tools/Profiler/ProfileScope.hpp"

ProfileScope::ProfileScope( const char* tag )
{
#ifndef DISABLE_PROFILER
	Profiler::StartProfile( tag );
#endif
}

ProfileScope::~ProfileScope()
{
#ifndef DISABLE_PROFILER
	Profiler::EndProfile();
#endif
}

ProfileLogScope::ProfileLogScope( const char* tag )
{
	m_tag = tag;
	m_startHPC = GetPerformanceCounter();
}

ProfileLogScope::~ProfileLogScope()
{
	uint64_t elapsedTime = GetPerformanceCounter() - m_startHPC;
	double elapsedSeconds = PerformanceCounterToSeconds( elapsedTime );
	DebuggerPrintf( "[%s] took %f seconds.\n", m_tag, elapsedSeconds );
}
