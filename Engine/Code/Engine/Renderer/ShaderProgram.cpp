#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/FileUtils/File.hpp"
#include "Engine/Renderer/GLFunctionBinding.hpp"
#include "Engine/Renderer/GLUtils.hpp"
#include "Engine/Renderer/PropertyBlock.hpp"
#include "Engine/Renderer/ShaderProgram.hpp"
#include <string>
#include <vector>

ShaderProgram::ShaderProgram()
{
	
}

ShaderProgram::ShaderProgram( const tinyxml2::XMLElement& shaderProgramElement )
{
	m_programHandle = ParseXmlAttribute( shaderProgramElement, "id", UINT_MAX );

	std::string defines = ParseXmlAttribute( shaderProgramElement, "define", "" );
	std::string fileName = ParseXmlAttribute( shaderProgramElement, "fileName", "" );

	if ( fileName != "" )
	{
		LoadFromFiles( fileName.c_str(), defines );
	}

	FillShaderInfo();
}

ShaderProgram::ShaderProgram( const ShaderProgram& copy )
{
	m_programHandle = copy.m_programHandle;
	FillShaderInfo();
}

ShaderProgram::~ShaderProgram()
{
	delete m_info;
	m_info = nullptr;
}

void ShaderProgram::FillShaderInfo()
{
	m_info = new ShaderProgramInfo();

	glUseProgram( m_programHandle );

	GLint uniformCount = 0;
	glGetProgramiv( m_programHandle, GL_ACTIVE_UNIFORMS, &uniformCount );

	const GLsizei maxNameLength = UNIFORM_NAME_MAX_LENGTH;
	char uniformName[ maxNameLength ];

	PropertyBlockInfo defaultPropertyBlock = PropertyBlockInfo( "Default", UNIFORM_NAME_MAX_LENGTH );
	m_info->m_propertyBlockInfos.push_back( defaultPropertyBlock );

	for ( GLint uniformIndex = 0; uniformIndex < uniformCount; uniformIndex++ )
	{
		GLsizei nameLength = 0;
		GLsizei size = 0;
		GLenum type = GL_NONE;

		glGetActiveUniform( m_programHandle, uniformIndex, maxNameLength, &nameLength, &size, &type, uniformName );
		if ( type != GL_NONE )
		{
			GLuint bindPoint = glGetUniformLocation( m_programHandle, uniformName );

			if ( bindPoint != -1 ) // If this is -1, the uniform is part of a Uniform Block
			{
				switch ( type )
				{
					case GL_SAMPLER_1D:
					case GL_SAMPLER_2D:
					case GL_SAMPLER_3D:
					case GL_SAMPLER_CUBE:
					{
						m_info->m_samplerBlockInfos.push_back( SamplerBlockInfo( uniformName, bindPoint ) );
						break;
					}
					case GL_INT			:
					case GL_UNSIGNED_INT:
					case GL_BOOL		:
					case GL_FLOAT		:
					case GL_FLOAT_VEC2	:
					case GL_FLOAT_VEC3	:
					case GL_FLOAT_VEC4	:
					case GL_FLOAT_MAT4	:
					case GL_DOUBLE		:
					{
						/*
						PropertyBlockInfo* defaultBlockInfo = &m_info->m_propertyBlockInfos[ m_info->m_propertyBlockInfos.size() - 1 ];

						GLint uniformOffset;
						glGetActiveUniformsiv( m_programHandle, 1, reinterpret_cast< GLuint* >( &uniformIndex ), GL_UNIFORM_OFFSET, &uniformOffset );
						GLint uniformType;
						glGetActiveUniformsiv( m_programHandle, 1, reinterpret_cast< GLuint* >( &uniformIndex ), GL_UNIFORM_TYPE, &uniformType );
						GLint uniformCount;
						glGetActiveUniformsiv( m_programHandle, 1, reinterpret_cast< GLuint* >( &uniformIndex ), GL_UNIFORM_SIZE, &uniformCount );

						defaultBlockInfo->m_propertyInfos.push_back( PropertyInfo( defaultBlockInfo, uniformName, uniformOffset, GLGetTypeSize( uniformType ), uniformCount ) );
						*/
						break;
					}
					default:
					{
						//ERROR_RECOVERABLE( "ShaderProgram::FillShaderInfo - Global uniform name provided : %s. Skipping... ", uniformName );
						break;
					}
				}
			}
		}
	}

	GLint blockCount = 0;
	glGetProgramiv( m_programHandle, GL_ACTIVE_UNIFORM_BLOCKS, &blockCount );
	GLsizei blockLength = 0;
	char blockName[ maxNameLength ];

	for ( GLint blockIndex = 0; blockIndex < blockCount; blockIndex++ )
	{
		blockLength = 0;
		glGetActiveUniformBlockName( m_programHandle, blockIndex, maxNameLength, &blockLength, blockName );
		if ( blockLength > 0 )
		{
			PropertyBlockInfo newBlockInfo = PropertyBlockInfo( blockName );

			GLint bindPoint = -1;
			glGetActiveUniformBlockiv( m_programHandle, blockIndex, GL_UNIFORM_BLOCK_BINDING,  &bindPoint );
			//ASSERT_RECOVERABLE( ( bindPoint != -1 ), "ShaderProgram::FillShaderInfo - Bind point is -1." );
			newBlockInfo.m_bindPoint = static_cast< unsigned int >( bindPoint );

			GLint blockSize = 0;
			glGetActiveUniformBlockiv( m_programHandle, blockIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize );
			newBlockInfo.m_maxSize = static_cast< size_t >( blockSize );

			FillBlockInfo( newBlockInfo, blockIndex );

			m_info->m_propertyBlockInfos.push_back( newBlockInfo );
		}
	}
}

