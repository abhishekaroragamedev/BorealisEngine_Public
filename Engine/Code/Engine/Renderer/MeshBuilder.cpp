#include "Engine/Core/StringUtils.hpp"
#include "Engine/FileUtils/File.hpp"
#include "Engine/Math/IntVector3.hpp"
#include "Engine/Renderer/GLFunctionBinding.hpp"
#include "Engine/Renderer/MeshBuilder.hpp"
#include "Engine/Renderer/MeshBuilderMikkTImplementation.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Tools/DevConsole.hpp"

VertexBuilder::VertexBuilder()
{

}

VertexBuilder::VertexBuilder( Vector3 position, Rgba color, Vector2 UVs )
	:	m_position( position ),
		m_color( color ),
		m_UVs( UVs )
{

}

VertexBuilder::VertexBuilder( Vector3 position, Rgba color, Vector2 UVs, Vector3 normal, Vector4 tangent )
	:	m_position( position ),
		m_color( color ),
		m_UVs( UVs ),
		m_normal( normal ),
		m_tangent( tangent )
{

}

VertexBuilder::VertexBuilder( const VertexBuilder& copy )
{
	m_position = copy.m_position;
	m_color = copy.m_color;
	m_UVs = copy.m_UVs;
	m_normal = copy.m_normal;
	m_tangent = copy.m_tangent;
}

/* static */
Vertex_3DPCU VertexBuilder::MakeVertex3D_PCU( const VertexBuilder& vertexBuilder )
{
	return Vertex_3DPCU( vertexBuilder.m_position, vertexBuilder.m_color, vertexBuilder.m_UVs );
}

/* static */
const std::vector< VertexAttribute > VertexBuilder::s_attributes = std::vector< VertexAttribute >(
{
	VertexAttribute( "POSITION", RendererDataType::RENDER_TYPE_FLOAT, 3, false, offsetof( VertexBuilder, m_position ) ),
	VertexAttribute( "COLOR", RendererDataType::RENDER_TYPE_FLOAT, 4, true, offsetof( VertexBuilder, m_color ) ),
	VertexAttribute( "UV", RendererDataType::RENDER_TYPE_FLOAT, 2, false, offsetof( VertexBuilder, m_UVs ) ),
	VertexAttribute( "NORMAL", RendererDataType::RENDER_TYPE_FLOAT, 3, false, offsetof( VertexBuilder, m_normal ) ),
	VertexAttribute( "TANGENT", RendererDataType::RENDER_TYPE_FLOAT, 4, false, offsetof( VertexBuilder, m_tangent ) )
}
);

/* static */
const VertexLayout VertexBuilder::s_layout = VertexLayout( sizeof( VertexBuilder ), VertexBuilder::s_attributes );

MeshBuilder::MeshBuilder()
{

}

MeshBuilder::MeshBuilder( const MeshBuilder& copy )
{
	m_drawInstruction = DrawInstruction( copy.m_drawInstruction );
	m_vertices = copy.m_vertices;
	m_indices = copy.m_indices;
	m_stamp = VertexBuilder( copy.m_stamp );
}

MeshBuilder::~MeshBuilder()
{

}

void MeshBuilder::Begin( DrawPrimitiveType primitive, bool useIndices /* = true */ )
{
	m_drawInstruction.m_primitive = primitive;
	m_drawInstruction.m_usesIndices = useIndices;

	if ( useIndices )
	{
		m_drawInstruction.m_startIndex = static_cast< unsigned int >( m_indices.size() );
	}
	else
	{
		m_drawInstruction.m_startIndex = static_cast< unsigned int >( m_vertices.size() );
	}
}

void MeshBuilder::End()
{
	unsigned int endIndex;

	if ( m_drawInstruction.m_usesIndices )
	{
		endIndex = static_cast< unsigned int >( m_indices.size() );
	}
	else
	{
		endIndex = static_cast< unsigned int >( m_vertices.size() );
	}

	m_drawInstruction.m_elementCount = endIndex - m_drawInstruction.m_startIndex;
}

