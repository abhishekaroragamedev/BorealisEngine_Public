#pragma once

#include "Engine/Math/Matrix44.hpp"
#include "Engine/Math/Vector3.hpp"
#include <vector>

class Transform
{

public:
	Transform();
	Transform( const Transform& copy );
	explicit Transform( Transform* parent, Vector3 position, Vector3 eulerAngles, Vector3 scale );
	explicit Transform( Transform* parent );
	explicit Transform( Vector3 position, Vector3 eulerAngles, Vector3 scale );
	~Transform();

	void operator=( const Transform& assignedTransform );

	void Translate( const Vector3& translation );
	void Rotate( const Vector3& rotationEulerAngles );
	void Scale( const Vector3& scale );
	void SetLocalFromMatrix( const Matrix44& matrix );
	void SetLocalPosition( const Vector3& position );
	void SetLocalEuler( const Vector3& euler );
	void SetLocalScale( const Vector3& scale );
	void AddChild( Transform* childTransform );
	bool RemoveChild( Transform* childTransform );
	void Reparent( Transform* newParent );
	void TransformToWorld();
	void MarkDirty();
	void MarkChildrenAsDirty();

	Vector3 GetLocalPosition() const;
	Vector3 GetLocalEuler() const;
	Vector3 GetLocalScale() const;
	Matrix44 GetAsMatrixLocal() const;

	Vector3 GetWorldPosition();
	Vector3 GetWorldEuler();
	Vector3 GetWorldScale();
	Matrix44 GetAsMatrixWorld();
	Transform GetWorldTransform();
	Matrix44 GetParentTransformsAsMatrixWorld() const;
	Transform* GetParent() const;

	bool IsChild() const;
	bool IsDirty() const;

public:
	static Transform FromMatrix( const Matrix44& matrix );

public:
	Vector3 m_position = Vector3::ZERO;
	Vector3 m_eulerAngles = Vector3::ZERO;	// Measured in Degrees
	Vector3 m_scale = Vector3::ONE;
	
private:
	Transform* m_parent = nullptr;
	std::vector< Transform* > m_children;

	Matrix44 m_cachedWorldMatrix = Matrix44::IDENTITY;
	bool m_isDirty = false;

};