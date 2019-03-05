#include "Game/TheGame.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Utils/VanDerCorputSequence.hpp"
#include "Game/Utils/WorleyNoise.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Core/Window.hpp"
#include "Engine/FileUtils/File.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Procedural/SmoothNoise.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Renderer/ForwardRenderingPath.hpp"
#include "Engine/Renderer/GLUtils.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/Mesh.hpp"
#include "Engine/Renderer/MeshBuilder.hpp"
#include "Engine/Renderer/ParticleEmitter.hpp"
#include "Engine/Renderer/Renderable.hpp"
#include "Engine/Renderer/RendererTypes.hpp"
#include "Engine/Renderer/Sampler.hpp"
#include "Engine/Renderer/Scene.hpp"
#include "Engine/Renderer/Texture3D.hpp"
#include "Engine/Tools/Command.hpp"
#include "Engine/Tools/DevConsole.hpp"
#include "Engine/Tools/Profiler/ProfileScope.hpp"
#include "ThirdParty/tinyXML2/tinyxml2.h"

#define NO_DEBUG
//#define LOG_DEBUG

constexpr float CAMERA_NEAR_Z = 0.1f;
constexpr float CAMERA_FAR_Z = 5000.0f;

TheGame::TheGame()
{
	m_defaultFont = g_renderer->CreateOrGetBitmapFont( DEFAULT_FONT_NAME );

	InitializeCamerasAndScratchTarget();
	InitializeDeferredTargets();
	InitializeTerrain();
	InitializeShip();
	InitializeLights();
	InitializeShaderResources();
	InitializeNoiseData();
	InitializeMieData();
	InitializeCloudRenderTargets();
	InitializeVanDerCorputSequence();

	CommandRegister( "set_ambient_light", SetAmbientLightCommand, "Sets the ambient light color and intensity associated with a specified shader, or with the active shader." );
	CommandRegister( "goto", CameraGoToCommand, "Moves the camera to the specified position." );
	CommandRegister( "goin", CameraGoInDirectionCommand, "Moves the camera by the specified displacement." );
	CommandRegister( "lookat", CameraLookAtCommand, "Makes the camera look in the specified direction. Global up is assumed." );
}

TheGame::~TheGame()
{
	DeleteRenderables();

	delete[] m_vanDerCorputSequence;
	m_vanDerCorputSequence = nullptr;

	delete m_debugOverlayCamera;
	m_debugOverlayCamera = nullptr;

	delete m_clampSampler;
	m_clampSampler = nullptr;

	delete m_volumetricShadowTarget;
	m_volumetricShadowTarget = nullptr;

	delete m_sceneNormalTarget;
	m_sceneNormalTarget = nullptr;

	delete m_sceneSpecularTarget;
	m_sceneSpecularTarget = nullptr;

	delete m_sceneColorTarget;
	m_sceneColorTarget = nullptr;

	delete m_gameCamera;
	m_gameCamera = nullptr;

	delete m_mieLUT;

	delete m_sceneDepthTargets[0];
	delete m_sceneDepthTargets[1];
	delete m_reprojectionTargets[ 0 ];
	delete m_reprojectionTargets[ 1 ];

	delete m_shapeTexture;
	delete m_detailTexture;

	for ( unsigned int index = 0U; index < 8U; index++ )
	{
		delete m_sceneLights[ index ];
	}

	CommandUnregister( "set_ambient_light" );
	CommandUnregister( "goto" );
	CommandUnregister( "goin" );
	CommandUnregister( "lookat" );
}

void TheGame::DeleteRenderables()
{
	delete m_mieGraphGrid;
	delete m_mieGraphR;
	delete m_mieGraphG;
	delete m_mieGraphB;
	delete m_ship;
	for ( Renderable* terrainChunk : m_terrainChunks )
	{
		delete terrainChunk;
	}
}

#pragma region Initialization

void TheGame::InitializeCamerasAndScratchTarget()
{
	unsigned int rtHeight = static_cast<unsigned int>( RENDER_TARGET_HEIGHT );
	unsigned int rtWidth = static_cast<unsigned int>( RENDER_TARGET_HEIGHT * CLIENT_ASPECT );
	m_sceneColorTarget = g_renderer->CreateRenderTarget( rtWidth, rtHeight, TEXTURE_FORMAT_RGBA8 );
	m_sceneDepthTargets[0] = g_renderer->CreateDepthStencilTarget( rtWidth, rtHeight );
	m_sceneDepthTargets[1] = g_renderer->CreateDepthStencilTarget( rtWidth, rtHeight );

	m_gameCamera = new Camera( m_sceneColorTarget, m_sceneDepthTargets[m_currentReprojectionTargetIndex], true );
	m_gameCamera->SetProjection( Matrix44::MakePerspective( 35.0f, Window::GetAspect(), CAMERA_NEAR_Z, CAMERA_FAR_Z ) );
	m_gameCamera->SetMaxShadowDepth( 10.0f );
	m_oldViewMatrix = m_gameCamera->GetViewMatrix();

	DebugRenderSet3DCamera( m_gameCamera );
	m_origin = Vector3( 0.0f, ( g_gameConfigBlackboard.GetValue( "terrainHeight", 500.0f ) + 1.0f ), 0.0f );
	m_gameCamera->Translate(
		m_origin + m_snapBottomPosition
	);

	m_debugOverlayCamera = new Camera( g_renderer->GetDefaultColorTarget() );	// Debug UI renders to default target after the copy - needs to be fullscreen

	m_lowResScratchTarget = g_renderer->CreateRenderTarget( rtWidth, rtHeight, TEXTURE_FORMAT_RGBA8 );
}

void TheGame::InitializeDeferredTargets()
{
	unsigned int targetWidth = static_cast< unsigned int >( m_sceneColorTarget->GetDimensions().x );
	unsigned int targetHeight = static_cast< unsigned int >( m_sceneColorTarget->GetDimensions().y );
	m_sceneNormalTarget = g_renderer->CreateRenderTarget( targetWidth, targetHeight );
	m_sceneSpecularTarget = g_renderer->CreateRenderTarget( targetWidth, targetHeight );
}

void TheGame::InitializeLights()
{
	m_sun = new Light();

	Vector3 sunPosCartesian = Vector3::ConvertToCartesian( m_sunPositionPolar );
	m_sun->SetDirectionalLight(
		sunPosCartesian,
		Vector3( -sunPosCartesian.x, -sunPosCartesian.y, -sunPosCartesian.z ).GetNormalized(),
		Vector4( 1.0f, 1.0f, 1.0f, m_sunlightIntensity ),
		Vector3::ZERO
	);
	m_sun->m_lightUBO.m_castsShadows = 0;

	m_sceneLights[ 0 ] = &m_sun->m_lightUBO;
	m_sceneLights[ 1 ] = new LightUBO();
	m_sceneLights[ 2 ] = new LightUBO();
	m_sceneLights[ 3 ] = new LightUBO();
	m_sceneLights[ 4 ] = new LightUBO();
	m_sceneLights[ 5 ] = new LightUBO();
	m_sceneLights[ 6 ] = new LightUBO();
	m_sceneLights[ 7 ] = new LightUBO();

	g_renderer->SetShaderPass( "DefaultLit" );
	g_renderer->SetAmbientLight( Rgba::WHITE, 0.0f );
}

void TheGame::InitializeShaderResources()
{
	TODO( "Write mip-level calculator from texture dimensions" );
	m_weatherTexture = g_renderer->CreateOrGetTexture( std::string( WEATHER_TEXTURE_FILEPATH ), 11U, TEXTURE_FORMAT_RGBA8 );
	m_heightSignal = g_renderer->CreateOrGetTexture( std::string( HEIGHT_SIGNAL_FILEPATH ), 8U, TEXTURE_FORMAT_RGBA8 );

	SamplerOptions samplerOptions = SamplerOptions{ SamplerWrapMode::SAMPLER_WRAP_CLAMP_EDGE, SamplerSampleMode::SAMPLER_LINEAR };
	m_clampSampler = new Sampler( samplerOptions );
}

void TheGame::InitializeTerrain()
{
	// Get maxRadius such that RaySphereIntersect of any point on the terrain will have a max root with a value of at least 0 (if not positive)
	float maxPointLength = m_cloudLayerAltitude;
	float terrainHeight = g_gameConfigBlackboard.GetValue( "terrainHeight", 500.0f );
	float maxRadius = sqrtf( (maxPointLength * maxPointLength) - (terrainHeight * terrainHeight) );
	UNUSED( maxRadius );

	int numChunks = g_gameConfigBlackboard.GetValue( "terrainNumChunks1D", 1 );
	FloatRange terrainRange( -2000.0f, 2000.0f );
	IntVector2 terrainNumSamples( 250, 250 );
	terrainNumSamples.x /= numChunks;
	terrainNumSamples.y /= numChunks;
	Vector2 terrainRangeIncrements = Vector2(
		terrainRange.Size() / static_cast<float>( numChunks ),
		terrainRange.Size() / static_cast<float>( numChunks )
	);

	Material* terrainMaterial = Renderer::GetInstance()->CreateOrGetMaterial( "Terrain" );
	MeshBuilder mb;
	for ( int i = 0; i < numChunks; i++ )
	{
		for ( int j = 0; j < numChunks; j++ )
		{
			mb.Begin( DrawPrimitiveType::TRIANGLES );

			FloatRange currentRangeX(
				terrainRange.min + ( static_cast<float>( i ) * terrainRangeIncrements.x ),
				terrainRange.min + ( static_cast<float>( i + 1 ) * terrainRangeIncrements.x )
			);
			FloatRange currentRangeZ(
				terrainRange.min + ( static_cast<float>( j ) * terrainRangeIncrements.y ),
				terrainRange.min + ( static_cast<float>( j + 1 ) * terrainRangeIncrements.y)
			);
			mb.AddSurfacePatch( TerrainSurfacePatch, currentRangeX, currentRangeZ, terrainNumSamples );
			// TODO: Rewrite radial patch
			//mb.AddSurfacePatch( TerrainSurfacePatchRadial, FloatRange( 0.0f, maxRadius ), FloatRange( 0.0f, 360.0f ), IntVector2( 300, 300 ) );

			mb.End();

			Renderable* newChunk = new Renderable();
			newChunk->SetModelMatrix( Matrix44::MakeTranslation( Vector3( 0.0f, terrainHeight, 0.0f ) ) );
			newChunk->SetMesh( mb.CreateMesh() );
			newChunk->ReplaceMaterial( terrainMaterial, false );
			m_terrainChunks.push_back( newChunk );

			mb.Clear();
		}
	}
}

void TheGame::InitializeShip()
{
	Mesh* shipMesh = MeshBuilder::FromFileOBJ( SHIP_OBJ_FILEPATH )[0];
	Material* shipMaterial = g_renderer->CreateOrGetMaterial( "Ship" );
	
	m_ship = new Renderable(
		shipMesh,
		shipMaterial,
		true,
		false
	);
	m_shipTransform.SetLocalPosition( Vector3( 0.0f, 5100.0f, 0.0f ) );
	m_shipTransform.MarkDirty();
}

