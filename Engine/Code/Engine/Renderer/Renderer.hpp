#pragma once

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Core/Vertex.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/Texture.hpp"

#pragma comment( lib, "opengl32" )	// Link in the OpenGL32.lib static library

constexpr int CIRCLE_NUM_SIDES = 36;

enum DrawPrimitiveType
{
	PRIMITIVE_TYPE_INVALID = -1,
	LINES,
	LINE_LOOP,
	TRIANGLES,
	QUADS,
	POLYGON,
	NUM_TYPES
};

enum RPolygonType
{
	INVALID = -1,
	REGULAR = 0,
	BROKEN_LINES = 1
};

enum TextDrawMode
{
	TEXT_DRAW_MODE_INVALID = -1,
	TEXT_DRAW_SHRINK_TO_FIT,
	TEXT_DRAW_WORD_WRAP,
	TEXT_DRAW_OVERRUN,
	NUM_MODES
};

enum RendererBlendMode
{
	ALPHA,
	ADDITIVE,
	NUM_BLEND_MODES
};

class Renderer
{

public:
	Renderer();
	~Renderer();

public:
	BitmapFont* CreateOrGetBitmapFont( const char* bitmapFontName );
	SpriteSheet* CreateOrGetSpriteSheetForBitmapFont( const std::string& spriteSheetFilePath );
	Texture* CreateOrGetTexture( const std::string& textureFilePath );
	void InitializeRenderer() const;
	int GetGLDrawPrimitive( const DrawPrimitiveType primitiveType ) const;
	void EnableTexturing( const Texture& texture ) const;
	void DisableTexturing() const;
	void DrawMeshImmediate( const Vertex_3DPCU* verts, int numVerts, DrawPrimitiveType drawPrimitive ) const;
	void DrawLine( const Vector2& startPoint, const Vector2& endPoint, const Rgba& startColor, const Rgba& endColor, const float lineThickness ) const;
	void DrawLineBorder( const AABB2& bounds, const Rgba& color, const float lineThickness ) const;
	void DrawPolygon( const Vector2& location, const float angle, const Vector2 vertices[], const int numSides, const RPolygonType polygonType, const Rgba& color ) const;
	void DrawAABB( const AABB2& bounds, const Rgba& color ) const;
	void DrawAABBBorder( const AABB2& innerBoundsAABB, const Rgba& colorAtInnerBorder, const AABB2& outerBoundsAABB, const Rgba& colorAtOuterBorder ) const;
	void DrawTexturedAABB( const AABB2& bounds, const Texture& texture, const Vector2& textureCoordinatesAtMins, const Vector2& textureCoordinatesAtMaxs, const Rgba& tint ) const;
	void DrawText2D( const Vector2& drawMins, const std::string& asciiText, float cellHeight, const Rgba& tint, float aspectScale, const BitmapFont* font ) const;
	void DrawTextInBox2D( const std::string& asciiText, const AABB2& boxBounds, float cellHeight, const Rgba& tint, float aspectScale, const BitmapFont* font, const TextDrawMode textDrawMode, const Vector2& alignment ) const;
	void ChangeRenderColor( const Rgba& ) const;
	void BeginFrame( float screenWidth, float screenHeight ) const;
	void EndFrame() const;
	void PushMatrix() const;
	void PopMatrix() const;
	void Translate( float xTranslation, float yTranslation, float zTranslation ) const;
	void Rotate( float angleDegrees, float xAxis, float yAxis, float zAxis ) const;
	void Scale( float xScale, float yScale, float zScale ) const;
	void SetOrtho( const Vector2& bottomLeft, const Vector2& topRight ) const;
	void SetBlendMode( RendererBlendMode newBlendMode ) const;
	void ClearScreen( const Rgba& clearColor ) const;

private:
	Vector2 GetAnchorPointForAlignmentWithTextBox( const AABB2& boxBounds, const Vector2& alignment ) const;
	void DrawTextInBox2DOverrun( const std::vector< std::string >& linesOfText, const AABB2& boxBounds, float cellHeight, const Rgba& tint, float aspectScale, const BitmapFont* font, const Vector2& alignment ) const;
	void DrawTextInBox2DShrinkToFit( const std::vector< std::string >& linesOfText, const AABB2& boxBounds, float cellHeight, const Rgba& tint, float aspectScale, const BitmapFont* font, const Vector2& alignment ) const;
	void DrawTextInBox2DWordWrap( std::vector< std::string >& linesOfText, const AABB2& boxBounds, float cellHeight, const Rgba& tint, float aspectScale, const BitmapFont* font, const Vector2& alignment ) const;

private:
	const float CIRCLE_THETA_INCREMENT_DEGREES = static_cast<float>( 360 / CIRCLE_NUM_SIDES );

	std::map< std::string, Texture* > m_loadedTexturesByName;
	std::map< std::string, SpriteSheet* > m_loadedFontSpriteSheetsByName;
	std::map< std::string, BitmapFont* > m_loadedFontsByName;

};
