#include "Engine/Core/Clock.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Window.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/OculusRift/OVRContext.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Tools/Command.hpp"
#include "Engine/Tools/DevConsole.hpp"
#include "Engine/Tools/Profiler/ProfileScope.hpp"
#include "Engine/Tools/Profiler/Profiler.hpp"
#include "Engine/Tools/Profiler/ProfilerMeasurement.hpp"
#include "Engine/Tools/Profiler/ProfilerReportEntry.hpp"

#ifndef DISABLE_PROFILER

/* DEFINITIONS */

Profiler* g_profiler = nullptr;

/* static */ Vector2 Profiler::s_dimensionScaling = Vector2( 1.0f, 1.0f );
/* static */ Rgba Profiler::s_backgroundColor = Rgba( 0, 0, 0, 120 );

/* INITIALIZATION */

Profiler::Profiler()
{
	InitializeFrameStacks();

	CommandRegister( "profiler", ProfilerCommand, "Displays profiling data as an overlay to the game." );
	CommandRegister( "profiler_pause", ProfilerPauseCommand, "Configures the profiler to not capture new frame data, preserving old frame data." );
	CommandRegister( "profiler_resume", ProfilerResumeCommand, "Allows the profiler to resume capturing new frame data and delete old frame data." );
	CommandRegister( "profiler_report", ProfilerReportCommand, "Prints a Tree or Flat report. Flat reports can be sorted by either Total or Self time." );

	g_profiler = this;
}

void Profiler::InitializeFrameStacks()
{
	m_currentFrameStack = nullptr;
	for ( int frameIndex = 0; frameIndex < PROFILER_HISTORY_MAX_FRAMES; frameIndex++ )
	{
		m_pastFrameStacks[ frameIndex ] = nullptr;
	}
}

void Profiler::InitializeCameraAndScaling()
{
	Texture* renderTarget = Renderer::GetInstance()->GetDefaultColorTarget();
	m_camera = new Camera( renderTarget );
	m_camera->SetProjectionOrtho( 2.0f, Window::GetAspect(), -1.0f, 1.0f );
	s_dimensionScaling = Vector2( Window::GetAspect(), 1.0f );
}

/* DESTRUCTION */

Profiler::~Profiler()
{
	CommandUnregister( "profiler" );
	CommandUnregister( "profiler_pause" );
	CommandUnregister( "profiler_resume" );
	CommandUnregister( "profiler_report" );

	delete m_camera;
	m_camera = nullptr;

	DeleteFrameStacks();
}

void Profiler::DeleteFrameStacks()
{
	if ( m_currentFrameStack != nullptr )
	{
		delete m_currentFrameStack;
		m_currentFrameStack = nullptr;
	}

	for ( int frameIndex = 0; frameIndex < PROFILER_HISTORY_MAX_FRAMES; frameIndex++ )
	{
		if ( m_pastFrameStacks[ frameIndex ] != nullptr )
		{
			delete m_pastFrameStacks[ frameIndex ];
			m_pastFrameStacks[ frameIndex ] = nullptr;
		}
	}
}

/* SETTERS */

void Profiler::Push( const char* id )
{
	if ( IsRunning() )
	{
		if ( m_currentFrameStack == nullptr )
		{
			ProfilerMeasurement* rootMeasurement = new ProfilerMeasurement( "Frame" );
			m_currentFrameStack = rootMeasurement;
		}
		ProfilerMeasurement* measurement = new ProfilerMeasurement( id );
		m_currentFrameStack->AddChild( measurement );
		m_currentFrameStack = measurement;
	}
}

ProfilerMeasurement* Profiler::Pop()
{
	if ( IsRunning() )
	{
		ASSERT_RECOVERABLE( ( m_currentFrameStack != nullptr ), "WARNING: Profiler::Pop() - Pop cannot be called when stack is empty." );
		m_currentFrameStack->Finish();
		ProfilerMeasurement* poppedMeasurement = m_currentFrameStack;
		m_currentFrameStack = m_currentFrameStack->m_parent;
		return poppedMeasurement;
	}
	return nullptr;
}

void Profiler::MarkFrame()
{
	if ( IsRunning() )
	{
		if ( m_currentFrameStack != nullptr )
		{
			if ( m_pastFrameStacks[ m_pastFrameStacksBackIndex ] != nullptr )
			{
				delete m_pastFrameStacks[ m_pastFrameStacksBackIndex ];
			}
			m_pastFrameStacks[ m_pastFrameStacksBackIndex ] = m_currentFrameStack;
			m_pastFrameStacksBackIndex = ( m_pastFrameStacksBackIndex + 1 ) % PROFILER_HISTORY_MAX_FRAMES;
			Pop();

			RegenerateReport();

			ASSERT_OR_DIE( ( m_currentFrameStack == nullptr ), "ERROR: Profiler::MarkFrame() - Stack not empty at end of frame. Aborting..." );
		}
		if ( m_pauseThisFrame )
		{
			m_isCapturingFrames = false;
		}
	}
	else if ( !m_pauseThisFrame )
	{
		m_isCapturingFrames = true;
	}
}

void Profiler::Pause()
{
	m_pauseThisFrame = true;
}

void Profiler::Resume()
{
	m_pauseThisFrame = false;
}

void Profiler::SetVisibility( bool isVisible )
{
	m_isVisible = isVisible;
}

void Profiler::ToggleVisibility()
{
	m_isVisible = !m_isVisible;
}

void Profiler::ToggleReportView()
{
	switch( m_reportView )
	{
		case ProfilerReportView::PROFILER_REPORT_TREE	:	m_reportView = ProfilerReportView::PROFILER_REPORT_FLAT;	break;
		case ProfilerReportView::PROFILER_REPORT_FLAT	:	m_reportView = ProfilerReportView::PROFILER_REPORT_TREE;	break;
	}

	RegenerateReport();
}

