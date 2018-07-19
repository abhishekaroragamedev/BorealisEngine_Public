#include "Engine/Core/Clock.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/Vertex.hpp"
#include "Engine/Core/Window.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/Mesh.hpp"
#include "Engine/Renderer/MeshBuilder.hpp"
#include "Engine/Renderer/Renderable.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Renderer/ShaderPass.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Tools/Command.hpp"
#include "Engine/Tools/DevConsole.hpp"
#include <vector>

class DebugRenderTask;
class DebugRender2DTextTask;

#pragma region DebugRenderSystem Variables

Renderer* g_rendererReference = nullptr;

bool g_debugRendererEnabled = false;

Camera* g_debugCamera3D = nullptr;
bool g_usesOwnCamera3D = false;

Camera* g_debugCamera2D = nullptr;
bool g_usesOwnCamera2D = false;

std::vector< DebugRenderTask* > g_debugRenderTasks;
std::vector< DebugRender2DTextTask* > g_debugRenderLogTasks;

bool g_shouldUpdateLog = false;

Material* g_debugRenderMaterial = nullptr; // Instantiated first, and instanced for any DebugRenderTask
Material* g_debugRenderWireframeMaterial = nullptr;
Material* g_defaultLitMaterial = nullptr;
Material* g_diffuseMaterial = nullptr;
Material* g_specularMaterial = nullptr;
Material* g_diffuseAndSpecularMaterial = nullptr;
Material* g_UVMaterial = nullptr;
Material* g_surfaceNormalMaterial = nullptr;
Material* g_worldNormalMaterial = nullptr;
Material* g_vertexNormalMaterial = nullptr;
Material* g_vertexTangentMaterial = nullptr;
Material* g_vertexBitangentMaterial = nullptr;
Material* g_colorOnlyMaterial = nullptr;

#pragma endregion

#pragma region InternalClasses

class DebugRenderTask
{

public:
	DebugRenderTask()
	{
		m_material = g_debugRenderMaterial->GetInstance();
	}

	explicit DebugRenderTask( DebugRenderOptions debugRenderOptions )
		:	m_options( debugRenderOptions ),
			m_timeToLive( debugRenderOptions.m_lifeTime )
	{
		m_material = ( m_options.m_polygonMode == RendererPolygonMode::POLYGON_MODE_FILL )? g_debugRenderMaterial->GetInstance() : g_debugRenderWireframeMaterial->GetInstance();
	}

	explicit DebugRenderTask( const Rgba& startColor, const Rgba& endColor, float lifeTime /* = 0.0f */, DebugRenderMode mode /* = DebugRenderMode::DEBUG_RENDER_USE_DEPTH */, DebugRenderCameraType cameraType /* = DebugRenderCameraType::DEBUG_RENDER_CAMERA_3D */, const Clock* clock /* = nullptr */ )
		:	m_options( startColor, endColor, lifeTime, mode, cameraType, clock ),
			m_timeToLive( lifeTime )
	{
		m_material = ( m_options.m_polygonMode == RendererPolygonMode::POLYGON_MODE_FILL )? g_debugRenderMaterial->GetInstance() : g_debugRenderWireframeMaterial->GetInstance();
	}

	~DebugRenderTask()
	{
		delete m_mesh;
		m_mesh = nullptr;

		delete m_material;
		m_material = nullptr;
	}

	bool DebugRenderTask::HasExpired() const
	{
		return ( IsFloatLesserThanOrEqualTo( m_timeToLive, 0.0f ) );
	}

	float DebugRenderTask::GetAge() const
	{
		return ( m_options.m_lifeTime - m_timeToLive );
	}

	float DebugRenderTask::GetNormalizedAge() const
	{
		float age = GetAge();
		if ( !IsFloatEqualTo( m_options.m_lifeTime, 0.0f ) )
		{
			return ( age/ m_options.m_lifeTime );
		}
		else
		{
			return 0.0f;
		}
	}

	float DebugRenderTask::GetNormalizedTimeToLive() const
	{
		if ( !IsFloatEqualTo( m_options.m_lifeTime, 0.0f ) )
		{
			return ( m_timeToLive / m_options.m_lifeTime );
		}
		else
		{
			return 0.0f;
		}
	}

	void DebugRenderTask::Age()
	{
		if ( m_timeToLive > 0.0f )
		{
			if ( !m_options.UsesMasterClock() )
			{
				m_timeToLive -= m_options.m_clock->GetDeltaSecondsF();
			}
			else
			{
				m_timeToLive -= GetMasterDeltaSecondsF();
			}
		}
	}

	virtual void DebugRenderTask::Update()
	{

	}

	void DebugRenderTask::Render()
	{
		if ( m_mesh == nullptr )
		{
			ConsolePrintf( Rgba::RED, "ERROR: DebugRenderTask::Render() - Mesh is null." );
		}
		else
		{
			switch( m_options.m_cameraType )
			{
				case DebugRenderCameraType::DEBUG_RENDER_CAMERA_3D	:	g_rendererReference->SetCamera( g_debugCamera3D ); break;
				case DebugRenderCameraType::DEBUG_RENDER_CAMERA_2D	:	g_rendererReference->SetCamera( g_debugCamera2D ); break;
			}

			switch( m_options.m_mode )
			{
				case DebugRenderMode::DEBUG_RENDER_IGNORE_DEPTH		:	m_material->GetShader()->GetPass( 0 )->SetDepth( DepthTestCompare::COMPARE_ALWAYS, false );	break;
				case DebugRenderMode::DEBUG_RENDER_USE_DEPTH		:	m_material->GetShader()->GetPass( 0 )->SetDepth( DepthTestCompare::COMPARE_LESS, true );	break;
				case DebugRenderMode::DEBUG_RENDER_HIDDEN			:	m_material->GetShader()->GetPass( 0 )->SetDepth( DepthTestCompare::COMPARE_GREATER, false );	break;
				case DebugRenderMode::DEBUG_RENDER_XRAY				:	m_material->GetShader()->GetPass( 0 )->SetDepth( DepthTestCompare::COMPARE_LESS, true );	break;	// The additional draw call will be made after the first draw
			}

			m_material->SetProperty( "STARTCOLOR", m_options.m_startColor );
			m_material->SetProperty( "ENDCOLOR", m_options.m_endColor );
			m_material->SetProperty( "TIMENORMALIZED", GetNormalizedAge() );
			Renderable debugRenderable = Renderable( m_mesh, m_material, false, false );
			g_rendererReference->DrawRenderable( debugRenderable );

			if ( m_options.m_mode == DebugRenderMode::DEBUG_RENDER_XRAY )
			{
				m_material->GetShader()->GetPass( 0 )->SetDepth( DepthTestCompare::COMPARE_GREATER, false );
				m_material->SetProperty( "STARTCOLOR", m_options.m_startColor.GetInverse() );
				m_material->SetProperty( "ENDCOLOR", m_options.m_endColor.GetInverse() );
				g_rendererReference->DrawRenderable( debugRenderable );
			}
		}
	}

protected:
	virtual void CreateMesh()	// Should ensure that m_mesh is not null!
	{

	}

public:
	float m_timeToLive = 0.0f;
	DebugRenderOptions m_options;

protected:
	Mesh* m_mesh = nullptr;
	Material* m_material = nullptr;

};

class DebugRender2DQuadTask : public DebugRenderTask
{

public:
	DebugRender2DQuadTask( const AABB2& bounds )
		:	m_bounds( bounds )
	{
		CreateMesh();
	}

	explicit DebugRender2DQuadTask( const AABB2& bounds, DebugRenderOptions debugRenderOptions )
		:	m_bounds( bounds ),
			DebugRenderTask( debugRenderOptions )
	{
		CreateMesh();
	}

	explicit DebugRender2DQuadTask( const AABB2& bounds, const Rgba& startColor, const Rgba& endColor, float lifeTime = 0.0f, DebugRenderCameraType cameraType = DebugRenderCameraType::DEBUG_RENDER_CAMERA_2D, Clock* clock = nullptr )
		:	m_bounds( bounds ),
			DebugRenderTask( startColor, endColor, lifeTime, DebugRenderMode::DEBUG_RENDER_IGNORE_DEPTH, cameraType, clock )
	{
		CreateMesh();
	}