void MeshBuilder::SetDrawInstruction( DrawInstruction drawInstruction )
{
	m_drawInstruction = drawInstruction;
}

void MeshBuilder::SetDrawInstruction( DrawPrimitiveType primitive, bool usesIndices, unsigned int startIndex, unsigned int elementCount )
{
	m_drawInstruction = DrawInstruction( primitive, usesIndices, startIndex, elementCount );
}

void MeshBuilder::SetColor( const Rgba& color )
{
	m_stamp.m_color = color;
}

void MeshBuilder::SetUVs( const Vector2& uv )
{
	m_stamp.m_UVs = uv;
}

void MeshBuilder::SetUVs( float u, float v )
{
	m_stamp.m_UVs = Vector2( u, v );
}

void MeshBuilder::SetNormal( const Vector3& normal )
{
	m_stamp.m_normal = normal;
}

void MeshBuilder::SetNormal( float x, float y, float z )
{
	m_stamp.m_normal = Vector3( x, y, z );
}

void MeshBuilder::SetTangent( const Vector4& tangent )
{
	m_stamp.m_tangent = tangent;
}

void MeshBuilder::SetTangent( float x, float y, float z, float w )
{
	m_stamp.m_tangent = Vector4( x, y, z, w );
}

void MeshBuilder::SetTangentAtIndex( unsigned int index, const Vector4& tangent )
{
	m_vertices[ index ].m_tangent = tangent;
}

void MeshBuilder::SetTangentAtIndex( unsigned int index, float x, float y, float z, float w )
{
	SetTangentAtIndex( index, Vector4( x, y, z, w ) );
}

unsigned int MeshBuilder::PushVertex( float x, float y, float z )
{
	return PushVertex( Vector3( x, y, z ) );
}

unsigned int MeshBuilder::PushVertex( const Vector3& position )
{
	m_stamp.m_position = position;
	m_vertices.push_back( m_stamp );

	return static_cast< unsigned int >( m_vertices.size() - 1 );
}

unsigned int MeshBuilder::PushVertex( VertexBuilder vertexBuilder )
{
	m_vertices.push_back( vertexBuilder );
	return static_cast< unsigned int >( m_vertices.size() - 1 );
}

unsigned int MeshBuilder::PushVertices( unsigned int count, const VertexBuilder* vertices )
{
	for ( unsigned int index = 0; index < count; index++ )
	{
		SetColor( vertices[ index ].m_color );
		SetUVs( vertices[ index ].m_UVs );
		SetNormal( vertices[ index ].m_normal );
		SetTangent( vertices[ index ].m_tangent );
		PushVertex( vertices[ index ].m_position );
	}
	return static_cast< unsigned int >( m_vertices.size() - 1 );
}

unsigned int MeshBuilder::PushVertices( unsigned int count, const Vertex_3DPCU* vertices )
{
	for ( unsigned int index = 0; index < count; index++ )
	{
		SetColor( vertices[ index ].m_color );
		SetUVs( vertices[ index ].m_UVs );
		PushVertex( vertices[ index ].m_position );
	}
	return static_cast< unsigned int >( m_vertices.size() - 1 );
}

unsigned int MeshBuilder::PushIndex( unsigned int index )
{
	m_indices.push_back( index );
	return static_cast< unsigned int >( m_indices.size() - 1 );
}

unsigned int MeshBuilder::PushIndices( unsigned int count, const unsigned int* indices )
{
	for ( unsigned int index = 0; index < count; index++ )
	{
		PushIndex( indices[ index ] );
	}
	return static_cast< unsigned int >( m_indices.size() - 1 );
}

