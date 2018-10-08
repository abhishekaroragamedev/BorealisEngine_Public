#include "Game/TheGame.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Projectile.hpp"
#include "Game/Ship.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Window.hpp"
#include "Engine/Networking/NetConnection.hpp"
#include "Engine/Networking/NetMessage.hpp"
#include "Engine/Networking/NetSession.hpp"
#include "Engine/Networking/UDPSocket.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Renderer/ForwardRenderingPath.hpp"
#include "Engine/Renderer/GLUtils.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/MeshBuilder.hpp"
#include "Engine/Renderer/ParticleEmitter.hpp"
#include "Engine/Renderer/Renderable.hpp"
#include "Engine/Renderer/RendererTypes.hpp"
#include "Engine/Renderer/Scene.hpp"
#include "Engine/Tools/Command.hpp"
#include "Engine/Tools/DevConsole.hpp"
#include "ThirdParty/tinyXML2/tinyxml2.h"

bool OnPing( NetMessage& message, NetConnection& connection )
{
	char* str = new char[ MESSAGE_MTU ];
	size_t sizeRead = message.ReadString( str, MESSAGE_MTU );
	str[ sizeRead ] = '\0';

	if ( sizeRead < 0 )
	{
		return false;
	}

	ConsolePrintf( "Received ping from %s: %s", connection.m_address.ToString().c_str(), str );

	uint8_t messageIndex = g_theGame->GetSession()->GetMessageIndex( "pong" );
	NetMessage pong( messageIndex );
	connection.Send( pong );

	delete[] str;

	return true;
}

bool OnPong( NetMessage& message, NetConnection& connection )
{
	UNUSED( message );
	ConsolePrintf( "Received pong from %s", connection.m_address.ToString().c_str() );
	return true;
}

bool OnAdd( NetMessage& message, NetConnection& connection )
{
	float val0;
	float val1;
	float sum;

	if ( !message.ReadFloat( &val0 ) || !message.ReadFloat( &val1 ) )
	{
		ConsolePrintf( "OnAdd(): Invalid add message received." );
		return false;
	}

	sum = val0 + val1;

	uint8_t messageIndex = g_theGame->GetSession()->GetMessageIndex( "add_response" );
	NetMessage msg( messageIndex );
	msg.WriteFloat( val0 );
	msg.WriteFloat( val1 );
	msg.WriteFloat( sum );
	connection.Send( msg );

	ConsolePrintf( "Received add from %s: %f + %f = %f", connection.m_address.ToString().c_str(), val0, val1, sum );
	return true;
}

bool OnAddResponse( NetMessage& message, NetConnection& connection )
{
	float val0;
	float val1;
	float sum;

	if ( !message.ReadFloat( &val0 ) || !message.ReadFloat( &val1 ) || !message.ReadFloat( &sum ) )
	{
		ConsolePrintf( "OnAddResponse(): Invalid add_response message received." );
		return false;
	}

	ConsolePrintf( "Received add response from %s: %f + %f = %f", connection.m_address.ToString().c_str(), val0, val1, sum );
	return true;
}

TheGame::TheGame()
{
	m_netSession = new NetSession();
	m_netSession->RegisterMessage( "ping", OnPing );
	m_netSession->RegisterMessage( "pong", OnPong );
	m_netSession->RegisterMessage( "add", OnAdd );
	m_netSession->RegisterMessage( "add_response", OnAddResponse );
	m_netSession->AddBinding( GAME_PORT );

	m_entityCount = 0;
	m_defaultFont = g_renderer->CreateOrGetBitmapFont( DEFAULT_FONT_NAME );
	
	m_gameCamera = new Camera( g_renderer->GetDefaultColorTarget(), g_renderer->GetDefaultDepthStencilTarget(), true );
	m_gameCamera->SetProjection( Matrix44::MakePerspective( 45.0f, Window::GetAspect(), 0.1f, 1000.0f ) );
	m_gameCamera->AddSkybox( TEXTURE_CUBE_FILEPATH );
	m_gameCamera->SetMaxShadowDepth( 10.0f );
	//m_gameCamera->SetBloomEnabled( true );

	m_uiCamera = new Camera( g_renderer->GetDefaultColorTarget() );
	m_uiCamera->SetProjectionOrtho( 2.0f, Window::GetAspect(), -1.0f, 1.0f );

	m_debugRenderMode = DebugRenderMode::DEBUG_RENDER_USE_DEPTH;
	DebugRenderSet3DCamera( m_gameCamera );

	CommandRegister( "set_ambient_light", SetAmbientLightCommand, "Sets the ambient light color and intensity associated with a specified shader, or with the active shader." );
	CommandRegister( "set_specular", SetSpecularPropertiesCommand, "Sets the specular properties associated with a specified shader, or with the active shader." );
	CommandRegister( "camera_light", CameraLightCommand, "Sets the type, color, attenuation and angles (if spot light) of the light currently attached to the camera." );
	CommandRegister( "detach_light", DetachCameraLightCommand, "Fixes the camera's current light in place and replaces it with a turned-off light (which can then be set by camera_light)." );

	CommandRegister( "add_connection", AddConnectionCommand, "Attach a session index to an IP address." );
	CommandRegister( "send_ping", SendPingCommand, "Send a ping to a client connected to the session by its index." );
	CommandRegister( "send_add", SendAddCommand, "Send an add command to a client connected to the session by its index." );

	m_ship = new Ship();
	m_gameCamera->GetTransform()->Reparent( m_ship->GetTransform() );
	m_gameCamera->Translate( m_ship->GetForward() * CAMERA_TRAIL_DISTANCE + m_ship->GetUp() * CAMERA_TRAIL_HEIGHT );

	InitializeRenderableMeshes();
	InitializeRenderableMaterials();

	m_gameScene = new Scene();
	InitializeLights();
	m_gameScene->AddCamera( m_gameCamera );
	m_gameScene->AddRenderable( m_plane );
	m_gameScene->AddRenderable( m_cube );
	m_gameScene->AddRenderable( m_sphere );
	m_gameScene->AddRenderable( m_ship->GetRenderable() );
	m_gameScene->AddRenderable( m_ship->GetLeftThruster()->GetRenderable() );
	m_gameScene->AddRenderable( m_ship->GetRightThruster()->GetRenderable() );
	m_gameScene->AddRenderable( m_snowMiku[ 0 ] );
	m_gameScene->AddRenderable( m_snowMiku[ 1 ] );
	m_gameScene->AddRenderable( m_snowMiku[ 2 ] );
	m_gameScene->AddRenderable( m_snowMiku[ 3 ] );
	m_gameScene->AddRenderable( m_halloweenMiku[ 0 ] );
	m_gameScene->AddRenderable( m_halloweenMiku[ 1 ] );
	m_gameScene->AddRenderable( m_halloweenMiku[ 2 ] );
	m_gameScene->AddRenderable( m_luka[ 0 ] );
	m_gameScene->AddRenderable( m_luka[ 1 ] );
	m_gameScene->AddRenderable( m_luka[ 2 ] );
	m_gameScene->AddRenderable( m_surfacePatchDemo );
}

TheGame::~TheGame()
{
	delete m_netSession;

	DeleteRenderables();

	for ( Light* light : m_lights )
	{
		delete light;
		light = nullptr;
	}

	delete m_uiCamera;
	delete m_gameCamera;

	delete m_gameScene;
	m_gameScene = nullptr;

	CommandUnregister( "add_connection" );
	CommandUnregister( "send_ping" );
	CommandUnregister( "send_add" );

	CommandUnregister( "set_ambient_light" );
	CommandUnregister( "set_specular" );
	CommandUnregister( "camera_light" );
	CommandUnregister( "detach_light" );
}

