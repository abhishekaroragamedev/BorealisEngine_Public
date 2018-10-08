#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Window.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/OculusRift/OVRContext.hpp"
#include "Engine/OculusRift/OVRHeadset.hpp"
#include "Engine/OculusRift/OVRHelper.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/Framebuffer.hpp"
#include "Engine/Renderer/GLFunctionBinding.hpp"
#include "Engine/Renderer/GLUtils.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Tools/DevConsole.hpp"
#include "ThirdParty/OculusSDK/Include/OVR_CAPI_Audio.h"
#include "ThirdParty/OculusSDK/Include/OVR_CAPI_GL.h"
#include "ThirdParty/OculusSDK/Include/Extras/OVR_CAPI_Util.h"
#include "ThirdParty/OculusSDK/Include/Extras/OVR_Math.h"

OVRHeadset::OVRHeadset( ovrSession session, ovrHmdDesc description )
	:	m_session( session ),
		m_headsetDescription( description )
{
	SetHUDEnabled( g_gameConfigBlackboard.GetValue( "vrHUDEnabled", false ) );
#ifndef ENGINE_DISABLE_AUDIO
	InitializeAudio();
#endif
}

OVRHeadset::~OVRHeadset()
{
	ovr_DestroyMirrorTexture( m_session, m_ovrMirrorTexture );
	ovr_DestroyTextureSwapChain( m_session, m_sceneSwapChain );
	ovr_DestroyTextureSwapChain( m_session, m_leftHUDSwapChain );
	ovr_DestroyTextureSwapChain( m_session, m_rightHUDSwapChain );

	delete m_flipYScratchTarget;
	m_flipYScratchTarget = nullptr;

	delete m_mirrorTexture;
	m_mirrorTexture = nullptr;

	delete m_flipYCamera;
	m_flipYCamera = nullptr;

	delete m_leftEyeCamera;
	m_leftEyeCamera = nullptr;

	delete m_rightEyeCamera;
	m_rightEyeCamera = nullptr;

	delete m_leftHUDCamera;
	m_leftHUDCamera = nullptr;

	delete m_rightHUDCamera;
	m_rightHUDCamera = nullptr;

	delete m_leftEyeDepthTarget;
	m_leftEyeDepthTarget = nullptr;

	delete m_rightEyeDepthTarget;
	m_rightEyeDepthTarget = nullptr;

	delete m_sceneColorTarget;
	m_sceneColorTarget = nullptr;

	delete m_leftHUDColorTarget;
	m_leftHUDColorTarget = nullptr;

	delete m_rightHUDColorTarget;
	m_rightHUDColorTarget = nullptr;

	delete m_sceneSwapChainFramebuffer;
	m_sceneSwapChainFramebuffer = nullptr;

	delete m_leftHUDSwapChainFramebuffer;
	m_leftHUDSwapChainFramebuffer = nullptr;

	delete m_rightHUDSwapChainFramebuffer;
	m_rightHUDSwapChainFramebuffer = nullptr;

	delete m_sceneSwapChainTexture;
	m_sceneSwapChainTexture = nullptr;

	delete m_leftHUDSwapChainTexture;
	m_leftHUDSwapChainTexture = nullptr;

	delete m_rightHUDSwapChainTexture;
	m_rightHUDSwapChainTexture = nullptr;
}