void Profiler::ToggleSortMode()
{
	switch( m_sortMode )
	{
		case ProfilerReportSortMode::PROFILER_REPORT_SORT_TOTAL	:	m_sortMode = ProfilerReportSortMode::PROFILER_REPORT_SORT_SELF;		break;
		case ProfilerReportSortMode::PROFILER_REPORT_SORT_SELF	:	m_sortMode = ProfilerReportSortMode::PROFILER_REPORT_SORT_TOTAL;	break;
	}
}

void Profiler::ToggleCollapsedNode( const ProfilerReportEntry& reportEntry )
{
	std::string id = GetIdForReportEntry( reportEntry );
	std::vector< std::string >::iterator found = std::find( m_collapsedNodesByID.begin(), m_collapsedNodesByID.end(), id );
	if ( found == m_collapsedNodesByID.end() )
	{
		// Collapse
		m_collapsedNodesByID.push_back( id );
	}
	else
	{
		// De-collapse
		m_collapsedNodesByID.erase( found );
	}
}

void Profiler::RegenerateReport()
{
	m_currentReport.EraseReport();
	
	int frameIndex = GetNewestFrameIndex();
	if ( m_graphSelectedFrameIndex != -1 )
	{
		frameIndex = GetSelectedFrameIndex();
	}

	switch ( m_reportView )
	{
		case ProfilerReportView::PROFILER_REPORT_TREE	:	m_currentReport.GenerateTreeReport( *m_pastFrameStacks[ frameIndex ] );	break;
		case ProfilerReportView::PROFILER_REPORT_FLAT	:	m_currentReport.GenerateFlatReport( *m_pastFrameStacks[ frameIndex ], m_sortMode );	break;
	}

	int displacement = m_graphSelectedFrameDisplacement;
	int displacementSignNegation = -Sign( displacement );
	while ( displacement != 0 )
	{
		int displacedFrameIndex = frameIndex + displacement;
		if ( displacedFrameIndex < 0 && ( displacementSignNegation > 0 ) )
		{
			displacedFrameIndex = PROFILER_HISTORY_MAX_FRAMES + displacedFrameIndex;
		}
		else if ( displacedFrameIndex >= PROFILER_HISTORY_MAX_FRAMES && ( displacementSignNegation < 0 ) )
		{
			displacedFrameIndex = displacedFrameIndex % PROFILER_HISTORY_MAX_FRAMES;
		}

		ProfilerReport selectedReport;
		switch ( m_reportView )
		{
			case ProfilerReportView::PROFILER_REPORT_TREE	:
			{
				selectedReport.GenerateTreeReport( *m_pastFrameStacks[ displacedFrameIndex ] );
				m_currentReport.AggregateReport( selectedReport );
				break;
			}
			case ProfilerReportView::PROFILER_REPORT_FLAT	:
			{
				selectedReport.GenerateFlatReport( *m_pastFrameStacks[ displacedFrameIndex ], m_sortMode );
				m_currentReport.AggregateReport( selectedReport, m_sortMode );
				break;
			}
		}
		displacement += displacementSignNegation;
	}
}

/* INPUT */

void Profiler::HandleMouseDownAtPosition( const Vector2& mousePosition )
{
	Vector2 mousePositionNDC = Vector2(
		RangeMapFloat( mousePosition.x, 0.0f, Window::GetWidthF(), -1.0f, 1.0f ),
		RangeMapFloat( mousePosition.y, 0.0f, Window::GetHeightF(), 1.0f, -1.0f )
	);

	AABB2 graphBounds = GetGraphBounds();
	graphBounds.mins /= s_dimensionScaling;
	graphBounds.maxs /= s_dimensionScaling;
	if ( graphBounds.IsPointInside( mousePositionNDC ) )
	{
		float selectedIndexFloat = RangeMapFloat( mousePositionNDC.x, graphBounds.mins.x, graphBounds.maxs.x, 0.0f, static_cast< float >( PROFILER_HISTORY_MAX_FRAMES - 1 ) );
		int selectedIndex = static_cast< int >( selectedIndexFloat );
		selectedIndex = ClampInt( selectedIndex, 0, ( PROFILER_HISTORY_MAX_FRAMES - 1 ) );
		
		int newDisplacement = selectedIndex - m_graphSelectedFrameIndex;
		if ( newDisplacement != m_graphSelectedFrameDisplacement )
		{
			m_graphSelectedFrameDisplacement = newDisplacement;
		}

		RegenerateReport();
	}
}

void Profiler::HandleMouseClickAtPosition( const Vector2& mousePosition )
{
	Vector2 mousePositionNDC = Vector2(
		RangeMapFloat( mousePosition.x, 0.0f, Window::GetWidthF(), -1.0f, 1.0f ),
		RangeMapFloat( mousePosition.y, 0.0f, Window::GetHeightF(), 1.0f, -1.0f )
	);

	HandleGraphMouseClick( mousePositionNDC );
	HandleButtonMouseClick( mousePositionNDC );
	HandleReportMouseClick( mousePositionNDC );
}

void Profiler::HandleGraphMouseClick( const Vector2& mousePositionNDC )
{
	AABB2 graphBounds = GetGraphBounds();
	graphBounds.mins /= s_dimensionScaling;
	graphBounds.maxs /= s_dimensionScaling;
	if ( graphBounds.IsPointInside( mousePositionNDC ) )
	{
		float selectedIndexFloat = RangeMapFloat( mousePositionNDC.x, graphBounds.mins.x, graphBounds.maxs.x, 0.0f, static_cast< float >( PROFILER_HISTORY_MAX_FRAMES - 1 ) );
		int selectedIndex = static_cast< int >( selectedIndexFloat );
		selectedIndex = ClampInt( selectedIndex, 0, ( PROFILER_HISTORY_MAX_FRAMES - 1 ) );
		m_graphSelectedFrameIndex = selectedIndex;
		m_graphSelectedFrameDisplacement = 0;
		RegenerateReport();
	}
}

