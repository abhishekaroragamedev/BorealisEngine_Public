#pragma once

#include "ThirdParty/tinyXML2/tinyxml2.h"
#include <string>
#include <vector>

class ShaderPass;
class ShaderProgram;

class Shader
{
	
public:
	Shader( const tinyxml2::XMLElement& shaderElement );
	Shader( ShaderPass* firstPass, const std::string& name = "" );
	~Shader();

	ShaderPass* GetPass( unsigned int index ) const;
	unsigned int GetPassCount() const;
	std::vector< const ShaderProgram* > GetPrograms() const;
	std::string GetName() const;

public:
	static Shader* AcquireResource( const std::string& shaderPassFilePath );

private:
	std::vector< ShaderPass* > m_passes;
	std::string m_name = "";


};
