#include "Game/GameCommon.hpp"
#include "Game/Ship.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Math/Transform.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/ParticleEmitter.hpp"
#include "Engine/Renderer/Renderable.hpp"
#include "Engine/Renderer/Renderer.hpp"

Ship::Ship()
{
	m_transform = new Transform;
	InitializeMesh();
	InitializeMaterial();
	InitializeParticleSystems();
}

Ship::Ship( const Vector3& startPosition, const Vector3& startEuler /* = Vector3::ZERO */, const Vector3& startScale /* = Vector3::ONE */ )
{
	m_transform = new Transform( startPosition, startEuler, startScale );
	InitializeMesh();
	InitializeMaterial();
	InitializeParticleSystems();
	m_renderable->SetModelMatrix( m_transform->GetAsMatrixWorld() );
}

Ship::~Ship()
{
	delete m_renderable;
	m_renderable = nullptr;

	delete m_transform;
	m_transform = nullptr;

	delete m_leftThruster;
	delete m_rightThruster;
}

void Ship::InitializeMesh()
{
	m_renderable = new Renderable();
	m_renderable->SetMesh( MeshBuilder::FromFileOBJ( SCI_FI_FIGHTER_MODEL_FILEPATH )[ 0 ] );
}

void Ship::InitializeParticleSystems()
{
	m_leftThruster = new ParticleEmitter( Vector3::ZERO, Vector3( 0.0f, 0.0f, 0.0f ), PARTICLE_EMITTER_IDLE_RATE, Renderer::GetInstance()->CreateOrGetTexture( PARTICLE_TEXTURE_FILEPATH ) );
	m_leftThruster->GetTransform()->Reparent( m_transform );
	m_leftThruster->GetTransform()->Translate( Vector3( -1.9f, 2.2f, -6.3f ) );
	m_leftThruster->m_forceOnParticles = Vector3( 0.0f, 0.0f, -0.1f );
	m_leftThruster->m_sizeRange = FloatRange( 0.05f, 0.5f );

	m_rightThruster = new ParticleEmitter( Vector3::ZERO, Vector3( 0.0f, 0.0f, 0.0f ), PARTICLE_EMITTER_IDLE_RATE, Renderer::GetInstance()->CreateOrGetTexture( PARTICLE_TEXTURE_FILEPATH ) );
	m_rightThruster->GetTransform()->Reparent( m_transform );
	m_rightThruster->GetTransform()->Translate( Vector3( 1.9f, 2.2f, -6.3f ) );
	m_rightThruster->m_forceOnParticles = Vector3( 0.0f, 0.0f, -0.1f );
	m_rightThruster->m_sizeRange = FloatRange( 0.05f, 0.5f );
}

void Ship::InitializeMaterial( bool haveMaterialsInitialized /* = false */, int renderModeIndex /* = 0 */ )
{
	Material* shipMaterial = GetMaterialForRenderMode( renderModeIndex )->GetInstance();
	shipMaterial->SetTextureAndSampler( 0, g_renderer->CreateOrGetMaterial( "Ship" )->GetTexture( 0 ), g_renderer->GetDefaultSampler() );
	shipMaterial->SetTextureAndSampler( 1, g_renderer->CreateOrGetMaterial( "Ship" )->GetTexture( 1 ), g_renderer->GetDefaultSampler() );
	if ( !haveMaterialsInitialized )
	{
		m_specularProperties = Vector2( g_renderer->CreateOrGetMaterial( "Ship" )->GetSpecularAmount(), g_renderer->CreateOrGetMaterial( "Ship" )->GetSpecularPower() );
	}
	shipMaterial->SetSpecularProperties( m_specularProperties.x, m_specularProperties.y );
	m_renderable->ReplaceMaterial( shipMaterial );
}

void Ship::Update()
{
	m_renderable->SetModelMatrix( m_transform->GetAsMatrixWorld() );

	HandleKeyboardInput();
	HandleMouseInput();
}

void Ship::UpdateParticleSystems( Camera* camera )
{
	m_leftThruster->Update( camera, GetMasterDeltaSecondsF() );
	m_rightThruster->Update( camera, GetMasterDeltaSecondsF() );
}