void TheGame::InitializeLights()
{
	Light* sun = new Light();
	Vector3 position = Vector3::ZERO;
	sun->SetDirectionalLight( position, Vector3( -SUN_DISTANCE, -SUN_DISTANCE, -SUN_DISTANCE ).GetNormalized(), Vector4( 1.0f, 1.0, 1.0f, 1.0f ), Vector3::ZERO );
	sun->m_lightUBO.m_castsShadows = 1;
	m_gameScene->AddLight( sun );
	m_lights.push_back( sun );
	m_lightSpheres.push_back( new Renderable( g_renderer->MakeUVSphereMesh( position, 1000.0f, 4, 4, Rgba( Vector4( 1.0f, 1.0f, 1.0f, 0.4f ) ) ).CreateMesh(), g_renderer->CreateOrGetMaterial( "Sphere" )->GetInstance() ) );
	m_gameScene->AddRenderable( m_lightSpheres[ m_lightSpheres.size() - 1 ] );
	m_gameScene->SetShadowViewingCamera( m_gameCamera );
	m_gameScene->SetShadowCastingLight( sun );

	SwitchCameraLight();

	g_renderer->SetShaderPass( "DefaultLit" );
	g_renderer->SetAmbientLight( Rgba::RED, 0.02f );
}

void TheGame::InitializeExtraLights()
{
	Light* sceneLight = new Light();
	Vector3 position = Vector3( -3.0f, 0.0f, 0.0f );
	sceneLight->SetSpotLight( position, -1.0f * position, Vector4( 0.0f, 0.0f, 1.0f, 25.0f ), GetRandomFloatInRange( 10.0f, 30.0f ), GetRandomFloatInRange( 40.0f, 65.0f ), Vector3( 0.0f, 0.5f, 0.05f ) );
	m_lights.push_back( sceneLight );
	m_extraLights.push_back( sceneLight );
	m_lightSpheres.push_back( new Renderable( g_renderer->MakeUVSphereMesh( position, 0.05f, 4, 4, Rgba( Vector4( 0.0f, 0.0f, 1.0f, 25.0f ) ) ).CreateMesh(), g_renderer->CreateOrGetMaterial( "Sphere" )->GetInstance() ) );
	m_gameScene->AddRenderable( m_lightSpheres[ m_lightSpheres.size() - 1 ] );

	sceneLight = new Light();
	position = Vector3( 3.0f, 0.0f, 0.0f );
	sceneLight->SetSpotLight( position, -1.0f * position, Vector4( 0.0f, 1.0f, 0.0f, 25.0f ), GetRandomFloatInRange( 10.0f, 30.0f ), GetRandomFloatInRange( 40.0f, 65.0f ), Vector3( 0.0f, 0.5f, 0.05f ) );
	m_lights.push_back( sceneLight );
	m_extraLights.push_back( sceneLight );
	m_lightSpheres.push_back( new Renderable( g_renderer->MakeUVSphereMesh( position, 0.05f, 4, 4, Rgba( Vector4( 0.0f, 1.0f, 0.0f, 25.0f ) ) ).CreateMesh(), g_renderer->CreateOrGetMaterial( "Sphere" )->GetInstance() ) );
	m_gameScene->AddRenderable( m_lightSpheres[ m_lightSpheres.size() - 1 ] );

	sceneLight = new Light();
	position = Vector3( 0.0f, 1.0f, -3.0f );
	sceneLight->SetSpotLight( position, -1.0f * position, Vector4( 1.0f, 0.0f, 0.0f, 25.0f ), GetRandomFloatInRange( 10.0f, 30.0f ), GetRandomFloatInRange( 40.0f, 65.0f ), Vector3( 0.0f, 0.5f, 0.05f ) );
	m_lights.push_back( sceneLight );
	m_extraLights.push_back( sceneLight );
	m_lightSpheres.push_back( new Renderable( g_renderer->MakeUVSphereMesh( position, 0.05f, 4, 4, Rgba( Vector4( 1.0f, 0.0f, 0.0f, 25.0f ) ) ).CreateMesh(), g_renderer->CreateOrGetMaterial( "Sphere" )->GetInstance() ) );
	m_gameScene->AddRenderable( m_lightSpheres[ m_lightSpheres.size() - 1 ] );

	sceneLight = new Light();
	position = Vector3( -3.0f, -3.0f, -3.0f );
	sceneLight->SetDirectionalLight( position, Vector3( 3.0f, 0.0f, 3.0f ), Vector4( 1.0f, 0.0f, 1.0f, 30.0f ), Vector3( 0.0f, 0.5f, 0.05f ) );
	m_lights.push_back( sceneLight );
	m_extraLights.push_back( sceneLight );
	m_lightSpheres.push_back( new Renderable( g_renderer->MakeUVSphereMesh( position, 0.05f, 4, 4, Rgba( Vector4( 1.0f, 0.0f, 1.0f, 30.0f ) ) ).CreateMesh(), g_renderer->CreateOrGetMaterial( "Sphere" )->GetInstance() ) );
	m_gameScene->AddRenderable( m_lightSpheres[ m_lightSpheres.size() - 1 ] );

	sceneLight = new Light();
	position = Vector3( 3.0f, -3.0f, -3.0f );
	sceneLight->SetDirectionalLight( position, Vector3( -3.0f, 0.0f, 3.0f ), Vector4( 0.0f, 1.0f, 0.0f, 30.0f ), Vector3( 0.0f, 0.5f, 0.05f ) );
	m_lights.push_back( sceneLight );
	m_extraLights.push_back( sceneLight );
	m_lightSpheres.push_back( new Renderable( g_renderer->MakeUVSphereMesh( position, 0.05f, 4, 4, Rgba( Vector4( 0.0f, 1.0f, 0.0f, 30.0f ) ) ).CreateMesh(), g_renderer->CreateOrGetMaterial( "Sphere" )->GetInstance() ) );
	m_gameScene->AddRenderable( m_lightSpheres[ m_lightSpheres.size() - 1 ] );

	sceneLight = new Light();
	position = Vector3( 0.0f, 6.5f, 0.0f );
	sceneLight->SetPointLight( position, Vector4( 1.0f, 0.0f, 0.0f, 25.0f ), Vector3( 0.0f, 0.5f, 0.05f ) );
	m_lights.push_back( sceneLight );
	m_extraLights.push_back( sceneLight );
	m_lightSpheres.push_back( new Renderable( g_renderer->MakeUVSphereMesh( position, 0.05f, 4, 4, Rgba( Vector4( 1.0f, 0.0f, 0.0f, 25.0f ) ) ).CreateMesh(), g_renderer->CreateOrGetMaterial( "Sphere" )->GetInstance() ) );
	m_gameScene->AddRenderable( m_lightSpheres[ m_lightSpheres.size() - 1 ] );

	AddExtraLightsToScene();
}

void TheGame::AddExtraLightsToScene()
{
	for ( Light* extraLight : m_extraLights )
	{
		m_gameScene->AddLight( extraLight );
	}
	m_areExtraLightsInScene = true;
}

