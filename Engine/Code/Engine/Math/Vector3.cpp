#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/IntVector3.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Vector4.hpp"
#include "Engine/Tools/DevConsole.hpp"

const Vector3 Vector3::ZERO = Vector3( 0.0f, 0.0f, 0.0f );

const Vector3 Vector3::ONE = Vector3( 1.0f, 1.0f, 1.0f );

const Vector3 Vector3::FORWARD = Vector3( 0.0f, 0.0f, 1.0f );

const Vector3 Vector3::UP = Vector3( 0.0f, 1.0f, 0.0f );

const Vector3 Vector3::RIGHT = Vector3( 1.0f, 0.0f, 0.0f );

Vector3::Vector3( float initialX, float initialY, float initialZ )
{
	x = initialX;
	y = initialY;
	z = initialZ;
}

Vector3::Vector3( const Vector4& vector4 )
{
	x = vector4.x;
	y = vector4.y;
	z = vector4.z;
}

Vector3::Vector3( const Vector3& copyFrom )
{
	x = copyFrom.x;
	y = copyFrom.y;
	z = copyFrom.z;
}

bool Vector3::IsEqual( const Vector3& compare ) const
{
	return ( *this == compare );
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

Vector3 Vector3::GetComponentInDirection( const Vector3& direction ) const
{
	return ( direction.GetNormalized() * GetProjectionInDirection( direction ) );
}

void Vector3::AddYaw( float degrees )
{
	Vector2 xzCoordinates = Vector2( x, z );
	xzCoordinates.ConvertToPolar();
	xzCoordinates.y += degrees;
	xzCoordinates.ConvertToCartestian();
	x = xzCoordinates.x;
	z = xzCoordinates.y;
}

void Vector3::AddPitch( float degrees )
{
	Vector2 yzCoordinates = Vector2( y, z );
	yzCoordinates.ConvertToPolar();
	yzCoordinates.y += degrees;
	yzCoordinates.ConvertToCartestian();
	y = yzCoordinates.x;
	z = yzCoordinates.y;
}

void Vector3::AddRoll( float degrees )
{
	Vector2 xyCoordinates = Vector2( x, y );
	xyCoordinates.ConvertToPolar();
	xyCoordinates.y += degrees;
	xyCoordinates.ConvertToCartestian();
	x = xyCoordinates.x;
	y = xyCoordinates.y;
}

void Vector3::RotateAboutAxes( const Vector3& eulerAngles )
{
	AddYaw( eulerAngles.y );
	AddPitch( eulerAngles.x );
	AddRoll( eulerAngles.z );
}

void Vector3::ConvertToCartesian()
{
	float radius = x;
	float rotation = y;
	float azimuth = z;

	// Lay out the radius along the x-axis
	Vector3 xzProjectionNormalized = Vector3::RIGHT;

	// Rotate the x-axis by "rotation" degrees to get the XZ (normalized) projection of the vector
	xzProjectionNormalized.AddYaw( rotation );

	Vector3 vectorNormalized =	Vector3(
											xzProjectionNormalized.x,
											TanDegrees( azimuth ),		// Since the length of xzProjectionNormalized is 1.0f
											xzProjectionNormalized.z
										).GetNormalized();

	Vector3 cartesianVector = vectorNormalized * radius;
	x = cartesianVector.x;
	y = cartesianVector.y;
	z = cartesianVector.z;
}

void Vector3::ConvertToPolar()
{
	float radius = GetLength();

	Vector3 normalizedOntoX = Vector3::RIGHT;
	Vector3 normalizedInXZ = Vector3( x, 0.0f, z ).GetNormalized();
	float rotationDegrees = ATan2Degrees( z, x );

	Vector3 normalizedVector = GetNormalized();
	float azimuthDegrees = ATan2Degrees( y, sqrtf( ( x * x ) + ( z * z ) ) );

	x = radius;
	y = rotationDegrees;
	z = azimuthDegrees;
}

void Vector3::SetFromText( const char* text )
{
	SetFromText( std::string( text ) );
}

void Vector3::SetFromText( const std::string& text )
{
	TokenizedString tokenizedString = TokenizedString( text, "," );
	std::vector< std::string > tokens = tokenizedString.GetTokens();

	x = std::stof( tokens[ 0 ].c_str() );
	y = std::stof( tokens[ 1 ].c_str() );
	z = std::stof( tokens[ 2 ].c_str() );
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

const Vector3 Vector3::operator*( const Vector3& nonUniformScale ) const
{
	return Vector3( ( x * nonUniformScale.x ), ( y * nonUniformScale.y ), ( z * nonUniformScale.z ) );
}

const Vector3 Vector3::operator/( const Vector3& nonUniformDivisor ) const
{
	if ( IsFloatEqualTo( nonUniformDivisor.x, 0.0f ) || IsFloatEqualTo( nonUniformDivisor.y, 0.0f ) || IsFloatEqualTo( nonUniformDivisor.z, 0.0f ) )
	{
		return Vector3( x, y, z );
	}

	return Vector3( ( x / nonUniformDivisor.x ), ( y / nonUniformDivisor.y ), ( z / nonUniformDivisor.z ) );
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

void Vector3::operator*=( const Vector3& nonUniformScale )
{
	x *= nonUniformScale.x;
	y *= nonUniformScale.y;
	z *= nonUniformScale.z;
}

void Vector3::operator/=( const Vector3& nonUniformDivisor )
{
	if ( !IsFloatEqualTo( nonUniformDivisor.x, 0.0f ) && !IsFloatEqualTo( nonUniformDivisor.y, 0.0f ) && !IsFloatEqualTo( nonUniformDivisor.z, 0.0f ) )
	{
		x /= nonUniformDivisor.x;
		y /= nonUniformDivisor.y;
		z /= nonUniformDivisor.z;
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

bool Vector3::operator>( const Vector3& compare ) const
{
	return ( ( x > compare.x ) && ( y > compare.y ) && ( z > compare.z ) );
}

bool Vector3::operator>=( const Vector3& compare ) const
{
	return ( ( *this > compare ) || ( *this == compare ) );
}

bool Vector3::operator<( const Vector3& compare ) const
{
	return ( ( x < compare.x ) && ( y < compare.y ) && ( z < compare.z ) );
}

bool Vector3::operator<=( const Vector3& compare ) const
{
	return ( ( *this < compare ) || ( *this == compare ) );
}

/* static */
Vector3  Vector3::FromEuler( const Vector3& euler )
{
	Vector3 vectorFromEuler = Vector3::RIGHT;
	vectorFromEuler.RotateAboutAxes( euler );
	return vectorFromEuler;
}

/* static */
Vector3 Vector3::ConvertToCartesian( const Vector3& polarVector )
{
	float radius = polarVector.x;
	float rotation = polarVector.y;
	float azimuth = polarVector.z;

	// Lay out the radius along the x-axis
	Vector3 xzProjectionNormalized = Vector3::RIGHT;

	// Rotate the x-axis by "rotation" degrees to get the XZ (normalized) projection of the vector
	xzProjectionNormalized.AddYaw( rotation );

	Vector3 vectorNormalized =	Vector3(
		xzProjectionNormalized.x,
		TanDegrees( azimuth ),		// Since the length of xzProjectionNormalized is 1.0f
		xzProjectionNormalized.z
	).GetNormalized();

	return ( vectorNormalized * radius );
}

/* static */
Vector3 Vector3::ConvertToPolar( const Vector3& cartesianVector )
{
	float radius = cartesianVector.GetLength();

	Vector3 normalizedOntoX = Vector3::RIGHT;
	Vector3 normalizedInXZ = Vector3( cartesianVector.x, 0.0f, cartesianVector.z ).GetNormalized();
	float rotationDegrees = ATan2Degrees( cartesianVector.z, cartesianVector.x );

	Vector3 normalizedVector = cartesianVector.GetNormalized();
	float azimuthDegrees = ATan2Degrees( cartesianVector.y, sqrtf( ( cartesianVector.x * cartesianVector.x ) + ( cartesianVector.z * cartesianVector.z ) ) );

	return Vector3( radius, rotationDegrees, azimuthDegrees );
}

/* static */
Vector3 Vector3::ConvertToCartesian( float radius, float rotation, float azimuth )
{
	// Lay out the radius along the x-axis
	Vector3 xzProjectionNormalized = Vector3::RIGHT;

	// Rotate the x-axis by "rotation" degrees to get the XZ (normalized) projection of the vector
	xzProjectionNormalized.AddYaw( rotation );

	Vector3 vectorNormalized =	Vector3(
		xzProjectionNormalized.x,
		TanDegrees( azimuth ),		// Since the length of xzProjectionNormalized is 1.0f
		xzProjectionNormalized.z
	).GetNormalized();

	return ( vectorNormalized * radius );
}

/* static */
Vector3 Vector3::ConvertToPolar( float x, float y, float z )
{
	Vector3 cartesianVector = Vector3( x, y, z );

	float radius = cartesianVector.GetLength();

	Vector3 normalizedOntoX = Vector3::RIGHT;
	Vector3 normalizedInXZ = Vector3( cartesianVector.x, 0.0f, cartesianVector.z ).GetNormalized();
	float rotationDegrees = ATan2Degrees( cartesianVector.z, cartesianVector.x );

	Vector3 normalizedVector = cartesianVector.GetNormalized();
	float azimuthDegrees = ATan2Degrees( cartesianVector.y, sqrtf( ( cartesianVector.x * cartesianVector.x ) + ( cartesianVector.z * cartesianVector.z ) ) );

	return Vector3( radius, rotationDegrees, azimuthDegrees );
}

/* static */
Vector3 Vector3::GetRandomPointOnSphere( float radius /* = 1.0f */ )
{
	Vector3 random = Vector3( GetRandomFloatInRange( 0.0f, 1.0f ), GetRandomFloatInRange( 0.0f, 1.0f ), GetRandomFloatInRange( 0.0f, 1.0f ) ).GetNormalized();
	random *= radius;
	return random;
}

/* static */
bool Vector3::IsValidString( const std::string& vec3AsString )
{
	if ( vec3AsString.empty() )
	{
		ConsolePrintf( Rgba::RED, "ERROR: Vector3::IsValidString: Empty Vector3 string provided" );
		return false;
	}

	TokenizedString tokenizedString = TokenizedString( vec3AsString, "," );
	std::vector< std::string > tokens = tokenizedString.GetTokens();

	if ( tokens.size() != 3 )
	{
		ConsolePrintf( Rgba::RED, "ERROR: Vector3::IsValidString: Invalid Vector3 string provided." );
		return false;
	}

	try
	{
		float dummyCast = stof( tokens[ 0 ] );
		dummyCast = stof( tokens[ 1 ] );
		dummyCast = stof( tokens[ 2 ] );

		UNUSED( dummyCast );
	}
	catch ( std::invalid_argument& invalidArgument )
	{
		UNUSED( invalidArgument );
		ConsolePrintf( Rgba::RED, "ERROR: Vector3::IsValidString: Invalid Vector3 string provided." );
		return false;
	}

	return true;
}

const Vector3 operator*( float uniformScale, const Vector3& vecToScale )
{
	return Vector3( uniformScale * vecToScale.x, uniformScale * vecToScale.y, uniformScale * vecToScale.z );
}

float DotProduct( const Vector3& a, const Vector3& b )
{
	return ( ( a.x * b.x ) + ( a.y * b.y ) + ( a.z * b.z ) );
}

Vector3 CrossProduct( const Vector3& a, const Vector3& b )
{
	Vector3 crossProduct = Vector3( 
		( ( a.y * b.z ) - ( a.z * b.y ) ),
		( ( a.z * b.x ) - ( a.x * b.z ) ),
		( ( a.x * b.y ) - ( a.y * b.x ) )
	);

	return crossProduct;
}

Vector3 ConvertIntVector3ToVector3( const IntVector3& intVecToConvert )
{
	return Vector3( static_cast< float >( intVecToConvert.x ), static_cast< float >( intVecToConvert.y ), static_cast< float >( intVecToConvert.z ) );
}

Vector3 ConvertVector2ToVector3( const Vector2& vector2ToConvert )
{
	return Vector3( vector2ToConvert.x, vector2ToConvert.y, 0.0f );
}

Vector3 Interpolate( const Vector3& start, const Vector3& end, float fractionTowardEnd )
{
	return Vector3( Interpolate( start.x, end.x, fractionTowardEnd ), Interpolate( start.y, end.y, fractionTowardEnd ), Interpolate( start.z, end.z, fractionTowardEnd ) );
}

Vector3 SLerp( const Vector3& start, const Vector3& end, float fractionTowardEnd )
{
	Vector3 startCopy = Vector3( start );
	Vector3 endCopy = Vector3( end );

	float startLength = startCopy.NormalizeAndGetLength();
	float endLength = endCopy.NormalizeAndGetLength();
	float lerpedLength = Interpolate( startLength, endLength, fractionTowardEnd );

	Vector3 sLerpedUnitVector = SLerpUnitVector( startCopy, endCopy, fractionTowardEnd );

	return ( lerpedLength * sLerpedUnitVector );
}

Vector3 SLerpUnitVector( const Vector3& start, const Vector3& end, float fractionTowardEnd )
{
	float cosAngle = ClampFloat( DotProduct( start, end ), -1.0f, 1.0f );
	float angleDegrees = ACosDegrees( cosAngle );

	if ( angleDegrees < FLT_EPSILON )
	{
		return Interpolate( start, end, fractionTowardEnd );
	}

	float blendForward = SinDegrees( fractionTowardEnd * angleDegrees ) / SinDegrees( angleDegrees );
	float blendBehind = SinDegrees( ( 1.0f - fractionTowardEnd ) * angleDegrees ) / SinDegrees( angleDegrees );
	Vector3 sLerpedUnitVector = ( ( blendBehind * start ) + ( blendForward * end ) );
	return sLerpedUnitVector;
}