void Profiler::HandleButtonMouseClick( const Vector2& mousePositionNDC )
{
	AABB2 buttonBounds = GetButtonBounds();
	buttonBounds.mins /= s_dimensionScaling;
	buttonBounds.maxs /= s_dimensionScaling;
	if ( buttonBounds.IsPointInside( mousePositionNDC ) )
	{
		if ( IsRunning() )
		{
			Pause();
		}
		else
		{
			Resume();
		}
	}
}

void Profiler::HandleReportMouseClick( const Vector2& mousePositionNDC )
{
	PopulateReportEntries( m_currentReport, *m_currentReport.m_rootEntry, 0, 0 );

	if ( m_reportView == ProfilerReportView::PROFILER_REPORT_TREE )
	{
		HandleReportEntryMouseClick( mousePositionNDC, m_currentReport, *m_currentReport.m_rootEntry );
	}
}

void Profiler::HandleReportEntryMouseClick( const Vector2& mousePositionNDC, const ProfilerReport& report, ProfilerReportEntry& reportEntry )
{
	if ( reportEntry.CanBeCollapsed() )
	{
		AABB2 entryBounds = GetReportEntryBounds( reportEntry.m_lineNumber );
		entryBounds.mins /= s_dimensionScaling;
		entryBounds.maxs /= s_dimensionScaling;
		if ( entryBounds.IsPointInside( mousePositionNDC ) )
		{
			ToggleCollapsedNode( reportEntry );
		}
	}
	if ( !IsNodeCollapsed( reportEntry ) )
	{
		for ( std::map< std::string, ProfilerReportEntry* >::iterator mapIterator = reportEntry.m_children.begin(); mapIterator != reportEntry.m_children.end(); mapIterator++ )
		{
			HandleReportEntryMouseClick( mousePositionNDC, report, *mapIterator->second );
		}
	}
}

void Profiler::DeselectFrame()
{
	m_graphSelectedFrameIndex = -1;
}

/* RENDERING */

void Profiler::RenderView()
{
	PROFILE_SCOPE( "Profiler::RenderView()" );
	if ( m_camera == nullptr )
	{
		InitializeCameraAndScaling();	// Lazy instantiation
	}
	if ( IsVisible() )
	{
		InitializeFrameRender();
		RenderBackground();
		RenderFPSAndFrameTime();
		RenderMouseToggleMessage();
		RenderGraphForHistory();
		RenderReportForSelectedFrame();
		RenderPauseOrResumeButton();
	}
}

void Profiler::InitializeFrameRender() const
{
	Renderer::GetInstance()->UseShaderProgram( DEFAULT_SHADER_NAME );
	Renderer::GetInstance()->BindModelMatrixToCurrentShader( Matrix44::IDENTITY );
	Renderer::GetInstance()->EnableDepth( DepthTestCompare::COMPARE_ALWAYS, false );
	Renderer::GetInstance()->SetCamera( m_camera );
}

void Profiler::RenderBackground() const
{
	AABB2 bounds = AABB2( GetScaledVector2( -1.0f, -1.0f ), GetScaledVector2( 1.0f, 1.0f ) );
	Renderer::GetInstance()->DrawAABB( bounds, s_backgroundColor );
}

void Profiler::RenderFPSAndFrameTime() const
{
	int frameIndex = ( m_graphSelectedFrameIndex == -1 )? GetNewestFrameIndex() : GetSelectedFrameIndex();

	float frameTimeSeconds = static_cast< float >( m_pastFrameStacks[ frameIndex ]->GetElapsedTime() );
	float fps = ( IsFloatEqualTo( frameTimeSeconds, 0.0f ) )? 0.0f : ( 1.0f / frameTimeSeconds );

	std::string fpsText = Stringf( "FPS: %.1f", fps );
	Renderer::GetInstance()->DrawText2D(
		GetScaledVector2( -0.95f, 0.925f ),
		fpsText,
		0.025f,
		Rgba::WHITE,
		1.0f,
		nullptr
	);

	float frameTimeMilliseconds = frameTimeSeconds * 1000.0f;
	std::string frameTimeText = Stringf( "Frame time: %.2f ms", frameTimeMilliseconds );
	Renderer::GetInstance()->DrawText2D(
		GetScaledVector2( -0.95f, 0.875f ),
		frameTimeText,
		0.025f,
		Rgba::WHITE,
		1.0f,
		nullptr
	);
}

void Profiler::RenderMouseToggleMessage() const
{
	std::string mouseToggleText = "[M] to toggle mouse visibility";
	Renderer::GetInstance()->DrawText2D(
		GetScaledVector2( 0.325f, 0.875f ),
		mouseToggleText,
		0.025f,
		Rgba::WHITE,
		1.0f,
		nullptr
	);
}