	~DebugRender2DQuadTask()
	{

	}

protected:
	void DebugRender2DQuadTask::CreateMesh() override
	{
		MeshBuilder meshBuilder;
		meshBuilder.Begin( DrawPrimitiveType::TRIANGLES, true );
		meshBuilder.SetColor( Rgba::WHITE );
		meshBuilder.SetUVs( Vector2::ZERO );

		Vertex_3DPCU vertices[ 4 ] =
		{
			Vertex_3DPCU( ConvertVector2ToVector3( m_bounds.mins ), Rgba::WHITE, Vector2::ZERO ),
			Vertex_3DPCU( ConvertVector2ToVector3( m_bounds.GetMinXMaxY() ), Rgba::WHITE, Vector2::ZERO ),
			Vertex_3DPCU( ConvertVector2ToVector3( m_bounds.GetMaxXMinY() ), Rgba::WHITE, Vector2::ZERO ),
			Vertex_3DPCU( ConvertVector2ToVector3( m_bounds.maxs ), Rgba::WHITE, Vector2::ZERO )
		};
		meshBuilder.PushVertices( 4, vertices );

		unsigned int indices[ 6 ] = { 0, 2, 1, 1, 2, 3 };
		meshBuilder.PushIndices( 6, indices );

		meshBuilder.End();
		m_mesh = meshBuilder.CreateMesh();
	}

protected:
	AABB2 m_bounds = AABB2::ZERO;

};

class DebugRender2DLineTask : public DebugRenderTask
{

public:
	DebugRender2DLineTask( const Vector2& start, const Vector2& end, const Rgba& startColor, const Rgba& endColor )
		:	m_position( start, end ),
			m_startColor( startColor ),
			m_endColor( endColor )
	{
		CreateMesh();
	}

	explicit DebugRender2DLineTask( const Vector2& start, const Vector2& end, const Rgba& startColor, const Rgba& endColor, DebugRenderOptions debugRenderOptions )
		:	m_position( start, end ),
			m_startColor( startColor ),
			m_endColor( endColor ),
			DebugRenderTask( debugRenderOptions )
	{
		CreateMesh();
	}

	explicit DebugRender2DLineTask( const Vector2& start, const Vector2& end, const Rgba& startColor, const Rgba& endColor, const Rgba& startColorTint, const Rgba& endColorTint, float lifeTime = 0.0f, DebugRenderCameraType cameraType = DebugRenderCameraType::DEBUG_RENDER_CAMERA_2D, Clock* clock = nullptr )
		:	m_position( start, end ),
			m_startColor( startColor ),
			m_endColor( endColor ),
			DebugRenderTask( startColorTint, endColorTint, lifeTime, DebugRenderMode::DEBUG_RENDER_IGNORE_DEPTH, cameraType, clock )
	{
		CreateMesh();
	}

	~DebugRender2DLineTask()
	{

	}

protected:
	void DebugRender2DLineTask::CreateMesh() override	// Creates a "star" along the positive and negative axes centered at the point
	{
		MeshBuilder meshBuilder;
		meshBuilder.Begin( DrawPrimitiveType::LINES, false );
		meshBuilder.SetUVs( Vector2::ZERO );

		meshBuilder.SetColor( m_startColor );
		meshBuilder.PushVertex( ConvertVector2ToVector3( m_position.mins ) );

		meshBuilder.SetColor( m_endColor );
		meshBuilder.PushVertex( ConvertVector2ToVector3( m_position.maxs ) );

		meshBuilder.End();
		m_mesh = meshBuilder.CreateMesh();
	}

protected:
	AABB2 m_position = AABB2::ZERO;
	Rgba m_startColor = Rgba::WHITE;
	Rgba m_endColor = Rgba::WHITE;

};

class DebugRender2DTextTask : public DebugRenderTask
{

public:
	DebugRender2DTextTask( const std::string& text, const Vector2& position, float cellHeight, float scale, const Vector2& alignment, const char* fontName )
		:	m_text( text ),
			m_position( position ),
			m_cellHeight( cellHeight ),
			m_scale( scale ),
			m_alignment( alignment )
	{
		m_font = g_rendererReference->CreateOrGetBitmapFont( fontName );
		m_material->SetTextureAndSampler( 0, m_font->GetTexture(), g_rendererReference->GetDefaultSampler() );

		CreateMesh();
	}

	explicit DebugRender2DTextTask( const std::string& text, const Vector2& position, float cellHeight, float scale, const Vector2& alignment, const char* fontName, DebugRenderOptions debugRenderOptions )
		:	m_text( text ),
			m_position( position ),
			m_cellHeight( cellHeight ),
			m_scale( scale ),
			m_alignment( alignment ),
			DebugRenderTask( debugRenderOptions )
	{
		m_font = g_rendererReference->CreateOrGetBitmapFont( fontName );
		m_material->SetTextureAndSampler( 0, m_font->GetTexture(), g_rendererReference->GetDefaultSampler() );

		CreateMesh();
	}

	explicit DebugRender2DTextTask( const char* fontName, const std::string& text, DebugRenderOptions options )		// Mainly used for Logs
		:	m_text( text ),
			DebugRenderTask( options )
	{
		m_font = g_rendererReference->CreateOrGetBitmapFont( fontName );
		m_material->SetTextureAndSampler( 0, m_font->GetTexture(), g_rendererReference->GetDefaultSampler() );
	}

	~DebugRender2DTextTask()
	{

	}

	void DebugRender2DTextTask::RegenerateMesh( const Vector2& position, float cellHeight, float scale = 1.0f, const Vector2& alignment = Vector2::ZERO )		// Mainly used for Logs
	{
		if ( m_mesh != nullptr )
		{
			delete m_mesh;
		}

		m_position = position;
		m_cellHeight = cellHeight;
		m_scale = scale;
		m_alignment = alignment;
		CreateMesh();
	}

protected:
	void DebugRender2DTextTask::RepositionFromAlignment()
	{
		float width = m_font->GetStringWidth( m_text, m_cellHeight, m_scale );
		float height = m_cellHeight;

		m_position -= Vector2( ( m_alignment.x * width ), ( m_alignment.y * height ) );
	}

	void DebugRender2DTextTask::CreateMesh() override	// Creates a "star" along the positive and negative axes centered at the point
	{
		RepositionFromAlignment();

		float characterWidth = m_font->GetCharacterWidth( m_cellHeight, m_scale );
		MeshBuilder meshBuilder = g_rendererReference->MakeTextMesh2D( m_position, m_text, m_cellHeight, characterWidth, Rgba::WHITE, m_font );
		m_mesh = meshBuilder.CreateMesh();
	}

protected:
	std::string m_text = "";
	float m_cellHeight;
	float m_scale = 0.0f;
	Vector2 m_position = Vector2::ZERO;
	Vector2 m_alignment = Vector2::ZERO;
	const BitmapFont* m_font = nullptr;

};

class DebugRenderPointTask : public DebugRenderTask
{

public:
	DebugRenderPointTask( const Vector3& position )
		:	m_position( position )
	{
		CreateMesh();
	}

	explicit DebugRenderPointTask( const Vector3& position, DebugRenderOptions debugRenderOptions )
		:	m_position( position ),
			DebugRenderTask( debugRenderOptions )
	{
		CreateMesh();
	}

	explicit DebugRenderPointTask( const Vector3& position, const Rgba& startColor, const Rgba& endColor, float lifeTime = 0.0f, DebugRenderMode mode = DebugRenderMode::DEBUG_RENDER_USE_DEPTH, DebugRenderCameraType cameraType = DebugRenderCameraType::DEBUG_RENDER_CAMERA_3D, Clock* clock = nullptr )
		:	m_position( position ),
			DebugRenderTask( startColor, endColor, lifeTime, mode, cameraType, clock )
	{
		CreateMesh();
	}

	~DebugRenderPointTask()
	{

	}

protected:
	void DebugRenderPointTask::CreateMesh() override	// Creates a "star" along the positive and negative axes centered at the point
	{
		MeshBuilder meshBuilder;
		meshBuilder.Begin( DrawPrimitiveType::LINES, true );
		meshBuilder.SetColor( Rgba::WHITE );
		meshBuilder.SetUVs( Vector2::ZERO );

		Vertex_3DPCU vertices[ 6 ] =
		{
			Vertex_3DPCU( ( m_position + Vector3::RIGHT ), Rgba::WHITE, Vector2::ZERO ),
			Vertex_3DPCU( ( m_position - Vector3::RIGHT ), Rgba::WHITE, Vector2::ZERO ),
			Vertex_3DPCU( ( m_position + Vector3::UP ), Rgba::WHITE, Vector2::ZERO ),
			Vertex_3DPCU( ( m_position - Vector3::UP ), Rgba::WHITE, Vector2::ZERO ),
			Vertex_3DPCU( ( m_position + Vector3::FORWARD ), Rgba::WHITE, Vector2::ZERO ),
			Vertex_3DPCU( ( m_position - Vector3::FORWARD ), Rgba::WHITE, Vector2::ZERO )
		};
		meshBuilder.PushVertices( 6, vertices );

		unsigned int indices[ 6 ] = { 0, 1, 2, 3, 4, 5 };
		meshBuilder.PushIndices( 6, indices );

		meshBuilder.End();
		m_mesh = meshBuilder.CreateMesh();
	}

protected:
	Vector3 m_position = Vector3::ZERO;

};

class DebugRenderLineSegmentTask : public DebugRenderTask
{

public:
	DebugRenderLineSegmentTask( const Vector3& start, const Vector3& end, const Rgba& startColor, const Rgba& endColor )
		:	m_position( start, end ),
			m_startColor( startColor ),
			m_endColor( endColor )
	{
		CreateMesh();
	}

