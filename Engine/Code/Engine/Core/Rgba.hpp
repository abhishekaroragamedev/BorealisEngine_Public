#pragma once

constexpr int RGBA_MAX = 255;

class Rgba
{

public:
	Rgba::Rgba();
	explicit Rgba::Rgba( unsigned char redByte, unsigned char greenByte, unsigned char blueByte, unsigned char alphaByte );

public:
	void Rgba::SetAsFloats( float normalizedRed, float normalizedGreen, float normalizedBlue, float normalizedAlpha );
	void Rgba::ScaleRGB( float rgbScale );
	void Rgba::ScaleAlpha( float alphaScale );

public:
	void Rgba::SetAsBytes( unsigned char redByte, unsigned char greenByte, unsigned char blueByte, unsigned char alphaByte );
	void Rgba::GetAsFloats( float& out_normalizedRed, float& out_normalizedGreen, float& out_normalizedBlue, float& out_normalizedAlpha ) const;

public:
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;

};
