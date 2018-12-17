#pragma once

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/Vector2.hpp"

#pragma comment( lib, "opengl32" )	// Link in the OpenGL32.lib static library

constexpr int CIRCLE_NUM_SIDES = 36;

enum RPolygonType
{
	INVALID = -1,
	REGULAR = 0,
	BROKEN_LINES = 1
};

class Renderer
{

public:
	Renderer();
	~Renderer();

public:
	void InitializeRenderer() const;
	void DrawLine( const Vector2& startPoint, const Vector2& endPoint, const Rgba& startColor, const Rgba& endColor, const float lineThickness ) const;
	void DrawPolygon( const Vector2& location, const float angle, const Vector2 vertices[], const int numSides, const RPolygonType polygonType ) const;
	void ChangeRenderColor( const Rgba& ) const;
	void BeginFrame( float screenWidth, float screenHeight ) const;
	void EndFrame() const;
	void PushMatrix() const;
	void PopMatrix() const;
	void Translate( float xTranslation, float yTranslation, float zTranslation ) const;
	void Rotate( float angleDegrees, float xAxis, float yAxis, float zAxis ) const;
	void Scale( float xScale, float yScale, float zScale ) const;
	void SetOrtho( const Vector2& bottomLeft, const Vector2& topRight ) const;

private:
	void ClearScreen( const Rgba& clearColor ) const;

private:
	const float CIRCLE_THETA_INCREMENT_DEGREES = static_cast<float>( 360 / CIRCLE_NUM_SIDES );
};
