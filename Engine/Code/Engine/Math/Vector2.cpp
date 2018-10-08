#include "Engine/Math/IntVector2.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Tools/DevConsole.hpp"
#include <string>
#include <vector>

const Vector2 Vector2::ZERO = Vector2( 0.0f, 0.0f );

const Vector2 Vector2::ONE = Vector2( 1.0f, 1.0f );

const Vector2 Vector2::UP = Vector2( 0.0f, 1.0f );

const Vector2 Vector2::DOWN = Vector2( 0.0f, -1.0f );

const Vector2 Vector2::LEFT = Vector2( -1.0f, 0.0f );

const Vector2 Vector2::RIGHT = Vector2( 1.0f, 0.0f );

//-----------------------------------------------------------------------------------------------
Vector2::Vector2( const Vector2& copy )
	: x( 99.f )
	, y( 99.f )
{
	x = copy.x;
	y = copy.y;
}


//-----------------------------------------------------------------------------------------------
Vector2::Vector2( float initialX, float initialY )
	: x( 99.f )
	, y( 99.f )
{
	x = initialX;
	y = initialY;
}


//-----------------------------------------------------------------------------------------------
const Vector2 Vector2::operator + ( const Vector2& vecToAdd ) const
{
	return Vector2(x + vecToAdd.x, y + vecToAdd.y);
}


//-----------------------------------------------------------------------------------------------
const Vector2 Vector2::operator-( const Vector2& vecToSubtract ) const
{
	return Vector2(x - vecToSubtract.x, y - vecToSubtract.y);
}

//-----------------------------------------------------------------------------------------------
const Vector2 Vector2::operator*( const Vector2& nonUniformScale ) const
{
	return Vector2( x * nonUniformScale.x, y * nonUniformScale.y );
}

//-----------------------------------------------------------------------------------------------
const Vector2 Vector2::operator/( const Vector2& nonUniformDivisor ) const
{
	return Vector2( x / nonUniformDivisor.x, y / nonUniformDivisor.y );
}

//-----------------------------------------------------------------------------------------------
const Vector2 Vector2::operator*( float uniformScale ) const
{
	return Vector2(uniformScale * x, uniformScale * y);
}


//-----------------------------------------------------------------------------------------------
const Vector2 Vector2::operator/( float inverseScale ) const
{
	if ( IsFloatEqualTo( inverseScale, 0.0f ) )
	{
		return Vector2( x, y );
	}

	return Vector2(x / inverseScale, y / inverseScale);
}


//-----------------------------------------------------------------------------------------------
void Vector2::operator+=( const Vector2& vecToAdd )
{
	x += vecToAdd.x;
	y += vecToAdd.y;
}


//-----------------------------------------------------------------------------------------------
void Vector2::operator-=( const Vector2& vecToSubtract )
{
	x -= vecToSubtract.x;
	y -= vecToSubtract.y;
}


//-----------------------------------------------------------------------------------------------
void Vector2::operator*=( const float uniformScale )
{
	x *= uniformScale;
	y *= uniformScale;
}


//-----------------------------------------------------------------------------------------------
void Vector2::operator/=( const float uniformDivisor )
{
	if ( !IsFloatEqualTo( uniformDivisor, 0.0f ) )
	{
		x /= uniformDivisor;
		y /= uniformDivisor;
	}
}

//-----------------------------------------------------------------------------------------------
void Vector2::operator*=( const Vector2& nonUniformScale )
{
	x *= nonUniformScale.x;
	y *= nonUniformScale.y;
}

//-----------------------------------------------------------------------------------------------
void Vector2::operator/=( const Vector2& nonUniformDivisor )
{
	x /= nonUniformDivisor.x;
	y /= nonUniformDivisor.y;
}

//-----------------------------------------------------------------------------------------------
void Vector2::operator=( const Vector2& copyFrom )
{
	x = copyFrom.x;
	y = copyFrom.y;
}


//-----------------------------------------------------------------------------------------------
const Vector2 operator*( float uniformScale, const Vector2& vecToScale )
{
	return Vector2( uniformScale * vecToScale.x, uniformScale * vecToScale.y );
}


//-----------------------------------------------------------------------------------------------
bool Vector2::operator==( const Vector2& compare ) const
{
	return ( IsFloatEqualTo( x, compare.x ) && IsFloatEqualTo( y, compare.y ) );
}


//-----------------------------------------------------------------------------------------------
bool Vector2::operator!=( const Vector2& compare ) const
{
	return ( !IsFloatEqualTo( x, compare.x ) || !IsFloatEqualTo( y, compare.y ) );
}

bool Vector2::operator>( const Vector2& compare ) const
{
	return ( ( x > compare.x ) && ( y > compare.y ) );
}

bool Vector2::operator>=( const Vector2& compare ) const
{
	return ( ( *this > compare ) || ( *this == compare ) );
}

bool Vector2::operator<( const Vector2& compare ) const
{
	return ( ( x < compare.x ) && ( y < compare.y ) );
}

