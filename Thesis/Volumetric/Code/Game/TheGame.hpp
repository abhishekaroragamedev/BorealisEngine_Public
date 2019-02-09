#pragma once

#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Vector4.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/Light.hpp"
#include "Engine/Renderer/Mesh.hpp"
#include "Engine/Renderer/Shader.hpp"
#include <iterator>
#include <vector>

enum DebugRenderMode;
class Command;
class Material;
class Mesh;
class Renderable;
class Sampler;
class Scene;
class Texture;
class Texture3D;

constexpr char DEFAULT_FONT_NAME[] = "SquirrelFixedFont";
constexpr char MIE_CHOPPED_TXT_FILEPATH[] = "Data/MieGraph/Mie_Chopped.txt";
constexpr char MIE_PEAK_TXT_FILEPATH[] = "Data/MieGraph/Mie_Peak.txt";
constexpr char WEATHER_TEXTURE_FILEPATH[] = "Data/Images/WeatherTextureLow.png";
constexpr char HEIGHT_SIGNAL_FILEPATH[] = "Data/Images/HeightSignalCumulus.png";
constexpr char SHIP_OBJ_FILEPATH[] = "Data/Models/SciFiFighter/scifi_fighter_mk6.obj";

enum ShaderToggleOption	:	int
{
	CLOUD_LAYER_ALTITUDE,
	CLOUD_LAYER_HEIGHT,
	SHADOWS,
	WEATHER_TEXTURE_TILE,
	SHAPE_TEXTURE_TILE,
	DETAIL_TEXTURE_TILE,
	SHAPE_WORLEY_INVERT_FACTOR,
	DETAIL_WORLEY_INVERT_FACTOR,
	MAX_DENSITY,
	SUNLIGHT_INTENSITY,
	CAMERA_FOV_DEGREES,
	VAN_DER_CORPUT_NUMBER,
	NUM_TOGGLE_OPTIONS
};

class TheGame
{

public:
	TheGame();
	~TheGame();

public:
	void Update();
	void Render() const;

	void MoveCameraTo( const Vector3& position );
	void MoveCamera( const Vector3& displacement );

private:
	void InitializeCamerasAndScratchTarget();
	void InitializeDeferredTargets();
	void InitializeLights();
	void InitializeShaderResources();
	void InitializeTerrain();
	void InitializeShip();
	void InitializeMieData();
	void InitializeNoiseData();
	void InitializeCloudRenderTargets();
	void InitializeVanDerCorputSequence();

	void HandleCameraPositionSnapInputs();
	void HandleFKeyInputs();
	void HandleCameraInput();
	void HandleSunPositionInput();
	bool HandleOptionsInput();
	void HandleShaderToggleInput();

	void UpdateAmbientLight();
	void UpdateCameraProperties();
	void UpdateRenderTargets();
	void UpdateNoiseTextures();
	void UpdateShipMatrixFromTransform();

	void RenderInit() const;
	void RenderScene() const;
	void RenderSky() const;
	void RenderScatteringTransmittance() const;
	void RenderPostProcess() const;
	void RenderCopyToDefaultColorTarget() const;
	void RenderVisualDebugOverlay() const;
	void RenderDepthTargetDebug() const;
	void RenderScatteringTransmittanceDebug() const;
	void RenderSceneShadowsDebug() const;
	void RenderSceneNormalsDebug() const;
	void RenderSceneSpecularDebug() const;
	void RenderMieGraph() const;
	void RenderWeatherTextureDebug() const;
	void RenderMieLUTDebug() const;
	void RenderShapeTextureLayerDebug() const;
	void RenderDetailTextureLayerDebug() const;
	void RenderGeneralDebugInfo() const;

	void TogglePossessShip();

	void DeleteRenderables();

	std::string GetNameForCurrentToggleOption() const;

private:
	// Game resources
	BitmapFont* m_defaultFont = nullptr;

