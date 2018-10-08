#include "Engine/Core/Endianness.hpp"
#include <algorithm>

Endianness GetPlatformEndianness()
{
	unsigned int one = 1U;
	if ( reinterpret_cast< char* >(&one)[ 0 ] == 0x01 )
	{
		return Endianness::LITTLE_ENDIAN;
	}
	else
	{
		return Endianness::BIG_ENDIAN;
	}
}

void ToEndianness( const size_t size, void* data, Endianness endianness )
{
	if ( endianness == GetPlatformEndianness() )
	{
		return;
	}

	char* dataBytes = reinterpret_cast< char* >( data );

	long front = 0L;
	long back = static_cast< long >( size ) - 1;
	while ( back > front )
	{
		std::swap( dataBytes[ front ], dataBytes[ back ] );
		front++;
		back--;
	}
}

void FromEndianness( const size_t size, void* data, Endianness endianness ) 
{
	if ( endianness == GetPlatformEndianness() )
	{
		return;
	}

	char* dataBytes = reinterpret_cast< char* >( data );

	size_t front = 0;
	size_t back = size - 1;
	while ( back > front )
	{
		std::swap( dataBytes[ front ], dataBytes[ back ] );
		front++;
		back--;
	}
}