void TheGame::RemoveExtraLightsFromScene()
{
	for ( Light* extraLight : m_extraLights )
	{
		m_gameScene->RemoveLight( extraLight );
	}
	m_areExtraLightsInScene = false;
}

NetSession* TheGame::GetSession() const
{
	return m_netSession;
}

void TheGame::InitializeRenderableMeshes()
{
	InitializeShapeMeshes();
	InitializeMikuMeshes();
	GenerateSurfacePatchDemoMesh();
}

void TheGame::InitializeShapeMeshes()
{
	m_plane = new Renderable();
	m_plane->SetMesh( g_renderer->MakePlaneMesh( Vector3( -2.5f, -4.5f, 0.0f ), Vector3::UP, Vector3::RIGHT, Vector2::ONE, Rgba::WHITE, AABB2::ZERO_TO_ONE ).CreateMesh() );

	m_cube = new Renderable();
	m_cube->SetMesh( g_renderer->MakeTexturedCubeMesh( Vector3( 0.0f, -4.0f, 0.0f ), Vector3::ONE, *g_renderer->CreateOrGetTexture( DEFAULT_TEXTURE_FILEPATH ), Rgba::WHITE ).CreateMesh() );
	
	m_sphere = new Renderable();
	m_sphere->SetMesh( g_renderer->MakeUVSphereMesh( Vector3( 2.0f, -4.0f, 0.0f ), 0.5f, 25, 25, Rgba::WHITE ).CreateMesh() );
}

void TheGame::InitializeMikuMeshes()
{
	std::vector< Mesh* > snowMikuMeshes = MeshBuilder::FromFileOBJ( SNOW_MIKU_MODEL_FILEPATH );
	Matrix44 snowMikuPosition = Matrix44::MakeTranslation( Vector3( 0.0f, 6.0f, 0.0f ) );

	m_snowMiku[ 0 ] = new Renderable();
	m_snowMiku[ 0 ]->SetMesh( snowMikuMeshes[ 0 ] );
	m_snowMiku[ 0 ]->SetModelMatrix( snowMikuPosition );
	m_snowMiku[ 1 ] = new Renderable();
	m_snowMiku[ 1 ]->SetMesh( snowMikuMeshes[ 1 ] );
	m_snowMiku[ 1 ]->SetModelMatrix( snowMikuPosition );
	m_snowMiku[ 2 ] = new Renderable();
	m_snowMiku[ 2 ]->SetMesh( snowMikuMeshes[ 2 ] );
	m_snowMiku[ 2 ]->SetModelMatrix( snowMikuPosition );
	m_snowMiku[ 3 ] = new Renderable();
	m_snowMiku[ 3 ]->SetMesh( snowMikuMeshes[ 3 ] );
	m_snowMiku[ 3 ]->SetModelMatrix( snowMikuPosition );

	std::vector< Mesh* > halloweenMikuMeshes = MeshBuilder::FromFileOBJ( HALLOWEEN_MIKU_MODEL_FILEPATH );
	Matrix44 halloweenMikuPosition = Matrix44::MakeTranslation( Vector3( 4.0f, 6.0f, 0.0f ) );

	m_halloweenMiku[ 0 ] = new Renderable();
	m_halloweenMiku[ 0 ]->SetMesh( halloweenMikuMeshes[ 0 ] );
	m_halloweenMiku[ 0 ]->SetModelMatrix( halloweenMikuPosition );
	m_halloweenMiku[ 1 ] = new Renderable();
	m_halloweenMiku[ 1 ]->SetMesh( halloweenMikuMeshes[ 1 ] );
	m_halloweenMiku[ 1 ]->SetModelMatrix( halloweenMikuPosition );
	m_halloweenMiku[ 2 ] = new Renderable();
	m_halloweenMiku[ 2 ]->SetMesh( halloweenMikuMeshes[ 2 ] );
	m_halloweenMiku[ 2 ]->SetModelMatrix( halloweenMikuPosition );

	std::vector< Mesh* > lukaMeshes = MeshBuilder::FromFileOBJ( LUKA_MODEL_FILEPATH );
	Matrix44 lukaPosition = Matrix44::MakeTranslation( Vector3( -4.0f, 6.0f, 0.0f ) );

	m_luka[ 0 ] = new Renderable();
	m_luka[ 0 ]->SetMesh( lukaMeshes[ 0 ] );
	m_luka[ 0 ]->SetModelMatrix( lukaPosition );
	m_luka[ 1 ] = new Renderable();
	m_luka[ 1 ]->SetMesh( lukaMeshes[ 1 ] );
	m_luka[ 1 ]->SetModelMatrix( lukaPosition );
	m_luka[ 2 ] = new Renderable();
	m_luka[ 2 ]->SetMesh( lukaMeshes[ 2 ] );
	m_luka[ 2 ]->SetModelMatrix( lukaPosition );
}

void TheGame::GenerateSurfacePatchDemoMesh()
{
	MeshBuilder mb;
	mb.Begin( DrawPrimitiveType::TRIANGLES );

	mb.AddSurfacePatch( GraphSurfacePatch, FloatRange( -40.0f, 40.0f ), FloatRange( -40.0f, 40.0f ), IntVector2( 50, 50 ) );

	mb.End();

	m_surfacePatchDemo = new Renderable();
	m_surfacePatchDemo->SetModelMatrix( Matrix44::MakeTranslation( Vector3( 0.0f, -7.0f, 0.0f ) ) );
	m_surfacePatchDemo->SetMesh( mb.CreateMesh() );
}

void TheGame::InitializeRenderableMaterials()
{
	InitializeShapeMaterials();
	InitializeShipMaterial();
	InitializeMikuMaterials();
	InitializeSurfacePatchDemoMaterial();

	m_renderModeChanged = false;
	m_materialsInitialized = true;
}

void TheGame::InitializeShapeMaterials()
{
	Material* planeMaterial = GetMaterialForRenderMode( m_renderModeIndex )->GetInstance();
	planeMaterial->SetTextureAndSampler( 0, g_renderer->CreateOrGetMaterial( "Plane" )->GetTexture( 0 ), g_renderer->GetDefaultSampler()  );
	planeMaterial->SetTextureAndSampler( 1, g_renderer->CreateOrGetMaterial( "Plane" )->GetTexture( 1 ), g_renderer->GetDefaultSampler()  );

	Material* cubeMaterial = GetMaterialForRenderMode( m_renderModeIndex )->GetInstance();
	cubeMaterial->SetTextureAndSampler( 0, g_renderer->CreateOrGetMaterial( "Cube" )->GetTexture( 0 ), g_renderer->GetDefaultSampler()  );
	cubeMaterial->SetTextureAndSampler( 1, g_renderer->CreateOrGetMaterial( "Cube" )->GetTexture( 1 ), g_renderer->GetDefaultSampler()  );

	Material* sphereMaterial = GetMaterialForRenderMode( m_renderModeIndex )->GetInstance();
	sphereMaterial->SetTextureAndSampler( 0, g_renderer->CreateOrGetMaterial( "Sphere" )->GetTexture( 0 ), g_renderer->GetDefaultSampler()  );
	sphereMaterial->SetTextureAndSampler( 1, g_renderer->CreateOrGetMaterial( "Sphere" )->GetTexture( 1 ), g_renderer->GetDefaultSampler()  );

	if ( !m_materialsInitialized )
	{
		m_shapeSpecularProperties = Vector2( g_renderer->CreateOrGetMaterial( "Plane" )->GetSpecularAmount(), g_renderer->CreateOrGetMaterial( "Plane" )->GetSpecularPower() );
	}
	planeMaterial->SetSpecularProperties( m_shapeSpecularProperties.x, m_shapeSpecularProperties.y );
	cubeMaterial->SetSpecularProperties( m_shapeSpecularProperties.x, m_shapeSpecularProperties.y );
	sphereMaterial->SetSpecularProperties( m_shapeSpecularProperties.x, m_shapeSpecularProperties.y );

	m_plane->ReplaceMaterial( planeMaterial );
	m_cube->ReplaceMaterial( cubeMaterial );
	m_sphere->ReplaceMaterial( sphereMaterial );
}

