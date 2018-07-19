#include "Game/ModelBuilderScene.hpp"
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

Vector3 PlaneSurfacePatch( float u, float v )
{
	return Vector3( u, 0., v );
}

ModelBuilderScene::ModelBuilderScene()
{
	SetupWorldObjects();
	SetupLights();
	SetupVRObjects();
	SetupMutableModel();
	SetupVRScene();
	DebugRenderSet3DCamera( OVRContext::GetHeadset()->GetLeftCamera() );
	DebugRenderSet2DCamera( OVRContext::GetHeadset()->GetLeftHUDCamera() );
}

ModelBuilderScene::~ModelBuilderScene()
{
	m_flashlight->m_transform.Reparent( nullptr );
	m_leftHandTransform.Reparent( nullptr );
	m_rightHandTransform.Reparent( nullptr );

	delete m_scene;
	m_scene = nullptr;

	delete m_flashlight;
	m_flashlight = nullptr;

	delete m_directionalLight;
	m_directionalLight = nullptr;

	delete m_mutableModel;
	m_mutableModel = nullptr;

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

void ModelBuilderScene::SetupLights()
{
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

void ModelBuilderScene::SetupWorldObjects()
{
	MeshBuilder gridMeshBuilder = g_renderer->MakeGridMesh( Vector3( 0.0f, -g_gameConfigBlackboard.GetValue( "vrCameraHeightm", 1.8f ), 0.0f ), AABB2( -5.0f, -5.0f, 5.0f, 5.0f ), Vector3::FORWARD, Vector3::RIGHT, IntVector2( 10, 10 ), Rgba::WHITE );
	m_grid = new Renderable(	gridMeshBuilder.CreateMesh(),
		g_renderer->CreateOrGetMaterial( "Grid" )->GetInstance()
	);
}

void ModelBuilderScene::SetupVRObjects()
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

	m_handHalfDimensions = handDimensions * 0.5f;
	m_rightHandBounds = OBB3(
		Vector3::ZERO,
		( m_handHalfDimensions.x * Vector3::RIGHT ),
		( m_handHalfDimensions.y * Vector3::UP ),
		( m_handHalfDimensions.z * Vector3::FORWARD )
	);
}

void ModelBuilderScene::SetupMutableModel()
{
	/*
	m_mutableMeshBuilder = g_renderer->MakeTexturedCubeMesh(
	Vector3::ZERO,
	( 0.25f * Vector3::ONE ),
	*g_renderer->GetDefaultTexture(),
	Rgba::WHITE
	);
	*/
	m_mutableMeshBuilder.Begin( DrawPrimitiveType::TRIANGLES );
	m_mutableMeshBuilder.SetColor( Rgba::WHITE );
	m_mutableMeshBuilder.AddSurfacePatch(
		PlaneSurfacePatch,
		FloatRange( -1.0f, 1.0f ),
		FloatRange( -1.0f, 1.0f ),
		IntVector2( 50, 50 ),
		Vector3( 0.0f, -1.0f, 0.0f ),
		1.0f
	);
	m_mutableMeshBuilder.End();

	/*
	m_mutableMeshBuilder = g_renderer->MakeUVSphereMesh(
		Vector3::ZERO,
		0.25f,
		25,
		25,
		Rgba::WHITE
	);
	*/
	m_mutableModel = new Renderable(
		m_mutableMeshBuilder.CreateMesh(),
		g_renderer->CreateOrGetMaterial( "Mutable" )->GetInstance()
	);
}

void ModelBuilderScene::SetupVRScene()
{
	m_scene = new Scene();
	m_scene->AddCamera( OVRContext::GetInstance()->GetHeadset()->GetLeftCamera() );
	m_scene->AddCamera( OVRContext::GetInstance()->GetHeadset()->GetRightCamera() );
	m_scene->AddLight( m_directionalLight );
	m_scene->AddLight( m_flashlight );
	m_scene->AddRenderable( m_grid );
	m_scene->AddRenderable( m_leftHand );
	m_scene->AddRenderable( m_rightHand );
	m_scene->AddRenderable( m_mutableModel );
}

void ModelBuilderScene::Update()
{
	UpdateHandSpatialInformation();

	if ( !IsDevConsoleOpen() )
	{
		HandleKeyboardInput();
		HandleOVRInput();
	}

	MoveHeldVerticesWithHand();
	UpdateHUDText();
}

void ModelBuilderScene::HandleKeyboardInput()
{

}


void ModelBuilderScene::HandleOVRInput()
{
	HandleOVRInputForSceneTransition();

	// Left controller
	HandleOVRInputForLightRenderSettings();
	HandleOVRInputForFlashlightAdjustment();

	// Right controller
	HandleOVRInputForModelBuilderVertexMovement();
	HandleOVRInputForModelSettings();
	HandleOVRInputForModelLocalRotation();
	HandleOVRInputForModelTranslation();
	HandleOVRInputForLightingRecomputation();

	TryVibrateControllers();
}

void ModelBuilderScene::HandleOVRInputForSceneTransition()
{
	if ( OVRContext::GetInputSystem()->WasButtonJustPressed( OVRButton::OVR_BUTTON_THUMB_L ) )
	{
		g_theGame->SetNextScene( OVRZooScene::SCENE_GALLERY );
	}
	if ( OVRContext::GetInputSystem()->WasButtonJustPressed( OVRButton::OVR_BUTTON_THUMB_R ) )
	{
		g_theGame->SetNextScene( OVRZooScene::SCENE_MENU );
	}
}

void ModelBuilderScene::HandleOVRInputForModelBuilderVertexMovement()
{
	float vertexMoveTriggerValue = OVRContext::GetInputSystem()->GetIndexTriggerValue( OVRControllerType::OVR_TOUCH_CONTROLLER_RIGHT );
	if ( vertexMoveTriggerValue > g_gameConfigBlackboard.GetValue( "vertexMoveTriggerThreshold", 0.5f ) )
	{
		Matrix44 modelMatrix = m_mutableModel->GetModelMatrix();
		for ( unsigned int vertIndex = 0; vertIndex < m_mutableMeshBuilder.GetVertexCount(); vertIndex++ )
		{
			VertexBuilder* vertRef = m_mutableMeshBuilder.GetVertexAsReference( vertIndex );
			Vector3 transformedPosition = modelMatrix.TransformPosition( vertRef->m_position );
			if ( m_numMovingVertices < MODEL_BUILDER_MAX_MOVABLE_VERTS && m_rightHandBounds.IsPointInside( transformedPosition ) )
			{
				AddMovingVertex( vertRef );
			}
		}
	}
	else
	{
		ClearMovingVertices();
	}
}

void ModelBuilderScene::HandleOVRInputForModelSettings()
{
	if ( OVRContext::GetInputSystem()->WasButtonJustPressed( OVRButton::OVR_BUTTON_A ) )
	{
		// TODO: Remap
	}
	if ( OVRContext::GetInputSystem()->WasButtonJustPressed( OVRButton::OVR_BUTTON_B ) )
	{
		m_mutableModelRenderWireframe = !m_mutableModelRenderWireframe;
		if ( m_mutableModelRenderWireframe )
		{
			m_mutableModel->ReplaceMaterial( g_renderer->CreateOrGetMaterial( "MutableWireframe" )->GetInstance() );
		}
		else
		{
			m_mutableModel->ReplaceMaterial( g_renderer->CreateOrGetMaterial( "Mutable" )->GetInstance() );
		}
	}
}

void ModelBuilderScene::AddMovingVertex( VertexBuilder* vertexToMove )
{
	if ( m_numMovingVertices < MODEL_BUILDER_MAX_MOVABLE_VERTS )
	{
		Matrix44 modelMatrix = m_mutableModel->GetModelMatrix();
		Vector3 vertexWorldPosition = modelMatrix.TransformPosition( vertexToMove->m_position );

		Transform* ovrRightHandTransform = OVRContext::GetInputSystem()->GetControllerTransformReference( OVRControllerType::OVR_TOUCH_CONTROLLER_RIGHT );
		Matrix44 ovrRightHandMatrix = ovrRightHandTransform->GetAsMatrixLocal();
		ovrRightHandMatrix.Invert();
		Vector3 ovrHandLocalPosition = ovrRightHandMatrix.TransformPosition( vertexWorldPosition );

		Vector3 rightHandPosition = m_rightHandTransform.GetLocalPosition();
		Vector3 rightHandLocalPosition = ovrHandLocalPosition - rightHandPosition;
		m_movingVertexTransforms[ m_numMovingVertices ].SetLocalPosition( rightHandLocalPosition );
		m_movingVertexTransforms[ m_numMovingVertices ].Reparent( &m_rightHandTransform );
		m_movingVertexRefs[ m_numMovingVertices ] = vertexToMove;
		m_numMovingVertices++;
	}
}

void ModelBuilderScene::ClearMovingVertices()
{
	while ( m_numMovingVertices > 0 )
	{
		m_numMovingVertices--;
		m_movingVertexRefs[ m_numMovingVertices ] = nullptr;
		m_movingVertexTransforms[ m_numMovingVertices ] = Transform();
		m_movingVertexTransforms[ m_numMovingVertices ].Reparent( nullptr );
	}
}

void ModelBuilderScene::HandleOVRInputForLightRenderSettings()
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

void ModelBuilderScene::HandleOVRInputForFlashlightAdjustment()
{
	Vector2 leftStickPositionCartesian = OVRContext::GetInputSystem()->GetJoystickPositionCartesian( OVRControllerType::OVR_TOUCH_CONTROLLER_LEFT );
	if ( leftStickPositionCartesian != Vector2::ZERO )
	{
		float intensityDelta = g_gameConfigBlackboard.GetValue( "flashlightIntensityDeltaPerSecond", 0.25f );
		m_flashlight->m_lightUBO.m_colorAndIntensity.w += leftStickPositionCartesian.y * intensityDelta * GetMasterDeltaSecondsF();
		m_flashlight->m_lightUBO.m_colorAndIntensity.w = ClampFloat( m_flashlight->m_lightUBO.m_colorAndIntensity.w, 0.0f, g_gameConfigBlackboard.GetValue( "flashlightMaxIntensity", 50.0f ) );
	}
}

void ModelBuilderScene::HandleOVRInputForModelLocalRotation()
{
	Vector2 rightStickPositionCartesian = OVRContext::GetInputSystem()->GetJoystickPositionCartesian( OVRControllerType::OVR_TOUCH_CONTROLLER_RIGHT );
	if ( rightStickPositionCartesian != Vector2::ZERO )
	{
		Vector3 eulerDelta = Vector3(
			0.0f, // ( rightStickPositionCartesian.y * g_gameConfigBlackboard.GetValue( "galleryItemMaxRotationDegreesPerSecond", 30.0f ) * GetMasterDeltaSecondsF() ),
			( -rightStickPositionCartesian.x * g_gameConfigBlackboard.GetValue( "galleryItemMaxRotationDegreesPerSecond", 30.0f ) * GetMasterDeltaSecondsF() ),
			0.0f
		);

		RotateModel( eulerDelta );
	}
}

void ModelBuilderScene::RotateModel( const Vector3& rotationEuler )
{
	Matrix44 modelMatrix = m_mutableModel->GetModelMatrix();
	Transform modelTransform;
	modelTransform.SetLocalFromMatrix( modelMatrix );
	modelTransform.Rotate( rotationEuler );
	Matrix44 rotatedMatrix = modelTransform.GetAsMatrixLocal();
	m_mutableModel->SetModelMatrix( rotatedMatrix );
}

void ModelBuilderScene::HandleOVRInputForModelTranslation()
{
	Vector3 headsetRightVector = OVRContext::GetHeadset()->GetRight();

	float rightHandTriggerValue = OVRContext::GetInputSystem()->GetHandTriggerValue( OVRControllerType::OVR_TOUCH_CONTROLLER_RIGHT );
	Vector3 rightHandLinearVelocity = OVRContext::GetInputSystem()->GetContollerLinearVelocity( OVRControllerType::OVR_TOUCH_CONTROLLER_RIGHT );
	float rightHandSpeed = rightHandLinearVelocity.NormalizeAndGetLength();
	float rightHandDotAlongRight = DotProduct( rightHandLinearVelocity, headsetRightVector );

	if ( IsFloatGreaterThanOrEqualTo( rightHandTriggerValue, g_gameConfigBlackboard.GetValue( "gallerySlotChangeTriggerThreshold", 0.5f ) ) )
	{
		Vector3 headsetUpVector = OVRContext::GetHeadset()->GetUp();
		float rightHandDotAlongUp = DotProduct( rightHandLinearVelocity, headsetUpVector );
		Vector3 headsetForwardVector = OVRContext::GetHeadset()->GetForward();
		float rightHandDotAlongForward = DotProduct( rightHandLinearVelocity, headsetForwardVector );

		Vector3 translation =
			( rightHandSpeed * g_gameConfigBlackboard.GetValue( "modelMaxTranslationPerSecond", 0.5f ) * GetMasterDeltaSecondsF() ) *								// Scalar component
			( ( rightHandDotAlongRight * headsetRightVector ) + ( rightHandDotAlongUp * headsetUpVector ) + ( rightHandDotAlongForward * headsetForwardVector ) )	// Vector component
			;
		TranslateModel( translation );
	}
}

void ModelBuilderScene::HandleOVRInputForLightingRecomputation()
{
	if ( OVRContext::GetInputSystem()->WasButtonJustPressed( OVRButton::OVR_BUTTON_A ) )
	{
		m_mutableMeshBuilder.RecomputeNormals();
		m_mutableMeshBuilder.RecomputeTangents();
		Mesh* movedMesh = m_mutableMeshBuilder.CreateMesh();
		m_mutableModel->ReplaceMesh( movedMesh );
	}
}

void ModelBuilderScene::TranslateModel( const Vector3& translation )
{
	Matrix44 modelMatrix = m_mutableModel->GetModelMatrix();
	Transform modelTransform;
	modelTransform.SetLocalFromMatrix( modelMatrix );
	modelTransform.Translate( translation );
	Matrix44 rotatedMatrix = modelTransform.GetAsMatrixLocal();
	m_mutableModel->SetModelMatrix( rotatedMatrix );
}

void ModelBuilderScene::TryVibrateControllers()
{
	float rightHandTriggerValue = OVRContext::GetInputSystem()->GetHandTriggerValue( OVRControllerType::OVR_TOUCH_CONTROLLER_RIGHT );
	bool isGalleryRotating = IsFloatGreaterThanOrEqualTo( rightHandTriggerValue, g_gameConfigBlackboard.GetValue( "gallerySlotChangeTriggerThreshold", 0.5f ) );

	bool areVerticesBeingMoved = m_numMovingVertices > 0;

	if ( isGalleryRotating || areVerticesBeingMoved )
	{
		OVRContext::GetInputSystem()->SetControllerVibration( OVRControllerType::OVR_TOUCH_CONTROLLER_RIGHT, 0.0f, Max( rightHandTriggerValue, 1.0f ) );
	}
	else
	{
		OVRContext::GetInputSystem()->StopControllerVibration( OVRControllerType::OVR_TOUCH_CONTROLLER_RIGHT );
	}
}

void ModelBuilderScene::UpdateHandSpatialInformation()
{
	Matrix44 leftHandMatrix = m_leftHandTransform.GetAsMatrixWorld();
	m_leftHand->SetModelMatrix( leftHandMatrix );
	Matrix44 rightHandMatrix = m_rightHandTransform.GetAsMatrixWorld();
	rightHandMatrix.SetIBasis( rightHandMatrix.GetIBasis() * -1.0f );
	m_rightHand->SetModelMatrix( rightHandMatrix );

	m_rightHandBounds.m_center = rightHandMatrix.GetTranslation();
	m_rightHandBounds.m_right = -1.0f * m_handHalfDimensions.x * rightHandMatrix.GetIBasis();	// The -1.0f is for the inverted basis vectors of the right hand
	m_rightHandBounds.m_up = m_handHalfDimensions.y * rightHandMatrix.GetJBasis();
	m_rightHandBounds.m_forward = m_handHalfDimensions.z * rightHandMatrix.GetKBasis();
}

void ModelBuilderScene::MoveHeldVerticesWithHand()
{
	for ( int movingVertIndex = 0; movingVertIndex < m_numMovingVertices; movingVertIndex++ )
	{
		Vector3 vertLocalPositionHandSpace = m_movingVertexTransforms[ movingVertIndex ].GetLocalPosition();
		vertLocalPositionHandSpace.x = Abs( vertLocalPositionHandSpace.x );
		vertLocalPositionHandSpace.y = Abs( vertLocalPositionHandSpace.y );
		vertLocalPositionHandSpace.z = Abs( vertLocalPositionHandSpace.z );
		vertLocalPositionHandSpace.x = RangeMapFloat( vertLocalPositionHandSpace.x, 0.0f, m_handHalfDimensions.x, 1.0f, 0.0f );
		vertLocalPositionHandSpace.y = RangeMapFloat( vertLocalPositionHandSpace.y, 0.0f, m_handHalfDimensions.y, 1.0f, 0.0f );
		vertLocalPositionHandSpace.z = RangeMapFloat( vertLocalPositionHandSpace.z, 0.0f, m_handHalfDimensions.z, 1.0f, 0.0f );

		float featheringFactor = ( vertLocalPositionHandSpace.x + vertLocalPositionHandSpace.y + vertLocalPositionHandSpace.z ) * 0.33f;
		int smoothing = 3;	// Sweet spot!
		while ( smoothing > 0 )
		{
			featheringFactor *= featheringFactor;
			smoothing--;
		}

		Matrix44 modelMatrix = m_mutableModel->GetModelMatrix();
		Matrix44 modelMatrixInverse = modelMatrix.GetInverse();

		Vector3 maxNewWorldPosition = m_movingVertexTransforms[ movingVertIndex ].GetWorldPosition();
		Vector3 currentWorldPosition = modelMatrix.TransformPosition( m_movingVertexRefs[ movingVertIndex ]->m_position );
		Vector3 displacement = maxNewWorldPosition - currentWorldPosition;
		currentWorldPosition += featheringFactor * displacement;
		m_movingVertexRefs[ movingVertIndex ]->m_position = modelMatrixInverse.TransformPosition( currentWorldPosition );	// Get back into local space - the model matrix may not be identity
	}

	if ( m_numMovingVertices > 0 )
	{
		Mesh* modifiedMesh = m_mutableMeshBuilder.CreateMesh();
		m_mutableModel->ReplaceMesh( modifiedMesh );
	}
}

void ModelBuilderScene::UpdateHUDText()
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

	instructionText = "A - Recompute Lighting (FPS Drop)";
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

	instructionText = "B - Toggle Model Mode ";
	instructionText += ( m_mutableModelRenderWireframe )? "Wireframe" : "Solid";
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

	instructionText = "Index Trigger - Grab Model Vertices";
	textMins = Vector2( ( -0.325f * halfScreenDimensions.x ), ( -0.025f * halfScreenDimensions.y ) );
	if (
		m_numMovingVertices > 0 &&
		OVRContext::GetInputSystem()->GetIndexTriggerValue( OVRControllerType::OVR_TOUCH_CONTROLLER_RIGHT ) > g_gameConfigBlackboard.GetValue( "gallerySlotChangeTriggerThreshold", 0.5f )
		)
	{
		textColor = Rgba::GREEN;
		instructionText = "Move Controller - Move Grabbed Vertices";
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

	instructionText = "Hand Trigger - Grab Model";
	textMins = Vector2( ( -0.325f * halfScreenDimensions.x ), ( -0.03f * halfScreenDimensions.y ) );
	if ( OVRContext::GetInputSystem()->GetHandTriggerValue( OVRControllerType::OVR_TOUCH_CONTROLLER_RIGHT ) > g_gameConfigBlackboard.GetValue( "gallerySlotChangeTriggerThreshold", 0.5f ) )
	{
		textColor = Rgba::GREEN;
		instructionText = "Move Controller - Translate Model";
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

void ModelBuilderScene::Render() const
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