unsigned int MeshBuilder::MergeMesh( const MeshBuilder& meshBuilder )
{
	for ( VertexBuilder newVertex : meshBuilder.m_vertices )
	{
		PushVertex( newVertex );
	}

	int maxIndex = GetMaxIndex();
	unsigned int nextStartIndex = static_cast< unsigned int >( maxIndex + 1 );
	for ( unsigned int newIndex : meshBuilder.m_indices )
	{
		PushIndex( nextStartIndex + newIndex );
	}
	return static_cast< unsigned int >( m_vertices.size() - 1 );
}

void MeshBuilder::AddSurfacePatch( std::function<Vector3( float, float )> surfacePatchCallback, const FloatRange& uRange, const FloatRange& vRange, const IntVector2& numSamples, const Vector3& position /* = Vector3::ZERO */, float scale /* = 1.0f */ )
{
	Vector2 numSamplesDivisorF = ConvertIntVector2ToVector2( numSamples );
	Vector2 sampleSize = Vector2( ( uRange.Size() / numSamplesDivisorF.x ), ( vRange.Size() / numSamplesDivisorF.y ) );

	for ( int vIndex = 0; vIndex <= numSamples.y; vIndex++ )
	{
		for ( int uIndex = 0; uIndex <= numSamples.x; uIndex++ )
		{
			float u = uRange.min + ( static_cast< float >( uIndex ) * sampleSize.x );
			float v = vRange.min + ( static_cast< float >( vIndex ) * sampleSize.y );
			SetUVs( u, v );
			SetNormal( GetSurfacePatchNormal( surfacePatchCallback, u, v, sampleSize.x, sampleSize.y ) );
			SetTangent( Vector4( GetSurfacePatchTangent( surfacePatchCallback, u, v, sampleSize.x ), 1.0f ) );
			PushVertex( position + ( scale * surfacePatchCallback( u, v ) ) );
		}
	}

	unsigned int numSamplesX = static_cast< unsigned int >( numSamples.x );
	unsigned int numSamplesY = static_cast< unsigned int >( numSamples.y );
	for ( unsigned int vSampleIndex = 0U; vSampleIndex < numSamplesY; vSampleIndex++ )
	{
		for ( unsigned int uSampleIndex = 0U; uSampleIndex < numSamplesX; uSampleIndex++ )
		{
			unsigned int bottomLeftIndex = ( ( numSamples.x + 1 ) * vSampleIndex ) + uSampleIndex;
			unsigned int topLeftIndex = bottomLeftIndex + ( numSamples.x + 1 );
			unsigned int bottomRightIndex = bottomLeftIndex + 1;
			unsigned int topRightIndex = topLeftIndex + 1;

			unsigned int indices[ 6 ] = { bottomLeftIndex, bottomRightIndex, topLeftIndex, topLeftIndex, bottomRightIndex, topRightIndex };
			PushIndices( 6, indices );
		}
	}
}

Mesh* MeshBuilder::CreateMesh() const
{
	Mesh* mesh = new Mesh;
	mesh->SetDrawInstruction( m_drawInstruction );
	mesh->SetVertices( static_cast< unsigned int >( m_vertices.size() ), m_vertices.data(), &VertexBuilder::s_layout );
	if ( m_drawInstruction.m_usesIndices )
	{
		mesh->SetIndices( static_cast< unsigned int >( m_indices.size() ), m_indices.data() );
	}
	return mesh;
}

void MeshBuilder::PopulateMesh( Mesh& mesh ) const
{
	mesh.SetDrawInstruction( m_drawInstruction );
	mesh.SetVertices( static_cast< unsigned int >( m_vertices.size() ), m_vertices.data(), &VertexBuilder::s_layout );
	if ( m_drawInstruction.m_usesIndices )
	{
		mesh.SetIndices( static_cast< unsigned int >( m_indices.size() ), m_indices.data() );
	}
}

Mesh* MeshBuilder::Flush()
{
	Mesh* mesh = CreateMesh();
	Clear();
	return mesh;
}