void TheGame::InitializeShipMaterial()
{
	m_ship->InitializeMaterial( m_materialsInitialized, m_renderModeIndex );
}

void TheGame::InitializeMikuMaterials()
{
	Material* snowMikuMaterial0 = g_renderer->CreateOrGetMaterial( "SnowMiku0" )->GetInstance();
	Material* snowMikuMaterial1 = g_renderer->CreateOrGetMaterial( "SnowMiku1" )->GetInstance();
	Material* snowMikuMaterial2 = g_renderer->CreateOrGetMaterial( "SnowMiku2" )->GetInstance();

	Material* halloweenMikuMaterial0 = g_renderer->CreateOrGetMaterial( "HalloweenMiku0" )->GetInstance();
	Material* halloweenMikuMaterial1 = g_renderer->CreateOrGetMaterial( "HalloweenMiku1" )->GetInstance();
	Material* halloweenMikuMaterial2 = g_renderer->CreateOrGetMaterial( "HalloweenMiku2" )->GetInstance();

	Material* lukaMaterial0 = g_renderer->CreateOrGetMaterial( "Luka0" )->GetInstance();
	Material* lukaMaterial1 = g_renderer->CreateOrGetMaterial( "Luka1" )->GetInstance();
	Material* lukaMaterial2 = g_renderer->CreateOrGetMaterial( "Luka2" )->GetInstance();

	if ( !m_materialsInitialized )
	{
		m_mikuSpecularProperties = Vector2( g_renderer->CreateOrGetMaterial( "SnowMiku0" )->GetSpecularAmount(), g_renderer->CreateOrGetMaterial( "SnowMiku0" )->GetSpecularPower() );
	}
	snowMikuMaterial0->SetSpecularProperties( m_mikuSpecularProperties.x, m_mikuSpecularProperties.y );
	snowMikuMaterial1->SetSpecularProperties( m_mikuSpecularProperties.x, m_mikuSpecularProperties.y );
	snowMikuMaterial2->SetSpecularProperties( m_mikuSpecularProperties.x, m_mikuSpecularProperties.y );
	halloweenMikuMaterial0->SetSpecularProperties( m_mikuSpecularProperties.x, m_mikuSpecularProperties.y );
	halloweenMikuMaterial1->SetSpecularProperties( m_mikuSpecularProperties.x, m_mikuSpecularProperties.y );
	halloweenMikuMaterial2->SetSpecularProperties( m_mikuSpecularProperties.x, m_mikuSpecularProperties.y );
	lukaMaterial0->SetSpecularProperties( m_mikuSpecularProperties.x, m_mikuSpecularProperties.y );
	lukaMaterial1->SetSpecularProperties( m_mikuSpecularProperties.x, m_mikuSpecularProperties.y );
	lukaMaterial2->SetSpecularProperties( m_mikuSpecularProperties.x, m_mikuSpecularProperties.y );

	m_snowMiku[ 0 ]->ReplaceMaterial( snowMikuMaterial0 );
	m_snowMiku[ 1 ]->ReplaceMaterial( snowMikuMaterial1 );
	m_snowMiku[ 2 ]->ReplaceMaterial( snowMikuMaterial2 );
	m_snowMiku[ 3 ]->ReplaceMaterial( snowMikuMaterial1, false );
	m_halloweenMiku[ 0 ]->ReplaceMaterial( halloweenMikuMaterial0 );
	m_halloweenMiku[ 1 ]->ReplaceMaterial( halloweenMikuMaterial1 );
	m_halloweenMiku[ 2 ]->ReplaceMaterial( halloweenMikuMaterial2 );
	m_luka[ 0 ]->ReplaceMaterial( lukaMaterial0 );
	m_luka[ 1 ]->ReplaceMaterial( lukaMaterial1 );
	m_luka[ 2 ]->ReplaceMaterial( lukaMaterial2 );
}

void TheGame::InitializeSurfacePatchDemoMaterial()
{
	Material* surfacePatchDemoMaterial = GetMaterialForRenderMode( m_renderModeIndex )->GetInstance();
	surfacePatchDemoMaterial->SetTextureAndSampler( 0, g_renderer->CreateOrGetMaterial( "SurfacePatchDemo" )->GetTexture( 0 ), g_renderer->GetDefaultSampler( true ) );
	surfacePatchDemoMaterial->SetTextureAndSampler( 1, g_renderer->CreateOrGetMaterial( "SurfacePatchDemo" )->GetTexture( 1 ), g_renderer->GetDefaultSampler( true ) );
	surfacePatchDemoMaterial->SetSpecularProperties( m_surfacePatchSpecularProperties.x, m_surfacePatchSpecularProperties.y );
	m_surfacePatchDemo->ReplaceMaterial( surfacePatchDemoMaterial );
}

void TheGame::DeleteRenderables()
{
	delete m_plane;
	delete m_cube;
	delete m_sphere;
	delete m_ship;
	for ( Projectile* projectile : m_projectiles )
	{
		delete projectile;
	}

	delete m_surfacePatchDemo;
	delete m_snowMiku[ 0 ];
	delete m_snowMiku[ 1 ];
	delete m_snowMiku[ 2 ];
	delete m_snowMiku[ 3 ];
	delete m_halloweenMiku[ 0 ];
	delete m_halloweenMiku[ 1 ];
	delete m_halloweenMiku[ 2 ];
	delete m_luka[ 0 ];
	delete m_luka[ 1 ];
	delete m_luka[ 2 ];

	for ( Renderable* lightSphere : m_lightSpheres )
	{
		delete lightSphere;
		lightSphere = nullptr;
	}
}

void TheGame::FireProjectile( const Vector3& spawnPosition, const Vector3& spawnVelocity )
{
	Projectile* newProjectile = new Projectile( spawnPosition, spawnVelocity );
	m_projectiles.push_back( newProjectile );
	m_gameScene->AddRenderable( newProjectile->GetRenderable() );
	m_gameScene->AddRenderable( newProjectile->GetParticleEmitter()->GetRenderable() );
}

void TheGame::DestroyDeadProjectiles()
{
	for ( size_t projectileIndex = 0; projectileIndex < m_projectiles.size(); projectileIndex++ )
	{
		if ( m_projectiles[ projectileIndex ]->IsDead() )
		{
			m_gameScene->RemoveRenderable( m_projectiles[ projectileIndex ]->GetRenderable() );
			m_gameScene->RemoveRenderable( m_projectiles[ projectileIndex ]->GetParticleEmitter()->GetRenderable() );
			delete m_projectiles[ projectileIndex ];
			m_projectiles.erase( m_projectiles.begin() + projectileIndex );
			projectileIndex--;
		}
	}
}

void TheGame::AddLight( Light* light )
{
	m_lights.push_back( light );
	m_gameScene->AddLight( light );
}

