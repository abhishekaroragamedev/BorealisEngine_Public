#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "Engine/Renderer/Renderer.hpp"
#include <gl/gl.h>

Renderer::Renderer()
{

}

Renderer::~Renderer()
{
	
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
	SetOrtho( Vector2( 0.0f, 0.0f ), Vector2( screenWidth, screenHeight ) );
	ClearScreen( Rgba( 0, 0, 0, 255 ) );		// Clear all screen (backbuffer) pixels to black
}

void Renderer::EndFrame() const
{
	HWND activeWindow = GetActiveWindow();
	HDC activeDeviceDisplayContext = GetDC( activeWindow );
	SwapBuffers( activeDeviceDisplayContext );
}

void Renderer::DrawLine( const Vector2& startPoint, const Vector2& endPoint, const Rgba& startColor, const Rgba& endColor, const float lineThickness ) const
{
	glLineWidth( lineThickness );

	glBegin( GL_LINES );

	ChangeRenderColor( startColor );
	glVertex2f( startPoint.x, startPoint.y );

	ChangeRenderColor( endColor );
	glVertex2f( endPoint.x, endPoint.y );

	glEnd();
}

void Renderer::DrawPolygon( const Vector2& location, const float angle, const Vector2 vertices[], const int numSides, RPolygonType polygonType ) const
{
	if (numSides == 0)
	{
		return;
	}

	float theta = 0.0f;
	float deltaTheta = 360.0f / static_cast<float>( numSides );

	PushMatrix();		// Perform the rendering in the polygon's local space
	Translate( location.x, location.y, 0.0f );
	Rotate( angle, 0.0f, 0.0f, 1.0f );

	glBegin(GL_LINES);

	for ( int i = 0; i < numSides; i++ )
	{
		if ( ( polygonType != RPolygonType::BROKEN_LINES ) || ( i % 2 == 0 ) )
		{
			glVertex2f( vertices[ ( i % numSides ) ].x, vertices[ ( i % numSides ) ].y );
			glVertex2f( vertices[ ( ( i + 1 ) % numSides ) ].x, vertices[ ( ( i + 1 ) % numSides ) ].y );
		}
	}

	glEnd();

	PopMatrix();
}

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

void Renderer::ClearScreen( const Rgba& clearColor ) const
{
	float rNormalized, gNormalized, bNormalized, aNormalized;
	clearColor.GetAsFloats( rNormalized, gNormalized, bNormalized, aNormalized );

	glClearColor( rNormalized, gNormalized, bNormalized, aNormalized );
	glClear( GL_COLOR_BUFFER_BIT );
}
