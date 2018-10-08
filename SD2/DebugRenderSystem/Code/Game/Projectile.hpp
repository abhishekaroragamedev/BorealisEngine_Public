#pragma once

#include "Engine/Core/Stopwatch.hpp"
#include "Engine/Math/Vector3.hpp"

struct LightUBO;
class Camera;
class ParticleEmitter;
class Renderable;
class Transform;

constexpr float PROJECTILE_RADIUS = 0.5f;
constexpr float PROJECTILE_DEFAULT_TIME_TO_LIVE = 5.0f;
constexpr unsigned int PROJECTILE_PARTICLE_BURST_COUNT = 300;

class Projectile
{

public:
	Projectile( const Vector3& spawnPosition, const Vector3& velocity, float timeToLive = PROJECTILE_DEFAULT_TIME_TO_LIVE );
	Projectile( const Vector3& spawnPosition, const Vector3& direction, float speed, float timeToLive = PROJECTILE_DEFAULT_TIME_TO_LIVE );
	~Projectile();

	void Update();
	void UpdateParticleSystems( Camera* camera );
	void MarkForDeath();

	Renderable* GetRenderable();
	Transform* GetTransform();
	ParticleEmitter* GetParticleEmitter();
	Vector3 GetPosition() const;
	bool IsDead() const;

private:
	Renderable* m_renderable = nullptr;
	Transform* m_transform = nullptr;
	Vector3 m_velocity = Vector3::ZERO;
	StopWatch m_stopWatch = nullptr;
	Light* m_light = nullptr;
	ParticleEmitter* m_particleBurstTrail = nullptr;
	bool m_isDead = false;

};