void TheGame::InitializeNoiseData()
{
	// Shape texture
	IntVector3 shapeDimensions = g_gameConfigBlackboard.GetValue( "shapeNoiseDimensions", IntVector3() );
	Image* shapeImages = new Image[ shapeDimensions.z ];

	Rgba colorStamp;
	for ( int k = 0; k < shapeDimensions.z; k++ )
	{
		Image& currentImage = shapeImages[ k ];
		currentImage.SetDimensions( shapeDimensions.x, shapeDimensions.y );

		for ( int i = 0; i < shapeDimensions.x; i++ )
		{
			for ( int j = 0; j < shapeDimensions.y; j++ )
			{
				colorStamp.SetAsFloats( 0.0f, 0.0f, 0.0f, 0.0f );
				currentImage.SetTexel( i, j, colorStamp );
			}
		}
	}
	m_shapeTexture = new Texture3D();
	unsigned int imageCount = static_cast< unsigned int >( shapeDimensions.z );
	m_shapeTexture->CreateFromImages(
		imageCount,
		shapeImages,
		8U,	// 128 -> 8 mip layers
		TEXTURE_FORMAT_RGBA8
	);

	delete[] shapeImages;

	// Detail texture
	IntVector3 detailDimensions = g_gameConfigBlackboard.GetValue( "detailNoiseDimensions", IntVector3() );

	Image* detailImages = new Image[ detailDimensions.z ];

	for ( int k = 0; k < detailDimensions.z; k++ )
	{
		Image& currentImage = detailImages[ k ];
		currentImage.SetDimensions( detailDimensions.x, detailDimensions.y );

		for ( int i = 0; i < detailDimensions.x; i++ )
		{
			for ( int j = 0; j < detailDimensions.y; j++ )
			{
				colorStamp.SetAsFloats( 0.0f, 0.0f, 0.0f, 0.0f );
				currentImage.SetTexel( i, j, colorStamp );
			}
		}
	}
	m_detailTexture = new Texture3D();
	imageCount = static_cast< unsigned int >( detailDimensions.z );
	m_detailTexture->CreateFromImages(
		imageCount,
		detailImages,
		6U, 	// 32 -> 6 mip layers
		TEXTURE_FORMAT_RGBA8
	);

	delete[] detailImages;

	UpdateNoiseTextures();
}

void TheGame::InitializeMieData()
{
	Image mieImage = Image();
	mieImage.SetDimensions( 1801, 1 );
	Vector4* texelsRawValue = new Vector4[ 1801 ];

	char* mieChoppedData = reinterpret_cast< char* >( FileReadToNewBuffer( MIE_CHOPPED_TXT_FILEPATH ) );
	char* miePeakData = reinterpret_cast< char* >( FileReadToNewBuffer( MIE_PEAK_TXT_FILEPATH ) );

	MeshBuilder graphBuilderR;
	graphBuilderR.Begin( DrawPrimitiveType::LINES, false );
	graphBuilderR.SetColor( Rgba::RED );
	MeshBuilder graphBuilderG;
	graphBuilderG.Begin( DrawPrimitiveType::LINES, false );
	graphBuilderG.SetColor( Rgba::GREEN );
	MeshBuilder graphBuilderB;
	graphBuilderB.Begin( DrawPrimitiveType::LINES, false );
	graphBuilderB.SetColor( Rgba::BLUE );

	float maxGraphValueX = 0.0f;
	float maxGraphValueY = 0.0f;

	TokenizedString choppedLineTokens = TokenizedString( mieChoppedData, "\n" );
	std::vector< std::string > choppedLines = choppedLineTokens.GetTokens();
	TokenizedString peakLineTokens = TokenizedString( miePeakData, "\n" );
	std::vector< std::string > peakLines = peakLineTokens.GetTokens();

	for ( size_t lineNumber = 0; lineNumber < 1801; lineNumber++ )
	{
		TokenizedString choppedLineRGB = TokenizedString( choppedLines[ lineNumber ], " " );
		std::vector< std::string > choppedRGB = choppedLineRGB.GetTokens();

		// LUT data - use only chopped values
		float rValue = stof( choppedRGB[ 0 ] );
		float gValue = stof( choppedRGB[ 1 ] );
		float bValue = stof( choppedRGB[ 2 ] );
		m_miePhaseFunctionMax = Max( m_miePhaseFunctionMax, Max( rValue, Max( gValue, bValue ) ) );	// Really need a variadic function...
		texelsRawValue[ lineNumber ] = Vector4( rValue, gValue, bValue, 1.0f );

		// Graph data
		float peakValue = stof( peakLines[ lineNumber ] );
		rValue = Max( rValue, peakValue );
		gValue = Max( gValue, peakValue );
		bValue = Max( bValue, peakValue );
		float rValuePos = rValue * 10000.0f;			// To avoid negative powers of 10 as values, which messes up the angular graph
		rValuePos = logf( rValuePos );	// Graph uses a logarithmic scale
		float gValuePos = gValue * 10000.0f;
		gValuePos = logf( gValuePos );
		float bValuePos = bValue * 10000.0f;
		bValuePos = logf( bValuePos );

		if ( graphBuilderR.GetVertexCount() > 1 )	// For GL_LINES
		{
			unsigned int lastVertIndex = graphBuilderR.GetVertexCount() - 1;
			VertexBuilder lastVertex = graphBuilderR.GetVertex( lastVertIndex );
			graphBuilderR.PushVertex( lastVertex );
			lastVertex = graphBuilderG.GetVertex( lastVertIndex );
			graphBuilderG.PushVertex( lastVertex );
			lastVertex = graphBuilderB.GetVertex( lastVertIndex );
			graphBuilderB.PushVertex( lastVertex );
		}

		float angleDegrees = static_cast< float >( lineNumber );
		angleDegrees *= 0.1f;	// As the file has 1800 lines for 180 degrees

		Vector2 rVertPos = Vector2( rValuePos, angleDegrees );
		rVertPos.ConvertToCartestian();
		graphBuilderR.PushVertex( Vector3( rVertPos.x, rVertPos.y, 0.0f ) );
		if ( Abs( rVertPos.x ) > maxGraphValueX )
		{
			maxGraphValueX = Abs( rVertPos.x );
		}
		if ( Abs( rVertPos.y ) > maxGraphValueY )
		{
			maxGraphValueY = Abs( rVertPos.y );
		}

		Vector2 gVertPos = Vector2( gValuePos, angleDegrees );
		gVertPos.ConvertToCartestian();
		graphBuilderG.PushVertex( Vector3( gVertPos.x, gVertPos.y, 0.0f ) );
		if ( Abs( gVertPos.x ) > maxGraphValueX )
		{
			maxGraphValueX = Abs( gVertPos.x );
		}
		if ( Abs( gVertPos.y ) > maxGraphValueY )
		{
			maxGraphValueY = Abs( gVertPos.y );
		}

		Vector2 bVertPos = Vector2( bValuePos, angleDegrees );
		bVertPos.ConvertToCartestian();
		graphBuilderB.PushVertex( Vector3( bVertPos.x, bVertPos.y, 0.0f ) );
		if ( Abs( bVertPos.x ) > maxGraphValueX )
		{
			maxGraphValueX = Abs( bVertPos.x );
		}
		if ( Abs( bVertPos.y ) > maxGraphValueY )
		{
			maxGraphValueY = Abs( bVertPos.y );
		}
	}

	Vector3 scaleValue = Vector3( ( 1.0f / maxGraphValueX ), ( 1.0f / maxGraphValueY ), 1.0f );
	for ( unsigned int vertIndex = graphBuilderR.GetVertexCount() - 1; vertIndex < graphBuilderR.GetVertexCount(); vertIndex-- )
	{
		VertexBuilder* rVert = graphBuilderR.GetVertexAsReference( vertIndex );
		rVert->m_position *= scaleValue;
		VertexBuilder* gVert = graphBuilderG.GetVertexAsReference( vertIndex );
		gVert->m_position *= scaleValue;
		VertexBuilder* bVert = graphBuilderB.GetVertexAsReference( vertIndex );
		bVert->m_position *= scaleValue;
	}
	for ( unsigned int vertIndex = graphBuilderR.GetVertexCount() - 1; vertIndex < graphBuilderR.GetVertexCount(); vertIndex-- )
	{
		// Mirroring
		unsigned int lastVertIndex = graphBuilderR.GetVertexCount() - 1;

		VertexBuilder newRVert0 = graphBuilderR.GetVertex( lastVertIndex );
		graphBuilderR.PushVertex( newRVert0 );
		VertexBuilder rVert = graphBuilderR.GetVertex( vertIndex );
		Vector3 newRVert1Pos = rVert.m_position;
		newRVert1Pos.y *= -1.0f;
		graphBuilderR.PushVertex( newRVert1Pos );

		VertexBuilder newGVert0 = graphBuilderG.GetVertex( lastVertIndex );
		graphBuilderG.PushVertex( newGVert0 );
		VertexBuilder gVert = graphBuilderG.GetVertex( vertIndex );
		Vector3 newGVert1Pos = gVert.m_position;
		newGVert1Pos.y *= -1.0f;
		graphBuilderG.PushVertex( newGVert1Pos );

		VertexBuilder newBVert0 = graphBuilderB.GetVertex( lastVertIndex );
		graphBuilderB.PushVertex( newBVert0 );
		VertexBuilder bVert = graphBuilderB.GetVertex( vertIndex );
		Vector3 newBVert1Pos = bVert.m_position;
		newBVert1Pos.y *= -1.0f;
		graphBuilderB.PushVertex( newBVert1Pos );
	}

	graphBuilderR.End();
	graphBuilderG.End();
	graphBuilderB.End();

	Material* graphMaterial = g_renderer->CreateOrGetMaterial( "UI" )->GetInstance();
	graphMaterial->SetTextureAndSampler( 0U, g_renderer->GetDefaultTexture(), g_renderer->GetDefaultSampler() );

	Mesh* graphMeshR = graphBuilderR.CreateMesh();
	m_mieGraphR = new Renderable( graphMeshR, graphMaterial );
	Mesh* graphMeshG = graphBuilderG.CreateMesh();
	m_mieGraphG = new Renderable( graphMeshG, graphMaterial, true, false );
	Mesh* graphMeshB = graphBuilderB.CreateMesh();
	m_mieGraphB = new Renderable( graphMeshB, graphMaterial, true, false );

	free( mieChoppedData );

	// Initialize the grid
	MeshBuilder gridMeshBuilder;
	gridMeshBuilder.Begin( DrawPrimitiveType::LINES, false );
	gridMeshBuilder.SetColor( Rgba::WHITE );
	gridMeshBuilder.PushVertex( Vector3( -1.0f, 0.0f, 0.0f ) );
	gridMeshBuilder.PushVertex( Vector3( 1.0f, 0.0f, 0.0f ) );
	gridMeshBuilder.PushVertex( Vector3( 0.0f, -1.0f, 0.0f ) );
	gridMeshBuilder.PushVertex( Vector3( 0.0f, 1.0f, 0.0f ) );
	gridMeshBuilder.End();

	Mesh* gridMesh = gridMeshBuilder.CreateMesh();
	m_mieGraphGrid = new Renderable( gridMesh, graphMaterial, true, false );

	Rgba colorStamp;
	for ( unsigned int texelIndex = 0U; texelIndex < 1801; texelIndex++ )
	{
		Vector4& rawColor = texelsRawValue[ texelIndex ];
		rawColor.x /= m_miePhaseFunctionMax;
		rawColor.y /= m_miePhaseFunctionMax;
		rawColor.z /= m_miePhaseFunctionMax;
		colorStamp.SetAsFloats( rawColor.x, rawColor.y, rawColor.z, 1.0f );
		mieImage.SetTexel( texelIndex, 0, colorStamp );
	}
	m_mieLUT = g_renderer->CreateFromImage( mieImage, 1U, TEXTURE_FORMAT_RGBA32 );
}

