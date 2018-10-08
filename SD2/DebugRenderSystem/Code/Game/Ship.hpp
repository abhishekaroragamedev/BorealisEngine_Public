#pragma once

#include "Engine/Math/Vector3.hpp"

constexpr char SCI_FI_FIGHTER_MODEL_FILEPATH[] = "Data/Models/scifi_fighter_mk6/scifi_fighter_mk6.obj";

constexpr float PROJECTILE_FIRE_SPEED = 5.0f;
constexpr float PARTICLE_EMITTER_IDLE_RATE = 30.0f;
constexpr float PARTICLE_EMITTER_ACCELERATE_RATE = 50.0f;
constexpr float SHIP_TRANSLATION_PER_SECOND = 5.0f;
constexpr float SHIP_ROTATION_PER_SECOND = 150.0f;

class ParticleEmitter;
class Renderable;
class Transform;

class Ship
{

public:
	Ship();
	explicit Ship( const Vector3& startPosition, const Vector3& startEuler = Vector3::ZERO, const Vector3& startScale = Vector3::ONE );
	~Ship();

	void Update();
	void UpdateParticleSystems( Camera* camera );

	Renderable* GetRenderable();
	Transform* GetTransform();
	ParticleEmitter* GetLeftThruster();
	ParticleEmitter* GetRightThruster();
	Vector3 GetPosition() const;
	Vector3 GetUp() const;
	Vector3 GetRight() const;
	Vector3 GetForward() const;

	void Translate( const Vector3& translation );
	void Rotate( const Vector3& rotationEuler );
	void SetSpecularProperties( float specularAmount, float specularPower );
	void InitializeMaterial( bool haveMaterialsInitialized = false, int renderModeIndex = 0 );

private:
	void InitializeMesh();
	void InitializeParticleSystems();
	void HandleKeyboardInput();
	void HandleMouseInput();

private:
	Renderable* m_renderable = nullptr;
	Transform* m_transform = nullptr;
	Vector2 m_specularProperties = Vector2( 0.9f, 16.0f );

	ParticleEmitter* m_leftThruster = nullptr;
	ParticleEmitter* m_rightThruster = nullptr;

};