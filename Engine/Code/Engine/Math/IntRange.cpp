#include "Engine/Math/IntRange.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include <string>

IntRange::IntRange()
{

}

IntRange::IntRange( int initialMin, int initialMax )
{
	min = initialMin;
	max = initialMax;
}

IntRange::IntRange( int initialMinMax )
{
	min = initialMinMax;
	max = initialMinMax;
}

void IntRange::SetFromText( const char* text )
{
	TokenizedString tokenizedString = TokenizedString( text, "~" );
	std::vector< std::string > tokens = tokenizedString.GetTokens();

	std::string minPortion = tokens[ 0 ];
	std::string maxPortion = tokens[ 0 ];

	if ( tokens.size() == 2 )
	{
		maxPortion = tokens[ 1 ];
	}

	min = atoi( minPortion.c_str() );
	max = atoi( maxPortion.c_str() );
}

std::vector< int > IntRange::GetAllIntsInRange() const
{
	std::vector< int > intsInRange;

	for ( int intIterator = min; intIterator <= max; ( ( max >= min )? intIterator++ : intIterator-- ) )
	{
		intsInRange.push_back( intIterator );
	}

	return intsInRange;
}

int IntRange::GetRandomInRange() const
{
	return GetRandomIntInRange( min, max );
}

bool DoRangesOverlap( const IntRange& a, const IntRange& b )
{
	if ( a.min <= b.max )
	{
		if ( a.min >= b.min )
		{
			return true;
		}
		else if ( a.max >= b.min )
		{
			return true;
		}
	}
	return false;
}

const IntRange Interpolate( const IntRange& start, const IntRange& end, float fractionTowardEnd )
{
	return IntRange( Interpolate( start.min, end.min, fractionTowardEnd ), Interpolate( start.max, end.max, fractionTowardEnd ) );
}

void SetFromText( std::vector< int >& out_vectorOfInts, const char* text )
{
	TokenizedString tokenizedIntString = TokenizedString( text, "," );
	std::vector< std::string > intTokens = tokenizedIntString.GetTokens();

	for ( std::vector< std::string >::iterator tokenIterator = intTokens.begin(); tokenIterator != intTokens.end(); tokenIterator++ )
	{
		int tokenAsInt = std::stoi( *tokenIterator );
		out_vectorOfInts.push_back( tokenAsInt );
	}
}