bool Vector2::operator<=( const Vector2& compare ) const
{
	return ( ( *this < compare ) || ( *this == compare ) );
}

//-----------------------------------------------------------------------------------------------
float Vector2::GetLength() const
{
	return sqrtf( GetLengthSquared() );
}

//-----------------------------------------------------------------------------------------------
float Vector2::GetLengthSquared() const
{
	return ( x * x ) + ( y * y );
}

//-----------------------------------------------------------------------------------------------
float Vector2::NormalizeAndGetLength()
{
	float oldLength = GetLength();
	Vector2 normalizedVector = GetNormalized();
	x = normalizedVector.x;
	y = normalizedVector.y;

	return oldLength;
}

//-----------------------------------------------------------------------------------------------
Vector2 Vector2::GetNormalized() const
{
	float rootOfSumOfSquares = GetLength();
	
	if ( rootOfSumOfSquares != 0 )
	{
		float normalizedX = x / rootOfSumOfSquares;
		float normalizedY = y / rootOfSumOfSquares;

		return Vector2( normalizedX, normalizedY );
	}
	else
	{
		return Vector2( 0.0f, 0.0f );
	}
}

//-----------------------------------------------------------------------------------------------
float Vector2::GetOrientationDegrees() const
{
	return ATan2Degrees( y, x );		// The angle made by this vector with respect to the origin
}

void Vector2::SetOrientationDegrees( float degrees )
{
	float magnitude = NormalizeAndGetLength();
	x = magnitude * CosDegrees( degrees );
	y = magnitude * SinDegrees( degrees );
}

float Vector2::GetProjectionInDirection( const Vector2& direction ) const
{
	return DotProduct( *this, direction );
}

Vector2 Vector2::GetComponentInDirection( const Vector2& direction ) const
{
	return ( direction.GetNormalized() * GetProjectionInDirection( direction ) );
}

void Vector2::ConvertToPolar()		// Converts to a vector (R, theta), where R = magnitude and theta = orientation in degrees
{
	float tempX = GetLength();
	float tempY = ATan2Degrees( y, x );
	
	x = tempX;
	y = tempY;
}

void Vector2::ConvertToCartestian()
{
	float tempX = x * CosDegrees( y );
	float tempY = x * SinDegrees( y );

	x = tempX;
	y = tempY;
}

void Vector2::RotateDegrees( float deltaDegrees )
{
	ConvertToPolar();
	y += deltaDegrees;
	ConvertToCartestian();
}

void Vector2::TurnTowardVector2( const Vector2& target, float maxTurnDegrees )
{
	ConvertToPolar();

	Vector2 targetPolar = Vector2( target );
	targetPolar.ConvertToPolar();

	y = TurnToward( y, targetPolar.y, maxTurnDegrees );
	ConvertToCartestian();
}

void Vector2::SetFromText( const char* text )
{
	SetFromText( std::string( text ) );
}

void Vector2::SetFromText( const std::string& text )
{
	TokenizedString tokenizedString = TokenizedString( text, "," );
	std::vector< std::string > tokens = tokenizedString.GetTokens();

	std::string xPortion = tokens[ 0 ];
	std::string yPortion = tokens[ 1 ];

	x = static_cast<float>( atof( xPortion.c_str() ) );
	y = static_cast<float>( atof( yPortion.c_str() ) );
}

/* static */
Vector2 Vector2::MakeDirectionAtDegrees( float degrees )
{
	Vector2 newVector = Vector2( CosDegrees( degrees ), SinDegrees( degrees ) );
	return newVector;
}

/* static */
bool Vector2::IsValidString( const std::string& vec2AsString )
{
	if ( vec2AsString.empty() )
	{
		ConsolePrintf( Rgba::RED, "ERROR: Vector2::IsValidString: Empty Vector2 string provided" );
		return false;
	}

	TokenizedString tokenizedString = TokenizedString( vec2AsString, "," );
	std::vector< std::string > tokens = tokenizedString.GetTokens();

	if ( tokens.size() != 2 )
	{
		ConsolePrintf( Rgba::RED, "ERROR: Vector2::IsValidString: Invalid Vector2 string provided." );
		return false;
	}

	try
	{
		float dummyCast = stof( tokens[ 0 ] );
		dummyCast = stof( tokens[ 1 ] );

		UNUSED( dummyCast );
	}
	catch ( std::invalid_argument& invalidArgument )
	{
		UNUSED( invalidArgument );
		ConsolePrintf( Rgba::RED, "ERROR: Vector2::IsValidString: Invalid Vector2 string provided." );
		return false;
	}

	return true;
}

/* STANDALONE FUNCTIONS */

float GetDistance( const Vector2& a, const Vector2& b )
{
	return ( a - b ).GetLength();
}

float GetDistanceSquared( const Vector2& a, const Vector2& b )
{
	return ( a - b ).GetLengthSquared();
}

float DotProduct( const Vector2& a, const Vector2& b )
{
	float dotProductX = a.x * b.x;
	float dotProductY = a.y * b.y;

	return ( dotProductX + dotProductY );
}

