#pragma once

#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/IntRange.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Transform.hpp"
#include "Engine/Math/Vector3.hpp"
#include <vector>

class Camera;
class Material;
class Mesh;
class MeshBuilder;
class Renderable;
class Texture;

struct Particle
{
public:
	explicit Particle( const Vector3& spawnPosition, const Vector3& spawnVelocity, const Vector3& initialForce, float size, float mass, float lifeTime )
		:	m_position( spawnPosition ),
			m_velocity( spawnVelocity ),
			m_perFrameForce( initialForce ),
			m_size( size ),
			m_mass( mass ),
			m_lifeTime( lifeTime )
	{
		if ( IsFloatEqualTo( m_mass, 0.0f ) )
		{
			m_mass = 1.0f;
		}
	}

	void Update( float deltaSeconds );
	inline bool IsDead();
	float GetNormalizedAge();

public:
	Vector3 m_position = Vector3::ZERO; 
	Vector3 m_velocity = Vector3::ZERO;
	Vector3 m_perFrameForce = Vector3::ZERO;
	float m_size = 0.0f;
	float m_mass = 0.0f;
	float m_lifeTime = 0.0f;
	float m_age = 0.0f;
};

class ParticleEmitter 
{

public:
	ParticleEmitter( const Vector3& position, const Vector3& eulerAngles, float spawnRate = 1.0f, const Texture* texture = nullptr );
	~ParticleEmitter();

	void Update( Camera* camera, float deltaSeconds );
	void SpawnParticle();
	void SpawnParticles( unsigned int count, bool convertToBurst = false );

	Transform* GetTransform();
	Renderable* GetRenderable();
	bool IsSafeToDestroy() const;

private:
	void TryCreateNewParticles( float deltaSeconds );
	void UpdateParticles( float deltaSeconds );
	void ConstructMesh( Camera* camera );

public:
	bool m_spawnsOverTime = true;
	float m_spawnRate = 0.0f;
	FloatRange m_sizeRange;
	FloatRange m_massRange;
	FloatRange m_lifetimeRange;
	IntRange m_burstRate;
	Vector3 m_forceOnParticles = ( Vector3::UP * -1.0f ); // Pseudo-gravity
	Rgba m_particleColor = Rgba::WHITE;

private:
	std::vector< Particle* > m_particles;
	Transform m_transform;
	Renderable* m_renderable = nullptr;
	Material* m_material = nullptr;
	Mesh* m_mesh = nullptr;
	MeshBuilder* m_meshBuilder = nullptr;
	const Texture* m_texture = nullptr;

};
