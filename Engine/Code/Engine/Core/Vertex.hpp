#pragma once

#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Vector4.hpp"
#include <string>
#include <vector>

#pragma region VertexLayout

enum RendererDataType;
struct VertexBuilder;

struct VertexAttribute
{

public:
	VertexAttribute();
	VertexAttribute( const std::string& name, RendererDataType type, unsigned int elementCount, bool normalized, unsigned int memberOffset );

public:
	std::string m_name = "";
	RendererDataType m_type;
	unsigned int m_elementCount = 0;
	unsigned long long m_memberOffset = 0;
	bool m_normalized = false;

};

class VertexLayout
{

public:
	VertexLayout();
	VertexLayout( unsigned int stride, const std::vector< VertexAttribute > attributes );

	unsigned int GetAttributeCount() const;
	VertexAttribute GetAttribute( const std::string& name ) const;
	VertexAttribute GetAttribute( unsigned int index ) const;

public:
	std::vector< VertexAttribute > m_attributes;
	unsigned int m_stride = 0;

};

#pragma endregion

#pragma region VertexTypes

struct Vertex_3DPCU
{

public:
	Vertex_3DPCU();
	explicit Vertex_3DPCU( Vector3 position, Rgba color, Vector2 UVs );
	explicit Vertex_3DPCU( const VertexBuilder& vertexBuilder );

public:
	Vector3 m_position = Vector3::ZERO;
	Rgba m_color = Rgba::WHITE;
	Vector2 m_UVs = Vector2::ZERO;

public:
	static const std::vector< VertexAttribute > s_attributes;
	static const VertexLayout s_layout;

};

struct Vertex_3DLit
{

public:
	Vertex_3DLit();
	explicit Vertex_3DLit( Vector3 position, Rgba color, Vector2 UVs, Vector3 normal, Vector4 tangent );
	explicit Vertex_3DLit( const VertexBuilder& vertexBuilder );

public:
	Vector3 m_position = Vector3::ZERO;
	Rgba m_color = Rgba::WHITE;
	Vector2 m_UVs = Vector2::ZERO;
	Vector3 m_normal = Vector3::ZERO;
	Vector4 m_tangent = Vector4::ZERO_DISPLACEMENT;

	static const std::vector< VertexAttribute > s_attributes;
	static const VertexLayout s_layout;

};

#pragma endregion