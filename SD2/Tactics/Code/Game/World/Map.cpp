#include "Game/GameCommon.hpp"
#include "Game/World/Map.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Tools/DevConsole.hpp"
#include "Engine/Renderer/GLFunctionBinding.hpp"
#include <queue>

Map::Map( const std::string& heightMapFileName, unsigned int maxHeight /* = 8U */ )
{
	g_renderer->CreateOrGetShaderProgram( BOX_OUTLINE_SHADER_NAME );
	LoadFromHeightMap( heightMapFileName, maxHeight );
	InitializeMesh();
}

Map::~Map()
{
	DestroyBlocks();
}

bool Map::LoadFromHeightMap( const std::string& fileName, unsigned int maxHeight /* = 8U */ )
{
	Image heightMap = Image( fileName );

	IntVector2 imageDimensions = heightMap.GetDimensions();
	m_dimensions = IntVector3( imageDimensions.x, imageDimensions.y, static_cast< int >( maxHeight ) );

	for ( int imageY = 0; imageY < imageDimensions.y; imageY++ )
	{
		for ( int imageX = 0; imageX < imageDimensions.x; imageX++ )
		{
			float grayScaleTexelValue = static_cast< float >( heightMap.GetTexel( imageX, imageY ).r );
			unsigned char rangeMappedGrayScaleTexelValue = static_cast< unsigned char >( RangeMapFloat( grayScaleTexelValue, 0.0f, 255.0f, 0.0f, ( static_cast< float >( maxHeight ) - 1.0f ) ) );
			Rgba finalTexelValue = Rgba( rangeMappedGrayScaleTexelValue, rangeMappedGrayScaleTexelValue, rangeMappedGrayScaleTexelValue, 255 );

			heightMap.SetTexel( imageX, imageY, finalTexelValue );
		}
	}

	for ( int mapZ = 0; mapZ < m_dimensions.z; mapZ++ )
	{
		for ( int mapY = 0; mapY < m_dimensions.y; mapY++ )
		{
			for ( int mapX = 0; mapX < m_dimensions.x; mapX++ )
			{
				IntVector3 mapCoordinates = IntVector3( mapX, mapY, mapZ );
				BlockDefinition* definitionToUse = nullptr;
				if ( heightMap.GetTexel( mapX, mapY ).r > mapZ )
				{
					definitionToUse = BlockDefinition::GetDefinition( "grass" );
				}
				Block* newBlock = new Block( mapCoordinates, definitionToUse );
				m_blocks.push_back( newBlock );
			}
		}
	}

	return true;
}

void Map::InitializeMesh()
{
	SpriteSheet* terrainSpriteSheet = g_theGame->GetTerrainTextureAtlas();

	m_mapMeshBuilder.Begin( DrawPrimitiveType::TRIANGLES );
	for ( int mapZ = 0; mapZ < m_dimensions.z; mapZ++ )
	{
		for ( int mapY = 0; mapY < m_dimensions.y; mapY++ )
		{
			for ( int mapX = 0; mapX < m_dimensions.x; mapX++ )
			{
				BlockDefinition* currentBlockDefinition = m_blocks[ GetBlockIndex( IntVector3( mapX, mapY, mapZ ) ) ]->GetType();
				if ( currentBlockDefinition != nullptr )
				{
					Vector3 currentWorldPosition = GetWorldPositionFrom3DMapPosition( IntVector3( mapX, mapY, mapZ ) );
					MeshBuilder subMesh = g_renderer->MakeTexturedCubeMesh( currentWorldPosition, ( Vector3::ONE * BLOCK_SIZE_WORLD_UNITS ), *terrainSpriteSheet->GetTexture(), Rgba::WHITE, currentBlockDefinition->GetTopUVs(), currentBlockDefinition->GetSideUVs(), currentBlockDefinition->GetBottomUVs() );
					m_mapMeshBuilder.MergeMesh( subMesh );
				}
			}
		}
	}
	m_mapMeshBuilder.End();
}

void Map::Render()
{
	SpriteSheet* terrainSpriteSheet = g_theGame->GetTerrainTextureAtlas();
	g_renderer->UseShaderProgram( BOX_OUTLINE_SHADER_NAME );
	g_renderer->BindAttribToCurrentShader( "BLOCKSIZE", 1.0f );
	g_renderer->BindAttribToCurrentShader( "THICKNESS", 0.03f );
	g_renderer->BindModelMatrixToCurrentShader( Matrix44::IDENTITY );
	g_renderer->BindTextureAndSampler( 0, *terrainSpriteSheet->GetTexture(), *g_renderer->GetDefaultSampler() );
	m_mapMeshBuilder.PopulateMesh( m_mapMesh );
	g_renderer->DrawMesh( &m_mapMesh );
}

