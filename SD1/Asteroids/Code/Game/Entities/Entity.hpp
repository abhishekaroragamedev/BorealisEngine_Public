#pragma once

#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Disc2.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Game/GameCommon.hpp"

class Entity
{

public:
	Entity();
	~Entity();

public:
	virtual void Update( float deltaSeconds );
	virtual void Render( bool developerModeEnabled ) const;
	Vector2 GetLocation() const;
	Vector2 GetVelocity() const;
	float GetOrientationDegrees() const;
	Disc2 GetCosmeticDisc2() const;
	Disc2 GetPhysicalDisc2() const;

protected:
	void Rotate( float deltaSeconds );
	void Translate( float deltaSeconds );
	void PopulateDeveloperModeCirclesVertices();

protected:
	Vector2 m_location;
	Vector2 m_velocity;
	float m_orientation;
	float m_rotationSpeed;
	float m_speed;
	Disc2 m_cosmeticDisc2;
	Disc2 m_physicalDisc2;
	Vector2 m_cosmeticVertices[ CIRCLE_NUM_SIDES ];
	Vector2 m_physicalVertices[ CIRCLE_NUM_SIDES ];

private:
	void WrapAroundScreenIfBeyondBounds();
	bool IsEntityBeyondNegativeXBound() const;
	bool IsEntityBeyondPositiveXBound() const;
	bool IsEntityBeyondNegativeYBound() const;
	bool IsEntityBeyondPositiveYBound() const;
	void RenderDeveloperMode() const;
	void RenderVelocityVector() const;
	void RenderPhysicalCircle() const;
	void RenderCosmeticCircle() const;
};
