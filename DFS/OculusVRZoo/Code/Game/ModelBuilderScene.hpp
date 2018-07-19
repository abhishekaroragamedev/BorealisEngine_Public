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

constexpr int MODEL_BUILDER_MAX_MOVABLE_VERTS = 100;

class ModelBuilderScene
{

public:
	ModelBuilderScene();
	~ModelBuilderScene();

public:
	void Update();
	void Render() const;

private:
	void SetupLights();
	void SetupWorldObjects();
	void SetupVRObjects();
	void SetupMutableModel();
	void SetupVRScene();

	void UpdateHandSpatialInformation();
	void UpdateHUDText();
	void MoveHeldVerticesWithHand();
	void AddMovingVertex( VertexBuilder* vertexToMove );
	void ClearMovingVertices();
	void TranslateModel( const Vector3& translation );
	void RotateModel( const Vector3& rotation );

	void HandleKeyboardInput();
	void HandleOVRInput();
	void HandleOVRInputForSceneTransition();
	void HandleOVRInputForLightRenderSettings();
	void HandleOVRInputForFlashlightAdjustment();
	void HandleOVRInputForModelBuilderVertexMovement();
	void HandleOVRInputForModelSettings();
	void HandleOVRInputForModelLocalRotation();
	void HandleOVRInputForModelTranslation();
	void HandleOVRInputForLightingRecomputation();
	void TryVibrateControllers();

private:
	// 3D Scene
	Light* m_directionalLight = nullptr;
	Light* m_flashlight = nullptr;	// A spot light
	Renderable* m_grid = nullptr;
	Scene* m_scene = nullptr;

	// VR Objects
	Renderable* m_leftHand = nullptr;
	Renderable* m_rightHand = nullptr;
	Transform m_leftHandTransform;
	Transform m_rightHandTransform;

	// Model Builder feature
	Renderable* m_mutableModel = nullptr;
	// Transform m_mutableModelTransform;
	MeshBuilder m_mutableMeshBuilder;
	bool m_mutableModelRenderWireframe = false;
	VertexBuilder* m_movingVertexRefs[ MODEL_BUILDER_MAX_MOVABLE_VERTS ];
	Transform m_movingVertexTransforms[ MODEL_BUILDER_MAX_MOVABLE_VERTS ];
	int m_numMovingVertices = 0;
	OBB3 m_rightHandBounds;
	Vector3 m_handHalfDimensions = Vector3::ZERO;
	Vector3 m_mutableModelCurrentTranslation = Vector3::ZERO;

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