	explicit DebugRenderLineSegmentTask( const Vector3& start, const Vector3& end, const Rgba& startColor, const Rgba& endColor, DebugRenderOptions debugRenderOptions )
		:	m_position( start, end ),
			m_startColor( startColor ),
			m_endColor( endColor ),
			DebugRenderTask( debugRenderOptions )
	{
		CreateMesh();
	}

	explicit DebugRenderLineSegmentTask( const Vector3& start, const Vector3& end, const Rgba& startColor, const Rgba& endColor, const Rgba& startColorTint, const Rgba& endColorTint, float lifeTime = 0.0f, DebugRenderMode mode = DebugRenderMode::DEBUG_RENDER_USE_DEPTH, DebugRenderCameraType cameraType = DebugRenderCameraType::DEBUG_RENDER_CAMERA_3D, Clock* clock = nullptr )
		:	m_position( start, end ),
			m_startColor( startColor ),
			m_endColor( endColor ),
			DebugRenderTask( startColorTint, endColorTint, lifeTime, mode, cameraType, clock )
	{
		CreateMesh();
	}

	~DebugRenderLineSegmentTask()
	{

	}

protected:
	void DebugRenderLineSegmentTask::CreateMesh() override	// Creates a "star" along the positive and negative axes centered at the point
	{
		MeshBuilder meshBuilder;
		meshBuilder.Begin( DrawPrimitiveType::LINES, false );
		meshBuilder.SetUVs( Vector2::ZERO );

		meshBuilder.SetColor( m_startColor );
		meshBuilder.PushVertex( m_position.mins );

		meshBuilder.SetColor( m_endColor );
		meshBuilder.PushVertex( m_position.maxs );

		meshBuilder.End();
		m_mesh = meshBuilder.CreateMesh();
	}

protected:
	AABB3 m_position = AABB3::ZERO;
	Rgba m_startColor = Rgba::WHITE;
	Rgba m_endColor = Rgba::WHITE;

};

class DebugRenderBasisTask : public DebugRenderTask
{

public:
	DebugRenderBasisTask( const Matrix44& basis )
		:	m_iBasis( basis.GetIBasis() ),
			m_jBasis( basis.GetJBasis() ),
			m_kBasis( basis.GetKBasis() ),
			m_position( basis. GetTranslation() )
	{
		CreateMesh();
	}

	explicit DebugRenderBasisTask( const Matrix44& basis, DebugRenderOptions debugRenderOptions )
		:	m_iBasis( basis.GetIBasis() ),
			m_jBasis( basis.GetJBasis() ),
			m_kBasis( basis.GetKBasis() ),
			m_position( basis. GetTranslation() ),
			DebugRenderTask( debugRenderOptions )
	{
		CreateMesh();
	}

	explicit DebugRenderBasisTask( const Matrix44& basis, const Rgba& startColor, const Rgba& endColor, float lifeTime = 0.0f, DebugRenderMode mode = DebugRenderMode::DEBUG_RENDER_USE_DEPTH, DebugRenderCameraType cameraType = DebugRenderCameraType::DEBUG_RENDER_CAMERA_3D, Clock* clock = nullptr )
		:	m_iBasis( basis.GetIBasis() ),
			m_jBasis( basis.GetJBasis() ),
			m_kBasis( basis.GetKBasis() ),
			m_position( basis. GetTranslation() ),
			DebugRenderTask( startColor, endColor, lifeTime, mode, cameraType, clock )
	{
		CreateMesh();
	}

	~DebugRenderBasisTask()
	{

	}

protected:
	void DebugRenderBasisTask::CreateMesh() override	// Creates a "star" along the positive and negative axes centered at the point
	{
		MeshBuilder meshBuilder;
		meshBuilder.Begin( DrawPrimitiveType::LINES, false );
		meshBuilder.SetUVs( Vector2::ZERO );

		meshBuilder.SetColor( Rgba::RED );
		meshBuilder.PushVertex( m_position );
		meshBuilder.PushVertex( m_position + m_iBasis );

		meshBuilder.SetColor( Rgba::GREEN );
		meshBuilder.PushVertex( m_position );
		meshBuilder.PushVertex( m_position + m_jBasis );

		meshBuilder.SetColor( Rgba::BLUE );
		meshBuilder.PushVertex( m_position );
		meshBuilder.PushVertex( m_position + m_kBasis );

		meshBuilder.End();
		m_mesh = meshBuilder.CreateMesh();
	}

protected:
	Vector3 m_iBasis = Vector3::RIGHT;
	Vector3 m_jBasis = Vector3::UP;
	Vector3 m_kBasis = Vector3::FORWARD;
	Vector3 m_position = Vector3::ZERO;

};

class DebugRenderSphereTask : public DebugRenderTask
{

public:
	DebugRenderSphereTask( const Vector3& position, float radius, unsigned int numWedges, unsigned int numSlices )
		:	m_position( position ),
			m_radius( radius ),
			m_numSlices( numSlices ),
			m_numWedges( numWedges )
	{
		CreateMesh();
	}

	explicit DebugRenderSphereTask( const Vector3& position, float radius, unsigned int numWedges, unsigned int numSlices, DebugRenderOptions debugRenderOptions )
		:	m_position( position ),
			m_radius( radius ),
			m_numSlices( numSlices ),
			m_numWedges( numWedges ),
			DebugRenderTask( debugRenderOptions )
	{
		CreateMesh();
	}

	explicit DebugRenderSphereTask( const Vector3& position, float radius, unsigned int numWedges, unsigned int numSlices, const Rgba& startColor, const Rgba& endColor, float lifeTime = 0.0f, DebugRenderMode mode = DebugRenderMode::DEBUG_RENDER_USE_DEPTH, DebugRenderCameraType cameraType = DebugRenderCameraType::DEBUG_RENDER_CAMERA_3D, Clock* clock = nullptr )
		:	m_position( position ),
			m_radius( radius ),
			m_numSlices( numSlices ),
			m_numWedges( numWedges ),
			DebugRenderTask( startColor, endColor, lifeTime, mode, cameraType, clock )
	{
		CreateMesh();
	}

	~DebugRenderSphereTask()
	{

	}

protected:
	void DebugRenderSphereTask::CreateMesh() override
	{
		MeshBuilder meshBuilder = g_rendererReference->MakeUVSphereMesh( m_position, m_radius, m_numWedges, m_numSlices, Rgba::WHITE );
		m_mesh = meshBuilder.CreateMesh();
	}

protected:
	Vector3 m_position = Vector3::ZERO;
	float m_radius = 0.0f;
	unsigned int m_numSlices = 0U;
	unsigned int m_numWedges = 0U;
};

class DebugRenderQuadTask : public DebugRenderTask
{

public:
	DebugRenderQuadTask( const Vector3& position, const Vector3& up, const Vector3& right, float minX, float maxX, float minY, float maxY, const AABB2& UVs = AABB2::ZERO_TO_ONE, const Texture* texture = nullptr )
		:	m_position( position ),
			m_up( up ),
			m_right( right ),
			m_bounds( Vector2( minX, minY ), Vector2( maxX, maxY ) ),
			m_UVs( UVs ),
			m_texture( texture )
	{
		if ( texture == nullptr )
		{
			texture = g_rendererReference->GetDefaultTexture();
		}
		m_material->SetTextureAndSampler( 0, m_texture, g_rendererReference->GetDefaultSampler() );
		CreateMesh();
	}

	explicit DebugRenderQuadTask( const Vector3& position, const Vector3& up, const Vector3& right, float minX, float maxX, float minY, float maxY, DebugRenderOptions debugRenderOptions, const AABB2& UVs = AABB2::ZERO_TO_ONE, const Texture* texture = nullptr )
		:	m_position( position ),
			m_up( up ),
			m_right( right ),
			m_bounds( Vector2( minX, minY ), Vector2( maxX, maxY ) ),
			m_UVs( UVs ),
			m_texture( texture ),
			DebugRenderTask( debugRenderOptions )
	{
		if ( texture == nullptr )
		{
			texture = g_rendererReference->GetDefaultTexture();
		}
		m_material->SetTextureAndSampler( 0, m_texture, g_rendererReference->GetDefaultSampler() );
		CreateMesh();
	}

	explicit DebugRenderQuadTask( const Vector3& position, const Vector3& up, const Vector3& right, float minX, float maxX, float minY, float maxY, const Rgba& startColor, const Rgba& endColor, float lifeTime = 0.0f, DebugRenderCameraType cameraType = DebugRenderCameraType::DEBUG_RENDER_CAMERA_3D, Clock* clock = nullptr, const AABB2& UVs = AABB2::ZERO_TO_ONE, const Texture* texture = nullptr )
		:	m_position( position ),
			m_up( up ),
			m_right( right ),
			m_bounds( Vector2( minX, minY ), Vector2( maxX, maxY ) ),
			m_UVs( UVs ),
			m_texture( texture ),
			DebugRenderTask( startColor, endColor, lifeTime, DebugRenderMode::DEBUG_RENDER_IGNORE_DEPTH, cameraType, clock )
	{
		if ( texture == nullptr )
		{
			texture = g_rendererReference->GetDefaultTexture();
		}
		m_material->SetTextureAndSampler( 0, m_texture, g_rendererReference->GetDefaultSampler() );
		CreateMesh();
	}

