#include "Game/Utils/VanDerCorputSequence.hpp"
#include "Engine/Math/MathUtils.hpp"

void MakeVanDerCorputSequence( float* out_sequence, unsigned int power )
{
	int numBits = static_cast< int >( ceil( log2f( static_cast< float >( power ) ) ) );

	for ( unsigned int index = 0U; index < power; index++ )
	{
		float number = static_cast< float >( index ) / static_cast< float >( power );

		char* bits = new char[ numBits ];
		char* reverse = new char[ numBits ];
		FractionToBits( number, numBits, bits );
		ReverseBits( bits, numBits, reverse );
		out_sequence[ index ] = BitsToFraction( reverse, numBits );
		delete[] bits;
		delete[] reverse;
	}
}