void MeshBuilder::RecomputeNormals()
{
	if ( m_drawInstruction.m_primitive != DrawPrimitiveType::TRIANGLES )
	{
		LogErrorf( "ERROR: MeshBuilder::RecomputeNormals(): Only TRIANGLES supported. Function won't do anything." );
		return;
	}

	for ( unsigned int vertIndex = 0U; vertIndex < GetVertexCount(); vertIndex++ )
	{
		float faceCount = 0.0f;
		Vector3 crossProductSum = Vector3::ZERO;

		if ( m_drawInstruction.m_usesIndices )
		{
			unsigned int triangleIndexA = 0U;
			unsigned int indexCount = GetIndexCount();
			while ( triangleIndexA < indexCount )
			{
				unsigned int triangleIndexB = triangleIndexA + 1U;
				unsigned int triangleIndexC = triangleIndexA + 2U;

				if (
					( m_indices[ triangleIndexA ] == vertIndex ) ||
					( m_indices[ triangleIndexB ] == vertIndex ) ||
					( m_indices[ triangleIndexC ] == vertIndex )
					)
				{
					// This face contains the vertex
					Vector3 triangleVertexA = m_vertices[ m_indices[ triangleIndexA ] ].m_position;
					Vector3 triangleVertexB = m_vertices[ m_indices[ triangleIndexB ] ].m_position;
					Vector3 triangleVertexC = m_vertices[ m_indices[ triangleIndexC ] ].m_position;

					Vector3 triangleSideAB = triangleVertexB - triangleVertexA;
					Vector3 triangleSideAC = triangleVertexC - triangleVertexA;

					Vector3 crossProduct = CrossProduct( triangleSideAC, triangleSideAB );
					crossProductSum += crossProduct;
					faceCount += 1.0f;
				}

				triangleIndexA += 3;
			}

			Vector3 averageCrossProduct = Vector3::ZERO;
			if ( faceCount > 0.0f )
			{
				averageCrossProduct = crossProductSum / faceCount;
			}
			m_vertices[ vertIndex ].m_normal = averageCrossProduct.GetNormalized();
		}
		else
		{
			unsigned int triangleVertIndexBase = ( vertIndex / 3 );
			unsigned int triangleVertIndexA = triangleVertIndexBase + 0U;
			unsigned int triangleVertIndexB = triangleVertIndexBase + 1U;
			unsigned int triangleVertIndexC = triangleVertIndexBase + 2U;

			Vector3 triangleVertexA = m_vertices[ triangleVertIndexA ].m_position;
			Vector3 triangleVertexB = m_vertices[ triangleVertIndexB ].m_position;
			Vector3 triangleVertexC = m_vertices[ triangleVertIndexC ].m_position;

			Vector3 triangleSideAB = triangleVertexB - triangleVertexA;
			Vector3 triangleSideAC = triangleVertexC - triangleVertexA;

			Vector3 crossProduct = CrossProduct( triangleSideAC, triangleSideAB );
			m_vertices[ vertIndex ].m_normal = crossProduct.GetNormalized();
		}
	}
}

void MeshBuilder::RecomputeTangents()
{
	PopulateTangentsMeshBuilderMikkT( *this );
}

void MeshBuilder::Clear()
{
	m_stamp = VertexBuilder();
	m_vertices.clear();
	m_indices.clear();
	m_drawInstruction = DrawInstruction();
}

VertexBuilder MeshBuilder::GetVertex( unsigned int index ) const
{
	return m_vertices[ index ];
}

VertexBuilder* MeshBuilder::GetVertexAsReference( unsigned int index )
{
	return &m_vertices[ index ];
}

DrawInstruction MeshBuilder::GetDrawInstruction() const
{
	return m_drawInstruction;
}

unsigned int MeshBuilder::GetVertexCount() const
{
	return static_cast< unsigned int >( m_vertices.size() );
}

unsigned int MeshBuilder::GetIndexCount() const
{
	return static_cast< unsigned int >( m_indices.size() );
}

const std::vector< unsigned int > MeshBuilder::GetIndices() const
{
	return m_indices;
}

