#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include <gl/gl.h>

Renderer::Renderer()
{

}

Renderer::~Renderer()
{
	for ( std::map< std::string, BitmapFont* >::iterator mapIterator = m_loadedFontsByName.begin(); mapIterator != m_loadedFontsByName.end(); mapIterator++ )
	{
		if ( mapIterator->second != nullptr )		// Deallocate memory for BitmapFont objects (in main memory)
		{
			delete mapIterator->second;
			mapIterator->second = nullptr;
		}
	}

	for ( std::map< std::string, SpriteSheet* >::iterator mapIterator = m_loadedFontSpriteSheetsByName.begin(); mapIterator != m_loadedFontSpriteSheetsByName.end(); mapIterator++ )
	{
		if ( mapIterator->second != nullptr )		// Deallocate memory for SpriteSheet objects (in main memory)
		{
			delete mapIterator->second;
			mapIterator->second = nullptr;
		}
	}

	for ( std::map< std::string, Texture* >::iterator mapIterator = m_loadedTexturesByName.begin(); mapIterator != m_loadedTexturesByName.end(); mapIterator++ )
	{
		if ( mapIterator->second != nullptr )		// Deallocate memory for Texture objects (in main memory)
		{
			delete mapIterator->second;
			mapIterator->second = nullptr;
		}
	}
}

void Renderer::InitializeRenderer() const
{
	glLineWidth( 1.5f );
	glEnable( GL_BLEND );
	glEnable( GL_LINE_SMOOTH );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
}

void Renderer::BeginFrame( const float screenWidth, const float screenHeight ) const
{
	
}

void Renderer::EndFrame() const
{
	HWND activeWindow = GetActiveWindow();
	HDC activeDeviceDisplayContext = GetDC( activeWindow );
	SwapBuffers( activeDeviceDisplayContext );
}

BitmapFont* Renderer::CreateOrGetBitmapFont( const char* bitmapFontName )
{
	std::string bitmapFontNameAsString = std::string( bitmapFontName );

	std::string fullBitmapFontFilePath = BITMAP_FONT_PATH + std::string( bitmapFontName ) + BITMAP_FONT_EXTENSION;
	SpriteSheet* spriteSheetForFont = CreateOrGetSpriteSheetForBitmapFont( fullBitmapFontFilePath );

	if ( m_loadedFontsByName.find( bitmapFontNameAsString ) == m_loadedFontsByName.end() )
	{
		m_loadedFontsByName[ bitmapFontNameAsString ] = new BitmapFont( *spriteSheetForFont, 1.0f );		// TODO: Revise base aspect if needed
	}

	return m_loadedFontsByName[ bitmapFontNameAsString ];
}

SpriteSheet* Renderer::CreateOrGetSpriteSheetForBitmapFont( const std::string& spriteSheetFilePath )
{
	Texture* fontTexture = CreateOrGetTexture( spriteSheetFilePath );

	if ( m_loadedFontSpriteSheetsByName.find( spriteSheetFilePath ) == m_loadedFontSpriteSheetsByName.end() )
	{
		m_loadedFontSpriteSheetsByName[ spriteSheetFilePath ] = new SpriteSheet( *fontTexture, BITMAP_FONT_SPRITESHEET_WIDTH_TILES, BITMAP_FONT_SPRITESHEET_HEIGHT_TILES );
	}

	return m_loadedFontSpriteSheetsByName[ spriteSheetFilePath ];
}

Texture* Renderer::CreateOrGetTexture( const std::string& textureFilePath )
{
	if ( m_loadedTexturesByName.find( textureFilePath ) == m_loadedTexturesByName.end() )
	{
		m_loadedTexturesByName[ textureFilePath ] = new Texture( textureFilePath );
	}

	return m_loadedTexturesByName[ textureFilePath ];
}

