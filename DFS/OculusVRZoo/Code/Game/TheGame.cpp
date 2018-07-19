#include "Game/TheGame.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/Window.hpp"
#include "Engine/OculusRift/OVRContext.hpp"
#include "Engine/OculusRift/OVRHeadset.hpp"
#include "Engine/OculusRift/OVRInput.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Renderer/ForwardRenderingPath.hpp"
#include "Engine/Renderer/Light.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/Renderable.hpp"
#include "Engine/Renderer/Scene.hpp"
#include "Engine/Tools/Command.hpp"
#include "Engine/Tools/DevConsole.hpp"

TheGame::TheGame()
{
	SetupWorldObjects();
	SetupLights();
	SetupVRObjects();
	SetupVRScene();
	DebugRenderSet3DCamera( OVRContext::GetHeadset()->GetLeftCamera() );
	DebugRenderSet2DCamera( OVRContext::GetHeadset()->GetLeftHUDCamera() );
}

TheGame::~TheGame()
{
	if ( m_modelBuilderScene != nullptr )
	{
		delete m_modelBuilderScene;
		m_modelBuilderScene = nullptr;
	}

	if ( m_galleryScene != nullptr )
	{
		delete m_galleryScene;
		m_galleryScene = nullptr;
	}

	delete m_scene;
	m_scene = nullptr;

	delete m_flashlight;
	m_flashlight = nullptr;

	delete m_directionalLight;
	m_directionalLight = nullptr;

	delete m_leftHand;
	m_leftHand = nullptr;

	delete m_rightHand;
	m_rightHand = nullptr;

	delete m_grid;
	m_grid = nullptr;

	delete m_xButtonInstruction;
	m_xButtonInstruction = nullptr;

	delete m_yButtonInstruction;
	m_yButtonInstruction = nullptr;

	delete m_aButtonInstruction;
	m_aButtonInstruction = nullptr;

	delete m_bButtonInstruction;
	m_bButtonInstruction = nullptr;

	delete m_lStickButtonInstruction;
	m_lStickButtonInstruction = nullptr;

	delete m_rStickButtonInstruction;
	m_rStickButtonInstruction = nullptr;

	delete m_lStickInstruction;
	m_lStickInstruction = nullptr;

	delete m_rStickInstruction;
	m_rStickInstruction = nullptr;

	delete m_rIndexTriggerInstruction;
	m_rIndexTriggerInstruction = nullptr;

	delete m_rHandTriggerInstruction;
	m_rHandTriggerInstruction = nullptr;

	delete m_lIndexTriggerInstruction;
	m_lIndexTriggerInstruction = nullptr;

	delete m_lHandTriggerInstruction;
	m_lHandTriggerInstruction = nullptr;
}

void TheGame::SetupLights()
{
	g_renderer->SetAmbientLight( Vector4( 1.0f, 1.0f, 1.0f, 0.25f ) );

	m_directionalLight = new Light();
	m_directionalLight->SetDirectionalLight( Vector3::ZERO, g_gameConfigBlackboard.GetValue( "directionalLightDirection", Vector3::ONE ), g_gameConfigBlackboard.GetValue( "directionalLightColor", Vector4::ONE_POINT ), Vector3::ZERO );
	m_directionalLight->m_lightUBO.m_castsShadows = 0;

	m_flashlight = new Light();
	m_flashlight->m_transform.Reparent( &m_leftHandTransform );
	Vector3 flashlightPosition = Vector3( 0.0f, 0.0f, 0.1f );	// Attach to front of hand mesh
	m_flashlight->SetSpotLight(
		m_flashlight->m_transform.GetWorldPosition(),
		m_leftHandTransform.GetAsMatrixLocal().GetKBasis(),
		g_gameConfigBlackboard.GetValue( "flashlightColor", Vector4::ONE_POINT ),
		20.0f,
		45.0f,
		Vector3( 0.0f, 1.0f, 0.0f )
	);
	m_flashlight->m_lightUBO.m_castsShadows = false;
}

