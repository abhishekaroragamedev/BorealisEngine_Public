#include "Engine/Renderer/GLFunctionBinding.hpp"

// WGL Functions
PFNWGLGETEXTENSIONSSTRINGARBPROC wglGetExtensionsStringARB = nullptr;
PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = nullptr;
PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = nullptr;

// GL Functions
PFNGLCLEARPROC glClear = nullptr;
PFNGLCLEARCOLORPROC glClearColor = nullptr;
PFNGLCLEARDEPTHPROC glClearDepth= nullptr;
PFNGLCLEARDEPTHFPROC glClearDepthf = nullptr;
PFNGLDEPTHFUNCPROC glDepthFunc = nullptr;
PFNGLDEPTHMASKPROC glDepthMask = nullptr;
PFNGLENABLEPROC glEnable = nullptr;
PFNGLDISABLEPROC glDisable = nullptr;
PFNGLGETERRORPROC glGetError = nullptr;
PFNGLBLENDFUNCPROC glBlendFunc = nullptr;
PFNGLBLENDFUNCSEPARATEPROC glBlendFuncSeparate = nullptr;
PFNGLBLENDFUNCSEPARATEIPROC glBlendFuncSeparatei = nullptr;
PFNGLBLENDEQUATIONSEPARATEPROC glBlendEquationSeparate = nullptr;
PFNGLBLENDEQUATIONSEPARATEIPROC glBlendEquationSeparatei = nullptr;
PFNGLLINEWIDTHPROC glLineWidth = nullptr;
PFNGLBINDTEXTUREPROC glBindTexture = nullptr;
PFNGLBINDIMAGETEXTUREPROC glBindImageTexture = nullptr;
PFNGLCREATESHADERPROC glCreateShader = nullptr;
PFNGLCOMPILESHADERPROC glCompileShader = nullptr;
PFNGLDELETESHADERPROC glDeleteShader = nullptr;
PFNGLSHADERSOURCEPROC glShaderSource = nullptr;
PFNGLGETSHADERIVPROC glGetShaderiv = nullptr;
PFNGLCREATEPROGRAMPROC glCreateProgram = nullptr;
PFNGLATTACHSHADERPROC glAttachShader = nullptr;
PFNGLLINKPROGRAMPROC glLinkProgram = nullptr;
PFNGLGETPROGRAMIVPROC glGetProgramiv = nullptr;
PFNGLGETACTIVEUNIFORMPROC glGetActiveUniform = nullptr;
PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC glGetActiveUniformBlockName = nullptr;
PFNGLGETACTIVEUNIFORMNAMEPROC glGetActiveUniformName = nullptr;
PFNGLGETACTIVEUNIFORMBLOCKIVPROC glGetActiveUniformBlockiv = nullptr;
PFNGLGETACTIVEUNIFORMSIVPROC glGetActiveUniformsiv = nullptr;
PFNGLDELETEPROGRAMPROC glDeleteProgram = nullptr;
PFNGLDETACHSHADERPROC glDetachShader = nullptr;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog = nullptr;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog = nullptr;
PFNGLGENBUFFERSPROC glGenBuffers = nullptr;
PFNGLDELETEBUFFERSPROC glDeleteBuffers = nullptr;
PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers = nullptr;
PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers = nullptr;
PFNGLBINDBUFFERPROC glBindBuffer = nullptr;
PFNGLBINDBUFFERBASEPROC glBindBufferBase = nullptr;
PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer = nullptr;
PFNGLBUFFERDATAPROC glBufferData = nullptr;
PFNGLFRAMEBUFFERTEXTUREPROC glFramebufferTexture = nullptr;
PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D = nullptr;
PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus = nullptr;
PFNGLBLITFRAMEBUFFERPROC glBlitFramebuffer = nullptr;
PFNGLGENVERTEXARRAYSPROC glGenVertexArrays = nullptr;
PFNGLBINDVERTEXARRAYPROC glBindVertexArray = nullptr;
PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays = nullptr;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation = nullptr;
PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation = nullptr;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray = nullptr;
PFNGLUNIFORM1FPROC glUniform1f = nullptr;
PFNGLUNIFORM2FPROC glUniform2f = nullptr;
PFNGLUNIFORM3FPROC glUniform3f = nullptr;
PFNGLUNIFORM4FPROC glUniform4f = nullptr;
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv = nullptr;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer = nullptr;
PFNGLUSEPROGRAMPROC glUseProgram = nullptr;
PFNGLDRAWARRAYSPROC glDrawArrays = nullptr;
PFNGLDRAWELEMENTSPROC glDrawElements = nullptr;
PFNGLDISPATCHCOMPUTEPROC glDispatchCompute = nullptr;
PFNGLDRAWBUFFERPROC glDrawBuffer = nullptr;
PFNGLDRAWBUFFERSPROC glDrawBuffers = nullptr;
PFNGLREADBUFFERPROC glReadBuffer= nullptr;
PFNGLGETINTEGERVPROC glGetIntegerv = nullptr;
PFNGLGENTEXTURESPROC glGenTextures = nullptr;
PFNGLGENERATEMIPMAPPROC glGenerateMipmap = nullptr;
PFNGLDELETETEXTURESPROC glDeleteTextures = nullptr;
PFNGLTEXSTORAGE2DPROC glTexStorage2D = nullptr;
PFNGLTEXSUBIMAGE2DPROC glTexSubImage2D = nullptr;
PFNGLTEXIMAGE2DPROC glTexImage2D = nullptr;
PFNGLTEXIMAGE2DMULTISAMPLEPROC glTexImage2DMultisample = nullptr;
PFNGLTEXSTORAGE3DPROC glTexStorage3D = nullptr;
PFNGLTEXSUBIMAGE3DPROC glTexSubImage3D = nullptr;
PFNGLTEXIMAGE3DPROC glTexImage3D = nullptr;
PFNGLGETTEXIMAGEPROC glGetTexImage = nullptr;
PFNGLGETTEXTUREIMAGEPROC glGetTextureImage = nullptr;
PFNGLREADPIXELSPROC glReadPixels = nullptr;
PFNGLREADNPIXELSPROC glReadnPixels = nullptr;
PFNGLGENSAMPLERSPROC glGenSamplers = nullptr;
PFNGLDELETESAMPLERSPROC glDeleteSamplers = nullptr;
PFNGLSAMPLERPARAMETERIPROC glSamplerParameteri = nullptr;
PFNGLSAMPLERPARAMETERIVPROC glSamplerParameteriv = nullptr;
PFNGLSAMPLERPARAMETERFVPROC glSamplerParameterfv = nullptr;
PFNGLPIXELSTOREIPROC glPixelStorei = nullptr;
PFNGLTEXPARAMETERIPROC glTexParameteri = nullptr;
PFNGLBINDSAMPLERPROC glBindSampler = nullptr;
PFNGLACTIVETEXTUREPROC glActiveTexture = nullptr;
PFNGLPOLYGONMODEPROC glPolygonMode = nullptr;
PFNGLCULLFACEPROC glCullFace = nullptr;
PFNGLFRONTFACEPROC glFrontFace = nullptr;
PFNGLVIEWPORTPROC glViewport = nullptr;