void TheGame::RemoveLight( Light* light )
{
	m_gameScene->RemoveLight( light );
	m_lights.erase( std::find( m_lights.begin(), m_lights.end(), light ) );
}

void TheGame::Update()
{
	m_netSession->ProcessIncoming();

	if ( !IsDevConsoleOpen() )
	{
		HandleKeyboardInput();
		HandleXboxControllerInputs();

		m_ship->Update();
		m_ship->UpdateParticleSystems( m_gameCamera );
		for ( Projectile* projectile : m_projectiles )
		{
			projectile->Update();
			projectile->UpdateParticleSystems( m_gameCamera );
		}

		Matrix44 lookAt = Matrix44::LookAt( m_gameCamera->GetPosition(), ( m_gameCamera->GetPosition() + m_gameCamera->GetForward() ), m_gameCamera->GetUp() );
		m_lights[ m_cameraLightIndex ]->m_transform.SetLocalFromMatrix( lookAt );
		m_lights[ m_cameraLightIndex ]->m_transform.m_position = m_gameCamera->GetPosition();

		if ( m_renderModeChanged )
		{
			InitializeRenderableMaterials();
		}

		UpdateMaterials();

		DestroyDeadProjectiles();
	}

	m_netSession->ProcessOutgoing();
}

void TheGame::UpdateMaterials()
{

}

void TheGame::Render() const
{
	SetShaderForRenderMode( m_renderModeIndex );
	ForwardRenderingPath::RenderScene( m_gameScene );
	g_renderer->UseShaderProgram( DEFAULT_SHADER_NAME );
	RenderNetSessionOverlay();
}

void TheGame::RenderNetSessionOverlay() const
{
	g_renderer->SetCamera( m_uiCamera );
	g_renderer->BindMaterial( *g_renderer->CreateOrGetMaterial( "UI" ) );
	g_renderer->DrawText2D(
		Vector2( ( -1.0f * Window::GetAspect() ), 0.95f ),
		"Session info:",
		0.025f,
		Rgba::WHITE,
		0.85f,
		g_renderer->CreateOrGetBitmapFont( DEFAULT_FONT_NAME )
	);

	NetAddressIPv4 boundAddr = m_netSession->GetSocket()->m_netAddress;
	g_renderer->DrawText2D(
		Vector2( ( -1.0f * Window::GetAspect() ), 0.925f ),
		Stringf( "\tBound address: %s", boundAddr.ToString().c_str() ),
		0.025f,
		Rgba::WHITE,
		0.85f,
		g_renderer->CreateOrGetBitmapFont( DEFAULT_FONT_NAME )
	);

	g_renderer->DrawText2D(
		Vector2( ( -1.0f * Window::GetAspect() ), 0.9f ),
		"\tConnections:",
		0.025f,
		Rgba::WHITE,
		0.85f,
		g_renderer->CreateOrGetBitmapFont( DEFAULT_FONT_NAME )
	);

	float drawY = 0.875f;
	unsigned int numConnections = m_netSession->GetNumConnections();
	for ( unsigned int index = 0U; index < numConnections; index++ )
	{
		NetConnection* connection = m_netSession->GetConnection( index );
		std::string text = Stringf( "\t[%u] - ", index );
		if ( connection != nullptr )
		{
			NetAddressIPv4 connectionAddr = connection->m_address;
			text += connectionAddr.ToString();
		}
		g_renderer->DrawText2D(
			Vector2( ( -1.0f * Window::GetAspect() ), drawY ),
			text,
			0.025f,
			Rgba::WHITE,
			0.85f,
			g_renderer->CreateOrGetBitmapFont( DEFAULT_FONT_NAME )
		);
		drawY -= 0.025f;
	}
}

void TheGame::SetCameraLight( LightType type, const Vector4& color, const Vector3& attenuation, const Vector2& angles /* = Vector2::ZERO */ )
{
	switch ( type )
	{
		case LightType::LIGHT_TYPE_POINT		:	m_lights[ m_cameraLightIndex ]->SetPointLight( ( m_ship->GetPosition() + ( m_ship->GetForward() * SHIP_LIGHT_LEAD_DISTANCE ) + ( CAMERA_TRAIL_HEIGHT * m_ship->GetUp() ) ), color, attenuation ); break;
		case LightType::LIGHT_TYPE_DIRECTIONAL	:	m_lights[ m_cameraLightIndex ]->SetDirectionalLight( ( m_ship->GetPosition() + ( m_ship->GetForward() * SHIP_LIGHT_LEAD_DISTANCE ) + ( CAMERA_TRAIL_HEIGHT * m_ship->GetUp() ) ), m_ship->GetForward(), color, attenuation ); break;
		case LightType::LIGHT_TYPE_SPOT			:	m_lights[ m_cameraLightIndex ]->SetSpotLight( ( m_ship->GetPosition() + ( m_ship->GetForward() * SHIP_LIGHT_LEAD_DISTANCE ) + ( CAMERA_TRAIL_HEIGHT * m_ship->GetUp() ) ), m_ship->GetForward(), color, angles.x, angles.y, attenuation ); break;
	}
}

void TheGame::SetShapeSpecularProperties( float specularAmount, float specularPower )
{
	m_shapeSpecularProperties = Vector2( specularAmount, specularPower );
	m_plane->GetMaterial()->SetSpecularProperties( m_shapeSpecularProperties.x, m_shapeSpecularProperties.y );
}

void TheGame::SetShipSpecularProperties( float specularAmount, float specularPower )
{
	m_ship->SetSpecularProperties( specularAmount, specularPower );
}

void TheGame::SetMikuSpecularProperties( float specularAmount, float specularPower )
{
	m_mikuSpecularProperties = Vector2( specularAmount, specularPower );
	m_snowMiku[ 0 ]->GetMaterial()->SetSpecularProperties( m_mikuSpecularProperties.x, m_mikuSpecularProperties.y );
	m_snowMiku[ 1 ]->GetMaterial()->SetSpecularProperties( m_mikuSpecularProperties.x, m_mikuSpecularProperties.y );
	m_snowMiku[ 2 ]->GetMaterial()->SetSpecularProperties( m_mikuSpecularProperties.x, m_mikuSpecularProperties.y );
	m_snowMiku[ 3 ]->GetMaterial()->SetSpecularProperties( m_mikuSpecularProperties.x, m_mikuSpecularProperties.y );
	m_halloweenMiku[ 0 ]->GetMaterial()->SetSpecularProperties( m_mikuSpecularProperties.x, m_mikuSpecularProperties.y );
	m_halloweenMiku[ 1 ]->GetMaterial()->SetSpecularProperties( m_mikuSpecularProperties.x, m_mikuSpecularProperties.y );
	m_halloweenMiku[ 2 ]->GetMaterial()->SetSpecularProperties( m_mikuSpecularProperties.x, m_mikuSpecularProperties.y );
	m_luka[ 0 ]->GetMaterial()->SetSpecularProperties( m_mikuSpecularProperties.x, m_mikuSpecularProperties.y );
	m_luka[ 1 ]->GetMaterial()->SetSpecularProperties( m_mikuSpecularProperties.x, m_mikuSpecularProperties.y );
	m_luka[ 2 ]->GetMaterial()->SetSpecularProperties( m_mikuSpecularProperties.x, m_mikuSpecularProperties.y );
}

