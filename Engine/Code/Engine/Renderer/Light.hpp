#pragma once

#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/Matrix44.hpp"
#include "Engine/Math/Transform.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Vector4.hpp"

class Camera;

constexpr unsigned int MAX_LIGHTS = 8;
constexpr unsigned int SHADOW_MAP_DEPTH_TARGET_HEIGHT = 500U;
constexpr float SHADOW_MAP_MAX_FAR_Z = 500.0f;

enum LightType	:	int
{
	LIGHT_TYPE_INVALID = -1,
	LIGHT_TYPE_POINT,
	LIGHT_TYPE_DIRECTIONAL,
	LIGHT_TYPE_SPOT
};

struct LightUBO		/* std140 */
{

	void operator=( const LightUBO& assignedLightUBO );

public:
	Vector3 m_position = Vector3::ZERO;									// Byte 0-11
	int m_castsShadows = 0;												// Byte 12-15
	Vector4 m_colorAndIntensity = Vector4( 1.0f, 1.0f, 1.0f, 0.0f );	// Byte 16-31
	Vector3 m_attenuation = Vector3::FORWARD;							// Byte 32-43
	float m_padding0 = 0.0f;											// Byte 44-47
	Vector3 m_direction = Vector3::RIGHT;								// Byte 48-59
	float m_padding1 = 0.0f;											// Byte 60-63
	float m_directionFactor = 0.0f;										// Byte 64-67
	float m_coneFactor = 0.0f;											// Byte 68-71
	float m_innerAngle = 0.0f;											// Byte 72-75
	float m_outerAngle = 0.0f;											// Byte 76-79
	Matrix44 m_shadowViewMatrix;										// Byte 80-143
	Matrix44 m_shadowProjectionMatrix;									// Byte 144-207

};

struct ActiveLightBufferUBO		/* std140 */
{

public:
	Vector4 m_ambientLightColorAndIntensity = Vector4( 1.0f, 1.0f, 1.0f, 0.0f );	// Byte 0-15
	LightUBO m_lights[ MAX_LIGHTS ];												// Byte 16-1679

};

struct ObjectLightPropertiesUBO		/* std140 */
{

public:
	ObjectLightPropertiesUBO();
	explicit ObjectLightPropertiesUBO( float specularAmount, float specularPower );
	explicit ObjectLightPropertiesUBO( const ObjectLightPropertiesUBO& copy );

public:
	float m_specularAmount = 1.0f;		// Byte 0-3
	float m_specularPower = 1.0f;		// Byte 4-7
	Vector2 m_padding = Vector2::ZERO;	// Byte 8-15		

};

class Light
{

public:
	Light();
	Light( const Light& copy );
	~Light();

public:
	void SetPointLight( const Vector3& position, const Vector4& colorAndIntensity, const Vector3& attenuation = Vector3::FORWARD );	// Point lights, by default, have a d^2 fall-off
	void SetDirectionalLight( const Vector3& position, const Vector3& direction, const Vector4& colorAndIntensity, const Vector3& attenuation = Vector3::UP );	// Directional lights, by default, have a d fall-off
	void SetSpotLight( const Vector3& position, const Vector3& direction, const Vector4& colorAndIntensity, float innerAngle, float outerAngle, const Vector3& attenuation = Vector3::FORWARD  );	// Spot lights, by default, have a d^2 fall-off
	void UpdateUBOFromTransform();

	bool IsPointLight();
	bool IsDirectionalLight();
	bool IsSpotLight();
	float GetIntensityAtPosition( const Vector3& position );
	float GetDistanceForZeroIntensity();
	float GetFarZForShadowMap();
	Camera* MakePerspectiveCameraForShadowMapAndSetLightMatrices();	// Caller is responsible for destroying the Camera
	Camera* MakeOrthoCameraForShadowMapAndSetLightMatrices( const Vector3& lightPosition, float orthoSize, float aspect, float nearZ, float farZ );

public:
	LightUBO m_lightUBO;
	Transform m_transform;

};

void CopyLights( const LightUBO source[ MAX_LIGHTS ], LightUBO out_destination[ MAX_LIGHTS ], unsigned int count );
