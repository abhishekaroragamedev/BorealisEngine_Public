#include "Engine/Math/IntVector3.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include <string>

const IntVector3 IntVector3::ZERO = IntVector3( 0, 0, 0 );
const IntVector3 IntVector3::ONE = IntVector3( 1, 1, 1 );

IntVector3::IntVector3()
{
	x = 0;
	y = 0;
}

IntVector3::IntVector3( int xCoordinate, int yCoordinate, int zCoordinate )
{
	x = xCoordinate;
	y = yCoordinate;
	z = zCoordinate;
}

IntVector3::IntVector3( const IntVector2& intVector2, int zCoordinate )
{
	x = intVector2.x;
	y = intVector2.y;
	z = zCoordinate;
}

IntVector3::~IntVector3()
{

}

void IntVector3::SetFromText( const char* text )
{
	TokenizedString tokenizedString = TokenizedString( text, "," );
	std::vector< std::string > tokens = tokenizedString.GetTokens();

	std::string xPortion = tokens[ 0 ];
	std::string yPortion = tokens[ 1 ];
	std::string zPortion = tokens[ 2 ];

	x = atoi( xPortion.c_str() );
	y = atoi( yPortion.c_str() );
	z = atoi( zPortion.c_str() );
}

const IntVector3 IntVector3::operator+( const IntVector3& vectorToAdd ) const
{
	return IntVector3( ( x + vectorToAdd.x ), ( y + vectorToAdd.y ), ( z + vectorToAdd.z ) );
}

const IntVector3 IntVector3::operator-( const IntVector3& vectorToSubtract ) const
{
	return IntVector3( ( x - vectorToSubtract.x ), ( y - vectorToSubtract.y ), ( z - vectorToSubtract.z ) );
}

const bool IntVector3::operator==( const IntVector3& vectorToCompare ) const
{
	return ( x == vectorToCompare.x && y == vectorToCompare.y && z == vectorToCompare.z );
}

const bool IntVector3::operator>( const IntVector3& vectorToCompare ) const
{
	return ( x > vectorToCompare.x && y > vectorToCompare.y && z > vectorToCompare.z );
}

const bool IntVector3::operator>=( const IntVector3& vectorToCompare ) const
{
	return ( x >= vectorToCompare.x && y >= vectorToCompare.y && z >= vectorToCompare.z );
}

const bool IntVector3::operator<( const IntVector3& vectorToCompare ) const
{
	return ( x < vectorToCompare.x && y < vectorToCompare.y && z < vectorToCompare.z );
}

const bool IntVector3::operator<=( const IntVector3& vectorToCompare ) const
{
	return ( x <= vectorToCompare.x && y <= vectorToCompare.y && z <= vectorToCompare.z );
}

const IntVector3 Interpolate( const IntVector3& start, const IntVector3& end, float fractionTowardEnd )
{
	return IntVector3( Interpolate( start.x, end.x, fractionTowardEnd ), Interpolate( start.y, end.y, fractionTowardEnd ), Interpolate( start.z, end.z, fractionTowardEnd ) );
}

IntVector3 ConvertVector3ToIntVector3( const Vector3& vecToConvert )
{
	return IntVector3( static_cast<int>( vecToConvert.x ), static_cast<int>( vecToConvert.y ), static_cast<int>( vecToConvert.z ) );
}