void TheGame::InitializeCloudRenderTargets()
{
	int width = m_sceneColorTarget->GetDimensions().x;
	int height = m_sceneColorTarget->GetDimensions().y;

	m_volumetricShadowTarget = g_renderer->CreateRenderTarget( width, height );

	m_reprojectionTargets[ 0 ] = g_renderer->CreateRenderTarget( width, height, TEXTURE_FORMAT_RGBA32 );	// Higher precision needed for convergence
	Camera* clearCamera = new Camera( m_reprojectionTargets[ 0 ] );
	g_renderer->SetCamera( clearCamera );
	g_renderer->ClearColor( Rgba( 0, 0, 0, 255 ) );	// Initial scattered light = 0, initial transmittance = 1
	clearCamera->RemoveColorTarget( 0U );

	m_reprojectionTargets[ 1 ] = g_renderer->CreateRenderTarget( width, height, TEXTURE_FORMAT_RGBA32 );
	clearCamera->SetColorTarget( m_reprojectionTargets[ 1 ] );
	g_renderer->SetCamera( clearCamera );
	g_renderer->ClearColor( Rgba( 0, 0, 0, 255 ) );	// Initial scattered light = 0, initial transmittance = 1
	clearCamera->RemoveColorTarget( 0U );

	g_renderer->UseDefaultCamera();
	delete clearCamera;
}

void TheGame::InitializeVanDerCorputSequence()
{
	m_vanDerCorputSequence = new float[ m_vanDerCorputPower ];
	MakeVanDerCorputSequence( m_vanDerCorputSequence, m_vanDerCorputPower );
}

#pragma endregion

#pragma region Update

void TheGame::Update()
{
	if ( !IsDevConsoleOpen() )
	{
		m_oldViewMatrix = m_gameCamera->GetViewMatrix();

		HandleFKeyInputs();
		HandleCameraInput();
		HandleCameraPositionSnapInputs();
		HandleSunPositionInput();

		bool debugOverlayOn = HandleOptionsInput();
		if ( !debugOverlayOn )
		{
			HandleShaderToggleInput();
		}

		UpdateAmbientLight();
		UpdateCameraProperties();
		UpdateRenderTargets();
		UpdateShipMatrixFromTransform();

		if ( m_temporalReprojectionEnabled )
		{
			m_vanDerCorputCurrentIndex = ( m_vanDerCorputCurrentIndex + 1U ) % m_vanDerCorputPower;
			m_vanDerCorputCurrentIndex = ClampInt( m_vanDerCorputCurrentIndex, 1U, ( m_vanDerCorputPower - 1U ) );	// The value 0 seems to cause noise issues for raymarch offsets, so don't allow it
		}
		if ( m_detailTexturePanningEnabled )
		{
			m_detailTextureOffset += GetMasterDeltaSecondsF();
		}
	}

	static float s_reprojectionDebug = 1.0f;
	static float s_reprojectionTarget = 2.0f;
	//DebugRenderLogF( GetMasterDeltaSecondsF(), Rgba::RED, Rgba::GREEN, nullptr, DEFAULT_FONT_NAME, "Value: %.6f, Target: %.6f", s_reprojectionDebug, s_reprojectionTarget );
	s_reprojectionDebug = (s_reprojectionDebug * 0.95f + s_reprojectionTarget * 0.05f);
	if ( s_reprojectionTarget - s_reprojectionDebug < 0.000001f )
	{
		s_reprojectionDebug = s_reprojectionTarget;
		s_reprojectionTarget += 1.0f;
	}
}

void TheGame::HandleCameraPositionSnapInputs()
{
	Transform* transformToModify = ( m_possessShip )? &m_shipTransform : m_gameCamera->GetTransform();
	if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_1 ) )
	{
		transformToModify->SetLocalPosition( m_origin + m_snapBottomPosition );
		transformToModify->MarkDirty();
	}
	else if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_2 ) )
	{
		transformToModify->SetLocalPosition(  m_origin + m_snapMiddlePosition );
		transformToModify->MarkDirty();
	}
	else if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_3 ) )
	{
		transformToModify->SetLocalPosition(  m_origin + m_snapTopPosition );
		transformToModify->MarkDirty();
	}
}

void TheGame::HandleFKeyInputs()
{
	if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_F9 ) )
	{
		UpdateNoiseTextures();
	}
}

void TheGame::HandleCameraInput()
{
	// Post-Process settings
	if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_6 ) )
	{
		//( m_gameCamera->IsMSAAEnabled() )? m_gameCamera->DisableMSAA() : m_gameCamera->EnableMSAA( 4U );
	}
	if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_8 ) )
	{
		m_gameCamera->SetBloomEnabled( !m_gameCamera->IsBloomEnabled() );
	}
	if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_P ) )
	{
		TogglePossessShip();
	}

	// Translation
	float translationMagnitude = g_gameConfigBlackboard.GetValue( "cameraTranslationPerSecond", 1.0f );
	float translationMultiplier = 1.f;
	if ( g_inputSystem->IsKeyDown( InputSystem::KEYBOARD_SHIFT ) )
	{
		translationMultiplier = g_gameConfigBlackboard.GetValue( "cameraTranslationMultiplier", 5.f );
	}
	translationMagnitude *= translationMultiplier;
	Transform* transformToUpdate = ( m_possessShip )? &m_shipTransform : m_gameCamera->GetTransform();
	if ( g_inputSystem->IsKeyDown( InputSystem::KEYBOARD_W ) )
	{
		transformToUpdate->Translate( translationMagnitude * m_gameCamera->GetForward() * GetMasterDeltaSecondsF() );
	}
	if ( g_inputSystem->IsKeyDown( InputSystem::KEYBOARD_S ) )
	{
		transformToUpdate->Translate( -translationMagnitude * m_gameCamera->GetForward() * GetMasterDeltaSecondsF() );
	}
	if ( g_inputSystem->IsKeyDown( InputSystem::KEYBOARD_A) )
	{
		transformToUpdate->Translate( -translationMagnitude * m_gameCamera->GetRight() * GetMasterDeltaSecondsF() );
	}
	if ( g_inputSystem->IsKeyDown( InputSystem::KEYBOARD_D ) )
	{
		transformToUpdate->Translate( translationMagnitude * m_gameCamera->GetRight() * GetMasterDeltaSecondsF() );
	}
	if ( g_inputSystem->IsKeyDown( InputSystem::KEYBOARD_T) )
	{
		transformToUpdate->Translate( translationMagnitude * m_gameCamera->GetUp() * GetMasterDeltaSecondsF() );
	}
	if ( g_inputSystem->IsKeyDown( InputSystem::KEYBOARD_G ) )
	{
		transformToUpdate->Translate( -translationMagnitude * m_gameCamera->GetUp() * GetMasterDeltaSecondsF() );
	}
	//m_gameCamera->GetTransform()->m_position.y = Max( m_gameCamera->GetTransform()->m_position.y, ( g_gameConfigBlackboard.GetValue( "terrainHeight", 500.0f ) + 1.0f ) );

	// Rotation
	if ( g_inputSystem->GetMouseClientDelta() != Vector2::ZERO )
	{
		float rotationMagnitude = g_gameConfigBlackboard.GetValue( "cameraRotationDegreesPerSecond", 1.0f );
		Vector2 mouseDelta = g_inputSystem->GetMouseClientDelta();
		mouseDelta = mouseDelta.GetNormalized();

		transformToUpdate->Rotate( 
			Vector3(
				( mouseDelta.y * rotationMagnitude ),
				( mouseDelta.x * rotationMagnitude ),
				0.0f
			)
		);
	}
}

void TheGame::TogglePossessShip()
{
	Vector3 cameraTranslation = g_gameConfigBlackboard.GetValue( "cameraShipRelativeTranslation", Vector3::ZERO );
	if ( m_possessShip )
	{
		m_gameCamera->GetTransform()->Reparent( nullptr );
		Matrix44 modelMatrix = m_shipTransform.GetAsMatrixLocal();
		modelMatrix.Translate( cameraTranslation );
		m_gameCamera->SetPosition( modelMatrix.GetTranslation() );
		m_gameCamera->SetEuler( modelMatrix.GetEulerAngles() );
		m_gameCamera->GetTransform()->MarkDirty();
	}
	else
	{
		m_gameCamera->GetTransform()->Reparent( &m_shipTransform );
		Matrix44 modelMatrix = Matrix44::MakeTranslation( cameraTranslation );
		modelMatrix.Translate( cameraTranslation );
		m_gameCamera->GetTransform()->SetLocalPosition( cameraTranslation );
		m_gameCamera->GetTransform()->SetLocalEuler( Vector3::ZERO );
		m_gameCamera->GetTransform()->MarkDirty();
	}
	m_possessShip = !m_possessShip;
}

void TheGame::HandleSunPositionInput()
{
	bool wasModified = false;
	if ( g_inputSystem->IsKeyDown( InputSystem::KEYBOARD_LEFT_ARROW ) )
	{
		m_sunPositionPolar.y -= g_gameConfigBlackboard.GetValue( "sunRotationDegreesPerSecond", 90.0f ) * GetMasterDeltaSecondsF();
		while ( m_sunPositionPolar.y < 0.0f )
		{
			m_sunPositionPolar.y += 360.0f;
		}

		wasModified = true;
	}
	if ( g_inputSystem->IsKeyDown( InputSystem::KEYBOARD_RIGHT_ARROW ) )
	{
		m_sunPositionPolar.y += g_gameConfigBlackboard.GetValue( "sunRotationDegreesPerSecond", 90.0f ) * GetMasterDeltaSecondsF();
		while ( m_sunPositionPolar.y > 360.0f )
		{
			m_sunPositionPolar.y -= 360.0f;
		}

		wasModified = true;
	}
	if ( g_inputSystem->IsKeyDown( InputSystem::KEYBOARD_DOWN_ARROW ) )
	{
		m_sunPositionPolar.z -= g_gameConfigBlackboard.GetValue( "sunRotationDegreesPerSecond", 90.0f ) * GetMasterDeltaSecondsF();
		m_sunPositionPolar.z = ClampFloat( m_sunPositionPolar.z, 0.0f, 90.0f );

		wasModified = true;
	}
	if ( g_inputSystem->IsKeyDown( InputSystem::KEYBOARD_UP_ARROW ) )
	{
		m_sunPositionPolar.z += g_gameConfigBlackboard.GetValue( "sunRotationDegreesPerSecond", 90.0f ) * GetMasterDeltaSecondsF();
		m_sunPositionPolar.z = ClampFloat( m_sunPositionPolar.z, 0.0f, 90.0f );

		wasModified = true;
	}

	if ( wasModified )
	{
		Vector3 sunPosCartesian = Vector3::ConvertToCartesian( m_sunPositionPolar );
		m_sun->SetDirectionalLight(
			sunPosCartesian,
			Vector3( -sunPosCartesian.x, -sunPosCartesian.y, -sunPosCartesian.z ).GetNormalized(),
			Vector4( 1.0f, 1.0f, 1.0f, m_sunlightIntensity ),
			Vector3::ZERO
		);
	}

	m_sunMovedThisFrame = wasModified;
}

