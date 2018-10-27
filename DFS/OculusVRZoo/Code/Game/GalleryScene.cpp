#include "Game/GalleryScene.hpp"
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

GalleryScene::GalleryScene()
{
	SetupWorldObjects();
	SetupLights();
	SetupVRObjects();
	SetupVRScene();
	DebugRenderSet3DCamera( OVRContext::GetHeadset()->GetLeftCamera() );
	DebugRenderSet2DCamera( OVRContext::GetHeadset()->GetLeftHUDCamera() );
}

GalleryScene::~GalleryScene()
{
	g_audioSystem->StopSound( m_shipAudioPlaybackID );

	m_flashlight->m_transform.Reparent( nullptr );
	m_leftHandTransform.Reparent( nullptr );
	m_rightHandTransform.Reparent( nullptr );

	delete m_scene;
	m_scene = nullptr;

	delete m_flashlight;
	m_flashlight = nullptr;

	delete m_spotLight;
	m_spotLight = nullptr;

	delete m_directionalLight;
	m_directionalLight = nullptr;

	delete m_leftHand;
	m_leftHand = nullptr;

	delete m_rightHand;
	m_rightHand = nullptr;

	delete m_box;
	m_box = nullptr;

	delete m_ship;
	m_ship = nullptr;

	delete m_grid;
	m_grid = nullptr;

	delete m_sphere;
	m_sphere = nullptr;

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

void GalleryScene::SetupLights()
{
	m_directionalLight = new Light();
	m_directionalLight->SetDirectionalLight( Vector3::ZERO, g_gameConfigBlackboard.GetValue( "directionalLightDirection", Vector3::ONE ), g_gameConfigBlackboard.GetValue( "directionalLightColor", Vector4::ONE_POINT ), Vector3::ZERO );
	m_directionalLight->m_lightUBO.m_castsShadows = 0;

	m_spotLight = new Light();
	Vector3 spotLightPosition = g_gameConfigBlackboard.GetValue( "spotLightRotationRadius", 0.25f ) * Vector3::ONE;
	Vector3 spotLightDirection = ( spotLightPosition * -1.0f );
	spotLightPosition += GetSlotPositionCartesian( 0 );
	m_spotLight->SetSpotLight( spotLightPosition, spotLightDirection, Vector4( 1.0f, 1.0f, 1.0f, 1.0f ), 20.0f, 45.0f, Vector3( 0.0f, 0.0f, 0.0f ) );
	m_spotLight->m_lightUBO.m_castsShadows = 0;

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

void GalleryScene::SetupWorldObjects()
{
	MeshBuilder sphereMeshBuilder = g_renderer->MakeUVSphereMesh( Vector3::ZERO, 0.1f, 20U, 20U, Rgba::WHITE );
	m_sphere = new Renderable(	sphereMeshBuilder.CreateMesh(),
		g_renderer->CreateOrGetMaterial( "Sphere" )->GetInstance()
	);

	MeshBuilder gridMeshBuilder = g_renderer->MakeGridMesh( Vector3( 0.0f, -g_gameConfigBlackboard.GetValue( "vrCameraHeightm", 1.8f ), 0.0f ), AABB2( -5.0f, -5.0f, 5.0f, 5.0f ), Vector3::FORWARD, Vector3::RIGHT, IntVector2( 10, 10 ), Rgba::WHITE );
	m_grid = new Renderable(	gridMeshBuilder.CreateMesh(),
		g_renderer->CreateOrGetMaterial( "Grid" )->GetInstance()
	);

	MeshBuilder boxMeshBuilder = g_renderer->MakeTexturedCubeMesh( Vector3::ZERO, Vector3( 0.25f, 0.25f, 0.25f ), *g_renderer->CreateOrGetMaterial( "Box" )->GetTexture( 0U ), Rgba::WHITE );
	Vector3 boxPosition = GetSlotPositionCartesian( 2 );
	m_box = new Renderable(
		boxMeshBuilder.CreateMesh(),
		g_renderer->CreateOrGetMaterial( "Box" )->GetInstance(),
		true,
		true,
		Matrix44::MakeTranslation( boxPosition )
	);

	Mesh* shipMesh = MeshBuilder::FromFileOBJ( SHIP_OBJ_FILEPATH )[ 0 ];
	Material* shipMaterial = g_renderer->CreateOrGetMaterial( "Ship" )->GetInstance();
	Vector3 shipPosition = GetSlotPositionCartesian( 0 );
	Matrix44 shipModelMatrix = Matrix44::MakeTranslation( shipPosition ) * Matrix44::MakeRotation( Vector3( 0.0f, 180.0f, 0.0f ) );
	shipModelMatrix *= Matrix44::MakeScale( Vector3( 0.1f, 0.1f, 0.1f ) );
	m_ship = new Renderable(
		shipMesh,
		shipMaterial,
		true,
		true,
		shipModelMatrix
	);
	SoundPlaybackID shipSound = g_audioSystem->CreateOrGetSound( "Data/Audio/Scifi_Racecar_Idle_Loop5.wav", true );
	m_shipAudioPlaybackID = g_audioSystem->PlaySound( shipSound, true, 1.5f );
	OVRContext::GetHeadset()->AddSound( m_shipAudioPlaybackID );

	std::vector< Mesh* > snowMikuMeshes = MeshBuilder::FromFileOBJ( "Data/Models/SnowMiku/SnowMiku.obj" );
	Vector3 mikuPosition = GetSlotPositionCartesian( 1 );
	Matrix44 snowMikuMatrix = Matrix44::MakeTranslation( mikuPosition ) * Matrix44::MakeRotation( Vector3( 0.0f, 180.0f, 0.0f ) );
	snowMikuMatrix *= Matrix44::MakeScale( Vector3( 0.2f, 0.2f, 0.2f ) );
	Material* snowMikuMaterial0 = g_renderer->CreateOrGetMaterial( "SnowMiku0" )->GetInstance();
	Material* snowMikuMaterial1 = g_renderer->CreateOrGetMaterial( "SnowMiku1" )->GetInstance();
	Material* snowMikuMaterial2 = g_renderer->CreateOrGetMaterial( "SnowMiku2" )->GetInstance();

	m_snowMiku[ 0 ] = new Renderable();
	m_snowMiku[ 0 ]->SetMesh( snowMikuMeshes[ 0 ] );
	m_snowMiku[ 0 ]->ReplaceMaterial( snowMikuMaterial0 );
	m_snowMiku[ 0 ]->SetModelMatrix( snowMikuMatrix );
	m_snowMiku[ 1 ] = new Renderable();
	m_snowMiku[ 1 ]->SetMesh( snowMikuMeshes[ 1 ] );
	m_snowMiku[ 1 ]->ReplaceMaterial( snowMikuMaterial1 );
	m_snowMiku[ 1 ]->SetModelMatrix( snowMikuMatrix );
	m_snowMiku[ 2 ] = new Renderable();
	m_snowMiku[ 2 ]->SetMesh( snowMikuMeshes[ 2 ] );
	m_snowMiku[ 2 ]->ReplaceMaterial( snowMikuMaterial2 );
	m_snowMiku[ 2 ]->SetModelMatrix( snowMikuMatrix );
	m_snowMiku[ 3 ] = new Renderable();
	m_snowMiku[ 3 ]->SetMesh( snowMikuMeshes[ 3 ] );
	m_snowMiku[ 3 ]->ReplaceMaterial( snowMikuMaterial1, false );
	m_snowMiku[ 3 ]->SetModelMatrix( snowMikuMatrix );

	m_galleryItems[ 0 ] = m_ship;
	m_galleryItems[ 1 ] = m_box;
	m_galleryItems[ 2 ] = m_snowMiku[ 0 ];
	m_galleryItems[ 3 ] = m_snowMiku[ 1 ];
	m_galleryItems[ 4 ] = m_snowMiku[ 2 ];
	m_galleryItems[ 5 ] = m_snowMiku[ 3 ];
}

void GalleryScene::SetupVRObjects()
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

void GalleryScene::SetupVRScene()
{
	m_scene = new Scene();
	m_scene->AddCamera( OVRContext::GetInstance()->GetHeadset()->GetLeftCamera() );
	m_scene->AddCamera( OVRContext::GetInstance()->GetHeadset()->GetRightCamera() );
	m_scene->AddLight( m_directionalLight );
	m_scene->AddLight( m_spotLight );
	m_scene->AddLight( m_flashlight );
	m_scene->AddRenderable( m_sphere );
	m_scene->AddRenderable( m_box );
	m_scene->AddRenderable( m_snowMiku[ 0 ] );
	m_scene->AddRenderable( m_snowMiku[ 1 ] );
	m_scene->AddRenderable( m_snowMiku[ 2 ] );
	m_scene->AddRenderable( m_snowMiku[ 3 ] );
	m_scene->AddRenderable( m_ship );
	m_scene->AddRenderable( m_grid );
	m_scene->AddRenderable( m_leftHand );
	m_scene->AddRenderable( m_rightHand );
}

void GalleryScene::Update()
{
	UpdateHandSpatialInformation();

	if ( !IsDevConsoleOpen() )
	{
		HandleKeyboardInput();
		HandleOVRInput();
	}

	UpdateSpotLightTransform();
	UpdateAudioSource();
	UpdateHUDText();
}

void GalleryScene::HandleKeyboardInput()
{

}


void GalleryScene::HandleOVRInput()
{
	HandleOVRInputForSceneTransition();

	// Left controller
	HandleOVRInputForLightRenderSettings();
	HandleOVRInputForFlashlightAdjustment();
	HandleOVRInputForVolumeAdjustment();

	// Right controller
	HandleOVRInputForModelSettings();
	HandleOVRInputForModelLocalRotation();
	HandleOVRInputForModelTranslation();

	TryVibrateControllers();
}

void GalleryScene::HandleOVRInputForSceneTransition()
{
	if ( OVRContext::GetInputSystem()->WasButtonJustPressed( OVRButton::OVR_BUTTON_THUMB_L ) )
	{
		g_theGame->SetNextScene( OVRZooScene::SCENE_MENU );
	}
	if ( OVRContext::GetInputSystem()->WasButtonJustPressed( OVRButton::OVR_BUTTON_THUMB_R ) )
	{
		g_theGame->SetNextScene( OVRZooScene::SCENE_MODEL );
	}
}

void GalleryScene::HandleOVRInputForModelSettings()
{
	if ( OVRContext::GetInputSystem()->WasButtonJustPressed( OVRButton::OVR_BUTTON_A ) )
	{
		// TODO: Remap
	}
	if ( OVRContext::GetInputSystem()->WasButtonJustPressed( OVRButton::OVR_BUTTON_B ) )
	{
		// TODO: Remap
	}
}

void GalleryScene::HandleOVRInputForLightRenderSettings()
{
	if ( OVRContext::GetInputSystem()->WasButtonJustPressed( OVR_BUTTON_X ) )
	{
		if ( !OVRContext::GetHeadset()->IsMSAAEnabled() )
		{
			OVRContext::GetHeadset()->EnableMSAA( 2U );
		}
		else if ( OVRContext::GetHeadset()->GetMSAANumSamples() == 2U )
		{
			OVRContext::GetHeadset()->EnableMSAA( 4U );
		}
		else if ( OVRContext::GetHeadset()->GetMSAANumSamples() == 4U )
		{
			OVRContext::GetHeadset()->DisableMSAA();
		}
	}
	if ( OVRContext::GetInputSystem()->WasButtonJustPressed( OVR_BUTTON_Y ) )
	{
		if ( OVRContext::GetHeadset()->IsBloomEnabled() )
		{
			OVRContext::GetHeadset()->SetBloomEnabled( false );
		}
		else
		{
			OVRContext::GetHeadset()->SetBloomEnabled( true );
		}
	}
}

void GalleryScene::HandleOVRInputForFlashlightAdjustment()
{
	Vector2 leftStickPositionCartesian = OVRContext::GetInputSystem()->GetJoystickPositionCartesian( OVRControllerType::OVR_TOUCH_CONTROLLER_LEFT );
	if ( leftStickPositionCartesian != Vector2::ZERO )
	{
		float intensityDelta = g_gameConfigBlackboard.GetValue( "flashlightIntensityDeltaPerSecond", 0.25f );
		m_flashlight->m_lightUBO.m_colorAndIntensity.w += leftStickPositionCartesian.y * intensityDelta * GetMasterDeltaSecondsF();
		m_flashlight->m_lightUBO.m_colorAndIntensity.w = ClampFloat( m_flashlight->m_lightUBO.m_colorAndIntensity.w, 0.0f, g_gameConfigBlackboard.GetValue( "flashlightMaxIntensity", 50.0f ) );
	}
}

void GalleryScene::HandleOVRInputForModelLocalRotation()
{
	Vector2 rightStickPositionCartesian = OVRContext::GetInputSystem()->GetJoystickPositionCartesian( OVRControllerType::OVR_TOUCH_CONTROLLER_RIGHT );
	if ( rightStickPositionCartesian != Vector2::ZERO )
	{
		Vector3 eulerDelta = Vector3(
			( rightStickPositionCartesian.y * g_gameConfigBlackboard.GetValue( "galleryItemMaxRotationDegreesPerSecond", 30.0f ) * GetMasterDeltaSecondsF() ),
			( -rightStickPositionCartesian.x * g_gameConfigBlackboard.GetValue( "galleryItemMaxRotationDegreesPerSecond", 30.0f ) * GetMasterDeltaSecondsF() ),
			0.0f
		);

		Renderable* renderableInSight = nullptr;
		float strongestDotProduct = 0.0f;
		Matrix44 modelMatrixToChange;

		for ( Renderable* renderable : m_galleryItems )
		{
			Matrix44 modelMatrix = renderable->GetModelMatrix();
			Vector3 displacement = modelMatrix.GetTranslation() - OVRContext::GetHeadset()->GetPosition();

			Transform rightControllerTransform = OVRContext::GetInputSystem()->GetControllerTransform( OVRControllerType::OVR_TOUCH_CONTROLLER_RIGHT );
			Matrix44 rightControllerModelMatrix = rightControllerTransform.GetAsMatrixWorld();
			Vector3 rightControllerForward = rightControllerModelMatrix.GetKBasis();

			float dotProduct = DotProduct( displacement, rightControllerForward );
			if ( dotProduct > strongestDotProduct )
			{
				strongestDotProduct = dotProduct;
				modelMatrixToChange = modelMatrix;
				renderableInSight = renderable;
			}
		}

		if ( renderableInSight == nullptr )
		{
			return;
		}

		Transform itemTransform;
		itemTransform.SetLocalFromMatrix( modelMatrixToChange );
		itemTransform.Rotate( eulerDelta );
		modelMatrixToChange = itemTransform.GetAsMatrixWorld();

		if (
			( renderableInSight == m_snowMiku[ 0 ] ) ||
			( renderableInSight == m_snowMiku[ 1 ] ) ||
			( renderableInSight == m_snowMiku[ 2 ] ) ||
			( renderableInSight == m_snowMiku[ 3 ] )
			)
		{
			m_snowMiku[ 0 ]->SetModelMatrix( modelMatrixToChange );
			m_snowMiku[ 1 ]->SetModelMatrix( modelMatrixToChange );
			m_snowMiku[ 2 ]->SetModelMatrix( modelMatrixToChange );
			m_snowMiku[ 3 ]->SetModelMatrix( modelMatrixToChange );
		}
		else
		{
			renderableInSight->SetModelMatrix( modelMatrixToChange );
		}
	}
}

void GalleryScene::HandleOVRInputForModelTranslation()
{
	Vector3 headsetRightVector = OVRContext::GetHeadset()->GetRight();

	float rightHandTriggerValue = OVRContext::GetInputSystem()->GetHandTriggerValue( OVRControllerType::OVR_TOUCH_CONTROLLER_RIGHT );
	Vector3 rightHandLinearVelocity = OVRContext::GetInputSystem()->GetContollerLinearVelocity( OVRControllerType::OVR_TOUCH_CONTROLLER_RIGHT );
	float rightHandSpeed = rightHandLinearVelocity.NormalizeAndGetLength();
	float rightHandDotAlongRight = DotProduct( rightHandLinearVelocity, headsetRightVector );

	if ( IsFloatGreaterThanOrEqualTo( rightHandTriggerValue, g_gameConfigBlackboard.GetValue( "gallerySlotChangeTriggerThreshold", 0.5f ) ) )
	{
		float rotationMagnitude = -rightHandDotAlongRight * rightHandSpeed * g_gameConfigBlackboard.GetValue( "galleryItemMaxTurnDegreesPerSecond", 30.0f ) * GetMasterDeltaSecondsF();
		RotateGalleryItems( rotationMagnitude );
	}
}

void GalleryScene::TryVibrateControllers()
{
	float rightHandTriggerValue = OVRContext::GetInputSystem()->GetHandTriggerValue( OVRControllerType::OVR_TOUCH_CONTROLLER_RIGHT );
	bool isGalleryRotating = IsFloatGreaterThanOrEqualTo( rightHandTriggerValue, g_gameConfigBlackboard.GetValue( "gallerySlotChangeTriggerThreshold", 0.5f ) );

	if ( isGalleryRotating )
	{
		OVRContext::GetInputSystem()->SetControllerVibration( OVRControllerType::OVR_TOUCH_CONTROLLER_RIGHT, 0.0f, Max( rightHandTriggerValue, 1.0f ) );
	}
	else
	{
		OVRContext::GetInputSystem()->StopControllerVibration( OVRControllerType::OVR_TOUCH_CONTROLLER_RIGHT );
	}
}

void GalleryScene::RotateGalleryItems( float angleDegrees )
{
	for ( Renderable* renderable : m_galleryItems )
	{
		Matrix44 modelMatrix = renderable->GetModelMatrix();
		modelMatrix.RotateAbout( angleDegrees, 0.0f, Vector3::ZERO );
		renderable->SetModelMatrix( modelMatrix );
	}
}

void GalleryScene::HandleOVRInputForVolumeAdjustment()
{
	float volumeAdjustment = g_gameConfigBlackboard.GetValue( "volumeAdjustmentMaxPerSecond", 1.0f );
	volumeAdjustment *= GetMasterDeltaSecondsF();

	float volumeDecrement = OVRContext::GetInputSystem()->GetIndexTriggerValue( OVRControllerType::OVR_TOUCH_CONTROLLER_LEFT );	// Left index trigger decreases volume
	volumeDecrement *= volumeAdjustment;
	OVRContext::GetHeadset()->IncreaseMasterVolume( -volumeDecrement );

	float volumeIncrement = OVRContext::GetInputSystem()->GetHandTriggerValue( OVRControllerType::OVR_TOUCH_CONTROLLER_LEFT );	// Left hand trigger increases volume
	volumeIncrement *= volumeAdjustment;
	OVRContext::GetHeadset()->IncreaseMasterVolume( volumeIncrement );
}

void GalleryScene::UpdateHandSpatialInformation()
{
	Matrix44 leftHandMatrix = m_leftHandTransform.GetAsMatrixWorld();
	m_leftHand->SetModelMatrix( leftHandMatrix );
	Matrix44 rightHandMatrix = m_rightHandTransform.GetAsMatrixWorld();
	rightHandMatrix.SetIBasis( rightHandMatrix.GetIBasis() * -1.0f );
	m_rightHand->SetModelMatrix( rightHandMatrix );
}

void GalleryScene::UpdateSpotLightTransform()
{
	static float s_spotLightRotationDegrees = 0.0f;

	if ( m_isSpotLightMoving )
	{
		Vector3 spotLightPosition = m_spotLight->m_transform.m_position;
		spotLightPosition -= GetSlotPositionCartesian( 0 );
		spotLightPosition.ConvertToPolar();
		spotLightPosition.y += g_gameConfigBlackboard.GetValue( "spotLightRotationDegreesPerSecond", 30.0f ) * GetMasterDeltaSecondsF();
		spotLightPosition.ConvertToCartesian();
		spotLightPosition += GetSlotPositionCartesian( 0 );
		Matrix44 lookAtCenter = Matrix44::LookAt( spotLightPosition, GetSlotPositionCartesian( 0 ) );
		m_spotLight->m_transform.SetLocalFromMatrix( lookAtCenter );
		m_spotLight->m_transform.m_position = spotLightPosition;
		m_sphere->SetModelMatrix( Matrix44::MakeTranslation( spotLightPosition ) );
	}
}

void GalleryScene::UpdateAudioSource()
{
	Matrix44 shipMatrix =  m_ship->GetModelMatrix();
	g_audioSystem->SetSoundPlayback3DAttributes( m_shipAudioPlaybackID, m_ship->GetModelMatrix().GetTranslation(), Vector3::ZERO );
}

void GalleryScene::UpdateHUDText()
{
	Vector2 halfScreenDimensions = ConvertIntVector2ToVector2( OVRContext::GetHeadset()->GetResolution() );
	Material* textMaterial = g_renderer->CreateOrGetMaterial( "UI" );
	textMaterial->SetTextureAndSampler( 0U, g_renderer->CreateOrGetBitmapFont( DEFAULT_FONT_NAME )->GetTexture(), g_renderer->GetDefaultSampler() );
	std::string instructionText;
	Vector2 textMins;
	MeshBuilder textBuilder;
	Rgba textColor;

	instructionText = "X - Toggle MSAA ";
	instructionText += ( !OVRContext::GetHeadset()->IsMSAAEnabled() )? "OFF" : ( ( OVRContext::GetHeadset()->GetMSAANumSamples() == 4 )? "4X" : "2X" );
	textMins = Vector2( ( -0.325f * halfScreenDimensions.x ), ( 0.0f * halfScreenDimensions.y ) );
	textColor = ( OVRContext::GetHeadset()->IsMSAAEnabled() )? Rgba::GREEN : Rgba::RED;
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

	instructionText = "Y - Toggle Bloom ";
	instructionText += ( OVRContext::GetHeadset()->IsBloomEnabled() )? "ON" : "OFF";
	textMins = Vector2( ( -0.325f * halfScreenDimensions.x ), ( -0.005f * halfScreenDimensions.y ) );
	textColor = ( OVRContext::GetHeadset()->IsBloomEnabled() )? Rgba::GREEN : Rgba::RED;
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

	instructionText = "Stick button - Go to Menu";
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

	instructionText = "Index Trigger - Decrease Volume ";
	instructionText += std::to_string( OVRContext::GetHeadset()->GetMasterVolume() );
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

	instructionText = "Hand Trigger - Increase Volume ";
	instructionText += std::to_string( OVRContext::GetHeadset()->GetMasterVolume() );
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

	instructionText = "Stick - Rotate Model";
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

	instructionText = "Hand Trigger - Grab Gallery";
	textMins = Vector2( ( -0.325f * halfScreenDimensions.x ), ( -0.03f * halfScreenDimensions.y ) );
	if ( OVRContext::GetInputSystem()->GetHandTriggerValue( OVRControllerType::OVR_TOUCH_CONTROLLER_RIGHT ) > g_gameConfigBlackboard.GetValue( "gallerySlotChangeTriggerThreshold", 0.5f ) )
	{
		textColor = Rgba::GREEN;
		instructionText = "Move Controller - Rotate Gallery";
	}
	else
	{
		textColor = Rgba::RED;
	}

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

void GalleryScene::Render() const
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
}

Vector3 GalleryScene::GetSlotPositionCartesian( int index ) const
{
	Vector2 slotPosition2D = m_gallerySlotsPolar[ index ];
	slotPosition2D.ConvertToCartestian();
	slotPosition2D += m_galleryOrigin;
	Vector3 slotPosition = Vector3( slotPosition2D.x, m_galleryItemHeight, slotPosition2D.y );
	return slotPosition;
}
