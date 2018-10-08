#pragma once

#include "Engine/Math/Vector3.hpp"
#include "ThirdParty/mikktspace/mikktspace.h"
#include <cstdlib>

// Data structures and functions expected by mikktspace
// Reference used: https://github.com/gltf-rs/mikktspace/blob/master/examples/generate.c

class MeshBuilder;

struct VertexMikkT
{

	float ( *m_position )[ 3 ];
	float ( *m_normal )[ 3 ];
	float ( *m_UVs )[ 2 ];

};

struct FaceMikkT
{

	VertexMikkT m_vertices[ 3 ];

};

struct InputMikkT
{

	float ( *m_positions )[ 3 ] = nullptr;
	float ( *m_normals )[ 3 ] = nullptr;
	float ( *m_UVs )[ 2 ] = nullptr;
	FaceMikkT* m_faces = nullptr;
	size_t m_numVertices = 0;
	size_t m_numFaces = 0;

};

// Functions to fill in the function pointers that form the struct SMikkTSpaceInterface
int GetNumFaces( const SMikkTSpaceContext* context );
int GetNumVerticesInFace( const SMikkTSpaceContext* context, int faceIndex );
void GetPosition( const SMikkTSpaceContext* context, float* destination, int faceIndex, int vertexIndex );
void GetNormal( const SMikkTSpaceContext* context, float* destination, int faceIndex, int vertexIndex );
void GetUVs( const SMikkTSpaceContext* context, float* destination, int faceIndex, int vertexIndex );
void SetTSpaceBasic( const SMikkTSpaceContext* context, const float* tangent, float w, int faceIndex, int vertexIndex );
void SetTSpace( const SMikkTSpaceContext* context, const float* tangent, const float* biTangent, float tW, float btW, tbool operation, int faceIndex, int vertexIndex );

void PopulateTangentsMeshBuilderMikkT( MeshBuilder& meshBuilder );