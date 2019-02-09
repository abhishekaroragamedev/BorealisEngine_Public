#pragma once

#ifndef DISABLE_PROFILER

#define PROFILER_HISTORY_MAX_FRAMES 512

//constexpr char REPORT_LINE_FORMAT[] = "%-72s %-8s %-8s %-16s %-8s %-16s";

#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Tools/Profiler/ProfilerReport.hpp"

class Camera;
class Command;
class ProfilerMeasurement;
class ProfilerReport;
class ProfilerReportEntry;

class Profiler
{

public:
	Profiler();
	~Profiler();

public:
	void RenderView();

	void Push( const char* id );
	ProfilerMeasurement* Pop();
	void MarkFrame();
	void Pause();
	void Resume();
	void SetVisibility( bool isVisible );
	void ToggleVisibility();
	void ToggleReportView();
	void ToggleSortMode();
	void ToggleCollapsedNode( const ProfilerReportEntry& reportEntry );
	void RegenerateReport();
	void HandleMouseClickAtPosition( const Vector2& mousePosition );	// Mouse origin - top-left, range- 0,0->screenWidth,screenHeight
	void HandleMouseDownAtPosition( const Vector2& mousePosition );	// Mouse origin - top-left, range- 0,0->screenWidth,screenHeight
	void DeselectFrame();

	void ConsolePrintfReport( ProfilerReportView view, ProfilerReportSortMode sortMode = ProfilerReportSortMode::PROFILER_REPORT_SORT_TOTAL ) const;
	void ConsolePrintfReportLine( const ProfilerReport& report, const ProfilerReportEntry& reportEntry, int depth, ProfilerReportView view, ProfilerReportSortMode sortMode = ProfilerReportSortMode::PROFILER_REPORT_SORT_TOTAL ) const;

	bool IsVisible() const;
	bool IsRunning() const;
	int GetPreviousFrames( int start, int end, ProfilerMeasurement*** out_frames ) const;	// start is most recent previous frame required, end is oldest frame required
	double GetWorstFrameTimeSeconds() const;
	int GetOldestFrameIndex() const;
	int GetNewestFrameIndex() const;
	int GetSelectedFrameIndex() const;

	static void Render();
	static void EndFrame();
	static void StartProfile( const char* id );
	static void EndProfile();
	static Profiler* GetInstance();

private:
	void InitializeFrameStacks();
	void InitializeCameraAndScaling();

	void InitializeFrameRender() const;
	void RenderBackground() const;
	void RenderFPSAndFrameTime() const;
	void RenderMouseToggleMessage() const;
	void RenderGraphForHistory() const;
	void RenderPauseOrResumeButton() const;
	void RenderReportForSelectedFrame() const;
	int PopulateReportEntries( const ProfilerReport& report, ProfilerReportEntry& reportEntry, int depth, int lineNumber ) const;
	void RenderReportLine( const ProfilerReport& report, const ProfilerReportEntry& reportEntry ) const;

	void HandleGraphMouseClick( const Vector2& mousePositionNDC );
	void HandleButtonMouseClick( const Vector2& mousePositionNDC );
	void HandleReportMouseClick( const Vector2& mousePositionNDC );
	void HandleReportEntryMouseClick( const Vector2& mousePositionNDC, const ProfilerReport& report, ProfilerReportEntry& reportEntry );

	void DeleteFrameStacks();

	Vector2 GetScaledVector2( float x, float y ) const;
	float GetGraphXPositionForFrameIndex( int frameIndex ) const;
	AABB2 GetGraphBounds() const;
	AABB2 GetButtonBounds() const;
	AABB2 GetReportEntryBounds( int lineNumber ) const;
	Vector2 GetReportStartMins() const;
	Vector2 GetReportLineSpacing() const;
	float GetReportFontHeight() const;
	Rgba GetColorForGraphPosition( const Vector2& position ) const;	// Re-maps the graph position to a frame time using the worst frame time and graph dimensions, and assigns a color to it
	Rgba GetColorForFPS( float fps ) const;
	std::string GetIdForReportEntry( const ProfilerReportEntry& entry ) const;	// Only for Tree view
	std::string GetPrettyTimeString( double milliseconds ) const;
	bool IsNodeCollapsed( const ProfilerReportEntry& entry ) const;

private:
	ProfilerMeasurement* m_currentFrameStack = nullptr;
	ProfilerMeasurement* m_pastFrameStacks[ PROFILER_HISTORY_MAX_FRAMES ];
	int m_pastFrameStacksBackIndex = 0;	// Point at which the next entry will be added

	ProfilerReportView m_reportView = ProfilerReportView::PROFILER_REPORT_TREE;
	ProfilerReportSortMode m_sortMode = ProfilerReportSortMode::PROFILER_REPORT_SORT_TOTAL;
	int m_graphSelectedFrameIndex = -1;
	int m_graphSelectedFrameDisplacement = 0;
	bool m_isVisible = false;
	bool m_isCapturingFrames = true;
	bool m_pauseThisFrame = false;
	ProfilerReport m_currentReport;
	std::vector< std::string > m_collapsedNodesByID;

	Camera* m_camera = nullptr;

	static Vector2 s_dimensionScaling;
	static Rgba s_backgroundColor;

};

void ProfilerStartup();
void ProfilerShutdown();

bool ProfilerCommand( Command& command );
bool ProfilerPauseCommand( Command& command );
bool ProfilerResumeCommand( Command& command );
bool ProfilerReportCommand( Command& command );

#endif