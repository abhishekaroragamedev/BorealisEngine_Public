#include "Engine/Core/Window.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/Light.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Tools/DevConsole.hpp"

void LightUBO::operator=( const LightUBO& assignedLightUBO )
{
	m_position = assignedLightUBO.m_position;
	m_castsShadows = assignedLightUBO.m_castsShadows;
	m_colorAndIntensity = assignedLightUBO.m_colorAndIntensity;
	m_attenuation = assignedLightUBO.m_attenuation;
	m_direction = assignedLightUBO.m_direction;
	m_directionFactor = assignedLightUBO.m_directionFactor;
	m_coneFactor = assignedLightUBO.m_coneFactor;
	m_innerAngle = assignedLightUBO.m_innerAngle;
	m_outerAngle = assignedLightUBO.m_outerAngle;
	m_shadowViewMatrix = assignedLightUBO.m_shadowViewMatrix;
	m_shadowProjectionMatrix = assignedLightUBO.m_shadowProjectionMatrix;
}

ObjectLightPropertiesUBO::ObjectLightPropertiesUBO()
{

}

ObjectLightPropertiesUBO::ObjectLightPropertiesUBO( float specularAmount, float specularPower )
	:	m_specularAmount( specularAmount ),
		m_specularPower( specularPower )
{

}

ObjectLightPropertiesUBO::ObjectLightPropertiesUBO( const ObjectLightPropertiesUBO& copy )
	:	m_specularAmount( copy.m_specularAmount ),
		m_specularPower( copy.m_specularPower )
{

}

Light::Light()
{

}

Light::Light( const Light& copy )
{
	m_transform = copy.m_transform;
	m_lightUBO = copy.m_lightUBO;
}

Light::~Light()
{

}

void Light::SetPointLight( const Vector3& position, const Vector4& colorAndIntensity, const Vector3& attenuation /* = Vector3::FORWARD */ )
{
	m_transform.m_position = position;

	m_lightUBO.m_position = position;
	m_lightUBO.m_colorAndIntensity = colorAndIntensity;
	m_lightUBO.m_attenuation = attenuation;
	m_lightUBO.m_coneFactor = 0.0f;
	m_lightUBO.m_directionFactor = 0.0f;
	m_lightUBO.m_castsShadows = 0;
}

void Light::SetDirectionalLight( const Vector3& position, const Vector3& direction, const Vector4& colorAndIntensity, const Vector3& attenuation /* = Vector3::UP */ )
{
	Matrix44 lookAt = Matrix44::LookAt( position, ( position + direction ) );
	m_transform.SetLocalFromMatrix( lookAt );
	m_transform.m_position = position;

	m_lightUBO.m_position = position;
	m_lightUBO.m_direction = direction;
	m_lightUBO.m_colorAndIntensity = colorAndIntensity;
	m_lightUBO.m_attenuation = attenuation;
	m_lightUBO.m_coneFactor = 0.0f;
	m_lightUBO.m_directionFactor = 1.0f;
	m_lightUBO.m_castsShadows = 0;
}

void Light::SetSpotLight( const Vector3& position, const Vector3& direction, const Vector4& colorAndIntensity, float innerAngle, float outerAngle, const Vector3& attenuation /* = Vector3::FORWARD */ )
{
	Matrix44 lookAt = Matrix44::LookAt( position, ( position + direction ) );
	m_transform.SetLocalFromMatrix( lookAt );
	m_transform.m_position = position;

	m_lightUBO.m_position = position;
	m_lightUBO.m_direction = direction;
	m_lightUBO.m_colorAndIntensity = colorAndIntensity;
	m_lightUBO.m_innerAngle = innerAngle;
	m_lightUBO.m_outerAngle = outerAngle;
	m_lightUBO.m_attenuation = attenuation;
	m_lightUBO.m_coneFactor = 1.0f;
	m_lightUBO.m_directionFactor = 0.0f;
	m_lightUBO.m_castsShadows = 0;
}

void Light::UpdateUBOFromTransform()
{
	Matrix44 worldMatrix = m_transform.GetAsMatrixWorld();
	m_lightUBO.m_position = worldMatrix.GetTranslation();
	m_lightUBO.m_direction = worldMatrix.GetKBasis();
}

bool Light::IsPointLight()
{
	return ( IsFloatEqualTo( m_lightUBO.m_directionFactor, 0.0f ) && IsFloatEqualTo( m_lightUBO.m_coneFactor, 0.0f ) );
}