bool TheGame::HandleOptionsInput()
{
	if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_0 ) )
	{
		m_debugOverlayEnabled = !m_debugOverlayEnabled;
	}
	if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_4 ) )
	{
		m_detailTexturePanningEnabled = !m_detailTexturePanningEnabled;
		DebugRenderLogF( 2.f, Rgba::GREEN, Rgba::RED, nullptr, DEFAULT_FONT_NAME, "Detail texture panning enabled: %s", (m_detailTexturePanningEnabled)? "true" : "false" );
	}
	if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_5 ) )
	{
		m_debugLodsEnabled = !m_debugLodsEnabled;
		DebugRenderLogF( 2.f, Rgba::GREEN, Rgba::RED, nullptr, DEFAULT_FONT_NAME, "Debug LODs enabled: %s", (m_debugLodsEnabled)? "true" : "false" );
	}
	if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_6 ) )
	{
		m_temporalReprojectionEnabled = !m_temporalReprojectionEnabled;
		DebugRenderLogF( 2.f, Rgba::GREEN, Rgba::RED, nullptr, DEFAULT_FONT_NAME, "Temporal reprojection enabled: %s", (m_temporalReprojectionEnabled)? "true" : "false" );
	}
	if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_L ) )
	{
		m_usePointSampler = !m_usePointSampler;
		DebugRenderLogF( 2.f, Rgba::GREEN, Rgba::RED, nullptr, DEFAULT_FONT_NAME, "Sampler: %s", ( m_usePointSampler )? "Nearest" : "Linear" );
	}
	if ( m_debugOverlayEnabled )
	{
		if ( g_inputSystem->IsKeyDown( InputSystem::KEYBOARD_N ) )
		{
			if ( g_inputSystem->IsKeyDown( InputSystem::KEYBOARD_SHIFT ) )
			{
				m_shapeTexRenderChannel--;
				if ( m_shapeTexRenderChannel < 0 )
				{
					m_shapeTexRenderChannel = 4;
				}

				m_detailTexRenderChannel--;
				if ( m_detailTexRenderChannel < 0 )
				{
					m_detailTexRenderChannel = 3;
				}
			}
			else
			{
				m_shapeTexRenderLayer--;
				if ( m_shapeTexRenderLayer < 0 )
				{
					m_shapeTexRenderLayer = g_gameConfigBlackboard.GetValue( "shapeNoiseDimensions", IntVector3() ).z - 1;
				}

				m_detailTexRenderLayer--;
				if ( m_detailTexRenderLayer < 0 )
				{
					m_detailTexRenderLayer = g_gameConfigBlackboard.GetValue( "detailNoiseDimensions", IntVector3() ).z - 1;
				}

				m_weatherTexUVOffset -= GetMasterDeltaSecondsF();
				if ( m_weatherTexUVOffset < 0.0f )
				{
					m_weatherTexUVOffset = 1.0f - m_weatherTexUVOffset;
				}
			}
		}
		else if ( g_inputSystem->IsKeyDown( InputSystem::KEYBOARD_M ) )
		{
			if ( g_inputSystem->IsKeyDown( InputSystem::KEYBOARD_SHIFT ) )
			{
				m_shapeTexRenderChannel = ( m_shapeTexRenderChannel + 1 ) % 5;
				m_detailTexRenderChannel = ( m_detailTexRenderChannel + 1 ) % 4;
			}
			else
			{
				m_shapeTexRenderLayer = ( m_shapeTexRenderLayer + 1 ) % g_gameConfigBlackboard.GetValue( "shapeNoiseDimensions", IntVector3() ).z;
				m_detailTexRenderLayer = ( m_detailTexRenderLayer + 1 ) % g_gameConfigBlackboard.GetValue( "detailNoiseDimensions", IntVector3() ).z;

				m_weatherTexUVOffset += GetMasterDeltaSecondsF();
				if ( m_weatherTexUVOffset > 1.0f )
				{
					m_weatherTexUVOffset -= 1.0f;
				}
			}
		}
		return true;
	}

	return false;
}

void TheGame::MoveCameraTo( const Vector3& position )
{
	Transform* transformToModify = ( m_possessShip )? &m_shipTransform : m_gameCamera->GetTransform();
	transformToModify->SetLocalPosition( position );
	transformToModify->MarkDirty();
}

void TheGame::MoveCamera( const Vector3& displacement )
{
	Transform* transformToModify = ( m_possessShip )? &m_shipTransform : m_gameCamera->GetTransform();
	Vector3 newPosition = m_gameCamera->GetPosition() + displacement;
	transformToModify->SetLocalPosition( newPosition );
	transformToModify->MarkDirty();
}

void TheGame::CameraLookAt( const Vector3& lookAtPosition )
{
	Vector3 cameraPos = m_gameCamera->GetPosition();
	m_gameCamera->LookAt( cameraPos, lookAtPosition );
}

std::string TheGame::GetNameForCurrentToggleOption() const
{
	switch ( m_toggleOption )
	{
		case CLOUD_LAYER_ALTITUDE			:	return "Cloud layer altitude";
		case CLOUD_LAYER_HEIGHT				:	return "Cloud layer height";
		case SHADOWS						:	return "Shadows";
		case WEATHER_TEXTURE_TILE			:	return "Weather texture tiling";
		case SHAPE_TEXTURE_TILE				:	return "Shape texture tiling";
		case DETAIL_TEXTURE_TILE			:	return "Detail texture tiling";
		case SHAPE_WORLEY_INVERT_FACTOR		:	return "Shape worley invert factor";
		case DETAIL_WORLEY_INVERT_FACTOR	:	return "Detail worley invert factor";
		case SUNLIGHT_INTENSITY				:	return "Sunlight intensity";
		case MAX_DENSITY					:	return "Max cloud density";
		case CAMERA_FOV_DEGREES				:	return "Camera FOV";
		case VAN_DER_CORPUT_NUMBER			:	return "Van der Corput number";
		case GOD_RAYS_WEIGHT				:	return "God Rays - Weight";
		case GOD_RAYS_DECAY					:	return "God Rays - Decay";
		case GOD_RAYS_EXPOSURE				:	return "God Rays - Exposure";
		case GOD_RAYS_NUM_SAMPLES			:	return "God Rays - Num Samples";
		default	:	return "";
	}
}