void Profiler::RenderGraphForHistory() const
{
	AABB2 graphBounds = GetGraphBounds();
	Vector2 graphMins = graphBounds.mins;
	Vector2 graphMaxs = graphBounds.maxs;
	AABB2 background = AABB2( graphMins, graphMaxs );
	Renderer::GetInstance()->DrawAABB(
		background,
		Rgba( 0, 0, 125, 120 )
	);

	float pointWidth = ( graphMaxs.x - graphMins.x ) / static_cast< float >( PROFILER_HISTORY_MAX_FRAMES - 1 );

	Vector2 points[ PROFILER_HISTORY_MAX_FRAMES ];
	for ( int pointIndex = 0; pointIndex < PROFILER_HISTORY_MAX_FRAMES; pointIndex++ )
	{
		points[ pointIndex ] = Vector2(
			( graphMins.x + ( static_cast< float >( pointIndex ) * pointWidth ) ),
			graphMins.y
		);
	}

	ProfilerMeasurement** frames = nullptr;
	int numAvailableFrames = GetPreviousFrames( 0, PROFILER_HISTORY_MAX_FRAMES, &frames );
	float worstFrameTimeSeconds = static_cast< float >( GetWorstFrameTimeSeconds() );
	for ( int frameIndex = 1; frameIndex <= numAvailableFrames; frameIndex++ )
	{
		float frameTimeSeconds = static_cast< float >( frames[ numAvailableFrames - frameIndex ]->GetElapsedTime() );

		Vector2 currentPoint;
		currentPoint.x = graphMins.x + ( static_cast< float >( PROFILER_HISTORY_MAX_FRAMES - frameIndex ) * pointWidth );
		currentPoint.y = RangeMapFloat( frameTimeSeconds, 0.0f, worstFrameTimeSeconds, graphMins.y, graphMaxs.y );
		points[ PROFILER_HISTORY_MAX_FRAMES - frameIndex ] = currentPoint;
	}
	free( frames );

	MeshBuilder graphLineBuilder;
	graphLineBuilder.Begin( DrawPrimitiveType::LINES, false );
	graphLineBuilder.SetColor( Rgba::BLACK.GetWithAlpha( 0.5f ) );
	Vector2 previousPoint = points[ 0 ];
	for ( int pointIndex = 1; pointIndex < PROFILER_HISTORY_MAX_FRAMES; pointIndex++ )
	{
		Vector2 currentPoint = points[ pointIndex ];

		graphLineBuilder.PushVertex( previousPoint.x, previousPoint.y, 0.0f );
		graphLineBuilder.PushVertex( currentPoint.x, currentPoint.y, 0.0f );

		previousPoint = currentPoint;
	}
	graphLineBuilder.End();
	
	Mesh* graphLineMesh = graphLineBuilder.CreateMesh();
	Material* uiMaterial = Renderer::GetInstance()->CreateOrGetMaterial( "UI" );
	Renderer::GetInstance()->DrawMeshWithMaterial( graphLineMesh, *uiMaterial );
	delete graphLineMesh;

	MeshBuilder graphPolyBuilder;
	graphPolyBuilder.Begin( DrawPrimitiveType::TRIANGLES );

	previousPoint = points[ 0 ];
	Vector2 previousPointProjected = Vector2( previousPoint.x, graphMins.y );
	graphPolyBuilder.SetColor( GetColorForGraphPosition( previousPoint ) );
	graphPolyBuilder.PushVertex( previousPointProjected.x, previousPointProjected.y, 0.0f );
	graphPolyBuilder.PushVertex( previousPoint.x, previousPoint.y, 0.0f );
	for ( int pointIndex = 1; pointIndex < PROFILER_HISTORY_MAX_FRAMES; pointIndex++ )

	{
		previousPointProjected = Vector2( previousPoint.x, graphMins.y );

		Vector2 currentPoint = points[ pointIndex ];
		Vector2 currentPointProjected = Vector2( currentPoint.x, graphMins.y );
		graphPolyBuilder.SetColor( GetColorForGraphPosition( currentPoint ) );

		graphPolyBuilder.PushVertex( currentPointProjected.x, currentPointProjected.y, 0.0f );
		graphPolyBuilder.PushVertex( currentPoint.x, currentPoint.y, 0.0f );

		graphPolyBuilder.PushIndex( 2 * ( pointIndex - 1 ) );
		graphPolyBuilder.PushIndex( 2 * pointIndex );
		graphPolyBuilder.PushIndex( ( 2 * ( pointIndex - 1 ) ) + 1 );
		graphPolyBuilder.PushIndex( ( 2 * ( pointIndex - 1 ) ) + 1 );
		graphPolyBuilder.PushIndex( 2 * pointIndex );
		graphPolyBuilder.PushIndex( ( 2 * pointIndex ) + 1 );
		previousPoint = currentPoint;
	}
	graphPolyBuilder.End();

	Mesh* graphPolyMesh = graphPolyBuilder.CreateMesh();
	Renderer::GetInstance()->DrawMeshWithMaterial( graphPolyMesh, *uiMaterial );
	delete graphPolyMesh;

	float worstFrameTimeMilliseconds = worstFrameTimeSeconds * 1000.0f;
	Renderer::GetInstance()->DrawText2D(
		( graphMaxs + Vector2( 0.025f, -0.025f ) ),
		Stringf( "%.2f ms", worstFrameTimeMilliseconds ),
		0.025f,
		Rgba::WHITE,
		1.0f,
		nullptr
	);

	if ( m_graphSelectedFrameIndex != -1 )
	{
		float selectedPositionX = GetGraphXPositionForFrameIndex( m_graphSelectedFrameIndex );

		if ( m_graphSelectedFrameDisplacement == 0 )
		{
			Renderer::GetInstance()->DrawLine(
				Vector2( selectedPositionX, graphMins.y ),
				Vector2( selectedPositionX, graphMaxs.y ),
				Rgba::WHITE,
				Rgba::WHITE,
				0.01f
			);
		}
		else
		{
			float selectedRangeEndX = GetGraphXPositionForFrameIndex( m_graphSelectedFrameIndex + m_graphSelectedFrameDisplacement );

			AABB2 selectionBounds = AABB2(
				Vector2( Min( selectedPositionX, selectedRangeEndX ), graphMins.y ),
				Vector2( Max( selectedPositionX, selectedRangeEndX ), graphMaxs.y )
			);

			Renderer::GetInstance()->DrawAABB(
				selectionBounds,
				Rgba::WHITE.GetWithAlpha( 0.5f )
			);
		}
	}
}

