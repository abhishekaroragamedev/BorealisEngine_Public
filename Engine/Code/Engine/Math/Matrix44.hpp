#pragma once

#include "Engine/Math/Vector2.hpp"

class Matrix44
{

public:
	Matrix44();
	explicit Matrix44( const float* sixteenValuesBasisMajor );
	explicit Matrix44( const Vector2& iBasis, const Vector2& jBasis, const Vector2& translation );

	Vector2 TransformPosition2D( const Vector2& position2D );
	Vector2 TransformDisplacement2D( const Vector2& displacement2D );

	void SetIdentity();
	void SetValues( const float* sixteenValuesBasisMajor );
	void Append( const Matrix44& matrixToAppend );
	void RotateDegrees2D( float rotationDegreesAboutZ );
	void Translate2D( const Vector2& translation );
	void ScaleUniform2D( float scaleXY );
	void Scale2D( float scaleX, float scaleY );

	static Matrix44 MakeRotationDegrees2D( float rotationDegreesAboutZ );
	static Matrix44 MakeTranslation2D( const Vector2& translation );
	static Matrix44 MakeScaleUniform2D( float scaleXY );
	static Matrix44 MakeScale2D( float scaleX, float scaleY );
	static Matrix44 MakeOrtho2D( const Vector2& bottomLeft, const Vector2& topRight );

public:
	float iX = 1.0f; float iY = 0.0f; float iZ = 0.0f; float iW = 0.0f;
	float jX = 0.0f; float jY = 1.0f; float jZ = 0.0f; float jW = 0.0f;
	float kX = 0.0f; float kY = 0.0f; float kZ = 1.0f; float kW = 0.0f;
	float tX = 0.0f; float tY = 0.0f; float tZ = 0.0f; float tW = 1.0f;

};