int MeshBuilder::GetMaxIndex() const
{
	int maxIndex = -1;
	for ( unsigned int index : m_indices )
	{
		if ( static_cast< int >( index ) > maxIndex )
		{
			maxIndex = static_cast< int >( index );
		}
	}
	return maxIndex;
}

Vector3 MeshBuilder::GetAverageVertexPosition() const
{
	Vector3 averagePosition = Vector3::ZERO;
	float numVerticesF = static_cast< float >( GetVertexCount() );

	for ( VertexBuilder vertex : m_vertices )
	{
		averagePosition += vertex.m_position;
	}

	averagePosition /= numVerticesF;
	return averagePosition;
}

void MeshBuilder::SetVerticesRelativeTo( const Vector3& localOrigin )
{
	for ( size_t vertIndex = 0; vertIndex < m_vertices.size(); vertIndex++ )
	{
		m_vertices[ vertIndex ].m_position = m_vertices[ vertIndex ].m_position - localOrigin;
	}
}

/* static */
std::vector< Mesh* > MeshBuilder::FromFileOBJ( const char* filePath )
{
	std::vector< Mesh* > meshes;

	const char* fileRaw = reinterpret_cast< const char* >( FileReadToNewBuffer( filePath ) );

	TokenizedString fileRawTokenized = TokenizedString( fileRaw, "\n" );
	std::vector< std::string > fileLines = fileRawTokenized.GetTokens();

	MeshBuilder objBuilder;
	objBuilder.SetColor( Rgba::WHITE );

	std::vector< Vector3 > vertices;
	std::vector< Vector3 > normals;
	std::vector< Vector2 > UVs;
	size_t faceCount = 0;
	
	std::vector< std::string > submeshMaterialNames;
	size_t subMeshCount = 1;

	// Populate vertices, normals and UVs; once done (once we come to faces), use the indices found to match them up and generate the mesh
	for ( size_t lineNum = 0; lineNum < fileLines.size(); lineNum++ )
	{
		TokenizedString lineTokenized = TokenizedString( fileLines[ lineNum ], " " );
		std::vector< std::string > tokens = lineTokenized.GetTokensNoNull();

		if ( tokens.size() == 0 )
		{
			continue;
		}

		if ( tokens[ 0 ] == "mtllib" )
		{
			std::string mtlFilePath = GetDirectoryFromFilePath( filePath ) + "/" + tokens[ 1 ];
			submeshMaterialNames = MeshBuilder::GetMaterialNamesFromMTL( mtlFilePath );
		}
		else if ( tokens[ 0 ] == "v" )
		{
			// Vertex; expects a Vector3
			if ( tokens.size() < 4 )
			{
				ConsolePrintf( Rgba::RED, "ERROR: MeshBuilder::FromFileOBJ - Vertex - %s, line %u - expected a Vector3, but did not find 4 tokens. Aborting..." );
				return meshes;
			}
			std::string vec3String = std::string( tokens[ 1 ] + "," + tokens[ 2 ] + "," + tokens[ 3 ] );
			if ( Vector3::IsValidString( vec3String ) )
			{
				Vector3 newVertex;
				newVertex.SetFromText( vec3String );
				newVertex.x *= -1.0f;	// Lightwave OBJ space is right-handed, so flip X
				vertices.push_back( newVertex );
			}
			else
			{
				ConsolePrintf( Rgba::RED, "ERROR: MeshBuilder::FromFileOBJ - Vertex - %s, line %u - invalid Vector3 string. Aborting..." );
				return meshes;
			}
		}
		else if ( tokens[ 0 ] == "vn" )
		{
			// Normal; expects a Vector3
			if ( tokens.size() < 4 )
			{
				ConsolePrintf( Rgba::RED, "ERROR: MeshBuilder::FromFileOBJ - Normal - %s, line %u - expected a Vector3, but did not find 4 tokens. Aborting..." );
				return meshes;
			}
			std::string vec3String = std::string( tokens[ 1 ] + "," + tokens[ 2 ] + "," + tokens[ 3 ] );
			if ( Vector3::IsValidString( vec3String ) )
			{
				Vector3 newNormal;
				newNormal.SetFromText( vec3String );
				newNormal.x *= -1.0f;	// Lightwave OBJ space is right-handed, so flip X
				normals.push_back( newNormal );
			}
			else
			{
				ConsolePrintf( Rgba::RED, "ERROR: MeshBuilder::FromFileOBJ - Normal - %s, line %u - invalid Vector3 string. Aborting..." );
				return meshes;
			}
		}
		else if ( tokens[ 0 ] == "vt" )
		{
			// UV; expects a Vector2
			if ( tokens.size() < 3 )
			{
				ConsolePrintf( Rgba::RED, "ERROR: MeshBuilder::FromFileOBJ - UV - %s, line %u - expected a Vector2, but did not find 3 tokens. Aborting..." );
				return meshes;
			}
			std::string vec2String = std::string( tokens[ 1 ] + "," + tokens[ 2 ] );
			if ( Vector2::IsValidString( vec2String ) )
			{
				Vector2 newUV;
				newUV.SetFromText( vec2String );
				UVs.push_back( newUV );
			}
			else
			{
				ConsolePrintf( Rgba::RED, "ERROR: MeshBuilder::FromFileOBJ - UV - %s, line %u - invalid Vector2 string. Aborting..." );
				return meshes;
			}
		}
		else if ( tokens[ 0 ] == "usemtl" )
		{
			if ( tokens.size() < 2 )
			{
				ConsolePrintf( Rgba::RED, "ERROR: MeshBuilder::FromFileOBJ - UseMtl - expected material name, but did not find one. Aborting..." );
				return meshes;
			}

			if ( objBuilder.GetVertexCount() > 0U )
			{
				PopulateTangentsMeshBuilderMikkT( objBuilder );
				objBuilder.SetDrawInstruction( DrawInstruction( DrawPrimitiveType::TRIANGLES, false, 0, objBuilder.GetVertexCount() ) );
				Mesh* subMesh = objBuilder.Flush();
				meshes.push_back( subMesh );
				objBuilder.SetColor( Rgba::WHITE );
			}
		}
		else if ( tokens[ 0 ] == "f" )
		{
			// There may be 3 or 4 vertices
			std::vector< IntVector3 > vertexUVNormalIndices; // Storing as x = vertex index, y = UV index, z = normal index
			for ( size_t vertIndex = 1; vertIndex < tokens.size(); vertIndex++ )
			{
				TokenizedString tokenizedIndices = TokenizedString( tokens[ vertIndex ], "/" );
				std::vector< std::string > indices = tokenizedIndices.GetTokens();

				// Expects 3 tokens, with the middle one (UV) possibly being ""
				IntVector3 newPoint;
				newPoint.x = stoi( indices[ 0 ] ) - 1;	// These indices start from 1 (ugh!)
				
				if ( indices[ 1 ] != "" )
				{
					newPoint.y = stoi( indices[ 1 ] ) - 1;
				}
				else
				{
					newPoint.y = -1;	// Use this as a placeholder value for the case where UVs need to be reused
				}

				if ( indices.size() > 2 )	// Normal information available
				{
					newPoint.z = stoi( indices[ 2 ] ) - 1;
				}
				else
				{
					newPoint.z = -1;	// Use this as a placeholder value for the case where normals need to be manually computed
				}
				

				vertexUVNormalIndices.push_back( newPoint );
			}

			// First triangle
			unsigned int firstTriangleIndices[ 3 ] = { 0, 1, 2 };

			Vector3 faceNormal;	// Compute the faceNormal using a Cross Product. Assumes left-handed cross product and anti-clockwise winding order
			Vector3 faceVectorFrom = vertices[ vertexUVNormalIndices[ 2 ].x ] - vertices[ vertexUVNormalIndices[ 0 ].x ];
			Vector3 faceVectorTo = vertices[ vertexUVNormalIndices[ 1 ].x ] - vertices[ vertexUVNormalIndices[ 0 ].x ];
			faceNormal = CrossProduct( faceVectorFrom, faceVectorTo ).GetNormalized();

			for ( unsigned int pointIndex : firstTriangleIndices )
			{
				if ( vertexUVNormalIndices[ pointIndex ].y != -1 )
				{
					objBuilder.SetUVs( UVs[ vertexUVNormalIndices[ pointIndex ].y ] );
				}

				if ( vertexUVNormalIndices[ pointIndex ].z != -1 )
				{
					objBuilder.SetNormal( normals[ vertexUVNormalIndices[ pointIndex ].z ] );
				}
				else
				{
					objBuilder.SetNormal( faceNormal );
				}

				objBuilder.PushVertex( vertices[ vertexUVNormalIndices[ pointIndex ].x ] );
			}
			faceCount++;

			if ( vertexUVNormalIndices.size() == 4 )
			{
				// Second triangle
				unsigned int secondTriangleIndices[ 3 ] = { 0, 2, 3 };

				faceVectorFrom = vertices[ vertexUVNormalIndices[ 3 ].x ] - vertices[ vertexUVNormalIndices[ 0 ].x ];
				faceVectorTo = vertices[ vertexUVNormalIndices[ 2 ].x ] - vertices[ vertexUVNormalIndices[ 0 ].x ];
				faceNormal = CrossProduct( faceVectorFrom, faceVectorTo ).GetNormalized();

				for ( unsigned int pointIndex : secondTriangleIndices )
				{
					if ( vertexUVNormalIndices[ pointIndex ].y != -1 )
					{
						objBuilder.SetUVs( UVs[ vertexUVNormalIndices[ pointIndex ].y ] );
					}

					if ( vertexUVNormalIndices[ pointIndex ].z != -1 )
					{
						objBuilder.SetNormal( normals[ vertexUVNormalIndices[ pointIndex ].z ] );
					}
					else
					{
						objBuilder.SetNormal( faceNormal );
					}

					objBuilder.PushVertex( vertices[ vertexUVNormalIndices[ pointIndex ].x ] );
				}
				faceCount++;
			}
		}
	}

	PopulateTangentsMeshBuilderMikkT( objBuilder );

	objBuilder.SetDrawInstruction( DrawInstruction( DrawPrimitiveType::TRIANGLES, false, 0, objBuilder.GetVertexCount() ) );
	meshes.push_back( objBuilder.CreateMesh() );
	return meshes;
}

