#pragma once

#include <map>
#include <string>

class ProfilerMeasurement;

class ProfilerReportEntry
{

	friend class Profiler;
	friend class ProfilerReport;

private:
	ProfilerReportEntry();
	ProfilerReportEntry( const ProfilerReportEntry& copy );
	~ProfilerReportEntry();

void PopulateTree( const ProfilerMeasurement& measurement );
double PopulateFlat( const ProfilerMeasurement& measurement );
void AccumulateData( const ProfilerMeasurement& measurement );
void AggregateEntry( ProfilerReportEntry& otherEntry );

bool CanBeCollapsed() const;

private:
	std::string m_id = "";
	unsigned int m_callCount = 1U;
	double m_totalTimeMS = 0.0;
	double m_selfTimeMS = 0.0;
	double m_percentageSelfTime = 0.0;

	ProfilerReportEntry* m_parent = nullptr;
	std::map< std::string, ProfilerReportEntry* > m_children;

	// Tree properties
	int m_lineNumber = -1;
	int m_depth = -1;

};