	~DebugRenderQuadTask()
	{

	}

protected:
	Vector3 DebugRenderQuadTask::GetBottomLeft() const
	{
		return ( m_position + ( m_bounds.mins.x * m_right ) + ( m_bounds.mins.y * m_up ) );
	}

	Vector3 DebugRenderQuadTask::GetTopLeft() const
	{
		return ( m_position + ( m_bounds.mins.x * m_right ) + ( m_bounds.maxs.y * m_up ) );
	}

	Vector3 DebugRenderQuadTask::GetBottomRight() const
	{
		return ( m_position + ( m_bounds.maxs.x * m_right ) + ( m_bounds.mins.y * m_up ) );
	}

	Vector3 DebugRenderQuadTask::GetTopRight() const
	{
		return ( m_position + ( m_bounds.maxs.x * m_right ) + ( m_bounds.maxs.y * m_up ) );
	}

	void DebugRenderQuadTask::CreateMesh() override
	{
		MeshBuilder meshBuilder;
		meshBuilder.Begin( DrawPrimitiveType::TRIANGLES, true );

		Vertex_3DPCU vertices[ 4 ] =
		{
			Vertex_3DPCU( GetBottomLeft(), Rgba::WHITE, m_UVs.mins ),
			Vertex_3DPCU( GetTopLeft(), Rgba::WHITE, m_UVs.GetMinXMaxY() ),
			Vertex_3DPCU( GetBottomRight(), Rgba::WHITE, m_UVs.GetMaxXMinY() ),
			Vertex_3DPCU( GetTopRight(), Rgba::WHITE, m_UVs.maxs )
		};
		meshBuilder.PushVertices( 4, vertices );

		unsigned int indices[ 6 ] = { 0, 2, 1, 1, 2, 3 };
		meshBuilder.PushIndices( 6, indices );

		meshBuilder.End();
		m_mesh = meshBuilder.CreateMesh();
	}

protected:
	Vector3 m_position = Vector3::ZERO;
	Vector3 m_right = Vector3::RIGHT;
	Vector3 m_up = Vector3::UP;
	AABB2 m_bounds = AABB2::MINUS_ONE_TO_ONE;
	AABB2 m_UVs = AABB2::ZERO_TO_ONE;
	const Texture* m_texture = nullptr;

};

class DebugRenderGlyphTask : public DebugRenderTask
{

public:
	DebugRenderGlyphTask( const Vector3& position, float minX, float maxX, float minY, float maxY, const AABB2& UVs = AABB2::ZERO_TO_ONE, const Texture* texture = nullptr )
		:	m_position( position ),
			m_bounds( Vector2( minX, minY ), Vector2( maxX, maxY ) ),
			m_UVs( UVs ),
			m_texture( texture )
	{
		if ( texture == nullptr )
		{
			texture = g_rendererReference->GetDefaultTexture();
		}
		m_material->SetTextureAndSampler( 0, m_texture, g_rendererReference->GetDefaultSampler() );
		CreateMesh();
	}

	explicit DebugRenderGlyphTask( const Vector3& position, float minX, float maxX, float minY, float maxY, DebugRenderOptions debugRenderOptions, const AABB2& UVs = AABB2::ZERO_TO_ONE, const Texture* texture = nullptr )
		:	m_position( position ),
			m_bounds( Vector2( minX, minY ), Vector2( maxX, maxY ) ),
			m_UVs( UVs ),
			m_texture( texture ),
			DebugRenderTask( debugRenderOptions )
	{
		if ( texture == nullptr )
		{
			texture = g_rendererReference->GetDefaultTexture();
		}
		m_material->SetTextureAndSampler( 0, m_texture, g_rendererReference->GetDefaultSampler() );
		CreateMesh();
	}

	explicit DebugRenderGlyphTask( const Vector3& position, float minX, float maxX, float minY, float maxY, const Rgba& startColor, const Rgba& endColor, float lifeTime = 0.0f, DebugRenderCameraType cameraType = DebugRenderCameraType::DEBUG_RENDER_CAMERA_3D, Clock* clock = nullptr, const AABB2& UVs = AABB2::ZERO_TO_ONE, const Texture* texture = nullptr )
		:	m_position( position ),
			m_bounds( Vector2( minX, minY ), Vector2( maxX, maxY ) ),
			m_UVs( UVs ),
			m_texture( texture ),
			DebugRenderTask( startColor, endColor, lifeTime, DebugRenderMode::DEBUG_RENDER_IGNORE_DEPTH, cameraType, clock )
	{
		if ( texture == nullptr )
		{
			texture = g_rendererReference->GetDefaultTexture();
		}
		m_material->SetTextureAndSampler( 0, m_texture, g_rendererReference->GetDefaultSampler() );
		CreateMesh();
	}

	~DebugRenderGlyphTask()
	{

	}

	void DebugRenderGlyphTask::Update() override
	{
		if ( m_mesh != nullptr )
		{
			delete m_mesh;
		}
		CreateMesh();
	}

protected:

	void DebugRenderGlyphTask::CreateMesh() override
	{
		g_rendererReference->SetCamera( g_debugCamera3D );
		RepositionFromAlignment();

		MeshBuilder meshBuilder = g_rendererReference->MakeTexturedOrientedQuadMesh( m_drawPosition, g_debugCamera3D->GetRight(), ( Vector2::ONE * 0.5f ), m_bounds.GetDimensions(), *m_texture, m_UVs.mins, m_UVs.maxs, Rgba::WHITE );
		m_mesh = meshBuilder.CreateMesh();
	}

	void DebugRenderGlyphTask::RepositionFromAlignment()
	{
		m_drawPosition = m_position + ( m_bounds.mins.x ) * g_debugCamera3D->GetRight();
		m_drawPosition = m_drawPosition + ( m_bounds.mins.y ) * g_debugCamera3D->GetUp();
	}

protected:
	Vector3 m_position = Vector3::ZERO;
	Vector3 m_drawPosition = Vector3::ZERO;
	AABB2 m_bounds = AABB2::MINUS_ONE_TO_ONE;
	AABB2 m_UVs = AABB2::ZERO_TO_ONE;
	const Texture* m_texture = nullptr;

};

class DebugRenderAABB3Task	:	public DebugRenderTask
{

public:
	DebugRenderAABB3Task( const Vector3& position, const Vector3& dimensions )
		:	m_position( position ),
			m_dimensions( dimensions )
	{
		CreateMesh();
	}

	explicit DebugRenderAABB3Task( const Vector3& position, const Vector3& dimensions, DebugRenderOptions debugRenderOptions )
		:	m_position( position ),
			m_dimensions( dimensions ),
			DebugRenderTask( DebugRenderOptions( debugRenderOptions ) )
	{
		CreateMesh();
	}

	explicit DebugRenderAABB3Task( const Vector3& position, const Vector3& dimensions, const Rgba& startColor, const Rgba& endColor, float lifeTime = 0.0f, DebugRenderMode mode = DebugRenderMode::DEBUG_RENDER_USE_DEPTH, DebugRenderCameraType cameraType = DebugRenderCameraType::DEBUG_RENDER_CAMERA_3D, Clock* clock = nullptr )
		:	m_position( position ),
			m_dimensions( dimensions ),
			DebugRenderTask( startColor, endColor, lifeTime, mode, cameraType, clock )
	{
		CreateMesh();
	}

	~DebugRenderAABB3Task()
	{

	}

protected:
	void DebugRenderAABB3Task::CreateMesh() override
	{
		MeshBuilder meshBuilder = g_rendererReference->MakeCubeMesh( m_position, m_dimensions, Rgba::WHITE );
		m_mesh = meshBuilder.CreateMesh();
	}

protected:
	Vector3 m_position = Vector3::ZERO;
	Vector3 m_dimensions = Vector3::ZERO;

};

class DebugRenderOBB3Task	:	public DebugRenderTask
{

public:
	explicit DebugRenderOBB3Task( const Vector3& center, const Vector3& right, const Vector3& up, const Vector3& forward )
		:	m_center( center ),
			m_right( right ),
			m_up( up ),
			m_forward( forward )
	{
		CreateMesh();
	}

	explicit DebugRenderOBB3Task( const Vector3& center, const Vector3& right, const Vector3& up, const Vector3& forward, DebugRenderOptions debugRenderOptions )
		:	m_center( center ),
			m_right( right ),
			m_up( up ),
			m_forward( forward ),
			DebugRenderTask( DebugRenderOptions( debugRenderOptions ) )
	{
		CreateMesh();
	}