void OVRHeadset::InitializeAudio()
{
#if defined( ENGINE_DISABLE_AUDIO )
	ERROR_RECOVERABLE( "OVRHeadset::InitializeAudio(): Audio System disabled. Audio will not be initialized." );
	return;
#endif

	FMOD::System* fmodSystem = AudioSystem::GetInstance()->m_fmodSystem;

	GUID ovrSpeakerGUID;
	ovr_GetAudioDeviceOutGuid( &ovrSpeakerGUID );

	int driverCount = 0;
	fmodSystem->getNumDrivers( &driverCount );

	int currentDriver = 0;
	while ( currentDriver < driverCount )
	{
		char name[ 256 ] = { 0 };
		FMOD_GUID fmodGuid = { 0 };
		fmodSystem->getDriverInfo( currentDriver, name, 256, &fmodGuid, nullptr, nullptr, nullptr );
		if (
			( fmodGuid.Data1 == ovrSpeakerGUID.Data1 ) &&
			( fmodGuid.Data2 == ovrSpeakerGUID.Data2 ) &&
			( fmodGuid.Data3 == ovrSpeakerGUID.Data3 ) &&
			( memcmp( fmodGuid.Data4, ovrSpeakerGUID.Data4, 8 ) == 0 )
		)
		{
			break;
		}

		currentDriver++;
	}

	if ( currentDriver < driverCount )
	{
		FMOD_RESULT result = fmodSystem->setDriver( currentDriver );
		AudioSystem::GetInstance()->ValidateResult( result );
	}
	else
	{
		ERROR_RECOVERABLE("OVRHeadset: Could not find Oculus audio driver." );
	}

	// Initialize ear transforms
	m_ears[ 0 ].m_transform.Reparent( &m_transform );
	Vector3 leftEarPosition = Vector3( -0.06f, 0.0f, -0.05f );	// Distances in meters
	Vector3 leftEarRight = Vector3::FORWARD;
	Vector3 leftEarUp = Vector3::UP;
	Vector3 leftEarForward = Vector3::RIGHT * -1.0f;
	Matrix44 leftEarMatrix = Matrix44( leftEarRight, leftEarUp, leftEarForward, leftEarPosition );
	m_ears[ 0 ].m_transform.SetLocalFromMatrix( leftEarMatrix );

	m_ears[ 1 ].m_transform.Reparent( &m_transform );
	Vector3 rightEarPosition = Vector3( 0.06f, 0.0f, -0.05f );	// Distances in meters
	Vector3 rightEarRight = Vector3::FORWARD * -1.0f;
	Vector3 rightEarUp = Vector3::UP;
	Vector3 rightEarForward = Vector3::RIGHT;
	Matrix44 rightEarMatrix = Matrix44( rightEarRight, rightEarUp, rightEarForward, rightEarPosition );
	m_ears[ 1 ].m_transform.SetLocalFromMatrix( rightEarMatrix );
}

void OVRHeadset::InitializeRenderPipeline()
{
	Renderer::GetInstance()->EnableSRGB();	// Oculus expects gamma-corrected textures. GL allows this through a Framebuffer setting
	InitializeCameras();
	InitializeSwapChains();
	InitializePostProcessingData();
	InitializeMirrorTexture();
	PrepareCompositorLayers();
}

void OVRHeadset::InitializeCameras()
{
	float halfInterpupillaryDistance = 0.5f * ( g_gameConfigBlackboard.GetValue( "vrEyeSpacingmm", 60.0f ) * 0.001f );

	IntVector2 resolution = GetResolution();

	// Set up Scene cameras
	m_sceneColorTarget = Renderer::GetInstance()->CreateRenderTarget( resolution.x, resolution.y, TextureFormat::TEXTURE_FORMAT_RGBA8 );

	m_leftEyeDepthTarget = Renderer::GetInstance()->CreateRenderTarget( resolution.x, resolution.y, TextureFormat::TEXTURE_FORMAT_D24S8 );
	m_leftEyeCamera = new Camera( m_sceneColorTarget, m_leftEyeDepthTarget );
	Matrix44 leftEyeProjection = GetEngineMatrixFromOVRMatrix( m_headsetDescription.DefaultEyeFov[ 0U ], 0.1f, g_gameConfigBlackboard.GetValue( "vrCameraFarZ", 1000.0f ), ovrProjection_LeftHanded );
	m_leftEyeCamera->SetProjection( leftEyeProjection );
	m_leftEyeCamera->SetViewport( AABB2( 0.0f, 0.0f, 0.5f, 1.0f ) );
	m_leftEyeCamera->LookAt( Vector3::ZERO, Vector3::FORWARD );
	m_leftEyeCamera->Translate( Vector3( -halfInterpupillaryDistance, 0.0f, 0.0f ) );
	m_leftEyeCamera->GetTransform()->Reparent( &m_transform );

	m_rightEyeDepthTarget = Renderer::GetInstance()->CreateRenderTarget( resolution.x, resolution.y, TextureFormat::TEXTURE_FORMAT_D24S8 );
	m_rightEyeCamera = new Camera( m_sceneColorTarget, m_rightEyeDepthTarget, false, true );
	Matrix44 rightEyeProjection = GetEngineMatrixFromOVRMatrix( m_headsetDescription.DefaultEyeFov[ 1U ], 0.1f, g_gameConfigBlackboard.GetValue( "vrCameraFarZ", 1000.0f ), ovrProjection_LeftHanded );
	m_rightEyeCamera->SetProjection( rightEyeProjection );
	m_rightEyeCamera->SetViewport( AABB2( 0.5f, 0.0f, 1.0f, 1.0f ) );
	m_rightEyeCamera->LookAt( Vector3::ZERO, Vector3::FORWARD );
	m_rightEyeCamera->Translate( Vector3( halfInterpupillaryDistance, 0.0f, 0.0f ) );
	m_rightEyeCamera->GetTransform()->Reparent( &m_transform );

	if ( g_gameConfigBlackboard.GetValue( "msaaEnabled", false ) )
	{
		int numSamples = g_gameConfigBlackboard.GetValue( "msaaSamples", 4 );
		EnableMSAA( numSamples );
	}
	if ( g_gameConfigBlackboard.GetValue( "bloomEnabled", false ) )
	{
		SetBloomEnabled( true );
	}

	// Set up HUD Cameras
	float aspect = static_cast< float >( resolution.x ) / static_cast< float >( resolution.y );

	m_leftHUDColorTarget = Renderer::GetInstance()->CreateRenderTarget( resolution.x, resolution.y, TextureFormat::TEXTURE_FORMAT_RGBA8 );
	m_leftHUDCamera = new Camera( m_leftHUDColorTarget );
	m_leftHUDCamera->SetProjectionOrtho( static_cast< float >( resolution.y ), aspect, -1.0f, 1.0f );

	m_rightHUDColorTarget = Renderer::GetInstance()->CreateRenderTarget( resolution.x, resolution.y, TextureFormat::TEXTURE_FORMAT_RGBA8 );
	m_rightHUDCamera = new Camera( m_rightHUDColorTarget );
	m_rightHUDCamera->SetProjectionOrtho( static_cast< float >( resolution.y ), aspect, -1.0f, 1.0f );
}

