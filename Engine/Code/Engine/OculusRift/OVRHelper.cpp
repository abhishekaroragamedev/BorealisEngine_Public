#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/Matrix44.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/OculusRift/OVRHelper.hpp"
#include "Engine/OculusRift/OVRInput.hpp"

Vector2 GetEngineVector2FromOVRVector2( const ovrVector2f& ovrVec2 )
{
	Vector2 engineVector = Vector2(
		ovrVec2.x,
		ovrVec2.y
	);
	return engineVector;
}

ovrVector2f GetOVRVector2FromEngineVector2( const Vector2& engineVec2 )
{
	ovrVector2f ovrVector;
	ovrVector.x = engineVec2.x;
	ovrVector.y = engineVec2.y;
	return ovrVector;
}

Vector3 GetEngineVector3FromOVRVector3( const ovrVector3f& ovrVec3 )
{
	Vector3 engineVector = Vector3(
		ovrVec3.x,
		ovrVec3.y,
		( -1.0f * ovrVec3.z )	// The Oculus SDK's Z-axis points in the opposite direction of this Engine's Z-axis
	);
	return engineVector;
}

ovrVector3f GetOVRVector3FromEngineVector3( const Vector3& engineVec3 )
{
	ovrVector3f ovrVector;
	ovrVector.x = engineVec3.x;
	ovrVector.y = engineVec3.y;
	ovrVector.z = -1.0f * engineVec3.z;	// The Oculus SDK's Z-axis points in the opposite direction of this Engine's Z-axis
	return ovrVector;
}

Vector3 GetEngineEulerFromOVRQuaternion( ovrQuatf ovrQuaternion )
{
	float pitch;
	float yaw;
	float roll;

	OVR::Quatf quaternion = OVR::Quatf( ovrQuaternion );
	quaternion.GetEulerAngles< OVR::Axis::Axis_Y, OVR::Axis::Axis_X, OVR::Axis::Axis_Z >( &yaw, &pitch, &roll );

	pitch = ConvertRadiansToDegrees( pitch );
	yaw = ConvertRadiansToDegrees( yaw );
	roll = ConvertRadiansToDegrees( roll );

	Vector3 engineEuler = Vector3(	// Oculus uses a right-handed coordinate system, while this engine's is left-handed
		( -1.0f * pitch ),
		( -1.0f * yaw ),
		roll
	);
	return engineEuler;
}

ovrQuatf GetOVRQuaternionFromEngineEuler( const Vector3& engineEuler )
{
	Vector3 engineEulerCopy = Vector3( engineEuler );
	engineEulerCopy.x *= -1.0f;	// Conversion between Engine and OVR coordinates
	engineEulerCopy.y *= -1.0f;
	engineEulerCopy.x = ConvertDegreesToRadians( engineEulerCopy.x );
	engineEulerCopy.y = ConvertDegreesToRadians( engineEulerCopy.y );
	engineEulerCopy.z = ConvertDegreesToRadians( engineEulerCopy.z );
	Matrix44 engineRotationMatrix = Matrix44::MakeFromEuler( engineEulerCopy );

	OVR::Matrix4f ovrRotationMatrix = GetOVRMatrixFromEngineMatrix( engineRotationMatrix );
	OVR::Quatf hudOrientation = OVR::Quatf( ovrRotationMatrix );
	ovrQuatf ovrQuaternion = ovrQuatf( hudOrientation );
	return ovrQuaternion;
}

Matrix44 GetEngineMatrixFromOVRMatrix( ovrFovPort fovPort, float nearZ, float farZ, ovrProjectionModifier_ modifier )
{
	ovrMatrix4f ovrMatrix = ovrMatrix4f_Projection( fovPort, nearZ, farZ, modifier );
	
	float valuesTransposed[ 16 ] = {
		ovrMatrix.M[ 0 ][ 0 ], ovrMatrix.M[ 1 ][ 0 ], ovrMatrix.M[ 2 ][ 0 ], ovrMatrix.M[ 3 ][ 0 ],
		ovrMatrix.M[ 0 ][ 1 ], ovrMatrix.M[ 1 ][ 1 ], ovrMatrix.M[ 2 ][ 1 ], ovrMatrix.M[ 3 ][ 1 ],
		ovrMatrix.M[ 0 ][ 2 ], ovrMatrix.M[ 1 ][ 2 ], ovrMatrix.M[ 2 ][ 2 ], ovrMatrix.M[ 3 ][ 2 ],
		ovrMatrix.M[ 0 ][ 3 ], ovrMatrix.M[ 1 ][ 3 ], ovrMatrix.M[ 2 ][ 3 ], ovrMatrix.M[ 3 ][ 3 ]
	};
	
	Matrix44 engineMatrix = Matrix44( valuesTransposed );
	return engineMatrix;
}

OVR::Matrix4f GetOVRMatrixFromEngineMatrix( const Matrix44& engineMatrix )
{
	Vector3 iBasis = engineMatrix.GetIBasis();
	Vector3 jBasis = engineMatrix.GetJBasis();
	Vector3 kBasis = engineMatrix.GetKBasis();
	Vector3 translation = engineMatrix.GetTranslation();

	OVR::Matrix4f matrix;

	matrix.SetXBasis( OVR::Vector3< float >( iBasis.x, iBasis.y, iBasis.z ) );
	matrix.SetYBasis( OVR::Vector3< float >( jBasis.x, jBasis.y, jBasis.z ) );
	matrix.SetZBasis( OVR::Vector3< float >( kBasis.x, kBasis.y, kBasis.z ) );
	matrix.SetTranslation( OVR::Vector3< float >( translation.x, translation.y, translation.z ) );

	return matrix;
}

ovrControllerType GetOVRControllerTypeForEngineControllerType( OVRControllerType controllerType )
{
	switch( controllerType )
	{
		case OVRControllerType::OVR_TOUCH_CONTROLLER_LEFT	:	return ovrControllerType_LTouch;
		case OVRControllerType::OVR_TOUCH_CONTROLLER_RIGHT	:	return ovrControllerType_RTouch;
	}
	return ovrControllerType_LTouch;
}
