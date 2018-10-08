#include "Engine/Renderer/GLFunctionBinding.hpp"
#include "Engine/Renderer/Mesh.hpp"
#include "Engine/Renderer/MeshBuilder.hpp"
#include "Engine/Renderer/Renderer.hpp"

DrawInstruction::DrawInstruction()
{

}

DrawInstruction::DrawInstruction( const DrawInstruction& copy )
{
	m_primitive = copy.m_primitive;
	m_usesIndices = copy.m_usesIndices;
	m_startIndex = copy.m_startIndex;
	m_elementCount = copy.m_elementCount;
}

DrawInstruction::DrawInstruction( DrawPrimitiveType primitive, bool usesIndices, unsigned int startIndex, unsigned int elementCount )
	:	m_primitive( primitive ),
		m_usesIndices( usesIndices ),
		m_startIndex( startIndex ),
		m_elementCount( elementCount )
{

}

Mesh::Mesh()
{
	m_layout = &Vertex_3DPCU::s_layout;
}

Mesh::Mesh( unsigned int vertexCount, const VertexLayout* layout, const void* vertices, unsigned int indexCount, const unsigned int* indices )
{
	m_layout = layout;
	SetVertices( vertexCount, vertices, m_layout );
	SetIndices( indexCount, indices );
}

Mesh::~Mesh()
{
	
}

unsigned int Mesh::GetVertexCount() const
{
	return static_cast< unsigned int >( m_vertexBuffer.m_vertexCount );
}

unsigned int Mesh::GetIndexCount() const
{
	return static_cast< unsigned int >( m_indexBuffer.m_indexCount );
}

const VertexLayout* Mesh::GetVertexLayout() const
{
	return m_layout;
}

AABB3 Mesh::GetBounds() const
{
	return m_bounds;
}

void Mesh::SetVertices( unsigned int count, const void* vertexData, const VertexLayout* layout )
{
	m_layout = layout;
	m_vertexBuffer.m_vertexCount = count;
	m_vertexBuffer.m_vertexStride = m_layout->m_stride;
	m_vertexBuffer.CopyToGPU( ( count * m_layout->m_stride ), vertexData );
	StretchBoundsToIncludePoints( count, vertexData, m_layout->m_stride );
}

void Mesh::SetVertices( unsigned int count, const Vertex_3DPCU* vertices )
{
	m_layout = &Vertex_3DPCU::s_layout;
	m_vertexBuffer.m_vertexCount = count;
	m_vertexBuffer.m_vertexStride = sizeof( Vertex_3DPCU );
	m_vertexBuffer.CopyToGPU( ( count * sizeof( Vertex_3DPCU ) ), vertices );
	StretchBoundsToIncludePoints( count, vertices, sizeof( Vertex_3DPCU ) );
}

void Mesh::SetVertices( const std::vector<Vertex_3DPCU>& vertices )
{
	m_layout = &Vertex_3DPCU::s_layout;
	m_vertexBuffer.m_vertexCount = static_cast< unsigned int >( vertices.size() );
	m_vertexBuffer.m_vertexStride = sizeof( Vertex_3DPCU );
	m_vertexBuffer.CopyToGPU( ( static_cast< unsigned int >( vertices.size() ) * sizeof( Vertex_3DPCU ) ), vertices.data() );
	StretchBoundsToIncludePoints( static_cast< unsigned int >( vertices.size() ), vertices.data(), sizeof( Vertex_3DPCU ) );
}

void Mesh::SetIndices( unsigned int count, const unsigned int* indices )
{
	m_indexBuffer.m_indexCount = count;
	m_indexBuffer.m_indexStride = sizeof( unsigned int );
	m_indexBuffer.CopyToGPU( ( count * sizeof( unsigned int ) ), indices );
}

void Mesh::SetIndices( const std::vector< unsigned int >& indices )
{
	m_indexBuffer.m_indexCount = static_cast< unsigned int >( indices.size() );
	m_indexBuffer.m_indexStride = sizeof( unsigned int );
	m_indexBuffer.CopyToGPU( ( static_cast< unsigned int >( indices.size() ) * sizeof( unsigned int ) ), indices.data() );
}

void Mesh::SetDrawInstruction( const DrawInstruction& drawInstruction )
{
	m_drawInstruction = drawInstruction;
}

void Mesh::SetDrawInstruction( DrawPrimitiveType primitive, bool usesIndices, unsigned int startIndex, unsigned int elementCount )
{
	m_drawInstruction = DrawInstruction( primitive, usesIndices, startIndex, elementCount );
}

void Mesh::StretchBoundsToIncludePoints( unsigned int count, const void* vertexData, unsigned int stride )
{
	const float* currentVertex = reinterpret_cast< const float* >( vertexData );
	unsigned int floatElementStride = static_cast< unsigned int >( static_cast< float >( stride ) / sizeof( float ) );

	for ( unsigned int index = 0U; index < count; index++ )
	{
		Vector3 point = Vector3( currentVertex[ 0 ], currentVertex[ 1 ], currentVertex[ 2 ] );
		m_bounds.StretchToIncludePoint( point );
		currentVertex += floatElementStride;
	}
}

void Mesh::Finalize() const
{
	m_vertexBuffer.Finalize();
	if ( m_drawInstruction.m_usesIndices )
	{
		m_indexBuffer.Finalize();
	}
}