void OVRHeadset::InitializeSwapChains()
{
	ovrTextureSwapChainDesc description = {};
	description.Type = ovrTexture_2D;
	description.ArraySize = 1;
	description.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
	description.Width = GetResolution().x;
	description.Height = GetResolution().y;
	description.MipLevels = 1;
	description.SampleCount = 1;
	description.StaticImage = ovrFalse;

	if ( ovr_CreateTextureSwapChainGL( m_session, &description, &m_sceneSwapChain ) == ovrSuccess )
	{
		m_sceneSwapChainTexture = new Texture();
		ovr_GetTextureSwapChainBufferGL( m_session, m_sceneSwapChain, m_currentSceneSwapChainIndex, &m_sceneSwapChainTexture->m_textureID );
		m_sceneSwapChainTexture->m_type = TextureType::TEXTURE_TYPE_2D;
		m_sceneSwapChainTexture->m_dimensions = GetResolution();
		m_sceneSwapChainTexture->m_format = TextureFormat::TEXTURE_FORMAT_RGBA8;

		m_sceneSwapChainFramebuffer = new FrameBuffer();
		m_sceneSwapChainFramebuffer->SetColorTarget( m_sceneSwapChainTexture );
	}
	else
	{
		ERROR_AND_DIE("OVRHeadset: Scene swap chain creation failed. Aborting..." );
	}
	TODO( "Flexible HUD SwapChain resolution" );
	if ( ovr_CreateTextureSwapChainGL( m_session, &description, &m_leftHUDSwapChain ) == ovrSuccess )
	{
		m_leftHUDSwapChainTexture = new Texture();
		ovr_GetTextureSwapChainBufferGL( m_session, m_leftHUDSwapChain, m_currentLeftHUDSwapChainIndex, &m_leftHUDSwapChainTexture->m_textureID );
		m_leftHUDSwapChainTexture->m_type = TextureType::TEXTURE_TYPE_2D;
		m_leftHUDSwapChainTexture->m_dimensions = GetResolution();
		m_leftHUDSwapChainTexture->m_format = TextureFormat::TEXTURE_FORMAT_RGBA8;

		m_leftHUDSwapChainFramebuffer = new FrameBuffer();
		m_leftHUDSwapChainFramebuffer->SetColorTarget( m_leftHUDSwapChainTexture );
	}
	else
	{
		ERROR_AND_DIE("OVRHeadset: HUD swap chain creation failed. Aborting..." );
	}
	if ( ovr_CreateTextureSwapChainGL( m_session, &description, &m_rightHUDSwapChain ) == ovrSuccess )
	{
		m_rightHUDSwapChainTexture = new Texture();
		ovr_GetTextureSwapChainBufferGL( m_session, m_rightHUDSwapChain, m_currentRightHUDSwapChainIndex, &m_rightHUDSwapChainTexture->m_textureID );
		m_rightHUDSwapChainTexture->m_type = TextureType::TEXTURE_TYPE_2D;
		m_rightHUDSwapChainTexture->m_dimensions = GetResolution();
		m_rightHUDSwapChainTexture->m_format = TextureFormat::TEXTURE_FORMAT_RGBA8;

		m_rightHUDSwapChainFramebuffer = new FrameBuffer();
		m_rightHUDSwapChainFramebuffer->SetColorTarget( m_rightHUDSwapChainTexture );
	}
	else
	{
		ERROR_AND_DIE("OVRHeadset: HUD swap chain creation failed. Aborting..." );
	}
}

