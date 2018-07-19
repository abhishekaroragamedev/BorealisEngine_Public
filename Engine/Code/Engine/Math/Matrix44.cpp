#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Matrix44.hpp"

Matrix44::Matrix44()
{
	
}

Matrix44::Matrix44( const float* sixteenValuesBasisMajor )
{
	iX = sixteenValuesBasisMajor[ 0 ]; jX = sixteenValuesBasisMajor[ 4 ]; kX = sixteenValuesBasisMajor[ 8 ]; tX = sixteenValuesBasisMajor[ 12 ];
	iY = sixteenValuesBasisMajor[ 1 ]; jY = sixteenValuesBasisMajor[ 5 ]; kY = sixteenValuesBasisMajor[ 9 ]; tY = sixteenValuesBasisMajor[ 13 ];
	iZ = sixteenValuesBasisMajor[ 2 ]; jZ = sixteenValuesBasisMajor[ 6 ]; kZ = sixteenValuesBasisMajor[ 10 ]; tZ = sixteenValuesBasisMajor[ 14 ];
	iW = sixteenValuesBasisMajor[ 3 ]; jW = sixteenValuesBasisMajor[ 7 ]; kW = sixteenValuesBasisMajor[ 11 ]; tW = sixteenValuesBasisMajor[ 15 ];
}

Matrix44::Matrix44( const Vector2& iBasis, const Vector2& jBasis, const Vector2& translation = Vector2::ZERO )
{
	iX = iBasis.x;
	iY = iBasis.y;
	jX = jBasis.x;
	jY = jBasis.y;
	tX = translation.x;
	tY = translation.y;
}

Vector2 Matrix44::TransformPosition2D( const Vector2& position2D )
{
	return Vector2
	(
		( iX * position2D.x + jX * position2D.y + tX ),
		( iY * position2D.x + jY * position2D.y + tY )
	);
}

Vector2 Matrix44::TransformDisplacement2D( const Vector2& displacement2D )
{
	return Vector2
	(
		( iX * displacement2D.x + jX * displacement2D.y ),
		( iY * displacement2D.x + jY * displacement2D.y )
	);
}

void Matrix44::SetIdentity()
{
	iX = 1.0f; jX = 0.0f; kX = 0.0f; tX = 0.0f;
	iY = 0.0f; jY = 1.0f; kY = 0.0f; tY = 0.0f;
	iZ = 0.0f; jZ = 0.0f; kZ = 1.0f; tZ = 0.0f;
	iW = 0.0f; jW = 0.0f; kW = 0.0f; tW = 1.0f;
}

void Matrix44::SetValues( const float* sixteenValuesBasisMajor )
{
	iX = sixteenValuesBasisMajor[ 0 ]; jX = sixteenValuesBasisMajor[ 4 ]; kX = sixteenValuesBasisMajor[ 8 ]; tX = sixteenValuesBasisMajor[ 12 ];
	iY = sixteenValuesBasisMajor[ 1 ]; jY = sixteenValuesBasisMajor[ 5 ]; kY = sixteenValuesBasisMajor[ 9 ]; tY = sixteenValuesBasisMajor[ 13 ];
	iZ = sixteenValuesBasisMajor[ 2 ]; jZ = sixteenValuesBasisMajor[ 6 ]; kZ = sixteenValuesBasisMajor[ 10 ]; tZ = sixteenValuesBasisMajor[ 14 ];
	iW = sixteenValuesBasisMajor[ 3 ]; jW = sixteenValuesBasisMajor[ 7 ]; kW = sixteenValuesBasisMajor[ 11 ]; tW = sixteenValuesBasisMajor[ 15 ];
}