void TheGame::SetSurfacePatchSpecularProperties( float specularAmount, float specularPower )
{
	m_surfacePatchSpecularProperties = Vector2( specularAmount, specularPower );
	m_surfacePatchDemo->GetMaterial()->SetSpecularProperties( m_surfacePatchSpecularProperties.x, m_surfacePatchSpecularProperties.y );
}

void TheGame::SwitchCameraLight()
{
	if ( m_cameraLightIndex >= 0 )
	{
		m_lights[ m_cameraLightIndex ]->m_lightUBO.m_castsShadows = 0;
		m_lightSpheres.push_back( new Renderable( g_renderer->MakeUVSphereMesh( m_gameCamera->GetPosition(), 0.05f, 4, 4, Rgba::WHITE ).CreateMesh(), g_renderer->CreateOrGetMaterial( "Sphere" )->GetInstance() ) );
		m_gameScene->AddRenderable( m_lightSpheres[ m_lightSpheres.size() - 1 ] );
		m_gameScene->SetShadowCastingLight( m_lights[ m_cameraLightIndex ] );
	}

	Light* newLight = new Light();
	newLight->SetSpotLight( m_gameCamera->GetPosition(), m_gameCamera->GetForward(), Vector4::ONE_DISPLACEMENT, 25.0f, 40.0f, Vector3( 0.0f, 0.1f, 0.0f ) );
	m_lights.push_back( newLight );

	m_cameraLightIndex = static_cast< int >( m_lights.size() - 1 );
	m_gameScene->AddLight( newLight );
}

int TheGame::GetRenderModeIndex() const
{
	return m_renderModeIndex;
}

bool TheGame::HaveMaterialsInitialized() const
{
	return m_materialsInitialized;
}

void TheGame::HandleKeyboardInput()
{
	if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_F9 ) )
	{
		InitializeRenderableMaterials();
	}
	HandleKeyboardInputForCamera();
	if ( IsDebugRenderingEnabled() )
	{
		HandleKeyboardInputForDebugRendering();
		HandleKeyboardInputToCycleMeshRenderMode();
		HandleKeyboardInputToSwitchDebugRenderMode();
	}
}

void TheGame::HandleKeyboardInputForCamera()
{
	if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_6 ) )
	{
		( m_gameCamera->IsMSAAEnabled() )? m_gameCamera->DisableMSAA() : m_gameCamera->EnableMSAA( 4U );
	}
	if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_7 ) )
	{
		( m_areExtraLightsInScene )? RemoveExtraLightsFromScene() : AddExtraLightsToScene();
	}
	if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_8 ) )
	{
		m_gameCamera->SetBloomEnabled( !m_gameCamera->IsBloomEnabled() );
	}
}

void TheGame::HandleKeyboardInputForDebugRendering()
{
	if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_P ) )
	{
		DebugRenderPoint( 10.0f, m_gameCamera->GetPosition(), Rgba::RED, Rgba::GREEN, m_debugRenderMode );
	}
	if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_U ) )
	{
		DebugRenderSphere( 10.0f, m_gameCamera->GetPosition(), 0.5f, 17, 17, Rgba::RED, Rgba::GREEN, m_debugRenderMode );
	}
	if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_J ) )
	{
		DebugRenderWireSphere( 10.0f, m_gameCamera->GetPosition(), 0.5f, 17, 17, Rgba::RED, Rgba::GREEN, m_debugRenderMode );
	}
	if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_I ) )
	{
		DebugRenderQuad( 10.0f, m_gameCamera->GetPosition(), m_gameCamera->GetRight(), -1.0f, 1.0f, m_gameCamera->GetUp(), -1.0f, 1.0f, g_renderer->CreateOrGetTexture( DEFAULT_TEXTURE_FILEPATH ), AABB2::ZERO_TO_ONE, Rgba::RED, Rgba::GREEN, m_debugRenderMode );
	}
	if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_V ) )
	{
		DebugRenderGlyph( 10.0f, m_gameCamera->GetPosition(), -1.0f, 1.0f, -1.0f, 1.0f, g_renderer->CreateOrGetTexture( DEFAULT_TEXTURE_FILEPATH ), AABB2::ZERO_TO_ONE, Rgba::RED, Rgba::GREEN, m_debugRenderMode );
	}
	if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_O ) )
	{
		DebugRenderWireAABB3( 10.0f, AABB3( m_gameCamera->GetPosition(), 0.5f, 0.5f, 0.5f ), Rgba::RED, Rgba::GREEN, m_debugRenderMode );
	}
	if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_G ) )
	{
		DebugRenderGrid( 10.0f, m_gameCamera->GetPosition(), Vector3::RIGHT, -5.0f, 5.0f, Vector3::FORWARD, -5.0f, 5.0f, IntVector2( 10, 10 ), Rgba::RED, Rgba::GREEN, m_debugRenderMode );
	}
	if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_B ) )
	{
		Vector3 position = m_gameCamera->GetPosition();
		Matrix44 modelMatrix = m_gameCamera->GetModelMatrix();
		modelMatrix.SetTranslation( position );	// Ensures that the basis renders at the camera's location
		DebugRenderBasis( 10.0f, modelMatrix, Rgba::WHITE, Rgba::WHITE, m_debugRenderMode );
	}
	if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_Q ) )
	{
		DebugRender2DQuad( 10.0f, AABB2( -( Window::GetWidthF() * 0.5f ), -( Window::GetHeightF() * 0.5f ), 0.0f, 0.0f ), Rgba::RED, Rgba::GREEN );
	}
	if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_L ) )
	{
		DebugRender2DLine( 10.0f, Vector2::ZERO, Rgba::YELLOW, Vector2( ( Window::GetWidthF() * 0.5f ), ( Window::GetHeightF() * 0.5f ) ), Rgba::MAGENTA, Rgba::RED, Rgba::GREEN );
	}
	if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_K ) )
	{
		DebugRenderLineSegment( 10.0f, Vector3::ZERO, Rgba::YELLOW, m_gameCamera->GetPosition(), Rgba::MAGENTA, Rgba::RED, Rgba::GREEN, m_debugRenderMode );
	}
	if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_N ) )
	{
		DebugRenderText( 5.0f, m_gameCamera->GetPosition(), m_gameCamera->GetRight(), m_gameCamera->GetUp(), 1.0f, 1.0f, Vector2( 0.5f, 0.5f ), Rgba::RED, Rgba::GREEN, m_debugRenderMode, nullptr, DEFAULT_FONT_NAME, "I'm not always facing you... %d", GetRandomIntLessThan( 600 ) );
	}
	if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_H ) )
	{
		DebugRenderTag( 5.0f, m_gameCamera->GetPosition(), 1.0f, 1.0f, Vector2( 0.5f, 0.5f ), Rgba::RED, Rgba::GREEN, m_debugRenderMode, nullptr, DEFAULT_FONT_NAME, "I'm always facing you! %d", GetRandomIntLessThan( 600 ) );
	}
	{
		if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_Y ) )
		{
			DebugRender2DText( 10.0f, Vector2( -( Window::GetWidthF() * 0.5f ), 0.0f ), 25.0f, 1.0f, Vector2( 0.0f, 0.5f ), Rgba::RED, Rgba::GREEN, nullptr, DEFAULT_FONT_NAME, "Pressed: %d", 1 );
		}
		if ( g_inputSystem->WasKeyJustReleased( InputSystem::KEYBOARD_Y ) )
		{
			DebugRender2DText( 10.0f, Vector2( ( Window::GetWidthF() * 0.5f ), 0.0f ), 25.0f, 1.0f, Vector2( 1.0f, 0.5f ), Rgba::RED, Rgba::GREEN, nullptr, DEFAULT_FONT_NAME, "Released: %d", 2 );
		}
	}
	{
		if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_M ) )
		{
			DebugRenderLogF( 3.0f, Rgba::RED, Rgba::GREEN, nullptr, DEFAULT_FONT_NAME, "This is a random %d log statement on button press!", GetRandomIntLessThan( 600 ) );
		}
		if ( g_inputSystem->WasKeyJustReleased( InputSystem::KEYBOARD_M ) )
		{
			DebugRenderLogF( 3.0f, Rgba::RED, Rgba::GREEN, nullptr, DEFAULT_FONT_NAME, "This is a random %d log statement on button release!", GetRandomIntLessThan( 600 ) );
		}
	}
}

