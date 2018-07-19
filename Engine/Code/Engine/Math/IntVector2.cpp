#include "Engine/Math/IntVector2.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include <string>

const IntVector2 IntVector2::ZERO = IntVector2( 0, 0 );
const IntVector2 IntVector2::ONE = IntVector2( 1, 1 );
const IntVector2 IntVector2::STEP_NORTH = IntVector2( 0, 1 );
const IntVector2 IntVector2::STEP_SOUTH = IntVector2( 0, -1 );
const IntVector2 IntVector2::STEP_WEST = IntVector2( -1, 0 );
const IntVector2 IntVector2::STEP_EAST = IntVector2( 1, 0 );
const IntVector2 IntVector2::STEP_NORTHWEST = IntVector2( -1, 1 );
const IntVector2 IntVector2::STEP_NORTHEAST	= IntVector2( 1, 1 );
const IntVector2 IntVector2::STEP_SOUTHWEST	= IntVector2( -1, -1 );
const IntVector2 IntVector2::STEP_SOUTHEAST	= IntVector2( 1, -1 );

IntVector2::IntVector2()
{
	x = 0;
	y = 0;
}

IntVector2::IntVector2( int xCoordinate, int yCoordinate )
{
	x = xCoordinate;
	y = yCoordinate;
}

IntVector2::~IntVector2()
{

}

void IntVector2::SetFromText( const char* text )
{
	TokenizedString tokenizedString = TokenizedString( text, "," );
	std::vector< std::string > tokens = tokenizedString.GetTokens();

	std::string xPortion = tokens[ 0 ];
	std::string yPortion = tokens[ 1 ];

	x = atoi( xPortion.c_str() );
	y = atoi( yPortion.c_str() );
}

const IntVector2 IntVector2::operator+( const IntVector2& vectorToAdd ) const
{
	return IntVector2( ( x + vectorToAdd.x ), ( y + vectorToAdd.y ) );
}

const IntVector2 IntVector2::operator-( const IntVector2& vectorToSubtract ) const
{
	return IntVector2( ( x - vectorToSubtract.x ), ( y - vectorToSubtract.y ) );
}

const bool IntVector2::operator==( const IntVector2& vectorToCompare ) const
{
	return ( x == vectorToCompare.x && y == vectorToCompare.y );
}

const IntVector2 Interpolate( const IntVector2& start, const IntVector2& end, float fractionTowardEnd )
{
	return IntVector2( Interpolate( start.x, end.x, fractionTowardEnd ), Interpolate( start.y, end.y, fractionTowardEnd ) );
}

IntVector2 ConvertVector2ToIntVector2( const Vector2& vecToConvert )
{
	return IntVector2( static_cast<int>( vecToConvert.x ), static_cast<int>( vecToConvert.y ) );
}