void OVRHeadset::InitializePostProcessingData()
{
	IntVector2 resolution = GetResolution();
	m_flipYMaterial = Renderer::GetInstance()->CreateOrGetMaterial( "OculusVRMirrorToScreen" );
	m_flipYCamera = new Camera( Renderer::GetInstance()->GetDefaultColorTarget() );
	m_flipYCamera->SetViewport( AABB2::ZERO_TO_ONE );
	m_flipYScratchTarget = Renderer::GetInstance()->CreateRenderTarget( Window::GetWidth(), Window::GetHeight() );
}

void OVRHeadset::InitializeMirrorTexture()
{
	ovrMirrorTextureDesc mirrorTextureDescription = {};
	mirrorTextureDescription.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
	mirrorTextureDescription.Width = Window::GetWidth();
	mirrorTextureDescription.Height = Window::GetHeight();
	mirrorTextureDescription.MirrorOptions = ovrMirrorOption_PostDistortion;

	if ( ovr_CreateMirrorTextureWithOptionsGL( m_session, &mirrorTextureDescription, &m_ovrMirrorTexture ) == ovrSuccess )
	{
		m_mirrorTexture = new Texture();
		ovr_GetMirrorTextureBufferGL( m_session, m_ovrMirrorTexture, &m_mirrorTexture->m_textureID );
		m_mirrorTexture->m_dimensions = IntVector2( Window::GetWidth(), Window::GetHeight() );
		m_mirrorTexture->m_format = TextureFormat::TEXTURE_FORMAT_RGBA8;
		m_mirrorTexture->m_type = TextureType::TEXTURE_TYPE_2D;

		m_mirrorFramebuffer = new FrameBuffer();
		m_mirrorFramebuffer->SetColorTarget( m_mirrorTexture );
	}
}

