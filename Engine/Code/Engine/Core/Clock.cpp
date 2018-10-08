#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Tools/Command.hpp"
#include "Engine/Tools/DevConsole.hpp"

static Clock g_masterClock;		// Must be created after g_timeSystem in Time.cpp for correct HPC

Clock::Clock( Clock* parentClock /* = nullptr */ )
{
	if ( this != &g_masterClock && parentClock == nullptr )
	{
		parentClock = &g_masterClock;
		m_parentClock = parentClock;
		m_parentClock->AddChild( this );
	}

	Reset();
}

void Clock::Reset()
{
	m_lastFrameHPC = GetPerformanceCounter();
	memset( &m_frameTime, 0, sizeof( TimeUnit ) );
	memset( &m_totalTime, 0, sizeof( TimeUnit ) );
	m_frameCount = 0;
}

void Clock::BeginFrame()
{
	uint64_t currentHPC = GetPerformanceCounter();
	uint64_t timeElapsed = currentHPC - m_lastFrameHPC;
	StepClock( timeElapsed );
	m_lastFrameHPC = currentHPC;
}

void Clock::StepClock( uint64_t timeElapsed )
{
	m_frameCount++;

	if ( m_paused )
	{
		timeElapsed = 0;
	}
	else
	{
		timeElapsed = static_cast< uint64_t >( static_cast< double >( timeElapsed ) * m_timeScale );
	}

	double elapsedSeconds = PerformanceCounterToSeconds( timeElapsed );
	m_frameTime.seconds = elapsedSeconds;
	m_frameTime.hpc = timeElapsed;
	m_frameTime.milliSeconds = static_cast< unsigned int >( 1000.0 * m_frameTime.seconds );

	m_totalTime.seconds += m_frameTime.seconds;
	m_totalTime.hpc += m_frameTime.hpc;
	m_totalTime.milliSeconds += m_frameTime.milliSeconds;

	for ( Clock* childClock : m_childClocks )
	{
		childClock->StepClock( timeElapsed );
	}
}

void Clock::AddChild( Clock* childClock )
{
	m_childClocks.push_back( childClock );
}

void Clock::Pause()
{
	m_paused = true;
}

void Clock::Unpause()
{
	m_paused = false;
}

void Clock::SetTimeScale( double timeScale )
{
	m_timeScale = timeScale;
}

void Clock::ResetTimeScale()
{
	m_timeScale = 1.0f;
}

TimeUnit Clock::GetLastFrameTime() const
{
	return m_frameTime;
}

TimeUnit Clock::GetTotalTime() const
{
	return m_totalTime;
}

double Clock::GetDeltaSeconds() const
{
	return m_frameTime.seconds;
}

float Clock::GetDeltaSecondsF() const
{
	return static_cast< float >( m_frameTime.seconds );
}

double Clock::GetTimeScale() const
{
	return m_timeScale;
}

float Clock::GetTimeScaleF() const
{
	return static_cast< float >( m_timeScale );
}

bool Clock::IsPaused() const
{
	return m_paused;
}

uint64_t Clock::GetLastFrameHPC() const
{
	return m_lastFrameHPC;
}

unsigned int Clock::GetFrameCount() const
{
	return m_frameCount;
}

// -------------------------------------------------------------

void ClockSystemStartup()
{
	g_masterClock.Reset();
}

void ClockSystemBeginFrame()
{
	g_masterClock.BeginFrame();
}

double GetMasterDeltaSeconds()
{
	return g_masterClock.GetDeltaSeconds();
}

float GetMasterDeltaSecondsF()
{
	return g_masterClock.GetDeltaSecondsF();
}

float GetMasterClockFrameTimeSecondsF()
{
	return static_cast< float >( g_masterClock.m_frameTime.seconds );
}

float GetMasterClockTimeElapsedSecondsF()
{
	return static_cast< float >( g_masterClock.m_totalTime.seconds );
}

unsigned int GetMasterClockFrameCount()
{
	return g_masterClock.GetFrameCount();
}

const Clock* GetMasterClockReference()
{
	return &g_masterClock;
}

#pragma region ConsoleCommands

void RegisterClockCommands()
{
	CommandRegister( "clock_pause", PauseMasterClockCommand, "Pauses the master clock." );
	CommandRegister( "clock_step", StepFrameCommand, "Steps the master clock forward by one frame." );
	CommandRegister( "time_scale", AdjustTimeScaleCommand, "Sets the master clock's time scale to the specified non-negative value." );
	CommandRegister( "clock_reset", ResetMasterClockSettingsCommand, "Resets the master clock's time scale and un-pauses it." );
}

bool PauseMasterClockCommand( Command& pauseCommand )
{
	if ( pauseCommand.GetName() == "clock_pause" )
	{
		g_masterClock.Pause();
		return true;
	}
	return false;
}

bool StepFrameCommand( Command& stepCommand )
{
	if ( stepCommand.GetName() == "clock_step" )
	{
		g_masterClock.StepClock( static_cast< uint64_t >( GetMasterDeltaSeconds() ) );
		return true;
	}
	return false;
}

bool AdjustTimeScaleCommand( Command& timeScaleCommand )
{
	if ( timeScaleCommand.GetName() == "time_scale" )
	{
		std::string timeScaleString = timeScaleCommand.GetNextString();
		if ( timeScaleString == "" )
		{
			ConsolePrintf( Rgba::RED, "ERROR: AdjustTimeScaleCommand: Time scale value missing. Please provide a positive number as time scale." );
			return false;
		}

		double timeScale = 0.0;
		try
		{
			timeScale = stod( timeScaleString );
		}
		catch ( std::invalid_argument& invalidArgument )
		{
			UNUSED( invalidArgument );
			ConsolePrintf( Rgba::RED, "ERROR: AdjustTimeScaleCommand: Numerical time scale value missing." );
			return false;
		}

		if ( timeScale < 0.0f )
		{
			ConsolePrintf( Rgba::RED, "ERROR: AdjustTimeScaleCommand: Time scale cannot be negative." );
			return false;
		}

		g_masterClock.SetTimeScale( timeScale );
		return true;
	}
	return false;
}

bool ResetMasterClockSettingsCommand( Command& resetCommand )
{
	if ( resetCommand.GetName() == "clock_reset" )
	{
		g_masterClock.ResetTimeScale();
		g_masterClock.Unpause();
		return true;
	}
	return false;
}

#pragma endregion
