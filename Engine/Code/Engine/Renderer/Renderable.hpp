#pragma once

#include "Engine/Math/Matrix44.hpp"

class InputLayout;
class Material;
class Mesh;

class Renderable
{

	friend class Renderer;

public:
	Renderable();
	explicit Renderable( Mesh* mesh, Material* material, bool ownsMesh = true, bool ownsMaterial = true, Matrix44 model = Matrix44::IDENTITY );
	~Renderable();

	void SetModelMatrix( const Matrix44& modelMatrix );
	void SetMesh( Mesh* mesh, bool ownsMesh = true );
	void ReplaceMesh( Mesh* mesh, bool ownsMesh = true );
	void SetMaterial( Material* material, bool ownsMaterial = true );
	void ReplaceMaterial( Material* material, bool ownsMaterial = true );
	void SetMeshOwnership( bool ownership );
	void SetMaterialOwnership( bool ownership );
	void SetCastsShadows( bool castsShadows );
	bool ToggleVisibility();

	Matrix44 GetModelMatrix() const;
	Mesh* GetMesh() const;
	Material* GetMaterial() const;
	InputLayout* GetInputLayout() const;
	bool OwnsMesh() const;
	bool OwnsMaterial() const;
	bool IsVisible() const;
	bool CastsShadows() const;

private:
	Matrix44 m_modelMatrix = Matrix44::IDENTITY;
	Mesh* m_mesh = nullptr;
	Material* m_material = nullptr;
	InputLayout* m_layout;
	bool m_ownsMesh = true;
	bool m_ownsMaterial = true;
	bool m_isVisible = true;
	bool m_castsShadows = true;

};
