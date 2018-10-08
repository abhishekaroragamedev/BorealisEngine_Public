#include "Engine/Renderer/GLFunctionBinding.hpp"
#include "Engine/Renderer/InputLayout.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/Mesh.hpp"
#include "Engine/Renderer/Renderable.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Renderer/ShaderPass.hpp"
#include "Engine/Renderer/ShaderProgram.hpp"

InputLayout::InputLayout()
{

}

InputLayout::InputLayout( const Mesh* mesh, std::vector<const ShaderProgram *> programs )
	:	m_mesh( mesh ),
		m_programs( programs )
{

}

InputLayout::InputLayout( const Mesh* mesh, const Material* material )
	:	m_mesh( mesh ),
		m_programs( material->GetShader()->GetPrograms() )
{
	
}

InputLayout::InputLayout( const Renderable& renderable )
	:	m_mesh( renderable.GetMesh() ),
		m_programs( renderable.GetMaterial()->GetShader()->GetPrograms() )
{
	
}

InputLayout::~InputLayout()
{
	glBindVertexArray( 0 );
	glDeleteVertexArrays( 1, &m_ID );
}

void InputLayout::UpdateBinding()
{

	for ( const ShaderProgram* program : m_programs )
	{
		if ( m_ID == UINT_MAX )
		{
			glGenVertexArrays( 1, &m_ID );
		}
		glBindVertexArray( m_ID );

		m_mesh->Finalize();	// Bind the vertex and index buffers

		unsigned int vertexStride = m_mesh->GetVertexLayout()->m_stride;
		unsigned int attributeCount = m_mesh->GetVertexLayout()->GetAttributeCount();

		GLuint programHandle = program->GetHandle();

		for ( unsigned int attributeIndex = 0; attributeIndex < attributeCount; attributeIndex++ )
		{
			VertexAttribute attribute = m_mesh->GetVertexLayout()->GetAttribute( attributeIndex );

			int bindPoint = glGetAttribLocation( programHandle, attribute.m_name.c_str() );

			if ( bindPoint >= 0 )
			{
				glEnableVertexAttribArray( bindPoint );
				glVertexAttribPointer( bindPoint, attribute.m_elementCount, Renderer::GetInstance()->GetGLDataType( attribute.m_type ), attribute.m_normalized, vertexStride, reinterpret_cast< GLvoid* >( attribute.m_memberOffset ) );
			}
		}	
	}
}

void InputLayout::SetMesh( const Mesh* mesh )
{
	m_mesh = mesh;
}

void InputLayout::SetProgram( unsigned int index, const ShaderProgram* program )
{
	m_programs[ index ] = program;
}

void InputLayout::SetPrograms( const std::vector< const ShaderProgram * >& programs )
{
	m_programs.clear();
	m_programs = programs;
}

const Mesh* InputLayout::GetMesh() const
{
	return m_mesh;
}

const ShaderProgram* InputLayout::GetProgram( unsigned int index ) const
{
	return m_programs[ index ];
}

std::vector< const ShaderProgram* > InputLayout::GetPrograms() const
{
	return m_programs;
}

unsigned int InputLayout::GetID() const
{
	return m_ID;
}