int Renderer::GetGLDrawPrimitive( const DrawPrimitiveType primitiveType ) const
{
	switch ( primitiveType )
	{
		case DrawPrimitiveType::LINES		:	return GL_LINES;
		case DrawPrimitiveType::LINE_LOOP	:	return GL_LINE_LOOP;
		case DrawPrimitiveType::TRIANGLES	:	return GL_TRIANGLES;
		case DrawPrimitiveType::QUADS		:	return GL_QUADS;
		case DrawPrimitiveType::POLYGON		:	return GL_POLYGON;
		default								:	return GL_LINES;		// TODO: Revise this if needed
	}
}

void Renderer::EnableTexturing( const Texture& texture ) const
{
	glEnable( GL_TEXTURE_2D );
	glBindTexture( GL_TEXTURE_2D, texture.m_textureID );
}

void Renderer::DisableTexturing() const
{
	glDisable(GL_TEXTURE_2D);
}

void Renderer::DrawMeshImmediate( const Vertex_3DPCU* verts, int numVerts, DrawPrimitiveType drawPrimitive ) const
{
	glEnableClientState( GL_VERTEX_ARRAY );
	glEnableClientState( GL_COLOR_ARRAY );
	glEnableClientState( GL_TEXTURE_COORD_ARRAY );

	glVertexPointer( 3, GL_FLOAT, sizeof( Vertex_3DPCU ), &verts[ 0 ].m_position );
	glColorPointer( 4, GL_UNSIGNED_BYTE, sizeof( Vertex_3DPCU ), &verts[ 0 ].m_color );
	glTexCoordPointer( 2, GL_FLOAT, sizeof( Vertex_3DPCU ), &verts[ 0 ].m_UVs );
	glDrawArrays( GetGLDrawPrimitive( drawPrimitive ), 0, numVerts );

	glDisableClientState( GL_TEXTURE_COORD_ARRAY );
	glDisableClientState( GL_COLOR_ARRAY );
	glDisableClientState( GL_VERTEX_ARRAY );
}

void Renderer::DrawLine( const Vector2& startPoint, const Vector2& endPoint, const Rgba& startColor, const Rgba& endColor, const float lineThickness ) const
{
	DisableTexturing();
	glLineWidth( lineThickness );

	Vertex_3DPCU verts[ 2 ] = {
		Vertex_3DPCU( ConvertVector2ToVector3( startPoint ), startColor, Vector2::ZERO ),
		Vertex_3DPCU( ConvertVector2ToVector3( endPoint ), endColor, Vector2::ZERO )
	};

	DrawMeshImmediate( verts, 2, DrawPrimitiveType::LINES );
}

void Renderer::DrawLineBorder( const AABB2& bounds, const Rgba& color, const float lineThickness ) const
{
	DisableTexturing();
	glLineWidth( lineThickness );

	Vertex_3DPCU verts[ 4 ] = {
		Vertex_3DPCU( ConvertVector2ToVector3( bounds.mins ), color, Vector2::ZERO ),
		Vertex_3DPCU( ConvertVector2ToVector3( Vector2( bounds.maxs.x, bounds.mins.y ) ), color, Vector2::ZERO ),
		Vertex_3DPCU( ConvertVector2ToVector3( bounds.maxs ), color, Vector2::ZERO ),
		Vertex_3DPCU( ConvertVector2ToVector3( Vector2( bounds.mins.x, bounds.maxs.y ) ), color, Vector2::ZERO )
	};

	DrawMeshImmediate( verts, 4, DrawPrimitiveType::LINE_LOOP );
}

void Renderer::DrawPolygon( const Vector2& location, const float angle, const Vector2 vertices[], const int numSides, RPolygonType polygonType, const Rgba& color ) const
{
	DisableTexturing();
	if (numSides == 0)
	{
		return;
	}

	float theta = 0.0f;
	float deltaTheta = 360.0f / static_cast<float>( numSides );

	PushMatrix();		// Perform the rendering in the polygon's local space
	Translate( location.x, location.y, 0.0f );
	Rotate( angle, 0.0f, 0.0f, 1.0f );

	Vertex_3DPCU* verts = new Vertex_3DPCU[ numSides * 2 ];

	for ( int i = 0; i < numSides; i++ )
	{
		if ( ( polygonType != RPolygonType::BROKEN_LINES ) || ( i % 2 == 0 ) )
		{
			verts[ 2 * i ].m_position = Vector3( vertices[ ( i % numSides ) ].x, vertices[ ( i % numSides ) ].y, 0.0f );
			verts[ 2 * i ].m_color = color;

			verts[ ( 2 * i ) + 1 ].m_position = Vector3( vertices[ ( ( i + 1 ) % numSides ) ].x, vertices[ ( ( i + 1 ) % numSides ) ].y, 0.0f );
			verts[ ( 2 * i ) + 1 ].m_color = color;
		}
	}

	DrawMeshImmediate( verts, ( numSides * 2 ), DrawPrimitiveType::LINES );

	delete[] verts;
	verts = nullptr;

	PopMatrix();
}

