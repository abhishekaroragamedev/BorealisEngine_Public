#pragma once

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Core/ThreadSafeTypes.hpp"
#include "Engine/Core/Vertex.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/IntVector3.hpp"
#include "Engine/Math/Matrix44.hpp"
#include "Engine/Math/OBB3.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/Fog.hpp"
#include "Engine/Renderer/Light.hpp"
#include "Engine/Renderer/Mesh.hpp"
#include "Engine/Renderer/MeshBuilder.hpp"
#include "Engine/Renderer/RendererTypes.hpp"
#include "Engine/Renderer/Sprite.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/TextureCube.hpp"
#include "Engine/Renderer/TimeUBO.hpp"

#pragma comment( lib, "opengl32" )	// Link in the OpenGL32.lib static library

// Forward declarations
typedef unsigned int GLenum;
struct DrawCall;
struct RenderState;
class Command;
class Material;
class Renderable;
class RenderBuffer;
class Sampler;
class Shader;
class ShaderPass;
class ShaderProgram;
class UniformBuffer;

constexpr int CIRCLE_NUM_SIDES = 36;

// BUILT-IN SHADER LIST (Excluding those present as raw strings in Shaders/ShadersAsRawStrings.hpp)
constexpr char SHADERS_RELATIVE_PATH[] = "Data/Shaders/";
constexpr char DEFAULT_SHADER_NAME[] = "Default";
constexpr char INVALID_SHADER_NAME[] = "Invalid";
constexpr char SHADERS_XML_FILEPATH[] = "Data/Shaders/Shaders.xml";
constexpr char MATERIALS_XML_FILEPATH[] = "Data/Shaders/Materials.xml";

// BUILT-IN TEXTURE LIST
constexpr char TEXTURES_RELATIVE_PATH[] = "Data/Images/";
constexpr char DEFAULT_TEXTURE_NAME[] = "Texture_Default.png";
constexpr char DEFAULT_BLACK_TEXTURE_NAME[] = "Texture_Default_Black.png";

// TEXTURE BIND POINTS
constexpr unsigned int TEXTURE_BIND_POINT_SHADOW_MAP = 5U;

// UNIFORM BUFFER BIND POINTS
constexpr unsigned int UNIFORM_BUFFER_TIME_BIND = 0U;
constexpr unsigned int UNIFORM_BUFFER_CAMERA_BIND = 1U;
constexpr unsigned int UNIFORM_BUFFER_ACTIVE_LIGHTS_BIND = 2U;
constexpr unsigned int UNIFORM_BUFFER_OBJECT_LIGHT_PROPERTIES_BIND = 3U;
constexpr unsigned int UNIFORM_BUFFER_MODEL_MATRIX_BIND = 4U;
constexpr unsigned int UNIFORM_BUFFER_FOG_SETTINGS_BIND = 5U;

struct ShaderFloatUniform
{
	
public:
	ShaderFloatUniform( const char* name, float value )
		:	m_name( std::string( name ) ),
			m_value( value )
	{

	}
	ShaderFloatUniform( const std::string& name, float value )
		:	m_name( name ),
			m_value( value )
	{

	}

public:
	std::string m_name = "";
	float m_value = 0.0f;
};

struct TextureAsyncLoadRequest
{
public:
	TextureAsyncLoadRequest() {}
	TextureAsyncLoadRequest( Image& image, Texture* texture, unsigned int mipLevels )
		:	m_image( image ),
			m_texture( texture ),
			m_mipLevels( mipLevels )
	{

	}

public:
	Image m_image;
	Texture* m_texture = nullptr;
	unsigned int m_mipLevels = 1U;
};

struct TextureCubeAsyncLoadRequest
{
public:
	TextureCubeAsyncLoadRequest() {}
	TextureCubeAsyncLoadRequest( Image& image, TextureCube* textureCube )
		:	m_image( image ),
			m_textureCube( textureCube )
	{

	}

public:
	Image m_image;
	TextureCube* m_textureCube = nullptr;
};