	explicit DebugRenderOBB3Task( const Vector3& center, const Vector3& right, const Vector3& up, const Vector3& forward, const Rgba& startColor, const Rgba& endColor, float lifeTime = 0.0f, DebugRenderMode mode = DebugRenderMode::DEBUG_RENDER_USE_DEPTH, DebugRenderCameraType cameraType = DebugRenderCameraType::DEBUG_RENDER_CAMERA_3D, Clock* clock = nullptr )
		:	m_center( center ),
			m_right( right ),
			m_up( up ),
			m_forward( forward ),
			DebugRenderTask( startColor, endColor, lifeTime, mode, cameraType, clock )
	{
		CreateMesh();
	}

	~DebugRenderOBB3Task()
	{

	}

protected:
	void DebugRenderOBB3Task::CreateMesh() override
	{
		MeshBuilder meshBuilder = g_rendererReference->MakeOBBMesh( m_center, m_right, m_up, m_forward, Rgba::WHITE );
		m_mesh = meshBuilder.CreateMesh();
	}

protected:
	Vector3 m_center = Vector3::ZERO;
	Vector3 m_right = Vector3::RIGHT;
	Vector3 m_up = Vector3::UP;
	Vector3 m_forward = Vector3::FORWARD;

};

class DebugRenderGridTask : public DebugRenderTask
{

public:
public:
	DebugRenderGridTask( const Vector3& position, const Vector3& up, const Vector3& right, float minX, float maxX, float minY, float maxY, const IntVector2& numUnits )
		:	m_position( position ),
			m_up( up ),
			m_right( right ),
			m_bounds( Vector2( minX, minY ), Vector2( maxX, maxY ) ),
			m_numUnits( numUnits )
	{
		CreateMesh();
	}

	explicit DebugRenderGridTask( const Vector3& position, const Vector3& up, const Vector3& right, float minX, float maxX, float minY, float maxY, const IntVector2& numUnits, DebugRenderOptions debugRenderOptions )
		:	m_position( position ),
			m_up( up ),
			m_right( right ),
			m_bounds( Vector2( minX, minY ), Vector2( maxX, maxY ) ),
			m_numUnits( numUnits ),
			DebugRenderTask( debugRenderOptions )
	{
		CreateMesh();
	}

	explicit DebugRenderGridTask( const Vector3& position, const Vector3& up, const Vector3& right, float minX, float maxX, float minY, float maxY, const IntVector2& numUnits, const Rgba& startColor, const Rgba& endColor, float lifeTime = 0.0f, DebugRenderCameraType cameraType = DebugRenderCameraType::DEBUG_RENDER_CAMERA_3D, Clock* clock = nullptr )
		:	m_position( position ),
			m_up( up ),
			m_right( right ),
			m_bounds( Vector2( minX, minY ), Vector2( maxX, maxY ) ),
			m_numUnits( numUnits ),
			DebugRenderTask( startColor, endColor, lifeTime, DebugRenderMode::DEBUG_RENDER_IGNORE_DEPTH, cameraType, clock )
	{
		CreateMesh();
	}

	~DebugRenderGridTask()
	{

	}

protected:
	Vector3 DebugRenderGridTask::GetBottomLeft() const
	{
		return ( m_position + ( m_bounds.mins.x * m_right ) + ( m_bounds.mins.y * m_up ) );
	}

	Vector3 DebugRenderGridTask::GetTopLeft() const
	{
		return ( m_position + ( m_bounds.mins.x * m_right ) + ( m_bounds.maxs.y * m_up ) );
	}

	Vector3 DebugRenderGridTask::GetBottomRight() const
	{
		return ( m_position + ( m_bounds.maxs.x * m_right ) + ( m_bounds.mins.y * m_up ) );
	}

	Vector3 DebugRenderGridTask::GetTopRight() const
	{
		return ( m_position + ( m_bounds.maxs.x * m_right ) + ( m_bounds.maxs.y * m_up ) );
	}

	void DebugRenderGridTask::CreateMesh() override
	{
		MeshBuilder meshBuilder = g_rendererReference->MakeGridMesh( m_position, m_bounds, m_up, m_right, m_numUnits, Rgba::WHITE );
		m_mesh = meshBuilder.CreateMesh();
	}

protected:
	Vector3 m_position = Vector3::ZERO;
	Vector3 m_right = Vector3::RIGHT;
	Vector3 m_up = Vector3::UP;
	AABB2 m_bounds = AABB2::MINUS_ONE_TO_ONE;
	IntVector2 m_numUnits = IntVector2::ZERO;

};

class DebugRenderTagTask : public DebugRenderTask
{

public:
	DebugRenderTagTask( const std::string& text, const Vector3& position, float cellHeight, float scale, const Vector2& alignment, const char* fontName )
		:	m_text( text ),
			m_position( position ),
			m_cellHeight( cellHeight ),
			m_scale( scale ),
			m_alignment( alignment )
	{
		m_font = g_rendererReference->CreateOrGetBitmapFont( fontName );
		m_material->SetTextureAndSampler( 0, m_font->GetTexture(), g_rendererReference->GetDefaultSampler() );
	}

	explicit DebugRenderTagTask( const std::string& text, const Vector3& position, float cellHeight, float scale, const Vector2& alignment, const char* fontName, DebugRenderOptions debugRenderOptions )
		:	m_text( text ),
			m_position( position ),
			m_cellHeight( cellHeight ),
			m_scale( scale ),
			m_alignment( alignment ),
			DebugRenderTask( debugRenderOptions )
	{
		m_font = g_rendererReference->CreateOrGetBitmapFont( fontName );
		m_material->SetTextureAndSampler( 0, m_font->GetTexture(), g_rendererReference->GetDefaultSampler() );
	}

	~DebugRenderTagTask()
	{

	}

	void DebugRenderTagTask::Update() override
	{
		if ( m_mesh != nullptr )
		{
			delete m_mesh;
		}
		CreateMesh();
	}

protected:
	void DebugRenderTagTask::RepositionFromAlignment()
	{
		float width = m_font->GetStringWidth( m_text, m_cellHeight, m_scale );
		float height = m_cellHeight;

		m_drawPosition = m_position - ( m_alignment.x * width ) * g_debugCamera3D->GetRight();
		m_drawPosition = m_drawPosition - ( m_alignment.y * height ) * g_debugCamera3D->GetUp();
	}

	void DebugRenderTagTask::CreateMesh() override
	{
		g_rendererReference->SetCamera( g_debugCamera3D );
		RepositionFromAlignment();

		float characterWidth = m_font->GetCharacterWidth( m_cellHeight, m_scale );
		MeshBuilder meshBuilder = g_rendererReference->MakeTextMesh3D( m_text, m_drawPosition, g_debugCamera3D->GetRight(), m_cellHeight, characterWidth, Rgba::WHITE, m_font );
		m_mesh = meshBuilder.CreateMesh();
	}

protected:
	std::string m_text = "";
	float m_cellHeight;
	float m_scale = 0.0f;
	Vector3 m_drawPosition = Vector3::ZERO;
	Vector3 m_position = Vector3::ZERO;
	Vector2 m_alignment = Vector2::ZERO;
	const BitmapFont* m_font = nullptr;

};

class DebugRenderTextTask : public DebugRenderTask
{

public:
	DebugRenderTextTask( const std::string& text, const Vector3& position, const Vector3& right, const Vector3& up, float cellHeight, float scale, const Vector2& alignment, const char* fontName )
		:	m_text( text ),
			m_position( position ),
			m_right( right ),
			m_up( up ),
			m_cellHeight( cellHeight ),
			m_scale( scale ),
			m_alignment( alignment )
	{
		m_font = g_rendererReference->CreateOrGetBitmapFont( fontName );
		m_material->SetTextureAndSampler( 0, m_font->GetTexture(), g_rendererReference->GetDefaultSampler() );
		RepositionFromAlignment();
		CreateMesh();
	}

	explicit DebugRenderTextTask( const std::string& text, const Vector3& position, const Vector3& right, const Vector3& up, float cellHeight, float scale, const Vector2& alignment, const char* fontName, DebugRenderOptions debugRenderOptions )
		:	m_text( text ),
			m_position( position ),
			m_right( right ),
			m_up( up ),
			m_cellHeight( cellHeight ),
			m_scale( scale ),
			m_alignment( alignment ),
			DebugRenderTask( debugRenderOptions )
	{
		m_font = g_rendererReference->CreateOrGetBitmapFont( fontName );
		m_material->SetTextureAndSampler( 0, m_font->GetTexture(), g_rendererReference->GetDefaultSampler() );
		RepositionFromAlignment();
		CreateMesh();
	}

	~DebugRenderTextTask()
	{

	}

protected:
	void DebugRenderTextTask::RepositionFromAlignment()
	{
		float width = m_font->GetStringWidth( m_text, m_cellHeight, m_scale );
		float height = m_cellHeight;

		m_position -= ( m_alignment.x * width ) * m_right;
		m_position -= ( m_alignment.y * height ) * m_up;
	}

