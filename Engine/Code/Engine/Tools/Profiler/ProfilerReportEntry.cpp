#include "Engine/Tools/Profiler/ProfilerMeasurement.hpp"
#include "Engine/Tools/Profiler/ProfilerReportEntry.hpp"

ProfilerReportEntry::ProfilerReportEntry()
{

}

ProfilerReportEntry::ProfilerReportEntry( const ProfilerReportEntry& copy )
{
	m_id = copy.m_id;
	m_callCount = copy.m_callCount;
	m_selfTimeMS = copy.m_selfTimeMS;
	m_totalTimeMS = copy.m_totalTimeMS;
	m_percentageSelfTime = copy.m_percentageSelfTime;

	for ( std::map< std::string, ProfilerReportEntry* >::const_iterator mapIterator = copy.m_children.begin(); mapIterator != copy.m_children.end(); mapIterator++ )
	{
		std::string key = mapIterator->first;
		m_children[ key ] = new ProfilerReportEntry( *( mapIterator->second ) );
	}
}

void ProfilerReportEntry::PopulateTree( const ProfilerMeasurement& measurement )
{
	AccumulateData( measurement );

	double totalChildrenTime = 0.0;
	for ( const ProfilerMeasurement* childMeasurement : measurement.m_children )
	{
		ProfilerReportEntry* childEntry = new ProfilerReportEntry();
		childEntry->PopulateTree( *childMeasurement );
		totalChildrenTime += childEntry->m_totalTimeMS;

		if ( m_children.find( childEntry->m_id ) == m_children.end() )
		{
			childEntry->m_parent = this;
			m_children[ childEntry->m_id ] = childEntry;
		}
		else // An entry with the same name already exists at this level in the hierarchy; reuse it and delete the new one
		{
			m_children[ childEntry->m_id ]->m_callCount++;
			m_children[ childEntry->m_id ]->m_selfTimeMS += childEntry->m_selfTimeMS;
			m_children[ childEntry->m_id ]->m_totalTimeMS += childEntry->m_totalTimeMS;
			m_children[ childEntry->m_id ]->m_percentageSelfTime = ( m_children[ childEntry->m_id ]->m_selfTimeMS / m_children[ childEntry->m_id ]->m_totalTimeMS ) * 100.0f;
			delete childEntry;
		}
	}

	m_selfTimeMS = m_totalTimeMS - totalChildrenTime;
	m_percentageSelfTime = ( m_selfTimeMS / m_totalTimeMS ) * 100.0f;
}

double ProfilerReportEntry::PopulateFlat( const ProfilerMeasurement& measurement )
{
	double totalChildrenTime = 0.0;
	for ( const ProfilerMeasurement* childMeasurement : measurement.m_children )
	{
		ProfilerReportEntry* childEntry = new ProfilerReportEntry();
		childEntry->AccumulateData( *childMeasurement );
		double childrenTime = PopulateFlat( *childMeasurement );
		childEntry->m_selfTimeMS = childEntry->m_totalTimeMS - childrenTime;
		childEntry->m_percentageSelfTime = ( childEntry->m_selfTimeMS / childEntry->m_totalTimeMS ) * 100.0f;

		totalChildrenTime += childEntry->m_totalTimeMS;

		// Populate in the root's child list
		if ( m_children.find( childEntry->m_id ) == m_children.end() )
		{
			childEntry->m_parent = this;
			m_children[ childEntry->m_id ] = childEntry;
		}
		else // An entry with the same name already exists at this level in the hierarchy; reuse it and delete the new one
		{
			m_children[ childEntry->m_id ]->m_callCount++;
			m_children[ childEntry->m_id ]->m_selfTimeMS += childEntry->m_selfTimeMS;
			m_children[ childEntry->m_id ]->m_totalTimeMS += childEntry->m_totalTimeMS;
			m_children[ childEntry->m_id ]->m_percentageSelfTime = ( m_children[ childEntry->m_id ]->m_selfTimeMS / m_children[ childEntry->m_id ]->m_totalTimeMS ) * 100.0f;
			delete childEntry;
		}
	}
	return totalChildrenTime;
}

void ProfilerReportEntry::AccumulateData( const ProfilerMeasurement& measurement )
{
	m_id = std::string( measurement.m_id ).c_str();
	m_totalTimeMS = measurement.GetElapsedTime() * 1000.0f;
}

void ProfilerReportEntry::AggregateEntry( ProfilerReportEntry& otherEntry )
{
	for ( std::map< std::string, ProfilerReportEntry* >::iterator mapIterator = otherEntry.m_children.begin(); mapIterator != otherEntry.m_children.end(); mapIterator++ )
	{
		std::string key = mapIterator->first;
		ProfilerReportEntry* otherEntryChild = otherEntry.m_children[ key ];
		if ( m_children.find( key ) != m_children.end() )
		{
			m_children[ key ]->AggregateEntry( *otherEntryChild );
		}
		else
		{
			m_children[ key ] = new ProfilerReportEntry( *otherEntryChild );
		}
	}

	m_callCount += otherEntry.m_callCount;
	m_selfTimeMS += otherEntry.m_selfTimeMS;
	m_totalTimeMS += otherEntry.m_totalTimeMS;
	m_percentageSelfTime = ( m_selfTimeMS / m_totalTimeMS ) * 100.0f;
}

bool ProfilerReportEntry::CanBeCollapsed() const
{
	return ( m_children.size() > 0 );
}

ProfilerReportEntry::~ProfilerReportEntry()
{
	for ( std::map< std::string, ProfilerReportEntry* >::iterator mapIterator = m_children.begin(); mapIterator != m_children.end(); mapIterator++ )
	{
		delete mapIterator->second;
		mapIterator->second = nullptr;
	}
}
