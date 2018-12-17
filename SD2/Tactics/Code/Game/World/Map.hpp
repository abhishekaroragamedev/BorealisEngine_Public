#pragma once

#include "Game/World/Block.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Math/IntVector2.hpp"
#include "Engine/Math/IntVector3.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Renderer/Mesh.hpp"
#include "Engine/Renderer/MeshBuilder.hpp"
#include <string>
#include <vector>

constexpr float BLOCK_SIZE_WORLD_UNITS = 1.0f;
constexpr float BLOCK_OUTLINE_SIZE_WORLD_UNITS = 0.001f;
constexpr float CURSOR_HEIGHT_MARGIN_FRACTION = 0.01f;
constexpr char BOX_OUTLINE_SHADER_NAME[] = "BoxOutline";

class Map
{

public:
	Map( const std::string& heightMapFileName, unsigned int maxHeight = 8U );
	~Map(); 

	bool LoadFromHeightMap( const std::string& fileName, unsigned int maxHeight = 8U );
	void DestroyBlocks(); 
	void Render();
	void RenderCursorAt( const IntVector2& mapCoordinates, const Rgba& color = Rgba::WHITE );
	void RenderCursorsAt( std::vector< IntVector2 > mapCoordinates, std::vector< IntVector2 > excludeCoordinates, const Rgba& color = Rgba::WHITE );
	void RenderPaths( const IntVector2& start, unsigned int speed, unsigned int jump, const Rgba& color = Rgba::WHITE );

	unsigned int GetBlockIndex( const IntVector3& position ) const;

	Block* GetBlock( const IntVector3& position ) const;
	void SetBlock( const IntVector3& position, const std::string& type );

	bool IsInBlock( const IntVector3& blockPosition, const Vector3& pointToCheck ) const;
	bool IsInSolidBlock( const Vector3& pointToCheck ) const;

	// Accessors for common map data
	int GetBlockCount() const;
	int GetWidth() const;
	int GetDepth() const;
	int GetHeight() const;

	// Returns he height of the first air block in map_coords
	// (this height may be equal to the height of the map, i.e., not actually 
	// associated with a block.
	int GetHeight( const IntVector2& tilePosition ) const;

	// Convert between a map coordinate and a world coordinate
	Vector3 GetWorldPositionFrom3DMapPosition( const IntVector3& position ) const;		// Assumes (0.0f, 0.0f, 0.0f) is the map origin
	Vector3 GetWorldPositionFrom2DMapPosition( const IntVector2& position2D ) const;		// Assumes (0.0f, 0.0f, 0.0f) is the map origin
	Vector3 GetWorldPositionOnSurfaceFrom2DMapPosition( const IntVector2& position ) const;
	IntVector2 Get2DMapPositionFromWorldPosition( const Vector3& position ) const;
	IntVector3 Get3DMapPositionFromWorldPosition( const Vector3& position ) const;
	IntVector3 Get3DMapPositionFrom2DMapPosition( const IntVector2& position2D ) const;

	// Pathing
	std::vector< IntVector2 > GetTraversableTiles( const IntVector2& start, unsigned int speed, unsigned int jump ) const;
	std::vector< IntVector2 > GetPath( const IntVector2& start, const IntVector2& end, unsigned int speed, unsigned int jump );
	std::vector< IntVector2 > GetNeighboringTiles( const IntVector2& start ) const;

private:
	void InitializeMesh();

public:
	static IntVector2 GetMapCoordinateDirection( const Vector3& cameraForward );

private:
	std::vector< Block* > m_blocks;
	IntVector3 m_dimensions;
	MeshBuilder m_mapMeshBuilder;
	Mesh m_mapMesh;

};