void TheGame::SetupWorldObjects()
{
	MeshBuilder gridMeshBuilder = g_renderer->MakeGridMesh( Vector3( 0.0f, -g_gameConfigBlackboard.GetValue( "vrCameraHeightm", 1.8f ), 0.0f ), AABB2( -5.0f, -5.0f, 5.0f, 5.0f ), Vector3::FORWARD, Vector3::RIGHT, IntVector2( 10, 10 ), Rgba::WHITE );
	m_grid = new Renderable(	gridMeshBuilder.CreateMesh(),
								g_renderer->CreateOrGetMaterial( "Grid" )->GetInstance()
	);
}

void TheGame::SetupVRObjects()
{
	Vector3 handDimensions = g_gameConfigBlackboard.GetValue( "handDimensions", Vector3( 0.125f, 0.125f, 0.2f ) );
	MeshBuilder handMeshBuilder = g_renderer->MakeTexturedCubeMesh(
		Vector3::ZERO,
		handDimensions,
		*g_renderer->GetDefaultTexture(),
		Rgba::WHITE
	);

	m_leftHand = new Renderable(
		handMeshBuilder.CreateMesh(),
		g_renderer->CreateOrGetMaterial( "LeftHand" ),
		true,
		false
	);
	m_rightHand = new Renderable(
		handMeshBuilder.CreateMesh(),
		g_renderer->CreateOrGetMaterial( "RightHand" ),
		true,
		false
	);

	m_leftHandTransform.Reparent( OVRContext::GetInputSystem()->GetControllerTransformReference( OVRControllerType::OVR_TOUCH_CONTROLLER_LEFT ) );
	m_rightHandTransform.Reparent( OVRContext::GetInputSystem()->GetControllerTransformReference( OVRControllerType::OVR_TOUCH_CONTROLLER_RIGHT ) );
}

void TheGame::SetupVRScene()
{
	m_scene = new Scene();
	m_scene->AddCamera( OVRContext::GetInstance()->GetHeadset()->GetLeftCamera() );
	m_scene->AddCamera( OVRContext::GetInstance()->GetHeadset()->GetRightCamera() );
	m_scene->AddLight( m_directionalLight );
	m_scene->AddLight( m_flashlight );
	m_scene->AddRenderable( m_grid );
	m_scene->AddRenderable( m_leftHand );
	m_scene->AddRenderable( m_rightHand );
}

void TheGame::Update()
{
	HandleSceneTransition();
	switch ( m_currentScene )
	{
		case SCENE_MENU:
		{
			UpdateHandSpatialInformation();

			if ( !IsDevConsoleOpen() )
			{
				HandleKeyboardInput();
				HandleOVRInput();
			}

			UpdateHUDText();
			break;
		}
		case SCENE_GALLERY:
		{
			m_galleryScene->Update();
			break;
		}
		case SCENE_MODEL:
		{
			m_modelBuilderScene->Update();
			break;
		}
	}
}

void TheGame::HandleSceneTransition()
{
	if ( m_nextScene != m_currentScene )
	{
		switch ( m_nextScene )
		{
			case SCENE_MENU:
			{
				m_leftHandTransform.Reparent( OVRContext::GetInputSystem()->GetControllerTransformReference( OVRControllerType::OVR_TOUCH_CONTROLLER_LEFT ) );
				m_rightHandTransform.Reparent( OVRContext::GetInputSystem()->GetControllerTransformReference( OVRControllerType::OVR_TOUCH_CONTROLLER_RIGHT ) );
				m_flashlight->m_transform.Reparent( &m_leftHandTransform );
				if ( m_modelBuilderScene != nullptr )
				{
					delete m_modelBuilderScene;
					m_modelBuilderScene = nullptr;
				}
				if ( m_galleryScene != nullptr )
				{
					delete m_galleryScene;
					m_galleryScene = nullptr;
				}
				m_currentScene = SCENE_MENU;
				break;
			}
			case SCENE_GALLERY:
			{
				m_flashlight->m_transform.Reparent( nullptr );
				m_leftHandTransform.Reparent( nullptr );
				m_rightHandTransform.Reparent( nullptr );
				if ( m_modelBuilderScene != nullptr )
				{
					delete m_modelBuilderScene;
					m_modelBuilderScene = nullptr;
				}
				m_galleryScene = new GalleryScene();
				m_currentScene = SCENE_GALLERY;
				break;
			}
			case SCENE_MODEL:
			{
				m_flashlight->m_transform.Reparent( nullptr );
				m_leftHandTransform.Reparent( nullptr );
				m_rightHandTransform.Reparent( nullptr );
				if ( m_galleryScene != nullptr )
				{
					delete m_galleryScene;
					m_galleryScene = nullptr;
				}
				m_modelBuilderScene = new ModelBuilderScene();
				m_currentScene = SCENE_MODEL;
				break;
			}
		}
	}
}

