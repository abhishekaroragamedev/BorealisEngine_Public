#pragma once

#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Renderer/Camera.hpp"

class OrbitCamera : public Camera
{

public:
	OrbitCamera( float minRadius, float maxRadius, float minAzimuth, float maxAzimuth, Texture* colorTarget = nullptr, Texture* depthTarget = nullptr );
	OrbitCamera( const FloatRange& radiusRange, const FloatRange& azimuthRange, Texture* colorTarget = nullptr, Texture* depthTarget = nullptr );
	~OrbitCamera();

	Vector3 GetTarget() const;
	Vector3 GetPolarCoordinates() const;
	Vector3 GetCartesianRelativeCoordinates() const;
	Vector3 GetCartesianAbsoluteCoordinates() const;

	void SetTarget( const Vector3& target );
	void SetSphericalCoordinates( float radius, float rotation, float azimuth );
	void AddToRadius( float amount );
	void AddToRotation( float amount );
	void AddToAzimuth( float amount );
	void SnapToNextFixedAngle();
	void SnapToPreviousFixedAngle();

private:
	void RecomputeMVPFromPolarCoordinates();
	void RecomputeOrthoFromRadiusChange();

private:
	const float FIXED_ANGLES[ 4 ] = { 45.0, 135.0, 225.0, 315.0 };

private:
	Vector3 m_target = Vector3::ZERO;
	float m_radius = 0.0f;
	float m_rotation = 30.0f;
	float m_azimuth = 0.0f;
	FloatRange m_radiusRange;
	FloatRange m_azimuthRange;

};