void OVRHeadset::PrepareCompositorLayers()
{
	IntVector2 resolution = GetResolution();

	// Prepare Scene Layer
	m_eyeRenderDescription[ 0 ] = ovr_GetRenderDesc( m_session, ovrEye_Left, m_headsetDescription.DefaultEyeFov[ 0 ] );
	m_eyeRenderDescription[ 1 ] = ovr_GetRenderDesc( m_session, ovrEye_Left, m_headsetDescription.DefaultEyeFov[ 1 ] );
	m_hmdToEyeViewPose[ 0 ] = m_eyeRenderDescription[ 0 ].HmdToEyePose;
	m_hmdToEyeViewPose[ 1 ] = m_eyeRenderDescription[ 1 ].HmdToEyePose;

	m_sceneLayer.Header.Type = ovrLayerType_EyeFov;
	m_sceneLayer.Header.Flags = ovrLayerFlag_TextureOriginAtBottomLeft;
	m_sceneLayer.ColorTexture[ 0 ] = m_sceneSwapChain;
	m_sceneLayer.ColorTexture[ 1 ] = m_sceneSwapChain;
	m_sceneLayer.Fov[ 0 ] = m_eyeRenderDescription[ 0 ].Fov;
	m_sceneLayer.Fov[ 1 ] = m_eyeRenderDescription[ 1 ].Fov;

	m_sceneLayer.Viewport[ 0 ].Pos.x = 0; m_sceneLayer.Viewport[ 0 ].Pos.y = 0;
	m_sceneLayer.Viewport[ 0 ].Size.w = static_cast< int >( static_cast< float >( resolution.x ) * 0.5f ) ; m_sceneLayer.Viewport[ 0 ].Size.h = resolution.y;
	m_sceneLayer.Viewport[ 1 ].Pos.x = static_cast< int >( static_cast< float >( resolution.x ) * 0.5f ); m_sceneLayer.Viewport[ 1 ].Pos.y = 0;
	m_sceneLayer.Viewport[ 1 ].Size.w = static_cast< int >( static_cast< float >( resolution.x ) * 0.5f ); m_sceneLayer.Viewport[ 1 ].Size.h = resolution.y;

	// Prepare HUD Layers
	m_leftHUDLayer.Header.Type = ovrLayerType_Quad;
	m_leftHUDLayer.Header.Flags = ovrLayerFlag_HighQuality | ovrLayerFlag_HeadLocked | ovrLayerFlag_TextureOriginAtBottomLeft;
	m_leftHUDLayer.ColorTexture = m_leftHUDSwapChain;

	Vector3 hudPosition = g_gameConfigBlackboard.GetValue( "vrLeftHUDPosition", Vector3( 0.0f, -0.15f, -0.2f ) );
	m_leftHUDLayer.QuadPoseCenter.Position = GetOVRVector3FromEngineVector3( hudPosition );

	Vector3 hudOrientationEuler = g_gameConfigBlackboard.GetValue( "vrLeftHUDOrientationEuler", Vector3( 0.0f, 0.0f, 0.0f ) );
	m_leftHUDLayer.QuadPoseCenter.Orientation = GetOVRQuaternionFromEngineEuler( hudOrientationEuler );

	Vector2 hudSize = g_gameConfigBlackboard.GetValue( "vrLeftHUDDimensions", Vector2( 0.5f, 0.3f ) );
	m_leftHUDLayer.QuadSize.x = hudSize.x;
	m_leftHUDLayer.QuadSize.y = hudSize.y;

	m_leftHUDLayer.Viewport.Pos.x = 0; m_leftHUDLayer.Viewport.Pos.y = 0;
	m_leftHUDLayer.Viewport.Size.w = resolution.x; m_leftHUDLayer.Viewport.Size.h = resolution.y;

	m_rightHUDLayer.Header.Type = ovrLayerType_Quad;
	m_rightHUDLayer.Header.Flags = ovrLayerFlag_HighQuality | ovrLayerFlag_HeadLocked | ovrLayerFlag_TextureOriginAtBottomLeft;
	m_rightHUDLayer.ColorTexture = m_rightHUDSwapChain;

	hudPosition = g_gameConfigBlackboard.GetValue( "vrRightHUDPosition", Vector3( 0.0f, -0.15f, -0.2f ) );
	m_rightHUDLayer.QuadPoseCenter.Position = GetOVRVector3FromEngineVector3( hudPosition );

	hudOrientationEuler = g_gameConfigBlackboard.GetValue( "vrRightHUDOrientationEuler", Vector3( 0.0f, 0.0f, 0.0f ) );
	m_rightHUDLayer.QuadPoseCenter.Orientation = GetOVRQuaternionFromEngineEuler( hudOrientationEuler );

	hudSize = g_gameConfigBlackboard.GetValue( "vrRightHUDDimensions", Vector2( 0.5f, 0.3f ) );
	m_rightHUDLayer.QuadSize.x = hudSize.x;
	m_rightHUDLayer.QuadSize.y = hudSize.y;

	m_rightHUDLayer.Viewport.Pos.x = 0; m_rightHUDLayer.Viewport.Pos.y = 0;
	m_rightHUDLayer.Viewport.Size.w = resolution.x; m_rightHUDLayer.Viewport.Size.h = resolution.y;
}

void OVRHeadset::BeginFrame()
{
	WaitToBeginRender();
	BeginRender();

	UpdateTransform();
	UpdateAudioSources();
	UpdateEyePosesInLayer();
}

void OVRHeadset::EndFrame()
{
	EndRender();
	RenderMirrorTexture();
}