void TheGame::SetNextScene( OVRZooScene scene )
{
	m_nextScene = scene;
}

void TheGame::HandleKeyboardInput()
{

}


void TheGame::HandleOVRInput()
{
	// Left controller
	if ( OVRContext::GetInputSystem()->WasButtonJustPressed( OVR_BUTTON_THUMB_L ) )
	{
		m_nextScene = SCENE_GALLERY;
	}
	HandleOVRInputForFlashlightAdjustment();

	// Right controller
	if ( OVRContext::GetInputSystem()->WasButtonJustPressed( OVR_BUTTON_THUMB_R ) )
	{
		m_nextScene = SCENE_MODEL;
	}
}

void TheGame::HandleOVRInputForFlashlightAdjustment()
{
	Vector2 leftStickPositionCartesian = OVRContext::GetInputSystem()->GetJoystickPositionCartesian( OVRControllerType::OVR_TOUCH_CONTROLLER_LEFT );
	if ( leftStickPositionCartesian != Vector2::ZERO )
	{
		float intensityDelta = g_gameConfigBlackboard.GetValue( "flashlightIntensityDeltaPerSecond", 0.25f );
		m_flashlight->m_lightUBO.m_colorAndIntensity.w += leftStickPositionCartesian.y * intensityDelta * GetMasterDeltaSecondsF();
		m_flashlight->m_lightUBO.m_colorAndIntensity.w = ClampFloat( m_flashlight->m_lightUBO.m_colorAndIntensity.w, 0.0f, g_gameConfigBlackboard.GetValue( "flashlightMaxIntensity", 50.0f ) );
	}
}

void TheGame::UpdateHandSpatialInformation()
{
	Matrix44 leftHandMatrix = m_leftHandTransform.GetAsMatrixWorld();
	m_leftHand->SetModelMatrix( leftHandMatrix );
	Matrix44 rightHandMatrix = m_rightHandTransform.GetAsMatrixWorld();
	rightHandMatrix.SetIBasis( rightHandMatrix.GetIBasis() * -1.0f );
	m_rightHand->SetModelMatrix( rightHandMatrix );
}