void Renderer::DrawAABB( const AABB2& bounds, const Rgba& color ) const
{
	DisableTexturing();

	Vertex_3DPCU verts[ 4 ] = {
		Vertex_3DPCU( Vector3( bounds.mins.x, bounds.mins.y, 0.0f ), color, Vector2::ZERO ),
		Vertex_3DPCU( Vector3( bounds.maxs.x, bounds.mins.y, 0.0f ), color, Vector2::ZERO ),
		Vertex_3DPCU( Vector3( bounds.maxs.x, bounds.maxs.y, 0.0f ), color, Vector2::ZERO ),
		Vertex_3DPCU( Vector3( bounds.mins.x, bounds.maxs.y, 0.0f ), color, Vector2::ZERO )
	};

	DrawMeshImmediate( verts, 4, DrawPrimitiveType::QUADS );
}

void Renderer::DrawAABBBorder( const AABB2& innerBoundsAABB, const Rgba& colorAtInnerBorder, const AABB2& outerBoundsAABB, const Rgba& colorAtOuterBorder ) const
{
	DisableTexturing();

	// Draw 8 AABBs for the sides and corners

	// Draw sides
	AABB2 leftAABB = AABB2( Vector2( outerBoundsAABB.mins.x, innerBoundsAABB.mins.y ), Vector2( innerBoundsAABB.mins.x, innerBoundsAABB.maxs.y ) );
	Vertex_3DPCU verts[ 4 ] = {
		Vertex_3DPCU( Vector3( leftAABB.mins.x, leftAABB.mins.y, 0.0f ), colorAtOuterBorder, Vector2::ZERO ),
		Vertex_3DPCU( Vector3( leftAABB.maxs.x, leftAABB.mins.y, 0.0f ), colorAtInnerBorder, Vector2::ZERO ),
		Vertex_3DPCU( Vector3( leftAABB.maxs.x, leftAABB.maxs.y, 0.0f ), colorAtInnerBorder, Vector2::ZERO ),
		Vertex_3DPCU( Vector3( leftAABB.mins.x, leftAABB.maxs.y, 0.0f ), colorAtOuterBorder, Vector2::ZERO )
	};
	DrawMeshImmediate( verts, 4, DrawPrimitiveType::QUADS );

	AABB2 topAABB = AABB2( Vector2( innerBoundsAABB.mins.x, innerBoundsAABB.maxs.y ), Vector2( innerBoundsAABB.maxs.x, outerBoundsAABB.maxs.y ) );
	verts[ 0 ] = Vertex_3DPCU( Vector3( topAABB.mins.x, topAABB.mins.y, 0.0f ), colorAtInnerBorder, Vector2::ZERO );
	verts[ 1 ] = Vertex_3DPCU( Vector3( topAABB.maxs.x, topAABB.mins.y, 0.0f ), colorAtInnerBorder, Vector2::ZERO );
	verts[ 2 ] = Vertex_3DPCU( Vector3( topAABB.maxs.x, topAABB.maxs.y, 0.0f ), colorAtOuterBorder, Vector2::ZERO );
	verts[ 3 ] = Vertex_3DPCU( Vector3( topAABB.mins.x, topAABB.maxs.y, 0.0f ), colorAtOuterBorder, Vector2::ZERO );
	DrawMeshImmediate( verts, 4, DrawPrimitiveType::QUADS );

	AABB2 rightAABB = AABB2( Vector2( innerBoundsAABB.maxs.x, innerBoundsAABB.mins.y ), Vector2( outerBoundsAABB.maxs.x, innerBoundsAABB.maxs.y ) );
	verts[ 0 ] = Vertex_3DPCU( Vector3( rightAABB.mins.x, rightAABB.mins.y, 0.0f ), colorAtInnerBorder, Vector2::ZERO );
	verts[ 1 ] = Vertex_3DPCU( Vector3( rightAABB.maxs.x, rightAABB.mins.y, 0.0f ), colorAtOuterBorder, Vector2::ZERO );
	verts[ 2 ] = Vertex_3DPCU( Vector3( rightAABB.maxs.x, rightAABB.maxs.y, 0.0f ), colorAtOuterBorder, Vector2::ZERO );
	verts[ 3 ] = Vertex_3DPCU( Vector3( rightAABB.mins.x, rightAABB.maxs.y, 0.0f ), colorAtInnerBorder, Vector2::ZERO );
	DrawMeshImmediate( verts, 4, DrawPrimitiveType::QUADS );

	AABB2 bottomAABB = AABB2( Vector2( innerBoundsAABB.mins.x, outerBoundsAABB.mins.y ), Vector2( innerBoundsAABB.maxs.x, innerBoundsAABB.mins.y ) );
	verts[ 0 ] = Vertex_3DPCU( Vector3( bottomAABB.mins.x, bottomAABB.mins.y, 0.0f ), colorAtOuterBorder, Vector2::ZERO );
	verts[ 1 ] = Vertex_3DPCU( Vector3( bottomAABB.maxs.x, bottomAABB.mins.y, 0.0f ), colorAtOuterBorder, Vector2::ZERO );
	verts[ 2 ] = Vertex_3DPCU( Vector3( bottomAABB.maxs.x, bottomAABB.maxs.y, 0.0f ), colorAtInnerBorder, Vector2::ZERO );
	verts[ 3 ] = Vertex_3DPCU( Vector3( bottomAABB.mins.x, bottomAABB.maxs.y, 0.0f ), colorAtInnerBorder, Vector2::ZERO );
	DrawMeshImmediate( verts, 4, DrawPrimitiveType::QUADS );

	// Draw corners
	AABB2 bottomLeftAABB = AABB2( outerBoundsAABB.mins, innerBoundsAABB.mins );
	verts[ 0 ] = Vertex_3DPCU( Vector3( bottomLeftAABB.mins.x, bottomLeftAABB.mins.y, 0.0f ), colorAtOuterBorder, Vector2::ZERO );
	verts[ 1 ] = Vertex_3DPCU( Vector3( bottomLeftAABB.maxs.x, bottomLeftAABB.mins.y, 0.0f ), colorAtOuterBorder, Vector2::ZERO );
	verts[ 2 ] = Vertex_3DPCU( Vector3( bottomLeftAABB.maxs.x, bottomLeftAABB.maxs.y, 0.0f ), colorAtInnerBorder, Vector2::ZERO );
	verts[ 3 ] = Vertex_3DPCU( Vector3( bottomLeftAABB.mins.x, bottomLeftAABB.maxs.y, 0.0f ), colorAtOuterBorder, Vector2::ZERO );
	DrawMeshImmediate( verts, 4, DrawPrimitiveType::QUADS );
	
	AABB2 topLeftAABB = AABB2( Vector2( outerBoundsAABB.mins.x, innerBoundsAABB.maxs.y ), Vector2( innerBoundsAABB.mins.x, outerBoundsAABB.maxs.y ) );
	verts[ 0 ] = Vertex_3DPCU( Vector3( topLeftAABB.mins.x, topLeftAABB.mins.y, 0.0f ), colorAtOuterBorder, Vector2::ZERO );
	verts[ 1 ] = Vertex_3DPCU( Vector3( topLeftAABB.maxs.x, topLeftAABB.mins.y, 0.0f ), colorAtInnerBorder, Vector2::ZERO );
	verts[ 2 ] = Vertex_3DPCU( Vector3( topLeftAABB.maxs.x, topLeftAABB.maxs.y, 0.0f ), colorAtOuterBorder, Vector2::ZERO );
	verts[ 3 ] = Vertex_3DPCU( Vector3( topLeftAABB.mins.x, topLeftAABB.maxs.y, 0.0f ), colorAtOuterBorder, Vector2::ZERO );
	DrawMeshImmediate( verts, 4, DrawPrimitiveType::QUADS );

	AABB2 topRightAABB = AABB2( innerBoundsAABB.maxs, outerBoundsAABB.maxs );
	verts[ 0 ] = Vertex_3DPCU( Vector3( topRightAABB.mins.x, topRightAABB.mins.y, 0.0f ), colorAtInnerBorder, Vector2::ZERO );
	verts[ 1 ] = Vertex_3DPCU( Vector3( topRightAABB.maxs.x, topRightAABB.mins.y, 0.0f ), colorAtOuterBorder, Vector2::ZERO );
	verts[ 2 ] = Vertex_3DPCU( Vector3( topRightAABB.maxs.x, topRightAABB.maxs.y, 0.0f ), colorAtOuterBorder, Vector2::ZERO );
	verts[ 3 ] = Vertex_3DPCU( Vector3( topRightAABB.mins.x, topRightAABB.maxs.y, 0.0f ), colorAtOuterBorder, Vector2::ZERO );
	DrawMeshImmediate( verts, 4, DrawPrimitiveType::QUADS );

	AABB2 bottomRightAABB = AABB2( Vector2( innerBoundsAABB.maxs.x, outerBoundsAABB.mins.y ), Vector2( outerBoundsAABB.maxs.x, innerBoundsAABB.mins.y ) );
	verts[ 0 ] = Vertex_3DPCU( Vector3( bottomRightAABB.mins.x, bottomRightAABB.mins.y, 0.0f ), colorAtOuterBorder, Vector2::ZERO );
	verts[ 1 ] = Vertex_3DPCU( Vector3( bottomRightAABB.maxs.x, bottomRightAABB.mins.y, 0.0f ), colorAtOuterBorder, Vector2::ZERO );
	verts[ 2 ] = Vertex_3DPCU( Vector3( bottomRightAABB.maxs.x, bottomRightAABB.maxs.y, 0.0f ), colorAtOuterBorder, Vector2::ZERO );
	verts[ 3 ] = Vertex_3DPCU( Vector3( bottomRightAABB.mins.x, bottomRightAABB.maxs.y, 0.0f ), colorAtInnerBorder, Vector2::ZERO );
	DrawMeshImmediate( verts, 4, DrawPrimitiveType::QUADS );
}

