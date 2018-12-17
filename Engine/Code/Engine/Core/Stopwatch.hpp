#pragma once

#include "Engine/Core/Clock.hpp"

class StopWatch
{

public:
	StopWatch( const Clock* referenceClock = nullptr );

	void SetClock( const Clock* newReferenceClock = nullptr );
	bool SetTimer( float seconds );
	void Reset();
	bool CheckAndReset( float newTimerSeconds = 0.0f );
	bool Decrement();
	unsigned int DecrementAll();
	//void Pause();
	//void Resume();

	float GetElapsedTime() const;
	float GetNormalizedElapsedTime() const;
	bool HasElapsed() const;

public:
	const Clock* m_referenceClock = nullptr;
	uint64_t m_startHPC;
	uint64_t m_intervalHPC;

};
