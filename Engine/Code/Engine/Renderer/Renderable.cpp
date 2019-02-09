#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Vector4.hpp"
#include "Engine/Renderer/InputLayout.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/Mesh.hpp"
#include "Engine/Renderer/Renderable.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Renderer/ShaderPass.hpp"

Renderable::Renderable()
{
	m_layout = new InputLayout;
}

Renderable::Renderable( Mesh* mesh, Material* material, bool ownsMesh /* = true */, bool ownsMaterial /* = true */, Matrix44 model /* = Matrix44::IDENTITY */ )
	:	m_mesh( mesh ),
		m_material( material ),
		m_ownsMesh( ownsMesh ),
		m_ownsMaterial( ownsMaterial ),
		m_modelMatrix( model )
{
	m_layout = new InputLayout( mesh, material );
}

Renderable::~Renderable()
{
	delete m_layout;
	m_layout = nullptr;

	if ( m_ownsMesh )
	{
		delete m_mesh;
	}
	m_mesh = nullptr;

	if ( m_ownsMaterial )
	{
		delete m_material;
	}
	m_material = nullptr;
}

void Renderable::SetModelMatrix( const Matrix44& modelMatrix )
{
	m_modelMatrix = modelMatrix;
}

void Renderable::SetMesh( Mesh* mesh, bool ownsMesh /* = true */ )
{
	m_mesh = mesh;
	m_ownsMesh = ownsMesh;

	m_layout->SetMesh( mesh );
}

void Renderable::ReplaceMesh( Mesh* mesh, bool ownsMesh /* = true */ )
{
	if ( m_mesh != nullptr && m_ownsMesh )
	{
		delete m_mesh;
	}
	SetMesh( mesh, ownsMesh );
}

void Renderable::SetMaterial( Material* material, bool ownsMaterial /* = true */ )
{
	m_material = material;
	m_ownsMaterial = ownsMaterial;

	m_layout->SetPrograms( material->GetShader()->GetPrograms() );
}

void Renderable::ReplaceMaterial( Material* material, bool ownsMaterial /* = true */ )
{
	if ( m_material != nullptr && m_ownsMaterial )
	{
		delete m_material;
	}
	SetMaterial( material, ownsMaterial );
}

void Renderable::SetMeshOwnership( bool ownership )
{
	m_ownsMesh = ownership;
}

void Renderable::SetMaterialOwnership( bool ownership )
{
	m_ownsMaterial = ownership;
}

void Renderable::SetCastsShadows( bool castsShadows )
{
	m_castsShadows = castsShadows;
}

bool Renderable::ToggleVisibility()
{
	m_isVisible = !m_isVisible;
	return m_isVisible;
}

Matrix44 Renderable::GetModelMatrix() const
{
	return m_modelMatrix;
}

Mesh* Renderable::GetMesh() const
{
	return m_mesh;
}

Material* Renderable::GetMaterial() const
{
	return m_material;
}

InputLayout* Renderable::GetInputLayout() const
{
	return m_layout;
}

bool Renderable::OwnsMesh() const
{
	return m_ownsMesh;
}

bool Renderable::OwnsMaterial() const
{
	return m_ownsMaterial;
}

bool Renderable::IsVisible() const
{
	return m_isVisible;
}

bool Renderable::CastsShadows() const
{
	return m_castsShadows;
}
