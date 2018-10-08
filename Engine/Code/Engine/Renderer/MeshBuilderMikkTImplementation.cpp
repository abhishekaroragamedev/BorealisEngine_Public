#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/MeshBuilder.hpp"
#include "Engine/Renderer/MeshBuilderMikkTImplementation.hpp"

SMikkTSpaceContext g_mikkTContext;
InputMikkT g_mikkTInputStamp;
MeshBuilder* currentMesh = nullptr;

int GetNumFaces( const SMikkTSpaceContext* context )
{
	return static_cast< int >( g_mikkTInputStamp.m_numFaces );
}

int GetNumVerticesInFace( const SMikkTSpaceContext* context, int faceIndex )
{
	return 3;
}

void GetPosition( const SMikkTSpaceContext* context, float* destination, int faceIndex, int vertexIndex )
{
	float ( *source )[ 3 ] = g_mikkTInputStamp.m_faces[ faceIndex ].m_vertices[ vertexIndex ].m_position;
	memcpy( destination, *source, ( 3 * sizeof( float ) ) );
}

void GetNormal( const SMikkTSpaceContext* context, float* destination, int faceIndex, int vertexIndex )
{
	float ( *source )[ 3 ] = g_mikkTInputStamp.m_faces[ faceIndex ].m_vertices[ vertexIndex ].m_normal;
	Vector3 normalNormalized = Vector3( *source[ 0 ], *source[ 1 ], *source[ 2 ] );	// Normalize vectors to make sure they have a magnitude of 1
	normalNormalized = normalNormalized.GetNormalized();

	memcpy( destination, &normalNormalized, ( 3 * sizeof( float ) ) );
}

void GetUVs( const SMikkTSpaceContext* context, float* destination, int faceIndex, int vertexIndex )
{
	float ( *source )[ 2 ] = g_mikkTInputStamp.m_faces[ faceIndex ].m_vertices[ vertexIndex ].m_UVs;
	memcpy( destination, *source, ( 2 * sizeof( float ) ) );
}

void SetTSpaceBasic( const SMikkTSpaceContext* context, const float* tangent, float w, int faceIndex, int vertexIndex )
{
	Vector3 tangentNormalized = Vector3( tangent[ 0 ], tangent[ 1 ], tangent[ 2 ] );	// Normalize vectors to make sure they have a magnitude of 1
	tangentNormalized = tangentNormalized.GetNormalized();

	currentMesh->SetTangentAtIndex( ( ( faceIndex * 3 ) + vertexIndex ), tangentNormalized.x, tangentNormalized.y, tangentNormalized.z, w );
}

void SetTSpace( const SMikkTSpaceContext* context, const float* tangent, const float* biTangent, float tW, float btW, tbool operation, int faceIndex, int vertexIndex )
{
	UNIMPLEMENTED()
	// SetTSpaceBasic( context, tangent, ( ( operation != 0 )? 1.0f : -1.0f ), faceIndex, vertexIndex );
}

void InitializeMikkTSpace()
{
	g_mikkTContext.m_pInterface = new SMikkTSpaceInterface;
	g_mikkTContext.m_pInterface->m_getNumFaces = GetNumFaces;
	g_mikkTContext.m_pInterface->m_getNumVerticesOfFace = GetNumVerticesInFace;
	g_mikkTContext.m_pInterface->m_getPosition = GetPosition;
	g_mikkTContext.m_pInterface->m_getNormal = GetNormal;
	g_mikkTContext.m_pInterface->m_getTexCoord = GetUVs;
	g_mikkTContext.m_pInterface->m_setTSpaceBasic = SetTSpaceBasic;
	g_mikkTContext.m_pInterface->m_setTSpace = SetTSpace;
}

void InitializeMikkTStamp( int numVertices )
{
	g_mikkTInputStamp.m_numVertices = numVertices;
	g_mikkTInputStamp.m_numFaces = g_mikkTInputStamp.m_numVertices / 3;
	g_mikkTInputStamp.m_positions = reinterpret_cast< float (*)[ 3 ] >( malloc( sizeof( float[ 3 ] ) * g_mikkTInputStamp.m_numVertices ) );
	g_mikkTInputStamp.m_normals = reinterpret_cast< float (*)[ 3 ] >( malloc( sizeof( float[ 3 ] ) * g_mikkTInputStamp.m_numVertices ) );
	g_mikkTInputStamp.m_UVs = reinterpret_cast< float (*)[ 2 ] >( malloc( sizeof( float[ 2 ] ) * g_mikkTInputStamp.m_numVertices ) );
	g_mikkTInputStamp.m_faces = reinterpret_cast< FaceMikkT* >( malloc( sizeof( FaceMikkT ) * g_mikkTInputStamp.m_numFaces ) );
}