	void DebugRenderTextTask::CreateMesh() override
	{
		float characterWidth = m_font->GetCharacterWidth( m_cellHeight, m_scale );
		MeshBuilder meshBuilder = g_rendererReference->MakeTextMesh3D( m_text, m_position, m_right, m_cellHeight, characterWidth, Rgba::WHITE, m_font, false, m_up );
		m_mesh = meshBuilder.CreateMesh();
	}

protected:
	std::string m_text = "";
	float m_cellHeight;
	float m_scale = 0.0f;
	Vector3 m_position = Vector3::ZERO;
	Vector3 m_right = Vector3::ZERO;
	Vector3 m_up = Vector3::ZERO;
	Vector2 m_alignment = Vector2::ZERO;
	const BitmapFont* m_font = nullptr;

};

#pragma endregion

#pragma region LifeTimeFunctions

void InitializeDebugMaterials()
{
	g_debugRenderMaterial = new Material( g_rendererReference->CreateOrGetShader( DEBUG_SOLID_SHADER_NAME ) );
	g_debugRenderWireframeMaterial = new Material( g_rendererReference->CreateOrGetShader( DEBUG_WIREFRAME_SHADER_NAME ) );
	g_defaultLitMaterial = new Material( g_rendererReference->CreateOrGetShader( GetMeshRenderModeName( MeshRenderMode::MODE_FINAL ) ) );
	g_diffuseMaterial = new Material( g_rendererReference->CreateOrGetShader( GetMeshRenderModeName( MeshRenderMode::MODE_DIFFUSE ) ) );
	g_specularMaterial = new Material( g_rendererReference->CreateOrGetShader( GetMeshRenderModeName( MeshRenderMode::MODE_SPECULAR ) ) );
	g_diffuseAndSpecularMaterial = new Material( g_rendererReference->CreateOrGetShader( GetMeshRenderModeName( MeshRenderMode::MODE_DIFFUSE_AND_SPECULAR ) ) );
	g_UVMaterial = new Material( g_rendererReference->CreateOrGetShader( GetMeshRenderModeName( MeshRenderMode::MODE_UV ) ) );
	g_surfaceNormalMaterial = new Material( g_rendererReference->CreateOrGetShader( GetMeshRenderModeName( MeshRenderMode::MODE_SURFACE_NORMAL ) ) );
	g_worldNormalMaterial = new Material( g_rendererReference->CreateOrGetShader( GetMeshRenderModeName( MeshRenderMode::MODE_WORLD_NORMAL ) ) );
	g_vertexNormalMaterial = new Material( g_rendererReference->CreateOrGetShader( GetMeshRenderModeName( MeshRenderMode::MODE_VERTEX_NORMAL ) ) );
	g_vertexTangentMaterial = new Material( g_rendererReference->CreateOrGetShader( GetMeshRenderModeName( MeshRenderMode::MODE_VERTEX_TANGENT ) ) );
	g_vertexBitangentMaterial = new Material( g_rendererReference->CreateOrGetShader( GetMeshRenderModeName( MeshRenderMode::MODE_VERTEX_BITANGENT ) ) );
	g_colorOnlyMaterial = new Material( g_rendererReference->CreateOrGetShader( GetMeshRenderModeName( MeshRenderMode::MODE_COLOR_ONLY ) ) );
}

void DeleteDebugMaterials()
{
	delete g_debugRenderMaterial;
	delete g_debugRenderWireframeMaterial;
	delete g_defaultLitMaterial;
	delete g_diffuseMaterial;
	delete g_specularMaterial;
	delete g_diffuseAndSpecularMaterial;
	delete g_UVMaterial;
	delete g_surfaceNormalMaterial;
	delete g_worldNormalMaterial;
	delete g_vertexNormalMaterial;
	delete g_vertexTangentMaterial;
	delete g_vertexBitangentMaterial;
	delete g_colorOnlyMaterial;
}

void DebugRenderStartup( Renderer* renderer, Camera* camera3D /* = nullptr */, Camera* camera2D /* = nullptr */, bool enableOnStart /* = false */ )
{
	if ( renderer == nullptr )
	{
		ERROR_AND_DIE( "DebugRenderSystem - DebugRenderStartup: Renderer cannot be null. Aborting..." );
	}
	g_rendererReference = renderer;

	if ( camera3D == nullptr )
	{
		camera3D = new Camera( g_rendererReference->GetDefaultColorTarget(), g_rendererReference->GetDefaultDepthStencilTarget() );
		camera3D->SetProjection( Matrix44::MakePerspective( DEBUG_SYSTEM_3D_CAMERA_FOV_DEFAULT, Window::GetAspect(), DEBUG_SYSTEM_3D_CAMERA_NEAR_Z_DEFAULT, DEBUG_SYSTEM_3D_CAMERA_FAR_Z_DEFAULT ) );
		g_usesOwnCamera3D = true;
	}

	if ( camera2D == nullptr )
	{
		camera2D = new Camera( g_rendererReference->GetDefaultColorTarget(), g_rendererReference->GetDefaultDepthStencilTarget() );
		camera2D->SetProjectionOrtho( Window::GetHeightF(), Window::GetAspect(), -1.0f, 1.0f );
		g_usesOwnCamera2D = true;
	}

	g_debugCamera3D = camera3D;
	g_debugCamera2D = camera2D;

	g_debugRendererEnabled = enableOnStart;

	CommandRegister( "debug_toggle", ToggleDebugRenderCommand, "Toggles Debug rendering on/off." );
	CommandRegister( "clear_debug", ClearDebugRenderQueueCommand, "Clears all currently active debug render tasks, including logs." );
	CommandRegister( "clear_debug_log", ClearDebugRenderLogCommand, "Clears all currently active debug render logs." );

	InitializeDebugMaterials();
}

void ClearLogTasks()
{
	for ( DebugRender2DTextTask* task : g_debugRenderLogTasks )
	{
		delete task;
	}
	g_debugRenderLogTasks.clear();
}

void ClearAllDebugRenderTasks()
{
	for ( DebugRenderTask* task : g_debugRenderTasks )
	{
		delete task;
	}
	g_debugRenderTasks.clear();
	ClearLogTasks();
}

void DebugRenderShutdown()
{
	DeleteDebugMaterials();

	ClearAllDebugRenderTasks();

	if ( g_usesOwnCamera3D )
	{
		delete g_debugCamera3D;
	}
	if ( g_usesOwnCamera2D )
	{
		delete g_debugCamera2D;
	}

	g_debugCamera3D = nullptr;
	g_debugCamera2D = nullptr;
	g_rendererReference = nullptr;
}

void SetLogRenderPositions()
{
	if ( g_shouldUpdateLog )
	{
		float lineHeight = Window::GetHeightF() * 0.01f;
		Vector2 lineIncrements = Vector2( 0.0f, ( -1.5f * lineHeight ) );
		Vector2 startPosition = Vector2( -( Window::GetWidthF() * 0.5f ), ( ( Window::GetHeightF() * 0.5f ) - lineHeight ) );

		for ( DebugRender2DTextTask* task : g_debugRenderLogTasks )
		{
			task->RegenerateMesh( startPosition, lineHeight, 1.0f, Vector2( 0.0f, 0.5f ) );
			startPosition += lineIncrements;
		}

		g_shouldUpdateLog = true;
	}
}

void AgeTasks()
{
	for ( DebugRenderTask* task : g_debugRenderTasks )
	{
		task->Age();
	}
	for ( DebugRender2DTextTask* task : g_debugRenderLogTasks )
	{
		task->Age();
	}
}

void UpdateTasks()
{
	for ( DebugRenderTask* task : g_debugRenderTasks )
	{
		task->Update();
	}
	for ( DebugRenderTask* task : g_debugRenderLogTasks )
	{
		task->Update();
	}
}

void RenderTasks()
{
	for ( DebugRenderTask* task : g_debugRenderTasks )
	{
		task->Render();
	}
	for ( DebugRenderTask* task : g_debugRenderLogTasks )
	{
		task->Render();
	}
}

void DeleteExpiredTasks()
{
	for ( size_t taskIndex = 0; taskIndex < g_debugRenderTasks.size(); taskIndex++ )	// Performs Fast Removal on expired tasks
	{
		DebugRenderTask* task = g_debugRenderTasks[ taskIndex ];

		if ( task->HasExpired() )
		{
			delete task;
			g_debugRenderTasks[ taskIndex ] = g_debugRenderTasks[ g_debugRenderTasks.size() - 1 ];
			g_debugRenderTasks.pop_back();
			taskIndex--;
		}
	}
	for ( size_t taskIndex = 0; taskIndex < g_debugRenderLogTasks.size(); taskIndex++ )	// Performs Fast Removal on expired tasks
	{
		DebugRender2DTextTask* task = g_debugRenderLogTasks[ taskIndex ];

		if ( task->HasExpired() )
		{
			delete task;
			g_debugRenderLogTasks[ taskIndex ] = g_debugRenderLogTasks[ g_debugRenderLogTasks.size() - 1 ];
			g_debugRenderLogTasks.pop_back();
			taskIndex--;

			g_shouldUpdateLog = true;
		}
	}
}