void ShaderProgram::FillBlockInfo( PropertyBlockInfo& out_blockInfo, int blockIndex )
{
	GLint activeUniformCount;
	glGetActiveUniformBlockiv( m_programHandle, blockIndex, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &activeUniformCount );
	if ( activeUniformCount <= 0 )
	{
		return;
	}

	GLint* indices = new GLint[ activeUniformCount ];
	glGetActiveUniformBlockiv( m_programHandle, blockIndex, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, indices );

	GLint* offsets = new GLint[ activeUniformCount ];
	GLint* types = new GLint[ activeUniformCount ];
	GLint* counts = new GLint[ activeUniformCount ];

	glGetActiveUniformsiv( m_programHandle, activeUniformCount, reinterpret_cast< GLuint* >( indices ), GL_UNIFORM_OFFSET, offsets );
	glGetActiveUniformsiv( m_programHandle, activeUniformCount, reinterpret_cast< GLuint* >( indices ), GL_UNIFORM_TYPE, types );
	glGetActiveUniformsiv( m_programHandle, activeUniformCount, reinterpret_cast< GLuint* >( indices ), GL_UNIFORM_SIZE, counts );

	for ( GLint uniformIndex = 0; uniformIndex < activeUniformCount; uniformIndex++ )
	{
		char name[ UNIFORM_NAME_MAX_LENGTH ];
		GLint length = 0;
		glGetActiveUniformName( m_programHandle, static_cast< GLuint >( indices[ uniformIndex ] ), sizeof( name ), &length, name );

		PropertyInfo propertyInfo = PropertyInfo( &out_blockInfo );
		propertyInfo.m_name = std::string( name );
		propertyInfo.m_offset = offsets[ uniformIndex ];
		propertyInfo.m_size = GLGetTypeSize( types[ uniformIndex ] );
		propertyInfo.m_count = counts[ uniformIndex ];

		out_blockInfo.m_propertyInfos.push_back( propertyInfo );
	}

	delete[] indices;
	delete[] offsets;
	delete[] types;
	delete[] counts;
}

GLuint ShaderProgram::GetHandle() const
{
	return m_programHandle;
}