void Map::RenderCursorAt( const IntVector2& mapCoordinates, const Rgba& color /* = Rgba::WHITE */ )
{
	g_renderer->BindTextureAndSampler( 0, *g_renderer->GetDefaultTexture(), *g_renderer->GetDefaultSampler() );

	IntVector3 currentMapCoordinates = IntVector3( mapCoordinates, GetHeight( mapCoordinates ) );
	Vector3 currentWorldCoordinates = GetWorldPositionFrom3DMapPosition( currentMapCoordinates );

	float halfSize = BLOCK_SIZE_WORLD_UNITS * 0.5f;
	float heightMargin = BLOCK_SIZE_WORLD_UNITS * CURSOR_HEIGHT_MARGIN_FRACTION;
	Vertex_3DPCU cursorMesh[ 4 ] = {
		Vertex_3DPCU( Vector3( ( currentWorldCoordinates.x - halfSize ), ( currentWorldCoordinates.y - ( halfSize - heightMargin ) ), ( currentWorldCoordinates.z - halfSize ) ), color, Vector2::ZERO ),
		Vertex_3DPCU( Vector3( ( currentWorldCoordinates.x + halfSize ), ( currentWorldCoordinates.y - ( halfSize - heightMargin ) ), ( currentWorldCoordinates.z - halfSize ) ), color, Vector2::ZERO ),
		Vertex_3DPCU( Vector3( ( currentWorldCoordinates.x - halfSize ), ( currentWorldCoordinates.y - ( halfSize - heightMargin ) ), ( currentWorldCoordinates.z + halfSize ) ), color, Vector2::ZERO ),
		Vertex_3DPCU( Vector3( ( currentWorldCoordinates.x + halfSize ), ( currentWorldCoordinates.y - ( halfSize - heightMargin ) ), ( currentWorldCoordinates.z + halfSize ) ), color, Vector2::ZERO ),
	};
	unsigned int indices[ 6 ] = {
		0, 1, 2, 1, 3, 2
	};
	g_renderer->DrawMeshImmediate( cursorMesh, 4, indices, 6, DrawPrimitiveType::TRIANGLES );
}

void Map::RenderCursorsAt( std::vector< IntVector2 > mapCoordinates, std::vector< IntVector2 > excludeCoordinates, const Rgba& color /* = Rgba::WHITE */ )
{
	for ( IntVector2 excludeCoordinate : excludeCoordinates  )
	{
		while ( std::find( mapCoordinates.begin(), mapCoordinates.end(), excludeCoordinate ) != mapCoordinates.end() )	// Remove the tile and duplicates, if any
		{
			mapCoordinates.erase( std::find( mapCoordinates.begin(), mapCoordinates.end(), excludeCoordinate ) );
		}
	}
	for ( IntVector2 coordinate : mapCoordinates )
	{
		RenderCursorAt( coordinate, color );
	}
}

void Map::RenderPaths( const IntVector2& start, unsigned int speed, unsigned int jump, const Rgba& color /* = Rgba::WHITE */ )
{
	std::vector< IntVector2 > traversibleTiles;
	traversibleTiles = GetTraversableTiles( start, speed, jump );

	for ( IntVector2 traversibleTile : traversibleTiles )
	{
		RenderCursorAt( traversibleTile, color );
	}
}

void Map::DestroyBlocks()
{
	for ( std::vector< Block* >::iterator blockIterator = m_blocks.begin(); blockIterator != m_blocks.end(); blockIterator++ )
	{
		delete *blockIterator;
		*blockIterator = nullptr;
	}
	m_blocks.clear();
}

unsigned int Map::GetBlockIndex( const IntVector3& position ) const
{
	unsigned int blockPosition =	( position.z * ( m_dimensions.y * m_dimensions.x ) )		// Which vertical layer?
									+ ( position.y * m_dimensions.x )							// Which row?
									+ position.x;												// Which column?
	return blockPosition;
}

Block* Map::GetBlock( const IntVector3& position ) const
{
	return m_blocks[ GetBlockIndex( position ) ];
}

void Map::SetBlock( const IntVector3& position, const std::string& type )
{
	m_blocks[ GetBlockIndex( position ) ]->SetType( BlockDefinition::GetDefinition( type ) );
}

