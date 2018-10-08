#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/Mesh.hpp"
#include "Engine/Renderer/MeshBuilder.hpp"
#include "Engine/Renderer/ParticleEmitter.hpp"
#include "Engine/Renderer/Renderable.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Tools/DevConsole.hpp"

void Particle::Update( float deltaSeconds )
{
	m_velocity += ( m_perFrameForce / m_mass ) * deltaSeconds;
	m_position += m_velocity * deltaSeconds;

	m_perFrameForce = Vector3::ZERO;

	m_age += deltaSeconds;
}

bool Particle::IsDead()
{
	return ( IsFloatGreaterThanOrEqualTo( m_age, m_lifeTime ) );
}

float Particle::GetNormalizedAge()
{
	return ( m_age / m_lifeTime );
}

ParticleEmitter::ParticleEmitter( const Vector3& position, const Vector3& eulerAngles, float spawnRate /* = 1.0f */, const Texture* texture /* = nullptr */ )
	:	m_transform( position, eulerAngles, Vector3::ONE ),
		m_spawnRate( spawnRate ),
		m_sizeRange( 0.5f, 1.0f ),
		m_massRange( 0.5f, 1.0f ),
		m_lifetimeRange( 1.0f, 5.0f ),
		m_texture( texture )
{
	m_material = new Material( Renderer::GetInstance()->CreateOrGetShader( "Additive" ), true );
	if ( m_texture == nullptr )
	{
		m_texture = Renderer::GetInstance()->GetDefaultTexture();
	}
	m_material->SetTextureAndSampler( 0U, m_texture, Renderer::GetInstance()->GetDefaultSampler() );
	m_meshBuilder = new MeshBuilder;
	m_renderable = new Renderable( m_mesh, m_material, true, true );
	// Mesh is regenerated every frame
}

ParticleEmitter::~ParticleEmitter()
{
	delete m_meshBuilder;
	delete m_renderable;

	if ( !IsSafeToDestroy() )
	{
		ConsolePrintf( Rgba::YELLOW, "WARNING: ParticleEmitter::~ParticleEmitter(): a Particle Emitter was destroyed before it was safe to do so. Be careful!" );
	}
	for ( Particle* particle : m_particles )
	{
		delete particle;
	}
}

void ParticleEmitter::Update( Camera* camera, float deltaSeconds )
{
	TryCreateNewParticles( deltaSeconds );
	UpdateParticles( deltaSeconds );
	ConstructMesh( camera );
}

void ParticleEmitter::TryCreateNewParticles( float deltaSeconds )
{
	static float s_timeSinceLastParticleSpawned = 0.0f;
	if ( m_spawnsOverTime && ( s_timeSinceLastParticleSpawned > m_spawnRate ) )
	{
		SpawnParticle();
	}
	s_timeSinceLastParticleSpawned += deltaSeconds;
}

void ParticleEmitter::UpdateParticles( float deltaSeconds )
{
	Matrix44 currentTransformMatrix = m_transform.GetParentTransformsAsMatrixWorld();
	Vector3 forceAfterTransform = currentTransformMatrix.TransformDisplacement( m_forceOnParticles );

	for ( size_t particleIndex = 0; particleIndex < m_particles.size(); particleIndex++ )
	{
		Particle* particle = m_particles[ particleIndex ];
		particle->m_perFrameForce = forceAfterTransform + Vector3::GetRandomPointOnSphere();
		particle->Update( deltaSeconds );

		if ( particle->IsDead() )
		{
			m_particles[ particleIndex ] = m_particles[ m_particles.size() - 1 ];
			m_particles.pop_back();
			delete particle;
			particleIndex--;
		}
	}
}

void ParticleEmitter::ConstructMesh( Camera* camera )
{
	m_meshBuilder->Clear();

	Renderer::GetInstance()->SetCamera( camera );

	m_meshBuilder->Begin( DrawPrimitiveType::TRIANGLES, true );
	for ( Particle* particle : m_particles )
	{
		m_meshBuilder->MergeMesh( Renderer::GetInstance()->MakeTexturedOrientedQuadMesh( particle->m_position, camera->GetRight(), Vector2( 0.5f, 0.5f ), Vector2( particle->m_size, particle->m_size ), *m_texture, Vector2::ZERO, Vector2::ONE, m_particleColor ) );
	}
	m_meshBuilder->End();

	m_mesh = m_meshBuilder->CreateMesh();
	m_renderable->ReplaceMesh( m_mesh );
}

void ParticleEmitter::SpawnParticle()
{
	m_particles.push_back(
		new Particle(	m_transform.GetWorldPosition(),
						m_transform.GetAsMatrixWorld().TransformDisplacement( m_forceOnParticles ),
						m_transform.GetAsMatrixWorld().TransformDisplacement( m_forceOnParticles ),
						GetRandomFloatInRange( m_sizeRange.min, m_sizeRange.max ),
						GetRandomFloatInRange( m_massRange.min, m_massRange.max ),
						GetRandomFloatInRange( m_lifetimeRange.min, m_lifetimeRange.max )
		) );
}

void ParticleEmitter::SpawnParticles( unsigned int count, bool convertToBurst /* = false */ )
{
	while ( count > 0 )
	{
		SpawnParticle();
		count--;
	}
	m_spawnsOverTime = !convertToBurst;
}

Transform* ParticleEmitter::GetTransform()
{
	return &m_transform;
}

Renderable* ParticleEmitter::GetRenderable()
{
	return m_renderable;
}

bool ParticleEmitter::IsSafeToDestroy() const
{
	return ( m_spawnsOverTime || ( m_particles.size() == 0 ) );
}
