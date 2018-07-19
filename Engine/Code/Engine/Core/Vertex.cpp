#include "Engine/Core/Vertex.hpp"

Vertex_3DPCU::Vertex_3DPCU()
{
	m_position = Vector3::ZERO;
	m_color = Rgba::WHITE;
	m_UVs = Vector2::ZERO;
}

Vertex_3DPCU::Vertex_3DPCU( Vector3 position, Rgba color, Vector2 UVs )
{
	m_position = position;
	m_color = color;
	m_UVs = UVs;
}