void DebugRenderUpdateAndRender()
{
	if ( g_debugRendererEnabled && ( ( g_debugRenderTasks.size() > 0 ) || ( g_debugRenderLogTasks.size() > 0 ) ) )
	{
		SetLogRenderPositions();
		UpdateTasks();
		RenderTasks();	// Render first to allow Tasks with lifeTime = 0 to render for one frame
		AgeTasks();
		DeleteExpiredTasks();
	}
}

void DebugRenderSet3DCamera( Camera *camera )
{
	if ( g_usesOwnCamera3D )
	{
		delete g_debugCamera3D;
		g_usesOwnCamera3D = false;
	}

	g_debugCamera3D = camera;
}

void DebugRenderSet2DCamera( Camera* camera )
{
	if ( g_usesOwnCamera2D )
	{
		delete g_debugCamera2D;
		g_usesOwnCamera2D = false;
	}

	g_debugCamera2D = camera;
}

void EnableDebugRenderer( bool enabled /* = true */ )
{
	g_debugRendererEnabled = enabled;
}

bool IsDebugRenderingEnabled()
{
	return g_debugRendererEnabled;
}

#pragma endregion

#pragma region 2DUtils

void DebugRender2DQuad( float lifeTime, const AABB2& bounds, const Rgba& startColor, const Rgba& endColor, const Clock* clock /* = nullptr */ )
{
	g_debugRenderTasks.push_back( new DebugRender2DQuadTask( bounds, DebugRenderOptions( startColor, endColor, lifeTime, DebugRenderMode::DEBUG_RENDER_IGNORE_DEPTH, DebugRenderCameraType::DEBUG_RENDER_CAMERA_2D, clock ) ) );
}

void DebugRender2DLine( float lifeTime, const Vector2& p0, const Rgba& p0Color, const Vector2& p1, const Rgba& p1Color, const Rgba& startColor, const Rgba& endColor, const Clock* clock /* = nullptr */ )
{
	g_debugRenderTasks.push_back( new DebugRender2DLineTask( p0, p1, p0Color, p1Color, DebugRenderOptions( startColor, endColor, lifeTime, DebugRenderMode::DEBUG_RENDER_IGNORE_DEPTH, DebugRenderCameraType::DEBUG_RENDER_CAMERA_2D, clock ) ) );
}

void DebugRender2DText( float lifeTime, const Vector2& position, float cellHeight, float scale, const Vector2& alignment, const Rgba& startColor, const Rgba& endColor, const Clock* clock, const char* fontName, const char* format, ... )
{
	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	std::string text = Stringv( format, variableArgumentList );
	va_end( variableArgumentList );

	g_debugRenderTasks.push_back( new DebugRender2DTextTask( text, position, cellHeight, scale, alignment, fontName, DebugRenderOptions( startColor, endColor, lifeTime, DebugRenderMode::DEBUG_RENDER_IGNORE_DEPTH, DebugRenderCameraType::DEBUG_RENDER_CAMERA_2D, clock ) ) );
}

void DebugRenderLogF( float lifeTime, const Rgba& startColor, const Rgba& endColor, const Clock* clock, const char* fontName, const char* format, ... )
{
	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	std::string text = Stringv( format, variableArgumentList );
	va_end( variableArgumentList );

	g_debugRenderLogTasks.push_back( new DebugRender2DTextTask( fontName, text, DebugRenderOptions( startColor, endColor, lifeTime, DebugRenderMode::DEBUG_RENDER_IGNORE_DEPTH, DebugRenderCameraType::DEBUG_RENDER_CAMERA_2D, clock ) ) );
	g_shouldUpdateLog = true;
}

#pragma endregion

#pragma region 3DUtils

void DebugRenderPoint( float lifeTime, const Vector3& position, const Rgba& startColor, const Rgba& endColor, const DebugRenderMode mode /* = DebugRenderMode::DEBUG_RENDER_USE_DEPTH */, const Clock* clock /* = nullptr */ )
{
	g_debugRenderTasks.push_back( new DebugRenderPointTask( position, DebugRenderOptions( startColor, endColor, lifeTime, mode, DebugRenderCameraType::DEBUG_RENDER_CAMERA_3D, clock ) ) );
}

void DebugRenderLineSegment( float lifeTime, const Vector3& p0, const Rgba& p0Color, const Vector3& p1, const Rgba& p1Color, const Rgba& startColor, const Rgba& endColor, const DebugRenderMode mode /* = DebugRenderMode::DEBUG_RENDER_USE_DEPTH */ , const Clock* clock /* = nullptr */ )
{
	g_debugRenderTasks.push_back( new DebugRenderLineSegmentTask( p0, p1, p0Color, p1Color, DebugRenderOptions( startColor, endColor, lifeTime, mode, DebugRenderCameraType::DEBUG_RENDER_CAMERA_3D, clock ) ) );
}

void DebugRenderBasis( float lifeTime, const Matrix44& basis, const Rgba& startColor, const Rgba& endColor, const DebugRenderMode mode /* = DebugRenderMode::DEBUG_RENDER_USE_DEPTH */, const Clock* clock /* = nullptr */ )
{
	g_debugRenderTasks.push_back( new DebugRenderBasisTask( basis, DebugRenderOptions( startColor, endColor, lifeTime, mode, DebugRenderCameraType::DEBUG_RENDER_CAMERA_3D, clock ) ) );
}

void DebugRenderSphere( float lifeTime, const Vector3& position, float radius, unsigned int numSlices, unsigned int numWedges, const Rgba& startColor, const Rgba& endColor, const DebugRenderMode mode , const Clock* clock )
{
	g_debugRenderTasks.push_back( new DebugRenderSphereTask( position, radius, numWedges, numSlices, DebugRenderOptions( startColor, endColor, lifeTime, mode, DebugRenderCameraType::DEBUG_RENDER_CAMERA_3D, clock ) ) );
}

void DebugRenderWireSphere( float lifeTime, const Vector3& position, float radius, unsigned int numSlices, unsigned int numWedges, const Rgba& startColor, const Rgba& endColor, const DebugRenderMode mode , const Clock* clock )
{
	g_debugRenderTasks.push_back( new DebugRenderSphereTask( position, radius, numWedges, numSlices, DebugRenderOptions( startColor, endColor, lifeTime, mode, DebugRenderCameraType::DEBUG_RENDER_CAMERA_3D, clock, RendererPolygonMode::POLYGON_MODE_LINE ) ) );
}

void DebugRenderQuad( float lifeTime, const Vector3& position, const Vector3& right, float xMin, float xMax, const Vector3& up, float yMin, float yMax, Texture* texture, const AABB2& UVs, const Rgba& startColor, const Rgba& endColor, DebugRenderMode mode /* = DebugRenderMode::DEBUG_RENDER_USE_DEPTH */, const Clock* clock /* = nullptr */ )
{
	g_debugRenderTasks.push_back( new DebugRenderQuadTask( position, up, right, xMin, xMax, yMin, yMax, DebugRenderOptions( startColor, endColor, lifeTime, mode, DebugRenderCameraType::DEBUG_RENDER_CAMERA_3D, clock ), UVs, texture ) );
}

void DebugRenderGlyph( float lifeTime, const Vector3& position, float xMin, float xMax, float yMin, float yMax, Texture* texture, const AABB2& UVs, const Rgba& startColor, const Rgba& endColor, DebugRenderMode mode /* = DebugRenderMode::DEBUG_RENDER_USE_DEPTH */, const Clock* clock /* = nullptr */ )
{
	g_debugRenderTasks.push_back( new DebugRenderGlyphTask( position, xMin, xMax, yMin, yMax, DebugRenderOptions( startColor, endColor, lifeTime, mode, DebugRenderCameraType::DEBUG_RENDER_CAMERA_3D, clock ), UVs, texture ) );
}

void DebugRenderWireAABB3( float lifeTime, const AABB3& bounds, const Rgba& startColor, const Rgba& endColor, DebugRenderMode mode /* = DebugRenderMode::DEBUG_RENDER_USE_DEPTH */, const Clock* clock /* = nullptr */ )
{
	g_debugRenderTasks.push_back( new DebugRenderAABB3Task( bounds.GetCenter(), bounds.GetDimensions(), DebugRenderOptions( startColor, endColor, lifeTime, mode, DebugRenderCameraType::DEBUG_RENDER_CAMERA_3D, clock, RendererPolygonMode::POLYGON_MODE_LINE ) ) );
}