void Profiler::RenderPauseOrResumeButton() const
{
	AABB2 bounds = GetButtonBounds();
	Renderer::GetInstance()->DrawAABB( bounds, s_backgroundColor );

	bounds.AddPaddingToSides( -0.015f, -0.015f );
	Vector2 dimensions = bounds.GetDimensions();

	MeshBuilder buttonBuilder;
	buttonBuilder.Begin( DrawPrimitiveType::TRIANGLES );
	buttonBuilder.SetColor( Rgba::WHITE );

	if ( IsRunning() )
	{
		// Pause button
		buttonBuilder.PushVertex( Vector3( bounds.mins.x, bounds.mins.y, 0.0f ) );
		buttonBuilder.PushVertex( Vector3( ( bounds.mins.x + ( 0.40f * dimensions.x ) ), bounds.mins.y, 0.0f ) );
		buttonBuilder.PushVertex( Vector3( bounds.mins.x, bounds.maxs.y, 0.0f ) );
		buttonBuilder.PushVertex( Vector3( ( bounds.mins.x + ( 0.40f * dimensions.x ) ), bounds.maxs.y, 0.0f ) );
		buttonBuilder.PushVertex( Vector3( ( bounds.maxs.x - ( 0.40f * dimensions.x ) ), bounds.mins.y, 0.0f ) );
		buttonBuilder.PushVertex( Vector3( bounds.maxs.x, bounds.mins.y, 0.0f ) );
		buttonBuilder.PushVertex( Vector3( ( bounds.maxs.x - ( 0.40f * dimensions.x ) ), bounds.maxs.y, 0.0f ) );
		buttonBuilder.PushVertex( Vector3( bounds.maxs.x, bounds.maxs.y, 0.0f ) );

		unsigned int indices[ 12 ] = {
			0, 1, 2,
			2, 1, 3,
			4, 5, 6,
			6, 5, 7
		};
		buttonBuilder.PushIndices( 12, indices );
	}
	else
	{
		// Resume button
		buttonBuilder.PushVertex( Vector3( ( bounds.mins.x + 0.01f ), bounds.mins.y, 0.0f ) );
		buttonBuilder.PushVertex( Vector3( ( bounds.maxs.x - 0.01f ), ( bounds.mins.y + ( 0.5f * dimensions.y ) ), 0.0f ) );
		buttonBuilder.PushVertex( Vector3( ( bounds.mins.x + 0.01f ), bounds.maxs.y, 0.0f ) );


		unsigned int indices[ 3 ] = {
			0, 1, 2
		};
		buttonBuilder.PushIndices( 3, indices );
	}

	buttonBuilder.End();
	Mesh* buttonMesh = buttonBuilder.CreateMesh();
	Material* buttonMaterial = Renderer::GetInstance()->CreateOrGetMaterial( "UI" );
	Renderer::GetInstance()->DrawMeshWithMaterial( buttonMesh, *buttonMaterial );
	delete buttonMesh;
}

void Profiler::RenderReportForSelectedFrame() const
{
	std::string reportTitleText = "Frame Report: ";
	reportTitleText += ( m_reportView == ProfilerReportView::PROFILER_REPORT_TREE )? "{TREE}" : "{FLAT}";
	reportTitleText += " [V] to toggle";
	if ( m_reportView == ProfilerReportView::PROFILER_REPORT_FLAT )
	{
		reportTitleText += " Sorting ";
		reportTitleText += ( m_sortMode == ProfilerReportSortMode::PROFILER_REPORT_SORT_TOTAL )? "{TOTAL}" : "{SELF}";
		reportTitleText += " [L] to toggle";
	}

	Renderer::GetInstance()->DrawText2D(
		GetScaledVector2( -0.95f, 0.575f ),
		reportTitleText,
		0.025f,
		Rgba::WHITE,
		1.0f,
		nullptr
	);
	std::string lineText = "------------";
	Renderer::GetInstance()->DrawText2D(
		GetScaledVector2( -0.95f, 0.55f ),
		lineText,
		0.025f,
		Rgba::WHITE,
		1.0f,
		nullptr
	);

	std::string reportTabs = Stringf(
		"%-72s%8s%16s%8s%16s%8s",
		"FUNCTION NAME\0",
		"CALLS\0",
		"TOTAL(TIME)\0",
		"TOTAL(%)\0",
		"SELF(TIME)\0",
		"SELF(%)\0"
	);
	Renderer::GetInstance()->DrawText2D(
		GetScaledVector2( -0.95f, 0.5f ),
		reportTabs,
		0.025f,
		Rgba::WHITE,
		1.0f,
		nullptr
	);

	int frameIndex = ( m_graphSelectedFrameIndex == -1 )? GetNewestFrameIndex() : GetSelectedFrameIndex();
	if ( m_pastFrameStacks[ frameIndex ] == nullptr )
	{
		std::string emptyText = "NO REPORTS AVAILABLE";
		Renderer::GetInstance()->DrawText2D(
			GetScaledVector2( -0.95f, 0.5f ),
			emptyText,
			0.025f,
			Rgba::RED,
			1.0f,
			nullptr
		);
		return;
	}

	PopulateReportEntries( m_currentReport, *m_currentReport.m_rootEntry, 0, 0 );
	RenderReportLine( m_currentReport, *m_currentReport.m_rootEntry );
}

int Profiler::PopulateReportEntries( const ProfilerReport& report, ProfilerReportEntry& reportEntry, int depth, int lineNumber ) const
{
	int cumulativeLineNumber = lineNumber;
	reportEntry.m_depth = depth;
	reportEntry.m_lineNumber = cumulativeLineNumber;

	if ( m_reportView == ProfilerReportView::PROFILER_REPORT_TREE && !IsNodeCollapsed( reportEntry ) )
	{
		for ( std::map< std::string, ProfilerReportEntry* >::const_iterator mapIterator = reportEntry.m_children.begin(); mapIterator != reportEntry.m_children.end(); mapIterator++ )
		{
			ProfilerReportEntry* entry = mapIterator->second;
			cumulativeLineNumber = PopulateReportEntries( report, *entry, ( depth + 1 ), ( cumulativeLineNumber + 1 ) );
		}
	}
	else if ( &reportEntry == report.m_rootEntry ) // Process only root entry for Flat view
	{
		for ( std::vector< ProfilerReportEntry* >::const_iterator vecIterator = report.m_flatViewSortedEntries.begin(); vecIterator != report.m_flatViewSortedEntries.end(); vecIterator++ )
		{
			ProfilerReportEntry* entry = *vecIterator;
			cumulativeLineNumber = PopulateReportEntries( report, *entry, ( depth + 1 ), ( cumulativeLineNumber + 1 ) );
		}
	}

	return cumulativeLineNumber;
}