class Renderer
{

public:
	Renderer();
	~Renderer();

public:
	BitmapFont* CreateOrGetBitmapFont( const char* bitmapFontName );
	SpriteSheet* CreateOrGetTextureAtlasForBitmapFont( const std::string& spriteSheetFilePath );
	Texture* CreateOrGetTexture( const std::string& textureFilePath, unsigned int mipLevels = 1U, bool loadAsync = true );
	Sprite* CreateOrReplaceSprite( const std::string& spriteName, Texture* texture, AABB2 UVs, float PPU, Vector2 pivot = Vector2::ZERO );
	Sprite* CreateOrReplaceSprite( const std::string& spriteName, const std::string& textureFilePath, AABB2 UVs, float PPU, Vector2 pivot = Vector2::ZERO );
	Sprite* CreateOrReplaceSprite( const std::string& spriteName, const tinyxml2::XMLElement& spriteElement );
	Mesh* CreateOrGetMesh( const std::string& meshPath );
	Sprite* GetSprite( const std::string& spriteName );
	Material* CreateOrGetMaterial( const std::string& materialName );
	Shader* CreateOrGetShader( const std::string& shaderName );
	ShaderProgram* CreateOrGetShaderProgram( const std::string& shaderName, const std::string& semiColonDelimitedDefines = "" );
	ShaderProgram* CreateOrGetShaderProgram( const char* shaderName, const char* semiColonDelimitedDefines = "" );
	ShaderProgram* CreateOrGetShaderProgramFromRawStrings( const std::string& shaderName, const std::string& vertexShaderText, const std::string& fragmentShaderText, const std::string& semiColonDelimitedDefines = "" );
	ShaderProgram* CreateOrGetShaderProgramFromRawStrings( const char* shaderName, const char* vertexShaderText, const char* fragmentShaderText, const char* semiColonDelimitedDefines = "" );
	Texture* CreateRenderTarget( unsigned int width, unsigned int height, TextureFormat format = TextureFormat::TEXTURE_FORMAT_RGBA8 );
	Texture* CreateDepthStencilTarget( unsigned int width, unsigned int height );
	Texture* CreateMSAATarget( unsigned int width, unsigned int height, TextureFormat format, unsigned int numSamples );
	Texture* GetDefaultTexture() const;
	TextureCube* GetDefaultTextureCube() const;
	Sampler* GetDefaultSampler( bool usesMipMaps = false ) const;
	Texture* GetDefaultColorTarget() const;
	Texture* GetDefaultDepthStencilTarget() const;
	Camera* GetCurrentCamera() const;
	Vector3 GetCurrentViewportUpVector() const;
	Vector3 GetOrientedQuadPositionWithPivotCorrection( const Vector3& position, const Vector3& rightVector, const Vector2& pivot, const Vector2& dimensions ) const;
	void ReloadAllFileShaders();
	void SetCamera( Camera* camera = nullptr );
	void UseDefaultCamera();
	int GetGLDrawPrimitive( const DrawPrimitiveType primitiveType ) const;
	int GetGLDataType( const RendererDataType dataType ) const;
	int GetGLCullMode( const RendererCullMode cullMode ) const;
	int GetGLPolygonMode( const RendererPolygonMode polygonMode ) const;
	int GetGLWindOrder( const RendererWindOrder windOrder ) const;
	int GetGLBlendOperation( const RendererBlendOperation blendOperation ) const;
	int GetGLBlendFactor( const RendererBlendFactor blendFactor ) const;
	int GetGLCubeSide( const TextureCubeSide cubeSide ) const;
	int GetGLTextureType( const TextureType textureType ) const;
	GLenum GetGLCompare( DepthTestCompare compare ) const;
	void BindTextureAndSampler( unsigned int slotIndex, const Texture& texture, const Sampler& sampler ) const;
	void BindFont( unsigned int slotIndex, const BitmapFont* bitmapFont ) const;	// Binds the bitmapFont's texture to the specified slot, along with the default sampler
	void EnableTexturing( const Texture& texture, const Sampler& sampler ) const;	// Binds texture and sampler to slot 0
	void DisableTexturing() const;
	void ClearColor( const Rgba& color = Rgba::BLACK ) const;
	void ClearDepth( float depth = 1.0f ) const;
	void ApplyEffect( ShaderProgram* effectShader, const std::map< const char*, float >& shaderParameters = std::map< const char*, float >() );
	void FinishEffects();
	void ApplyEffect( Material* effectMaterial, Camera* useCamera = nullptr, Texture* useScratchTarget = nullptr );	// Add the source target to the Camera
	void FinishEffects( Texture* originalSource );	// Swaps between the scratch target and the first texture on the material
	void Draw( const DrawCall& drawCall );
	void DrawMesh( const Mesh* mesh ) const;
	void DrawMeshWithMaterial( const Mesh* mesh, Material& material );
	void DrawRenderable( Renderable& renderable, unsigned int passIndex = 0U );
	void DrawMeshImmediate( const Vertex_3DPCU* verts, int numVerts, const unsigned int* indices, int numIndices, DrawPrimitiveType drawPrimitive ) const;
	void DrawFullScreenImmediate( Material& material, const Rgba& color = Rgba::WHITE, unsigned int passIndex = 0U );
	void DrawLine( const Vector2& startPoint, const Vector2& endPoint, const Rgba& startColor, const Rgba& endColor, const float lineThickness ) const;
	void DrawLineBorder( const AABB2& bounds, const Rgba& color, const float lineThickness ) const;
	void DrawPolygon( const Vector2& location, const float angle, const Vector2 vertices[], const int numSides, const RPolygonType polygonType, const Rgba& color ) const;
	void DrawAABB( const AABB2& bounds, const Rgba& color ) const;
	void DrawAABBBorder( const AABB2& innerBoundsAABB, const Rgba& colorAtInnerBorder, const AABB2& outerBoundsAABB, const Rgba& colorAtOuterBorder ) const;
	void DrawTexturedAABB( const AABB2& bounds, const Texture& texture, const Vector2& textureCoordinatesAtMins, const Vector2& textureCoordinatesAtMaxs, const Rgba& tint ) const;
	void DrawText2D( const Vector2& drawMins, const std::string& asciiText, float cellHeight, const Rgba& tint, float aspectScale, const BitmapFont* font ) const;
	void DrawTextInBox2D( const std::string& asciiText, const AABB2& boxBounds, float cellHeight, const Rgba& tint, float aspectScale, const BitmapFont* font, const TextDrawMode textDrawMode, const Vector2& alignment ) const;
	void DrawTextInBox3D( const std::string& asciiText, const Vector3& position, const Vector3& rightVector, const Vector2& boxDimensions, float cellHeight, const Rgba& boxColor, const Rgba& textTint, float aspectScale, const BitmapFont* font, const Vector2& boxPivot, const Vector2& alignment, bool usesCameraForward = true, const Vector3& upVector = Vector3::UP ) const;	// Only SHRINK_TO_FIT mode supported
	void DrawSprite( Sprite* sprite, const Vector3& position, const Vector3& rightVector, float spriteScale, const Rgba& tint, Vector2 uvFlip = Vector2::ONE );
	void DrawSprite( const std::string& spriteName, const Vector3& position, const Vector3& rightVector, float spriteScale, const Rgba& tint, Vector2 uvFlip = Vector2::ONE );
	void DrawTextureCube( const TextureCube& textureCube, const Vector3& dimensions );	// Debug function; assumes everything else is set
	MeshBuilder MakeTexturedAABBMesh( const AABB2& bounds, const Texture& texture, const Vector2& textureCoordinatesAtMins, const Vector2& textureCoordinatesAtMaxs, const Rgba& tint ) const;
	MeshBuilder MakeTextMesh2D( const Vector2& drawMins, const std::string& asciiText, float cellHeight, float characterWidth, const Rgba& tint, const BitmapFont* font ) const;
	MeshBuilder MakeOrientedQuadMesh( const Vector3& position, const Vector3& rightVector, const Vector2& pivot, const Vector2& dimensions, const Rgba& color, bool usesCameraForward = true, const Vector3& upVector = Vector3::UP ) const;
	MeshBuilder MakeTexturedOrientedQuadMesh( const Vector3& position, const Vector3& rightVector, const Vector2& pivot, const Vector2& dimensions, const Texture& texture, const Vector2& textureCoordinatesAtMins, const Vector2& textureCoordinatesAtMaxs, const Rgba& tint, bool usesCameraForward = true, const Vector3& upVector = Vector3::UP ) const;
	MeshBuilder MakeTextMesh3D( const std::string& asciiText, const Vector3& position, const Vector3& rightVector, float cellHeight, float characterWidth, const Rgba& tint, const BitmapFont* font, bool usesCameraForward = true, const Vector3& upVector = Vector3::UP ) const;
	MeshBuilder MakeCubeMesh( const Vector3& center, const Vector3& dimensions, const Rgba& color ) const;
	MeshBuilder MakeCubeMesh( const AABB3& bounds, const Rgba& color ) const;
	MeshBuilder MakeTexturedCubeMesh( const Vector3& center, const Vector3& dimensions, const Texture& texture, const Rgba& tint, const AABB2& uvTop = AABB2::ZERO_TO_ONE, const AABB2& uvSide = AABB2::ZERO_TO_ONE, const AABB2& uvBottom = AABB2::ZERO_TO_ONE  ) const;
	MeshBuilder MakeTexturedCubeMesh( const AABB3& bounds, const Texture& texture, const Rgba& tint, const AABB2& uvTop = AABB2::ZERO_TO_ONE, const AABB2& uvSide = AABB2::ZERO_TO_ONE, const AABB2& uvBottom = AABB2::ZERO_TO_ONE  ) const;
	MeshBuilder MakeOBBMesh( const OBB3& bounds, const Rgba& color ) const;
	MeshBuilder MakeOBBMesh( const Vector3& center, const Vector3& right, const Vector3& up, const Vector3& forward, const Rgba& color ) const;
	MeshBuilder MakeGridMesh( const Vector3& center, const AABB2& bounds, const Vector3& up, const Vector3& right, const IntVector2& numUnits, const Rgba& color ) const;
	MeshBuilder MakePlaneMesh( const Vector3& bottomLeft, const Vector3& upVector, const Vector3& rightVector, const Vector2& dimensions, const Rgba& tint, const AABB2& uv ) const;
	MeshBuilder MakeUVSphereMesh( const Vector3& center, float radius, unsigned int numWedges, unsigned int numSlices, const Rgba& tint ) const;
	MeshBuilder MakeBasisMesh( const Vector3& iBasis, const Vector3& jBasis, const Vector3& kBasis, const Vector3& magnitudes ) const;
	void ChangeRenderColor( const Rgba& ) const;
	void BindRenderState( const RenderState& renderState ) const;
	void SetShaderPass( ShaderPass* shader );
	void SetShaderPass( const std::string& loadedShaderName, unsigned int passIndex = 0U );
	void UseActiveShaderProgram() const;
	void UseShaderProgram( ShaderProgram* shaderProgram );
	void UseShaderProgram( const std::string& shaderName );
	void BindMeshToShaderProgram( const Mesh* mesh, ShaderProgram* shaderProgram ) const;
	void BindAttribToCurrentShader( const char* shaderAttribName, float attribValue );
	void BindAttribToCurrentShader( const ShaderFloatUniform& shaderAttrib );
	void BindVector3ToCurrentShader( const char* shaderUniformName, const Vector3& vec3 );
	void BindVector4ToCurrentShader( const char* shaderUniformName, const Vector4& vec4 );
	void BindRgbaToCurrentShader( const char* shaderUniformName, const Rgba& color );
	void BindMatrixToCurrentShader( const char* shaderUniformName, const Matrix44* matrix );
	void BindModelMatrixToCurrentShader( const Matrix44& matrix );
	void BindMaterial( Material& material, unsigned int passIndex = 0U );
	void BindInputLayout( Renderable* renderable );
	void PushMatrix() const;
	void PopMatrix() const;
	void Translate( float xTranslation, float yTranslation, float zTranslation ) const;
	void Rotate( float angleDegrees, float xAxis, float yAxis, float zAxis ) const;
	void Scale( float xScale, float yScale, float zScale ) const;
	void SetProjectionMatrix( const Matrix44& projectionMatrix ) const;
	void SetOrtho3D( const Vector3& mins, const Vector3& maxs ) const;
	void SetOrtho( const Vector2& bottomLeft, const Vector2& topRight ) const;
	void WriteDepthImmediate( bool shouldWrite );	// Sets GL's depth write flag immediately instead of setting it on the default shader
	void EnableDepth( DepthTestCompare compare, bool shouldWrite );
	void DisableDepth();
	void EnableSRGB();
	void DisableSRGB();
	void SetPolygonMode( RendererPolygonMode newPolygonMode );
	void SetBlendMode( RendererBlendMode newBlendMode );
	void ClearScreen( const Rgba& clearColor ) const;
	void TakeScreenshotAtEndOfFrame();
	bool CopyFrameBuffer( FrameBuffer* destination, FrameBuffer* source ) const;

