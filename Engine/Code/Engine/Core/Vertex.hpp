#pragma once

#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"

struct Vertex_3DPCU
{
public:
	Vertex_3DPCU();
	Vertex_3DPCU( Vector3 position, Rgba color, Vector2 UVs );

public:
	Vector3 m_position;
	Rgba m_color;
	Vector2 m_UVs;

};