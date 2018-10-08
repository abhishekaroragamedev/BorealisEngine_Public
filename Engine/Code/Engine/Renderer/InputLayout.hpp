#pragma once

#include <vector>

class Material;
class Mesh;
class Renderable;
class ShaderPass;
class ShaderProgram;

class InputLayout
{

public:
	InputLayout();
	explicit InputLayout( const Mesh* mesh, std::vector< const ShaderProgram* > programs );
	explicit InputLayout( const Mesh* mesh, const Material* material );
	explicit InputLayout( const Renderable& renderable );
	~InputLayout();

	void UpdateBinding();

	void SetMesh( const Mesh* mesh );
	void SetProgram( unsigned int index, const ShaderProgram* program );
	void SetPrograms( const std::vector< const ShaderProgram * >& programs );

	const Mesh* GetMesh() const;
	const ShaderProgram* GetProgram( unsigned int index ) const;
	std::vector< const ShaderProgram* > GetPrograms() const;
	unsigned int GetID() const;

private:
	const Mesh* m_mesh = nullptr;
	std::vector< const ShaderProgram* > m_programs;
	unsigned int m_ID = UINT_MAX;

};