void Renderer::DrawTexturedAABB( const AABB2& bounds, const Texture& texture, const Vector2& textureCoordinatesAtMins, const Vector2& textureCoordinatesAtMaxs, const Rgba& tint ) const
{
	EnableTexturing( texture );

	Vertex_3DPCU verts[ 4 ] = {
		Vertex_3DPCU( Vector3( bounds.mins.x, bounds.maxs.y, 0.0f ), tint, Vector2( textureCoordinatesAtMins.x, textureCoordinatesAtMins.y ) ),
		Vertex_3DPCU( Vector3( bounds.maxs.x, bounds.maxs.y, 0.0f ), tint, Vector2( textureCoordinatesAtMaxs.x, textureCoordinatesAtMins.y ) ),
		Vertex_3DPCU( Vector3( bounds.maxs.x, bounds.mins.y, 0.0f ), tint, Vector2( textureCoordinatesAtMaxs.x, textureCoordinatesAtMaxs.y ) ),
		Vertex_3DPCU( Vector3( bounds.mins.x, bounds.mins.y, 0.0f ), tint, Vector2( textureCoordinatesAtMins.x, textureCoordinatesAtMaxs.y ) )
	};
	DrawMeshImmediate( verts, 4, DrawPrimitiveType::QUADS );

	DisableTexturing();
}