const ShaderProgramInfo* ShaderProgram::GetInfo() const
{
	return m_info;
}

bool ShaderProgram::LoadFromStringLiterals( const char* vertexShaderText, const char* fragmentShaderText, const std::string& semiColonDelimitedDefines )
{
	GLuint vertShader = LoadShaderFromText( vertexShaderText, GL_VERTEX_SHADER, semiColonDelimitedDefines );
	GLuint fragShader = LoadShaderFromText( fragmentShaderText, GL_FRAGMENT_SHADER, semiColonDelimitedDefines ); 

	if ( vertShader == NULL || fragShader == NULL )
	{
		return false;
	}

	m_programHandle = LinkShadersAndCreateProgram( vertShader, fragShader ); 
	glDeleteShader( vertShader ); 
	glDeleteShader( fragShader ); 

	return ( m_programHandle != NULL );
}

GLuint ShaderProgram::LoadShaderFromText( const char* shaderText, GLenum type, const std::string& semiColonDelimitedDefines )
{
	GLuint shaderID = glCreateShader( type );
	GUARANTEE_RECOVERABLE( shaderID != NULL, "ShaderProgram::LoadShaderFromText: Unable to instantiate shader! Aborting..." );

	std::string finalShaderText = std::string( shaderText );
	finalShaderText = InjectDefinesIntoShader( shaderText, semiColonDelimitedDefines );
	const char* finalShaderTextAsCStr = finalShaderText.c_str();

	GLint shaderLength = ( GLint )strlen( finalShaderTextAsCStr );
	glShaderSource( shaderID, 1, &finalShaderTextAsCStr, &shaderLength );
	glCompileShader( shaderID );

	GLint status;
	glGetShaderiv( shaderID, GL_COMPILE_STATUS, &status );

	if ( status == GL_FALSE )
	{
		LogCompileError( shaderID ); // function we write
		glDeleteShader( shaderID );
		shaderID = NULL;
	}

	return shaderID;
}

bool ShaderProgram::LoadFromFiles( const char* root, const std::string& semiColonDelimitedDefines )
{
	std::string vsFile = root;
	vsFile += ".vs"; 

	std::string fsFile = root; 
	fsFile += ".fs"; 

	// Compile the two stages we're using (all shaders will implement the vertex and fragment stages)
	// later on, we can add in more stages;
	GLuint vertShader = LoadShaderFromFile( vsFile.c_str(), GL_VERTEX_SHADER, semiColonDelimitedDefines ); 
	GLuint fragShader = LoadShaderFromFile( fsFile.c_str(), GL_FRAGMENT_SHADER, semiColonDelimitedDefines ); 

	if ( vertShader == NULL || fragShader == NULL )
	{
		return false;
	}

	// Link the program
	// program_handle is a member GLuint. 
	m_programHandle = LinkShadersAndCreateProgram( vertShader, fragShader ); 
	glDeleteShader( vertShader ); 
	glDeleteShader( fragShader ); 

	return ( m_programHandle != NULL );
}

GLuint ShaderProgram::LoadShaderFromFile( const char* fileName, GLenum type, const std::string& semiColonDelimitedDefines )
{
	char *src = ( char* ) FileReadToNewBuffer( fileName );
	GUARANTEE_RECOVERABLE( src != nullptr, "ShaderProgram::LoadShaderFromFile: Cannot find shader file! Aborting..." );

	// Create a shader
	GLuint shaderID = glCreateShader( type );
	GUARANTEE_RECOVERABLE( shaderID != NULL, "ShaderProgram::LoadShaderFromFile: Unable to instantiate shader! Aborting..." );

	std::string finalShaderText =  std::string( src );
	finalShaderText = InjectDefinesIntoShader( src, semiColonDelimitedDefines );

	const char* finalShaderTextAsCStr = finalShaderText.c_str();

	// Bind source to it, and compile
	// You can add multiple strings to a shader – they will 
	// be concatenated together to form the actual source object.
	GLint shaderLength = ( GLint )strlen( finalShaderTextAsCStr );
	glShaderSource( shaderID, 1, &finalShaderTextAsCStr, &shaderLength );
	glCompileShader( shaderID );

	// Check status
	GLint status;
	glGetShaderiv( shaderID, GL_COMPILE_STATUS, &status );

	if ( status == GL_FALSE )
	{
		LogCompileError( shaderID ); // function we write
		glDeleteShader( shaderID );
		shaderID = NULL;
	}

	free( src );

	return shaderID;
}

