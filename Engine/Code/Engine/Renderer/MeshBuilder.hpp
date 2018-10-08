#pragma once
#include "Engine/Core/Vertex.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Renderer/Mesh.hpp"
#include "Engine/Renderer/RenderBuffer.hpp"
#include "Engine/Renderer/SurfacePatch.hpp"
#include <functional>

enum DrawPrimitiveType;

struct VertexBuilder
{

public:
	VertexBuilder();
	VertexBuilder( Vector3 position, Rgba color, Vector2 UVs );	// VertexBuilder as Vertex_3DPCU
	VertexBuilder( Vector3 position, Rgba color, Vector2 UVs, Vector3 normal, Vector4 tangent );	// VertexBuilder as Vertex_3DLit
	VertexBuilder( const VertexBuilder& copy );

public:
	Vector3 m_position = Vector3::ZERO;
	Rgba m_color = Rgba::WHITE;
	Vector2 m_UVs = Vector2::ZERO;
	Vector3 m_normal = Vector3::FORWARD;
	Vector4 m_tangent = Vector4::ZERO_DISPLACEMENT;

public:
	static Vertex_3DPCU MakeVertex3D_PCU( const VertexBuilder& vertexBuilder );

public:
	static const std::vector< VertexAttribute > s_attributes;
	static const VertexLayout s_layout;

};

class MeshBuilder
{

public:
	MeshBuilder();
	MeshBuilder( const MeshBuilder& copy );
	~MeshBuilder();

	void Begin( DrawPrimitiveType primitive, bool useIndices = true );
	void End();

	void SetDrawInstruction( DrawInstruction drawInstruction );
	void SetDrawInstruction( DrawPrimitiveType primitive, bool usesIndices, unsigned int startIndex, unsigned int elementCount );
	void SetColor( const Rgba& color );
	void SetUVs( const Vector2& uv );
	void SetUVs( float u, float v );
	void SetNormal( const Vector3& normal );
	void SetNormal( float x, float y, float z );
	void SetTangent( const Vector4& tangent );
	void SetTangent( float x, float y, float z, float w );
	void SetTangentAtIndex( unsigned int index, const Vector4& tangent );	// Usually called by MikkT Implementation
	void SetTangentAtIndex( unsigned int index, float x, float y, float z, float w );

	unsigned int PushVertex( float x, float y, float z );
	unsigned int PushVertex( const Vector3& position );
	unsigned int PushVertex( VertexBuilder vertexBuilder );
	unsigned int PushVertices( unsigned int count, const VertexBuilder* vertices );
	unsigned int PushVertices( unsigned int count, const Vertex_3DPCU* vertices );
	unsigned int PushIndex( unsigned int index );
	unsigned int PushIndices( unsigned int count, const unsigned int* indices );
	
	unsigned int MergeMesh( const MeshBuilder& meshBuilder );	// Only compatible if Primitive and UseIndices is the same for both meshes; returns vertex count after merge

	void AddSurfacePatch( std::function< Vector3( float, float ) > surfacePatchCallback, const FloatRange& uRange, const FloatRange& vRange, const IntVector2& numSamples, const Vector3& position = Vector3::ZERO, float scale = 1.0f ); // Does not use indices

	Mesh* CreateMesh() const;
	void PopulateMesh( Mesh& mesh ) const;
	Mesh* Flush();
	void RecomputeNormals();
	void RecomputeTangents();

	void Clear();

	VertexBuilder GetVertex( unsigned int index ) const;
	VertexBuilder* GetVertexAsReference( unsigned int index );
	DrawInstruction GetDrawInstruction() const;
	unsigned int GetVertexCount() const;
	unsigned int GetIndexCount() const;
	const std::vector< unsigned int > GetIndices() const;

	Vector3 GetAverageVertexPosition() const;	// Returns the average of the positions of all vertices currently in the Builder
	void SetVerticesRelativeTo( const Vector3& localOrigin );	// Makes all positions relative to the specified position

public:
	static std::vector< Mesh* > FromFileOBJ( const char* filePath );
	static std::vector< Mesh* > FromFileOBJ( const std::string& filePath );
	static std::vector< std::string > GetMaterialNamesFromMTL( const std::string& filePath );

private:
	int GetMaxIndex() const;

private:
	VertexBuilder m_stamp;
	std::vector< VertexBuilder > m_vertices;
	std::vector< unsigned int > m_indices;
	DrawInstruction m_drawInstruction;

};