#pragma region Text2DTools

void Renderer::DrawText2D( const Vector2& drawMins, const std::string& asciiText, float cellHeight, const Rgba& tint = Rgba::WHITE, float aspectScale = 1.0f, const BitmapFont* font = nullptr ) const
{
	EnableTexturing( *font->m_spriteSheet.GetTexture() );

	float characterWidth = font->GetCharacterWidth( cellHeight, aspectScale );

	for ( size_t currentCharacterIndex = 0; currentCharacterIndex < asciiText.length(); currentCharacterIndex++ )
	{
		AABB2 characterUvs = font->GetUVsForGlyph( asciiText[ currentCharacterIndex ] );
		Vector2 characterBoundsMins = drawMins + Vector2( ( static_cast<float>( currentCharacterIndex ) * characterWidth ), 0.0f );
		Vector2 characterBoundsMaxs = drawMins + Vector2( ( static_cast<float>( currentCharacterIndex + 1 ) * characterWidth ), cellHeight );
		AABB2 characterBounds = AABB2( characterBoundsMins, characterBoundsMaxs );

		DrawTexturedAABB( characterBounds, *( font->m_spriteSheet.GetTexture() ), characterUvs.mins, characterUvs.maxs, tint );
	}

	DisableTexturing();
}

void Renderer::DrawTextInBox2D( const std::string& asciiText, const AABB2& boxBounds, float cellHeight, const Rgba& tint = Rgba::WHITE, float aspectScale = 1.0f, const BitmapFont* font = nullptr, const TextDrawMode textDrawMode = TextDrawMode::TEXT_DRAW_WORD_WRAP, const Vector2& alignment = Vector2( 0.5f, 0.5f ) ) const
{
	TokenizedString tokenizedText = TokenizedString( asciiText, "\n" );
	std::vector< std::string > linesOfText = tokenizedText.GetTokens();

	switch( textDrawMode )
	{
		case TextDrawMode::TEXT_DRAW_OVERRUN:
		{
			DrawTextInBox2DOverrun( linesOfText, boxBounds, cellHeight, tint, aspectScale, font, alignment );
			break;
		}
		case TextDrawMode::TEXT_DRAW_SHRINK_TO_FIT:
		{
			DrawTextInBox2DShrinkToFit( linesOfText, boxBounds, cellHeight, tint, aspectScale, font, alignment );
			break;
		}
		case TextDrawMode::TEXT_DRAW_WORD_WRAP:
		{
			DrawTextInBox2DWordWrap( linesOfText, boxBounds, cellHeight, tint, aspectScale, font, alignment );
			break;
		}
	}
}