void CleanupMikkTSpace()
{
	delete g_mikkTContext.m_pInterface;
	g_mikkTContext.m_pInterface = nullptr;
}

void CleanupMikkTStamp()
{
	free( g_mikkTInputStamp.m_positions );
	g_mikkTInputStamp.m_positions = nullptr;
	free( g_mikkTInputStamp.m_normals );
	g_mikkTInputStamp.m_normals = nullptr;
	free( g_mikkTInputStamp.m_UVs );
	g_mikkTInputStamp.m_UVs = nullptr;
	free( g_mikkTInputStamp.m_faces );
	g_mikkTInputStamp.m_faces = nullptr;
	g_mikkTInputStamp.m_numVertices = 0;
	g_mikkTInputStamp.m_numFaces = 0;
}

void PopulateTangentsMeshBuilderMikkT( MeshBuilder& meshBuilder )
{
	InitializeMikkTSpace();
	InitializeMikkTStamp( meshBuilder.GetVertexCount() );

	for ( unsigned int vertexIndex = 0U; vertexIndex < meshBuilder.GetVertexCount(); vertexIndex ++ )
	{
		VertexBuilder vertex = meshBuilder.GetVertex( vertexIndex );

		g_mikkTInputStamp.m_positions[ vertexIndex ][ 0 ] = vertex.m_position.x; g_mikkTInputStamp.m_positions[ vertexIndex ][ 1 ] = vertex.m_position.y; g_mikkTInputStamp.m_positions[ vertexIndex ][ 2 ] = vertex.m_position.z;
		g_mikkTInputStamp.m_normals[ vertexIndex ][ 0 ] = vertex.m_normal.x; g_mikkTInputStamp.m_normals[ vertexIndex ][ 1 ] = vertex.m_normal.y; g_mikkTInputStamp.m_normals[ vertexIndex ][ 2 ] = vertex.m_normal.z;
		g_mikkTInputStamp.m_UVs[ vertexIndex ][ 0 ] = vertex.m_UVs.x; g_mikkTInputStamp.m_UVs[ vertexIndex ][ 1 ] = vertex.m_UVs.y;
		
		if ( vertexIndex % 3 == 2 ) // Only add a face when all three vertices are in
		{
			unsigned int faceIndex = vertexIndex / 3;
			g_mikkTInputStamp.m_faces[ faceIndex ].m_vertices[ 0 ].m_position = &g_mikkTInputStamp.m_positions[ vertexIndex - 2 ];	g_mikkTInputStamp.m_faces[ faceIndex ].m_vertices[ 1 ].m_position = &g_mikkTInputStamp.m_positions[ vertexIndex - 1 ];	g_mikkTInputStamp.m_faces[ faceIndex ].m_vertices[ 2 ].m_position = &g_mikkTInputStamp.m_positions[ vertexIndex ];
			g_mikkTInputStamp.m_faces[ faceIndex ].m_vertices[ 0 ].m_normal = &g_mikkTInputStamp.m_normals[ vertexIndex - 2 ];		g_mikkTInputStamp.m_faces[ faceIndex ].m_vertices[ 1 ].m_normal = &g_mikkTInputStamp.m_normals[ vertexIndex - 1 ];		g_mikkTInputStamp.m_faces[ faceIndex ].m_vertices[ 2 ].m_normal = &g_mikkTInputStamp.m_normals[ vertexIndex ];
			g_mikkTInputStamp.m_faces[ faceIndex ].m_vertices[ 0 ].m_UVs = &g_mikkTInputStamp.m_UVs[ vertexIndex - 2 ];				g_mikkTInputStamp.m_faces[ faceIndex ].m_vertices[ 1 ].m_UVs = &g_mikkTInputStamp.m_UVs[ vertexIndex - 1 ];				g_mikkTInputStamp.m_faces[ faceIndex ].m_vertices[ 2 ].m_UVs = &g_mikkTInputStamp.m_UVs[ vertexIndex ];
		}
	}

	currentMesh = &meshBuilder;

	genTangSpaceDefault( &g_mikkTContext );

	CleanupMikkTStamp();
	CleanupMikkTSpace();

	currentMesh = nullptr;
}