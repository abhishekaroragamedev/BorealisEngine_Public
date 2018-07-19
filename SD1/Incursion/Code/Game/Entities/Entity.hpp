#pragma once

#include "Engine/Math/Vector2.hpp"
#include "Engine/Renderer/Renderer.hpp"

enum EntityType
{
	ENTITY_TYPE_PLAYER_TANK,
	ENTITY_TYPE_ENEMY_TANK,
	ENTITY_TYPE_ENEMY_TURRET,
	ENTITY_TYPE_PLAYER_BULLET,
	ENTITY_TYPE_ENEMY_BULLET,
	ENTITY_TYPE_EXPLOSION,
	NUM_ENTITY_TYPES
};

class Entity
{

	friend class TheEntityFactory;		// No class should be able to instantiate an Entity other than the Entity Factory

public:
	virtual void Update( float deltaSeconds ) = 0;
	virtual void DoDamage();
	virtual void Render( bool developerModeEnabled ) const = 0;
	Vector2 GetLocation() const;
	Vector2 GetVelocity() const;
	float GetOrientationDegrees() const;
	int GetHealth() const;
	bool IsDead() const;

protected:
	void Rotate( float deltaSeconds );
	virtual void Translate( float deltaSeconds );
	void MarkForDeathIfHealthIsZero();
	void PopulateDeveloperModeCirclesVertices( float cosmeticRadius, float physicalRadius );
	void RenderDeveloperMode() const;
	void RenderPhysicalCircle() const;
	void RenderCosmeticCircle() const;

protected:
	Vector2 m_location;
	Vector2 m_velocity;
	float m_orientation;
	float m_rotationSpeed;
	float m_speed;
	int m_health;
	bool m_isDead;

	Vector2 m_cosmeticVertices[ CIRCLE_NUM_SIDES ];
	Vector2 m_physicalVertices[ CIRCLE_NUM_SIDES ];

};
