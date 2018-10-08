#pragma once

#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/IntVector3.hpp"
#include "Engine/Math/Matrix44.hpp"
#include "Engine/Math/OBB3.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include <string>

class Camera;
class Clock;
class Command;
class Renderer;
class Texture;

#pragma region DefaultValues

constexpr float DEBUG_SYSTEM_3D_CAMERA_FOV_DEFAULT = 45.0f;
constexpr float DEBUG_SYSTEM_3D_CAMERA_NEAR_Z_DEFAULT = 10.0f;
constexpr float DEBUG_SYSTEM_3D_CAMERA_FAR_Z_DEFAULT = 200.0f;

constexpr char DEBUG_SOLID_SHADER_NAME[] = "DebugRenderSolid";
constexpr char DEBUG_WIREFRAME_SHADER_NAME[] = "DebugRenderWireframe";

#pragma endregion

#pragma region DataFormats

enum DebugRenderCameraType
{
	DEBUG_RENDER_CAMERA_3D,
	DEBUG_RENDER_CAMERA_2D
};

enum DebugRenderMode 
{
	DEBUG_RENDER_IGNORE_DEPTH, // will always draw and be visible 
	DEBUG_RENDER_USE_DEPTH,    // draw using normal depth rules
	DEBUG_RENDER_HIDDEN,       // only draws if it would be hidden by depth
	DEBUG_RENDER_XRAY,         // always draws, but hidden area will be drawn differently
};

struct DebugRenderOptions
{

public:
	DebugRenderOptions()
	{

	}

	explicit DebugRenderOptions( const Rgba& startColor, const Rgba& endColor, float lifeTime = 0.0f, DebugRenderMode mode = DebugRenderMode::DEBUG_RENDER_USE_DEPTH, DebugRenderCameraType cameraType = DebugRenderCameraType::DEBUG_RENDER_CAMERA_3D, const Clock* clock = nullptr, RendererPolygonMode polygonMode = RendererPolygonMode::POLYGON_MODE_FILL )
		:	m_startColor( startColor ),
			m_endColor( endColor ),
			m_lifeTime( lifeTime ),
			m_mode( mode ),
			m_cameraType( cameraType ),
			m_clock( clock ),
			m_polygonMode( polygonMode )
	{

	}

	DebugRenderOptions( const DebugRenderOptions& copy )
	{
		m_startColor = copy.m_startColor;
		m_endColor = copy.m_endColor;
		m_lifeTime = copy.m_lifeTime;
		m_mode = copy.m_mode;
		m_cameraType = copy.m_cameraType;
		m_polygonMode = copy.m_polygonMode;
		m_clock = copy.m_clock;
	}

	bool UsesMasterClock()
	{
		return ( m_clock == nullptr );
	}

	Rgba m_startColor = Rgba::WHITE;
	Rgba m_endColor = Rgba::WHITE;
	float m_lifeTime = 0.0f;
	DebugRenderMode m_mode = DebugRenderMode::DEBUG_RENDER_USE_DEPTH;
	DebugRenderCameraType m_cameraType = DebugRenderCameraType::DEBUG_RENDER_CAMERA_3D;
	RendererPolygonMode m_polygonMode = RendererPolygonMode::POLYGON_MODE_FILL;
	const Clock* m_clock = nullptr;

}; 

#pragma endregion

#pragma region LifeTimeFunctions

void DebugRenderStartup( Renderer* renderer, Camera* camera3D = nullptr, Camera* camera2D = nullptr, bool enableOnStart = false );
void DebugRenderShutdown();

void InitializeDebugMaterials();
void DeleteDebugMaterials();

void DebugRenderUpdateAndRender();

void DebugRenderSet3DCamera( Camera *camera );
void DebugRenderSet2DCamera( Camera* camera );

void EnableDebugRenderer( bool enabled = true );
bool IsDebugRenderingEnabled();

#pragma endregion

#pragma region 2DUtils

void DebugRender2DQuad( float lifeTime, 
	const AABB2& bounds, 
	const Rgba& startColor, 
	const Rgba& endColor,
	const Clock* clock = nullptr );

void DebugRender2DLine( float lifeTime, 
	const Vector2& p0, const Rgba& p0Color,
	const Vector2& p1, const Rgba& p1Color,
	const Rgba& startColor,   // tint the overall line
	const Rgba& endColor,
	const Clock* clock = nullptr );   // tint the overall line

void DebugRender2DText( float lifeTime,
	const Vector2& position,
	float cellHeight,
	float scale,
	const Vector2& alignment,
	const Rgba& startColor,
	const Rgba& endColor,
	const Clock* clock,
	const char* fontName,
	const char* format,
	... );

void DebugRenderLogF( float lifeTime,
	const Rgba& startColor,
	const Rgba& endColor,
	const Clock* clock,
	const char* fontName,
	const char* format,
	... );

#pragma endregion

#pragma region 3DUtils

void DebugRenderPoint( float lifeTime, 
	const Vector3& position, 
	const Rgba& startColor, 
	const Rgba& endColor,
	const DebugRenderMode mode = DebugRenderMode::DEBUG_RENDER_USE_DEPTH,
	const Clock* clock = nullptr ); 

