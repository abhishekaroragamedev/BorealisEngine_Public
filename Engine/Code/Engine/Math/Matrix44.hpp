#pragma once

#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Vector4.hpp"

class Matrix44
{

public:
	Matrix44();
	Matrix44( const Matrix44& copy );
	explicit Matrix44( const float* sixteenValuesBasisMajor );
	explicit Matrix44( const Vector2& iBasis, const Vector2& jBasis, const Vector2& translation );
	explicit Matrix44( const Vector3& iBasis, const Vector3& jBasis, const Vector3& kBasis, const Vector3& translation );

	Vector2 TransformPosition2D( const Vector2& position2D );
	Vector2 TransformDisplacement2D( const Vector2& displacement2D );
	Vector3 TransformPosition( const Vector3& position );
	Vector3 TransformDisplacement( const Vector3& displacement );
	Vector4 Transform( const Vector4& vector );

	Vector3 GetTranslation() const;
	Vector3 GetEulerAngles() const;
	Vector3 GetIBasis() const;
	Vector3 GetJBasis() const;
	Vector3 GetKBasis() const;
	Matrix44 GetTranspose() const;
	Matrix44 GetFastInverse() const;	// Assumes basis vectors are orthonormal
	Matrix44 GetInverse() const;
	float GetTrace() const;
	float GetTrace33() const;	// Only the 3x3 matrix is considered for Trace value

	void SetIdentity();
	void SetValues( const float* sixteenValuesBasisMajor );
	void SetIBasis( const Vector3& newIBasis );
	void SetJBasis( const Vector3& newJBasis );
	void SetKBasis( const Vector3& newKBasis );
	void SetTranslation( const Vector3& newTranslation );
	void SetBases( const Vector3& newIBasis, const Vector3& newJBasis, const Vector3& newKBasis );
	void SetBases( const Vector3& newIBasis, const Vector3& newJBasis, const Vector3& newKBasis, const Vector3& newTranslation );
	void NormalizeBases();
	void Append( const Matrix44& matrixToAppend );	// this = [matrixToAppend] * [this] (multiplied on the right) -> Always append in "reverse"!
	void FastInvert();		// Transpose rotation and negate translation
	void Invert();
	void RotateDegrees2D( float rotationDegreesAboutZ );
	void Translate2D( const Vector2& translation );
	void ScaleUniform2D( float scaleXY );
	void Scale2D( float scaleX, float scaleY );
	void Translate( const Vector3& translation );
	void RotateAbout( float rotationDegrees, float azimuthDegrees, const Vector3& pivot );
	void ScaleUniform( float scaleXYZ );
	void Scale( float scaleX, float scaleY, float scaleZ );
	void Transpose();
	void TurnToward( const Matrix44& target, float maxTurnDegrees );	// Assumes basis vectors are orthonormal

	const Matrix44 operator+( const Matrix44& matrixToAdd ) const;
	const Matrix44 operator-( const Matrix44& matrixToAdd ) const;
	const Matrix44 operator*( const Matrix44& matrixToMultiply ) const;	// this = [matrixToMultiply] * [this] (multiplied on the right) -> Always multiply in "reverse"!
	const Vector4 operator*( const Vector4& vectorToMultiply ) const;
	void operator=( const Matrix44& matrixToCopy );
	void operator+=( const Matrix44& matrixToAdd );
	void operator-=( const Matrix44& matrixToAdd );
	void operator*=( const Matrix44& matrixToMultiply );	// this = [matrixToMultiply] * [this] (multiplied on the right) -> Always multiply in "reverse"!

	static Matrix44 MakeRotationDegrees2D( float rotationDegreesAboutZ );
	static Matrix44 MakeYaw( float rotationDegreesAboutY );	// WRONG
	static Matrix44 MakePitch( float rotationDegreesAboutX );	// WRONG
	static Matrix44 MakeRoll( float rotationDegreesAboutZ );
	static Matrix44 MakeRotation( const Vector3& eulerAngles );
	static Matrix44 MakeFromEuler( const Vector3& eulerAngles );		// Same result as MakeRotation, but faster under the hood; avoids multiple matrix multiplications
	static Matrix44 MakeTranslation2D( const Vector2& translation );
	static Matrix44 MakeScaleUniform2D( float scaleXY );
	static Matrix44 MakeScale2D( float scaleX, float scaleY );
	static Matrix44 MakeTranslation( const Vector3& translation );
	static Matrix44 MakeScaleUniform( float scaleXYZ );
	static Matrix44 MakeScale( float scaleX, float scaleY, float scaleZ );
	static Matrix44 MakeScale( const Vector3& scale );
	static Matrix44 MakeOrtho2D( const Vector2& bottomLeft, const Vector2& topRight );
	static Matrix44 MakePerspective( float fovDegrees, float aspect, float nearZ, float farZ );
	static Matrix44 LookAt( const Vector3& position, const Vector3& target, const Vector3& up = Vector3::UP );
	static Matrix44 GetTurnTowardMatrix( const Matrix44& current, const Matrix44& target, float maxTurnDegrees );

public:
	static const Matrix44 IDENTITY;

public:
	float iX = 1.0f; float iY = 0.0f; float iZ = 0.0f; float iW = 0.0f;
	float jX = 0.0f; float jY = 1.0f; float jZ = 0.0f; float jW = 0.0f;
	float kX = 0.0f; float kY = 0.0f; float kZ = 1.0f; float kW = 0.0f;
	float tX = 0.0f; float tY = 0.0f; float tZ = 0.0f; float tW = 1.0f;

};

Matrix44 Interpolate( const Matrix44& start, const Matrix44& end, float fractionTowardEnd );