void TheGame::UpdateHUDText()
{
	Vector2 halfScreenDimensions = ConvertIntVector2ToVector2( OVRContext::GetHeadset()->GetResolution() );
	Material* textMaterial = g_renderer->CreateOrGetMaterial( "UI" );
	textMaterial->SetTextureAndSampler( 0U, g_renderer->CreateOrGetBitmapFont( DEFAULT_FONT_NAME )->GetTexture(), g_renderer->GetDefaultSampler() );
	std::string instructionText;
	Vector2 textMins;
	MeshBuilder textBuilder;
	Rgba textColor;

	instructionText = "X - Unmapped";
	textMins = Vector2( ( -0.325f * halfScreenDimensions.x ), ( 0.0f * halfScreenDimensions.y ) );
	textColor = Rgba::WHITE;
	textBuilder = g_renderer->MakeTextMesh2D(
		textMins,
		instructionText,
		( 0.0035f * halfScreenDimensions.y ),
		( 0.0035f * halfScreenDimensions.x ),
		textColor,
		g_renderer->CreateOrGetBitmapFont( DEFAULT_FONT_NAME )
	);
	Mesh* xButtonMesh = textBuilder.CreateMesh();
	if ( m_xButtonInstruction == nullptr )
	{
		m_xButtonInstruction = new Renderable(
			xButtonMesh,
			textMaterial->GetInstance()
		);
	}
	else
	{
		m_xButtonInstruction->ReplaceMesh( xButtonMesh );
	}

	instructionText = "Y - Unmapped";
	textMins = Vector2( ( -0.325f * halfScreenDimensions.x ), ( -0.005f * halfScreenDimensions.y ) );
	textColor = Rgba::WHITE;
	textBuilder = g_renderer->MakeTextMesh2D(
		textMins,
		instructionText,
		( 0.0035f * halfScreenDimensions.y ),
		( 0.0035f * halfScreenDimensions.x ),
		textColor,
		g_renderer->CreateOrGetBitmapFont( DEFAULT_FONT_NAME )
	);
	Mesh* yButtonMesh = textBuilder.CreateMesh();
	if ( m_yButtonInstruction == nullptr )
	{
		m_yButtonInstruction = new Renderable(
			yButtonMesh,
			textMaterial->GetInstance()
		);
	}
	else
	{
		m_yButtonInstruction->ReplaceMesh( yButtonMesh );
	}

	instructionText = "A - Unmapped";
	textMins = Vector2( ( -0.325f * halfScreenDimensions.x ), ( 0.0f * halfScreenDimensions.y ) );
	textColor = Rgba::WHITE;
	textBuilder = g_renderer->MakeTextMesh2D(
		textMins,
		instructionText,
		( 0.0035f * halfScreenDimensions.y ),
		( 0.0035f * halfScreenDimensions.x ),
		textColor,
		g_renderer->CreateOrGetBitmapFont( DEFAULT_FONT_NAME )
	);
	Mesh* aButtonMesh = textBuilder.CreateMesh();
	if ( m_aButtonInstruction == nullptr )
	{
		m_aButtonInstruction = new Renderable(
			aButtonMesh,
			textMaterial->GetInstance()
		);
	}
	else
	{
		m_aButtonInstruction->ReplaceMesh( aButtonMesh );
	}

	instructionText = "B - Unmapped";
	textMins = Vector2( ( -0.325f * halfScreenDimensions.x ), ( -0.005f * halfScreenDimensions.y ) );
	textColor = Rgba::WHITE;
	textBuilder = g_renderer->MakeTextMesh2D(
		textMins,
		instructionText,
		( 0.0035f * halfScreenDimensions.y ),
		( 0.0035f * halfScreenDimensions.x ),
		textColor,
		g_renderer->CreateOrGetBitmapFont( DEFAULT_FONT_NAME )
	);
	Mesh* bButtonMesh = textBuilder.CreateMesh();
	if ( m_bButtonInstruction == nullptr )
	{
		m_bButtonInstruction = new Renderable(
			bButtonMesh,
			textMaterial->GetInstance()
		);
	}
	else
	{
		m_bButtonInstruction->ReplaceMesh( bButtonMesh );
	}

	instructionText = "Stick button - Go to Gallery";
	textMins = Vector2( ( -0.325f * halfScreenDimensions.x ), ( -0.01f * halfScreenDimensions.y ) );
	textColor = Rgba::WHITE;
	textBuilder = g_renderer->MakeTextMesh2D(
		textMins,
		instructionText,
		( 0.0035f * halfScreenDimensions.y ),
		( 0.0035f * halfScreenDimensions.x ),
		textColor,
		g_renderer->CreateOrGetBitmapFont( DEFAULT_FONT_NAME )
	);
	Mesh* stickButtonMesh = textBuilder.CreateMesh();
	if ( m_lStickButtonInstruction == nullptr )
	{
		m_lStickButtonInstruction = new Renderable(
			stickButtonMesh,
			textMaterial->GetInstance()
		);
	}
	else
	{
		m_lStickButtonInstruction->ReplaceMesh( stickButtonMesh );
	}

	instructionText = "Index Trigger - Unmapped";
	textMins = Vector2( ( -0.325f * halfScreenDimensions.x ), ( -0.025f * halfScreenDimensions.y ) );
	textColor = Rgba::WHITE;
	textBuilder = g_renderer->MakeTextMesh2D(
		textMins,
		instructionText,
		( 0.0035f * halfScreenDimensions.y ),
		( 0.0035f * halfScreenDimensions.x ),
		textColor,
		g_renderer->CreateOrGetBitmapFont( DEFAULT_FONT_NAME )
	);
	Mesh* lIndexTriggerMesh = textBuilder.CreateMesh();
	if ( m_lIndexTriggerInstruction == nullptr )
	{
		m_lIndexTriggerInstruction = new Renderable(
			lIndexTriggerMesh,
			textMaterial->GetInstance()
		);
	}
	else
	{
		m_lIndexTriggerInstruction->ReplaceMesh( lIndexTriggerMesh );
	}

	instructionText = "Hand Trigger - Unmapped";
	textMins = Vector2( ( -0.325f * halfScreenDimensions.x ), ( -0.03f * halfScreenDimensions.y ) );
	textColor = Rgba::WHITE;
	textBuilder = g_renderer->MakeTextMesh2D(
		textMins,
		instructionText,
		( 0.0035f * halfScreenDimensions.y ),
		( 0.0035f * halfScreenDimensions.x ),
		textColor,
		g_renderer->CreateOrGetBitmapFont( DEFAULT_FONT_NAME )
	);
	Mesh* lHandTriggerMesh = textBuilder.CreateMesh();
	if ( m_lHandTriggerInstruction == nullptr )
	{
		m_lHandTriggerInstruction = new Renderable(
			lHandTriggerMesh,
			textMaterial->GetInstance()
		);
	}
	else
	{
		m_lHandTriggerInstruction->ReplaceMesh( lHandTriggerMesh );
	}

	instructionText = "Stick - Tune Flashlight Intensity ";
	instructionText += std::to_string( m_flashlight->m_lightUBO.m_colorAndIntensity.w );
	textMins = Vector2( ( -0.325f * halfScreenDimensions.x ), ( -0.015f * halfScreenDimensions.y ) );
	textColor = Rgba::WHITE;
	textBuilder = g_renderer->MakeTextMesh2D(
		textMins,
		instructionText,
		( 0.0035f * halfScreenDimensions.y ),
		( 0.0035f * halfScreenDimensions.x ),
		textColor,
		g_renderer->CreateOrGetBitmapFont( DEFAULT_FONT_NAME )
	);
	Mesh* stickMesh = textBuilder.CreateMesh();
	if ( m_lStickInstruction == nullptr )
	{
		m_lStickInstruction = new Renderable(
			stickMesh,
			textMaterial->GetInstance()
		);
	}
	else
	{
		m_lStickInstruction->ReplaceMesh( stickMesh );
	}

	instructionText = "Stick button - Go to Model Builder";
	textMins = Vector2( ( -0.325f * halfScreenDimensions.x ), ( -0.01f * halfScreenDimensions.y ) );
	textColor = Rgba::WHITE;
	textBuilder = g_renderer->MakeTextMesh2D(
		textMins,
		instructionText,
		( 0.0035f * halfScreenDimensions.y ),
		( 0.0035f * halfScreenDimensions.x ),
		textColor,
		g_renderer->CreateOrGetBitmapFont( DEFAULT_FONT_NAME )
	);
	stickButtonMesh = textBuilder.CreateMesh();
	if ( m_rStickButtonInstruction == nullptr )
	{
		m_rStickButtonInstruction = new Renderable(
			stickButtonMesh,
			textMaterial->GetInstance()
		);
	}
	else
	{
		m_rStickButtonInstruction->ReplaceMesh( stickButtonMesh );
	}

	instructionText = "Stick - Unmapped";
	textMins = Vector2( ( -0.325f * halfScreenDimensions.x ), ( -0.015f * halfScreenDimensions.y ) );
	textColor = Rgba::WHITE;
	textBuilder = g_renderer->MakeTextMesh2D(
		textMins,
		instructionText,
		( 0.0035f * halfScreenDimensions.y ),
		( 0.0035f * halfScreenDimensions.x ),
		textColor,
		g_renderer->CreateOrGetBitmapFont( DEFAULT_FONT_NAME )
	);
	stickMesh = textBuilder.CreateMesh();
	if ( m_rStickInstruction == nullptr )
	{
		m_rStickInstruction = new Renderable(
			stickMesh,
			textMaterial->GetInstance()
		);
	}
	else
	{
		m_rStickInstruction->ReplaceMesh( stickMesh );
	}

	instructionText = "Index Trigger - Unmapped";
	textMins = Vector2( ( -0.325f * halfScreenDimensions.x ), ( -0.025f * halfScreenDimensions.y ) );
	textColor = Rgba::WHITE;
	textBuilder = g_renderer->MakeTextMesh2D(
		textMins,
		instructionText,
		( 0.0035f * halfScreenDimensions.y ),
		( 0.0035f * halfScreenDimensions.x ),
		textColor,
		g_renderer->CreateOrGetBitmapFont( DEFAULT_FONT_NAME )
	);
	Mesh* rIndexTriggerMesh = textBuilder.CreateMesh();
	if ( m_rIndexTriggerInstruction == nullptr )
	{
		m_rIndexTriggerInstruction = new Renderable(
			rIndexTriggerMesh,
			textMaterial->GetInstance()
		);
	}
	else
	{
		m_rIndexTriggerInstruction->ReplaceMesh( rIndexTriggerMesh );
	}

	instructionText = "Hand Trigger - Unmapped";
	textMins = Vector2( ( -0.325f * halfScreenDimensions.x ), ( -0.03f * halfScreenDimensions.y ) );
	textColor = Rgba::WHITE;
	
	textBuilder = g_renderer->MakeTextMesh2D(
		textMins,
		instructionText,
		( 0.0035f * halfScreenDimensions.y ),
		( 0.0035f * halfScreenDimensions.x ),
		textColor,
		g_renderer->CreateOrGetBitmapFont( DEFAULT_FONT_NAME )
	);
	Mesh* triggerMesh = textBuilder.CreateMesh();
	if ( m_rHandTriggerInstruction == nullptr )
	{
		m_rHandTriggerInstruction = new Renderable(
			triggerMesh,
			textMaterial->GetInstance()
		);
	}
	else
	{
		m_rHandTriggerInstruction->ReplaceMesh( triggerMesh );
	}
}

