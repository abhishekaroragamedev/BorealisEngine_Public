#include "Engine/Core/Vertex.hpp"
#include "Engine/Renderer/MeshBuilder.hpp"
#include "Engine/Renderer/Renderer.hpp"

#pragma region VertexLayout

VertexAttribute::VertexAttribute()
{

}

VertexAttribute::VertexAttribute( const std::string& name, RendererDataType type, unsigned int elementCount, bool normalized, unsigned int memberOffset )
	:	m_name( name ),
		m_type( type ),
		m_normalized( normalized ),
		m_elementCount( elementCount ),
		m_memberOffset( memberOffset )
{

}

VertexLayout::VertexLayout()
{

}

VertexLayout::VertexLayout( unsigned int stride, const std::vector< VertexAttribute > attributes )
	:	m_stride( stride ),
		m_attributes( attributes )
{

}

unsigned int VertexLayout::GetAttributeCount() const
{
	return static_cast< unsigned int >( m_attributes.size() );
}

VertexAttribute VertexLayout::GetAttribute( const std::string& name ) const
{
	for ( size_t index = 0; index < m_attributes.size(); index++ )
	{
		if ( m_attributes[ index ].m_name == name )
		{
			return m_attributes[ index ];
		}
	}

	return VertexAttribute();
}

VertexAttribute VertexLayout::GetAttribute( unsigned int index ) const
{
	return m_attributes[ index ];
}

#pragma endregion

#pragma region VertexTypes

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

Vertex_3DPCU::Vertex_3DPCU( const VertexBuilder& vertexBuilder )
	:	m_position( vertexBuilder.m_position ),
		m_color( vertexBuilder.m_color ),
		m_UVs( vertexBuilder.m_UVs )
{

}

/* static */
const std::vector< VertexAttribute > Vertex_3DPCU::s_attributes = std::vector< VertexAttribute >(
	{
		VertexAttribute( "POSITION", RendererDataType::RENDER_TYPE_FLOAT, 3, false, offsetof( Vertex_3DPCU, m_position ) ),
		VertexAttribute( "COLOR", RendererDataType::RENDER_TYPE_FLOAT, 4, false, offsetof( Vertex_3DPCU, m_color ) ),
		VertexAttribute( "UV", RendererDataType::RENDER_TYPE_FLOAT, 2, false, offsetof( Vertex_3DPCU, m_UVs ) )
	}
);

/* static */
const VertexLayout Vertex_3DPCU::s_layout = VertexLayout( sizeof( Vertex_3DPCU ), Vertex_3DPCU::s_attributes );

Vertex_3DLit::Vertex_3DLit()
{

}

Vertex_3DLit::Vertex_3DLit( Vector3 position, Rgba color, Vector2 UVs, Vector3 normal, Vector4 tangent )
	:	m_position( position ),
		m_color( color ),
		m_UVs( UVs ),
		m_normal( normal ),
		m_tangent( tangent )
{
	m_normal.NormalizeAndGetLength();
}

Vertex_3DLit::Vertex_3DLit( const VertexBuilder& vertexBuilder )
	:	m_position( vertexBuilder.m_position ),
		m_color( vertexBuilder.m_color ),
		m_UVs( vertexBuilder.m_UVs ),
		m_normal( vertexBuilder.m_normal ),
		m_tangent( vertexBuilder.m_tangent )
{

}

/* static */
const std::vector< VertexAttribute > Vertex_3DLit::s_attributes = std::vector< VertexAttribute >(
	{
		VertexAttribute( "POSITION", RendererDataType::RENDER_TYPE_FLOAT, 3, false, offsetof( Vertex_3DLit, m_position ) ),
		VertexAttribute( "COLOR", RendererDataType::RENDER_TYPE_FLOAT, 4, false, offsetof( Vertex_3DLit, m_color ) ),
		VertexAttribute( "UV", RendererDataType::RENDER_TYPE_FLOAT, 2, false, offsetof( Vertex_3DLit, m_UVs ) ),
		VertexAttribute( "NORMAL", RendererDataType::RENDER_TYPE_FLOAT, 3, false, offsetof( Vertex_3DLit, m_normal ) ),
		VertexAttribute( "TANGENT", RendererDataType::RENDER_TYPE_FLOAT, 4, false, offsetof( Vertex_3DLit, m_tangent ) )
	}
);

/* static */
const VertexLayout Vertex_3DLit::s_layout = VertexLayout( sizeof( Vertex_3DLit ), Vertex_3DLit::s_attributes );

#pragma endregion