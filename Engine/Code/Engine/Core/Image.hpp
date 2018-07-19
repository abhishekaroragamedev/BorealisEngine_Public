#pragma once

#include <string>
#include <vector>
#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/IntVector2.hpp"

class Image
{

public:
	Image();
	explicit Image( const std::string& imageFilePath );
	Image( const Image& copy );
	IntVector2 GetDimensions() const;
	std::vector< Rgba > GetTexels() const;
	Rgba GetTexel( int x, int y ) const;
	const Rgba* GetTexelReference( int x, int y ) const;
	void SetTexel( int x, int y, const Rgba& color );

private:
	void SetTexelsFromImageData( unsigned char* imageData, int numComponents );

private:
	IntVector2 m_dimensions = IntVector2::ZERO;
	std::vector< Rgba >	m_texels;

};