void Profiler::RenderReportLine( const ProfilerReport& report, const ProfilerReportEntry& reportEntry ) const
{
	Vector2 startMins = GetReportStartMins();
	Vector2 lineSpacing = GetReportLineSpacing();	// mins - ( lineNumber * spacing )
	float fontHeight = GetReportFontHeight();

	Vector2 minsForLine = startMins - ( static_cast< float >( reportEntry.m_lineNumber ) * lineSpacing );

	double percentageOfTotalTime = ( reportEntry.m_totalTimeMS / report.GetTotalTime() ) * 100.0;

	std::string fullId = std::string( reportEntry.m_id );
	if ( reportEntry.CanBeCollapsed() )
	{
		std::string toBePrefixed = "[-] ";
		if ( IsNodeCollapsed( reportEntry ) )
		{
			toBePrefixed = "[+] ";
		}
		fullId = toBePrefixed + fullId;
	}
	const char* entryId = fullId.c_str();

	double totalTimeMS = reportEntry.m_totalTimeMS;
	std::string totalTimeString = GetPrettyTimeString( totalTimeMS );
	double selfTimeMS = reportEntry.m_selfTimeMS;
	std::string selfTimeString = GetPrettyTimeString( selfTimeMS );

	std::string lineText = Stringf(
		"%*s%-*s%8d%16s%7.2f%%%16s%7.2f%%\0",
		reportEntry.m_depth, "",
		( 72 - reportEntry.m_depth ), entryId,
		reportEntry.m_callCount,
		totalTimeString.c_str(),
		percentageOfTotalTime,
		selfTimeString.c_str(),
		reportEntry.m_percentageSelfTime
	);
	Renderer::GetInstance()->DrawText2D(
		minsForLine,
		lineText,
		fontHeight,
		Rgba::WHITE,
		1.0f,
		nullptr
	);

	if ( m_reportView == ProfilerReportView::PROFILER_REPORT_TREE && !IsNodeCollapsed( reportEntry ) )
	{
		for ( std::map< std::string, ProfilerReportEntry* >::const_iterator mapIterator = reportEntry.m_children.begin(); mapIterator != reportEntry.m_children.end(); mapIterator++ )
		{
			ProfilerReportEntry* entry = mapIterator->second;
			RenderReportLine( report, *entry );
		}
	}
	else if ( &reportEntry == report.m_rootEntry ) // Process only root entry for Flat view
	{
		for ( std::vector< ProfilerReportEntry* >::const_iterator vecIterator = report.m_flatViewSortedEntries.begin(); vecIterator != report.m_flatViewSortedEntries.end(); vecIterator++ )
		{
			ProfilerReportEntry* entry = *vecIterator;
			RenderReportLine( report, *entry );
		}
	}
}

void Profiler::ConsolePrintfReport( ProfilerReportView view, ProfilerReportSortMode sortMode /* = ProfilerReportSortMode::PROFILER_REPORT_SORT_TOTAL */ ) const
{
	int frameIndex = GetNewestFrameIndex();

	ProfilerReport report;
	switch ( view )
	{
		case ProfilerReportView::PROFILER_REPORT_TREE	:	report.GenerateTreeReport( *m_pastFrameStacks[ frameIndex ] );	break;
		case ProfilerReportView::PROFILER_REPORT_FLAT	:	report.GenerateFlatReport( *m_pastFrameStacks[ frameIndex ], sortMode );	break;
	}

	ConsolePrintf( "Frame Report: " );
	ConsolePrintf(  "------------" );
	ConsolePrintf(
		"%-58s%8s%16s%8s%16s%8s",
		"FUNCTION NAME\0",
		"CALLS\0",
		"TOTAL(TIME)\0",
		"TOTAL(%)\0",
		"SELF(TIME)\0",
		"SELF(%)\0"
	);

	ConsolePrintfReportLine( report, *report.m_rootEntry, 0, view, sortMode );
}

void Profiler::ConsolePrintfReportLine( const ProfilerReport& report, const ProfilerReportEntry& reportEntry, int depth, ProfilerReportView view, ProfilerReportSortMode sortMode /* = ProfilerReportSortMode::PROFILER_REPORT_SORT_TOTAL */ ) const
{
	double percentageOfTotalTime = ( reportEntry.m_totalTimeMS / report.GetTotalTime() ) * 100.0;

	double totalTimeMS = reportEntry.m_totalTimeMS;
	std::string totalTimeString = GetPrettyTimeString( totalTimeMS );
	double selfTimeMS = reportEntry.m_selfTimeMS;
	std::string selfTimeString = GetPrettyTimeString( selfTimeMS );

	ConsolePrintf(
		"%*s%-*s%8d%16s%7.2f%%%16s%7.2f%%\0",
		depth, "",
		( 58 - depth ), reportEntry.m_id.c_str(),
		reportEntry.m_callCount,
		totalTimeString.c_str(),
		percentageOfTotalTime,
		selfTimeString.c_str(),
		reportEntry.m_percentageSelfTime
	);

	if ( view == ProfilerReportView::PROFILER_REPORT_TREE )
	{
		for ( std::map< std::string, ProfilerReportEntry* >::const_iterator mapIterator = reportEntry.m_children.begin(); mapIterator != reportEntry.m_children.end(); mapIterator++ )
		{
			ProfilerReportEntry* entry = mapIterator->second;
			ConsolePrintfReportLine( report, *entry, ( depth + 1 ), view, sortMode );
		}
	}
	else if ( &reportEntry == report.m_rootEntry ) // Process only root entry for Flat view
	{
		for ( std::vector< ProfilerReportEntry* >::const_iterator vecIterator = report.m_flatViewSortedEntries.begin(); vecIterator != report.m_flatViewSortedEntries.end(); vecIterator++ )
		{
			ProfilerReportEntry* entry = *vecIterator;
			ConsolePrintfReportLine( report, *entry, ( depth + 1 ), view, sortMode );
		}
	}
}

