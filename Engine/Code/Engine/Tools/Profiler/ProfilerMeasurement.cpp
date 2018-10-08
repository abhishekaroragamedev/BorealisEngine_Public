#include "Engine/Core/Time.hpp"
#include "Engine/Tools/Profiler/ProfilerMeasurement.hpp"

#ifndef DISABLE_PROFILER
ProfilerMeasurement::ProfilerMeasurement( const char* id )
	:	m_id( id )
{
	m_startHPC = GetPerformanceCounter();
}

ProfilerMeasurement::~ProfilerMeasurement()
{
	DeleteChildrenRecursive();
}

void ProfilerMeasurement::DeleteChildrenRecursive()
{
	for ( ProfilerMeasurement* childMeasurement : m_children )
	{
		delete childMeasurement;
	}
}

void ProfilerMeasurement::AddChild( ProfilerMeasurement* child )
{
	child->m_parent = this;
	m_children.push_back( child );
}

void ProfilerMeasurement::Finish()
{
	m_endHPC = GetPerformanceCounter();
}

double ProfilerMeasurement::GetElapsedTime() const
{
	double elapsedTime = PerformanceCounterToSeconds( m_endHPC - m_startHPC );
	return elapsedTime;
}

#endif