void OVRHeadset::UpdateTransform()
{
	ovrTrackingState currentTrackingState = GetTrackingState();
	if ( currentTrackingState.StatusFlags && ( ovrStatus_OrientationTracked | ovrStatus_PositionTracked ) )
	{
		ovrPosef pose = currentTrackingState.HeadPose.ThePose;

		ovrVector3f ovrPosition = pose.Position;
		Vector3 position = GetEngineVector3FromOVRVector3( ovrPosition );

		ovrQuatf ovrOrientation = pose.Orientation;
		Vector3 engineEuler = GetEngineEulerFromOVRQuaternion( ovrOrientation );

		m_transform.SetLocalPosition( position );
		m_transform.SetLocalEuler( engineEuler );
	}
	else
	{
		ConsolePrintf( Rgba::YELLOW, "OVRHeadset::UpdateTransform() - Position/Orientation not tracked." );
	}
}

void OVRHeadset::UpdateAudioSources()
{
	for ( int audioSourceIndex = 0; audioSourceIndex < m_numAudioSources; audioSourceIndex++ )
	{
		m_audioSources[ audioSourceIndex ]->UpdateCurrentSoundForListeners( m_ears[ 0 ], m_ears[ 1 ], m_masterVolume );
	}
}

void OVRHeadset::UpdateEyePosesInLayer()
{
	ovrTrackingState currentTrackingState = GetTrackingState();
	ovr_CalcEyePoses( currentTrackingState.HeadPose.ThePose, m_hmdToEyeViewPose, m_sceneLayer.RenderPose );
}

ovrResult OVRHeadset::WaitToBeginRender()
{
	// Check which Textures from the Swap Chains are currently being used, and update the CPU-side Texture objects with their IDs
	ovr_GetTextureSwapChainCurrentIndex( m_session, m_sceneSwapChain, &m_currentSceneSwapChainIndex );
	ovr_GetTextureSwapChainBufferGL( m_session, m_sceneSwapChain, m_currentSceneSwapChainIndex, &m_sceneSwapChainTexture->m_textureID );
	ovr_GetTextureSwapChainCurrentIndex( m_session, m_leftHUDSwapChain, &m_currentLeftHUDSwapChainIndex );
	ovr_GetTextureSwapChainBufferGL( m_session, m_leftHUDSwapChain, m_currentLeftHUDSwapChainIndex, &m_leftHUDSwapChainTexture->m_textureID );
	ovr_GetTextureSwapChainCurrentIndex( m_session, m_rightHUDSwapChain, &m_currentRightHUDSwapChainIndex );
	ovr_GetTextureSwapChainBufferGL( m_session, m_rightHUDSwapChain, m_currentRightHUDSwapChainIndex, &m_rightHUDSwapChainTexture->m_textureID );

	m_sceneSwapChainFramebuffer->Finalize();
	Renderer::GetInstance()->ClearColor();

	if ( m_HUDEnabled )
	{
		m_leftHUDSwapChainFramebuffer->Finalize();
		Renderer::GetInstance()->ClearColor( Rgba::BLACK.GetWithAlpha( 0.0f ) );
		Renderer::GetInstance()->SetCamera( m_leftHUDCamera );
		Renderer::GetInstance()->ClearColor( Rgba::BLACK.GetWithAlpha( 0.0f ) );

		m_rightHUDSwapChainFramebuffer->Finalize();
		Renderer::GetInstance()->ClearColor( Rgba::BLACK.GetWithAlpha( 0.0f ) );
		Renderer::GetInstance()->SetCamera( m_rightHUDCamera );
		Renderer::GetInstance()->ClearColor( Rgba::BLACK.GetWithAlpha( 0.0f ) );
	}

	ovrResult result = ovr_WaitToBeginFrame( m_session, OVRContext::GetFrameIndex() );
	if ( result != ovrSuccess )
	{
		ConsolePrintf( Rgba::RED, "OVRHeadset::WaitToBeginRender(): Error: %d; ovr_WaitToBeginFrame failed...", result );
	}

	m_ovrResult = result;
	return result;
}

ovrResult OVRHeadset::BeginRender()
{
	ovrResult result = ovr_BeginFrame( m_session, OVRContext::GetFrameIndex() );
	if ( result != ovrSuccess )
	{
		ConsolePrintf( Rgba::RED, "OVRHeadset::BeginRender(): Error: %d; ovr_BeginFrame failed...", result );
	}

	m_ovrResult = result;
	return result;
}

