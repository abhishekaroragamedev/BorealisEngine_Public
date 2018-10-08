#pragma once

class ProfilerMeasurement;
class ProfilerReportEntry;

enum ProfilerReportView
{
	PROFILER_REPORT_TREE,
	PROFILER_REPORT_FLAT
};

enum ProfilerReportSortMode
{
	PROFILER_REPORT_SORT_INVALID = -1,
	PROFILER_REPORT_SORT_SELF,
	PROFILER_REPORT_SORT_TOTAL
};

class ProfilerReport
{

	friend class Profiler;

private:
	ProfilerReport();
	~ProfilerReport();

	void GenerateTreeReport( const ProfilerMeasurement& frameMeasurement );
	void GenerateFlatReport( const ProfilerMeasurement& frameMeasurement, ProfilerReportSortMode sortMode );
	void AggregateReport( ProfilerReport& report, ProfilerReportSortMode sortMode = ProfilerReportSortMode::PROFILER_REPORT_SORT_INVALID );			// Assumes both reports have the same nodes
	void EraseReport();

	void SortEntriesByTotalTime();
	void SortEntriesBySelfTime();

	double GetTotalTime() const;

private:
	ProfilerReportEntry* m_rootEntry = nullptr;
	std::vector< ProfilerReportEntry* > m_flatViewSortedEntries;

};