void DebugRenderLineSegment( float lifeTime, 
	const Vector3& p0, const Rgba& p0Color, 
	const Vector3& p1, const Rgba& p1Color, 
	const Rgba& startColor, 
	const Rgba& endColor, 
	const DebugRenderMode mode = DebugRenderMode::DEBUG_RENDER_USE_DEPTH,
	const Clock* clock = nullptr );

void DebugRenderBasis( float lifeTime, 
	const Matrix44& basis, 
	const Rgba& startColor, 
	const Rgba& endColor, 
	const DebugRenderMode mode = DebugRenderMode::DEBUG_RENDER_USE_DEPTH,
	const Clock* clock = nullptr ); 

void DebugRenderSphere( float lifeTime, 
	const Vector3& position, 
	float radius,
	unsigned int numSlices,
	unsigned int numWedges,
	const Rgba& startColor, 
	const Rgba& endColor, 
	const DebugRenderMode mode = DebugRenderMode::DEBUG_RENDER_USE_DEPTH,
	const Clock* clock = nullptr ); 

void DebugRenderWireSphere( float lifeTime,
	const Vector3& position,
	float radius,
	unsigned int numSlices,
	unsigned int numWedges,
	const Rgba& startColor,
	const Rgba& endColor,
	const DebugRenderMode mode = DebugRenderMode::DEBUG_RENDER_USE_DEPTH,
	const Clock* clock = nullptr );

void DebugRenderWireAABB3( float lifeTime,
	const AABB3& bounds,
	const Rgba& startColor,
	const Rgba& endColor,
	DebugRenderMode mode = DebugRenderMode::DEBUG_RENDER_USE_DEPTH,
	const Clock* clock = nullptr );

void DebugRenderWireOBB3( float lifeTime,
	const OBB3& bounds,
	const Rgba& startColor,
	const Rgba& endColor,
	DebugRenderMode mode = DebugRenderMode::DEBUG_RENDER_USE_DEPTH,
	const Clock* clock = nullptr );

void DebugRenderQuad( float lifeTime,
	const Vector3& position,
	const Vector3& right, float xMin, float xMax,
	const Vector3& up, float yMin, float yMax,
	Texture* texture,        // default to a white texture if nullptr
	const AABB2& UVs,
	const Rgba& startColor,
	const Rgba& endColor,
	DebugRenderMode mode = DebugRenderMode::DEBUG_RENDER_USE_DEPTH,
	const Clock* clock = nullptr );

void DebugRenderGlyph( float lifeTime,
	const Vector3& position,
	float xMin, float xMax,
	float yMin, float yMax,
	Texture* texture,        // default to a white texture if nullptr
	const AABB2& UVs,
	const Rgba& startColor,
	const Rgba& endColor,
	DebugRenderMode mode = DebugRenderMode::DEBUG_RENDER_USE_DEPTH,
	const Clock* clock = nullptr );

void DebugRenderGrid( float lifeTime,
	const Vector3& position,
	const Vector3& right, float xMin, float xMax,
	const Vector3& up, float yMin, float yMax,
	const IntVector2& numUnits,
	const Rgba& startColor,
	const Rgba& endColor,
	DebugRenderMode mode = DebugRenderMode::DEBUG_RENDER_USE_DEPTH,
	const Clock* clock = nullptr );

void DebugRenderTag( float lifeTime,
	const Vector3& position,
	float cellHeight,
	float scale,
	const Vector2& alignment,
	const Rgba& startColor,
	const Rgba& endColor,
	DebugRenderMode mode,
	const Clock* clock,
	const char* fontName,
	const char* format,
	... );

void DebugRenderText( float lifeTime,
	const Vector3& position,
	const Vector3& rightVector,
	const Vector3& upVector,
	float cellHeight,
	float scale,
	const Vector2& alignment,
	const Rgba& startColor,
	const Rgba& endColor,
	DebugRenderMode mode,
	const Clock* clock,
	const char* fontName,
	const char* format,
	... );

#pragma endregion

#pragma region MeshRenderModes

enum MeshRenderMode
{
	MODE_INVALID = -1,
	MODE_FINAL,
	MODE_DIFFUSE,
	MODE_SPECULAR,
	MODE_DIFFUSE_AND_SPECULAR,
	MODE_UV,
	MODE_SURFACE_NORMAL,
	MODE_WORLD_NORMAL,
	MODE_VERTEX_NORMAL,
	MODE_VERTEX_TANGENT,
	MODE_VERTEX_BITANGENT,
	MODE_COLOR_ONLY,
	NUM_RENDER_MODES,
};

MeshRenderMode GetMeshRenderModeFromIndex( int renderModeAsInt );
const char* GetMeshRenderModeName( MeshRenderMode mode );
const char* GetMeshRenderModeName( int renderModeAsInt );

void SetShaderForRenderMode( MeshRenderMode renderMode );
void SetShaderForRenderMode( int renderModeIndex );
Material* GetMaterialForRenderMode( MeshRenderMode renderMode );
Material* GetMaterialForRenderMode( int renderModeIndex );

#pragma endregion

#pragma region ConsoleCommands

bool ToggleDebugRenderCommand( Command& toggleDebugRenderCommand );
bool ClearDebugRenderQueueCommand( Command& clearDebugQueueCommand );
bool ClearDebugRenderLogCommand( Command& clearDebugLogCommand );

#pragma endregion