std::string ShaderProgram::InjectDefinesIntoShader( const char* shaderText, const std::string& semiColonDelimitedDefines )
{
	std::string shaderTextAsString = std::string( shaderText );
	if ( semiColonDelimitedDefines.length() == 0 )
	{
		return std::string( shaderText );
	}

	size_t positionToInject = shaderTextAsString.find( "#version" );
	positionToInject = shaderTextAsString.find_first_of( "\n", positionToInject ) + 1;		// Find first line break after the version line

	TokenizedString tokenizedString = TokenizedString( semiColonDelimitedDefines, ";" );
	std::vector< std::string > tokens = tokenizedString.GetTokens();
	for ( std::vector< std::string >::iterator tokenIterator = tokens.begin(); tokenIterator != tokens.end(); tokenIterator++ )
	{
		std::string defineText = *tokenIterator;
		if ( defineText.find( "=" ) != std::string::npos )	// If there's an "=" in the define, we need to replace it with a " "
		{
			defineText[ defineText.find( "=" ) ] = ' ';
		}
		defineText = "#define " + defineText + "\n";

		shaderTextAsString.insert( positionToInject, defineText );
		positionToInject += defineText.length();
	}

	return shaderTextAsString;
}

GLuint ShaderProgram::LinkShadersAndCreateProgram( GLint vertexShader, GLint fragmentShader )
{
	// Create the program handle - how you will reference
	// this program within OpenGL, like a texture handle
	GLuint programID = glCreateProgram();
	GUARANTEE_RECOVERABLE( programID != 0, "Cannot find shader program!" );

	// Attach the shaders you want to use
	glAttachShader( programID, vertexShader );
	glAttachShader( programID, fragmentShader );

	// Link the program (create the GPU program)
	glLinkProgram( programID );

	// Check for link errors - usually a result
	// of incompatibility between stages.
	GLint linkStatus;
	glGetProgramiv( programID, GL_LINK_STATUS, &linkStatus );

	if ( linkStatus == GL_FALSE )
	{
		LogLinkError( programID );
		glDeleteProgram( programID );
		programID = 0;
	} 

	// no longer need the shaders, you can detach them if you want
	// (not necessary)
	glDetachShader( programID, vertexShader );
	glDetachShader( programID, fragmentShader );

	return programID;
}


void ShaderProgram::LogCompileError( GLuint shaderID )
{
	// figure out how large the buffer needs to be
	GLint length;
	glGetShaderiv( shaderID, GL_INFO_LOG_LENGTH, &length );

	// make a buffer, and copy the log to it. 
	char *buffer = new char[ length + 1 ];
	glGetShaderInfoLog( shaderID, length, &length, buffer );

	// Print it out (may want to do some additional formatting)
	buffer[ length ] = NULL;
	DebuggerPrintf( "Shader Compile Error: ", buffer );
	ERROR_RECOVERABLE( buffer );

	// free up the memory we used. 
	delete buffer;
}

void ShaderProgram::LogLinkError(GLuint programID)
{
	// get the buffer length
	GLint length;
	glGetProgramiv( programID, GL_INFO_LOG_LENGTH, &length );

	// copy the log into a new buffer
	char *buffer = new char[ length + 1 ];
	glGetProgramInfoLog( programID, length, &length, buffer );

	// print it to the output pane
	buffer[length] = NULL;
	DebuggerPrintf( "Shader Link Error: ", buffer );
	ERROR_RECOVERABLE( buffer );         

	// cleanup
	delete buffer;
}