void Ship::HandleKeyboardInput()
{
	float translationSpeed = SHIP_TRANSLATION_PER_SECOND * GetMasterDeltaSecondsF();
	if ( g_inputSystem->IsKeyDown( InputSystem::KEYBOARD_W ) )
	{
		Translate( GetForward() * translationSpeed );
		m_leftThruster->m_spawnRate = PARTICLE_EMITTER_ACCELERATE_RATE;
		m_rightThruster->m_spawnRate = PARTICLE_EMITTER_ACCELERATE_RATE;
	}
	if ( g_inputSystem->WasKeyJustReleased( InputSystem::KEYBOARD_W ) )
	{
		m_leftThruster->m_spawnRate = PARTICLE_EMITTER_IDLE_RATE;
		m_rightThruster->m_spawnRate = PARTICLE_EMITTER_ACCELERATE_RATE;
	}
	if ( g_inputSystem->IsKeyDown( InputSystem::KEYBOARD_S ) )
	{
		Translate( GetForward() * translationSpeed * -1.0f );
	}
	if ( g_inputSystem->IsKeyDown( InputSystem::KEYBOARD_A ) )
	{
		Translate( GetRight() * translationSpeed * -1.0f );
	}
	if ( g_inputSystem->IsKeyDown( InputSystem::KEYBOARD_D ) )
	{
		Translate( GetRight() * translationSpeed );
	}
	if ( g_inputSystem->IsKeyDown( InputSystem::KEYBOARD_R ) )
	{
		Translate( GetUp() * translationSpeed );
	}
	if ( g_inputSystem->IsKeyDown( InputSystem::KEYBOARD_F ) )
	{
		Translate( GetUp() * translationSpeed * -1.0f );
	}

	float rotationSpeed = SHIP_ROTATION_PER_SECOND * GetMasterDeltaSecondsF();
	if ( g_inputSystem->IsKeyDown( InputSystem::KEYBOARD_LEFT_ARROW ) )
	{
		Rotate( Vector3::UP * rotationSpeed * -1.0f );
	}
	if ( g_inputSystem->IsKeyDown( InputSystem::KEYBOARD_RIGHT_ARROW ) )
	{
		Rotate( Vector3::UP * rotationSpeed );
	}
	if ( g_inputSystem->IsKeyDown( InputSystem::KEYBOARD_UP_ARROW ) )
	{
		Rotate( Vector3::RIGHT * rotationSpeed * -1.0f );
	}
	if ( g_inputSystem->IsKeyDown( InputSystem::KEYBOARD_DOWN_ARROW ) )
	{
		Rotate( Vector3::RIGHT * rotationSpeed );
	}

	if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_Z ) )
	{
		GetRenderable()->ToggleVisibility();
	}
}

void Ship::HandleMouseInput()
{
	Vector2 mouseDelta = g_inputSystem->GetMouseClientDelta().GetNormalized();
	if ( mouseDelta != Vector2::ZERO )
	{
		mouseDelta *= SHIP_ROTATION_PER_SECOND * GetMasterDeltaSecondsF();
		Rotate( Vector3( mouseDelta.y, mouseDelta.x, 0.0f ) );
	}
	if ( g_inputSystem->WasMouseButtonJustPressed( MouseButton::LEFT_MOUSE_BUTTON ) )
	{
		g_theGame->FireProjectile( m_transform->GetWorldPosition(), ( GetForward() * PROJECTILE_FIRE_SPEED ) );
	}
}

void Ship::Translate( const Vector3& translation )
{
	m_transform->Translate( translation );
}

void Ship::Rotate( const Vector3& rotationEuler )
{
	m_transform->Rotate( rotationEuler );
}

void Ship::SetSpecularProperties( float specularAmount, float specularPower )
{
	m_specularProperties = Vector2( specularAmount, specularPower );
	m_renderable->GetMaterial()->SetSpecularProperties( m_specularProperties.x, m_specularProperties.y );
}

ParticleEmitter* Ship::GetLeftThruster()
{
	return m_leftThruster;
}

ParticleEmitter* Ship::GetRightThruster()
{
	return m_rightThruster;
}

Renderable* Ship::GetRenderable()
{
	return m_renderable;
}

Transform* Ship::GetTransform()
{
	return m_transform;
}

Vector3 Ship::GetPosition() const
{
	return m_transform->GetWorldPosition();
}

Vector3 Ship::GetUp() const
{
	return m_transform->GetAsMatrixWorld().GetJBasis();
}

Vector3 Ship::GetRight() const
{
	return m_transform->GetAsMatrixWorld().GetIBasis();
}

Vector3 Ship::GetForward() const
{
	return m_transform->GetAsMatrixWorld().GetKBasis();
}