bool Map::IsInBlock( const IntVector3& blockPosition, const Vector3& pointToCheck ) const
{
	Vector3 blockCenter = GetWorldPositionFrom3DMapPosition( blockPosition );
	Vector3 blockMins = blockCenter - ( Vector3::ONE * ( BLOCK_SIZE_WORLD_UNITS * 0.5f ) );
	Vector3 blockMaxs = blockCenter + ( Vector3::ONE * ( BLOCK_SIZE_WORLD_UNITS * 0.5f ) );
	
	return IsBetween( pointToCheck, blockMins, blockMaxs );
}

bool Map::IsInSolidBlock( const Vector3& pointToCheck ) const
{
	IntVector3 blockCoordinates = Get3DMapPositionFromWorldPosition( pointToCheck );
	if ( !IsBetween( blockCoordinates, IntVector3::ZERO, IntVector3( GetWidth() - 1, GetDepth() - 1, GetHeight() - 1 ) ) )
	{
		return false;
	}
	return !( m_blocks[ GetBlockIndex( blockCoordinates ) ]->IsAir() );
}

int Map::GetBlockCount() const
{
	return ( ( m_dimensions.x * m_dimensions.y ) * m_dimensions.z );
}

int Map::GetWidth() const
{
	return m_dimensions.x;
}

int Map::GetDepth() const
{
	return m_dimensions.y;
}

int Map::GetHeight() const
{
	return m_dimensions.z;
}

int Map::GetHeight( const IntVector2& tilePosition ) const
{
	int heightOfFirstAirBlock = 0;
	for ( int height = 0; height < m_dimensions.z; height++ )
	{
		if ( m_blocks[ GetBlockIndex( IntVector3( tilePosition, height ) ) ]->IsAir() )
		{
			heightOfFirstAirBlock = height;
			break;
		}
	}
	return heightOfFirstAirBlock;
}

Vector3 Map::GetWorldPositionFrom3DMapPosition( const IntVector3& position ) const		// Returns the center of the block
{
	Vector3 worldPositionWithDifferentAxes = ConvertIntVector3ToVector3( position ) * BLOCK_SIZE_WORLD_UNITS;
	Vector3 worldPosition = Vector3( worldPositionWithDifferentAxes.x, worldPositionWithDifferentAxes.z, worldPositionWithDifferentAxes.y );
	return ( worldPosition * BLOCK_SIZE_WORLD_UNITS ) + ( Vector3::ONE * ( BLOCK_SIZE_WORLD_UNITS * 0.5f ) );
}

Vector3 Map::GetWorldPositionFrom2DMapPosition( const IntVector2& position2D ) const
{
	IntVector3 position = Get3DMapPositionFrom2DMapPosition( position2D );
	return GetWorldPositionFrom3DMapPosition( position );
}

Vector3 Map::GetWorldPositionOnSurfaceFrom2DMapPosition( const IntVector2& position ) const
{
	return GetWorldPositionFrom3DMapPosition( IntVector3( position, GetHeight( position ) ) );
}

IntVector2 Map::Get2DMapPositionFromWorldPosition( const Vector3& position ) const
{
	IntVector3 mapPositionWithDifferentAxes = ConvertVector3ToIntVector3( position / BLOCK_SIZE_WORLD_UNITS ) ;
	IntVector3 mapPosition = IntVector3( mapPositionWithDifferentAxes.x, mapPositionWithDifferentAxes.z, mapPositionWithDifferentAxes.y );
	return IntVector2( mapPosition.x, mapPosition.y );
}

IntVector3 Map::Get3DMapPositionFromWorldPosition( const Vector3& position ) const
{
	IntVector3 mapPositionWithDifferentAxes = ConvertVector3ToIntVector3( position / BLOCK_SIZE_WORLD_UNITS ) ;
	return IntVector3( mapPositionWithDifferentAxes.x, mapPositionWithDifferentAxes.z, mapPositionWithDifferentAxes.y );
}

IntVector3 Map::Get3DMapPositionFrom2DMapPosition( const IntVector2& position2D ) const
{
	return IntVector3( position2D, GetHeight( position2D ) );
}

std::vector< IntVector2 > Map::GetTraversableTiles( const IntVector2& start, unsigned int speed, unsigned int jump ) const
{
	std::vector< IntVector2 > visitedTiles;

	std::queue< IntVector3 > tilesToVisit;	// The "z" value will be the current speed
	tilesToVisit.push( IntVector3( start, speed ) );

	while ( !tilesToVisit.empty() )
	{
		IntVector3 currentElement = tilesToVisit.front();
		IntVector2 currentTile = IntVector2( currentElement.x, currentElement.y );
		int currentSpeed = currentElement.z;
		
		if ( ( std::find( visitedTiles.begin(), visitedTiles.end(), currentTile ) == visitedTiles.end() ) && currentSpeed > 0 )
		{
			for ( IntVector2 neighboringTile : GetNeighboringTiles( currentTile ) )
			{
				int heightDifference = Abs( GetHeight( neighboringTile ) - GetHeight( currentTile ) );
				if ( ( std::find( visitedTiles.begin(), visitedTiles.end(), neighboringTile ) == visitedTiles.end() ) )
				{
					if ( jump >= static_cast< unsigned int >( heightDifference ) )
					{
						tilesToVisit.push( IntVector3( neighboringTile, ( currentSpeed - 1 ) ) );
					}
				}
			}
		}

		tilesToVisit.pop();
		visitedTiles.push_back( currentTile );
	}

	return visitedTiles;
}

