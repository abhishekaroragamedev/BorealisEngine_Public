#pragma once

#include "ThirdParty/OculusSDK/Include/Extras/OVR_CAPI_Util.h"
#include "ThirdParty/OculusSDK/Include/Extras/OVR_Math.h"

enum OVRControllerType;
class Matrix44;
class Vector2;
class Vector3;

Vector2 GetEngineVector2FromOVRVector2( const ovrVector2f& ovrVec2 );
ovrVector2f GetOVRVector2FromEngineVector2( const Vector2& engineVec2 );

Vector3 GetEngineVector3FromOVRVector3( const ovrVector3f& ovrVec3 );		// Inverts Z
ovrVector3f GetOVRVector3FromEngineVector3( const Vector3& engineVec3 );	// Inverts Z

Vector3 GetEngineEulerFromOVRQuaternion( ovrQuatf ovrQuaternion );
ovrQuatf GetOVRQuaternionFromEngineEuler( const Vector3& engineEuler );

Matrix44 GetEngineMatrixFromOVRMatrix( ovrFovPort fovPort, float nearZ, float farZ, ovrProjectionModifier_ modifier );
OVR::Matrix4f GetOVRMatrixFromEngineMatrix( const Matrix44& engineMatrix );

ovrControllerType GetOVRControllerTypeForEngineControllerType( OVRControllerType controllerType );