Vector2 RotateVector2( const Vector2& vecToRotate, float theta )
{
	float cosTheta = CosDegrees( theta );
	float sinTheta = SinDegrees( theta );

	float newX = vecToRotate.x * cosTheta - vecToRotate.y * sinTheta;
	float newY = vecToRotate.x * sinTheta + vecToRotate.y * cosTheta;

	return Vector2( newX, newY );
}

Vector2 RotateVector2RightAngle( const Vector2& vecToRotate, bool isClockwise )
{
	if ( isClockwise )
	{
		return Vector2( vecToRotate.y, ( -1 * vecToRotate.x ) );
	}
	else
	{
		return Vector2( ( -1 * vecToRotate.y ), vecToRotate.x );
	}
}

Vector2 GetMidPoint( const Vector2& a, const Vector2& b )
{
	return ( ( a + b ) / 2.0f );
}

Vector2 GetProjectedVector( const Vector2& vectorToProject, const Vector2& projectOnto )
{
	Vector2 unitVectorOfProjectionTarget = Vector2( projectOnto );
	float originalLength = unitVectorOfProjectionTarget.NormalizeAndGetLength();
	float lengthOfProjection = DotProduct( vectorToProject, unitVectorOfProjectionTarget );

	return (lengthOfProjection * unitVectorOfProjectionTarget);
}

const Vector2 GetTransformedIntoBasis( const Vector2& originalVector, const Vector2& newBasisI, const Vector2& newBasisJ )
{
	float projectionOnNewI = DotProduct( originalVector, newBasisI );
	float projectionOnNewJ = DotProduct( originalVector, newBasisJ );

	return Vector2( projectionOnNewI, projectionOnNewJ );
}

const Vector2 GetTransformedOutOfBasis( const Vector2& vectorInBasis, const Vector2& oldBasisI, const Vector2& oldBasisJ )
{
	float absoluteXValue = vectorInBasis.x * ( oldBasisI.x ) + vectorInBasis.y * ( oldBasisJ.x );
	float absoluteYValue = vectorInBasis.x * ( oldBasisI.y ) + vectorInBasis.y * ( oldBasisJ.y );

	return Vector2( absoluteXValue, absoluteYValue );
}

void DecomposeVectorIntoBasis( const Vector2& originalVector, const Vector2& newBasisI, const Vector2& newBasisJ, Vector2& out_vectorAlongI, Vector2& out_vectorAlongJ )
{
	out_vectorAlongI = DotProduct( originalVector, newBasisI ) * newBasisI;
	out_vectorAlongJ = DotProduct( originalVector, newBasisJ ) * newBasisJ;
}

Vector2 GetStrongestCardinalDirection( const Vector2& actualDirection )
{
	float strongestDotProduct = 0;
	Vector2 strongestDirection = Vector2::ZERO;

	std::vector< Vector2 > directionsToCheck;
	directionsToCheck.push_back( Vector2::UP );
	directionsToCheck.push_back( Vector2::DOWN );
	directionsToCheck.push_back( Vector2::LEFT );
	directionsToCheck.push_back( Vector2::RIGHT );

	for ( std::vector< Vector2 >::iterator vectorIterator = directionsToCheck.begin(); vectorIterator < directionsToCheck.end(); vectorIterator++ )
	{
		float dotProduct = DotProduct( *vectorIterator, actualDirection );
		if ( dotProduct > strongestDotProduct )
		{
			strongestDotProduct = dotProduct;
			strongestDirection = *vectorIterator;
		}
	}

	return strongestDirection;
}

const Vector2 Interpolate( const Vector2& start, const Vector2& end, float fractionTowardEnd )
{
	return Vector2( Interpolate( start.x, end.x, fractionTowardEnd ), Interpolate( start.y, end.y, fractionTowardEnd ) );
}

Vector2 Reflect( const Vector2& vectorToReflect, const Vector2& reflectionNormal )
{
	Vector2 reflectionTangent = Vector2( -reflectionNormal.y, reflectionNormal.x );		// 2D rotation by 90 degrees in anti-clockwise direction

	Vector2 vectorToReflectTComponent;
	Vector2 vectorToReflectNComponent;

	DecomposeVectorIntoBasis( vectorToReflect, reflectionTangent, reflectionNormal, vectorToReflectTComponent, vectorToReflectNComponent );

	vectorToReflectNComponent *= -1;
	Vector2 reflectedVectorInTNBasis = vectorToReflectTComponent + vectorToReflectNComponent;

	return reflectedVectorInTNBasis;
}

Vector2 ConvertIntVector2ToVector2( const IntVector2& intVecToConvert )
{
	return Vector2( static_cast<float>( intVecToConvert.x ), static_cast<float>( intVecToConvert.y ) );
}

Vector2 ConvertVector3ToVector2( const Vector3& vector3ToConvert )
{
	return Vector2( vector3ToConvert.x, vector3ToConvert.y );
}
