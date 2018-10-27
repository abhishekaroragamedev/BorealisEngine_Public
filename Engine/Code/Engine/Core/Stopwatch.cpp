#include "Engine/Core/Stopwatch.hpp"

StopWatch::StopWatch( const Clock* parentClock /* = nullptr */ )
{
	SetClock( parentClock );
}

void StopWatch::SetClock( const Clock* newReferenceClock /* = nullptr */ )
{
	if ( newReferenceClock == nullptr )
	{
		m_referenceClock = GetMasterClockReference();
	}
	else
	{
		m_referenceClock = newReferenceClock;
	}
}

bool StopWatch::SetTimer( float seconds )
{
	m_startHPC = m_referenceClock->GetLastFrameHPC();
	m_intervalHPC = m_startHPC + SecondsToPerformanceCount( seconds );

	return true;
}

void StopWatch::Reset()
{
	m_startHPC = m_referenceClock->GetLastFrameHPC();
	m_intervalHPC = m_startHPC;
}

bool StopWatch::CheckAndReset( float newTimerSeconds /* = 0.0f */ )
{
	if ( m_referenceClock->GetLastFrameHPC() > m_intervalHPC )
	{
		Reset();
		m_intervalHPC = m_startHPC + SecondsToPerformanceCount( newTimerSeconds );
		return true;
	}
	else
	{
		return false;
	}
}

bool StopWatch::Decrement()
{
	bool hasElapsed = HasElapsed();

	if ( hasElapsed )
	{
		m_startHPC += m_intervalHPC;
		m_intervalHPC += m_intervalHPC;
	}

	return hasElapsed;
}

unsigned int StopWatch::DecrementAll()
{
	uint64_t numDecrements = 0;

	if ( HasElapsed() )
	{
		uint64_t currentTime = m_referenceClock->GetLastFrameHPC();
		uint64_t elapsedTime = currentTime - m_startHPC;
		uint64_t interval = m_intervalHPC - m_startHPC;

		if ( elapsedTime != interval )
		{
			numDecrements = elapsedTime/ interval;
		}
		else
		{
			numDecrements = 0;
		}
	}

	m_startHPC += numDecrements * m_intervalHPC;
	m_intervalHPC += numDecrements * m_intervalHPC;

	return static_cast< unsigned int >( numDecrements );
}

float StopWatch::GetElapsedTime() const
{
	return static_cast< float >( PerformanceCounterToSeconds( m_referenceClock->GetLastFrameHPC() - m_startHPC ) );
}

float StopWatch::GetNormalizedElapsedTime() const
{
	if ( m_startHPC == m_intervalHPC )
	{
		return 0.0f;
	}
	return ( GetElapsedTime() / static_cast< float >( PerformanceCounterToSeconds( m_intervalHPC - m_startHPC ) ) );
}

bool StopWatch::HasElapsed() const
{
	return ( ( m_referenceClock->GetLastFrameHPC() >= m_intervalHPC ) );
}
