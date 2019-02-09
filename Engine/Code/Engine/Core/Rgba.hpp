#pragma once

#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Vector4.hpp"
#include <string>

constexpr int RGBA_MAX = 255;
constexpr float RGBA_MAX_F = 255.0f;

class Rgba
{

public:
	Rgba();
	explicit Rgba( unsigned char redByte, unsigned char greenByte, unsigned char blueByte, unsigned char alphaByte );
	explicit Rgba( const Vector4& colorAsVec4 );
	Rgba( const Rgba& copy );

public:
	void SetFromText( const char* text );
	void SetFromText( const std::string& text );
	void SetAsFloats( float normalizedRed, float normalizedGreen, float normalizedBlue, float normalizedAlpha );
	void ScaleRGB( float rgbScale );
	void ScaleAlpha( float alphaScale );
	void SetAsBytes( unsigned char redByte, unsigned char greenByte, unsigned char blueByte, unsigned char alphaByte );
	void GetAsFloats( float& out_normalizedRed, float& out_normalizedGreen, float& out_normalizedBlue, float& out_normalizedAlpha ) const;
	Vector3 GetRGBAsFloats() const;
	Vector4 GetAsFloats() const;
	Rgba GetWithAlpha( float alphaAsFraction ) const;
	Rgba GetInverse() const;	// Does not invert Alpha
	void operator=( const Rgba& assign );
	bool operator==( const Rgba& colorToCompare ) const;
	bool operator!=( const Rgba& colorToCompare ) const;

public:
	static bool AreColorsEqualExceptAlpha( const Rgba& firstColor, const Rgba& secondColor );
	static bool IsValidString( const std::string& rgbaAsString );
	static Vector3 GetRGBNormalized( const Rgba& color );
	static Vector4 GetRGBANormalized( const Rgba& color );

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
	static const Rgba NO_COLOR;

public:
	float r = 0.0f;
	float g = 0.0f;
	float b = 0.0f;
	float a = 0.0f;

};

const Rgba Interpolate( const Rgba& start, const Rgba& end, float fractionTowardEnd );
