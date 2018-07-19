#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/Matrix44.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Vector4.hpp"
#include "Engine/Renderer/GLFunctionBinding.hpp"
#include "Engine/Renderer/GLUtils.hpp"
#include "Engine/Tools/DevConsole.hpp"

//------------------------------------------------------------------------
bool GLCheckError( char const *file, int line )
{
#if defined(_DEBUG)
	GLenum error = glGetError();
	if ( error != GL_NO_ERROR )
	{
		DebuggerPrintf( "\nGL ERROR [0x%04x] at [%s(%i)]\n", error, file, line );
		ConsolePrintf( "GL ERROR [0x%04x] at [%s(%i)]", error, file, line );
		return true; 
	}
#endif
	return false; 
}

//------------------------------------------------------------------------
bool GLFailed()
{
	return GL_CHECK_ERROR(); 
}

//------------------------------------------------------------------------
bool GLSucceeded()
{
	return !GLFailed();
}

size_t GLGetTypeSize( GLint type )
{
	switch( type )
	{
		case GL_INT			:	return sizeof( int );
		case GL_UNSIGNED_INT:	return sizeof( unsigned int );
		case GL_BOOL		:	return sizeof( bool );
		case GL_FLOAT		:	return sizeof( float );
		case GL_FLOAT_VEC2	:	return sizeof( Vector2 );
		case GL_FLOAT_VEC3	:	return sizeof( Vector3 );
		case GL_FLOAT_VEC4	:	return sizeof( Vector4 );
		case GL_FLOAT_MAT4	:	return sizeof( Matrix44 );
		case GL_DOUBLE		:	return sizeof( double );
		default				:	return 0;
	}
}
