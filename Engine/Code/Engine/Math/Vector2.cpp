#include "Engine/Math/Vector2.hpp"


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
	return (x == compare.x && y == compare.y);
}


//-----------------------------------------------------------------------------------------------
bool Vector2::operator!=( const Vector2& compare ) const
{
	return (x != compare.x || y != compare.y);
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

Vector2 Vector2::MakeDirectionAtDegrees( float degrees )
{
	Vector2 newVector = Vector2( CosDegrees( degrees ), SinDegrees( degrees ) );
	return newVector;
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

Vector2 RotateVector2( const Vector2& vecToRotate, float theta )
{
	float cosTheta = CosDegrees( theta );
	float sinTheta = SinDegrees( theta );

	float newX = vecToRotate.x * cosTheta - vecToRotate.y * sinTheta;
	float newY = vecToRotate.x * sinTheta + vecToRotate.y * cosTheta;

	return Vector2( newX, newY );
}

Vector2 GetMidPoint( const Vector2& a, const Vector2& b )
{
	return ( ( a + b ) / 2.0f );
}