void TheGame::HandleKeyboardInputToCycleMeshRenderMode()
{
	if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_9 ) )
	{
		m_renderModeIndex--;
		if ( m_renderModeIndex == -1 )
		{
			m_renderModeIndex = static_cast< int >( MeshRenderMode::NUM_RENDER_MODES ) - 1;
		}
		DebugRenderLogF( 4.0f, Rgba::RED, Rgba::GREEN, nullptr, DEFAULT_FONT_NAME, "Render mode: %s", GetMeshRenderModeName( m_renderModeIndex ) );
		m_renderModeChanged = true;
	}
	if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_0 ) )
	{
		m_renderModeIndex = ( m_renderModeIndex + 1 ) % static_cast< int >( MeshRenderMode::NUM_RENDER_MODES );
		DebugRenderLogF( 4.0f, Rgba::RED, Rgba::GREEN, nullptr, DEFAULT_FONT_NAME, "Render mode: %s", GetMeshRenderModeName( m_renderModeIndex ) );
		m_renderModeChanged = true;
	}
}

void TheGame::HandleKeyboardInputToSwitchDebugRenderMode()
{
	if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_1 ) )
	{
		m_debugRenderMode = DebugRenderMode::DEBUG_RENDER_USE_DEPTH;
	}
	if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_2 ) )
	{
		m_debugRenderMode = DebugRenderMode::DEBUG_RENDER_IGNORE_DEPTH;
	}
	if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_3 ) )
	{
		m_debugRenderMode = DebugRenderMode::DEBUG_RENDER_HIDDEN;
	}
	if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_4 ) )
	{
		m_debugRenderMode = DebugRenderMode::DEBUG_RENDER_XRAY;
	}
}

void TheGame::HandleXboxControllerInputs()
{
	
}

Vector3 GraphSurfacePatch( float u, float v ) // Represents the equation y = sin( sqrt( x^2 + z^2 ) ); arbitrary, just a tech demo
{
	Vector3 pointOnGraph = Vector3( u, 0.0f, v );
	pointOnGraph.y = SinDegrees( 360.0f * sqrt( pointOnGraph.x * pointOnGraph.x + pointOnGraph.z * pointOnGraph.z ) );
	return pointOnGraph;
}

Vector3 GraphSurfacePatchZ( float u, float v ) // Represents the equation z = sin( sqrt( x^2 + y^2 ) ); arbitrary, just a tech demo
{
	Vector3 pointOnGraph = Vector3( u, v, 0.0f );
	pointOnGraph.z = SinDegrees( 360.0f * sqrt( pointOnGraph.x * pointOnGraph.x + pointOnGraph.y * pointOnGraph.y ) ) + 3.0f;
	return pointOnGraph;
}

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

bool SetSpecularPropertiesCommand( Command& specularPropertiesCommand )
{
	if ( specularPropertiesCommand.GetName() == "set_specular" )
	{
		std::string specularObjectString = specularPropertiesCommand.GetNextString();
		if ( specularObjectString != "shapes" && specularObjectString != "ship" && specularObjectString != "patch" && specularObjectString != "miku" )
		{
			ConsolePrintf( Rgba::RED, "ERROR: set_specular: Invalid object string provided. Must be \"shapes\", \"ship\", \"miku\" or \"patch\"" );
			return false;
		}

		std::string specularAmountString = specularPropertiesCommand.GetNextString();
		if ( specularAmountString == "" )
		{
			ConsolePrintf( Rgba::RED, "ERROR: set_specular: No specular amount string provided" );
			return false;
		}

		float specularAmount = 0.0f;
		try
		{
			specularAmount = stof( specularAmountString );
		}
		catch ( std::invalid_argument& invalidArgument )
		{
			UNUSED( invalidArgument );
			ConsolePrintf( Rgba::RED, "ERROR: set_specular: Specular amount must be a number between 0.0 and 1.0" );
			return false;
		}
		if ( specularAmount < 0.0f || specularAmount > 1.0f )
		{
			ConsolePrintf( Rgba::RED, "ERROR: set_specular: Specular amount must be a number between 0.0 and 1.0" );
			return false;
		}

		std::string specularPowerString = specularPropertiesCommand.GetNextString();
		if ( specularPowerString == "" )
		{
			ConsolePrintf( Rgba::RED, "ERROR: set_specular: No specular power string provided" );
			return false;
		}
		float specularPower = 0.0f;
		try
		{
			specularPower = stof( specularPowerString );
		}
		catch ( std::invalid_argument& invalidArgument )
		{
			UNUSED( invalidArgument );
			ConsolePrintf( Rgba::RED, "ERROR: set_specular: Specular power must be a number above or equal to 1.0" );
			return false;
		}
		if ( specularPower < 1.0f )
		{
			ConsolePrintf( Rgba::RED, "ERROR: set_specular: Specular power must be a number above or equal to 1.0" );
			return false;
		}

		std::string shaderName = specularPropertiesCommand.GetNextString();
		if ( shaderName == "" )
		{
			shaderName = "DefaultLit";
		}

		if ( specularObjectString == "shapes" )
		{
			g_theGame->SetShapeSpecularProperties( specularAmount, specularPower );
		}
		else if ( specularObjectString == "ship" )
		{
			g_theGame->SetShipSpecularProperties( specularAmount, specularPower );
		}
		else if ( specularObjectString == "patch" )
		{
			g_theGame->SetSurfacePatchSpecularProperties( specularAmount, specularPower );
		}
		else if ( specularObjectString == "miku" )
		{
			g_theGame->SetMikuSpecularProperties( specularAmount, specularPower );
		}

		return true;
	}
	return false;
}