void TheGame::Render() const
{
	switch ( m_currentScene )
	{
		case SCENE_MENU:
		{
			ForwardRenderingPath::RenderScene( m_scene );

			g_renderer->SetCamera( OVRContext::GetHeadset()->GetLeftHUDCamera() );
			g_renderer->DrawRenderable( *m_xButtonInstruction );
			g_renderer->DrawRenderable( *m_yButtonInstruction );
			g_renderer->DrawRenderable( *m_lStickButtonInstruction );
			g_renderer->DrawRenderable( *m_lStickInstruction );
			g_renderer->DrawRenderable( *m_lIndexTriggerInstruction );
			g_renderer->DrawRenderable( *m_lHandTriggerInstruction );

			g_renderer->SetCamera( OVRContext::GetHeadset()->GetRightHUDCamera() );
			g_renderer->DrawRenderable( *m_aButtonInstruction );
			g_renderer->DrawRenderable( *m_bButtonInstruction );
			g_renderer->DrawRenderable( *m_rStickButtonInstruction );
			g_renderer->DrawRenderable( *m_rStickInstruction );
			g_renderer->DrawRenderable( *m_rIndexTriggerInstruction );
			g_renderer->DrawRenderable( *m_rHandTriggerInstruction );
			break;
		}
		case SCENE_GALLERY:
		{
			m_galleryScene->Render();
			break;
		}
		case SCENE_MODEL:
		{
			m_modelBuilderScene->Render();
			break;
		}
	}
}