void TheGame::HandleShaderToggleInput()
{
	float wheelDelta = g_inputSystem->GetMouseWheelDelta();
	if ( !IsFloatEqualTo( wheelDelta, 0.0f ) )
	{
		int sign = static_cast< int >( Sign( wheelDelta ) );
		m_toggleOption = static_cast< ShaderToggleOption >( static_cast< int >( m_toggleOption ) + sign );
		if ( m_toggleOption < 0 )
		{
			m_toggleOption = static_cast< ShaderToggleOption >(static_cast< int >( NUM_TOGGLE_OPTIONS ) - 1 );
		}
		else
		{
			m_toggleOption = static_cast< ShaderToggleOption >( m_toggleOption % NUM_TOGGLE_OPTIONS );
		}
		DebugRenderLogF( 2.0f, Rgba::GREEN, Rgba::RED, nullptr, DEFAULT_FONT_NAME, "Toggle Option: %s", GetNameForCurrentToggleOption().c_str() );
	}
	if ( g_inputSystem->IsKeyDown( InputSystem::KEYBOARD_N ) )
	{
		std::string debugText = "";
		switch ( m_toggleOption )
		{
			case CLOUD_LAYER_ALTITUDE			:	m_cloudLayerAltitude -= 10.0f * GetMasterDeltaSecondsF(); m_cloudLayerAltitude = Max( m_cloudLayerAltitude, g_gameConfigBlackboard.GetValue( "terrainHeight", 500.0f ) );	debugText = Stringf( "%s: %f", GetNameForCurrentToggleOption().c_str(), m_cloudLayerAltitude );	break;
			case CLOUD_LAYER_HEIGHT				:	m_cloudLayerHeight -= 5.0f * GetMasterDeltaSecondsF(); m_cloudLayerHeight = Max( m_cloudLayerHeight, 0.0f );	debugText = Stringf( "%s: %f", GetNameForCurrentToggleOption().c_str(), m_cloudLayerHeight );	break;
			case SHADOWS						:	m_shadowsEnabled = 0.0f;	debugText = Stringf( "%s: %f", GetNameForCurrentToggleOption().c_str(), m_shadowsEnabled );	break;
			case WEATHER_TEXTURE_TILE			:	m_weatherTextureTileFactor -= GetMasterDeltaSecondsF();	m_weatherTextureTileFactor = Max( m_weatherTextureTileFactor, 1.0f );	debugText = Stringf( "%s: %f", GetNameForCurrentToggleOption().c_str(), m_weatherTextureTileFactor );	break;
			case SHAPE_TEXTURE_TILE				:	m_shapeTextureTileFactor -= GetMasterDeltaSecondsF();	m_shapeTextureTileFactor = Max( m_shapeTextureTileFactor, 1.0f );	debugText = Stringf( "%s: %f", GetNameForCurrentToggleOption().c_str(), m_shapeTextureTileFactor );	break;
			case DETAIL_TEXTURE_TILE			:	m_detailTextureTileFactor -= GetMasterDeltaSecondsF();	m_detailTextureTileFactor = Max( m_detailTextureTileFactor, 1.0f );	debugText = Stringf( "%s: %f", GetNameForCurrentToggleOption().c_str(), m_detailTextureTileFactor );	break;
			case SHAPE_WORLEY_INVERT_FACTOR		:	m_shapeWorleyInvertFactor -= GetMasterDeltaSecondsF();	m_shapeWorleyInvertFactor = ClampFloat( m_shapeWorleyInvertFactor, 0.0f, 1.0f );	debugText = Stringf( "%s: %f", GetNameForCurrentToggleOption().c_str(), m_shapeWorleyInvertFactor );	break;
			case DETAIL_WORLEY_INVERT_FACTOR	:	m_detailWorleyInvertFactor -= GetMasterDeltaSecondsF();	m_detailWorleyInvertFactor = ClampFloat( m_detailWorleyInvertFactor, 0.0f, 1.0f );	debugText = Stringf( "%s: %f", GetNameForCurrentToggleOption().c_str(), m_detailWorleyInvertFactor );	break;
			case MAX_DENSITY					:	m_maxCloudDensity -= GetMasterDeltaSecondsF();	m_maxCloudDensity = ClampFloat( m_maxCloudDensity, 0.0f, 1.0f );	debugText = Stringf( "%s: %f", GetNameForCurrentToggleOption().c_str(), m_maxCloudDensity );	break;
			case SUNLIGHT_INTENSITY				:	m_sunlightIntensity -= 50.0f * GetMasterDeltaSecondsF();	m_sunlightIntensity = Max( m_sunlightIntensity, 0.0f );	debugText = Stringf( "%s: %f", GetNameForCurrentToggleOption().c_str(), m_sunlightIntensity );	break;
			case CAMERA_FOV_DEGREES				:	m_cameraFOV -= GetMasterDeltaSecondsF();	m_cameraFOV = Max( m_cameraFOV, 20.0f );	debugText = Stringf( "%s: %f", GetNameForCurrentToggleOption().c_str(), m_cameraFOV );	break;
			case VAN_DER_CORPUT_NUMBER			:	m_vanDerCorputCurrentIndex = ( m_vanDerCorputCurrentIndex == 0U )? ( m_vanDerCorputPower - 1U ) : ( m_vanDerCorputCurrentIndex - 1U );	debugText = Stringf( "%s: %u", GetNameForCurrentToggleOption().c_str(), m_vanDerCorputCurrentIndex );	break;
			case GOD_RAYS_WEIGHT				:	m_godRaysWeight = ClampFloat( m_godRaysWeight - ( 0.05f * GetMasterDeltaSecondsF() ), 0.f, 1.f );	debugText = Stringf( "%s: %.2f", GetNameForCurrentToggleOption().c_str(), m_godRaysWeight );	break;
			case GOD_RAYS_DECAY					:	m_godRaysDecay = ClampFloat( m_godRaysDecay - ( 0.01f * GetMasterDeltaSecondsF() ), 0.f, 1.f );	debugText = Stringf( "%s: %.2f", GetNameForCurrentToggleOption().c_str(), m_godRaysDecay );	break;
			case GOD_RAYS_EXPOSURE				:	m_godRaysExposure = ClampFloat( m_godRaysExposure - ( 0.1f * GetMasterDeltaSecondsF() ), 0.f, 100.f );	debugText = Stringf( "%s: %.2f", GetNameForCurrentToggleOption().c_str(), m_godRaysExposure );	break;
			case GOD_RAYS_NUM_SAMPLES			:	m_godRaysNumSamples = ClampFloat( m_godRaysNumSamples - ( 5.f * GetMasterDeltaSecondsF() ), 0.f, 5000.f );	debugText = Stringf( "%s: %.2f", GetNameForCurrentToggleOption().c_str(), m_godRaysNumSamples );	break;
		}

		DebugRenderLogF( 2.0f, Rgba::GREEN, Rgba::RED, nullptr, DEFAULT_FONT_NAME, debugText.c_str() );
	}
	if ( g_inputSystem->IsKeyDown( InputSystem::KEYBOARD_M ) )
	{
		std::string debugText = "";
		switch ( m_toggleOption )
		{
			case CLOUD_LAYER_ALTITUDE			:	m_cloudLayerAltitude += 10.0f * GetMasterDeltaSecondsF();	debugText = Stringf( "%s: %f", GetNameForCurrentToggleOption().c_str(), m_cloudLayerAltitude );	break;
			case CLOUD_LAYER_HEIGHT				:	m_cloudLayerHeight += 5.0f * GetMasterDeltaSecondsF();	debugText = Stringf( "%s: %f", GetNameForCurrentToggleOption().c_str(), m_cloudLayerHeight );	break;
			case SHADOWS						:	m_shadowsEnabled = 1.0f;	debugText = Stringf( "%s: %f", GetNameForCurrentToggleOption().c_str(), m_shadowsEnabled );	break;
			case WEATHER_TEXTURE_TILE			:	m_weatherTextureTileFactor += GetMasterDeltaSecondsF();	m_weatherTextureTileFactor = Max( m_weatherTextureTileFactor, 1.0f );	debugText = Stringf( "%s: %f", GetNameForCurrentToggleOption().c_str(), m_weatherTextureTileFactor );	break;
			case SHAPE_TEXTURE_TILE				:	m_shapeTextureTileFactor += GetMasterDeltaSecondsF();	m_shapeTextureTileFactor = Max( m_shapeTextureTileFactor, 1.0f );	debugText = Stringf( "%s: %f", GetNameForCurrentToggleOption().c_str(), m_shapeTextureTileFactor );	break;
			case DETAIL_TEXTURE_TILE			:	m_detailTextureTileFactor += GetMasterDeltaSecondsF();	m_detailTextureTileFactor = Max( m_detailTextureTileFactor, 1.0f );	debugText = Stringf( "%s: %f", GetNameForCurrentToggleOption().c_str(), m_detailTextureTileFactor );	break;
			case SHAPE_WORLEY_INVERT_FACTOR		:	m_shapeWorleyInvertFactor += GetMasterDeltaSecondsF();	m_shapeWorleyInvertFactor = ClampFloat( m_shapeWorleyInvertFactor, 0.0f, 1.0f );	debugText = Stringf( "%s: %f", GetNameForCurrentToggleOption().c_str(), m_shapeWorleyInvertFactor );	break;
			case DETAIL_WORLEY_INVERT_FACTOR	:	m_detailWorleyInvertFactor += GetMasterDeltaSecondsF();	m_detailWorleyInvertFactor = ClampFloat( m_detailWorleyInvertFactor, 0.0f, 1.0f );	debugText = Stringf( "%s: %f", GetNameForCurrentToggleOption().c_str(), m_detailWorleyInvertFactor );	break;
			case MAX_DENSITY					:	m_maxCloudDensity += GetMasterDeltaSecondsF();	m_maxCloudDensity = ClampFloat( m_maxCloudDensity, 0.0f, 1.0f );	debugText = Stringf( "%s: %f", GetNameForCurrentToggleOption().c_str(), m_maxCloudDensity );	break;
			case SUNLIGHT_INTENSITY				:	m_sunlightIntensity += 50.0f * GetMasterDeltaSecondsF();	m_sunlightIntensity = Max( m_sunlightIntensity, 0.0f );	debugText = Stringf( "%s: %f", GetNameForCurrentToggleOption().c_str(), m_sunlightIntensity );	break;
			case CAMERA_FOV_DEGREES				:	m_cameraFOV += GetMasterDeltaSecondsF();	m_cameraFOV = Min( m_cameraFOV, 90.0f );	debugText = Stringf( "%s: %f", GetNameForCurrentToggleOption().c_str(), m_cameraFOV );	break;
			case VAN_DER_CORPUT_NUMBER			:	m_vanDerCorputCurrentIndex = ( m_vanDerCorputCurrentIndex + 1U ) % m_vanDerCorputPower;	debugText = Stringf( "%s: %u", GetNameForCurrentToggleOption().c_str(), m_vanDerCorputCurrentIndex );	break;
			case GOD_RAYS_WEIGHT				:	m_godRaysWeight = ClampFloat( m_godRaysWeight + ( 0.05f * GetMasterDeltaSecondsF() ), 0.f, 1.f );	debugText = Stringf( "%s: %.2f", GetNameForCurrentToggleOption().c_str(), m_godRaysWeight );	break;
			case GOD_RAYS_DECAY					:	m_godRaysDecay = ClampFloat( m_godRaysDecay + ( 0.01f * GetMasterDeltaSecondsF() ), 0.f, 1.f );	debugText = Stringf( "%s: %.2f", GetNameForCurrentToggleOption().c_str(), m_godRaysDecay );	break;
			case GOD_RAYS_EXPOSURE				:	m_godRaysExposure = ClampFloat( m_godRaysExposure + ( 0.1f * GetMasterDeltaSecondsF() ), 0.f, 100.f );	debugText = Stringf( "%s: %.2f", GetNameForCurrentToggleOption().c_str(), m_godRaysExposure );	break;
			case GOD_RAYS_NUM_SAMPLES			:	m_godRaysNumSamples = ClampFloat( m_godRaysNumSamples + ( 5.f * GetMasterDeltaSecondsF() ), 0.f, 5000.f );	debugText = Stringf( "%s: %.2f", GetNameForCurrentToggleOption().c_str(), m_godRaysNumSamples );	break;
		}

		DebugRenderLogF( 2.0f, Rgba::GREEN, Rgba::RED, nullptr, DEFAULT_FONT_NAME, debugText.c_str() );
	}
	if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_C ) )
	{
		m_cloudsEnabled = !m_cloudsEnabled;
	}
	if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_V ) )
	{
		m_godRaysEnabled = !m_godRaysEnabled;
	}
}

void TheGame::UpdateAmbientLight()
{
	Vector4 ambientLight = g_renderer->GetAmbientLight();
	ambientLight.w = RangeMapFloat( m_sunPositionPolar.z, 0.0f, 90.0f, 0.01f, 1.0f );
	ambientLight.w = SmoothStop3( ambientLight.w );
	g_renderer->SetAmbientLight( Vector4( 1.f, 1.f, 1.f, 1.f ) );
	//g_renderer->SetAmbientLight( ambientLight );
}

void TheGame::UpdateCameraProperties()
{
	m_gameCamera->SetProjection( Matrix44::MakePerspective( m_cameraFOV, Window::GetAspect(), CAMERA_NEAR_Z, CAMERA_FAR_Z ) );
}

void TheGame::UpdateRenderTargets()
{
	unsigned int nextRenderTargetIndex = ( m_currentReprojectionTargetIndex + 1U ) % 2U;
	m_currentReprojectionTargetIndex = nextRenderTargetIndex;
}

void TheGame::UpdateShipMatrixFromTransform()
{
	Matrix44 modelMatrix = m_shipTransform.GetAsMatrixLocal();
	m_ship->SetModelMatrix( modelMatrix );
}

void TheGame::UpdateNoiseTextures()
{
	Shader* computeShader = g_renderer->CreateOrGetShader( "3DNoiseCompute" );
	ShaderProgram* computeProgram = computeShader->GetPass( 0U )->GetProgram();

	IntVector3 shapeDimensions = g_gameConfigBlackboard.GetValue( "shapeNoiseDimensions", IntVector3() );
	g_renderer->BindTexture3DAsImage( m_shapeTexture, TextureAccessType::TEXTURE_ACCESS_READ_WRITE );
	g_renderer->DispatchCompute( computeProgram, shapeDimensions.x, shapeDimensions.y, shapeDimensions.z );
	m_shapeTexture->RegenerateMipMaps();

	IntVector3 detailDimensions = g_gameConfigBlackboard.GetValue( "detailNoiseDimensions", IntVector3() );
	g_renderer->BindTexture3DAsImage( m_detailTexture, TextureAccessType::TEXTURE_ACCESS_READ_WRITE );
	g_renderer->DispatchCompute( computeProgram, detailDimensions.x, detailDimensions.y, detailDimensions.z );
	m_detailTexture->RegenerateMipMaps();
}

