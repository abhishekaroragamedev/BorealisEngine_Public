#include "Engine/Tools/Profiler/ProfilerMeasurement.hpp"
#include "Engine/Tools/Profiler/ProfilerReport.hpp"
#include "Engine/Tools/Profiler/ProfilerReportEntry.hpp"
#include <algorithm>

ProfilerReport::ProfilerReport()
{

}

void ProfilerReport::GenerateTreeReport( const ProfilerMeasurement& frameMeasurement )
{
	m_rootEntry = new ProfilerReportEntry();
	m_rootEntry->PopulateTree( frameMeasurement );
}

void ProfilerReport::GenerateFlatReport( const ProfilerMeasurement& frameMeasurement, ProfilerReportSortMode sortMode )
{
	m_rootEntry = new ProfilerReportEntry();
	m_rootEntry->AccumulateData( frameMeasurement );
	double totalChildrenTime = m_rootEntry->PopulateFlat( frameMeasurement );
	m_rootEntry->m_selfTimeMS = m_rootEntry->m_totalTimeMS - totalChildrenTime;
	m_rootEntry->m_percentageSelfTime = ( m_rootEntry->m_selfTimeMS / m_rootEntry->m_totalTimeMS ) * 100.0f;

	switch( sortMode )
	{
		case ProfilerReportSortMode::PROFILER_REPORT_SORT_TOTAL:	SortEntriesByTotalTime();	break;
		case ProfilerReportSortMode::PROFILER_REPORT_SORT_SELF:		SortEntriesBySelfTime();	break;
	}
}

void ProfilerReport::AggregateReport( ProfilerReport& report, ProfilerReportSortMode sortMode /* = ProfilerReportSortMode::PROFILER_REPORT_SORT_INVALID */ )
{
	m_rootEntry->AggregateEntry( *report.m_rootEntry );

	if ( sortMode != ProfilerReportSortMode::PROFILER_REPORT_SORT_INVALID )
	{
		m_flatViewSortedEntries.clear();
		switch( sortMode )
		{
			case ProfilerReportSortMode::PROFILER_REPORT_SORT_TOTAL:	SortEntriesByTotalTime();	break;
			case ProfilerReportSortMode::PROFILER_REPORT_SORT_SELF:		SortEntriesBySelfTime();	break;
		}
	}
}

void ProfilerReport::EraseReport()
{
	if ( m_rootEntry != nullptr )
	{
		delete m_rootEntry;
		m_rootEntry;
	}
	m_flatViewSortedEntries.clear();
}

void ProfilerReport::SortEntriesByTotalTime()
{
	for ( std::map< std::string, ProfilerReportEntry* >::const_iterator mapIterator = m_rootEntry->m_children.begin(); mapIterator != m_rootEntry->m_children.end(); mapIterator++ )
	{
		m_flatViewSortedEntries.push_back( mapIterator->second );
	}
	std::sort( m_flatViewSortedEntries.begin(), m_flatViewSortedEntries.end(), []( const ProfilerReportEntry* a, const ProfilerReportEntry* b ) {
		return a->m_totalTimeMS > b->m_totalTimeMS;
	} );
}

void ProfilerReport::SortEntriesBySelfTime()
{
	for ( std::map< std::string, ProfilerReportEntry* >::const_iterator mapIterator = m_rootEntry->m_children.begin(); mapIterator != m_rootEntry->m_children.end(); mapIterator++ )
	{
		m_flatViewSortedEntries.push_back( mapIterator->second );
	}
	std::sort( m_flatViewSortedEntries.begin(), m_flatViewSortedEntries.end(), []( const ProfilerReportEntry* a, const ProfilerReportEntry* b ) {
		return a->m_selfTimeMS > b->m_selfTimeMS;
	} );
}

ProfilerReport::~ProfilerReport()
{
	delete m_rootEntry;
	m_rootEntry = nullptr;
}

double ProfilerReport::GetTotalTime() const
{
	return m_rootEntry->m_totalTimeMS;
}
