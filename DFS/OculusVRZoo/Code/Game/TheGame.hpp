#pragma once

#include "Game/GalleryScene.hpp"
#include "Game/ModelBuilderScene.hpp"

enum OVRZooScene
{
	SCENE_MENU,
	SCENE_GALLERY,
	SCENE_MODEL
};

class TheGame
{

public:
	TheGame();
	~TheGame();

public:
	void Update();
	void Render() const;

	void SetNextScene( OVRZooScene scene );

private:
	void SetupLights();
	void SetupWorldObjects();
	void SetupVRObjects();
	void SetupVRScene();

	void HandleSceneTransition();
	void UpdateHandSpatialInformation();

	void HandleKeyboardInput();
	void HandleOVRInput();
	void HandleOVRInputForFlashlightAdjustment();

	void UpdateHUDText();

private:
	OVRZooScene m_currentScene = OVRZooScene::SCENE_MENU;
	OVRZooScene m_nextScene = OVRZooScene::SCENE_MENU;
	GalleryScene* m_galleryScene = nullptr;
	ModelBuilderScene* m_modelBuilderScene = nullptr;

	Light* m_directionalLight = nullptr;
	Light* m_flashlight = nullptr;	// A spot light
	Renderable* m_grid = nullptr;
	Scene* m_scene = nullptr;

	Renderable* m_leftHand = nullptr;
	Renderable* m_rightHand = nullptr;
	Transform m_leftHandTransform;
	Transform m_rightHandTransform;

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
