#include "Engine/Math/Transform.hpp"

Transform::Transform()
{
	MarkDirty();
}

Transform::Transform( Vector3 position, Vector3 eulerAngles, Vector3 scale )
	:	m_position( position ),
		m_eulerAngles( eulerAngles ),
		m_scale( scale )
{
	MarkDirty();
}

Transform::Transform( Transform* parent )
	:	m_parent( parent )
{
	MarkDirty();
}

Transform::Transform( Transform* parent, Vector3 position, Vector3 eulerAngles, Vector3 scale )
	:	m_parent( parent ),
		m_position( position ),
		m_eulerAngles( eulerAngles ),
		m_scale( scale )
{
	MarkDirty();
}

Transform::Transform( const Transform& copy )
	:	m_position( copy.m_position ),
		m_eulerAngles( copy.m_eulerAngles ),
		m_scale( copy.m_scale ),
		m_parent( copy.m_parent ),
		m_children( copy.m_children )
{
	MarkDirty();
}

Transform::~Transform()
{

}

void Transform::operator=( const Transform& assignedTransform )
{
	m_position = assignedTransform.m_position;
	m_eulerAngles = assignedTransform.m_eulerAngles;
	m_scale = assignedTransform.m_scale;
	m_parent = assignedTransform.m_parent;
	m_children = assignedTransform.m_children;
	MarkDirty();
}

void Transform::Translate( const Vector3& translation )
{
	m_position += translation;
	MarkDirty();
}

void Transform::Rotate( const Vector3& rotationEulerAngles )
{
	m_eulerAngles += rotationEulerAngles;
	MarkDirty();
}

void Transform::Scale( const Vector3& scale )
{
	m_scale *= scale;
	MarkDirty();
}

void Transform::SetLocalFromMatrix( const Matrix44& matrix )
{
	Matrix44 matrixCopy = Matrix44( matrix );
	
	m_position = matrixCopy.GetTranslation();
	m_scale = Vector3( matrixCopy.GetIBasis().GetLength(), matrixCopy.GetJBasis().GetLength(), matrixCopy.GetKBasis().GetLength() );
	
	matrixCopy.NormalizeBases();
	m_eulerAngles = matrixCopy.GetEulerAngles();
	MarkDirty();
}

void Transform::SetLocalPosition( const Vector3& position )
{
	m_position = position;
	MarkDirty();
}

void Transform::SetLocalEuler( const Vector3& euler )
{
	m_eulerAngles = euler;
	MarkDirty();
}

void Transform::SetLocalScale( const Vector3& scale )
{
	m_scale = scale;
	MarkDirty();
}

void Transform::AddChild( Transform* childTransform )
{
	m_children.push_back( childTransform );
	childTransform->MarkDirty();
}

bool Transform::RemoveChild( Transform* childTransform )
{
	for ( size_t childIndex = 0; childIndex < m_children.size(); childIndex++ )
	{
		if ( m_children[ childIndex ] == childTransform )
		{
			m_children.erase( m_children.begin() + childIndex );
			childTransform->MarkDirty();
			return true;
		}
	}
	return false;
}

void Transform::Reparent( Transform* newParent )
{
	if ( m_parent != nullptr )
	{
		m_parent->RemoveChild( this );
	}
	m_parent = newParent;
	if ( m_parent != nullptr )
	{
		m_parent->AddChild( this );
	}
	MarkDirty();
}

void Transform::TransformToWorld()
{
	Transform* parent = m_parent;
	Matrix44 childMatrix = GetAsMatrixLocal();
	while ( parent != nullptr )
	{
		childMatrix = parent->GetAsMatrixLocal() * childMatrix;
		parent = parent->m_parent;
	}
	SetLocalFromMatrix( childMatrix );
	m_parent = nullptr;
	MarkDirty();
	m_children.clear();
}

void Transform::MarkDirty()
{
	m_isDirty = true;
	MarkChildrenAsDirty();
}

void Transform::MarkChildrenAsDirty()
{
	for ( Transform* child : m_children )
	{
		child->MarkDirty();
	}
}

Vector3 Transform::GetLocalPosition() const
{
	return m_position;
}

Vector3 Transform::GetLocalEuler() const
{
	return m_eulerAngles;
}

Vector3 Transform::GetLocalScale() const
{
	return m_scale;
}

Matrix44 Transform::GetAsMatrixLocal() const
{
	Matrix44 transformMatrix = Matrix44::MakeTranslation( m_position );
	transformMatrix.Append( Matrix44::MakeFromEuler( m_eulerAngles ) );
	transformMatrix.Append( Matrix44::MakeScale( m_scale ) );
	return transformMatrix;
}

Vector3 Transform::GetWorldPosition()
{
	return GetAsMatrixWorld().GetTranslation();
}

Vector3 Transform::GetWorldEuler()
{
	return GetAsMatrixWorld().GetEulerAngles();
}

Vector3 Transform::GetWorldScale()
{
	return GetWorldTransform().GetLocalScale();
}

Matrix44 Transform::GetAsMatrixWorld()
{
	if ( !IsDirty() )
	{
		return m_cachedWorldMatrix;
	}
	m_cachedWorldMatrix = GetWorldTransform().GetAsMatrixLocal();
	m_isDirty = false;
	return m_cachedWorldMatrix;
}

Transform Transform::GetWorldTransform()
{
	Transform worldTransform = Transform( *this );
	worldTransform.TransformToWorld();
	m_cachedWorldMatrix = worldTransform.GetAsMatrixLocal();
	m_isDirty = false;
	return worldTransform;
}

Matrix44 Transform::GetParentTransformsAsMatrixWorld() const
{
	Transform* parent = m_parent;
	Matrix44 childMatrix = Matrix44::IDENTITY;
	while ( parent != nullptr )
	{
		childMatrix = parent->GetAsMatrixLocal() * childMatrix;
		parent = parent->m_parent;
	}
	return childMatrix;
}

Transform* Transform::GetParent() const
{
	return m_parent;
}

bool Transform::IsChild() const
{
	return ( m_parent != nullptr );
}

bool Transform::IsDirty() const
{
	return m_isDirty;
}

/* static */
Transform Transform::FromMatrix( const Matrix44& matrix )
{
	Transform transform;
	transform.SetLocalFromMatrix( matrix );
	return transform;
}