std::vector< IntVector2 > Map::GetPath( const IntVector2& start, const IntVector2& end, unsigned int speed, unsigned int jump )
{
	std::vector< IntVector2 > traversableTiles = GetTraversableTiles( start, speed, jump );
	
	std::vector< IntVector2 > path;
	path.push_back( start );

	IntVector2 delta = end - start;
	int maxTries = Abs( delta.x ) + Abs( delta.y );

	IntVector2 currentPosition = start;
	while( currentPosition != end )
	{
		IntVector2 currentDelta = IntVector2::GetAsSigns( end - currentPosition );
		if ( currentDelta.x != 0 && std::find( traversableTiles.begin(), traversableTiles.end(), ( currentPosition + IntVector2( currentDelta.x, 0 ) ) ) != traversableTiles.end() )
		{
			// If an x-difference from the target exists, and the tile at the next x-position is traversable
			currentPosition = currentPosition + IntVector2( currentDelta.x, 0 );
			path.push_back( currentPosition );
		}
		else if ( currentDelta.y != 0 && std::find( traversableTiles.begin(), traversableTiles.end(), ( currentPosition + IntVector2( 0, currentDelta.y ) ) ) != traversableTiles.end() )
		{
			// If an x-difference from the target exists, and the tile at the next y-position is traversable
			currentPosition = currentPosition + IntVector2( 0, currentDelta.y );
			path.push_back( currentPosition );
		}
		else if ( currentDelta == IntVector2::ZERO && currentPosition != end )
		{
			ConsolePrintf( Rgba::RED, "ERROR: Map::GetPath: Cannot find path to destination - delta is zero, but destination not found." );
			path.clear();
			return path;
		}
		else if ( maxTries < 0 )
		{
			ConsolePrintf( Rgba::RED, "ERROR: Map::GetPath: Cannot find path to destination - max tries exceeded." );
			path.clear();
			return path;
		}

		maxTries--;
	}

	return path;
}

std::vector< IntVector2 > Map::GetNeighboringTiles( const IntVector2& start ) const
{
	std::vector< IntVector2 > neighboringTiles;

	if ( ( start.x + 1 ) < m_dimensions.x )
	{
		neighboringTiles.push_back( IntVector2( ( start.x + 1 ), start.y ) );
	}
	if ( ( start.x - 1 ) >= 0 )
	{
		neighboringTiles.push_back( IntVector2( ( start.x - 1 ), start.y ) );
	}
	if ( ( start.y + 1 ) < m_dimensions.y )
	{
		neighboringTiles.push_back( IntVector2( start.x, ( start.y + 1 ) ) );
	}
	if ( ( start.y - 1 ) >= 0 )
	{
		neighboringTiles.push_back( IntVector2( start.x, ( start.y - 1 ) ) );
	}

	return neighboringTiles;
}

IntVector2 Map::GetMapCoordinateDirection( const Vector3& cameraForward )
{
	Vector3 possibleWorldSpaceDirections[ 8 ] = {
		Vector3( 1.0f, 0.0f, 0.0f ),	// EAST
		Vector3( 0.0f, 0.0f, 1.0f ),	// NORTH
		Vector3( -1.0f, 0.0f, 0.0f ),	// WEST
		Vector3( 0.0f, 0.0f, -1.0f )	// SOUTH
	};

	float greatestDotProduct = 0.0f;
	Vector3 greatestCandidateVector = possibleWorldSpaceDirections[ 0 ];
	for ( Vector3 candidateVector : possibleWorldSpaceDirections )
	{
		float dotProduct = DotProduct( cameraForward, candidateVector );
		if ( dotProduct > greatestDotProduct )
		{
			greatestDotProduct = dotProduct;
			greatestCandidateVector = candidateVector;
		}
	}

	IntVector3 intVector3 = ConvertVector3ToIntVector3( greatestCandidateVector );

	return IntVector2( intVector3.x, intVector3.z );
}
