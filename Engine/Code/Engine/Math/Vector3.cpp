#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"

const Vector3 Vector3::ZERO = Vector3( 0.0f, 0.0f, 0.0f );

const Vector3 Vector3::ONE = Vector3( 1.0f, 1.0f, 1.0f );

Vector3::Vector3( float initialX, float initialY, float initialZ )
{
	x = initialX;
	y = initialY;
	z = initialZ;
}

Vector3::Vector3( const Vector3& copyFrom )
{
	x = copyFrom.x;
	y = copyFrom.y;
	z = copyFrom.z;
}

float Vector3::GetLength() const
{
	return sqrtf( GetLengthSquared() );
}

float Vector3::GetLengthSquared() const
{
	return ( ( x * x ) + ( y * y ) + ( z * z ) );
}

float Vector3::NormalizeAndGetLength()
{
	float oldLength = GetLength();
	Vector3 normalizedVector = GetNormalized();
	x = normalizedVector.x;
	y = normalizedVector.y;
	z = normalizedVector.z;

	return oldLength;
}

Vector3 Vector3::GetNormalized() const
{
	float rootOfSumOfSquares = GetLength();

	if ( rootOfSumOfSquares != 0 )
	{
		float normalizedX = x / rootOfSumOfSquares;
		float normalizedY = y / rootOfSumOfSquares;
		float normalizedZ = z / rootOfSumOfSquares;

		return Vector3( normalizedX, normalizedY, normalizedZ );
	}
	else
	{
		return Vector3( 0.0f, 0.0f, 0.0f );
	}
}

float Vector3::GetProjectionInDirection( const Vector3& direction ) const
{
	return DotProduct( *this, direction );
}

const Vector3 Vector3::operator+( const Vector3& vecToAdd ) const
{
	return Vector3( ( x + vecToAdd.x ), ( y + vecToAdd.y ), ( z + vecToAdd.z ) );
}

const Vector3 Vector3::operator-( const Vector3& vecToSubtract ) const
{
	return Vector3( ( x - vecToSubtract.x ), ( y - vecToSubtract.y ), ( z - vecToSubtract.z ) );
}

const Vector3 Vector3::operator*( float uniformScale ) const
{
	return Vector3( ( x * uniformScale ), ( y * uniformScale ), ( z * uniformScale ) );
}

const Vector3 Vector3::operator/( float inverseScale ) const
{

	if ( IsFloatEqualTo( inverseScale, 0.0f ) )
	{
		return Vector3( x, y, z );
	}

	return Vector3( ( x / inverseScale ), ( y / inverseScale ), ( z / inverseScale ) );
}

void Vector3::operator+=( const Vector3& vecToAdd )
{
	x += vecToAdd.x;
	y += vecToAdd.y;
	z += vecToAdd.z;
}

void Vector3::operator-=( const Vector3& vecToSubtract )
{
	x -= vecToSubtract.x;
	y -= vecToSubtract.y;
	z -= vecToSubtract.z;
}

void Vector3::operator*=( const float uniformScale )
{
	x *= uniformScale;
	y *= uniformScale;
	z *= uniformScale;
}

void Vector3::operator/=( const float uniformDivisor )
{
	if ( !IsFloatEqualTo( uniformDivisor, 0.0f ) )
	{
		x /= uniformDivisor;
		y /= uniformDivisor;
		z /= uniformDivisor;
	}
}

void Vector3::operator=( const Vector3& copyFrom )
{
	x = copyFrom.x;
	y = copyFrom.y;
	z = copyFrom.z;
}

bool Vector3::operator==( const Vector3& compare ) const
{
	return ( IsFloatEqualTo( x, compare.x ) && IsFloatEqualTo( y, compare.y ) && IsFloatEqualTo( z, compare.z ) );
}

bool Vector3::operator!=( const Vector3& compare ) const
{
	return ( !IsFloatEqualTo( x, compare.x ) || !IsFloatEqualTo( y, compare.y ) || !IsFloatEqualTo( z, compare.z ) );
}

float DotProduct( const Vector3& a, const Vector3& b )
{
	return ( ( a.x * b.x ) + ( a.y * b.y ) + ( a.z * b.z ) );
}

Vector3 ConvertVector2ToVector3( const Vector2& vector2ToConvert )
{
	return Vector3( vector2ToConvert.x, vector2ToConvert.y, 0.0f );
}