void OVRHeadset::RenderMirrorTexture()
{
	// Copy mirror texture's contents onto FlipY Target
	m_mirrorFramebuffer->MarkDirty();
	m_mirrorFramebuffer->Finalize();
	m_flipYCamera->SetColorTarget( Renderer::GetInstance()->GetDefaultColorTarget() );
	Renderer::GetInstance()->SetCamera( m_flipYCamera );
	Renderer::GetInstance()->CopyFrameBuffer( m_flipYCamera->GetFrameBuffer(), m_mirrorFramebuffer );

	// Using FlipY Target as a source, copy onto Default Render Target
	Renderer::GetInstance()->ApplyEffect( m_flipYMaterial, m_flipYCamera, m_flipYScratchTarget );
	Renderer::GetInstance()->FinishEffects( Renderer::GetInstance()->GetDefaultColorTarget() );
}

ovrResult OVRHeadset::EndRender()
{
	ovrResult result = false;

	if ( IsVisible() )
	{
		m_sceneSwapChainFramebuffer->MarkDirty();
		m_sceneSwapChainFramebuffer->Finalize();
		Renderer::GetInstance()->CopyFrameBuffer( m_sceneSwapChainFramebuffer, m_leftEyeCamera->GetFrameBuffer() );

		if ( m_HUDEnabled )
		{
			m_leftHUDSwapChainFramebuffer->MarkDirty();
			m_leftHUDSwapChainFramebuffer->Finalize();
			Renderer::GetInstance()->CopyFrameBuffer( m_leftHUDSwapChainFramebuffer, m_leftHUDCamera->GetFrameBuffer() );

			m_rightHUDSwapChainFramebuffer->MarkDirty();
			m_rightHUDSwapChainFramebuffer->Finalize();
			Renderer::GetInstance()->CopyFrameBuffer( m_rightHUDSwapChainFramebuffer, m_rightHUDCamera->GetFrameBuffer() );
		}
	}

	Renderer::GetInstance()->SetCamera( m_leftEyeCamera );	// Unbind the SwapChainFramebuffer

	result = ovr_CommitTextureSwapChain( m_session, m_sceneSwapChain );
	result = ovr_CommitTextureSwapChain( m_session, m_leftHUDSwapChain );
	result = ovr_CommitTextureSwapChain( m_session, m_rightHUDSwapChain );
	if ( result != ovrSuccess )
	{
		ConsolePrintf( Rgba::RED, "OVRHeadset::EndRender(): Error: %d;  ovr_CommitTextureSwapChain failed...", result );
	}

	ovrLayerHeader* layers[ 3 ]= { &m_sceneLayer.Header, &m_leftHUDLayer.Header, &m_rightHUDLayer.Header };

	ovrViewScaleDesc viewScaleDesc;
	viewScaleDesc.HmdSpaceToWorldScaleInMeters = 1.0f;
	viewScaleDesc.HmdToEyePose[ 0 ] = m_hmdToEyeViewPose[ 0 ];
	viewScaleDesc.HmdToEyePose[ 1 ] = m_hmdToEyeViewPose[ 1 ];

	result = ovr_EndFrame( m_session, OVRContext::GetFrameIndex(), &viewScaleDesc, layers, 3 );

	if ( result != ovrSuccess )
	{
		ConsolePrintf( Rgba::RED, "OVRHeadset::EndRender(): Error: %d;  ovr_EndFrame failed...", result );
	}

	Renderer::GetInstance()->UseDefaultCamera();

	m_ovrResult = result;
	return result;
}

void OVRHeadset::SetHUDEnabled( bool HUDEnabled )
{
	m_HUDEnabled = HUDEnabled;
}

void OVRHeadset::EnableMSAA( unsigned int numSamples )
{
	m_leftEyeCamera->EnableMSAA( numSamples );
	m_rightEyeCamera->EnableMSAA( numSamples );
}

void OVRHeadset::DisableMSAA()
{
	m_leftEyeCamera->DisableMSAA();
	m_rightEyeCamera->DisableMSAA();
}

void OVRHeadset::SetBloomEnabled( bool bloomEnabled )
{
	m_leftEyeCamera->SetBloomEnabled( bloomEnabled );
	m_rightEyeCamera->SetBloomEnabled( bloomEnabled );
}