Vector2 Renderer::GetAnchorPointForAlignmentWithTextBox( const AABB2& boxBounds, const Vector2& alignment ) const
{
	Vector2 anchorPoint = Vector2( 
		Interpolate( boxBounds.mins.x, boxBounds.maxs.x, alignment.x ),
		Interpolate( boxBounds.maxs.y, boxBounds.mins.y, alignment.y )
	);

	return anchorPoint;
}

void Renderer::DrawTextInBox2DOverrun( const std::vector< std::string >& linesOfText, const AABB2& boxBounds, float cellHeight, const Rgba& tint, float aspectScale, const BitmapFont* font, const Vector2& alignment ) const
{
	for ( unsigned int lineIndex = 0; lineIndex < linesOfText.size(); lineIndex++ )
	{
		Vector2 drawMins;
		Vector2 anchorPoint = GetAnchorPointForAlignmentWithTextBox( boxBounds, alignment );

		float lineWidth = font->GetStringWidth( linesOfText[ lineIndex ], cellHeight, aspectScale );

		drawMins.x	=	anchorPoint.x -		// left, center or right
						( alignment.x		// 0 for left, 0.5 for center, 1 for right
						* lineWidth );		// drawMins.x will be equal to the bounds.mins.x if left aligned

		// Calculate drawMins.y based on line height, line number, bounds.y and alignment
		drawMins.y	=	( anchorPoint.y +	// top, center or bottom
							( alignment.y *	// 0 for top, 0.5 for center, 1 for bottom 
								( cellHeight * static_cast< float >( linesOfText.size() ) )
							)
						)					// The Y value, as calculated till here, is the maxs.y for the first (topmost) line of text
						- ( static_cast< float >( lineIndex + 1 ) * cellHeight );

		DrawText2D( drawMins, linesOfText[ lineIndex ], cellHeight, tint, aspectScale, font );
	}
}