void Matrix44::Append( const Matrix44& matrixToAppend )
{
	Matrix44 oldMatrix = Matrix44( *this );

	iX = ( oldMatrix.iX * matrixToAppend.iX ) + ( oldMatrix.jX * matrixToAppend.iY ) + ( oldMatrix.kX * matrixToAppend.iZ ) + ( oldMatrix.tX * matrixToAppend.iW );
	iY = ( oldMatrix.iY * matrixToAppend.iX ) + ( oldMatrix.jY * matrixToAppend.iY ) + ( oldMatrix.kY * matrixToAppend.iZ ) + ( oldMatrix.tY * matrixToAppend.iW );
	iZ = ( oldMatrix.iZ * matrixToAppend.iX ) + ( oldMatrix.jZ * matrixToAppend.iY ) + ( oldMatrix.kZ * matrixToAppend.iZ ) + ( oldMatrix.tZ * matrixToAppend.iW );
	iW = ( oldMatrix.iW * matrixToAppend.iX ) + ( oldMatrix.jW * matrixToAppend.iY ) + ( oldMatrix.kW * matrixToAppend.iZ ) + ( oldMatrix.tW * matrixToAppend.iW );

	jX = ( oldMatrix.iX * matrixToAppend.jX ) + ( oldMatrix.jX * matrixToAppend.jY ) + ( oldMatrix.kX * matrixToAppend.jZ ) + ( oldMatrix.tX * matrixToAppend.jW );
	jY = ( oldMatrix.iY * matrixToAppend.jX ) + ( oldMatrix.jY * matrixToAppend.jY ) + ( oldMatrix.kY * matrixToAppend.jZ ) + ( oldMatrix.tY * matrixToAppend.jW );
	jZ = ( oldMatrix.iZ * matrixToAppend.jX ) + ( oldMatrix.jZ * matrixToAppend.jY ) + ( oldMatrix.kZ * matrixToAppend.jZ ) + ( oldMatrix.tZ * matrixToAppend.jW );
	jW = ( oldMatrix.iW * matrixToAppend.jX ) + ( oldMatrix.jW * matrixToAppend.jY ) + ( oldMatrix.kW * matrixToAppend.jZ ) + ( oldMatrix.tW * matrixToAppend.jW );

	kX = ( oldMatrix.iX * matrixToAppend.kX ) + ( oldMatrix.jX * matrixToAppend.kY ) + ( oldMatrix.kX * matrixToAppend.kZ ) + ( oldMatrix.tX * matrixToAppend.kW );
	kY = ( oldMatrix.iY * matrixToAppend.kX ) + ( oldMatrix.jY * matrixToAppend.kY ) + ( oldMatrix.kY * matrixToAppend.kZ ) + ( oldMatrix.tY * matrixToAppend.kW );
	kZ = ( oldMatrix.iZ * matrixToAppend.kX ) + ( oldMatrix.jZ * matrixToAppend.kY ) + ( oldMatrix.kZ * matrixToAppend.kZ ) + ( oldMatrix.tZ * matrixToAppend.kW );
	kW = ( oldMatrix.iW * matrixToAppend.kX ) + ( oldMatrix.jW * matrixToAppend.kY ) + ( oldMatrix.kW * matrixToAppend.kZ ) + ( oldMatrix.tW * matrixToAppend.kW );

	tX = ( oldMatrix.iX * matrixToAppend.tX ) + ( oldMatrix.jX * matrixToAppend.tY ) + ( oldMatrix.kX * matrixToAppend.tZ ) + ( oldMatrix.tX * matrixToAppend.tW );
	tY = ( oldMatrix.iY * matrixToAppend.tX ) + ( oldMatrix.jY * matrixToAppend.tY ) + ( oldMatrix.kY * matrixToAppend.tZ ) + ( oldMatrix.tY * matrixToAppend.tW );
	tZ = ( oldMatrix.iZ * matrixToAppend.tX ) + ( oldMatrix.jZ * matrixToAppend.tY ) + ( oldMatrix.kZ * matrixToAppend.tZ ) + ( oldMatrix.tZ * matrixToAppend.tW );
	tW = ( oldMatrix.iW * matrixToAppend.tX ) + ( oldMatrix.jW * matrixToAppend.tY ) + ( oldMatrix.kW * matrixToAppend.tZ ) + ( oldMatrix.tW * matrixToAppend.tW );
}

void Matrix44::RotateDegrees2D( float rotationDegreesAboutZ )
{
	Matrix44 rotationMatrix = Matrix44::MakeRotationDegrees2D( rotationDegreesAboutZ );
	Append( rotationMatrix );
}