void OVRHeadset::AddAudioSource( AudioSource3D* source )
{
	m_audioSources[ m_numAudioSources ] = source;
	m_numAudioSources++;
}

bool OVRHeadset::RemoveAudioSource( AudioSource3D* source )
{
	for ( int sourceIndex = 0; sourceIndex < m_numAudioSources; sourceIndex++ )
	{
		if ( m_audioSources[ sourceIndex ] == source )
		{
			m_audioSources[ sourceIndex ] = m_audioSources[ m_numAudioSources - 1 ];
			m_numAudioSources--;
			return true;
		}
	}
	return false;
}

void OVRHeadset::SetMasterVolume( float volume )
{
	m_masterVolume = volume;
	m_masterVolume = ClampFloat( m_masterVolume, 0.0f, 1.0f );
}

void OVRHeadset::IncreaseMasterVolume( float volume )
{
	m_masterVolume += volume;
	m_masterVolume = ClampFloat( m_masterVolume, 0.0f, 1.0f );
}

bool OVRHeadset::IsVisible() const
{
	return ( m_ovrResult == ovrSuccess );
}

bool OVRHeadset::IsHUDEnabled() const
{
	return m_HUDEnabled;
}

bool OVRHeadset::IsMSAAEnabled() const
{
	return m_leftEyeCamera->IsMSAAEnabled();
}

bool OVRHeadset::IsBloomEnabled() const
{
	return m_leftEyeCamera->IsBloomEnabled();
}

unsigned int OVRHeadset::GetMSAANumSamples() const
{
	return m_leftEyeCamera->GetNumMSAASamples();
}

int OVRHeadset::GetNumAudioSources() const
{
	return m_numAudioSources;
}

float OVRHeadset::GetMasterVolume() const
{
	return m_masterVolume;
}

ovrTrackingState OVRHeadset::GetTrackingState() const
{
	double displayMidpointSeconds = ovr_GetPredictedDisplayTime( m_session, OVRContext::GetFrameIndex() );
	return ovr_GetTrackingState( m_session, displayMidpointSeconds, ovrTrue );
}

Vector3 OVRHeadset::GetPosition() const
{
	return m_transform.GetLocalPosition();	// The headset's transform should never be a child!
}

Vector3 OVRHeadset::GetRight() const
{
	Matrix44 modelMatrix = m_transform.GetAsMatrixLocal();	// The headset's transform should never be a child!
	return modelMatrix.GetIBasis();
}

Vector3 OVRHeadset::GetUp() const
{
	Matrix44 modelMatrix = m_transform.GetAsMatrixLocal();	// The headset's transform should never be a child!
	return modelMatrix.GetJBasis();
}

Vector3 OVRHeadset::GetForward() const
{
	Matrix44 modelMatrix = m_transform.GetAsMatrixLocal();	// The headset's transform should never be a child!
	return modelMatrix.GetKBasis();
}

Vector3 OVRHeadset::GetEulerAngles() const
{
	return m_transform.GetLocalEuler();	// The headset's transform should never be a child!
}

IntVector2 OVRHeadset::GetResolution() const
{
	ovrSizei recommendedLeftEyeSize = ovr_GetFovTextureSize( m_session, ovrEye_Left, m_headsetDescription.DefaultEyeFov[ 0U ], 1.0f );
	ovrSizei recommendedRightEyeSize = ovr_GetFovTextureSize( m_session, ovrEye_Right, m_headsetDescription.DefaultEyeFov[ 1U ], 1.0f );
	ovrSizei textureSize;	textureSize.w = ( recommendedLeftEyeSize.w + recommendedRightEyeSize.w );	textureSize.h = Max( recommendedLeftEyeSize.h, recommendedRightEyeSize.h );
	return IntVector2( textureSize.w, textureSize.h );
}

Transform OVRHeadset::GetTransform()
{
	Transform transform;
	transform.SetLocalPosition( GetPosition() );
	transform.SetLocalEuler( GetEulerAngles() );
	return transform;
}

Camera* OVRHeadset::GetLeftCamera()
{
	return m_leftEyeCamera;
}

Camera* OVRHeadset::GetRightCamera()
{
	return m_rightEyeCamera;
}

Camera* OVRHeadset::GetLeftHUDCamera()
{
	return m_leftHUDCamera;
}

Camera* OVRHeadset::GetRightHUDCamera()
{
	return m_rightHUDCamera;
}
