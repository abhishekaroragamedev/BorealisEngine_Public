#pragma once

enum Endianness 
{
	LITTLE_ENDIAN = 0U,
	BIG_ENDIAN = 1U,
};

Endianness GetPlatformEndianness(); 

void ToEndianness( const size_t size, void* data, Endianness endianness );
void FromEndianness( const size_t size, void* data, Endianness endianness ); 