void Matrix44::Translate2D( const Vector2& translation )
{
	Matrix44 translationMatrix = Matrix44::MakeTranslation2D( translation );
	Append( translationMatrix );
}

void Matrix44::ScaleUniform2D( float scaleXY )
{
	Matrix44 translationMatrix = Matrix44::MakeScaleUniform2D( scaleXY );
	Append( translationMatrix );
}

void Matrix44::Scale2D( float scaleX, float scaleY )
{
	Matrix44 translationMatrix = Matrix44::MakeScale2D( scaleX, scaleY );
	Append( translationMatrix );
}

Matrix44 Matrix44::MakeRotationDegrees2D( float rotationDegreesAboutZ )
{
	float cosDegreesAboutZ = CosDegrees( rotationDegreesAboutZ );
	float sinDegreesAboutZ = SinDegrees( rotationDegreesAboutZ );

	float matrixValuesBasisMajor[ 16 ] =
	{
		cosDegreesAboutZ,	sinDegreesAboutZ,	0.0f,	0.0f,
		-sinDegreesAboutZ,	cosDegreesAboutZ,	0.0f,	0.0f,
		0.0f,	0.0f,	1.0f,	0.0f,
		0.0f,	0.0f,	0.0f,	1.0f
	};

	return Matrix44( matrixValuesBasisMajor );
}

Matrix44 Matrix44::MakeTranslation2D( const Vector2& translation )
{
	float matrixValuesBasisMajor[ 16 ] =
	{
		1.0f,	0.0f,	0.0f,	0.0f,
		0.0f,	1.0f,	0.0f,	0.0f,
		0.0f,	0.0f,	1.0f,	0.0f,
		translation.x,	translation.y,	0.0f,	1.0f
	};

	return Matrix44( matrixValuesBasisMajor );
}

Matrix44 Matrix44::MakeScaleUniform2D( float scaleXY )
{
	float matrixValuesBasisMajor[ 16 ] =
	{
		scaleXY,	0.0f,	0.0f,	0.0f,
		0.0f,	scaleXY,	0.0f,	0.0f,
		0.0f,	0.0f,	1.0f,	0.0f,
		0.0f,	0.0f,	0.0f,	1.0f
	};

	return Matrix44( matrixValuesBasisMajor );
}

Matrix44 Matrix44::MakeScale2D( float scaleX, float scaleY )
{
	float matrixValuesBasisMajor[ 16 ] =
	{
		scaleX,	0.0f,	0.0f,	0.0f,
		0.0f,	scaleY,	0.0f,	0.0f,
		0.0f,	0.0f,	1.0f,	0.0f,
		0.0f,	0.0f,	0.0f,	1.0f
	};

	return Matrix44( matrixValuesBasisMajor );
}

Matrix44 Matrix44::MakeOrtho2D( const Vector2& bottomLeft, const Vector2& topRight )
{
	ASSERT_OR_DIE( !IsFloatEqualTo( ( topRight.x - bottomLeft.x ), 0.0f ), "Matrix44::MakeOrtho2D - topRight and bottomLeft of ortho have same x values, which is not valid. Aborting..." );
	ASSERT_OR_DIE( !IsFloatEqualTo( ( topRight.y - bottomLeft.y ), 0.0f ), "Matrix44::MakeOrtho2D - topRight and bottomLeft of ortho have same y values, which is not valid. Aborting..." );

	float scaleX = 2.0f / ( topRight.x - bottomLeft.x );
	float scaleY = 2.0f / ( topRight.y - bottomLeft.y );
	Vector2 center = Vector2( ( ( bottomLeft.x + topRight.x ) * ( scaleX / 2.0f ) ), ( ( bottomLeft.y + topRight.y ) * ( scaleY / 2.0f ) ) );

	float matrixValuesBasisMajor[ 16 ] =
	{
		scaleX,	0.0f,	0.0f,	0.0f,
		0.0f,	scaleY,	0.0f,	0.0f,
		0.0f,	0.0f,	-2.0f,	0.0f,		// kZ is -2 since the range z[0, 1] needs to become z[1, -1]
		-center.x,	-center.y,	-1.0f,	1.0f
	};

	return Matrix44( matrixValuesBasisMajor );
}