/* GETTERS */

bool Profiler::IsVisible() const
{
	return m_isVisible;
}

bool Profiler::IsRunning() const
{
	return m_isCapturingFrames;
}

int Profiler::GetPreviousFrames( int start, int end, ProfilerMeasurement*** out_frames ) const
{
	if ( start < 0 || end < 0 )
	{
		ERROR_RECOVERABLE( "WARNING: Profiler::GetPreviousFrames() - \"start\" and \"end\" cannot be negative. Method will not process." );
		return 0;
	}
	if ( end < start )
	{
		ERROR_RECOVERABLE( "WARNING: Profiler::GetPreviousFrames() - \"end\" cannot be lesser than \"start\". Method will not process." );
		return 0;
	}

	int numFramesRequested = end - start;
	numFramesRequested = ClampInt( numFramesRequested, 0, PROFILER_HISTORY_MAX_FRAMES );

	int firstIndex = m_pastFrameStacksBackIndex - numFramesRequested;
	if ( firstIndex < 0 )
	{
		firstIndex = PROFILER_HISTORY_MAX_FRAMES + firstIndex;
	}

	int realFrameCount = numFramesRequested;
	int lastIndex = GetNewestFrameIndex();
	while ( ( m_pastFrameStacks[ firstIndex ] == nullptr ) && ( firstIndex != lastIndex ) )
	{
		firstIndex++;
		realFrameCount--;
	}

	*out_frames = reinterpret_cast< ProfilerMeasurement** >( malloc( sizeof( ProfilerMeasurement* ) * realFrameCount ) );
	for ( int frameIndex = 1; frameIndex <= realFrameCount; frameIndex++ )
	{
		int destinationIndex = realFrameCount - frameIndex;
		int sourceIndex = ( firstIndex + destinationIndex ) % PROFILER_HISTORY_MAX_FRAMES;
		( *out_frames )[ destinationIndex ] = m_pastFrameStacks[ sourceIndex ];
	}

	return realFrameCount;
}

double Profiler::GetWorstFrameTimeSeconds() const
{
	double worstTime = 0.0f;
	for ( int frameIndex = 0; frameIndex < PROFILER_HISTORY_MAX_FRAMES; frameIndex++ )
	{
		if ( m_pastFrameStacks[ frameIndex ] != nullptr )
		{
			double frameTime = m_pastFrameStacks[ frameIndex ]->GetElapsedTime();
			if ( frameTime > worstTime )
			{
				worstTime = frameTime;
			}
		}
	}
	return worstTime;
}

int Profiler::GetOldestFrameIndex() const
{
	int frameIndex = GetNewestFrameIndex();
	while ( m_pastFrameStacks[ frameIndex ] != nullptr && frameIndex != m_pastFrameStacksBackIndex )
	{
		frameIndex--;
		if ( frameIndex < 0 )
		{
			frameIndex = PROFILER_HISTORY_MAX_FRAMES + frameIndex;
		}
	}
	if ( m_pastFrameStacks[ frameIndex ] == nullptr )
	{
		frameIndex = ( frameIndex + 1 ) % PROFILER_HISTORY_MAX_FRAMES;
	}
	return frameIndex;
}

int Profiler::GetNewestFrameIndex() const
{
	int frameIndex = m_pastFrameStacksBackIndex - 1;
	if ( frameIndex < 0 )
	{
		frameIndex = PROFILER_HISTORY_MAX_FRAMES + frameIndex;
	}
	return frameIndex;
}

int Profiler::GetSelectedFrameIndex() const
{
	int actualSelectedIndex = ( m_pastFrameStacksBackIndex + 1 ) % PROFILER_HISTORY_MAX_FRAMES;
	actualSelectedIndex += m_graphSelectedFrameIndex;
	actualSelectedIndex = actualSelectedIndex % PROFILER_HISTORY_MAX_FRAMES;
	return actualSelectedIndex;
}

Vector2 Profiler::GetScaledVector2( float x, float y ) const
{
	Vector2 unscaledVector2 = Vector2( x, y );
	return ( unscaledVector2 * s_dimensionScaling );
}

float Profiler::GetGraphXPositionForFrameIndex( int frameIndex ) const
{
	AABB2 graphBounds = GetGraphBounds();
	float graphXMin = graphBounds.mins.x;
	float graphXMax = graphBounds.maxs.x;
	float indexFloat = static_cast< float >( frameIndex );
	float maxIndexFloat = static_cast< float >( PROFILER_HISTORY_MAX_FRAMES - 1 );
	float xPosition = RangeMapFloat( indexFloat, 0.0f, maxIndexFloat, graphXMin, graphXMax );
	return xPosition;
}

AABB2 Profiler::GetGraphBounds() const
{
	return AABB2(
		GetScaledVector2( -0.95f, 0.65f ),
		GetScaledVector2( 0.75f, 0.85f )
	);
}

AABB2 Profiler::GetButtonBounds() const
{
	return AABB2(
		GetScaledVector2( 0.775f, 0.65f ),
		GetScaledVector2( 0.85f, 0.75f )
	);
}

AABB2 Profiler::GetReportEntryBounds( int lineNumber ) const
{
	Vector2 minsForLine = GetReportStartMins() - ( static_cast< float >( lineNumber ) * GetReportLineSpacing() );
	Vector2 maxsForLine = minsForLine + Vector2( 0.5f, GetReportFontHeight() );
	return AABB2( minsForLine, maxsForLine );
}

Vector2 Profiler::GetReportStartMins() const
{
	return GetScaledVector2( -0.95f, 0.45f );
}

Vector2 Profiler::GetReportLineSpacing() const
{
	return GetScaledVector2( 0.0f, 0.05f );
}

float Profiler::GetReportFontHeight() const
{
	return 0.025f;
};

