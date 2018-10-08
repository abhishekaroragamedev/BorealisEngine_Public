#include "Game/GameCommon.hpp"
#include "Game/Projectile.hpp"
#include "Engine/Math/Transform.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/ParticleEmitter.hpp"
#include "Engine/Renderer/Renderable.hpp"

Projectile::Projectile( const Vector3& spawnPosition, const Vector3& velocity, float timeToLive /* = PROJECTILE_DEFAULT_TIME_TO_LIVE */ )
	:	m_velocity( velocity )
{
	m_stopWatch.SetTimer( timeToLive );
	m_renderable = new Renderable( g_renderer->MakeUVSphereMesh( Vector3::ZERO, PROJECTILE_RADIUS, 4, 4, Rgba::RED ).CreateMesh(), g_renderer->CreateOrGetMaterial( "Projectile" )->GetInstance() );
	m_transform = new Transform( spawnPosition, Vector3::ZERO, Vector3::ONE );

	m_light = new Light();
	m_light->SetPointLight( spawnPosition, Vector4( 1.0f, 0.0f, 0.0f, 100.0f ), Vector3( 0.0f, 2.0f, 0.2f ) );
	m_light->m_transform.Reparent( m_transform );
	g_theGame->AddLight( m_light );
	
	m_particleBurstTrail = new ParticleEmitter( Vector3::ZERO, Vector3::ZERO, 0.0f, Renderer::GetInstance()->CreateOrGetTexture( PARTICLE_TEXTURE_FILEPATH ) );
	m_particleBurstTrail->GetTransform()->Reparent( m_transform );
	m_particleBurstTrail->GetTransform()->Translate( Vector3( 0.0f, 0.0f, -2.0f ) );
	m_particleBurstTrail->m_particleColor = Rgba::YELLOW;
	m_particleBurstTrail->m_spawnsOverTime = false;
	m_particleBurstTrail->m_sizeRange = FloatRange( 0.05f, 0.5f );
	m_particleBurstTrail->m_lifetimeRange = FloatRange( 3.0f, 5.0f );
	m_particleBurstTrail->SpawnParticles( PROJECTILE_PARTICLE_BURST_COUNT, true );
}

Projectile::Projectile( const Vector3& spawnPosition, const Vector3& direction, float speed, float timeToLive /* = PROJECTILE_DEFAULT_TIME_TO_LIVE */ )
	:	m_velocity( speed * direction )
{
	m_stopWatch.SetTimer( timeToLive );
	m_renderable = new Renderable( g_renderer->MakeUVSphereMesh( Vector3::ZERO, PROJECTILE_RADIUS, 4, 4, Rgba::RED ).CreateMesh(), g_renderer->CreateOrGetMaterial( "Projectile" )->GetInstance() );
	m_transform = new Transform( spawnPosition, Vector3::ZERO, Vector3::ONE );

	m_light = new Light();
	m_light->SetPointLight( spawnPosition, Vector4( 1.0f, 0.0f, 0.0f, 100.0f ), Vector3( 0.0f, 2.0f, 0.2f ) );
	m_light->m_transform.Reparent( m_transform );
	g_theGame->AddLight( m_light );
	
	m_particleBurstTrail = new ParticleEmitter( Vector3::ZERO, Vector3::ZERO, 0.0f, Renderer::GetInstance()->CreateOrGetTexture( PARTICLE_TEXTURE_FILEPATH ) );
	m_particleBurstTrail->GetTransform()->Reparent( m_transform );
	m_particleBurstTrail->GetTransform()->Translate( Vector3( 0.0f, 0.0f, -2.0f ) );
	m_particleBurstTrail->m_particleColor = Rgba::YELLOW;
	m_particleBurstTrail->m_spawnsOverTime = false;
	m_particleBurstTrail->m_sizeRange = FloatRange( 0.02f, 0.1f );
	m_particleBurstTrail->m_lifetimeRange = FloatRange( 1.0f, 3.0f );
	m_particleBurstTrail->SpawnParticles( PROJECTILE_PARTICLE_BURST_COUNT, true );
}

Projectile::~Projectile()
{
	delete m_particleBurstTrail;
	m_particleBurstTrail = nullptr;

	delete m_renderable;
	m_renderable = nullptr;

	delete m_transform;
	m_transform = nullptr;

	g_theGame->RemoveLight( m_light );
	delete m_light;
	m_light = nullptr;
}

void Projectile::Update()
{
	m_transform->Translate( m_velocity * GetMasterDeltaSecondsF() );
	m_renderable->SetModelMatrix( m_transform->GetAsMatrixWorld() );

	if ( m_stopWatch.HasElapsed() )
	{
		MarkForDeath();
	}
}

void Projectile::UpdateParticleSystems( Camera* camera )
{
	m_particleBurstTrail->Update( camera, GetMasterDeltaSecondsF() );
}

void Projectile::MarkForDeath()
{
	m_isDead = true;
}

ParticleEmitter* Projectile::GetParticleEmitter()
{
	return m_particleBurstTrail;
}

Renderable* Projectile::GetRenderable()
{
	return m_renderable;
}

Transform* Projectile::GetTransform()
{
	return m_transform;
}

Vector3 Projectile::GetPosition() const
{
	return m_transform->GetWorldPosition();
}

bool Projectile::IsDead() const
{
	return ( m_isDead && m_particleBurstTrail->IsSafeToDestroy() );
}