void Renderer::DrawTextInBox2DShrinkToFit( const std::vector< std::string >& linesOfText, const AABB2& boxBounds, float cellHeight, const Rgba& tint, float aspectScale, const BitmapFont* font, const Vector2& alignment ) const
{
	float maxLineWidth = 0.0f;
	for ( unsigned int lineIndex = 0; lineIndex < linesOfText.size(); lineIndex++ )
	{
		float lineWidth = font->GetStringWidth( linesOfText[ lineIndex ], cellHeight, aspectScale );
		if ( lineWidth > maxLineWidth )
		{
			maxLineWidth = lineWidth;
		}
	}
	if ( maxLineWidth > boxBounds.GetDimensions().x )
	{
		cellHeight *= boxBounds.GetDimensions().x / maxLineWidth;		// Compress text based on max horizontal width
	}
	float totalTextHeight = cellHeight * static_cast< float > ( linesOfText.size() );
	if ( totalTextHeight > boxBounds.GetDimensions().y )
	{
		cellHeight *= boxBounds.GetDimensions().y / totalTextHeight;
	}

	for ( unsigned int lineIndex = 0; lineIndex < linesOfText.size(); lineIndex++ )
	{
		Vector2 drawMins;
		Vector2 anchorPoint = GetAnchorPointForAlignmentWithTextBox( boxBounds, alignment );

		float lineWidth = font->GetStringWidth( linesOfText[ lineIndex ], cellHeight, aspectScale );

		drawMins.x	=	anchorPoint.x -		// left, center or right
			( alignment.x		// 0 for left, 0.5 for center, 1 for right
				* lineWidth );		// drawMins.x will be equal to the bounds.mins.x if left aligned

									// Calculate drawMins.y based on line height, line number, bounds.y and alignment
		drawMins.y	=	( anchorPoint.y +	// top, center or bottom
			( alignment.y *	// 0 for top, 0.5 for center, 1 for bottom 
			( cellHeight * static_cast< float >( linesOfText.size() ) )
				)
			)					// The Y value, as calculated till here, is the maxs.y for the first (topmost) line of text
			- ( static_cast< float >( lineIndex + 1 ) * cellHeight );

		DrawText2D( drawMins, linesOfText[ lineIndex ], cellHeight, tint, aspectScale, font );
	}
}