/* static */
std::vector< Mesh* > MeshBuilder::FromFileOBJ( const std::string& filePath )
{
	return MeshBuilder::FromFileOBJ( filePath.c_str() );
}

/* static */
std::vector< std::string > MeshBuilder::GetMaterialNamesFromMTL( const std::string& filePath )
{
	std::vector< std::string > materialNames;

	const char* fileRaw = reinterpret_cast< const char* >( FileReadToNewBuffer( filePath.c_str() ) );

	TokenizedString fileRawTokenized = TokenizedString( fileRaw, "\n" );
	std::vector< std::string > fileLines = fileRawTokenized.GetTokens();

	for ( size_t lineNum = 0; lineNum < fileLines.size(); lineNum++ )
	{
		TokenizedString lineTokenized = TokenizedString( fileLines[ lineNum ], " " );
		std::vector< std::string > tokens = lineTokenized.GetTokensNoNull();

		if ( tokens.size() == 0 )
		{
			continue;
		}

		if ( tokens[ 0 ] == "newmtl" )
		{
			if ( tokens.size() < 2 )
			{
				ConsolePrintf( Rgba::RED, "ERROR:  MeshBuilder::GetMaterialNamesFromMTL - expected new material string. Aborting..." );
				return materialNames;
			}
			materialNames.push_back( tokens[ 1 ] );
		}
	}

	return materialNames;
}