#pragma endregion

#pragma region Render

void TheGame::Render() const
{
	// Render 3D to low-res targets
	RenderInit();
	RenderScene();
	RenderSky();
	if ( m_cloudsEnabled )
	{
		RenderScatteringTransmittance();
		RenderPostProcess();
		RenderDeferredLighting();
	}
	// Copy low-res image to high-res default target
	RenderCopyToDefaultColorTarget();

	// Render 2D to high-res default target
	// Switch back to default color target after scene has been copied to it
	m_gameCamera->SetColorTarget( g_renderer->GetDefaultColorTarget() );
	m_gameCamera->SetDepthStencilTarget( nullptr );
	g_renderer->SetCamera( m_gameCamera );
	RenderGeneralDebugInfo();
	RenderVisualDebugOverlay();
}

void TheGame::RenderInit() const
{
	// Clear engine's default color target (default depth not used)
	m_gameCamera->SetColorTarget( g_renderer->GetDefaultColorTarget(), 0U );
	g_renderer->SetCamera( m_gameCamera );
	g_renderer->ClearColor();

	// Clear scene targets
	m_gameCamera->RemoveColorTarget( 0U );
	m_gameCamera->SetColorTarget( m_sceneColorTarget, 0U );
	m_gameCamera->SetColorTarget( m_sceneNormalTarget, 1U );
	m_gameCamera->SetColorTarget( m_sceneSpecularTarget, 2U );
	m_gameCamera->SetDepthStencilTarget( m_sceneDepthTargets[m_currentReprojectionTargetIndex] );
	g_renderer->SetCamera( m_gameCamera );
	g_renderer->ClearColor();
	g_renderer->WriteDepthImmediate( true );
	g_renderer->ClearDepth( 1.0f );

	m_gameCamera->RemoveColorTarget( 2U );
	m_gameCamera->RemoveColorTarget( 1U );

	m_sun->UpdateUBOFromTransform();
	g_renderer->BindLights( *m_sceneLights );
}

void TheGame::RenderScene() const
{
	PROFILE_SCOPE( "TheGame::RenderScene()" );

	m_gameCamera->SetColorTarget( m_sceneNormalTarget, 1U );
	m_gameCamera->SetColorTarget( m_sceneSpecularTarget, 2U );
	g_renderer->SetCamera( m_gameCamera );

	Material* materialRef = g_renderer->CreateOrGetMaterial( "Terrain" );
	g_renderer->BindMaterial( *materialRef );
	for ( Renderable* chunk : m_terrainChunks )
	{
		g_renderer->DrawRenderable( *chunk );
	}

	materialRef = g_renderer->CreateOrGetMaterial( "Ship" );
	g_renderer->BindMaterial( *materialRef );
	g_renderer->DrawRenderable( *m_ship );

	m_gameCamera->RemoveColorTarget( 2U );
	m_gameCamera->RemoveColorTarget( 1U );
	g_renderer->SetCamera( m_gameCamera );
}

void TheGame::RenderSky() const
{
	PROFILE_SCOPE( "TheGame::RenderSky()" );

	Texture* depthTarget =  m_sceneDepthTargets[m_currentReprojectionTargetIndex];
	m_gameCamera->SetDepthStencilTarget( nullptr );
	g_renderer->SetCamera( m_gameCamera );

	Texture* originalTarget = m_sceneColorTarget;
	
	Material* skyEffect = g_renderer->CreateOrGetMaterial( "Sky" );
	skyEffect->SetProperty( "SUNLIGHT_INTENSITY", m_sunlightIntensity );
	skyEffect->SetProperty( "MIE_PHASE_FUNCTION_MAX_VALUE", m_miePhaseFunctionMax );
	skyEffect->SetProperty( "TERRAIN_HEIGHT", g_gameConfigBlackboard.GetValue( "terrainHeight", 500.0f ) );
	skyEffect->SetProperty( "ATMOSPHERE_LAYER_ALTITUDE", m_cloudLayerAltitude + ( 2.0f * m_cloudLayerHeight ) );
	skyEffect->SetProperty( "ATMOSPHERE_LAYER_HEIGHT", 10000.0f );
	skyEffect->SetTextureAndSampler( 0U, originalTarget, m_clampSampler );
	skyEffect->SetTextureAndSampler( 1U, depthTarget, m_clampSampler );
	skyEffect->SetTextureAndSampler( 2U, m_mieLUT, m_clampSampler );

	g_renderer->ApplyEffect( skyEffect, m_gameCamera, m_lowResScratchTarget );
	g_renderer->FinishEffects( originalTarget );
}

void TheGame::RenderScatteringTransmittance() const
{
	PROFILE_SCOPE( "TheGame::RenderScatteringTransmittance()" );

	Material* raymarcher = g_renderer->CreateOrGetMaterial( "Volumetric" );
	raymarcher->SetProperty( "OLD_VIEW", m_oldViewMatrix );
	raymarcher->SetProperty( "RAYMARCH_OFFSET_FRACTION", m_vanDerCorputSequence[ m_vanDerCorputCurrentIndex ] );	// TODO: May need to get a better random
	raymarcher->SetProperty( "TERRAIN_HEIGHT", g_gameConfigBlackboard.GetValue( "terrainHeight", 500.0f ) );
	raymarcher->SetProperty( "CLOUD_LAYER_ALTITUDE", m_cloudLayerAltitude );
	raymarcher->SetProperty( "CLOUD_LAYER_HEIGHT", m_cloudLayerHeight );
	raymarcher->SetProperty( "SHADOWS_ENABLED", m_shadowsEnabled );
	raymarcher->SetProperty( "TEMPORAL_REPROJECTION_ENABLED", (m_temporalReprojectionEnabled)? 1.0f : 0.0f );
	raymarcher->SetProperty( "SUN_MOVED_THIS_FRAME", (m_sunMovedThisFrame)? 1.0f : 0.0f );
	raymarcher->SetProperty( "WEATHER_TEXTURE_TILE_FACTOR", m_weatherTextureTileFactor );
	raymarcher->SetProperty( "SHAPE_TEXTURE_TILE_FACTOR", m_shapeTextureTileFactor );
	raymarcher->SetProperty( "DETAIL_TEXTURE_TILE_FACTOR", m_detailTextureTileFactor );
	raymarcher->SetProperty( "DETAIL_TEXTURE_OFFSET", m_detailTextureOffset );
	raymarcher->SetProperty( "DETAIL_TEXTURE_PANNING", (m_detailTexturePanningEnabled)? 1.0f : 0.0f );
	raymarcher->SetProperty( "SHAPE_WORLEY_INVERSE", m_shapeWorleyInvertFactor );
	raymarcher->SetProperty( "DETAIL_WORLEY_INVERSE", m_detailWorleyInvertFactor );
	raymarcher->SetProperty( "MAX_DENSITY", m_maxCloudDensity );
	raymarcher->SetProperty( "MIE_PHASE_FUNCTION_MAX_VALUE", m_miePhaseFunctionMax );
	raymarcher->SetProperty( "SUNLIGHT_INTENSITY", m_sunlightIntensity );
	raymarcher->SetProperty( "DEBUG_VIEW_LODS", (m_debugLodsEnabled)? 1.0f : 0.0f );
	raymarcher->SetTextureAndSampler( 0U, m_weatherTexture, g_renderer->GetDefaultSampler(true) );
	raymarcher->SetTextureAndSampler( 1U, m_heightSignal, g_renderer->GetDefaultSampler(true) );
	g_renderer->BindTexture3DAndSampler( 2U, *m_shapeTexture, *g_renderer->GetDefaultSampler(true) );
	g_renderer->BindTexture3DAndSampler( 3U, *m_detailTexture, *g_renderer->GetDefaultSampler(true) );
	g_renderer->BindTextureAndSampler( 4U, *m_mieLUT, *m_clampSampler );
	raymarcher->SetTextureAndSampler( 5U, m_reprojectionTargets[ m_currentReprojectionTargetIndex ], m_clampSampler );
	raymarcher->SetTextureAndSampler( 6U, m_sceneDepthTargets[m_currentReprojectionTargetIndex], m_clampSampler );
	raymarcher->SetTextureAndSampler( 7U, m_sceneDepthTargets[(m_currentReprojectionTargetIndex + 1) % 2], m_clampSampler );

	m_gameCamera->SetDepthStencilTarget( nullptr );
	m_gameCamera->SetColorTarget( m_reprojectionTargets[ ( m_currentReprojectionTargetIndex + 1U ) % 2U ], 0U );
	m_gameCamera->SetColorTarget( m_volumetricShadowTarget, 1U );
	g_renderer->SetCamera( m_gameCamera );

	g_renderer->DrawFullScreenImmediate( *raymarcher );
	
	m_gameCamera->RemoveColorTarget(1U);
	m_gameCamera->Finalize();
};

void TheGame::RenderPostProcess() const
{
	PROFILE_SCOPE( "TheGame::RenderPostProcess()" );
	m_gameCamera->SetColorTarget( m_sceneColorTarget );

	if (m_godRaysEnabled)
	{
		Material* godRayShader = g_renderer->CreateOrGetMaterial( "GodRays" );
		godRayShader->SetProperty( "NUM_SAMPLES", m_godRaysNumSamples );
		godRayShader->SetProperty( "WEIGHT", m_godRaysWeight );
		godRayShader->SetProperty( "DECAY", m_godRaysDecay );
		godRayShader->SetProperty( "EXPOSURE", m_godRaysExposure );
		godRayShader->SetTextureAndSampler( 1U, m_reprojectionTargets[ ( m_currentReprojectionTargetIndex + 1U ) % 2U ], m_clampSampler );
		g_renderer->ApplyEffect( godRayShader, m_gameCamera, m_lowResScratchTarget );
		g_renderer->FinishEffects( m_sceneColorTarget );
	}
}

void TheGame::RenderDeferredLighting() const
{
	PROFILE_SCOPE( "TheGame::RenderDeferredLighting()" );

	m_gameCamera->GetFrameBuffer()->SetDepthStencilTarget( nullptr );
	Material* cloudFullscreenShader = g_renderer->CreateOrGetMaterial( "CloudsFullscreen" );
	cloudFullscreenShader->SetTextureAndSampler( 1U, m_reprojectionTargets[ ( m_currentReprojectionTargetIndex + 1U ) % 2U ], m_clampSampler );
	cloudFullscreenShader->SetTextureAndSampler( 2U, m_sceneNormalTarget, m_clampSampler );
	cloudFullscreenShader->SetTextureAndSampler( 3U, m_volumetricShadowTarget, m_clampSampler );
	cloudFullscreenShader->SetTextureAndSampler( 4U, m_sceneSpecularTarget, m_clampSampler );
	cloudFullscreenShader->SetTextureAndSampler( 5U, m_sceneDepthTargets[m_currentReprojectionTargetIndex], m_clampSampler );
	g_renderer->ApplyEffect( cloudFullscreenShader, m_gameCamera, m_lowResScratchTarget );
	g_renderer->FinishEffects( m_sceneColorTarget );
}