template <typename T>
bool wglGetTypedProcAddress( HMODULE glLibrary, T *out, char const *name ) 
{
	*out = (T) wglGetProcAddress( name ); 

	if ((*out) == nullptr) {
		*out = (T) GetProcAddress( glLibrary, name ); 
	}

	return (*out != nullptr); 
}

#define GL_BIND_FUNCTION( glLibrary, f ) wglGetTypedProcAddress( glLibrary, &f, #f )

void BindNewWGLFunctions( HMODULE glLibrary )
{
	GL_BIND_FUNCTION( glLibrary, wglGetExtensionsStringARB ); 
	GL_BIND_FUNCTION( glLibrary, wglChoosePixelFormatARB ); 
	GL_BIND_FUNCTION( glLibrary, wglCreateContextAttribsARB );
}

void BindGLFunctions( HMODULE glLibrary )
{
	GL_BIND_FUNCTION( glLibrary, glClear );
	GL_BIND_FUNCTION( glLibrary, glClearColor );
	GL_BIND_FUNCTION( glLibrary, glClearDepth );
	GL_BIND_FUNCTION( glLibrary, glClearDepthf );
	GL_BIND_FUNCTION( glLibrary, glDepthFunc );
	GL_BIND_FUNCTION( glLibrary, glDepthMask );
	GL_BIND_FUNCTION( glLibrary, glEnable );
	GL_BIND_FUNCTION( glLibrary, glDisable );
	GL_BIND_FUNCTION( glLibrary, glGetError );
	GL_BIND_FUNCTION( glLibrary, glBlendFunc );
	GL_BIND_FUNCTION( glLibrary, glBlendFuncSeparate );
	GL_BIND_FUNCTION( glLibrary, glBlendFuncSeparatei );
	GL_BIND_FUNCTION( glLibrary, glBlendEquationSeparate );
	GL_BIND_FUNCTION( glLibrary, glBlendEquationSeparatei );
	GL_BIND_FUNCTION( glLibrary, glLineWidth );
	GL_BIND_FUNCTION( glLibrary, glBindTexture );
	GL_BIND_FUNCTION( glLibrary, glBindImageTexture );
	GL_BIND_FUNCTION( glLibrary, glCreateShader );
	GL_BIND_FUNCTION( glLibrary, glCompileShader );
	GL_BIND_FUNCTION( glLibrary, glDeleteShader );
	GL_BIND_FUNCTION( glLibrary, glShaderSource );
	GL_BIND_FUNCTION( glLibrary, glGetShaderiv );
	GL_BIND_FUNCTION( glLibrary, glCreateProgram );
	GL_BIND_FUNCTION( glLibrary, glAttachShader );
	GL_BIND_FUNCTION( glLibrary, glLinkProgram );
	GL_BIND_FUNCTION( glLibrary, glGetProgramiv );
	GL_BIND_FUNCTION( glLibrary, glGetActiveUniform );
	GL_BIND_FUNCTION( glLibrary, glGetActiveUniformBlockName );
	GL_BIND_FUNCTION( glLibrary, glGetActiveUniformName );
	GL_BIND_FUNCTION( glLibrary, glGetActiveUniformBlockiv );
	GL_BIND_FUNCTION( glLibrary, glGetActiveUniformsiv );
	GL_BIND_FUNCTION( glLibrary, glDeleteProgram );
	GL_BIND_FUNCTION( glLibrary, glDetachShader );
	GL_BIND_FUNCTION( glLibrary, glGetShaderInfoLog );
	GL_BIND_FUNCTION( glLibrary, glGetProgramInfoLog );
	GL_BIND_FUNCTION( glLibrary, glGenBuffers );
	GL_BIND_FUNCTION( glLibrary, glDeleteBuffers );
	GL_BIND_FUNCTION( glLibrary, glGenFramebuffers );
	GL_BIND_FUNCTION( glLibrary, glDeleteFramebuffers );
	GL_BIND_FUNCTION( glLibrary, glBindBuffer );
	GL_BIND_FUNCTION( glLibrary, glBindBufferBase );
	GL_BIND_FUNCTION( glLibrary, glBindFramebuffer );
	GL_BIND_FUNCTION( glLibrary, glBufferData );
	GL_BIND_FUNCTION( glLibrary, glFramebufferTexture );
	GL_BIND_FUNCTION( glLibrary, glFramebufferTexture2D );
	GL_BIND_FUNCTION( glLibrary, glCheckFramebufferStatus );
	GL_BIND_FUNCTION( glLibrary, glBlitFramebuffer );
	GL_BIND_FUNCTION( glLibrary, glGenVertexArrays );
	GL_BIND_FUNCTION( glLibrary, glBindVertexArray );
	GL_BIND_FUNCTION( glLibrary, glDeleteVertexArrays );
	GL_BIND_FUNCTION( glLibrary, glGetUniformLocation );
	GL_BIND_FUNCTION( glLibrary, glGetAttribLocation );
	GL_BIND_FUNCTION( glLibrary, glEnableVertexAttribArray );
	GL_BIND_FUNCTION( glLibrary, glUniform1f );
	GL_BIND_FUNCTION( glLibrary, glUniform2f );
	GL_BIND_FUNCTION( glLibrary, glUniform3f );
	GL_BIND_FUNCTION( glLibrary, glUniform4f );
	GL_BIND_FUNCTION( glLibrary, glUniformMatrix4fv );
	GL_BIND_FUNCTION( glLibrary, glVertexAttribPointer );
	GL_BIND_FUNCTION( glLibrary, glUseProgram );
	GL_BIND_FUNCTION( glLibrary, glDrawArrays );
	GL_BIND_FUNCTION( glLibrary, glDrawElements );
	GL_BIND_FUNCTION( glLibrary, glDispatchCompute );
	GL_BIND_FUNCTION( glLibrary, glDrawBuffer );
	GL_BIND_FUNCTION( glLibrary, glDrawBuffers );
	GL_BIND_FUNCTION( glLibrary, glReadBuffer );
	GL_BIND_FUNCTION( glLibrary, glGetIntegerv );
	GL_BIND_FUNCTION( glLibrary, glGenTextures );
	GL_BIND_FUNCTION( glLibrary, glGenerateMipmap );
	GL_BIND_FUNCTION( glLibrary, glDeleteTextures );
	GL_BIND_FUNCTION( glLibrary, glTexStorage2D );
	GL_BIND_FUNCTION( glLibrary, glTexSubImage2D );
	GL_BIND_FUNCTION( glLibrary, glTexImage2D );
	GL_BIND_FUNCTION( glLibrary, glTexImage2DMultisample );
	GL_BIND_FUNCTION( glLibrary, glTexStorage3D );
	GL_BIND_FUNCTION( glLibrary, glTexSubImage3D );
	GL_BIND_FUNCTION( glLibrary, glTexImage3D );
	GL_BIND_FUNCTION( glLibrary, glGetTexImage );
	GL_BIND_FUNCTION( glLibrary, glGetTextureImage );
	GL_BIND_FUNCTION( glLibrary, glReadPixels );
	GL_BIND_FUNCTION( glLibrary, glReadnPixels );
	GL_BIND_FUNCTION( glLibrary, glGenSamplers );
	GL_BIND_FUNCTION( glLibrary, glDeleteSamplers );
	GL_BIND_FUNCTION( glLibrary, glSamplerParameteri );
	GL_BIND_FUNCTION( glLibrary, glSamplerParameteriv );
	GL_BIND_FUNCTION( glLibrary, glSamplerParameterfv );
	GL_BIND_FUNCTION( glLibrary, glPixelStorei );
	GL_BIND_FUNCTION( glLibrary, glTexParameteri );
	GL_BIND_FUNCTION( glLibrary, glBindSampler );
	GL_BIND_FUNCTION( glLibrary, glActiveTexture );
	GL_BIND_FUNCTION( glLibrary, glPolygonMode );
	GL_BIND_FUNCTION( glLibrary, glCullFace );
	GL_BIND_FUNCTION( glLibrary, glFrontFace );
	GL_BIND_FUNCTION( glLibrary, glViewport );
}
