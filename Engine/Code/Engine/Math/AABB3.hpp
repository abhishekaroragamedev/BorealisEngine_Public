#pragma once

#include "Engine/Math/Vector3.hpp"

class AABB3
{

public:
	AABB3() {}
	~AABB3() {}
	AABB3( const AABB3& copy );

	explicit AABB3( float minX, float minY, float minZ, float maxX, float maxY, float maxZ );
	explicit AABB3( const Vector3& mins, const Vector3& maxs );
	explicit AABB3( const Vector3& center, float radiusX, float radiusY, float radiusZ );

public:
	void StretchToIncludePoint( float x, float y, float z );
	void StretchToIncludePoint( const Vector3& point );
	void AddPaddingToSides( float xPaddingRadius, float yPaddingRadius, float zPaddingRadius );
	void Translate( const Vector3& translation );
	void Translate( float translationX, float translationY, float translationZ );
	void SetFromText( const char* text );

public:
	bool IsPointInside( float x, float y, float z ) const;
	bool IsPointInside( const Vector3& point ) const;
	bool IsValid() const;
	Vector3 GetDimensions() const;
	Vector3 GetCenter() const;
	Vector3 GetMinXMaxYMaxZ() const;
	Vector3 GetMaxXMinYMaxZ() const;
	Vector3 GetMaxXMaxYMinZ() const;
	Vector3 GetMinXMaxYMinZ() const;
	Vector3 GetMaxXMinYMinZ() const;
	Vector3 GetMinXMinYMaxZ() const;

public:
	void operator=( const AABB3& assigned );
	void operator+=(const Vector3& translation);
	void operator-=( const Vector3& antiTranslation );
	void operator*=( const Vector3& upScaling );
	void operator/=( const Vector3& downScaling );
	AABB3 operator+( const Vector3& translation ) const;
	AABB3 operator-( const Vector3& antiTranslation ) const;

public:
	static const AABB3 ZERO;
	static const AABB3 ONE;
	static const AABB3 ZERO_TO_ONE;
	static const AABB3 MINUS_ONE_TO_ONE;

public:
	Vector3 mins = Vector3::ZERO;
	Vector3 maxs = Vector3::ZERO;

};

/* STANDALONE FUNCTIONS */

bool DoAABBsOverlap( const AABB3& a, const AABB3& b );

const AABB3 Interpolate( const AABB3& start, const AABB3& end, float fractionTowardEnd );