void Renderer::DrawTextInBox2DWordWrap( std::vector< std::string >& linesOfText, const AABB2& boxBounds, float cellHeight, const Rgba& tint, float aspectScale, const BitmapFont* font, const Vector2& alignment ) const
{
	for ( unsigned int lineIndex = 0; lineIndex < linesOfText.size(); lineIndex++ )		// CAUTION: This loop may resize the vector "linesOfText"!
	{
		float lineWidth = font->GetStringWidth( linesOfText[ lineIndex ], cellHeight, aspectScale );

		if ( lineWidth > boxBounds.GetDimensions().x )
		{
			TokenizedString lineOfTextTokenized = TokenizedString( linesOfText[ lineIndex ], " " );
			std::vector< std::string > wordsInLine = lineOfTextTokenized.GetTokens();
			float currentLength = font->GetStringWidth( wordsInLine[ 0 ], cellHeight, aspectScale );	// We don't have to do anything if the first word is too long
			unsigned int currentCharLength = wordsInLine[ 0 ].size();

			for ( unsigned int wordIndex = 1; wordIndex < wordsInLine.size(); wordIndex++ )		// Start from 1 since we've already calculated the first word's length
			{
				float newLength = currentLength + font->GetStringWidth( ( " " + wordsInLine[ wordIndex ] ), cellHeight, aspectScale );
				unsigned int newCharLength = currentCharLength + 1 + wordsInLine[ wordIndex ].size();
				if ( newLength > boxBounds.GetDimensions().x )
				{
					// Push the substring from the beginning of this word to the end to the next index of the linesOfText vector
					std::string overflowSubstring = linesOfText[ lineIndex ].substr( currentCharLength + 1 );
					linesOfText[ lineIndex ] = linesOfText[ lineIndex ].substr( 0, currentCharLength );		// Exclude the space preceding the word as well
					linesOfText.insert( ( linesOfText.begin() + ( lineIndex + 1 ) ), overflowSubstring );
					break;
				}
				else
				{
					currentLength = newLength;
					currentCharLength = newCharLength;
				}
			}
		}
	}

	float totalTextHeight = cellHeight * static_cast< float > ( linesOfText.size() );
	if ( totalTextHeight > boxBounds.GetDimensions().y )
	{
		cellHeight *= boxBounds.GetDimensions().y / totalTextHeight;
	}

	for ( unsigned int lineIndex = 0; lineIndex < linesOfText.size(); lineIndex++ )
	{
		Vector2 drawMins;
		Vector2 anchorPoint = GetAnchorPointForAlignmentWithTextBox( boxBounds, alignment );

		float lineWidth = font->GetStringWidth( linesOfText[ lineIndex ], cellHeight, aspectScale );

		drawMins.x	=	anchorPoint.x -		// left, center or right
			( alignment.x		// 0 for left, 0.5 for center, 1 for right
				* lineWidth );		// drawMins.x will be equal to the bounds.mins.x if left aligned

									// Calculate drawMins.y based on line height, line number, bounds.y and alignment
		drawMins.y	=	( anchorPoint.y +	// top, center or bottom
			( alignment.y *	// 0 for top, 0.5 for center, 1 for bottom 
			( cellHeight * static_cast< float >( linesOfText.size() ) )
				)
			)					// The Y value, as calculated till here, is the maxs.y for the first (topmost) line of text
			- ( static_cast< float >( lineIndex + 1 ) * cellHeight );

		DrawText2D( drawMins, linesOfText[ lineIndex ], cellHeight, tint, aspectScale, font );
	}
}

#pragma endregion

void Renderer::ChangeRenderColor( const Rgba& color ) const
{
	glColor4ub( color.r, color.g, color.b, color.a );
}

void Renderer::PushMatrix() const
{
	glPushMatrix();
}

void Renderer::PopMatrix() const
{
	glPopMatrix();
}

void Renderer::Translate( float xTranslation, float yTranslation, float zTranslation ) const
{
	glTranslatef( xTranslation, yTranslation, zTranslation );
}

void Renderer::Rotate( float angleDegrees, float xAxis, float yAxis, float zAxis ) const
{
	glRotatef( angleDegrees, xAxis, yAxis, zAxis );
}

void Renderer::Scale( float xScale, float yScale, float zScale ) const
{
	glScalef( xScale, yScale, zScale );
}

void Renderer::SetOrtho( const Vector2& bottomLeft, const Vector2& topRight ) const
{
	glLoadIdentity();
	glOrtho( bottomLeft.x, topRight.x, bottomLeft.y, topRight.y, 0.f, 1.f );
}

void Renderer::SetBlendMode( RendererBlendMode newBlendMode ) const
{
	switch ( newBlendMode )
	{
		case RendererBlendMode::ALPHA:
		{
			glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
			break;
		}
		case RendererBlendMode::ADDITIVE:
		{
			glBlendFunc( GL_SRC_ALPHA, GL_ONE );
			break;
		}
		default:
		{
			break;
		}
	}
}

void Renderer::ClearScreen( const Rgba& clearColor ) const
{
	float rNormalized, gNormalized, bNormalized, aNormalized;
	clearColor.GetAsFloats( rNormalized, gNormalized, bNormalized, aNormalized );

	glClearColor( rNormalized, gNormalized, bNormalized, aNormalized );
	glClear( GL_COLOR_BUFFER_BIT );
}