Rgba Profiler::GetColorForGraphPosition( const Vector2& position ) const
{
	AABB2 graphBounds = GetGraphBounds();
	float worstTimeSeconds = static_cast< float >( GetWorstFrameTimeSeconds() );

	float frameTimeSeconds = RangeMapFloat( position.y, graphBounds.mins.y, graphBounds.maxs.y, 0.0f, worstTimeSeconds );

	if ( frameTimeSeconds > 0.0f )
	{
		float fps = 1.0f / frameTimeSeconds;
		return GetColorForFPS( fps );
	}
	else
	{
		return Rgba::GREEN;
	}
}

Rgba Profiler::GetColorForFPS( float fps ) const
{
	float thresholds[ 3 ] = {
		g_gameConfigBlackboard.GetValue( "profilerFPSGood", 60.0f ),
		g_gameConfigBlackboard.GetValue( "profilerFPSFair", 30.0f ),
		g_gameConfigBlackboard.GetValue( "profilerFPSBad", 20.0f )
	};
	if ( OVRContext::IsVREnabled() )
	{
		thresholds[ 0 ] = g_gameConfigBlackboard.GetValue( "profilerFPSGoodVR", 120.0f );
		thresholds[ 1 ] = g_gameConfigBlackboard.GetValue( "profilerFPSFairVR", 90.0f );
		thresholds[ 2 ] = g_gameConfigBlackboard.GetValue( "profilerFPSBadVR", 75.0f );
	}

	if ( IsFloatGreaterThanOrEqualTo( fps, thresholds[ 0 ] ) )
	{
		return Rgba::GREEN;
	}
	else if ( IsFloatGreaterThanOrEqualTo( fps, thresholds[ 1 ] ) )
	{
		return Rgba::YELLOW;
	}
	else if ( IsFloatGreaterThanOrEqualTo( fps, thresholds[ 2 ] ) )
	{
		return Rgba::ORANGE;
	}
	else
	{
		return Rgba::RED;
	}
}

std::string Profiler::GetIdForReportEntry( const ProfilerReportEntry& entry ) const
{
	std::string id = "";
	const ProfilerReportEntry* entryPtr = &entry;
	while ( entryPtr != nullptr )
	{
		id += std::string( entryPtr->m_id + "/" );
		entryPtr = entryPtr->m_parent;
	}
	return id;
}

std::string Profiler::GetPrettyTimeString( double milliseconds ) const
{
	std::string timeString = Stringf( "%.3f ms", milliseconds );

	if ( IsDoubleGreaterThanOrEqualTo( milliseconds, 1000.0 ) )
	{
		double seconds = milliseconds * 0.001;
		timeString = Stringf( "%.3f s", seconds );
	}
	else if ( milliseconds < 0.001 )
	{
		double microseconds = milliseconds * 1000.0;
		timeString = Stringf( "%.3f us", microseconds );
	}

	return timeString;
}

bool Profiler::IsNodeCollapsed( const ProfilerReportEntry& entry ) const
{
	std::string nodeID = GetIdForReportEntry( entry );
	bool found = ( std::find( m_collapsedNodesByID.begin(), m_collapsedNodesByID.end(), nodeID ) != m_collapsedNodesByID.end() );
	return found;
}

/* STATIC METHODS */

/* static */
void Profiler::Render()
{
	g_profiler->RenderView();
}

/* static */
void Profiler::EndFrame()
{
	g_profiler->MarkFrame();
}

/* static */
void Profiler::StartProfile( const char* id )
{
	g_profiler->Push( id );
}

/* static */
void Profiler::EndProfile()
{
	g_profiler->Pop();
}

/* static */
Profiler* Profiler::GetInstance()
{
	return g_profiler;
}

/* STANDALONE FUNCTIONS */

void ProfilerStartup()
{
	g_profiler = new Profiler();
}

void ProfilerShutdown()
{
	delete g_profiler;
	g_profiler = nullptr;
}

bool ProfilerCommand( Command& command )
{
	if ( command.GetName() == "profiler" )
	{
		g_profiler->ToggleVisibility();
		return true;
	}
	else
	{
		return false;
	}
}

bool ProfilerPauseCommand( Command& command )
{
	if ( command.GetName() == "profiler_pause" )
	{
		g_profiler->Pause();
		return true;
	}
	else
	{
		return false;
	}
}

bool ProfilerResumeCommand( Command& command )
{
	if ( command.GetName() == "profiler_resume" )
	{
		g_profiler->Resume();
		return true;
	}
	else
	{
		return false;
	}
}

bool ProfilerReportCommand( Command& command )
{
	if ( command.GetName() == "profiler_report" )
	{
		std::string viewString = command.GetNextString();
		ProfilerReportView view = ProfilerReportView::PROFILER_REPORT_TREE;
		if ( viewString == "tree" || viewString == "Tree" || viewString == "t" || viewString == "T" )
		{

		}
		else if ( viewString == "flat" || viewString == "Flat" || viewString == "f" || viewString == "F" )
		{
			view = ProfilerReportView::PROFILER_REPORT_FLAT;
		}
		else
		{
			ConsolePrintf( Rgba::RED, "ERROR: profiler_report: No view option provided. \"Tree\" and \"Flat\" are accepted values." );
			return false;
		}

		ProfilerReportSortMode sortMode = ProfilerReportSortMode::PROFILER_REPORT_SORT_TOTAL;
		if ( view == ProfilerReportView::PROFILER_REPORT_FLAT )
		{
			std::string sortString = command.GetNextString();
			if ( sortString == "Total" || sortString == "total" || viewString == "t" || viewString == "T" )
			{

			}
			else if ( sortString == "self" || sortString == "Self" || sortString == "s" || sortString == "S" )
			{
				sortMode = ProfilerReportSortMode::PROFILER_REPORT_SORT_SELF;
			}
			else
			{
				ConsolePrintf( Rgba::RED, "ERROR: profiler_report: No sort option provided. \"Total\" and \"Self\" are accepted values." );
				return false;
			}
		}

		g_profiler->ConsolePrintfReport( view, sortMode );

		return true;
	}
	else
	{
		return false;
	}
}

#endif