	// Scene variables
	Light* m_sun = nullptr;
	LightUBO* m_sceneLights[ 8 ] = {};
	Vector3 m_sunPositionPolar = Vector3( 999999.0f, 45.0f, 45.0f );
	Camera* m_gameCamera = nullptr;
	Renderable* m_terrain = nullptr;
	Renderable* m_ship = nullptr;
	Transform m_shipTransform;

	// Shader data
	Texture3D* m_shapeTexture = nullptr;
	Texture3D* m_detailTexture = nullptr;
	Texture* m_weatherTexture = nullptr;
	Texture* m_heightSignal = nullptr;
	Texture* m_mieLUT = nullptr;

	// Deferred lighting
	Texture* m_sceneNormalTarget = nullptr;
	Texture* m_sceneSpecularTarget = nullptr;
	Texture* m_volumetricShadowTarget = nullptr;
	Sampler* m_depthPointSampler = nullptr;
	bool m_usePointSampler = false;

	// For scaling up in the end
	Texture* m_sceneColorTarget = nullptr;
	Texture* m_sceneDepthTarget = nullptr;
	Texture* m_lowResScratchTarget = nullptr;

	// Temporal upsampling
	unsigned int m_currentUpsamplingTargetIndex = 0U;
	Texture* m_upsamplingTargets[ 2 ] = {};
	Matrix44 m_oldViewMatrix;
	float m_raymarchOffset;
	float* m_vanDerCorputSequence = nullptr;
	unsigned int m_vanDerCorputPower = 16U;
	unsigned int m_vanDerCorputCurrentIndex = 0U;	// So that 0U is the first value used

	// UI variables
	Renderable* m_mieGraphGrid = nullptr;
	Renderable* m_mieGraphR = nullptr;
	Renderable* m_mieGraphG = nullptr;
	Renderable* m_mieGraphB = nullptr;
	int m_detailTexRenderChannel = 0;
	int m_detailTexRenderLayer = 0;
	int m_shapeTexRenderChannel = 0;	// 0-r, 1-g, 2-b, 3-a
	int m_shapeTexRenderLayer = 0;		// [0-31]
	float m_weatherTexUVOffset = 0.0f;

	// Debug variables
	bool m_debugOverlayEnabled = false;
	Camera* m_debugOverlayCamera = nullptr;
	ShaderToggleOption m_toggleOption = ShaderToggleOption::SHADOWS;
	float m_cloudLayerAltitude = 5200.0f;
	float m_cloudLayerHeight = 200.0f;
	float m_shadowsEnabled = 1.0f;
	float m_weatherTextureTileFactor = 1.5f;
	float m_shapeTextureTileFactor = 2.0f;
	float m_detailTextureTileFactor = 1.8f;
	float m_shapeWorleyInvertFactor = 1.0f;
	float m_detailWorleyInvertFactor = 0.15f;
	float m_maxCloudDensity = 1.0f;
	float m_miePhaseFunctionMax = 0.0f;
	float m_sunlightIntensity = 0.1f;
	float m_detailTextureOffset = 0.0f;
	float m_cameraFOV = 35.0f;
	bool m_temporalUpsamplingEnabled = true;
	bool m_detailTexturePanningEnabled = true;
	bool m_debugLodsEnabled = false;
	bool m_cloudsEnabled = true;
	bool m_godRaysEnabled = true;
	bool m_possessShip = false;

	Vector3 m_origin					= Vector3::ZERO;	// Will be set from XML on start
	const Vector3 m_snapBottomPosition	= Vector3( 0.0f, 1.0f, 0.0f );
	const Vector3 m_snapMiddlePosition	= Vector3( 0.0f, 300.0f, 0.0f );
	const Vector3 m_snapTopPosition		= Vector3( 0.0f, 500.0f, 0.0f );

};

Vector3 TerrainSurfacePatch( float u, float v );
Vector3 TerrainSurfacePatchRadial( float u, float v );

bool SetAmbientLightCommand( Command& command );
bool CameraGoToCommand( Command& command );
bool CameraGoInDirectionCommand( Command& command );
