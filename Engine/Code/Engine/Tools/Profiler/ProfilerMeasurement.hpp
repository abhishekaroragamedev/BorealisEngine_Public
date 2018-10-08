#pragma once

#include "Engine/Core/EngineCommon.hpp"
#include <vector>

#ifndef DISABLE_PROFILER
class ProfilerMeasurement
{

	friend class Profiler;
	friend class ProfilerReportEntry;

private:
	ProfilerMeasurement( const char* id );
	~ProfilerMeasurement();

	void AddChild( ProfilerMeasurement* child );
	void Finish();

	double GetElapsedTime() const;

	void DeleteChildrenRecursive();

private:
	const char* m_id = nullptr;
	ProfilerMeasurement* m_parent = nullptr;
	std::vector< ProfilerMeasurement* > m_children;
	uint64_t m_startHPC = 0;
	uint64_t m_endHPC = 0;

};
#endif
