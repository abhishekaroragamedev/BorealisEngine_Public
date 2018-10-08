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
class NetSession;
class Projectile;
class Renderable;
class Scene;
class Ship;
class Texture;

constexpr char SNOW_MIKU_MODEL_FILEPATH[] = "Data/Models/SnowMiku/SnowMiku.obj";
constexpr char HALLOWEEN_MIKU_MODEL_FILEPATH[] = "Data/Models/HalloweenMiku/HalloweenMiku.obj";
constexpr char LUKA_MODEL_FILEPATH[] = "Data/Models/Luka/LukaFigure.obj";

constexpr char DEFAULT_TEXTURE_FILEPATH[] ="Data/Images/Test_StbiAndDirectX.png";
constexpr char PARTICLE_TEXTURE_FILEPATH[] = "Data/Images/Basic_Particle.png";
constexpr char TERRAIN8x8_TEXTURE_FILEPATH[] ="Data/Images/Terrain_8x8.png";
constexpr char TERRAIN32x32_TEXTURE_FILEPATH[] ="Data/Images/Terrain_32x32.png";
constexpr char DEFAULT_FONT_NAME[] = "SquirrelFixedFont";

constexpr char TEXTURE_CUBE_FILEPATH[] = "Data/Images/GalaxyCube.png";

constexpr float SHIP_LIGHT_LEAD_DISTANCE = 0.1f;
constexpr float CAMERA_TRAIL_HEIGHT = 3.0f;
constexpr float CAMERA_TRAIL_DISTANCE = -12.0f;

constexpr float LIGHT_ROTATION_DEGREES_PER_SECOND = 60.0f;
constexpr float SUN_DISTANCE = 1000000000.0f;

class TheGame
{

public:
	TheGame();
	~TheGame();

public:
	void Update();
	void Render() const;

	void UpdateMaterials();

	void SetCameraLight( LightType type, const Vector4& color, const Vector3& attenuation, const Vector2& angles = Vector2::ZERO );
	void SetShapeSpecularProperties( float specularAmount, float specularPower );
	void SetShipSpecularProperties( float specularAmount, float specularPower );
	void SetMikuSpecularProperties( float specularAmount, float specularPower );
	void SetSurfacePatchSpecularProperties( float specularAmount, float specularPower );
	void SwitchCameraLight();

	int GetRenderModeIndex() const;
	bool HaveMaterialsInitialized() const;

	void FireProjectile( const Vector3& spawnPosition, const Vector3& spawnVelocity );
	void DestroyDeadProjectiles();

	void AddLight( Light* light );
	void RemoveLight( Light* light );

	void InitializeExtraLights();
	void AddExtraLightsToScene();
	void RemoveExtraLightsFromScene();

	NetSession* GetSession() const;

private:
	void InitializeLights();
	void InitializeRenderableMeshes();
	void InitializeShapeMeshes();
	void InitializeMikuMeshes();
	void GenerateSurfacePatchDemoMesh();
	void InitializeRenderableMaterials();
	void InitializeShapeMaterials();
	void InitializeShipMaterial();
	void InitializeMikuMaterials();
	void InitializeSurfacePatchDemoMaterial();

	void DeleteRenderables();

	void HandleKeyboardInput();
	void HandleKeyboardInputForCamera();
	void HandleKeyboardInputForDebugRendering();
	void HandleKeyboardInputToCycleMeshRenderMode();
	void HandleKeyboardInputToSwitchDebugRenderMode();
	void HandleXboxControllerInputs();

	void RenderNetSessionOverlay() const;

private:
	int m_entityCount = 0;
	int m_cameraLightIndex = -1;
	Texture* m_sampleTexture = nullptr;
	BitmapFont* m_defaultFont = nullptr;
	DebugRenderMode m_debugRenderMode;
	int m_renderModeIndex = 0;
	bool m_renderModeChanged = true;
	bool m_materialsInitialized = false;
	bool m_areExtraLightsInScene = false;

	Scene* m_gameScene = nullptr;
	Camera* m_gameCamera = nullptr;
	Camera* m_uiCamera = nullptr;
	std::vector< Light* > m_lights;
	std::vector< Light* > m_extraLights; // Copy for easy access
	std::vector< Renderable* > m_lightSpheres;
	Renderable* m_surfacePatchDemo = nullptr;
	Renderable* m_plane = nullptr;
	Renderable* m_cube = nullptr;
	Renderable* m_sphere = nullptr;
	Renderable* m_snowMiku[ 4 ] = { nullptr, nullptr, nullptr, nullptr };
	Renderable* m_halloweenMiku[ 3 ] = { nullptr, nullptr, nullptr };
	Renderable* m_luka[ 3 ] = { nullptr, nullptr, nullptr };
	Ship* m_ship = nullptr;
	std::vector< Projectile* > m_projectiles;

	Vector2 m_shapeSpecularProperties = Vector2( 0.5f, 5.0f );
	Vector2 m_surfacePatchSpecularProperties = Vector2( 0.1f, 1.0f );
	Vector2 m_mikuSpecularProperties = Vector2( 0.0f, 1.0f );

	Transform m_parentable;

	NetSession* m_netSession = nullptr;

};

Vector3 GraphSurfacePatch( float u, float v );
Vector3 GraphSurfacePatchZ( float u, float v );

bool SetAmbientLightCommand( Command& command );
bool SetSpecularPropertiesCommand( Command& command );
bool CameraLightCommand( Command& command );
bool DetachCameraLightCommand( Command& command );

bool AddConnectionCommand( Command& command );
bool SendPingCommand( Command& command );
bool SendAddCommand( Command& command );
