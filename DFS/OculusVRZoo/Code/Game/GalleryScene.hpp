#pragma once

#include "Engine/Audio/Audio3D.hpp"
#include "Engine/Math/OBB3.hpp"
#include "Engine/Math/Transform.hpp"
#include "Engine/Renderer/MeshBuilder.hpp"

class Camera;
class Light;
class Renderable;
class Scene;
class Texture;

typedef size_t SoundPlaybackID;

constexpr char SHIP_OBJ_FILEPATH[] = "Data/Models/scifi_fighter_mk6/scifi_fighter_mk6.obj";

class GalleryScene
{

public:
	GalleryScene();
	~GalleryScene();

public:
	void Update();
	void Render() const;

private:
	void SetupLights();
	void SetupWorldObjects();
	void SetupVRObjects();
	void SetupVRScene();

	void UpdateHandSpatialInformation();
	void UpdateSpotLightTransform();
	void UpdateHUDText();
	void UpdateAudioSource();
	void RotateGalleryItems( float angleDegrees );

	void HandleKeyboardInput();
	void HandleOVRInput();
	void HandleOVRInputForSceneTransition();
	void HandleOVRInputForLightRenderSettings();
	void HandleOVRInputForFlashlightAdjustment();
	void HandleOVRInputForVolumeAdjustment();
	void HandleOVRInputForModelSettings();
	void HandleOVRInputForModelLocalRotation();
	void HandleOVRInputForModelTranslation();
	void TryVibrateControllers();

	Vector3 GetSlotPositionCartesian( int index ) const;

private:
	// 3D Scene
	Light* m_directionalLight = nullptr;
	Light* m_spotLight = nullptr;
	Light* m_flashlight = nullptr;	// A spot light
	bool m_isSpotLightMoving = true;
	Renderable* m_sphere = nullptr;
	Renderable* m_grid = nullptr;
	Renderable* m_ship = nullptr;
	Renderable* m_box = nullptr;
	Renderable* m_snowMiku[ 4 ] = { nullptr, nullptr, nullptr, nullptr };
	size_t m_shipAudioPlaybackID;
	Scene* m_scene = nullptr;

	// Scene gallery properties
	Vector2 m_galleryOrigin = Vector2::ZERO;
	float m_galleryItemHeight = -0.5f;
	const Vector2 m_gallerySlotsPolar[ 3 ] = {
		Vector2( 1.0f, 90.0f ),
		Vector2( 1.0f, 225.0f ),
		Vector2( 1.0f, 315.0f )
	};
	Renderable* m_galleryItems[ 6 ] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };

	// VR Objects
	Renderable* m_leftHand = nullptr;
	Renderable* m_rightHand = nullptr;
	Transform m_leftHandTransform;
	Transform m_rightHandTransform;

	// HUD
	Renderable* m_xButtonInstruction = nullptr;
	Renderable* m_yButtonInstruction = nullptr;
	Renderable* m_aButtonInstruction = nullptr;
	Renderable* m_bButtonInstruction = nullptr;
	Renderable* m_lStickInstruction = nullptr;
	Renderable* m_rStickInstruction = nullptr;
	Renderable* m_lStickButtonInstruction = nullptr;
	Renderable* m_rStickButtonInstruction = nullptr;
	Renderable* m_lIndexTriggerInstruction = nullptr;
	Renderable* m_lHandTriggerInstruction = nullptr;
	Renderable* m_rIndexTriggerInstruction = nullptr;
	Renderable* m_rHandTriggerInstruction = nullptr;

};