void TheGame::RenderCopyToDefaultColorTarget() const
{
	Material* fullscreenPassThroughShader = g_renderer->CreateOrGetMaterial( "Fullscreen" );
	fullscreenPassThroughShader->SetTextureAndSampler( 0U, m_sceneColorTarget, g_renderer->GetDefaultSampler() );

	m_gameCamera->SetColorTarget( g_renderer->GetDefaultColorTarget(), 0U );
	m_gameCamera->SetDepthStencilTarget( nullptr );
	g_renderer->SetCamera( m_gameCamera );
	g_renderer->DrawFullScreenImmediate( *fullscreenPassThroughShader );
}

void TheGame::RenderGeneralDebugInfo() const
{
	PROFILE_SCOPE( "TheGame::RenderGeneralDebugInfo()" );

	static float s_frameTime = 0.f;
	float currentTime = GetCurrentTimeSecondsF();
	s_frameTime = currentTime;

#ifdef NO_DEBUG
#elif defined( LOG_DEBUG )	// High performance - don't print to screen, but log FPS
	float deltaTime = currentTime - s_frameTime;
	LogPrintf(
		"%.3fms | %.3f FPS", ( deltaTime * 1000.f ), ( ( deltaTime > 0.f )? ( 1.f / deltaTime ) : 0.f )
	);
#else
	float deltaTime = currentTime - s_frameTime;
	float deltaTimeForPrint = GetMasterDeltaSecondsF();
	DebugRenderLogF(
		deltaTimeForPrint,
		Rgba::WHITE,
		Rgba::WHITE,
		nullptr,
		DEFAULT_FONT_NAME,
		"%.3fms | %.3f FPS", ( deltaTime * 1000.f ), ( ( deltaTime > 0.f )? ( 1.f / deltaTime ) : 0.f )
	);
	
	Vector3 cameraPosition = m_gameCamera->GetPosition();
	DebugRenderLogF(
		deltaTimeForPrint,
		Rgba::YELLOW,
		Rgba::YELLOW,
		nullptr,
		DEFAULT_FONT_NAME,
		"Camera position: %.2f, %.2f, %.2f",
		cameraPosition.x, cameraPosition.y, cameraPosition.z
	);
#endif
}

void TheGame::RenderVisualDebugOverlay() const
{
	PROFILE_SCOPE( "TheGame::RenderVisualDebugOverlay()" );

	m_debugOverlayCamera->SetViewport( AABB2( 0.0f, 0.0f, 1.0f, 1.0f ) );
	g_renderer->SetCamera( m_debugOverlayCamera );		// Set this here as the default target needs to be bound anyway

	if ( m_debugOverlayEnabled )
	{
		Material* fullscreenOverlay = g_renderer->CreateOrGetMaterial( "Fullscreen" );
		fullscreenOverlay->SetTextureAndSampler( 0U, g_renderer->CreateOrGetTexture( "Data/Images/Texture_Default.png" ), g_renderer->GetDefaultSampler() );
		Rgba overlayColor = Rgba::BLACK.GetWithAlpha( 0.5f );
		g_renderer->DrawFullScreenImmediate( *fullscreenOverlay, overlayColor );

		g_renderer->DrawText2D(
			Vector2( -0.85f, -0.45f ),
			"World Depth",
			0.0325f,
			Rgba::WHITE,
			0.75f,
			m_defaultFont
		);

		g_renderer->DrawText2D(
			Vector2( -0.45f, -0.45f ),
			"Mie phase function",
			0.0325f,
			Rgba::WHITE,
			0.75f,
			m_defaultFont
		);

		g_renderer->DrawText2D(
			Vector2( -0.3f, 0.15f ),
			"Mie LUT",
			0.0325f,
			Rgba::WHITE,
			0.75f,
			m_defaultFont
		);

		g_renderer->DrawText2D(
			Vector2( -0.95f, 0.15f ),
			"Scene Shadows",
			0.0325f,
			Rgba::WHITE,
			0.75f,
			m_defaultFont
		);

		g_renderer->DrawText2D(
			Vector2( -0.925f, 0.75f ),
			"Scene Normals",
			0.0325f,
			Rgba::WHITE,
			0.75f,
			m_defaultFont
		);

		g_renderer->DrawText2D(
			Vector2( -0.425f, 0.75f ),
			"Scene Specular",
			0.0325f,
			Rgba::WHITE,
			0.75f,
			m_defaultFont
		);

		g_renderer->DrawText2D(
			Vector2( 0.6f, 0.15f ),
			"Scat-Transm",
			0.0325f,
			Rgba::WHITE,
			0.75f,
			m_defaultFont
		);

		g_renderer->DrawText2D(
			Vector2( 0.55f, -0.4f ),
			"Weather Texture",
			0.0325f,
			Rgba::WHITE,
			0.75f,
			m_defaultFont
		);

		g_renderer->DrawText2D(
			Vector2( 0.1f, -0.4f ),
			"Shape Texture",
			0.0325f,
			Rgba::WHITE,
			0.75f,
			m_defaultFont
		);
		g_renderer->DrawText2D(
			Vector2( 0.05f, -0.45f ),
			"Channel " + std::to_string( m_shapeTexRenderChannel ),
			0.0225f,
			Rgba::WHITE,
			0.75f,
			m_defaultFont
		);
		g_renderer->DrawText2D(
			Vector2( 0.3f, -0.45f ),
			"Layer " + std::to_string( m_shapeTexRenderLayer ),
			0.0225f,
			Rgba::WHITE,
			0.75f,
			m_defaultFont
		);

		g_renderer->DrawText2D(
			Vector2( 0.1f, 0.2f ),
			"Detail Texture",
			0.0325f,
			Rgba::WHITE,
			0.75f,
			m_defaultFont
		);
		g_renderer->DrawText2D(
			Vector2( 0.05f, 0.15f ),
			"Channel " + std::to_string( m_detailTexRenderChannel ),
			0.0225f,
			Rgba::WHITE,
			0.75f,
			m_defaultFont
		);
		g_renderer->DrawText2D(
			Vector2( 0.3f, 0.15f ),
			"Layer " + std::to_string( m_detailTexRenderLayer ),
			0.0225f,
			Rgba::WHITE,
			0.75f,
			m_defaultFont
		);

		RenderDepthTargetDebug();
		RenderScatteringTransmittanceDebug();
		RenderSceneShadowsDebug();
		RenderSceneNormalsDebug();
		RenderSceneSpecularDebug();
		RenderWeatherTextureDebug();
		RenderMieLUTDebug();
		RenderMieGraph();
		RenderShapeTextureLayerDebug();
		RenderDetailTextureLayerDebug();
	}
}

void TheGame::RenderDepthTargetDebug() const
{
	Texture* depthTarget = m_sceneDepthTargets[m_currentReprojectionTargetIndex];
	m_debugOverlayCamera->SetViewport( AABB2( 0.0f, 0.0f, 1.0f, 1.0f ) );
	g_renderer->SetCamera( m_debugOverlayCamera );
	g_renderer->BindMaterial( *g_renderer->CreateOrGetMaterial( "FullscreenDepth" ) );
	g_renderer->DrawTexturedAABB( AABB2( -0.95f, -0.95f, -0.50f, -0.50f ), *depthTarget, Vector2::ZERO, Vector2::ONE, Rgba::WHITE );
	// TODO: Debug render old depth target?
}

void TheGame::RenderScatteringTransmittanceDebug() const
{
	m_debugOverlayCamera->SetViewport( AABB2( 0.0f, 0.0f, 1.0f, 1.0f ) );
	g_renderer->SetCamera( m_debugOverlayCamera );
	g_renderer->BindMaterial( *g_renderer->CreateOrGetMaterial( "Fullscreen" ) );
	g_renderer->DrawTexturedAABB( AABB2( 0.5f, -0.35f, 0.95f, 0.1f ), *m_reprojectionTargets[ m_currentReprojectionTargetIndex ], Vector2::ZERO, Vector2::ONE, Rgba::WHITE );
}

void TheGame::RenderSceneShadowsDebug() const
{
	m_debugOverlayCamera->SetViewport( AABB2( 0.0f, 0.0f, 1.0f, 1.0f ) );
	g_renderer->SetCamera( m_debugOverlayCamera );
	g_renderer->BindMaterial( *g_renderer->CreateOrGetMaterial( "Fullscreen" ) );
	g_renderer->DrawTexturedAABB( AABB2( -0.95f, -0.35f, -0.50f, 0.1f ), *m_volumetricShadowTarget, Vector2::ZERO, Vector2::ONE, Rgba::WHITE );
}

void TheGame::RenderSceneNormalsDebug() const
{
	m_debugOverlayCamera->SetViewport( AABB2( 0.0f, 0.0f, 1.0f, 1.0f ) );
	g_renderer->SetCamera( m_debugOverlayCamera );
	g_renderer->BindMaterial( *g_renderer->CreateOrGetMaterial( "Fullscreen" ) );
	g_renderer->DrawTexturedAABB( AABB2( -0.95f, 0.25f, -0.50f, 0.7f ), *m_sceneNormalTarget, Vector2::ZERO, Vector2::ONE, Rgba::WHITE );
}

void TheGame::RenderSceneSpecularDebug() const
{
	m_debugOverlayCamera->SetViewport( AABB2( 0.0f, 0.0f, 1.0f, 1.0f ) );
	g_renderer->SetCamera( m_debugOverlayCamera );
	g_renderer->BindMaterial( *g_renderer->CreateOrGetMaterial( "Fullscreen" ) );
	g_renderer->DrawTexturedAABB( AABB2( -0.45f, 0.25f, 0.0f, 0.7f ), *m_sceneSpecularTarget, Vector2::ZERO, Vector2::ONE, Rgba::WHITE );
}

void TheGame::RenderWeatherTextureDebug() const
{
	float actualHeight = 0.45f;
	float aspect = Window::GetAspect();
	float oneByAspect = 1.0f / aspect;
	float difference = actualHeight * ( 1.0f - oneByAspect );
	difference *= 0.5f;
	Material* materialRef = g_renderer->CreateOrGetMaterial( "2DWeatherTextureDebug" );
	switch ( m_shapeTexRenderChannel )	// No need for its own flag
	{
		case 0 : materialRef->SetProperty( "CHANNEL_MASK", Vector4( 1.0f, 0.0f, 0.0f, 0.0f ) ); break;
		case 1 : materialRef->SetProperty( "CHANNEL_MASK", Vector4( 0.0f, 1.0f, 0.0f, 0.0f ) ); break;
		case 2 : materialRef->SetProperty( "CHANNEL_MASK", Vector4( 0.0f, 0.0f, 1.0f, 0.0f ) ); break;
		case 3 : materialRef->SetProperty( "CHANNEL_MASK", Vector4( 1.0f, 1.0f, 1.0f, 0.0f ) ); break;
		default: break;
	}
	materialRef->SetProperty( "UV_OFFSET", m_weatherTexUVOffset );
	g_renderer->BindMaterial( *materialRef );
	g_renderer->DrawTexturedAABB(
		AABB2( ( 0.5f + difference ), ( -0.5f - actualHeight ), ( 0.5f + ( actualHeight * oneByAspect ) + difference ), -0.5f ),	// Getting aspect-correct AABB2 bounds
		*m_weatherTexture,
		Vector2::ZERO, Vector2::ONE,
		Rgba::WHITE
	);
}

