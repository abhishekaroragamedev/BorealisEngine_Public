#pragma once

#include "Engine/Core/Vertex.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Renderer/RenderBuffer.hpp"
#include <vector>

enum DrawPrimitiveType;
class MeshBuilder;

struct DrawInstruction
{

public:
	DrawInstruction();
	DrawInstruction( const DrawInstruction& copy );
	explicit DrawInstruction( DrawPrimitiveType primitive, bool usesIndices, unsigned int startIndex, unsigned int elementCount );

public:
	DrawPrimitiveType m_primitive;
	unsigned int m_startIndex = 0U;
	unsigned int m_elementCount = 0U;
	bool m_usesIndices = true;

};

class Mesh
{

	friend class Renderer;

public:
	Mesh();
	explicit Mesh( unsigned int vertexCount, const VertexLayout* layout, const void* vertices, unsigned int indexCount, const unsigned int* indices );
	~Mesh();

	unsigned int GetVertexCount() const;
	unsigned int GetIndexCount() const;
	const VertexLayout* GetVertexLayout() const;
	AABB3 GetBounds() const;

	template < typename VERTTYPE = Vertex_3DLit >
	void SetVertices( unsigned int count, const VERTTYPE* vertices )
	{
		m_layout = &VERTTYPE::s_layout;
		m_vertexBuffer.CopyToGPU( ( count * sizeof( VERTTYPE ) ), vertices );
	}
	template < typename VERTTYPE = Vertex_3DLit >
	void FromBuilder( const MeshBuilder& meshBuilder )
	{
		unsigned int vertexCount = meshBuilder.GetVertexCount();

		VERTTYPE* vertices = reinterpret_cast< VERTTYPE* >( malloc( VERTTYPE::s_layout.m_stride * vertexCount ) );
		for ( unsigned int vertexIndex = 0; vertexIndex < vertexCount; vertexIndex++ )
		{
			vertices[ vertexIndex ] = VERTTYPE( meshBuilder.GetVertex( vertexIndex ) );
		}
		SetVertices< VERTTYPE >( vertexCount, vertices );
		
		SetIndices( meshBuilder.GetIndices() );
		SetDrawInstruction( meshBuilder.GetDrawInstruction() );

		free( vertices );
	}

	void SetVertices( unsigned int count, const void* vertexData, const VertexLayout* layout );
	void SetVertices( unsigned int count, const Vertex_3DPCU* vertices );
	void SetVertices( const std::vector< Vertex_3DPCU >& vertices );
	void SetIndices( unsigned int count, const unsigned int* indices );
	void SetIndices( const std::vector< unsigned int >& indices );
	void SetDrawInstruction( const DrawInstruction& drawInstruction );
	void SetDrawInstruction( DrawPrimitiveType primitive, bool usesIndices, unsigned int startIndex, unsigned int elementCount );
	void StretchBoundsToIncludePoints( unsigned int count, const void* vertexData, unsigned int stride );
	void Finalize() const;

private:
	VertexBuffer m_vertexBuffer;
	IndexBuffer m_indexBuffer;
	DrawInstruction m_drawInstruction;
	const VertexLayout* m_layout = &Vertex_3DLit::s_layout;
	AABB3 m_bounds;

};
