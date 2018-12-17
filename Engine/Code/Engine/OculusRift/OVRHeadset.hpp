#pragma once

#include "Engine/Math/Transform.hpp"
#include "ThirdParty/OculusSDK/Include/OVR_CAPI.h"

class Camera;
class FrameBuffer;
class IntVector2;
class Material;
class Texture;
class Vector3;

constexpr size_t OVR_AUDIO_MAX_SOUNDS = 1000;

class OVRHeadset
{

public:
	OVRHeadset( ovrSession session, ovrHmdDesc description );
	~OVRHeadset();

	void InitializeAudio();
	void InitializeRenderPipeline();
	void BeginFrame();
	void EndFrame();

	ovrResult WaitToBeginRender();	// Resembles Oculus's ovr_WaitToBeginFrame(); Invoked by the ForwardRenderingPath
	ovrResult BeginRender();			// Resembles Oculus's ovr_BeginFrame(); Invoked by the ForwardRenderingPath
	ovrResult EndRender();			// Resembles Oculus's ovr_EndFrame(); Invoked by the ForwardRenderingPath

	void SetHUDEnabled( bool HUDEnabled );
	void EnableMSAA( unsigned int numSamples );
	void DisableMSAA();
	void SetBloomEnabled( bool bloomEnabled );
	void AddSound( size_t soundPlaybackID );
	bool RemoveSound( size_t soundPlaybackID );
	void SetMasterVolume( float volume );
	void IncreaseMasterVolume( float volume );	// Negative value will decrease this

	bool IsVisible() const;
	bool IsHUDEnabled() const;
	bool IsMSAAEnabled() const;
	bool IsBloomEnabled() const;
	unsigned int GetMSAANumSamples() const;
	size_t GetNumRegisteredSounds() const;
	float GetMasterVolume() const;
	ovrTrackingState GetTrackingState() const;
	Transform GetTransform();
	Camera* GetLeftCamera();
	Camera* GetRightCamera();
	Camera* GetLeftHUDCamera();
	Camera* GetRightHUDCamera();
	Vector3 GetPosition() const;
	Vector3 GetRight() const;
	Vector3 GetUp() const;
	Vector3 GetForward() const;
	Vector3 GetEulerAngles() const;
	IntVector2 GetResolution() const;

private:
	void InitializeCameras();
	void InitializeSwapChains();
	void InitializePostProcessingData();
	void InitializeMirrorTexture();
	void PrepareCompositorLayers();

	void UpdateTransform();
	void UpdateAudio();
	void UpdateEyePosesInLayer();

	void RenderMirrorTexture();

private:
	// Oculus Context Properties
	ovrSession m_session;
	ovrHmdDesc m_headsetDescription;
	ovrResult m_ovrResult = ovrTrue;

	// Scene Layer Properties
	ovrLayerEyeFov m_sceneLayer;
	ovrEyeRenderDesc m_eyeRenderDescription[ 2 ];
	ovrPosef m_hmdToEyeViewPose[ 2 ];
	ovrTextureSwapChain m_sceneSwapChain = nullptr;
	FrameBuffer* m_sceneSwapChainFramebuffer = nullptr;
	Texture* m_sceneSwapChainTexture = nullptr;
	int m_currentSceneSwapChainIndex = 0;

	// Scene Layer Cameras
	Camera* m_leftEyeCamera = nullptr;
	Camera* m_rightEyeCamera = nullptr;
	Texture* m_sceneColorTarget = nullptr;
	Texture* m_leftEyeDepthTarget = nullptr;
	Texture* m_rightEyeDepthTarget = nullptr;

	// HUD state
	bool m_HUDEnabled = false;

	// Left HUD Layer Properties
	ovrLayerQuad m_leftHUDLayer;
	ovrTextureSwapChain m_leftHUDSwapChain = nullptr;
	FrameBuffer* m_leftHUDSwapChainFramebuffer = nullptr;
	Texture* m_leftHUDSwapChainTexture = nullptr;
	int m_currentLeftHUDSwapChainIndex = 0;

	// Left HUD Layer Camera
	Camera* m_leftHUDCamera = nullptr;
	Texture* m_leftHUDColorTarget = nullptr;

	// Right HUD Layer Properties
	ovrLayerQuad m_rightHUDLayer;
	ovrTextureSwapChain m_rightHUDSwapChain = nullptr;
	FrameBuffer* m_rightHUDSwapChainFramebuffer = nullptr;
	Texture* m_rightHUDSwapChainTexture = nullptr;
	int m_currentRightHUDSwapChainIndex = 0;

	// Right HUD Layer Camera
	Camera* m_rightHUDCamera = nullptr;
	Texture* m_rightHUDColorTarget = nullptr;

	// Post-processing
	Material* m_flipYMaterial = nullptr;
	Camera* m_flipYCamera = nullptr;

	// Mirror Texture
	ovrMirrorTexture m_ovrMirrorTexture;
	FrameBuffer* m_mirrorFramebuffer = nullptr;
	Texture* m_mirrorTexture = nullptr;
	Texture* m_flipYScratchTarget = nullptr;

	// Audio Properties
	int m_listenerID = -1;
	size_t m_soundPlaybackIDs[ OVR_AUDIO_MAX_SOUNDS ];
	size_t m_numSoundPlaybackIDs = 0U;
	float m_masterVolume = 1.0f;

	// World Properties
	Transform m_transform;

};
