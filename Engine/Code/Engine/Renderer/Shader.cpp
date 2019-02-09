#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/XmlUtilities.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Renderer/ShaderPass.hpp"
#include "Engine/Renderer/ShaderProgram.hpp"

Shader::Shader( const tinyxml2::XMLElement& shaderElement )
{
	m_name = ParseXmlAttribute( shaderElement, "name", m_name );
	for ( const tinyxml2::XMLElement* childElement = shaderElement.FirstChildElement(); childElement != nullptr; childElement = childElement->NextSiblingElement() )
	{
		m_passes.push_back( new ShaderPass( *childElement ) );
	}
}

Shader::Shader( ShaderPass* firstPass, const std::string& name /* = "" */ )
{
	m_name = name;
	m_passes.push_back( firstPass );
}

Shader::~Shader()
{
	for ( ShaderPass* pass : m_passes )
	{
		delete pass;
		pass = nullptr;
	}
}

void Shader::LoadFromXML( const tinyxml2::XMLElement& shaderElement )
{
	m_name = ParseXmlAttribute( shaderElement, "name", m_name );

	size_t passCount = 0;
	for ( const tinyxml2::XMLElement* childElement = shaderElement.FirstChildElement(); childElement != nullptr; childElement = childElement->NextSiblingElement() )
	{
		m_passes[ passCount ]->LoadFromXML( *childElement );
		passCount++;
	}

	if ( passCount != m_passes.size() )
	{
		ERROR_RECOVERABLE( "WARNING: Shader::LoadFromXML(): Number of passes in new XML not same as during initialization. This is not supported. Please reload the application for deterministic behavior." );
	}
}

ShaderPass* Shader::GetPass( unsigned int index ) const
{
	return m_passes[ index ];
}

unsigned int Shader::GetPassCount() const
{
	return static_cast< unsigned int >( m_passes.size() );
}

std::string Shader::GetName() const
{
	return m_name;
}

std::vector< const ShaderProgram* > Shader::GetPrograms() const
{
	std::vector< const ShaderProgram* > programs;
	for ( ShaderPass* pass : m_passes )
	{
		programs.push_back( pass->GetProgram() );
	}
	return programs;
}

/* static */
Shader* Shader::AcquireResource( const std::string& shaderPassFilePath )
{
	return new Shader( ShaderPass::AcquireResource( shaderPassFilePath ), shaderPassFilePath );
}
