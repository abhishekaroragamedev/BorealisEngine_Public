#pragma once

#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Disc2.hpp"
#include "Engine/Renderer/Renderer.hpp"

class Entity
{

public:
	~Entity();

public:
	virtual void Update( float deltaSeconds );
	virtual void Render() const;
	Vector2 GetLocation() const;
	Vector2 GetVelocity() const;
	float GetOrientationDegrees() const;

private:
	Entity();		// Should be created by a Factory

protected:
	virtual void Rotate( float deltaSeconds );
	virtual void Translate( float deltaSeconds );

protected:
	Vector2 m_location = Vector2::ZERO;
	Vector2 m_velocity = Vector2::ZERO;
	float m_orientation = 0.0f;
	float m_rotationSpeed = 0.0f;
	float m_speed = 0.0f;

};