bool Light::IsDirectionalLight()
{
	return ( IsFloatEqualTo( m_lightUBO.m_directionFactor, 1.0f ) && IsFloatEqualTo( m_lightUBO.m_coneFactor, 0.0f ) );
}

bool Light::IsSpotLight()
{
	return ( IsFloatEqualTo( m_lightUBO.m_directionFactor, 0.0f ) && IsFloatEqualTo( m_lightUBO.m_coneFactor, 1.0f ) );
}

float Light::GetIntensityAtPosition( const Vector3& position )
{
	float intensity = m_lightUBO.m_colorAndIntensity.w;

	if ( m_lightUBO.m_attenuation != Vector3::ZERO )
	{
		float distance = ( m_lightUBO.m_position - position ).GetLength();
		intensity /= ( m_lightUBO.m_attenuation.x + ( m_lightUBO.m_attenuation.y * distance ) + ( m_lightUBO.m_attenuation.z * ( distance * distance ) ) );
	}

	return intensity;
}

float Light::GetDistanceForZeroIntensity()
{
	if ( IsFloatEqualTo( m_lightUBO.m_colorAndIntensity.w, 0.0f ) )
	{
		return 0.0f;
	}
	else if ( m_lightUBO.m_attenuation == Vector3::ZERO )
	{
		return FLT_MAX;
	}
	else
	{
		// intensity / (a0 + a1d + a2d^2) ~= 0 (or = epsilon)
		Vector2 distanceSolutions;
		if( SolveQuadratic( distanceSolutions, m_lightUBO.m_attenuation.z, m_lightUBO.m_attenuation.y, m_lightUBO.m_attenuation.x - ( m_lightUBO.m_colorAndIntensity.w / 0.0001f ) ) )
		{
			return distanceSolutions.y; // Greater solution
		}
		else
		{
			return FLT_MAX;
		}
	}
}

float Light::GetFarZForShadowMap()
{
	return Min( GetDistanceForZeroIntensity(), SHADOW_MAP_MAX_FAR_Z );
}

Camera* Light::MakePerspectiveCameraForShadowMapAndSetLightMatrices() // TODO: Parameter for shadow casting scene area for directional lights
{
	if ( !IsSpotLight() || !m_lightUBO.m_castsShadows )
	{
		return nullptr;
	}
	Camera* shadowMapCamera = new Camera();	// Scene manages the Depth Target
	shadowMapCamera->LookAt( m_lightUBO.m_position, ( m_lightUBO.m_position + m_lightUBO.m_direction ) );
	m_lightUBO.m_shadowViewMatrix = shadowMapCamera->GetViewMatrix();
	m_lightUBO.m_shadowProjectionMatrix = Matrix44::MakePerspective( m_lightUBO.m_outerAngle, Window::GetAspect(), 0.1f, GetFarZForShadowMap() );
	shadowMapCamera->SetProjection( m_lightUBO.m_shadowProjectionMatrix );
	return shadowMapCamera;
}

Camera* Light::MakeOrthoCameraForShadowMapAndSetLightMatrices( const Vector3& lightPosition, float orthoSize, float aspect, float nearZ, float farZ )
{
	if ( !IsDirectionalLight() || !m_lightUBO.m_castsShadows )
	{
		return nullptr;
	}
	Camera* shadowMapCamera = new Camera;	// Scene manages the Depth Target
	shadowMapCamera->LookAt( lightPosition, ( lightPosition + m_lightUBO.m_direction ) );
	m_lightUBO.m_shadowViewMatrix = shadowMapCamera->GetViewMatrix();
	shadowMapCamera->SetProjectionOrtho( orthoSize, aspect, nearZ, farZ );
	m_lightUBO.m_shadowProjectionMatrix = shadowMapCamera->GetProjectionMatrix();
	return shadowMapCamera;
}

void CopyLights( const LightUBO source[ MAX_LIGHTS ], LightUBO out_destination[ MAX_LIGHTS ], unsigned int count )
{
	if ( count > MAX_LIGHTS )
	{
		ConsolePrintf( Rgba::RED, "ERROR: CopyLights() - More than MAX_LIGHTS lights cannot be copied. Skipping..." );
	}
	else
	{
		memcpy( out_destination, source, ( sizeof( LightUBO ) * static_cast< size_t >( count ) ) );
	}
}