bool CameraLightCommand( Command& cameraLightCommand )
{
	if ( cameraLightCommand.GetName() == "camera_light" )
	{
		std::string lightTypeString = cameraLightCommand.GetNextString();
		LightType lightType = LightType::LIGHT_TYPE_INVALID;
		if ( lightTypeString == "" )
		{
			ConsolePrintf( Rgba::RED, "ERROR: camera_light: No light type string provided. Accepted values are P, D and S, for Point, Directional and Spot." );
			return false;
		}
		else if ( lightTypeString == "p" || lightTypeString == "P" )
		{
			lightType = LightType::LIGHT_TYPE_POINT;
		}
		else if ( lightTypeString == "d" || lightTypeString == "D" )
		{
			lightType = LightType::LIGHT_TYPE_DIRECTIONAL;
		}
		else if ( lightTypeString == "s" || lightTypeString == "S" )
		{
			lightType = LightType::LIGHT_TYPE_SPOT;
		}

		std::string colorString = cameraLightCommand.GetNextString();
		Vector4 color = Vector4::ZERO_DISPLACEMENT;
		if ( Vector4::IsValidString( colorString ) )
		{
			color.SetFromText( colorString );
		}
		else
		{
			ConsolePrintf( Rgba::RED, "ERROR: camera_light: No color string provided. Must provide a Vector4 in the format \"r,g,b,a\"." );
			return false;
		}

		std::string attenuationString = cameraLightCommand.GetNextString();
		Vector3 attenuation = Vector3::ZERO;
		if ( Vector3::IsValidString( attenuationString ) )
		{
			attenuation.SetFromText( attenuationString );
		}
		else
		{
			ConsolePrintf( Rgba::RED, "ERROR: camera_light: No attenuation string provided. Must provide a Vector3 in the format \"a0,a1,a2\"." );
			return false;
		}

		float innerAngle = 0.0f;
		float outerAngle = 0.0f;
		if ( lightType == LightType::LIGHT_TYPE_SPOT )
		{
			std::string innerAngleString = cameraLightCommand.GetNextString();
			if ( innerAngleString == "" )
			{
				ConsolePrintf( Rgba::RED, "ERROR: camera_light: No inner angle provided" );
				return false;
			}
			try
			{
				innerAngle = stof( innerAngleString );
			}
			catch ( std::invalid_argument& invalidArgument )
			{
				UNUSED( invalidArgument );
				ConsolePrintf( Rgba::RED, "ERROR: camera_light: Inner angle invalid" );
				return false;
			}

			std::string outerAngleString = cameraLightCommand.GetNextString();
			if ( outerAngleString == "" )
			{
				ConsolePrintf( Rgba::RED, "ERROR: camera_light: No outer angle provided" );
				return false;
			}
			try
			{
				outerAngle = stof( outerAngleString );
			}
			catch ( std::invalid_argument& invalidArgument )
			{
				UNUSED( invalidArgument );
				ConsolePrintf( Rgba::RED, "ERROR: camera_light: Outer angle invalid" );
				return false;
			}
		}

		g_theGame->SetCameraLight( lightType, color, attenuation, Vector2( innerAngle, outerAngle ) );

		return true;
	}
	return false;
}

bool DetachCameraLightCommand( Command& detachCommand )
{
	if ( detachCommand.GetName() == "detach_light" )
	{
		g_theGame->SwitchCameraLight();
		return true;
	}
	return false;
}

bool AddConnectionCommand( Command& command )
{
	if ( command.GetName() == "add_connection" )
	{
		int index;
		NetAddressIPv4 addr;

		std::string indexStr = command.GetNextString();
		if ( indexStr == "" )
		{
			ConsolePrintf( Rgba::RED, "ERROR: add_connection: No index provided." );
			return false;
		}

		try
		{
			index = stoi( indexStr );
		}
		catch ( std::invalid_argument& arg )
		{
			UNUSED( arg );
			ConsolePrintf( Rgba::RED, "ERROR: add_connection: Invalid index provided." );
			return false;
		}

		if ( index < 0 )
		{
			ConsolePrintf( Rgba::RED, "ERROR: add_connection: Index must be a whole number." );
			return false;
		}

		std::string addrStr = command.GetNextString();
		if ( addrStr == "" || !NetAddressIPv4::IsValid( addrStr.c_str() ) )
		{
			ConsolePrintf( Rgba::RED, "ERROR: add_connection: No or invalid address provided." );
			return false;
		}

		addr.FromString( addrStr );

		NetSession* session = g_theGame->GetSession();
		NetConnection* connection = session->AddConnection( index, addr );
		if ( connection == nullptr )
		{
			ConsolePrintf( Rgba::RED, "ERROR: add_connection: Could not create a connection." );
			return false;
		}

		ConsolePrintf( Rgba::GREEN, "add_connection: Connection added at index %u.", index );
		return true;
	}

	return false;
}

bool SendPingCommand( Command& command )
{
	if ( command.GetName() == "send_ping" )
	{
		int index;

		std::string indexStr = command.GetNextString();
		if ( indexStr == "" )
		{
			ConsolePrintf( Rgba::RED, "ERROR: send_ping: No index provided." );
			return false;
		}

		try
		{
			index = stoi( indexStr );
		}
		catch ( std::invalid_argument& arg )
		{
			UNUSED( arg );
			ConsolePrintf( Rgba::RED, "ERROR: send_ping: Invalid index provided." );
			return false;
		}

		if ( index < 0 )
		{
			ConsolePrintf( Rgba::RED, "ERROR: send_ping: Index must be a whole number." );
			return false;
		}

		NetSession* session = g_theGame->GetSession();
		NetConnection* connection = session->GetConnection( index );
		if ( connection == nullptr )
		{
			ConsolePrintf( Rgba::RED, "ERROR: send_ping: No connection at provided index." );
			return false;
		}

		std::string pingStr = command.GetNextString();

		uint8_t messageIndex = g_theGame->GetSession()->GetMessageIndex( "ping" );
		NetMessage ping( messageIndex );
		ping.WriteString( pingStr.c_str() );
		connection->Send( ping );

		return true;
	}

	return false;
}

bool SendAddCommand( Command& command )
{
	if ( command.GetName() == "send_add" )
	{
		int index;
		std::string indexStr = command.GetNextString();
		if ( indexStr == "" )
		{
			ConsolePrintf( Rgba::RED, "ERROR: send_add: No index provided." );
			return false;
		}
		try
		{
			index = stoi( indexStr );
		}
		catch ( std::invalid_argument& arg )
		{
			UNUSED( arg );
			ConsolePrintf( Rgba::RED, "ERROR: send_add: Invalid index provided." );
			return false;
		}
		if ( index < 0 )
		{
			ConsolePrintf( Rgba::RED, "ERROR: send_add: Index must be a whole number." );
			return false;
		}

		float val0;
		float val1;

		std::string valStr = command.GetNextString();
		if ( valStr == "" )
		{
			ConsolePrintf( Rgba::RED, "ERROR: send_add: No value provided." );
			return false;
		}
		try
		{
			val0 = stof( valStr );
		}
		catch ( std::invalid_argument& arg )
		{
			UNUSED( arg );
			ConsolePrintf( Rgba::RED, "ERROR: send_add: Invalid first value provided." );
			return false;
		}
		valStr = command.GetNextString();
		if ( valStr == "" )
		{
			ConsolePrintf( Rgba::RED, "ERROR: send_add: No second value provided." );
			return false;
		}
		try
		{
			val1 = stof( valStr );
		}
		catch ( std::invalid_argument& arg )
		{
			UNUSED( arg );
			ConsolePrintf( Rgba::RED, "ERROR: send_add: Invalid second value provided." );
			return false;
		}

		NetSession* session = g_theGame->GetSession();
		NetConnection* connection = session->GetConnection( index );
		if ( connection == nullptr )
		{
			ConsolePrintf( Rgba::RED, "ERROR: send_add: No connection at provided index." );
			return false;
		}

		uint8_t messageIndex = g_theGame->GetSession()->GetMessageIndex( "add" );
		NetMessage msg( messageIndex );
		msg.WriteFloat( val0 );
		msg.WriteFloat( val1 );
		connection->Send( msg );

		return true;
	}

	return false;
}

#pragma endregion
