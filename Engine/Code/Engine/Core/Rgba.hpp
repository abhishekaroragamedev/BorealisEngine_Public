#pragma once

#include "Engine/Math//MathUtils.hpp"

constexpr int RGBA_MAX = 255;

class Rgba
{

public:
	Rgba();
	explicit Rgba( unsigned char redByte, unsigned char greenByte, unsigned char blueByte, unsigned char alphaByte );

public:
	void SetFromText( const char* text );
	void SetAsFloats( float normalizedRed, float normalizedGreen, float normalizedBlue, float normalizedAlpha );
	void ScaleRGB( float rgbScale );
	void ScaleAlpha( float alphaScale );
	void SetAsBytes( unsigned char redByte, unsigned char greenByte, unsigned char blueByte, unsigned char alphaByte );
	void GetAsFloats( float& out_normalizedRed, float& out_normalizedGreen, float& out_normalizedBlue, float& out_normalizedAlpha ) const;
	Rgba GetWithAlpha( float alphaAsFraction ) const;

public:
	static bool AreColorsEqualExceptAlpha( const Rgba& firstColor, const Rgba& secondColor );

public:
	static const Rgba WHITE;
	static const Rgba BLACK;
	static const Rgba RED;
	static const Rgba GREEN;
	static const Rgba BLUE;
	static const Rgba MAGENTA;
	static const Rgba YELLOW;
	static const Rgba CYAN;
	static const Rgba ORANGE;
	static const Rgba PURPLE;

public:
	unsigned char r = 0;
	unsigned char g = 0;
	unsigned char b = 0;
	unsigned char a = 0;

};

const Rgba Interpolate( const Rgba& start, const Rgba& end, float fractionTowardEnd );