void TheGame::RenderMieLUTDebug() const
{
	float actualHeight = 0.45f;
	float aspect = Window::GetAspect();
	float oneByAspect = 1.0f / aspect;
	float difference = actualHeight * ( 1.0f - oneByAspect );
	difference *= 0.5f;
	Material* materialRef = g_renderer->CreateOrGetMaterial( "2DWeatherTextureDebug" );
	switch ( m_shapeTexRenderChannel )	// No need for its own flag
	{
		case 0 : materialRef->SetProperty( "CHANNEL_MASK", Vector4( 1.0f, 0.0f, 0.0f, 0.0f ) ); break;
		case 1 : materialRef->SetProperty( "CHANNEL_MASK", Vector4( 0.0f, 1.0f, 0.0f, 0.0f ) ); break;
		case 2 : materialRef->SetProperty( "CHANNEL_MASK", Vector4( 0.0f, 0.0f, 1.0f, 0.0f ) ); break;
		case 3 : materialRef->SetProperty( "CHANNEL_MASK", Vector4( 1.0f, 1.0f, 1.0f, 0.0f ) ); break;
		default: break;
	}
	materialRef->SetProperty( "UV_OFFSET", m_weatherTexUVOffset );
	g_renderer->BindMaterial( *materialRef );
	g_renderer->DrawTexturedAABB(
		AABB2( ( -( actualHeight * oneByAspect ) - difference ), ( 0.1f - actualHeight ), -difference, 0.1f ),	// Getting aspect-correct AABB2 bounds
		*m_mieLUT,
		Vector2::ZERO, Vector2::ONE,
		Rgba::WHITE
	);
}

void TheGame::RenderMieGraph() const
{
	m_debugOverlayCamera->SetViewport( AABB2( 0.275f, 0.025f, 0.5f, 0.25f ) );
	g_renderer->SetCamera( m_debugOverlayCamera );
	g_renderer->DrawRenderable( *m_mieGraphGrid );
	g_renderer->DrawRenderable( *m_mieGraphR );
	g_renderer->DrawRenderable( *m_mieGraphG );
	g_renderer->DrawRenderable( *m_mieGraphB );
}

void TheGame::RenderShapeTextureLayerDebug() const
{
	float actualHeight = 0.225f;
	float aspect = Window::GetAspect();
	float oneByAspect = 1.0f / aspect;
	float difference = actualHeight * ( 1.0f - oneByAspect );
	difference *= 0.5f;

	m_debugOverlayCamera->SetViewport( AABB2( ( 0.75f - ( actualHeight * oneByAspect ) - difference ), ( 0.25f - actualHeight ), ( 0.75f - difference ), 0.25f ) );
	g_renderer->SetCamera( m_debugOverlayCamera );
	Material* materialRef = g_renderer->CreateOrGetMaterial( "3DTexLayer" );
	materialRef->SetProperty( "NUM_LAYERS", static_cast< float >( g_gameConfigBlackboard.GetValue( "shapeNoiseDimensions", IntVector3() ).z ) );
	materialRef->SetProperty( "LAYER_MASK", static_cast< float >( m_shapeTexRenderLayer ) );
	materialRef->SetProperty( "WORLEY_INVERSE", m_shapeWorleyInvertFactor );
	switch ( m_shapeTexRenderChannel )
	{
		case 0 : materialRef->SetProperty( "CHANNEL_MASK", Vector4( 1.0f, 0.0f, 0.0f, 0.0f ) ); break;
		case 1 : materialRef->SetProperty( "CHANNEL_MASK", Vector4( 0.0f, 1.0f, 0.0f, 0.0f ) ); break;
		case 2 : materialRef->SetProperty( "CHANNEL_MASK", Vector4( 0.0f, 0.0f, 1.0f, 0.0f ) ); break;
		case 3 : materialRef->SetProperty( "CHANNEL_MASK", Vector4( 0.0f, 0.0f, 0.0f, 1.0f ) ); break;
		case 4 : materialRef->SetProperty( "CHANNEL_MASK", Vector4( 1.0f, 1.0f, 1.0f, 1.0f ) ); break;
		default: break;
	}
	g_renderer->BindMaterial( *materialRef );
	g_renderer->BindTexture3DAndSampler( 8U, *m_shapeTexture, *g_renderer->GetDefaultSampler(true) );
	g_renderer->DrawFullScreenImmediate( *materialRef, Rgba::WHITE, 0, false );
}

void TheGame::RenderDetailTextureLayerDebug() const
{
	float actualHeight = 0.225f;
	float aspect = Window::GetAspect();
	float oneByAspect = 1.0f / aspect;
	float difference = actualHeight * ( 1.0f - oneByAspect );
	difference *= 0.5f;

	m_debugOverlayCamera->SetViewport( AABB2( ( 0.75f - ( actualHeight * oneByAspect ) - difference ), ( 0.55f - actualHeight ), ( 0.75f - difference ), 0.55f ) );
	g_renderer->SetCamera( m_debugOverlayCamera );
	Material* materialRef = g_renderer->CreateOrGetMaterial( "3DTexLayer" );
	materialRef->SetProperty( "NUM_LAYERS", static_cast< float >( g_gameConfigBlackboard.GetValue( "detailNoiseDimensions", IntVector3() ).z ) );
	materialRef->SetProperty( "LAYER_MASK", static_cast< float >( m_detailTexRenderLayer ) );
	materialRef->SetProperty( "WORLEY_INVERSE", m_detailWorleyInvertFactor );
	switch ( m_detailTexRenderChannel )
	{
		case 0 : materialRef->SetProperty( "CHANNEL_MASK", Vector4( 1.0f, 0.0f, 0.0f, 0.0f ) ); break;
		case 1 : materialRef->SetProperty( "CHANNEL_MASK", Vector4( 0.0f, 1.0f, 0.0f, 0.0f ) ); break;
		case 2 : materialRef->SetProperty( "CHANNEL_MASK", Vector4( 0.0f, 0.0f, 1.0f, 0.0f ) ); break;
		case 4 : materialRef->SetProperty( "CHANNEL_MASK", Vector4( 1.0f, 1.0f, 1.0f, 1.0f ) ); break;
		default: break;
	}
	g_renderer->BindMaterial( *materialRef );
	g_renderer->BindTexture3DAndSampler( 8U, *m_detailTexture, *g_renderer->GetDefaultSampler(true) );
	g_renderer->DrawFullScreenImmediate( *materialRef, Rgba::WHITE, 0, false );
}

#pragma endregion

#pragma region Terrain

Vector3 TerrainSurfacePatch( float u, float v ) // Represents the equation y = sin( sqrt( x^2 + z^2 ) ); arbitrary, just a tech demo
{
	Vector3 pointOnGraph = Vector3( u, 0.0f, v );
	pointOnGraph.y = Compute2dPerlinNoise( u, v, 100.f, 4U );
	pointOnGraph.y = RangeMapFloat( pointOnGraph.y, -1.f, 1.f, 0.f, 1.f );
	pointOnGraph.y *= 50.f;
	return pointOnGraph;
}

Vector3 TerrainSurfacePatchRadial( float u, float v )
{
	Vector2 polar = Vector2( u, v );
	polar.ConvertToCartestian();
	Vector3 pointOnGraph = Vector3( polar.x, 0.0f, polar.y );
	return pointOnGraph;
}

#pragma endregion

#pragma region Console Commands

bool SetAmbientLightCommand( Command& ambientLightCommand )
{
	if ( ambientLightCommand.GetName() == "set_ambient_light" )
	{
		std::string rgbaString = ambientLightCommand.GetNextString();
		if ( rgbaString == "" )
		{
			ConsolePrintf( Rgba::RED, "ERROR: set_ambient_light: No color string provided" );
			return false;
		}
		if ( !Rgba::IsValidString( rgbaString ) )
		{
			ConsolePrintf( Rgba::RED, "ERROR: set_ambient_light: Invalid color provided; must be in format \'R,G,B,A\', where \',A\' is optional, and 0 <= RGBA <= 255" );
			return false;
		}
		Rgba color;
		color.SetFromText( rgbaString );

		std::string intensityString = ambientLightCommand.GetNextString();
		float intensity = 1.0f;
		try
		{
			intensity = stof( intensityString );
		}
		catch ( std::invalid_argument& invalidArgument )
		{
			UNUSED( invalidArgument );
			ConsolePrintf( Rgba::RED, "ERROR: set_ambient_light: Invalid intensity provided; must be a positive float" );
			return false;
		}
		if ( intensity < 0.0f )
		{
			ConsolePrintf( Rgba::RED, "ERROR: set_ambient_light: Invalid intensity provided; must be a positive float" );
			return false;
		}

		g_renderer->SetAmbientLight( color, intensity );
		return true;
	}
	return false;
}

bool CameraGoToCommand( Command& command )
{
	if ( command.GetName() == "goto" )
	{
		std::string positionStr = command.GetNextString();
		if ( positionStr == "" )
		{
			ConsolePrintf( Rgba::RED, "ERROR: goto: Invalid position provided." );
			return false;
		}

		Vector3 position;
		try
		{
			position.SetFromText( positionStr );
		}
		catch ( std::invalid_argument& arg )
		{
			UNUSED( arg );
			ConsolePrintf( Rgba::RED, "ERROR: goto: Invalid position provided." );
			return false;
		}

		g_theGame->MoveCameraTo( position );
		return true;
	}
	return false;
}

bool CameraGoInDirectionCommand( Command& command )
{
	if ( command.GetName() == "goin" )
	{
		std::string directionStr = command.GetNextString();
		if ( directionStr == "" )
		{
			ConsolePrintf( Rgba::RED, "ERROR: goin: Invalid direction provided." );
			return false;
		}

		Vector3 direction;
		try
		{
			direction.SetFromText( directionStr );
		}
		catch ( std::invalid_argument& arg )
		{
			UNUSED( arg );
			ConsolePrintf( Rgba::RED, "ERROR: goin: Invalid direction provided." );
			return false;
		}

		g_theGame->MoveCamera( direction );
		return true;
	}
	return false;
}

bool CameraLookAtCommand( Command& command )
{
	if ( command.GetName() == "lookat" )
	{
		std::string positionStr = command.GetNextString();
		if ( positionStr == "" )
		{
			ConsolePrintf( Rgba::RED, "ERROR: lookat: Invalid position provided." );
			return false;
		}

		Vector3 position;
		try
		{
			position.SetFromText( positionStr );
		}
		catch ( std::invalid_argument& arg )
		{
			UNUSED( arg );
			ConsolePrintf( Rgba::RED, "ERROR: lookat: Invalid position provided." );
			return false;
		}

		g_theGame->CameraLookAt( position );
		return true;
	}
	return false;
}

#pragma endregion
