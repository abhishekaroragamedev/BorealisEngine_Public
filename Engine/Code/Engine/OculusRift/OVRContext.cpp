#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/OculusRift/OVRContext.hpp"
#include "Engine/OculusRift/OVRHeadset.hpp"
#include "Engine/OculusRift/OVRInput.hpp"
#include "Engine/Tools/DevConsole.hpp"

// GLOBALS
OVRContext* g_ovrContext = nullptr;

// OVRCONTEXT IMPLEMENTATION

OVRContext::OVRContext()
{
	Initialize();
}

OVRContext::~OVRContext()
{
	delete m_inputSystem;
	m_inputSystem = nullptr;

	delete m_hmd;
	m_hmd = nullptr;

	ovr_Destroy( m_session );
	ovr_Shutdown();
}

void OVRContext::Initialize()
{
	ovrResult initResult = ovr_Initialize( nullptr );
	if ( OVR_FAILURE( initResult ) )
	{
		ERROR_AND_DIE( "ERROR: OVRContext::Initialize() - could not initialize the OculusSDK. Aborting..." );
		return;
	}

	initResult = ovr_Create( &m_session, &m_luid );
	if ( OVR_FAILURE( initResult ) )
	{
		ERROR_AND_DIE( "ERROR: OVRContext::Initialize() - could not initialize the Oculus session. Aborting..." );
		return;
	}

	ovrHmdDesc description = ovr_GetHmdDesc( m_session );
	m_hmd = new OVRHeadset( m_session, description );

	m_inputSystem = new OVRInput( m_session );
}

/* static */
void OVRContext::Startup()
{
	g_ovrContext = new OVRContext();
}

/* static */
void OVRContext::BeginFrame()
{
	g_ovrContext->m_frameIndex++;
	g_ovrContext->GetHeadset()->BeginFrame();
	g_ovrContext->GetInputSystem()->BeginFrame();
}

/* static */
void OVRContext::EndFrame()
{
	g_ovrContext->GetHeadset()->EndFrame();
	g_ovrContext->GetInputSystem()->EndFrame();
}

/* static */
void OVRContext::Shutdown()
{
	delete g_ovrContext;
}

/* static */
OVRContext* OVRContext::GetInstance()
{
	return g_ovrContext;
}

/* static */
OVRHeadset* OVRContext::GetHeadset()
{
	return g_ovrContext->m_hmd;
}

/* static */
OVRInput* OVRContext::GetInputSystem()
{
	return g_ovrContext->m_inputSystem;
}

/* static */
bool OVRContext::IsVREnabled()
{
	return ( g_ovrContext != nullptr );
}

/* static */
long long OVRContext::GetFrameIndex()
{
	return g_ovrContext->m_frameIndex;
}
