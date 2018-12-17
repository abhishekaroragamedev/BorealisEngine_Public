#pragma once

#include "Engine/Core/Time.hpp"
#include <vector>

class Command;

struct TimeUnit
{

public:
	double seconds = 0.0f;
	uint64_t hpc = 0;
	unsigned int milliSeconds = 0;

};

class Clock
{

public:
	Clock( Clock* parentClock = nullptr );
	Clock( const Clock& copy ) = delete;	// Copying is disallowed

	void Reset();
	void BeginFrame();
	void StepClock( uint64_t timeElapsed );
	void AddChild( Clock* childClock );
	void Pause();
	void Unpause();
	void SetTimeScale( double timeScale );
	void ResetTimeScale();
	void SetParent( Clock* newParent );	// Can be used to set parent to nullptr
	
	TimeUnit GetLastFrameTime() const;
	TimeUnit GetTotalTime() const;
	double GetDeltaSeconds() const;
	float GetDeltaSecondsF() const;
	double GetTimeScale() const;
	float GetTimeScaleF() const;
	bool IsPaused() const;
	uint64_t GetLastFrameHPC() const;
	unsigned int GetFrameCount() const;

public:
	TimeUnit m_frameTime;
	TimeUnit m_totalTime;

private:
	Clock* m_parentClock = nullptr;
	std::vector< Clock* > m_childClocks;
	unsigned int m_frameCount = 0;
	double m_timeScale = 1.0f;
	bool m_paused = false;
	uint64_t m_lastFrameHPC = 0;

};

void ClockSystemStartup();		// Used to instantiate the master clock during app initialization
void ClockSystemBeginFrame();	// Called at the beginning of every frame
double GetMasterDeltaSeconds();	// Returns the absolute deltaSeconds for the frame
float GetMasterDeltaSecondsF();
float GetMasterClockFrameTimeSecondsF();
float GetMasterClockTimeElapsedSecondsF();
unsigned int GetMasterClockFrameCount();
const Clock* GetMasterClockReference();

void RegisterClockCommands();
bool PauseMasterClockCommand( Command& command );
bool StepFrameCommand( Command& command );
bool AdjustTimeScaleCommand( Command& command );
bool ResetMasterClockSettingsCommand( Command& command );