	void AddToTextureLoadQueue( TextureAsyncLoadRequest* loadInfo );
	void AddToTextureCubeLoadQueue( TextureCubeAsyncLoadRequest* loadInfo );
	void ProcessTextureLoadQueue();

#pragma region Pipeline

	void InitializeRenderer();
	void PostStartup();
	void BeginFrame( float screenWidth, float screenHeight );
	void EndFrame() const;

#pragma endregion

#pragma region Effects

	void UpdateTimeUBOWithFrameTime();
	void SetTimeUBO( const TimeUBO& time );
	void SetTimeUBO( float currentTime, float frameTime, unsigned int frameCount );
	void SetFog( const FogUBO& fog );
	void SetFog( const Vector4& fogColor, float fogNearFactor, float fogFarFactor, float fogNearZ, float fogFarZ );

#pragma endregion

#pragma region Lighting

	void SetAmbientLight( const Vector4& lightColorAndIntensity ); // Alpha of color is light intensity
	void SetAmbientLight( const Rgba& light, float intensity = 1.0f );
	void BindLights( const LightUBO lights[ MAX_LIGHTS ] );
	void SetSpecularProperties( float specularAmount, float specularPower );

#pragma endregion

#pragma region Template Draws

	template < typename VERTTYPE = Vertex_3DLit >
	void DrawOrientedQuad( const Vector3& position, const Vector3& rightVector, const Vector2& pivot, const Vector2& dimensions, const Rgba& color, bool usesCameraForward = true, const Vector3& upVector = Vector3::UP ) const
	{
		MeshBuilder quadMeshBuilder = MakeOrientedQuadMesh( position, rightVector, pivot, dimensions, color, usesCameraForward, upVector );
		Mesh* quadMesh = new Mesh;
		quadMesh->FromBuilder< VERTTYPE >( quadMeshBuilder );
		DrawMesh( quadMesh );
		delete quadMesh;
	}
	template < typename VERTTYPE = Vertex_3DLit >
	void DrawText3D( const std::string& asciiText, const Vector3& position, const Vector3& rightVector, float cellHeight, const Rgba& tint, float aspectScale, const BitmapFont* font, bool usesCameraForward = true, const Vector3& upVector = Vector3::UP ) const
	{
		if ( font == nullptr )
		{
			font = m_defaultBitmapFont;
		}

		float characterWidth = font->GetCharacterWidth( cellHeight, aspectScale );

		BindTextureAndSampler( 0, *font->m_spriteSheet.GetTexture(), *m_defaultSampler );

		MeshBuilder textMeshBuilder = MakeTextMesh3D( asciiText, position, rightVector, cellHeight, characterWidth, tint, font, usesCameraForward, upVector );
		Mesh* textMesh = new Mesh;
		textMesh->FromBuilder< VERTTYPE >( textMeshBuilder );
		DrawMesh( textMesh );
		delete textMesh;
	}
	template < typename VERTTYPE = Vertex_3DLit >
	void DrawCube( const Vector3& center, const Vector3& dimensions, const Rgba& color ) const
	{
		MeshBuilder cubeMeshBuilder = MakeCubeMesh( center, dimensions, color );
		BindTextureAndSampler( 0, *m_defaultTexture, *m_defaultSampler );
		Mesh* cubeMesh = new Mesh;
		cubeMesh->FromBuilder< VERTTYPE >( cubeMeshBuilder );
		DrawMesh( cubeMesh );
		delete cubeMesh;
	}
	template < typename VERTTYPE = Vertex_3DLit >
	void DrawTexturedCube( const Vector3& center, const Vector3& dimensions, const Texture& texture, const Rgba& tint, const AABB2& uvTop = AABB2::ZERO_TO_ONE, const AABB2& uvSide = AABB2::ZERO_TO_ONE, const AABB2& uvBottom = AABB2::ZERO_TO_ONE ) const
	{
		MeshBuilder cubeMeshBuilder = MakeTexturedCubeMesh( center, dimensions, texture, tint, uvTop, uvSide, uvBottom );
		BindTextureAndSampler( 0, texture, *m_defaultSampler );
		Mesh* cubeMesh = new Mesh;
		cubeMesh->FromBuilder< VERTTYPE >( cubeMeshBuilder );
		DrawMesh( cubeMesh );
		delete cubeMesh;
	}
	template < typename VERTTYPE = Vertex_3DLit >
	void DrawPlane( const Vector3& bottomLeft, const Vector3& upVector, const Vector3& rightVector, const Vector2& dimensions, const Texture& texture, const Rgba& tint, const AABB2& uv ) const
	{
		MeshBuilder planeBuilder = MakePlaneMesh( bottomLeft, upVector, rightVector, dimensions, tint, uv );
		BindTextureAndSampler( 0, texture, *m_defaultSampler );
		Mesh* planeMesh = new Mesh;
		planeMesh->FromBuilder< VERTTYPE >( planeBuilder );
		DrawMesh( planeMesh );
		delete planeMesh;
	}
	template < typename VERTTYPE = Vertex_3DLit >
	void DrawUVSphere( const Vector3& center, float radius, unsigned int numWedges, unsigned int numSlices, const Texture& texture, const Rgba& tint ) const
	{
		MeshBuilder uvSphereBuilder = MakeUVSphereMesh( center, radius, numWedges, numSlices, tint );
		BindTextureAndSampler( 0, texture, *m_defaultSampler );
		Mesh* uvSphereMesh = new Mesh;
		uvSphereMesh->FromBuilder< VERTTYPE >( uvSphereBuilder );
		DrawMesh( uvSphereMesh );
		delete uvSphereMesh;
	}

#pragma endregion

public:
	static Renderer* GetInstance();

private:
	bool RenderStartup();
	Vector2 GetAnchorPointForAlignmentWithTextBox2D( const AABB2& boxBounds, const Vector2& alignment ) const;
	Vector3 GetAnchorPointForAlignmentWithTextBox3D( const Vector3& boxMins, const Vector3& rightVector, const Vector2& boxDimensions, const Vector2& alignment ) const;
	void LoadShadersFromFile();
	void LoadMaterialsFromFile();
	void ReloadShadersFromFile();
	void ReloadMaterialsFromFile();
	void DrawTextInBox2DOverrun( const std::vector< std::string >& linesOfText, const AABB2& boxBounds, float cellHeight, const Rgba& tint, float aspectScale, const BitmapFont* font, const Vector2& alignment ) const;
	void DrawTextInBox2DShrinkToFit( const std::vector< std::string >& linesOfText, const AABB2& boxBounds, float cellHeight, const Rgba& tint, float aspectScale, const BitmapFont* font, const Vector2& alignment ) const;
	void DrawTextInBox2DWordWrap( std::vector< std::string >& linesOfText, const AABB2& boxBounds, float cellHeight, const Rgba& tint, float aspectScale, const BitmapFont* font, const Vector2& alignment ) const;
	void DrawTextInBox3DShrinkToFit( const std::vector< std::string >& linesOfText, const Vector3& position, const Vector3& rightVector, const Vector2& boxDimensions, float cellHeight, const Rgba& boxColor, const Rgba& tint, float aspectScale, const BitmapFont* font, const Vector2& alignment, bool usesCameraForward = true, const Vector3& upVector = Vector3::UP ) const;
	void SaveScreenshot() const;

private:
	static Renderer* s_renderer;

