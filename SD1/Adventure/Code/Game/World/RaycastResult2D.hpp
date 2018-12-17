#pragma once

#include "Engine/Math/IntVector2.hpp"
#include "Engine/Math/Vector2.hpp"

struct RaycastResult2D
{
	bool m_didImpact;
	Vector2 m_impactPosition;
	IntVector2 m_impactTileCoordinates;
	Vector2 m_impactNormal;
	float m_impactDistanceToMaxDistanceRatio;
};