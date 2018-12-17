#pragma once

#include "Engine/Core/XmlUtilities.hpp"
#include "Engine/Renderer/External/gl/glcorearb.h"
#include "Engine/Renderer/External/gl/glext.h"
#include "ThirdParty/tinyXML2/tinyxml2.h"
#include <string>

constexpr int UNIFORM_NAME_MAX_LENGTH = 64;

struct PropertyBlockInfo;
class ShaderProgramInfo;

class ShaderProgram
{
public:
	ShaderProgram();
	ShaderProgram( const tinyxml2::XMLElement& shaderProgramElement );
	ShaderProgram( const ShaderProgram& copy );
	~ShaderProgram();

	void LoadFromXML( const tinyxml2::XMLElement& shaderProgramElement );
	bool LoadFromStringLiterals( const char* vertexShaderText, const char* fragmentShaderText, const std::string& semiColonDelimitedDefines = "" );
	bool LoadFromFiles( const char* root, const std::string& semiColonDelimitedDefines = "" ); // load a shader from file
	unsigned int GetHandle() const;
	const ShaderProgramInfo* GetInfo() const;

private:
	void FillShaderInfo();
	void FillBlockInfo( PropertyBlockInfo& out_blockInfo, int blockIndex );

private:
	static GLuint LoadShaderFromText( const char* shaderText, GLenum type, const std::string& semiColonDelimitedDefines = "" );
	static GLuint LoadShaderFromFile( const char* fileName, GLenum type, const std::string& semiColonDelimitedDefines = "" );
	static std::string ExpandIncludesInShader( const char* filename, std::vector< std::string >& visitedFilenames );
	static std::string InjectDefinesIntoShader( const char* shaderText, const std::string& semiColonDelimitedDefines );
	static GLuint LinkShadersAndCreateProgram( GLint vertexShader, GLint fragmentShader );
	static void LogCompileError( GLuint shaderID );
	static void LogLinkError( GLuint programID );

public:
	unsigned int m_programHandle = 0; // OpenGL handle for this program, default 0
	ShaderProgramInfo* m_info = nullptr;
};