	const float CIRCLE_THETA_INCREMENT_DEGREES = static_cast<float>( 360 / CIRCLE_NUM_SIDES );

	std::map< std::string, Sprite* > m_loadedSpritesByName;
	std::map< std::string, Mesh* > m_loadedMeshesByName;
	std::map< std::string, Texture* > m_loadedTexturesByName;
	std::map< std::string, SpriteSheet* > m_loadedFontSpriteSheetsByName;
	std::map< std::string, BitmapFont* > m_loadedFontsByName;
	std::map< std::string, Shader* > m_loadedShadersFromFileByName;
	std::map< std::string, Material* > m_loadedMaterialsFromFileByName;
	std::map< std::string, ShaderProgram* > m_loadedShaderProgramsByName;
	
	std::vector< std::string > m_namesOfShadersLoadedFromStrings;
	std::map< std::string, std::string > m_definesInjectedIntoShadersByName;

	unsigned int m_defaultVAO = 0;
	BitmapFont* m_defaultBitmapFont = nullptr;		// The first loaded font will be the default font
	Camera* m_defaultCamera = nullptr;
	Sampler* m_defaultSampler = nullptr;
	Sampler* m_defaultSamplerMipMap = nullptr;
	Texture* m_defaultTexture = nullptr;
	TextureCube* m_defaultTextureCube = nullptr;
	Texture* m_defaultColorTarget = nullptr;
	Texture* m_defaultDepthStencilTarget = nullptr;
	Texture* m_effectsSwapTarget = nullptr;

	Mesh* m_immediateMesh = nullptr;

	Camera* m_currentCamera = nullptr;
	ShaderPass* m_activeShaderPass = nullptr;
	ShaderPass* m_defaultShaderPass = nullptr;
	bool m_takeScreenshotThisFrame = false;

	// UBOs
	UniformBuffer* m_frameTimeUBO = nullptr;
	UniformBuffer* m_currentCameraUBO = nullptr;
	UniformBuffer* m_activeLightsUBO = nullptr;
	UniformBuffer* m_objectLightPropertiesUBO = nullptr;
	UniformBuffer* m_modelMatrixUBO = nullptr;
	UniformBuffer* m_fogUBO = nullptr;

	// Light structs
	ActiveLightBufferUBO m_activeLightsStruct;
	ObjectLightPropertiesUBO m_objectLightPropertiesStruct;

	// Effects framework members
	Camera* m_effectsCamera = nullptr;
	Texture* m_effectsSourceTarget = nullptr;
	Texture* m_effectsScratchTarget = nullptr;

	// Async loading
	ThreadSafeQueue< TextureAsyncLoadRequest* > m_textureLoadQueue;
	ThreadSafeQueue< TextureCubeAsyncLoadRequest* > m_textureCubeLoadQueue;

};
