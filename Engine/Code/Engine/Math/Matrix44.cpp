#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Matrix44.hpp"

const Matrix44 Matrix44::IDENTITY;

Matrix44::Matrix44()
{
	
}

Matrix44::Matrix44( const Matrix44& copy )
{
	iX = copy.iX; iY = copy.iY; iZ = copy.iZ; iW = copy.iW;
	jX = copy.jX; jY = copy.jY; jZ = copy.jZ; jW = copy.jW;
	kX = copy.kX; kY = copy.kY; kZ = copy.kZ; kW = copy.kW;
	tX = copy.tX; tY = copy.tY; tZ = copy.tZ; tW = copy.tW;
}

Matrix44::Matrix44( const float* sixteenValuesBasisMajor )
{
	iX = sixteenValuesBasisMajor[ 0 ]; iY = sixteenValuesBasisMajor[ 1 ]; iZ = sixteenValuesBasisMajor[ 2 ]; iW = sixteenValuesBasisMajor[ 3 ];
	jX = sixteenValuesBasisMajor[ 4 ]; jY = sixteenValuesBasisMajor[ 5 ]; jZ = sixteenValuesBasisMajor[ 6 ]; jW = sixteenValuesBasisMajor[ 7 ];
	kX = sixteenValuesBasisMajor[ 8 ]; kY = sixteenValuesBasisMajor[ 9 ]; kZ = sixteenValuesBasisMajor[ 10 ]; kW = sixteenValuesBasisMajor[ 11 ];
	tX = sixteenValuesBasisMajor[ 12 ]; tY = sixteenValuesBasisMajor[ 13 ]; tZ = sixteenValuesBasisMajor[ 14 ]; tW = sixteenValuesBasisMajor[ 15 ];
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

Matrix44::Matrix44( const Vector3& iBasis, const Vector3& jBasis, const Vector3& kBasis, const Vector3& translation )
{
	iX = iBasis.x;
	iY = iBasis.y;
	iZ = iBasis.z;
	jX = jBasis.x;
	jY = jBasis.y;
	jZ = jBasis.z;
	kX = kBasis.x;
	kY = kBasis.y;
	kZ = kBasis.z;
	tX = translation.x;
	tY = translation.y;
	tZ = translation.z;
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

Vector3 Matrix44::TransformPosition( const Vector3& position )
{
	return Vector3( 
		( iX * position.x + jX * position.y + kX * position.z + tX ),
		( iY * position.x + jY * position.y + kY * position.z + tY ),
		( iZ * position.x + jZ * position.y + kZ * position.z + tZ )
	);
}

Vector3 Matrix44::TransformDisplacement( const Vector3& displacement )
{
	return Vector3( 
		( iX * displacement.x + jX * displacement.y + kX * displacement.z ),
		( iY * displacement.x + jY * displacement.y + kY * displacement.z ),
		( iZ * displacement.x + jZ * displacement.y + kZ * displacement.z )
	);
}

Vector4 Matrix44::Transform( const Vector4& vector )
{
	return Vector4(
		( iX * vector.x + jX * vector.y + kX * vector.z + tX * vector.w ),
		( iY * vector.x + jY * vector.y + kY * vector.z + tY * vector.w ),
		( iZ * vector.x + jZ * vector.y + kZ * vector.z + tZ * vector.w ),
		( iW * vector.x + jW * vector.y + kW * vector.z + tW * vector.w )
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
	iX = sixteenValuesBasisMajor[ 0 ]; iY = sixteenValuesBasisMajor[ 1 ]; iZ = sixteenValuesBasisMajor[ 2 ]; iW = sixteenValuesBasisMajor[ 3 ];
	jX = sixteenValuesBasisMajor[ 4 ]; jY = sixteenValuesBasisMajor[ 5 ]; jZ = sixteenValuesBasisMajor[ 6 ]; jW = sixteenValuesBasisMajor[ 7 ];
	kX = sixteenValuesBasisMajor[ 8 ]; kY = sixteenValuesBasisMajor[ 9 ]; kZ = sixteenValuesBasisMajor[ 10 ]; kW = sixteenValuesBasisMajor[ 11 ];
	tX = sixteenValuesBasisMajor[ 12 ]; tY = sixteenValuesBasisMajor[ 13 ]; tZ = sixteenValuesBasisMajor[ 14 ]; tW = sixteenValuesBasisMajor[ 15 ];
}

void Matrix44::SetIBasis( const Vector3& newIBasis )
{
	iX = newIBasis.x;
	iY = newIBasis.y;
	iZ = newIBasis.z;
}

void Matrix44::SetJBasis( const Vector3& newJBasis )
{
	jX = newJBasis.x;
	jY = newJBasis.y;
	jZ = newJBasis.z;
}

void Matrix44::SetKBasis( const Vector3& newKBasis )
{
	kX = newKBasis.x;
	kY = newKBasis.y;
	kZ = newKBasis.z;
}

void Matrix44::SetTranslation( const Vector3& newTranslation )
{
	tX = newTranslation.x;
	tY = newTranslation.y;
	tZ = newTranslation.z;
}

void Matrix44::SetBases( const Vector3& newIBasis, const Vector3& newJBasis, const Vector3& newKBasis )
{
	SetIBasis( newIBasis );
	SetJBasis( newJBasis );
	SetKBasis( newKBasis );
}

void Matrix44::SetBases( const Vector3& newIBasis, const Vector3& newJBasis, const Vector3& newKBasis, const Vector3& newTranslation )
{
	SetBases( newIBasis, newJBasis, newKBasis );
	SetTranslation( newTranslation );
}

void Matrix44::NormalizeBases()
{
	float iMagnitude = GetIBasis().GetLength(); if ( iMagnitude > 0.0f ) { iX /= iMagnitude; iY /= iMagnitude; iZ /= iMagnitude; }
	float jMagnitude = GetJBasis().GetLength(); if ( jMagnitude > 0.0f ) { jX /= jMagnitude; jY /= jMagnitude; jZ /= jMagnitude; }
	float kMagnitude = GetKBasis().GetLength(); if ( kMagnitude > 0.0f ) { kX /= kMagnitude; kY /= kMagnitude; kZ /= kMagnitude; }
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

void Matrix44::FastInvert()
{
	// Transpose rotation
	float temp = iY;
	iY = jX;
	jX = temp;
	temp = iZ;
	iZ = kX;
	kX = temp;
	temp = kY;
	kY = jZ;
	jZ = temp;

	// Negate translation
	tX *= -1.0f;
	tY *= -1.0f;
	tZ *= -1.0f;
}

void Matrix44::Invert()
{
	Matrix44 inverse = GetInverse();
	*this = inverse;
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

void Matrix44::Translate( const Vector3& translation )
{
	Matrix44 translationMatrix = Matrix44::MakeTranslation( translation );
	Append( translationMatrix );
}

void Matrix44::RotateAbout( float rotationDegrees, float azimuthDegrees, const Vector3& pivot )
{
	Vector3 currentPosition = GetTranslation();
	currentPosition -= pivot;
	currentPosition.ConvertToPolar();

	currentPosition.y += rotationDegrees;
	currentPosition.z += azimuthDegrees;

	currentPosition.ConvertToCartesian();
	currentPosition += pivot;
	SetTranslation( currentPosition );
}

void Matrix44::ScaleUniform( float scaleXYZ )
{
	Matrix44 translationMatrix = Matrix44::MakeScaleUniform( scaleXYZ );
	Append( translationMatrix );
}

void Matrix44::Scale( float scaleX, float scaleY, float scaleZ )
{
	Matrix44 translationMatrix = Matrix44::MakeScale( scaleX, scaleY, scaleZ );
	Append( translationMatrix );
}

void Matrix44::Transpose()
{
	Matrix44 copy = Matrix44( *this );

	iY = copy.jX;
	iZ = copy.kX;
	iW = copy.tX;

	jX = copy.iY;
	jZ = copy.kY;
	jW = copy.tY;

	kX = copy.iZ;
	kY = copy.jZ;
	kW = copy.tZ;

	tX = copy.iW;
	tY = copy.jW;
	tZ = copy.kW;
}

void Matrix44::TurnToward( const Matrix44& target, float maxTurnDegrees )
{
	// Get rotation matrix from current and target
	Matrix44 inverseOfCurrent = GetFastInverse();
	Matrix44 rotationMatrix =  inverseOfCurrent * target;	// Since rotation * current = target

	// Compute theta from rotation matrix
	float trace = rotationMatrix.GetTrace33();	// 1 + 2 * cos( theta ) = trace
	float cosTheta = ( trace - 1.0f ) * 0.5f;
	cosTheta = ClampFloat( cosTheta, -1.0f, 1.0f );
	float thetaDegrees = ACosDegrees( cosTheta );

	if ( thetaDegrees > 0.0f )
	{
		// Set the matrix as the lerped matrix
		float t = Min ( ( maxTurnDegrees / thetaDegrees ), 1.0f );
		Matrix44 interpolatedMatrix = Interpolate( *this, target, t );
		*this = interpolatedMatrix;
	}
}

const Matrix44 Matrix44::operator+( const Matrix44& matrixToAdd ) const
{
	Matrix44 sum = Matrix44( *this );
	sum.iX += matrixToAdd.iX; sum.iY += matrixToAdd.iY; sum.iZ += matrixToAdd.iZ; sum.iW += matrixToAdd.iW;
	sum.jX += matrixToAdd.jX; sum.jY += matrixToAdd.jY; sum.jZ += matrixToAdd.jZ; sum.jW += matrixToAdd.jW;
	sum.kX += matrixToAdd.kX; sum.kY += matrixToAdd.kY; sum.kZ += matrixToAdd.kZ; sum.kW += matrixToAdd.kW;
	sum.tX += matrixToAdd.tX; sum.tY += matrixToAdd.tY; sum.tZ += matrixToAdd.tZ; sum.tW += matrixToAdd.tW;
	return sum;
}

const Matrix44 Matrix44::operator-( const Matrix44& matrixToAdd ) const
{
	Matrix44 difference = Matrix44( *this );
	difference.iX -= matrixToAdd.iX; difference.iY -= matrixToAdd.iY; difference.iZ -= matrixToAdd.iZ; difference.iW -= matrixToAdd.iW;
	difference.jX -= matrixToAdd.jX; difference.jY -= matrixToAdd.jY; difference.jZ -= matrixToAdd.jZ; difference.jW -= matrixToAdd.jW;
	difference.kX -= matrixToAdd.kX; difference.kY -= matrixToAdd.kY; difference.kZ -= matrixToAdd.kZ; difference.kW -= matrixToAdd.kW;
	difference.tX -= matrixToAdd.tX; difference.tY -= matrixToAdd.tY; difference.tZ -= matrixToAdd.tZ; difference.tW -= matrixToAdd.tW;
	return difference;
}

const Matrix44 Matrix44::operator*( const Matrix44& matrixToMultiply ) const
{
	Matrix44 product = Matrix44( *this );
	product.Append( matrixToMultiply );
	return product;
}

const Vector4 Matrix44::operator*( const Vector4& vectorToMultiply ) const
{
	return Vector4(
		DotProduct( Vector4( iX, jX, kX, tX ), vectorToMultiply ),
		DotProduct( Vector4( iY, jY, kY, tY ), vectorToMultiply ),
		DotProduct( Vector4( iZ, jZ, kZ, tZ ), vectorToMultiply ),
		DotProduct( Vector4( iW, jW, kW, tW ), vectorToMultiply )
	);
}

void Matrix44::operator=( const Matrix44& matrixToCopy )
{
	iX = matrixToCopy.iX; iY = matrixToCopy.iY; iZ = matrixToCopy.iZ; iW = matrixToCopy.iW;
	jX = matrixToCopy.jX; jY = matrixToCopy.jY; jZ = matrixToCopy.jZ; jW = matrixToCopy.jW;
	kX = matrixToCopy.kX; kY = matrixToCopy.kY; kZ = matrixToCopy.kZ; kW = matrixToCopy.kW;
	tX = matrixToCopy.tX; tY = matrixToCopy.tY; tZ = matrixToCopy.tZ; tW = matrixToCopy.tW;
}

void Matrix44::operator+=( const Matrix44& matrixToAdd )
{
	iX += matrixToAdd.iX; jX += matrixToAdd.jX;	kX += matrixToAdd.kX;	tX += matrixToAdd.tX;
	iY += matrixToAdd.iY; jY += matrixToAdd.jY;	kY += matrixToAdd.kY;	tY += matrixToAdd.tY;
	iZ += matrixToAdd.iZ; jZ += matrixToAdd.jZ;	kZ += matrixToAdd.kZ;	tZ += matrixToAdd.tZ;
	iW += matrixToAdd.iW; jW += matrixToAdd.jW;	kW += matrixToAdd.kW;	tW += matrixToAdd.tW;
}

void Matrix44::operator-=( const Matrix44& matrixToAdd )
{
	iX -= matrixToAdd.iX; jX -= matrixToAdd.jX;	kX -= matrixToAdd.kX;	tX -= matrixToAdd.tX;
	iY -= matrixToAdd.iY; jY -= matrixToAdd.jY;	kY -= matrixToAdd.kY;	tY -= matrixToAdd.tY;
	iZ -= matrixToAdd.iZ; jZ -= matrixToAdd.jZ;	kZ -= matrixToAdd.kZ;	tZ -= matrixToAdd.tZ;
	iW -= matrixToAdd.iW; jW -= matrixToAdd.jW;	kW -= matrixToAdd.kW;	tW -= matrixToAdd.tW;
}

void Matrix44::operator*=( const Matrix44& matrixToMultiply )
{
	*this = *this * matrixToMultiply;
}

Vector3 Matrix44::GetIBasis() const
{
	return Vector3( iX, iY, iZ );
}

Vector3 Matrix44::GetJBasis() const
{
	return Vector3( jX, jY, jZ );
}

Vector3 Matrix44::GetKBasis() const
{
	return Vector3( kX, kY, kZ );
}

Matrix44 Matrix44::GetTranspose() const
{
	Matrix44 transpose = Matrix44( *this );
	transpose.Transpose();
	return transpose;
}

Matrix44 Matrix44::GetFastInverse() const
{
	Matrix44 inverse = Matrix44( *this );
	inverse.FastInvert();
	return inverse;
}

/* Implementation from: https://stackoverflow.com/questions/1148309/inverting-a-4x4-matrix */
Matrix44 Matrix44::GetInverse() const
{
	Matrix44 inverse = Matrix44::IDENTITY;
	float determinant = 0.0f;

	inverse.iX =
		jY  * kZ * tW - 
		jY  * kW * tZ - 
		kY  * jZ  * tW + 
		kY  * jW  * tZ +
		tY * jZ  * kW - 
		tY * jW  * kZ;

	inverse.jX =
		-jX  * kZ * tW + 
		jX  * kW * tZ + 
		kX  * jZ  * tW - 
		kX  * jW  * tZ - 
		tX * jZ  * kW + 
		tX * jW  * kZ;

	inverse.kX =
		jX  * kY * tW - 
		jX  * kW * tY - 
		kX  * jY * tW + 
		kX  * jW * tY + 
		tX * jY * kW - 
		tX * jW * kY;

	inverse.tX =
		-jX  * kY * tZ + 
		jX  * kZ * tY +
		kX  * jY * tZ - 
		kX  * jZ * tY - 
		tX * jY * kZ + 
		tX * jZ * kY;

	inverse.iY =
		-iY  * kZ * tW + 
		iY  * kW * tZ + 
		kY  * iZ * tW - 
		kY  * iW * tZ - 
		tY * iZ * kW + 
		tY * iW * kZ;

	inverse.jY =
		iX  * kZ * tW - 
		iX  * kW * tZ - 
		kX  * iZ * tW + 
		kX  * iW * tZ + 
		tX * iZ * kW - 
		tX * iW * kZ;

	inverse.kY =
		-iX  * kY * tW + 
		iX  * kW * tY + 
		kX  * iY * tW - 
		kX  * iW * tY - 
		tX * iY * kW + 
		tX * iW * kY;

	inverse.tY =
		iX  * kY * tZ - 
		iX  * kZ * tY - 
		kX  * iY * tZ + 
		kX  * iZ * tY + 
		tX * iY * kZ - 
		tX * iZ * kY;

	inverse.iZ =
		iY  * jZ * tW - 
		iY  * jW * tZ - 
		jY  * iZ * tW + 
		jY  * iW * tZ + 
		tY * iZ * jW - 
		tY * iW * jZ;

	inverse.jZ =
		-iX  * jZ * tW + 
		iX  * jW * tZ + 
		jX  * iZ * tW - 
		jX  * iW * tZ - 
		tX * iZ * jW + 
		tX * iW * jZ;

	inverse.kZ =
		iX  * jY * tW - 
		iX  * jW * tY - 
		jX  * iY * tW + 
		jX  * iW * tY + 
		tX * iY * jW - 
		tX * iW * jY;

	inverse.tZ =
		-iX  * jY * tZ + 
		iX  * jZ * tY + 
		jX  * iY * tZ - 
		jX  * iZ * tY - 
		tX * iY * jZ + 
		tX * iZ * jY;

	inverse.iW =
		-iY * jZ * kW + 
		iY * jW * kZ + 
		jY * iZ * kW - 
		jY * iW * kZ - 
		kY * iZ * jW + 
		kY * iW * jZ;

	inverse.jW =
		iX * jZ * kW - 
		iX * jW * kZ - 
		jX * iZ * kW + 
		jX * iW * kZ + 
		kX * iZ * jW - 
		kX * iW * jZ;

	inverse.kW =
		-iX * jY * kW + 
		iX * jW * kY + 
		jX * iY * kW - 
		jX * iW * kY - 
		kX * iY * jW + 
		kX * iW * jY;

	inverse.tW =
		iX * jY * kZ - 
		iX * jZ * kY - 
		jX * iY * kZ + 
		jX * iZ * kY + 
		kX * iY * jZ - 
		kX * iZ * jY;

	determinant = iX * inverse.iX + iY * inverse.jX + iZ * inverse.kX + iW * inverse.tX;

	if ( determinant == 0 )
	{
		return Matrix44::IDENTITY;
	}

	determinant = 1.0f / determinant;

	inverse.iX = inverse.iX * determinant;
	inverse.iY = inverse.iY * determinant;
	inverse.iZ = inverse.iZ * determinant;
	inverse.iW = inverse.iW * determinant;
	inverse.jX = inverse.jX * determinant;
	inverse.jY = inverse.jY * determinant;
	inverse.jZ = inverse.jZ * determinant;
	inverse.jW = inverse.jW * determinant;
	inverse.kX = inverse.kX * determinant;
	inverse.kY = inverse.kY * determinant;
	inverse.kZ = inverse.kZ * determinant;
	inverse.kW = inverse.kW * determinant;
	inverse.tX = inverse.tX * determinant;
	inverse.tY = inverse.tY * determinant;
	inverse.tZ = inverse.tZ * determinant;
	inverse.tW = inverse.tW * determinant;

	return inverse;
}

float Matrix44::GetTrace() const
{
	float trace = iX + jY + kZ + tW;
	return trace;
}

float Matrix44::GetTrace33() const
{
	float trace33 = iX + jY + kZ;
	return trace33;
}

Vector3 Matrix44::GetEulerAngles() const
{
	// Use Matrix44::MakeFromEuler as a reference
	Vector3 eulerAngles = Vector3::ZERO;

	float sinX = -kY;
	sinX = ClampFloat( sinX, -1.0f, 1.0f );
	eulerAngles.x = ASinDegrees( sinX );
	float cosX = CosDegrees( eulerAngles.x );
	
	if ( !IsFloatEqualTo( cosX, 0.0f ) )
	{
		eulerAngles.y = ATan2Degrees( kX, kZ );

		eulerAngles.z = ATan2Degrees( iY, jY );
	}
	else
	{
		// Gimbal lock (since we pitched 90 degrees up or down) - one degree of freedom lost, so we choose to set roll to 0 instead of yaw
		eulerAngles.y = ATan2Degrees( jX, iX );

		eulerAngles.z = 0.0f;
	}

	return eulerAngles;
}

Vector3 Matrix44::GetTranslation() const
{
	return Vector3( tX, tY, tZ );
}

/* static */
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

/* static */
Matrix44 Matrix44::MakeYaw( float rotationDegreesAboutY )
{
	float cosDegreesAboutY = CosDegrees( rotationDegreesAboutY );
	float sinDegreesAboutY = SinDegrees( rotationDegreesAboutY );

	float matrixValuesBasisMajor[ 16 ] =
	{
		cosDegreesAboutY,	0.0f,	-sinDegreesAboutY,	0.0f,
		0.0f,	1.0f,	0.0f,	0.0f,
		sinDegreesAboutY,	0.0f,	cosDegreesAboutY,	0.0f,
		0.0f,	0.0f,	0.0f,	1.0f
	};

	return Matrix44( matrixValuesBasisMajor );
}

/* static */
Matrix44 Matrix44::MakePitch( float rotationDegreesAboutX )
{
	float cosDegreesAboutX = CosDegrees( rotationDegreesAboutX );
	float sinDegreesAboutX = SinDegrees( rotationDegreesAboutX );

	float matrixValuesBasisMajor[ 16 ] =
	{
		1.0f,	0.0f,	0.0f,	0.0f,
		0.0f,	cosDegreesAboutX,	sinDegreesAboutX,	0.0f,
		0.0f,	-sinDegreesAboutX,	cosDegreesAboutX,	0.0f,
		0.0f,	0.0f,	0.0f,	1.0f
	};

	return Matrix44( matrixValuesBasisMajor );
}

/* static */
Matrix44 Matrix44::MakeRoll( float rotationDegreesAboutZ )
{
	return Matrix44::MakeRotationDegrees2D( rotationDegreesAboutZ );
}

/* static */
Matrix44 Matrix44::MakeRotation( const Vector3& eulerAngles )
{
	Matrix44 rotation = Matrix44::MakeYaw( eulerAngles.y );
	rotation.Append( Matrix44::MakePitch( eulerAngles.x ) );
	rotation.Append( Matrix44::MakeRoll( eulerAngles.z ) );
	return rotation;
}

/* static */
Matrix44 Matrix44::MakeFromEuler( const Vector3& eulerAngles )
{
	float cosX = CosDegrees( eulerAngles.x );
	float sinX = SinDegrees( eulerAngles.x );
	float cosY = CosDegrees( eulerAngles.y );
	float sinY = SinDegrees( eulerAngles.y );
	float cosZ = CosDegrees( eulerAngles.z );
	float sinZ = SinDegrees( eulerAngles.z );

	float valuesBasisMajor[] = {
		( ( cosY * cosZ ) + ( sinX * sinY * sinZ ) ),	( cosX * sinZ ),	( -( sinY * cosZ ) + ( sinX * cosY * sinZ ) ),	0.0f,
		( -( cosY * sinZ ) + ( sinX * sinY * cosZ ) ),	( cosX * cosZ ),	( ( sinY * sinZ) + ( sinX * cosY * cosZ ) ),	0.0f,
		( cosX * sinY ),								-sinX,				( cosX * cosY ),								0.0f,
		0.0f,											0.0f,											0.0f,				1.0f
	};

	return Matrix44( valuesBasisMajor );
}

/* static */
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

/* static */
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

/* static */
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

/* static */
Matrix44 Matrix44::MakeTranslation( const Vector3& translation )
{
	float matrixValuesBasisMajor[ 16 ] =
	{
		1.0f,	0.0f,	0.0f,	0.0f,
		0.0f,	1.0f,	0.0f,	0.0f,
		0.0f,	0.0f,	1.0f,	0.0f,
		translation.x,	translation.y,	translation.z,	1.0f
	};

	return Matrix44( matrixValuesBasisMajor );
}

/* static */
Matrix44 Matrix44::MakeScaleUniform( float scaleXYZ )
{
	float matrixValuesBasisMajor[ 16 ] =
	{
		scaleXYZ,	0.0f,	0.0f,	0.0f,
		0.0f,	scaleXYZ,	0.0f,	0.0f,
		0.0f,	0.0f,	scaleXYZ,	0.0f,
		0.0f,	0.0f,	0.0f,	1.0f
	};

	return Matrix44( matrixValuesBasisMajor );
}

/* static */
Matrix44 Matrix44::MakeScale( float scaleX, float scaleY, float scaleZ )
{
	float matrixValuesBasisMajor[ 16 ] =
	{
		scaleX,	0.0f,	0.0f,	0.0f,
		0.0f,	scaleY,	0.0f,	0.0f,
		0.0f,	0.0f,	scaleZ,	0.0f,
		0.0f,	0.0f,	0.0f,	1.0f
	};

	return Matrix44( matrixValuesBasisMajor );
}

/* static */
Matrix44 Matrix44::MakeScale( const Vector3& scale )
{
	float matrixValuesBasisMajor[ 16 ] =
	{
		scale.x,	0.0f,	0.0f,	0.0f,
		0.0f,	scale.y,	0.0f,	0.0f,
		0.0f,	0.0f,	scale.z,	0.0f,
		0.0f,	0.0f,	0.0f,	1.0f
	};

	return Matrix44( matrixValuesBasisMajor );
}

/* static */
Matrix44 Matrix44::MakeOrtho2D( const Vector2& bottomLeft, const Vector2& topRight )
{
	ASSERT_OR_DIE( !IsFloatEqualTo( ( topRight.x - bottomLeft.x ), 0.0f ), "ERROR: Matrix44::MakeOrtho2D - topRight and bottomLeft of ortho have same x values, which is not valid. Aborting..." );
	ASSERT_OR_DIE( !IsFloatEqualTo( ( topRight.y - bottomLeft.y ), 0.0f ), "ERROR: Matrix44::MakeOrtho2D - topRight and bottomLeft of ortho have same y values, which is not valid. Aborting..." );

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

/* static */
Matrix44 Matrix44::MakePerspective( float fovDegrees, float aspect, float nearZ, float farZ )
{
	ASSERT_OR_DIE( !IsFloatEqualTo( ( farZ - nearZ ), 0.0f ), "ERROR: Matrix44::MakePerspective - nearZ cannot be the same as farZ." );
	
	float d = 1.0f / TanDegrees( fovDegrees );
	float q = 1.0f / ( farZ - nearZ ); 

	float matrixValuesBasisMajor[ 16 ] =
	{
		( d / aspect ),		0,    0,								0, 
		0,					d,    0,								0,
		0,					0,    ( ( nearZ + farZ ) * q ),			1,
		0,					0,    ( -2.0f * nearZ * farZ * q ),		0
	};

	return Matrix44( matrixValuesBasisMajor ); 
}

/* static */
Matrix44 Matrix44::LookAt( const Vector3& position, const Vector3& target, const Vector3& up/* = Vector3::UP*/ )
{
	ASSERT_RECOVERABLE( ( position != target ), "WARNING: Matrix44::LookAt - camera position same as target - special case not handled yet." );

	Vector3 cameraForward = ( target - position ).GetNormalized();
	Vector3 cameraUp = ( up - ( cameraForward * up.GetProjectionInDirection( cameraForward ) ) ).GetNormalized();
	ASSERT_RECOVERABLE( !IsFloatEqualTo( cameraUp.GetLengthSquared(), 0.0f ), "WARNING: Matrix44::LookAt - camera looking along up axis - special case not handled yet." );

	Vector3 cameraRight = CrossProduct( cameraUp, cameraForward ).GetNormalized();	// Works for a Left-Handed coordinate system

	Vector3 viewSpaceTranslation = Vector3( DotProduct( position, cameraRight ), DotProduct( position, cameraUp ), DotProduct( position, cameraForward ) );
	Matrix44 lookAtMatrix = Matrix44( cameraRight, cameraUp, cameraForward, viewSpaceTranslation );
	return lookAtMatrix;
}

/* static */
Matrix44 Matrix44::GetTurnTowardMatrix( const Matrix44& current, const Matrix44& target, float maxTurnDegrees )
{
	Matrix44 turnTowardMatrix = Matrix44( current );
	turnTowardMatrix.TurnToward( target, maxTurnDegrees );
	return turnTowardMatrix;
}

// STANDALONE FUNCTIONS

Matrix44 Interpolate( const Matrix44& start, const Matrix44& end, float fractionTowardEnd )
{
	Vector3 sLerpedRight = SLerp( start.GetIBasis(), end.GetIBasis(), fractionTowardEnd );
	Vector3 sLerpedUp = SLerp( start.GetJBasis(), end.GetJBasis(), fractionTowardEnd );
	Vector3 sLerpedForward = SLerp( start.GetKBasis(), end.GetKBasis(), fractionTowardEnd );
	Vector3 lerpedTranslation = Interpolate( start.GetTranslation(), end.GetTranslation(), fractionTowardEnd );

	// Re-orthogonalize to account for possible SLerp errors, using Up as the correct vector
	sLerpedUp.NormalizeAndGetLength();
	sLerpedRight = sLerpedRight - ( sLerpedUp * sLerpedRight.GetComponentInDirection( sLerpedUp ) );
	sLerpedRight.NormalizeAndGetLength();
	sLerpedForward = CrossProduct( sLerpedRight, sLerpedUp );

	Matrix44 lerpedMatrix = Matrix44( sLerpedRight, sLerpedUp, sLerpedForward, lerpedTranslation );
	return lerpedMatrix;
}