void DebugRenderWireOBB3( float lifeTime, const OBB3& bounds, const Rgba& startColor, const Rgba& endColor, DebugRenderMode mode /* = DebugRenderMode::DEBUG_RENDER_USE_DEPTH */, const Clock* clock /* = nullptr */ )
{
	g_debugRenderTasks.push_back( new DebugRenderOBB3Task( bounds.m_center, bounds.m_right, bounds.m_up, bounds.m_forward, DebugRenderOptions( startColor, endColor, lifeTime, mode, DebugRenderCameraType::DEBUG_RENDER_CAMERA_3D, clock, RendererPolygonMode::POLYGON_MODE_LINE ) ) );
}

void DebugRenderGrid( float lifeTime, const Vector3& position, const Vector3& right, float xMin, float xMax, const Vector3& up, float yMin, float yMax, const IntVector2& numUnits, const Rgba& startColor, const Rgba& endColor, DebugRenderMode mode /* = DebugRenderMode::DEBUG_RENDER_USE_DEPTH */, const Clock* clock /* = nullptr */ )
{
	g_debugRenderTasks.push_back( new DebugRenderGridTask( position, up, right, xMin, xMax, yMin, yMax, numUnits, DebugRenderOptions( startColor, endColor, lifeTime, mode, DebugRenderCameraType::DEBUG_RENDER_CAMERA_3D, clock, RendererPolygonMode::POLYGON_MODE_LINE ) ) );
}

void DebugRenderTag( float lifeTime, const Vector3& position, float cellHeight, float scale, const Vector2& alignment, const Rgba& startColor, const Rgba& endColor, DebugRenderMode mode, const Clock* clock, const char* fontName, const char* format, ... )
{
	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	std::string text = Stringv( format, variableArgumentList );
	va_end( variableArgumentList );

	g_debugRenderTasks.push_back( new DebugRenderTagTask( text, position, cellHeight, scale, alignment, fontName, DebugRenderOptions( startColor, endColor, lifeTime, mode, DebugRenderCameraType::DEBUG_RENDER_CAMERA_3D, clock ) ) );
}

void DebugRenderText( float lifeTime, const Vector3& position, const Vector3& rightVector, const Vector3& upVector, float cellHeight, float scale, const Vector2& alignment, const Rgba& startColor, const Rgba& endColor, DebugRenderMode mode, const Clock* clock, const char* fontName, const char* format, ... )
{
	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	std::string text = Stringv( format, variableArgumentList );
	va_end( variableArgumentList );

	g_debugRenderTasks.push_back( new DebugRenderTextTask( text, position, rightVector, upVector, cellHeight, scale, alignment, fontName, DebugRenderOptions( startColor, endColor, lifeTime, mode, DebugRenderCameraType::DEBUG_RENDER_CAMERA_3D, clock ) ) );
}

#pragma endregion

#pragma region MeshRenderModes

MeshRenderMode GetMeshRenderModeFromIndex( int renderModeAsInt )
{
	if ( renderModeAsInt < 0 || renderModeAsInt >= static_cast< int >( MeshRenderMode::NUM_RENDER_MODES ) )
	{
		ConsolePrintf( Rgba::RED, "ERROR: GetRenderModeFromIndex: Invalid index provided." );
		return MeshRenderMode::MODE_INVALID;
	}
	return static_cast< MeshRenderMode >( renderModeAsInt );
}

const char* GetMeshRenderModeName( MeshRenderMode mode )
{
	switch ( mode )
	{
		case MeshRenderMode::MODE_FINAL						:		return "DefaultLit";
		case MeshRenderMode::MODE_DIFFUSE					:		return "LitDiffuse";
		case MeshRenderMode::MODE_SPECULAR					:		return "LitSpecular";
		case MeshRenderMode::MODE_DIFFUSE_AND_SPECULAR		:		return "LitDiffuseSpecular";
		case MeshRenderMode::MODE_UV						:		return "UV";
		case MeshRenderMode::MODE_SURFACE_NORMAL			:		return "LitSurfaceNormalDebug";
		case MeshRenderMode::MODE_WORLD_NORMAL				:		return "LitWorldNormalDebug";
		case MeshRenderMode::MODE_VERTEX_NORMAL				:		return "LitNormalDebug";
		case MeshRenderMode::MODE_VERTEX_TANGENT			:		return "LitTangentDebug";
		case MeshRenderMode::MODE_VERTEX_BITANGENT			:		return "LitBitangentDebug";
		case MeshRenderMode::MODE_COLOR_ONLY				:		return "LitColor";
		default												:		return "Invalid";
	}
}

const char* GetMeshRenderModeName( int renderModeAsInt )
{
	return GetMeshRenderModeName( GetMeshRenderModeFromIndex( renderModeAsInt ) );
}

void SetShaderForRenderMode( MeshRenderMode renderMode )	// Names defined in Shaders.xml
{
	switch ( renderMode )
	{
		case MeshRenderMode::MODE_FINAL						:		g_rendererReference->SetShaderPass( "DefaultLit" ); break;
		case MeshRenderMode::MODE_DIFFUSE					:		g_rendererReference->SetShaderPass( "LitDiffuse" ); break;
		case MeshRenderMode::MODE_SPECULAR					:		g_rendererReference->SetShaderPass( "LitSpecular" );break;
		case MeshRenderMode::MODE_DIFFUSE_AND_SPECULAR		:		g_rendererReference->SetShaderPass( "LitDiffuseSpecular" );break;
		case MeshRenderMode::MODE_UV						:		g_rendererReference->SetShaderPass( "UV" ); break;
		case MeshRenderMode::MODE_SURFACE_NORMAL			:		g_rendererReference->SetShaderPass( "LitSurfaceNormalDebug" ); break;
		case MeshRenderMode::MODE_WORLD_NORMAL				:		g_rendererReference->SetShaderPass( "LitWorldNormalDebug" ); break;
		case MeshRenderMode::MODE_VERTEX_NORMAL				:		g_rendererReference->SetShaderPass( "LitNormalDebug" ); break;
		case MeshRenderMode::MODE_VERTEX_TANGENT			:		g_rendererReference->SetShaderPass( "LitTangentDebug" ); break;
		case MeshRenderMode::MODE_VERTEX_BITANGENT			:		g_rendererReference->SetShaderPass( "LitBitangentDebug" ); break;
		case MeshRenderMode::MODE_COLOR_ONLY				:		g_rendererReference->SetShaderPass( "LitColor" );break;
		default												:		break;
	}

	//ConsolePrintf( Rgba::CYAN, "Renderer: Switched to Render Mode %s", GetRenderModeName( renderMode ) ); NOTE: This spams
}

void SetShaderForRenderMode( int renderModeIndex )
{
	SetShaderForRenderMode( GetMeshRenderModeFromIndex( renderModeIndex ) );
}

Material* GetMaterialForRenderMode( MeshRenderMode renderMode )
{
	switch ( renderMode )
	{
		case MeshRenderMode::MODE_FINAL						:		return g_defaultLitMaterial;
		case MeshRenderMode::MODE_DIFFUSE					:		return g_diffuseMaterial;
		case MeshRenderMode::MODE_SPECULAR					:		return g_specularMaterial;
		case MeshRenderMode::MODE_DIFFUSE_AND_SPECULAR		:		return g_diffuseAndSpecularMaterial;
		case MeshRenderMode::MODE_UV						:		return g_UVMaterial;
		case MeshRenderMode::MODE_SURFACE_NORMAL			:		return g_surfaceNormalMaterial;
		case MeshRenderMode::MODE_WORLD_NORMAL				:		return g_worldNormalMaterial;
		case MeshRenderMode::MODE_VERTEX_NORMAL				:		return g_vertexNormalMaterial;
		case MeshRenderMode::MODE_VERTEX_TANGENT			:		return g_vertexTangentMaterial;
		case MeshRenderMode::MODE_VERTEX_BITANGENT			:		return g_vertexBitangentMaterial;
		case MeshRenderMode::MODE_COLOR_ONLY				:		return g_colorOnlyMaterial;
		default												:		return nullptr;
	}
}

Material* GetMaterialForRenderMode( int renderModeIndex )
{
	return GetMaterialForRenderMode( GetMeshRenderModeFromIndex( renderModeIndex ) );
}

#pragma endregion

#pragma region ConsoleCommands

bool ToggleDebugRenderCommand( Command& toggleDebugRenderCommand )
{
	if ( toggleDebugRenderCommand.GetName() == "debug_toggle" )
	{
		EnableDebugRenderer( !IsDebugRenderingEnabled() );
		return true;
	}
	return false;
}

bool ClearDebugRenderQueueCommand( Command& clearDebugQueueCommand )
{
	if ( clearDebugQueueCommand.GetName() == "clear_debug" )
	{
		ClearAllDebugRenderTasks();
		return true;
	}
	return false;
}

bool ClearDebugRenderLogCommand( Command& clearDebugLogCommand )
{
	if ( clearDebugLogCommand.GetName() == "clear_debug_log" )
	{
		ClearLogTasks();
		return true;
	}
	return false;
}

#pragma endregion
