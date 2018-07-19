#include "Engine/Math/FloatRange.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include <string>

FloatRange::FloatRange( float initialMin, float initialMax )
{
	min = initialMin;
	max = initialMax;
}

FloatRange::FloatRange( float initialMinMax )
{
	min = initialMinMax;
	max = initialMinMax;
}

void FloatRange::SetFromText( const char* text )
{
	TokenizedString tokenizedString = TokenizedString( text, "~" );
	std::vector< std::string > tokens = tokenizedString.GetTokens();

	std::string minPortion = tokens[ 0 ];
	std::string maxPortion = tokens[ 0 ];

	if ( tokens.size() == 2 )
	{
		maxPortion = tokens[ 1 ];
	}

	min = static_cast<float>( atof( minPortion.c_str() ) );
	max = static_cast<float>( atof( maxPortion.c_str() ) );
}

float FloatRange::GetRandomInRange() const
{
	return GetRandomFloatInRange( min, max );
}

float FloatRange::Size() const
{
	return Abs( max - min );
}

bool DoRangesOverlap( const FloatRange& a, const FloatRange& b )
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

const FloatRange Interpolate( const FloatRange& start, const FloatRange& end, float fractionTowardEnd )
{
	return FloatRange( Interpolate( start.min, end.min, fractionTowardEnd ), Interpolate( start.max, end.max, fractionTowardEnd ) );
}
