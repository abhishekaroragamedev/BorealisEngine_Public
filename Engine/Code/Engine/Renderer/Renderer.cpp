#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/Threading.hpp"
#include "Engine/Core/Window.hpp"
#include "Engine/Core/XmlUtilities.hpp"
#include "Engine/FileUtils/File.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Renderer/ForwardRenderingPath.hpp"
#include "Engine/Renderer/GLFunctionBinding.hpp"
#include "Engine/Renderer/GLUtils.hpp"
#include "Engine/Renderer/InputLayout.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/Renderable.hpp"
#include "Engine/Renderer/RenderBuffer.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Sampler.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Renderer/ShaderPass.hpp"
#include "Engine/Renderer/ShaderProgram.hpp"
#include "Engine/Renderer/ShadersAsRawStrings.hpp"
#include "Engine/Renderer/UniformBuffer.hpp"
#include "Engine/Renderer/External/gl/glcorearb.h"
#include "Engine/Renderer/External/gl/glext.h"
#include "Engine/Renderer/External/gl/wglext.h"
#include "Engine/Tools/Command.hpp"
#include "Engine/Tools/DevConsole.hpp"
#include "Engine/Tools/Profiler/ProfileScope.hpp"
#include "ThirdParty/stb/stb_image_write.h"
#include "ThirdParty/tinyXML2/tinyxml2.h"
#include <algorithm>

// INTERNAL CONSTANTS
HMODULE g_glLibrary;
HDC* g_deviceDisplayContext = nullptr;
HGLRC g_glContext = nullptr;

Renderer* Renderer::s_renderer;

/* static */
Renderer* Renderer::GetInstance()
{
	return Renderer::s_renderer;
}

Renderer::Renderer()
{
	Renderer::s_renderer = this;

	RenderStartup();
	InitializeRenderer();
	PostStartup();
}

Renderer::~Renderer()
{
	ForwardRenderingPath::Shutdown();

	delete m_defaultShaderPass;
	m_defaultShaderPass = nullptr;

	delete m_effectsCamera;
	m_effectsCamera = nullptr;

	delete m_effectsSwapTarget;
	m_effectsSwapTarget = nullptr;

	delete m_defaultColorTarget;
	m_defaultColorTarget = nullptr;

	delete m_defaultDepthStencilTarget;
	m_defaultDepthStencilTarget = nullptr;

	delete m_defaultCamera;
	m_defaultCamera = nullptr;

	if ( m_currentCameraUBO != nullptr )
	{
		delete m_currentCameraUBO;
		m_currentCameraUBO = nullptr;
	}
	if ( m_activeLightsUBO != nullptr )
	{
		delete m_activeLightsUBO;
		m_activeLightsUBO = nullptr;
	}
	if ( m_objectLightPropertiesUBO != nullptr )
	{
		delete m_objectLightPropertiesUBO;
		m_objectLightPropertiesUBO = nullptr;
	}
	if ( m_modelMatrixUBO != nullptr )
	{
		delete m_modelMatrixUBO;
		m_modelMatrixUBO = nullptr;
	}

	delete m_immediateMesh;
	m_immediateMesh = nullptr;

	delete m_defaultSamplerMipMap;
	m_defaultSamplerMipMap = nullptr;

	delete m_defaultSampler;
	m_defaultSampler = nullptr;

	m_defaultBitmapFont = nullptr;

	for ( std::map< std::string, BitmapFont* >::iterator mapIterator = m_loadedFontsByName.begin(); mapIterator != m_loadedFontsByName.end(); mapIterator++ )
	{
		if ( mapIterator->second != nullptr )		// Deallocate memory for BitmapFont objects
		{
			delete mapIterator->second;
			mapIterator->second = nullptr;
		}
	}

	for ( std::map< std::string, Mesh* >::iterator mapIterator = m_loadedMeshesByName.begin(); mapIterator != m_loadedMeshesByName.end(); mapIterator++ )
	{
		if ( mapIterator->second != nullptr )		// Deallocate memory for Mesh objects
		{
			delete mapIterator->second;
			mapIterator->second = nullptr;
		}
	}

	for ( std::map< std::string, Sprite* >::iterator mapIterator = m_loadedSpritesByName.begin(); mapIterator != m_loadedSpritesByName.end(); mapIterator++ )
	{
		if ( mapIterator->second != nullptr )		// Deallocate memory for Sprite objects
		{
			delete mapIterator->second;
			mapIterator->second = nullptr;
		}
	}

	for ( std::map< std::string, SpriteSheet* >::iterator mapIterator = m_loadedFontSpriteSheetsByName.begin(); mapIterator != m_loadedFontSpriteSheetsByName.end(); mapIterator++ )
	{
		if ( mapIterator->second != nullptr )		// Deallocate memory for TextureAtlas objects
		{
			delete mapIterator->second;
			mapIterator->second = nullptr;
		}
	}

	for ( std::map< std::string, Texture* >::iterator mapIterator = m_loadedTexturesByName.begin(); mapIterator != m_loadedTexturesByName.end(); mapIterator++ )
	{
		if ( mapIterator->second != nullptr )		// Deallocate memory for Texture objects (in main memory)
		{
			delete mapIterator->second;
			mapIterator->second = nullptr;
		}
	}

	for ( std::map< std::string, Material* >::iterator mapIterator = m_loadedMaterialsFromFileByName.begin(); mapIterator != m_loadedMaterialsFromFileByName.end(); mapIterator++ )
	{
		if ( mapIterator->second != nullptr )		// Deallocate memory for Material objects (in main memory)
		{
			delete mapIterator->second;
			mapIterator->second = nullptr;
		}
	}

	for ( std::map< std::string, Shader* >::iterator mapIterator = m_loadedShadersFromFileByName.begin(); mapIterator != m_loadedShadersFromFileByName.end(); mapIterator++ )
	{
		if ( mapIterator->second != nullptr )		// Deallocate memory for Shader objects (in main memory)
		{
			for ( unsigned int passIndex = 0U; passIndex < mapIterator->second->GetPassCount(); passIndex++ )
			{
				delete mapIterator->second->GetPass( passIndex )->m_program;	// Delete the ShaderPrograms associated with this shader, as it's not referenced by the Renderer
				mapIterator->second->GetPass( passIndex )->m_program = nullptr;
			}

			delete mapIterator->second;
			mapIterator->second = nullptr;
		}
	}

	for ( std::map< std::string, ShaderProgram* >::iterator mapIterator = m_loadedShaderProgramsByName.begin(); mapIterator != m_loadedShaderProgramsByName.end(); mapIterator++ )
	{
		if ( mapIterator->second != nullptr )		// Deallocate memory for ShaderProgram objects (in main memory)
		{
			delete mapIterator->second;
			mapIterator->second = nullptr;
		}
	}
}

#pragma region RendererInitialization

HGLRC CreateOldRenderContext()		// Hidden function
{
	PIXELFORMATDESCRIPTOR pixelFormatDescriptor;
	memset( &pixelFormatDescriptor, 0, sizeof( pixelFormatDescriptor ) );
	pixelFormatDescriptor.nSize = sizeof( pixelFormatDescriptor );
	pixelFormatDescriptor.nVersion = 1;
	pixelFormatDescriptor.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pixelFormatDescriptor.iPixelType = PFD_TYPE_RGBA;
	pixelFormatDescriptor.cColorBits = 24;
	pixelFormatDescriptor.cDepthBits = 24;
	pixelFormatDescriptor.cAccumBits = 0;
	pixelFormatDescriptor.cStencilBits = 8;

	int pixelFormatCode = ChoosePixelFormat( *g_deviceDisplayContext, &pixelFormatDescriptor );
	SetPixelFormat( *g_deviceDisplayContext, pixelFormatCode, &pixelFormatDescriptor );

	HGLRC openGlRenderingContext = wglCreateContext( *g_deviceDisplayContext );
	return openGlRenderingContext;
}

//------------------------------------------------------------------------
// Creates a real context as a specific version (major.minor)
static HGLRC CreateRealRenderContext( int major, int minor )		// Hidden function
{
	// So similar to creating the temp one - we want to define 
	// the style of surface we want to draw to.  But now, to support
	// extensions, it takes key_value pairs
	int const format_attribs[] = {
		WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,    // The rc will be used to draw to a window
		WGL_SUPPORT_OPENGL_ARB, GL_TRUE,    // ...can be drawn to by GL
		WGL_DOUBLE_BUFFER_ARB, GL_TRUE,     // ...is double buffered
		WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB, // ...uses a RGBA texture
		WGL_COLOR_BITS_ARB, 24,             // 24 bits for color (8 bits per channel)
											// WGL_DEPTH_BITS_ARB, 24,          // if you wanted depth a default depth buffer...
											// WGL_STENCIL_BITS_ARB, 8,         // ...you could set these to get a 24/8 Depth/Stencil.
											NULL, NULL,                         // Tell it we're done.
	};

	// Given the above criteria, we're going to search for formats
	// our device supports that give us it.  I'm allowing 128 max returns (which is overkill)
	size_t const MAX_PIXEL_FORMATS = 128;
	int formats[MAX_PIXEL_FORMATS];
	int pixel_format = 0;
	UINT format_count = 0;

	BOOL succeeded = wglChoosePixelFormatARB( *g_deviceDisplayContext, 
		format_attribs, 
		nullptr, 
		MAX_PIXEL_FORMATS, 
		formats, 
		(UINT*)&format_count );

	if (!succeeded) {
		return NULL; 
	}

	// Loop through returned formats, till we find one that works
	for (unsigned int i = 0; i < format_count; ++i) {
		pixel_format = formats[i];
		succeeded = SetPixelFormat( *g_deviceDisplayContext, pixel_format, NULL ); // same as the temp context; 
		if (succeeded) {
			break;
		} else {
			DWORD error = GetLastError();
			DebuggerPrintf( "Failed to set the format: %u", error ); 
		}
	}

	if (!succeeded) {
		return NULL; 
	}

	// Okay, HDC is setup to the right format, now create our GL context

	// First, options for creating a debug context (potentially slower, but 
	// driver may report more useful errors). 
	int context_flags = 0; 
#if defined(_DEBUG)
	context_flags |= WGL_CONTEXT_DEBUG_BIT_ARB; 
#endif

	// describe the context
	int const attribs[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB, major,                             // Major GL Version
		WGL_CONTEXT_MINOR_VERSION_ARB, minor,                             // Minor GL Version
		WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,   // Restrict to core (no compatibility)
		WGL_CONTEXT_FLAGS_ARB, context_flags,                             // Misc flags (used for debug above)
		0, 0
	};

	// Try to create context
	HGLRC context = wglCreateContextAttribsARB( *g_deviceDisplayContext, NULL, attribs );
	if (context == NULL) {
		return NULL; 
	}

	return context;
}

bool Renderer::RenderStartup()
{
	g_glLibrary = ::LoadLibraryA( "opengl32.dll" );

	g_deviceDisplayContext = new HDC ( GetDC( *( ( HWND* )Window::GetInstance()->GetHandle() ) ) );

	HGLRC tempRenderingContext = CreateOldRenderContext();
	wglMakeCurrent( *g_deviceDisplayContext, tempRenderingContext );

	BindNewWGLFunctions( g_glLibrary );

	HGLRC realRenderingContext = CreateRealRenderContext( 4, 2 );

	// Set and cleanup
	wglMakeCurrent( *g_deviceDisplayContext, realRenderingContext ); 
	wglDeleteContext( tempRenderingContext ); 

	// Bind all our OpenGL functions we'll be using.
	BindGLFunctions( g_glLibrary ); 

	g_glContext = realRenderingContext;

	return true; 
}

void Renderer::InitializeRenderer()
{
	glLineWidth( 1.5f );
	glEnable( GL_BLEND );
	glEnable( GL_LINE_SMOOTH );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
}

void Renderer::PostStartup()
{
	// Bind default Input Layout
	glGenVertexArrays( 1, ( GLuint* ) &m_defaultVAO ); 
	glBindVertexArray( m_defaultVAO );
	
	// Instantiate default targets and camera
	unsigned int width = ( unsigned int ) Window::GetWidth();
	unsigned int height = ( unsigned int ) Window::GetHeight();
	m_defaultColorTarget = CreateRenderTarget( width, height );
	m_defaultDepthStencilTarget = CreateRenderTarget( width, height, TextureFormat::TEXTURE_FORMAT_D24S8 );
	m_defaultCamera = new Camera( m_defaultColorTarget, m_defaultDepthStencilTarget );
	SetCamera( m_defaultCamera );

	// Instantiate default sampler, texture and meshes
	SamplerOptions options;
	options.m_sampleMode = SamplerSampleMode::SAMPLER_LINEAR;
	m_defaultSampler = new Sampler( options );
	SamplerOptions mipMapSamplerOptions;
	mipMapSamplerOptions.m_wrapMode = SamplerWrapMode::SAMPLER_WRAP_WRAP;
	mipMapSamplerOptions.m_sampleMode = SamplerSampleMode::SAMPLER_LINEAR_MIPMAP_LINEAR;
	m_defaultSamplerMipMap = new Sampler( mipMapSamplerOptions );

	CreateOrGetShaderProgram( INVALID_SHADER_NAME );	// Create the invalid shader program; this absolutely needs to work correctly, or things will not end well!
	CreateOrGetShaderProgram( DEFAULT_SHADER_NAME, "USE_LIGHTING;HAS_BONE_WEIGHTS;MAX_LIGHTS=8;USE_FOG" );	// Create the default shader program with #defines included
	std::string defaultTexturePath = std::string( TEXTURES_RELATIVE_PATH ) + std::string( DEFAULT_TEXTURE_NAME );
	m_defaultTexture = CreateOrGetTexture( defaultTexturePath, 1, false );
	m_defaultTextureCube = new TextureCube();
	m_defaultTextureCube->MakeFromImage( defaultTexturePath.c_str(), false );
	m_immediateMesh = new Mesh();

	// Instantiate Effects target and camera
	m_effectsSwapTarget = CreateRenderTarget( width, height );
	m_effectsCamera = new Camera( m_effectsSwapTarget );

	// Instantiate default Shader pass
	m_defaultShaderPass = new ShaderPass( nullptr );

	// Instantiate Active Lights UBO
	m_activeLightsUBO = new UniformBuffer();

	LoadShadersFromFile();
	LoadMaterialsFromFile();

	glEnable( GL_TEXTURE_CUBE_MAP_SEAMLESS );

	ForwardRenderingPath::Startup();
}

void Renderer::LoadShadersFromFile()
{
	tinyxml2::XMLDocument xmlDoc;
	tinyxml2::XMLError loadSuccess = xmlDoc.LoadFile( SHADERS_XML_FILEPATH );
	if ( loadSuccess == tinyxml2::XMLError::XML_SUCCESS )
	{
		tinyxml2::XMLElement* rootElement = xmlDoc.FirstChildElement();
		for ( const tinyxml2::XMLElement* shaderElement = rootElement->FirstChildElement(); shaderElement != nullptr; shaderElement = shaderElement->NextSiblingElement() )
		{
			Shader* newShader = new Shader( *shaderElement );
			m_loadedShadersFromFileByName[ newShader->GetName() ] = newShader;
		}
	}
}

void Renderer::LoadMaterialsFromFile()
{
	tinyxml2::XMLDocument xmlDoc;
	tinyxml2::XMLError loadSuccess = xmlDoc.LoadFile( MATERIALS_XML_FILEPATH );
	if ( loadSuccess == tinyxml2::XMLError::XML_SUCCESS )
	{
		tinyxml2::XMLElement* rootElement = xmlDoc.FirstChildElement();
		for ( const tinyxml2::XMLElement* materialElement = rootElement->FirstChildElement(); materialElement != nullptr; materialElement = materialElement->NextSiblingElement() )
		{
			Material* newMaterial = new Material( *materialElement );
			m_loadedMaterialsFromFileByName[ ParseXmlAttribute( *materialElement, "name", "" ) ] = newMaterial;
		}
	}
}

#pragma endregion

void Renderer::BeginFrame( const float screenWidth, const float screenHeight )
{
	UNUSED( screenWidth );
	UNUSED( screenHeight );
	m_takeScreenshotThisFrame = false;
	UseShaderProgram( DEFAULT_SHADER_NAME );	// Set the current shader program to the default shader program
	UseDefaultCamera();
	UpdateTimeUBOWithFrameTime();
	ProcessTextureLoadQueue();
}

void Renderer::EndFrame() const
{
	HWND appWindow = *( reinterpret_cast< HWND* >( Window::GetInstance()->GetHandle() ) );
	HDC displayContext = GetDC( appWindow );
	if ( m_takeScreenshotThisFrame )
	{
		SaveScreenshot();
	}

	CopyFrameBuffer( nullptr, m_currentCamera->GetFrameBuffer() );
	SwapBuffers( displayContext );
}

void Renderer::UpdateTimeUBOWithFrameTime()
{
	float currentTime = GetMasterClockTimeElapsedSecondsF();
	float frameTime = GetMasterClockFrameTimeSecondsF();
	int frameCount = GetMasterClockFrameCount();

	SetTimeUBO( currentTime, frameTime, frameCount );
}

bool Renderer::CopyFrameBuffer( FrameBuffer* destination, FrameBuffer* source ) const
{
	if ( source == nullptr )
	{
		return false; 
	}

	source->MarkDirty();

	GLuint sourceFBO = source->GetHandle();
	GLuint destinationFBO = NULL; 
	if ( destination != nullptr )
	{
		destinationFBO = destination->GetHandle(); 
		destination->MarkDirty();
	}

	if ( destinationFBO == sourceFBO ) {
		return false; 
	}

	glBindFramebuffer( GL_READ_FRAMEBUFFER, sourceFBO );  

	glBindFramebuffer( GL_DRAW_FRAMEBUFFER, destinationFBO ); 

	GLint width = ( GLint )source->GetWidth();     
	GLint height = ( GLint )source->GetHeight();

	if ( destination == nullptr )
	{
		glReadBuffer( GL_COLOR_ATTACHMENT0 );
		glDrawBuffer( GL_BACK );
		glBlitFramebuffer( 0, 0,
			width, height,
			0, 0,
			width, height,
			GL_COLOR_BUFFER_BIT,
			GL_NEAREST );
	}
	else
	{
		// Account for multiple source and destination color targets
		for ( unsigned int bufferIndex = 0U; bufferIndex < source->GetColorTargetCount(); bufferIndex++ )
		{
			glReadBuffer( GL_COLOR_ATTACHMENT0 + bufferIndex );
			glDrawBuffer( GL_COLOR_ATTACHMENT0 + bufferIndex );
			glBlitFramebuffer( 0, 0,
				width, height,
				0, 0,
				width, height,
				GL_COLOR_BUFFER_BIT,
				GL_NEAREST );
		}
	}

	glBindFramebuffer( GL_READ_FRAMEBUFFER, NULL ); 
	glBindFramebuffer( GL_DRAW_FRAMEBUFFER, NULL );

	return GLSucceeded();
}

void Renderer::AddToTextureLoadQueue( TextureAsyncLoadRequest* loadInfo )
{
	m_textureLoadQueue.Enqueue( loadInfo );
}

void Renderer::AddToTextureCubeLoadQueue( TextureCubeAsyncLoadRequest* loadInfo )
{
	m_textureCubeLoadQueue.Enqueue( loadInfo );
}

void Renderer::ProcessTextureLoadQueue()
{
	TextureAsyncLoadRequest* texInfo = nullptr;
	while ( m_textureLoadQueue.Dequeue( &texInfo ) )
	{
		texInfo->m_texture->CreateFromImage( texInfo->m_image, texInfo->m_mipLevels );
		delete texInfo;
		texInfo = nullptr;
	}

	TextureCubeAsyncLoadRequest* texCubeInfo = nullptr;
	while ( m_textureCubeLoadQueue.Dequeue( &texCubeInfo ) )
	{
		texCubeInfo->m_textureCube->MakeFromImage( texCubeInfo->m_image );
		delete texCubeInfo;
		texCubeInfo = nullptr;
	}
}

BitmapFont* Renderer::CreateOrGetBitmapFont( const char* bitmapFontName )
{
	std::string bitmapFontNameAsString = std::string( bitmapFontName );

	std::string fullBitmapFontFilePath = BITMAP_FONT_PATH + std::string( bitmapFontName ) + BITMAP_FONT_EXTENSION;
	SpriteSheet* spriteSheetForFont = CreateOrGetTextureAtlasForBitmapFont( fullBitmapFontFilePath );

	if ( m_loadedFontsByName.find( bitmapFontNameAsString ) == m_loadedFontsByName.end() )
	{
		m_loadedFontsByName[ bitmapFontNameAsString ] = new BitmapFont( *spriteSheetForFont, 1.0f );		// TODO: Revise base aspect if needed
		if ( m_defaultBitmapFont == nullptr )
		{
			m_defaultBitmapFont = m_loadedFontsByName[ bitmapFontNameAsString ];
		}
	}

	return m_loadedFontsByName[ bitmapFontNameAsString ];
}

SpriteSheet* Renderer::CreateOrGetTextureAtlasForBitmapFont( const std::string& spriteSheetFilePath )
{
	Texture* fontTexture = CreateOrGetTexture( spriteSheetFilePath );

	if ( m_loadedFontSpriteSheetsByName.find( spriteSheetFilePath ) == m_loadedFontSpriteSheetsByName.end() )
	{
		m_loadedFontSpriteSheetsByName[ spriteSheetFilePath ] = new SpriteSheet( *fontTexture, BITMAP_FONT_SPRITESHEET_WIDTH_TILES, BITMAP_FONT_SPRITESHEET_HEIGHT_TILES );
	}

	return m_loadedFontSpriteSheetsByName[ spriteSheetFilePath ];
}

Texture* Renderer::CreateOrGetTexture( const std::string& textureFilePath, unsigned int mipLevels /*= 1U*/, bool loadAsync /*= true*/ )
{
	if ( m_loadedTexturesByName.find( textureFilePath ) == m_loadedTexturesByName.end() )
	{
		m_loadedTexturesByName[ textureFilePath ] = new Texture( textureFilePath, mipLevels, loadAsync );
	}

	return m_loadedTexturesByName[ textureFilePath ];
}

Sprite* Renderer::CreateOrReplaceSprite( const std::string& spriteName, Texture* texture, AABB2 UVs, float PPU, Vector2 pivot /* = Vector2::ZERO */ )
{
	Sprite* newSprite = new Sprite( texture, UVs, PPU, pivot );
	if ( m_loadedSpritesByName.find( spriteName ) != m_loadedSpritesByName.end() && m_loadedSpritesByName[ spriteName ] != nullptr )
	{
		delete m_loadedSpritesByName[ spriteName ];
	}
	m_loadedSpritesByName[ spriteName ] = newSprite;
	return newSprite;
}

Sprite* Renderer::CreateOrReplaceSprite( const std::string& spriteName, const std::string& textureFilePath, AABB2 UVs, float PPU, Vector2 pivot /* = Vector2::ZERO */ )
{
	Sprite* newSprite = new Sprite( CreateOrGetTexture( textureFilePath ), UVs, PPU, pivot );
	if ( m_loadedSpritesByName.find( spriteName ) != m_loadedSpritesByName.end() && m_loadedSpritesByName[ spriteName ] != nullptr )
	{
		delete m_loadedSpritesByName[ spriteName ];
	}
	m_loadedSpritesByName[ spriteName ] = newSprite;
	return newSprite;
}

Sprite* Renderer::CreateOrReplaceSprite( const std::string& spriteName, const tinyxml2::XMLElement& spriteElement )
{
	Sprite* newSprite = new Sprite( spriteElement, *this );
	if ( m_loadedSpritesByName.find( spriteName ) != m_loadedSpritesByName.end() && m_loadedSpritesByName[ spriteName ] != nullptr )
	{
		delete m_loadedSpritesByName[ spriteName ];
	}
	m_loadedSpritesByName[ spriteName ] = newSprite;
	return newSprite;
}

Mesh* Renderer::CreateOrGetMesh( const std::string& meshPath )
{
	if ( m_loadedMeshesByName.find( meshPath ) == m_loadedMeshesByName.end() )
	{
		Mesh* meshArray = MeshBuilder::FromFileOBJ( meshPath )[ 0 ];
		m_loadedMeshesByName[ meshPath ] = meshArray;
	}
	
	return m_loadedMeshesByName[ meshPath ];
}

Texture* Renderer::CreateRenderTarget( unsigned int width, unsigned int height, TextureFormat format/* = TextureFormat::TEXTURE_FORMAT_RGBA8*/ )
{
	Texture* texture = new Texture();
	texture->CreateRenderTarget( width, height, format );
	return texture;
}

Texture* Renderer::CreateDepthStencilTarget( unsigned int width, unsigned int height )
{
	return CreateRenderTarget( width, height, TextureFormat::TEXTURE_FORMAT_D24S8 ); 
}

Texture* Renderer:: CreateMSAATarget( unsigned int width, unsigned int height, TextureFormat format, unsigned int numSamples )
{
	Texture* texture = new Texture();
	texture->CreateMultiSampleTarget( width, height, format, numSamples );
	return texture;
}

Texture* Renderer::GetDefaultTexture() const
{
	return m_defaultTexture;
}

TextureCube* Renderer::GetDefaultTextureCube() const
{
	return m_defaultTextureCube;
}

Sampler* Renderer::GetDefaultSampler( bool usesMipMaps /* = false */ ) const
{
	return ( usesMipMaps )? m_defaultSamplerMipMap : m_defaultSampler;
}

Texture* Renderer::GetDefaultColorTarget() const
{
	return m_defaultColorTarget;
}

Texture* Renderer::GetDefaultDepthStencilTarget() const
{
	return m_defaultDepthStencilTarget;
}

Camera* Renderer::GetCurrentCamera() const
{
	return m_currentCamera;
}

Vector3 Renderer::GetCurrentViewportUpVector() const
{
	return CrossProduct( m_currentCamera->GetForward(), m_currentCamera->GetRight() ).GetNormalized();
}

Material* Renderer::CreateOrGetMaterial( const std::string& materialName )
{
	if ( m_loadedMaterialsFromFileByName.find( materialName ) == m_loadedMaterialsFromFileByName.end() )
	{
		ConsolePrintf( Rgba::RED, "ERROR: Renderer::CreateOrGetMaterial() - Could not find material with name %s. Returning null.", materialName.c_str() );
		return nullptr;
	}

	return m_loadedMaterialsFromFileByName[ materialName ];
}

Shader* Renderer::CreateOrGetShader( const std::string& shaderName )
{
	if ( m_loadedShadersFromFileByName.find( shaderName ) == m_loadedShadersFromFileByName.end() )
	{
		Shader* shaderFromFile = Shader::AcquireResource( SHADERS_RELATIVE_PATH + shaderName );
		m_loadedShadersFromFileByName[ shaderFromFile->GetName() ] = shaderFromFile;
	}

	return m_loadedShadersFromFileByName[ shaderName ];
}

ShaderProgram* Renderer::CreateOrGetShaderProgram( const std::string& shaderName, const std::string& semiColonDelimitedDefines )
{
	if ( m_loadedShaderProgramsByName.find( shaderName ) == m_loadedShaderProgramsByName.end() )
	{
		ShaderProgram* newShaderProgram = new ShaderProgram();

		// Try to load the provided file(s), and if it fails, load the invalid shader into the same object
		if ( !newShaderProgram->LoadFromFiles( ( std::string( SHADERS_RELATIVE_PATH ) + shaderName ).c_str(), semiColonDelimitedDefines ) )
		{
			// Assumes the Invalid shader is initialized before anything else and is not invalid itself
			newShaderProgram->LoadFromFiles(( std::string( SHADERS_RELATIVE_PATH ) + INVALID_SHADER_NAME  ).c_str(), "" );
		}
		m_loadedShaderProgramsByName[ shaderName ] = newShaderProgram;
		m_definesInjectedIntoShadersByName[ shaderName ] = semiColonDelimitedDefines;
	}

	return m_loadedShaderProgramsByName[ shaderName ];
}

ShaderProgram* Renderer::CreateOrGetShaderProgram( const char* shaderName, const char* semiColonDelimitedDefines )
{
	return CreateOrGetShaderProgram( std::string( shaderName ), std::string( semiColonDelimitedDefines ) );
}

ShaderProgram* Renderer::CreateOrGetShaderProgramFromRawStrings( const std::string& shaderName, const std::string& vertexShaderText, const std::string& fragmentShaderText, const std::string& semiColonDelimitedDefines )
{
	return CreateOrGetShaderProgramFromRawStrings( shaderName.c_str(), vertexShaderText.c_str(), fragmentShaderText.c_str(), semiColonDelimitedDefines.c_str() );
}

ShaderProgram* Renderer::CreateOrGetShaderProgramFromRawStrings( const char* shaderName, const char* vertexShaderText, const char* fragmentShaderText, const char* semiColonDelimitedDefines )
{
	if ( m_loadedShaderProgramsByName.find( shaderName ) == m_loadedShaderProgramsByName.end() )
	{
		ShaderProgram* newShaderProgram = new ShaderProgram();

		// Try to load the provided file(s), and if it fails, load the invalid shader into the same object
		if ( !newShaderProgram->LoadFromStringLiterals( vertexShaderText, fragmentShaderText, semiColonDelimitedDefines ) )
		{
			// Assumes the Invalid shader is initialized before anything else and is not invalid itself
			newShaderProgram->LoadFromFiles( ( std::string( SHADERS_RELATIVE_PATH ) + INVALID_SHADER_NAME  ).c_str(), "" );
		}
		m_loadedShaderProgramsByName[ shaderName ] = newShaderProgram;
		m_namesOfShadersLoadedFromStrings.push_back( shaderName );
		m_definesInjectedIntoShadersByName[ shaderName ] = semiColonDelimitedDefines;
	}

	return m_loadedShaderProgramsByName[ shaderName ];
}

void Renderer::ReloadAllFileShaders()		// Only reload shaders instantiated from files; raw string shaders cannot be hot loaded as they are present in C++ code
{
	ReloadShadersFromFile();
	ReloadMaterialsFromFile();

	for ( std::map< std::string, ShaderProgram* >::iterator mapIterator = m_loadedShaderProgramsByName.begin(); mapIterator != m_loadedShaderProgramsByName.end(); mapIterator++ )
	{
		if ( std::find( m_namesOfShadersLoadedFromStrings.begin(), m_namesOfShadersLoadedFromStrings.end(), mapIterator->first ) == m_namesOfShadersLoadedFromStrings.end() )
		{
			if ( !mapIterator->second->LoadFromFiles( ( std::string( SHADERS_RELATIVE_PATH ) + mapIterator->first ).c_str(), m_definesInjectedIntoShadersByName[ mapIterator->first ] ) )
			{
				// Assumes the Invalid shader is initialized before anything else and is not invalid itself
				mapIterator->second->LoadFromFiles(( std::string( SHADERS_RELATIVE_PATH ) + INVALID_SHADER_NAME  ).c_str(), "" );
			}
		}
	}
}

void Renderer::ReloadShadersFromFile()
{
	tinyxml2::XMLDocument xmlDoc;
	tinyxml2::XMLError loadSuccess = xmlDoc.LoadFile( SHADERS_XML_FILEPATH );
	if ( loadSuccess == tinyxml2::XMLError::XML_SUCCESS )
	{
		tinyxml2::XMLElement* rootElement = xmlDoc.FirstChildElement();
		for ( const tinyxml2::XMLElement* shaderElement = rootElement->FirstChildElement(); shaderElement != nullptr; shaderElement = shaderElement->NextSiblingElement() )
		{
			std::string shaderName = ParseXmlAttribute( *shaderElement, "name", "" );
			if ( m_loadedShadersFromFileByName.find( shaderName ) != m_loadedShadersFromFileByName.end() )
			{
				m_loadedShadersFromFileByName[ shaderName ]->LoadFromXML( *shaderElement );
			}
			else
			{
				ERROR_RECOVERABLE( "WARNING: Renderer::ReloadShadersFromFile() - Loading new Shaders on the fly not supported. New Shader will be skipped." )
			}
		}
	}
}

void Renderer::ReloadMaterialsFromFile()
{
	tinyxml2::XMLDocument xmlDoc;
	tinyxml2::XMLError loadSuccess = xmlDoc.LoadFile( MATERIALS_XML_FILEPATH );
	if ( loadSuccess == tinyxml2::XMLError::XML_SUCCESS )
	{
		tinyxml2::XMLElement* rootElement = xmlDoc.FirstChildElement();
		for ( const tinyxml2::XMLElement* materialElement = rootElement->FirstChildElement(); materialElement != nullptr; materialElement = materialElement->NextSiblingElement() )
		{
			std::string materialName = ParseXmlAttribute( *materialElement, "name", "" );
			if ( m_loadedMaterialsFromFileByName.find( materialName ) != m_loadedMaterialsFromFileByName.end() )
			{
				m_loadedMaterialsFromFileByName[ materialName ]->LoadFromXML( *materialElement, true );
			}
			else
			{
				ERROR_RECOVERABLE( "WARNING: Renderer::ReloadMaterialsFromFile() - Loading new Materials on the fly not supported. New Material will be skipped." )
			}
		}
	}
}

void Renderer::BindRenderState( const RenderState& renderState ) const
{
	// Blend settings
	glEnable( GL_BLEND );
	glBlendEquationSeparate( GetGLBlendOperation( renderState.m_colorBlendOperation ), GetGLBlendOperation( renderState.m_alphaBlendOperation ) );
	glBlendFuncSeparate( GetGLBlendFactor( renderState.m_colorSourceFactor ), GetGLBlendFactor( renderState.m_colorDestinationFactor ), GetGLBlendFactor( renderState.m_alphaSourceFactor ), GetGLBlendFactor( renderState.m_alphaDestinationFactor ) );

	// Depth settings
	glEnable( GL_DEPTH_TEST ); 
	glDepthFunc( GetGLCompare( renderState.m_depthCompare ) ); 
	glDepthMask( renderState.m_depthWrite ? GL_TRUE : GL_FALSE );

	// Fill settings
	glPolygonMode( GL_FRONT_AND_BACK, GetGLPolygonMode( renderState.m_fillMode ) );

	// Cull settings
	if ( renderState.m_cullMode == RendererCullMode::CULL_MODE_NONE )
	{
		glDisable( GL_CULL_FACE );
	}
	else
	{
		glEnable( GL_CULL_FACE );
		glCullFace( GetGLCullMode( renderState.m_cullMode ) );
	}

	// Winding Order settings
	glFrontFace( GetGLWindOrder( renderState.m_frontFace ) );
}

void Renderer::SetShaderPass( ShaderPass* shader )
{
	m_activeShaderPass = shader;
	UseActiveShaderProgram();
	BindRenderState( shader->m_state );
}

void Renderer::SetShaderPass( const std::string& loadedShaderName, unsigned int passIndex /* = 0U */ )
{
	SetShaderPass( m_loadedShadersFromFileByName[ loadedShaderName ]->GetPass( passIndex ) );
}

void Renderer::UseActiveShaderProgram() const
{
	if ( m_activeShaderPass->m_program != nullptr )
	{
		glUseProgram( m_activeShaderPass->m_program->GetHandle() );
	}
	else
	{
		ConsolePrintf( Rgba::RED, "ERROR: Renderer::UseActiveShaderProgram() - Trying to bind a null shader program." );
	}
}

void Renderer::UseShaderProgram( ShaderProgram* shaderProgram )
{
	ShaderProgram* programToUse = shaderProgram;
	if ( shaderProgram == nullptr )
	{
		programToUse = m_loadedShaderProgramsByName[ std::string( DEFAULT_SHADER_NAME ) ];
	}
	m_defaultShaderPass->SetShaderProgram( programToUse );
	glUseProgram( m_defaultShaderPass->m_program->GetHandle() );
	m_activeShaderPass = m_defaultShaderPass;
}

void Renderer::UseShaderProgram( const std::string& shaderName )
{
	if ( m_loadedShaderProgramsByName.find( shaderName ) == m_loadedShaderProgramsByName.end() )
	{
		DebuggerPrintf( std::string( "Renderer::UseShaderProgram: Invalid shader name provided: " + shaderName + ". Using built-in default shader." ).c_str() );
		m_defaultShaderPass->SetShaderProgram( m_loadedShaderProgramsByName[ std::string( DEFAULT_SHADER_NAME ) ] );
	}
	else
	{
		m_defaultShaderPass->SetShaderProgram( m_loadedShaderProgramsByName[ shaderName ] );
	}
	glUseProgram( m_defaultShaderPass->m_program->GetHandle() );
	m_activeShaderPass = m_defaultShaderPass;
}

void Renderer::BindAttribToCurrentShader( const char* shaderAttribName, float attribValue )
{
	GLuint programHandle = m_activeShaderPass->m_program->GetHandle();
	GLint attribBind = glGetUniformLocation( programHandle, shaderAttribName );

	if ( attribBind >= 0 )
	{
		glUniform1f( attribBind, attribValue );
	}
}

void Renderer::BindAttribToCurrentShader( const ShaderFloatUniform& shaderAttrib )
{
	GLuint programHandle = m_activeShaderPass->m_program->GetHandle();
	GLint attribBind = glGetUniformLocation( programHandle, shaderAttrib.m_name.c_str() );

	if ( attribBind >= 0 )
	{
		glUniform1f( attribBind, shaderAttrib.m_value );
	}
}

void Renderer::BindVector3ToCurrentShader( const char* shaderUniformName, const Vector3& vec3 )
{
	GLuint programHandle = m_activeShaderPass->m_program->GetHandle();
	GLint attribBind = glGetUniformLocation( programHandle, shaderUniformName );

	if ( attribBind >= 0 )
	{
		glUniform3f( attribBind, vec3.x, vec3.y, vec3.z );
	}
}

void Renderer::BindVector4ToCurrentShader( const char* shaderUniformName, const Vector4& vec4 )
{
	GLuint programHandle = m_activeShaderPass->m_program->GetHandle();
	GLint attribBind = glGetUniformLocation( programHandle, shaderUniformName );

	if ( attribBind >= 0 )
	{
		glUniform4f( attribBind, vec4.x, vec4.y, vec4.z, vec4.w );
	}
}

void Renderer::BindRgbaToCurrentShader( const char* shaderUniformName, const Rgba& color )
{
	GLuint programHandle = m_activeShaderPass->m_program->GetHandle();
	GLint attribBind = glGetUniformLocation( programHandle, shaderUniformName );

	if ( attribBind >= 0 )
	{
		Vector4 rgbaFloat;
		color.GetAsFloats( rgbaFloat.x, rgbaFloat.y, rgbaFloat.z, rgbaFloat.w );
		glUniform4f( attribBind, rgbaFloat.x, rgbaFloat.y, rgbaFloat.z, rgbaFloat.w );
	}
}

void Renderer::BindMatrixToCurrentShader( const char* shaderUniformName, const Matrix44* matrix )
{
	GLuint programHandle = m_activeShaderPass->m_program->GetHandle();
	GLint matrixBind = glGetUniformLocation( programHandle, shaderUniformName );

	if ( matrixBind >= 0 )
	{
		glUniformMatrix4fv( matrixBind, 1, GL_FALSE, ( GLfloat* )matrix );
	}
}

void Renderer::BindModelMatrixToCurrentShader( const Matrix44& matrix )
{
	if ( m_modelMatrixUBO == nullptr )
	{
		m_modelMatrixUBO = new UniformBuffer();
	}
	m_modelMatrixUBO->SetDataAndCopyToGPU< Matrix44 >( matrix );
	glBindBufferBase( GL_UNIFORM_BUFFER, UNIFORM_BUFFER_MODEL_MATRIX_BIND, m_modelMatrixUBO->m_handle );
}

void Renderer::BindMaterial( Material& material, unsigned int passIndex /* = 0U */ )
{
	SetShaderPass( material.GetShader()->GetPass( passIndex ) );
	BindRenderState( material.GetShader()->GetPass( passIndex )->m_state );
	SetSpecularProperties( material.GetSpecularAmount(), material.GetSpecularPower() );

	for ( unsigned int slotIndex = 0; slotIndex < material.GetTextureCount(); slotIndex++ )
	{
		if ( material.GetTexture( slotIndex ) != nullptr )
		{
			BindTextureAndSampler( slotIndex, *material.GetTexture( slotIndex ), *material.GetSampler( slotIndex ) );
		}
	}

	material.BindPropertyBlocks( passIndex );
}

void Renderer::BindInputLayout( Renderable* renderable )
{
	renderable->GetInputLayout()->UpdateBinding();
}

void Renderer::SetAmbientLight( const Vector4& lightColorAndIntensity )
{
	m_activeLightsStruct.m_ambientLightColorAndIntensity = lightColorAndIntensity;
	m_activeLightsUBO->SetDataAndCopyToGPU< ActiveLightBufferUBO >( m_activeLightsStruct );
	glBindBufferBase( GL_UNIFORM_BUFFER, UNIFORM_BUFFER_ACTIVE_LIGHTS_BIND, m_activeLightsUBO->m_handle );
}

void Renderer::SetAmbientLight( const Rgba& light, float intensity /* = 1.0f */ )
{
	Vector4 lightF = light.GetAsFloats();
	lightF.w = intensity;

	m_activeLightsStruct.m_ambientLightColorAndIntensity = lightF;
	m_activeLightsUBO->SetDataAndCopyToGPU< ActiveLightBufferUBO >( m_activeLightsStruct );
	glBindBufferBase( GL_UNIFORM_BUFFER, UNIFORM_BUFFER_ACTIVE_LIGHTS_BIND, m_activeLightsUBO->m_handle );
}

void Renderer::BindLights( const LightUBO lights[ MAX_LIGHTS ] )
{
	CopyLights( lights, m_activeLightsStruct.m_lights, MAX_LIGHTS );
	m_activeLightsUBO->SetDataAndCopyToGPU< ActiveLightBufferUBO >( m_activeLightsStruct );
	glBindBufferBase( GL_UNIFORM_BUFFER, UNIFORM_BUFFER_ACTIVE_LIGHTS_BIND, m_activeLightsUBO->m_handle );
}

void Renderer::SetSpecularProperties( float specularAmount, float specularPower )
{
	if ( m_objectLightPropertiesUBO == nullptr )
	{
		m_objectLightPropertiesUBO = new UniformBuffer();
	}
	m_objectLightPropertiesStruct.m_specularAmount = specularAmount;
	m_objectLightPropertiesStruct.m_specularPower = specularPower;
	m_objectLightPropertiesUBO->SetDataAndCopyToGPU< ObjectLightPropertiesUBO >( m_objectLightPropertiesStruct );
	glBindBufferBase( GL_UNIFORM_BUFFER, UNIFORM_BUFFER_OBJECT_LIGHT_PROPERTIES_BIND, m_objectLightPropertiesUBO->m_handle );
}

void Renderer::SetTimeUBO( const TimeUBO& time )
{
	if ( m_frameTimeUBO == nullptr )
	{
		m_frameTimeUBO = new UniformBuffer();
	}
	m_frameTimeUBO->SetDataAndCopyToGPU< TimeUBO >( time );
	glBindBufferBase( GL_UNIFORM_BUFFER, UNIFORM_BUFFER_TIME_BIND, m_frameTimeUBO->m_handle );
}

void Renderer::SetTimeUBO( float currentTime, float frameTime, unsigned int frameCount )
{
	TimeUBO time = TimeUBO( currentTime, frameTime, frameCount );
	SetTimeUBO( time );
}

void Renderer::SetFog( const FogUBO& fog )
{
	if ( m_fogUBO == nullptr )
	{
		m_fogUBO = new UniformBuffer();
	}
	m_fogUBO->SetDataAndCopyToGPU< FogUBO >( fog );
	glBindBufferBase( GL_UNIFORM_BUFFER, UNIFORM_BUFFER_FOG_SETTINGS_BIND, m_fogUBO->m_handle );
}

void Renderer::SetFog( const Vector4& fogColor, float fogNearFactor, float fogFarFactor, float fogNearZ, float fogFarZ )
{
	FogUBO fog = FogUBO( fogColor, fogNearFactor, fogFarFactor, fogNearZ, fogFarZ );
	SetFog( fog );
}

int Renderer::GetGLDrawPrimitive( const DrawPrimitiveType primitiveType ) const
{
	switch ( primitiveType )
	{
		case DrawPrimitiveType::LINES		:	return GL_LINES;
		case DrawPrimitiveType::LINE_LOOP	:	return GL_LINE_LOOP;
		case DrawPrimitiveType::TRIANGLES	:	return GL_TRIANGLES;
		case DrawPrimitiveType::QUADS		:	return GL_QUADS;
		default								:	return GL_LINES;
	}
	return 0;
}

int Renderer::GetGLDataType( const RendererDataType dataType ) const
{
	switch( dataType )
	{
		case RendererDataType::RENDER_TYPE_FLOAT		:	return GL_FLOAT;
		case RendererDataType::RENDER_TYPE_UNSIGNED_INT	:	return GL_UNSIGNED_BYTE;
	}
	return 0;
}

int Renderer::GetGLCullMode( const RendererCullMode cullMode ) const
{
	switch( cullMode )
	{
		case RendererCullMode::CULL_MODE_FRONT				:	return GL_FRONT;
		case RendererCullMode::CULL_MODE_BACK				:	return GL_BACK;
		case RendererCullMode::CULL_MODE_FRONT_AND_BACK		:	return GL_FRONT_AND_BACK;
		case RendererCullMode::CULL_MODE_NONE				:	return GL_NONE;
		default												:	return GL_FRONT_AND_BACK;
	}
}

int Renderer::GetGLPolygonMode( const RendererPolygonMode polygonMode ) const
{
	switch( polygonMode )
	{
		case RendererPolygonMode::POLYGON_MODE_FILL		:	return GL_FILL;
		case RendererPolygonMode::POLYGON_MODE_LINE		:	return GL_LINE;
		default											:	return GL_FILL;
	}
}

int Renderer::GetGLWindOrder( const RendererWindOrder windOrder ) const
{
	switch( windOrder )
	{
		case RendererWindOrder::WIND_CLOCKWISE			:	return GL_CW;
		case RendererWindOrder::WIND_COUNTER_CLOCKWISE	:	return GL_CCW;
		default											:	return GL_CCW;
	}
}

int Renderer::GetGLBlendOperation( const RendererBlendOperation blendOperation ) const
{
	switch( blendOperation )
	{
		case RendererBlendOperation::OPERATION_ADD				:	return GL_FUNC_ADD;
		case RendererBlendOperation::OPERATION_SUBTRACT			:	return GL_FUNC_SUBTRACT;
		case RendererBlendOperation::OPERATION_REVERSE_SUBRACT	:	return GL_FUNC_REVERSE_SUBTRACT;
		case RendererBlendOperation::OPERATION_MIN				:	return GL_MIN;
		case RendererBlendOperation::OPERATION_MAX				:	return GL_MAX;
		default													:	return GL_FUNC_ADD;	
	}
}

int Renderer::GetGLBlendFactor( const RendererBlendFactor blendFactor ) const
{
	switch( blendFactor )
	{
		case RendererBlendFactor::FACTOR_ZERO					:	return GL_ZERO;
		case RendererBlendFactor::FACTOR_ONE					:	return GL_ONE;
		case RendererBlendFactor::FACTOR_SRC_COLOR				:	return GL_SRC_COLOR;
		case RendererBlendFactor::FACTOR_ONE_MINUS_SRC_COLOR	:	return GL_ONE_MINUS_SRC_COLOR;
		case RendererBlendFactor::FACTOR_DST_COLOR				:	return GL_DST_COLOR;
		case RendererBlendFactor::FACTOR_ONE_MINUS_DST_COLOR	:	return GL_ONE_MINUS_DST_COLOR;
		case RendererBlendFactor::FACTOR_SRC_ALPHA				:	return GL_SRC_ALPHA;
		case RendererBlendFactor::FACTOR_ONE_MINUS_SRC_ALPHA	:	return GL_ONE_MINUS_SRC_ALPHA;
		case RendererBlendFactor::FACTOR_DST_ALPHA				:	return GL_DST_ALPHA;
		case RendererBlendFactor::FACTOR_ONE_MINUS_DST_ALPHA	:	return GL_ONE_MINUS_DST_ALPHA;
		default													:	return GL_ONE;
	}
}

int Renderer::GetGLCubeSide( const TextureCubeSide cubeSide ) const
{
	switch ( cubeSide )
	{
		case TextureCubeSide::TEXCUBE_RIGHT		:	return GL_TEXTURE_CUBE_MAP_POSITIVE_X;
		case TextureCubeSide::TEXCUBE_LEFT		:	return GL_TEXTURE_CUBE_MAP_NEGATIVE_X;
		case TextureCubeSide::TEXCUBE_TOP		:	return GL_TEXTURE_CUBE_MAP_POSITIVE_Y;
		case TextureCubeSide::TEXCUBE_BOTTOM	:	return GL_TEXTURE_CUBE_MAP_NEGATIVE_Y;
		case TextureCubeSide::TEXCUBE_BACK		:	return GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
		case TextureCubeSide::TEXCUBE_FRONT		:	return GL_TEXTURE_CUBE_MAP_POSITIVE_Z;
		default									:	return GL_TEXTURE_CUBE_MAP_NEGATIVE_X;
	}
}

int Renderer::GetGLTextureType( const TextureType textureType ) const
{
	switch ( textureType )
	{
		case TextureType::TEXTURE_TYPE_2D				:	return GL_TEXTURE_2D;
		case TextureType::TEXTURE_TYPE_2D_MULTISAMPLE	:	return GL_TEXTURE_2D_MULTISAMPLE;
		case TextureType::TEXTURE_TYPE_CUBEMAP			:	return GL_TEXTURE_CUBE_MAP;
		default											:	return GL_TEXTURE_2D;
	}
}

GLenum Renderer::GetGLCompare( DepthTestCompare compare ) const
{
	switch( compare )
	{
		case DepthTestCompare::COMPARE_NEVER		:	return GL_NEVER;
		case DepthTestCompare::COMPARE_LESS			:	return GL_LESS;
		case DepthTestCompare::COMPARE_LEQUAL		:	return GL_LEQUAL;
		case DepthTestCompare::COMPARE_GREATER		:	return GL_GREATER;
		case DepthTestCompare::COMPARE_GEQUAL		:	return GL_GEQUAL;
		case DepthTestCompare::COMPARE_EQUAL		:	return GL_EQUAL;
		case DepthTestCompare::COMPARE_NOT_EQUAL	:	return GL_NOTEQUAL;
		case DepthTestCompare::COMPARE_ALWAYS		:	return GL_ALWAYS;
		default										:	return GL_NEVER;
	}
}

Sprite* Renderer::GetSprite( const std::string& spriteName )
{
	return m_loadedSpritesByName[ spriteName ];
}

void Renderer::BindTextureAndSampler( unsigned int slotIndex, const Texture& texture, const Sampler& sampler ) const
{
	glActiveTexture( GL_TEXTURE0 + slotIndex );
	glBindTexture( GetGLTextureType( texture.GetType() ), texture.m_textureID );
	glBindSampler( slotIndex, sampler.GetHandle() );
}

void Renderer::BindFont( unsigned int slotIndex, const BitmapFont* bitmapFont ) const
{
	BindTextureAndSampler( slotIndex, *bitmapFont->m_spriteSheet.GetTexture(), *GetDefaultSampler() );
}

void Renderer::EnableTexturing( const Texture& texture, const Sampler& sampler ) const
{
	BindTextureAndSampler( 0, texture, sampler );
}

void Renderer::DisableTexturing() const		// Sets the default texture (pure white) as the current bound texture
{
	BindTextureAndSampler( 0, *m_defaultTexture, *m_defaultSampler );
}

void Renderer::WriteDepthImmediate( bool shouldWrite )
{
	glEnable( GL_DEPTH_TEST );
	glDepthMask( shouldWrite? GL_TRUE : GL_FALSE );
}

void Renderer::EnableDepth( DepthTestCompare compare, bool shouldWrite )
{
	m_defaultShaderPass->SetDepth( compare, shouldWrite );
}

void Renderer::DisableDepth()
{
	m_defaultShaderPass->DisableDepth();
}

void Renderer::EnableSRGB()
{
	glEnable( GL_FRAMEBUFFER_SRGB );
}

void Renderer::DisableSRGB()
{
	glDisable( GL_FRAMEBUFFER_SRGB );
}

void Renderer::ClearColor( const Rgba& clearColor/* = Rgba::BLACK */ ) const
{
	glClearColor( clearColor.r, clearColor.g, clearColor.b, clearColor.a );
	glClear( GL_COLOR_BUFFER_BIT );
}

void Renderer::ClearDepth( float depth/* = 1.0f*/ ) const
{
	glClearDepthf( depth );
	glClear( GL_DEPTH_BUFFER_BIT ); 
}

void Renderer::ApplyEffect( ShaderProgram* effectShader, const std::map<const char *, float>& shaderParameters /* = std::map<const char *, float>() */ )
{
	if ( m_effectsSourceTarget == nullptr )
	{
		m_effectsSourceTarget = m_currentCamera->GetFrameBuffer()->m_colorTargets[ 0 ];
	}
	if ( m_effectsScratchTarget == nullptr )
	{
		m_effectsScratchTarget = m_effectsSwapTarget;
	}

	m_effectsCamera->SetColorTarget( m_effectsScratchTarget );

	UseShaderProgram( effectShader );
	DisableDepth();
	SetCamera( m_effectsCamera );

	for ( std::map<const char *, float>::const_iterator mapIterator = shaderParameters.begin(); mapIterator != shaderParameters.end(); mapIterator++ )
	{
		BindAttribToCurrentShader( mapIterator->first, mapIterator->second );
	}

	DrawTexturedAABB( AABB2::MINUS_ONE_TO_ONE, *m_effectsSourceTarget, Vector2::ZERO, Vector2::ONE, Rgba::WHITE );
	
	std::swap( m_effectsSourceTarget, m_effectsScratchTarget );
}

void Renderer::FinishEffects()
{

	if ( m_effectsSourceTarget == nullptr && m_effectsScratchTarget == m_effectsSwapTarget )
	{
		return;
	}

	if ( m_effectsSourceTarget != m_defaultColorTarget )
	{
		FrameBuffer* defaultTargetFrameBuffer = new FrameBuffer();		// Create a temporary FrameBuffer to copy the scratch target's contents onto the default color target
		defaultTargetFrameBuffer->SetColorTarget( m_defaultColorTarget );
		defaultTargetFrameBuffer->Finalize();
		CopyFrameBuffer( defaultTargetFrameBuffer, m_effectsCamera->GetFrameBuffer() );
		delete defaultTargetFrameBuffer;
	}
	m_effectsScratchTarget = m_effectsSwapTarget;
	m_effectsSourceTarget = nullptr;
}

void Renderer::ApplyEffect( Material* effectMaterial, Camera* useCamera /* = nullptr */, Texture* useScratchTarget /* = nullptr */ )
{
	for ( unsigned int passIndex = 0U; passIndex < effectMaterial->GetShader()->GetPassCount(); passIndex++ )
	{
		if ( m_effectsScratchTarget == nullptr )
		{
			m_effectsScratchTarget = useScratchTarget;
			if ( useScratchTarget == nullptr )
			{
				m_effectsScratchTarget = m_effectsSwapTarget;
			}
		}

		Camera* effectCamera = useCamera;
		if ( effectCamera == nullptr )
		{
			effectCamera = m_effectsCamera;
		}
		if ( m_effectsSourceTarget == nullptr )
		{
			m_effectsSourceTarget = effectCamera->GetFrameBuffer()->m_colorTargets[ 0 ];
		}

		effectMaterial->SetTexture( 0U, m_effectsSourceTarget );
		effectCamera->SetColorTarget( m_effectsScratchTarget );

		DisableDepth();
		SetCamera( effectCamera );

		DrawFullScreenImmediate( *effectMaterial, Rgba::WHITE, passIndex );

		std::swap( m_effectsSourceTarget, m_effectsScratchTarget );
	}
}

void Renderer::FinishEffects( Texture* originalSource )
{
	if ( m_effectsSourceTarget != originalSource )
	{
		FrameBuffer* defaultTargetFrameBuffer = new FrameBuffer();
		defaultTargetFrameBuffer->SetColorTarget( originalSource );
		defaultTargetFrameBuffer->Finalize();
		CopyFrameBuffer( defaultTargetFrameBuffer, m_currentCamera->GetFrameBuffer() );
		delete defaultTargetFrameBuffer;
	}

	m_effectsSourceTarget= nullptr;
	m_effectsScratchTarget = nullptr;
	return;
}

void Renderer::Draw( const DrawCall& drawCall )
{
	PROFILE_SCOPE( "Renderer::Draw()" );
	Renderable renderableFromDrawCall = Renderable( drawCall.m_mesh, drawCall.m_material, false, false, drawCall.m_modelMatrix );
	DrawRenderable( renderableFromDrawCall, drawCall.m_passIndex );
}

void Renderer::DrawMesh( const Mesh* mesh ) const
{
	UseActiveShaderProgram();
	BindRenderState( m_activeShaderPass->m_state );
	BindMeshToShaderProgram( mesh, m_activeShaderPass->m_program );
	// Now that it is described and bound, draw using our program
	if ( mesh->m_drawInstruction.m_usesIndices )
	{
		glDrawElements( GetGLDrawPrimitive( mesh->m_drawInstruction.m_primitive ), mesh->GetIndexCount(), GL_UNSIGNED_INT, 0 );
	}
	else
	{
		glDrawArrays( GetGLDrawPrimitive( mesh->m_drawInstruction.m_primitive ), 0, mesh->GetVertexCount() );
	}
}

void Renderer::DrawMeshWithMaterial( const Mesh* mesh, Material& material )
{
	BindMaterial( material );
	DrawMesh( mesh );
}

void Renderer::DrawRenderable( Renderable& renderable, unsigned int passIndex /* = 0U */ )
{
	BindMaterial( *renderable.GetMaterial(), passIndex );
	BindModelMatrixToCurrentShader( renderable.GetModelMatrix() );
	
	renderable.GetInputLayout()->UpdateBinding();

	// Now that it is described and bound, draw using our program
	if ( renderable.GetMesh()->m_drawInstruction.m_usesIndices )
	{
		glDrawElements( GetGLDrawPrimitive( renderable.GetMesh()->m_drawInstruction.m_primitive ), renderable.GetMesh()->GetIndexCount(), GL_UNSIGNED_INT, 0 );
	}
	else
	{
		glDrawArrays( GetGLDrawPrimitive( renderable.GetMesh()->m_drawInstruction.m_primitive ), 0, renderable.GetMesh()->GetVertexCount() );
	}
}

void Renderer::DrawMeshImmediate( const Vertex_3DPCU* verts, int numVerts, const unsigned int* indices, int numIndices, DrawPrimitiveType drawPrimitive ) const
{	
	bool usesIndices = numIndices > 0;
	unsigned int elementCount = ( usesIndices )? static_cast< unsigned int >( numIndices ) : static_cast< unsigned int >( numVerts );
	m_immediateMesh->SetDrawInstruction( DrawInstruction( drawPrimitive, usesIndices, 0, elementCount ) );
	m_immediateMesh->SetVertices( numVerts, verts );
	m_immediateMesh->SetIndices( numIndices, indices );

	DrawMesh( m_immediateMesh );
}

void Renderer::DrawFullScreenImmediate( Material& material, const Rgba& color /* = Rgba::WHITE */, unsigned int passIndex /* = 0U */ )
{
	BindMaterial( material, passIndex );

	AABB2 currentViewport = m_currentCamera->GetViewport();

	MeshBuilder screenBuilder;	// NDC space
	screenBuilder.Begin( DrawPrimitiveType::TRIANGLES );
	screenBuilder.SetColor( color );

	screenBuilder.SetUVs( currentViewport.mins.x, currentViewport.mins.y );	// Mins
	screenBuilder.PushVertex( -1.0f, -1.0f, 0.0f );

	screenBuilder.SetUVs( currentViewport.mins.x, currentViewport.maxs.y );	// Mins.x, Maxs.y
	screenBuilder.PushVertex( -1.0f, 1.0f, 0.0f );

	screenBuilder.SetUVs( currentViewport.maxs.x, currentViewport.mins.y );	// Maxs.x, Mins.y
	screenBuilder.PushVertex( 1.0f, -1.0f, 0.0f );

	screenBuilder.SetUVs( currentViewport.maxs.x, currentViewport.maxs.y );	// Maxs
	screenBuilder.PushVertex( 1.0f, 1.0f, 0.0f );

	unsigned int indices[ 6 ] = { 0, 2, 1, 1, 2, 3 };
	screenBuilder.PushIndices( 6, indices );

	screenBuilder.End();

	Mesh* fullscreenMesh = screenBuilder.CreateMesh();
	DrawMesh( fullscreenMesh );
	delete fullscreenMesh;
}

void Renderer::BindMeshToShaderProgram( const Mesh* mesh, ShaderProgram* shaderProgram ) const
{
	glBindVertexArray( m_defaultVAO );

	mesh->Finalize();	// Bind the vertex and index buffers
	
	unsigned int vertexStride = mesh->GetVertexLayout()->m_stride;
	unsigned int attributeCount = mesh->GetVertexLayout()->GetAttributeCount();

	GLuint programHandle = shaderProgram->GetHandle();

	for ( unsigned int attributeIndex = 0; attributeIndex < attributeCount; attributeIndex++ )
	{
		VertexAttribute attribute = mesh->GetVertexLayout()->GetAttribute( attributeIndex );

		int bindPoint = glGetAttribLocation( programHandle, attribute.m_name.c_str() );

		if ( bindPoint >= 0 )
		{
			glEnableVertexAttribArray( bindPoint );
			glVertexAttribPointer( bindPoint, attribute.m_elementCount, GetGLDataType( attribute.m_type ), attribute.m_normalized, vertexStride, reinterpret_cast< GLvoid* >( attribute.m_memberOffset ) );
		}
	}
}

void Renderer::DrawLine( const Vector2& startPoint, const Vector2& endPoint, const Rgba& startColor, const Rgba& endColor, const float lineThickness ) const
{
	DisableTexturing();
	glLineWidth( lineThickness );

	Vertex_3DPCU verts[ 2 ] = {
		Vertex_3DPCU( ConvertVector2ToVector3( startPoint ), startColor, Vector2::ZERO ),
		Vertex_3DPCU( ConvertVector2ToVector3( endPoint ), endColor, Vector2::ZERO )
	};

	unsigned int indices[ 2 ] = { 0, 1 };

	DrawMeshImmediate( verts, 2, indices, 2, DrawPrimitiveType::LINES );
}

void Renderer::DrawLineBorder( const AABB2& bounds, const Rgba& color, const float lineThickness ) const
{
	DisableTexturing();
	glLineWidth( lineThickness );

	Vertex_3DPCU verts[ 4 ] = {
		Vertex_3DPCU( ConvertVector2ToVector3( bounds.mins ), color, Vector2::ZERO ),
		Vertex_3DPCU( ConvertVector2ToVector3( Vector2( bounds.maxs.x, bounds.mins.y ) ), color, Vector2::ZERO ),
		Vertex_3DPCU( ConvertVector2ToVector3( bounds.maxs ), color, Vector2::ZERO ),
		Vertex_3DPCU( ConvertVector2ToVector3( Vector2( bounds.mins.x, bounds.maxs.y ) ), color, Vector2::ZERO )
	};

	unsigned int indices[ 4 ] = { 0, 1, 2, 3 };

	DrawMeshImmediate( verts, 4, indices, 4, DrawPrimitiveType::LINE_LOOP );
}

void Renderer::DrawPolygon( const Vector2& location, const float angle, const Vector2 vertices[], const int numSides, RPolygonType polygonType, const Rgba& color ) const
{
	DisableTexturing();
	if (numSides == 0)
	{
		return;
	}

	float theta = 0.0f;
	float deltaTheta = 360.0f / static_cast<float>( numSides );

	PushMatrix();		// Perform the rendering in the polygon's local space
	Translate( location.x, location.y, 0.0f );
	Rotate( angle, 0.0f, 0.0f, 1.0f );

	Vertex_3DPCU* verts = new Vertex_3DPCU[ numSides * 2 ];

	for ( int i = 0; i < numSides; i++ )
	{
		if ( ( polygonType != RPolygonType::BROKEN_LINES ) || ( i % 2 == 0 ) )
		{
			verts[ 2 * i ].m_position = Vector3( vertices[ ( i % numSides ) ].x, vertices[ ( i % numSides ) ].y, 0.0f );
			verts[ 2 * i ].m_color = color;

			verts[ ( 2 * i ) + 1 ].m_position = Vector3( vertices[ ( ( i + 1 ) % numSides ) ].x, vertices[ ( ( i + 1 ) % numSides ) ].y, 0.0f );
			verts[ ( 2 * i ) + 1 ].m_color = color;
		}
	}

	unsigned int* indices = new unsigned int[ numSides * 2 ];
	for ( int index = 0; index < ( numSides * 2 ); index++ )	// Mirror the vertex array
	{
		indices[ index ] = index;
	}

	DrawMeshImmediate( verts, ( numSides * 2 ), indices, ( numSides * 2 ), DrawPrimitiveType::LINES );

	delete[] indices;
	indices = nullptr;
	delete[] verts;
	verts = nullptr;

	PopMatrix();
}

void Renderer::DrawAABB( const AABB2& bounds, const Rgba& color ) const
{
	PROFILE_SCOPE( "Renderer::DrawAABB()" );
	DisableTexturing();

	Vertex_3DPCU verts[ 4 ] = {
		Vertex_3DPCU( Vector3( bounds.mins.x, bounds.mins.y, 0.0f ), color, Vector2::ZERO ),
		Vertex_3DPCU( Vector3( bounds.maxs.x, bounds.mins.y, 0.0f ), color, Vector2::ZERO ),
		Vertex_3DPCU( Vector3( bounds.maxs.x, bounds.maxs.y, 0.0f ), color, Vector2::ZERO ),
		Vertex_3DPCU( Vector3( bounds.mins.x, bounds.maxs.y, 0.0f ), color, Vector2::ZERO )
	};

	unsigned int indices[ 6 ] = { 0, 1, 2, 2, 3, 0 };

	DrawMeshImmediate( verts, 4, indices, 6, DrawPrimitiveType::TRIANGLES );
}

void Renderer::DrawAABBBorder( const AABB2& innerBoundsAABB, const Rgba& colorAtInnerBorder, const AABB2& outerBoundsAABB, const Rgba& colorAtOuterBorder ) const
{
	DisableTexturing();

	// Draw 8 AABBs for the sides and corners

	// Draw sides
	AABB2 leftAABB = AABB2( Vector2( outerBoundsAABB.mins.x, innerBoundsAABB.mins.y ), Vector2( innerBoundsAABB.mins.x, innerBoundsAABB.maxs.y ) );
	Vertex_3DPCU verts[ 4 ] = {
		Vertex_3DPCU( Vector3( leftAABB.mins.x, leftAABB.mins.y, 0.0f ), colorAtOuterBorder, Vector2::ZERO ),
		Vertex_3DPCU( Vector3( leftAABB.maxs.x, leftAABB.mins.y, 0.0f ), colorAtInnerBorder, Vector2::ZERO ),
		Vertex_3DPCU( Vector3( leftAABB.maxs.x, leftAABB.maxs.y, 0.0f ), colorAtInnerBorder, Vector2::ZERO ),
		Vertex_3DPCU( Vector3( leftAABB.mins.x, leftAABB.maxs.y, 0.0f ), colorAtOuterBorder, Vector2::ZERO )
	};
	unsigned int indices[ 6 ] = { 0, 1, 2, 2, 3, 0 };
	DrawMeshImmediate( verts, 4, indices, 6, DrawPrimitiveType::TRIANGLES );

	AABB2 topAABB = AABB2( Vector2( innerBoundsAABB.mins.x, innerBoundsAABB.maxs.y ), Vector2( innerBoundsAABB.maxs.x, outerBoundsAABB.maxs.y ) );
	verts[ 0 ] = Vertex_3DPCU( Vector3( topAABB.mins.x, topAABB.mins.y, 0.0f ), colorAtInnerBorder, Vector2::ZERO );
	verts[ 1 ] = Vertex_3DPCU( Vector3( topAABB.maxs.x, topAABB.mins.y, 0.0f ), colorAtInnerBorder, Vector2::ZERO );
	verts[ 2 ] = Vertex_3DPCU( Vector3( topAABB.maxs.x, topAABB.maxs.y, 0.0f ), colorAtOuterBorder, Vector2::ZERO );
	verts[ 3 ] = Vertex_3DPCU( Vector3( topAABB.mins.x, topAABB.maxs.y, 0.0f ), colorAtOuterBorder, Vector2::ZERO );
	DrawMeshImmediate( verts, 4, indices, 6, DrawPrimitiveType::TRIANGLES );

	AABB2 rightAABB = AABB2( Vector2( innerBoundsAABB.maxs.x, innerBoundsAABB.mins.y ), Vector2( outerBoundsAABB.maxs.x, innerBoundsAABB.maxs.y ) );
	verts[ 0 ] = Vertex_3DPCU( Vector3( rightAABB.mins.x, rightAABB.mins.y, 0.0f ), colorAtInnerBorder, Vector2::ZERO );
	verts[ 1 ] = Vertex_3DPCU( Vector3( rightAABB.maxs.x, rightAABB.mins.y, 0.0f ), colorAtOuterBorder, Vector2::ZERO );
	verts[ 2 ] = Vertex_3DPCU( Vector3( rightAABB.maxs.x, rightAABB.maxs.y, 0.0f ), colorAtOuterBorder, Vector2::ZERO );
	verts[ 3 ] = Vertex_3DPCU( Vector3( rightAABB.mins.x, rightAABB.maxs.y, 0.0f ), colorAtInnerBorder, Vector2::ZERO );
	DrawMeshImmediate( verts, 4, indices, 6, DrawPrimitiveType::TRIANGLES );

	AABB2 bottomAABB = AABB2( Vector2( innerBoundsAABB.mins.x, outerBoundsAABB.mins.y ), Vector2( innerBoundsAABB.maxs.x, innerBoundsAABB.mins.y ) );
	verts[ 0 ] = Vertex_3DPCU( Vector3( bottomAABB.mins.x, bottomAABB.mins.y, 0.0f ), colorAtOuterBorder, Vector2::ZERO );
	verts[ 1 ] = Vertex_3DPCU( Vector3( bottomAABB.maxs.x, bottomAABB.mins.y, 0.0f ), colorAtOuterBorder, Vector2::ZERO );
	verts[ 2 ] = Vertex_3DPCU( Vector3( bottomAABB.maxs.x, bottomAABB.maxs.y, 0.0f ), colorAtInnerBorder, Vector2::ZERO );
	verts[ 3 ] = Vertex_3DPCU( Vector3( bottomAABB.mins.x, bottomAABB.maxs.y, 0.0f ), colorAtInnerBorder, Vector2::ZERO );
	DrawMeshImmediate( verts, 4, indices, 6, DrawPrimitiveType::TRIANGLES );

	// Draw corners
	AABB2 bottomLeftAABB = AABB2( outerBoundsAABB.mins, innerBoundsAABB.mins );
	verts[ 0 ] = Vertex_3DPCU( Vector3( bottomLeftAABB.mins.x, bottomLeftAABB.mins.y, 0.0f ), colorAtOuterBorder, Vector2::ZERO );
	verts[ 1 ] = Vertex_3DPCU( Vector3( bottomLeftAABB.maxs.x, bottomLeftAABB.mins.y, 0.0f ), colorAtOuterBorder, Vector2::ZERO );
	verts[ 2 ] = Vertex_3DPCU( Vector3( bottomLeftAABB.maxs.x, bottomLeftAABB.maxs.y, 0.0f ), colorAtInnerBorder, Vector2::ZERO );
	verts[ 3 ] = Vertex_3DPCU( Vector3( bottomLeftAABB.mins.x, bottomLeftAABB.maxs.y, 0.0f ), colorAtOuterBorder, Vector2::ZERO );
	DrawMeshImmediate( verts, 4, indices, 6, DrawPrimitiveType::TRIANGLES );
	
	AABB2 topLeftAABB = AABB2( Vector2( outerBoundsAABB.mins.x, innerBoundsAABB.maxs.y ), Vector2( innerBoundsAABB.mins.x, outerBoundsAABB.maxs.y ) );
	verts[ 0 ] = Vertex_3DPCU( Vector3( topLeftAABB.mins.x, topLeftAABB.mins.y, 0.0f ), colorAtOuterBorder, Vector2::ZERO );
	verts[ 1 ] = Vertex_3DPCU( Vector3( topLeftAABB.maxs.x, topLeftAABB.mins.y, 0.0f ), colorAtInnerBorder, Vector2::ZERO );
	verts[ 2 ] = Vertex_3DPCU( Vector3( topLeftAABB.maxs.x, topLeftAABB.maxs.y, 0.0f ), colorAtOuterBorder, Vector2::ZERO );
	verts[ 3 ] = Vertex_3DPCU( Vector3( topLeftAABB.mins.x, topLeftAABB.maxs.y, 0.0f ), colorAtOuterBorder, Vector2::ZERO );
	DrawMeshImmediate( verts, 4, indices, 6, DrawPrimitiveType::TRIANGLES );

	AABB2 topRightAABB = AABB2( innerBoundsAABB.maxs, outerBoundsAABB.maxs );
	verts[ 0 ] = Vertex_3DPCU( Vector3( topRightAABB.mins.x, topRightAABB.mins.y, 0.0f ), colorAtInnerBorder, Vector2::ZERO );
	verts[ 1 ] = Vertex_3DPCU( Vector3( topRightAABB.maxs.x, topRightAABB.mins.y, 0.0f ), colorAtOuterBorder, Vector2::ZERO );
	verts[ 2 ] = Vertex_3DPCU( Vector3( topRightAABB.maxs.x, topRightAABB.maxs.y, 0.0f ), colorAtOuterBorder, Vector2::ZERO );
	verts[ 3 ] = Vertex_3DPCU( Vector3( topRightAABB.mins.x, topRightAABB.maxs.y, 0.0f ), colorAtOuterBorder, Vector2::ZERO );
	DrawMeshImmediate( verts, 4, indices, 6, DrawPrimitiveType::TRIANGLES );

	AABB2 bottomRightAABB = AABB2( Vector2( innerBoundsAABB.maxs.x, outerBoundsAABB.mins.y ), Vector2( outerBoundsAABB.maxs.x, innerBoundsAABB.mins.y ) );
	verts[ 0 ] = Vertex_3DPCU( Vector3( bottomRightAABB.mins.x, bottomRightAABB.mins.y, 0.0f ), colorAtOuterBorder, Vector2::ZERO );
	verts[ 1 ] = Vertex_3DPCU( Vector3( bottomRightAABB.maxs.x, bottomRightAABB.mins.y, 0.0f ), colorAtOuterBorder, Vector2::ZERO );
	verts[ 2 ] = Vertex_3DPCU( Vector3( bottomRightAABB.maxs.x, bottomRightAABB.maxs.y, 0.0f ), colorAtOuterBorder, Vector2::ZERO );
	verts[ 3 ] = Vertex_3DPCU( Vector3( bottomRightAABB.mins.x, bottomRightAABB.maxs.y, 0.0f ), colorAtInnerBorder, Vector2::ZERO );
	DrawMeshImmediate( verts, 4, indices, 6, DrawPrimitiveType::TRIANGLES );
}

void Renderer::DrawTexturedAABB( const AABB2& bounds, const Texture& texture, const Vector2& textureCoordinatesAtMins, const Vector2& textureCoordinatesAtMaxs, const Rgba& tint ) const
{
	MeshBuilder aabbMeshBuilder = MakeTexturedAABBMesh( bounds, texture, textureCoordinatesAtMins, textureCoordinatesAtMaxs, tint );
	BindTextureAndSampler( 0, texture, *GetDefaultSampler( texture.UsesMipMaps() ) );
	Mesh* mesh = aabbMeshBuilder.CreateMesh();
	DrawMesh( mesh );
	delete mesh;
}

MeshBuilder Renderer::MakeTexturedAABBMesh( const AABB2& bounds, const Texture& texture, const Vector2& textureCoordinatesAtMins, const Vector2& textureCoordinatesAtMaxs, const Rgba& tint ) const
{	
	BindTextureAndSampler( 0, texture, *GetDefaultSampler( texture.UsesMipMaps() ) );

	MeshBuilder meshBuilder;
	meshBuilder.Begin( DrawPrimitiveType::TRIANGLES );
	meshBuilder.SetColor( tint );

	meshBuilder.SetUVs( textureCoordinatesAtMins.x, textureCoordinatesAtMins.y );	// Mins
	meshBuilder.PushVertex( bounds.mins.x, bounds.mins.y, 0.0f );

	meshBuilder.SetUVs( textureCoordinatesAtMins.x, textureCoordinatesAtMaxs.y );	// Mins.x, Maxs.y
	meshBuilder.PushVertex( bounds.mins.x, bounds.maxs.y, 0.0f );

	meshBuilder.SetUVs( textureCoordinatesAtMaxs.x, textureCoordinatesAtMins.y );	// Maxs.x, Mins.y
	meshBuilder.PushVertex( bounds.maxs.x, bounds.mins.y, 0.0f );

	meshBuilder.SetUVs( textureCoordinatesAtMaxs.x, textureCoordinatesAtMaxs.y );	// Maxs
	meshBuilder.PushVertex( bounds.maxs.x, bounds.maxs.y, 0.0f );

	unsigned int indices[ 6 ] = { 0, 2, 1, 1, 2, 3 };
	meshBuilder.PushIndices( 6, indices );

	meshBuilder.End();

	return meshBuilder;
}

#pragma region Text2DTools

MeshBuilder Renderer::MakeTextMesh2D( const Vector2& drawMins, const std::string& asciiText, float cellHeight, float characterWidth, const Rgba& tint, const BitmapFont* font ) const
{
	MeshBuilder textMeshBuilder;
	textMeshBuilder.Begin( DrawPrimitiveType::TRIANGLES );
	for ( size_t currentCharacterIndex = 0; currentCharacterIndex < asciiText.length(); currentCharacterIndex++ )
	{
		AABB2 characterUvs = font->GetUVsForGlyph( asciiText[ currentCharacterIndex ] );
		Vector2 characterBoundsMins = drawMins + Vector2( ( static_cast<float>( currentCharacterIndex ) * characterWidth ), 0.0f );
		Vector2 characterBoundsMaxs = drawMins + Vector2( ( static_cast<float>( currentCharacterIndex + 1 ) * characterWidth ), cellHeight );
		AABB2 characterBounds = AABB2( characterBoundsMins, characterBoundsMaxs );

		textMeshBuilder.MergeMesh( MakeTexturedAABBMesh( characterBounds, *( font->m_spriteSheet.GetTexture() ), characterUvs.mins, characterUvs.maxs, tint ) );
	}

	textMeshBuilder.End();
	return textMeshBuilder;
}

void Renderer::DrawText2D( const Vector2& drawMins, const std::string& asciiText, float cellHeight, const Rgba& tint = Rgba::WHITE, float aspectScale = 1.0f, const BitmapFont* font = nullptr ) const
{
	PROFILE_SCOPE( "Renderer::DrawText2D()" );
	if ( font == nullptr )
	{
		font = m_defaultBitmapFont;
	}

	float characterWidth = font->GetCharacterWidth( cellHeight, aspectScale );

	
	BindTextureAndSampler( 0, *font->m_spriteSheet.GetTexture(), *GetDefaultSampler( font->m_spriteSheet.GetTexture()->UsesMipMaps() ) );

	MeshBuilder textMeshBuilder = MakeTextMesh2D( drawMins, asciiText, cellHeight, characterWidth, tint, font );
	Mesh* textMesh = textMeshBuilder.CreateMesh();
	DrawMesh( textMesh );
	delete textMesh;
}

void Renderer::DrawTextInBox2D( const std::string& asciiText, const AABB2& boxBounds, float cellHeight, const Rgba& tint = Rgba::WHITE, float aspectScale = 1.0f, const BitmapFont* font = nullptr, const TextDrawMode textDrawMode = TextDrawMode::TEXT_DRAW_WORD_WRAP, const Vector2& alignment = Vector2( 0.5f, 0.5f ) ) const
{
	if ( font == nullptr )
	{
		font = m_defaultBitmapFont;
	}

	TokenizedString tokenizedText = TokenizedString( asciiText, "\n" );
	std::vector< std::string > linesOfText = tokenizedText.GetTokens();

	switch( textDrawMode )
	{
		case TextDrawMode::TEXT_DRAW_OVERRUN:
		{
			DrawTextInBox2DOverrun( linesOfText, boxBounds, cellHeight, tint, aspectScale, font, alignment );
			break;
		}
		case TextDrawMode::TEXT_DRAW_SHRINK_TO_FIT:
		{
			DrawTextInBox2DShrinkToFit( linesOfText, boxBounds, cellHeight, tint, aspectScale, font, alignment );
			break;
		}
		case TextDrawMode::TEXT_DRAW_WORD_WRAP:
		{
			DrawTextInBox2DWordWrap( linesOfText, boxBounds, cellHeight, tint, aspectScale, font, alignment );
			break;
		}
	}
}

Vector3 Renderer::GetOrientedQuadPositionWithPivotCorrection( const Vector3& position, const Vector3& rightVector, const Vector2& pivot, const Vector2& dimensions ) const
{
	Vector3 cameraForward = m_currentCamera->GetForward();
	Vector3 rightVectorOrthoToCameraForward = rightVector - rightVector.GetComponentInDirection( cameraForward );
	rightVectorOrthoToCameraForward.GetNormalized();
	Vector3 upVector = CrossProduct( cameraForward, rightVectorOrthoToCameraForward );

	Vector3 pivotDisplacement = ( rightVectorOrthoToCameraForward * ( dimensions.x * pivot.x ) ) +	// Right component
		( upVector * ( dimensions.y * pivot.y ) );	// Up component
	Vector3 effectivePosition = position - pivotDisplacement;
	return effectivePosition;
}

MeshBuilder Renderer::MakeOrientedQuadMesh( const Vector3& position, const Vector3& rightVector, const Vector2& pivot, const Vector2& dimensions, const Rgba& color, bool usesCameraForward /* = true*/, const Vector3& upVector /* = Vector3::UP*/ ) const
{
	BindTextureAndSampler( 0, *m_defaultTexture, *m_defaultSampler );

	Vector3 drawRightVector = rightVector;
	Vector3 drawUpVector = upVector;
	if ( usesCameraForward )
	{
		Vector3 cameraForward = m_currentCamera->GetForward();
		Vector3 rightVectorOrthoToCameraForward = rightVector - rightVector.GetComponentInDirection( cameraForward );
		rightVectorOrthoToCameraForward.GetNormalized();
		drawUpVector = CrossProduct( cameraForward, rightVectorOrthoToCameraForward );
		drawRightVector = rightVectorOrthoToCameraForward;
	}
	
	Vector3 effectivePosition = GetOrientedQuadPositionWithPivotCorrection( position, drawRightVector, pivot, dimensions );

	MeshBuilder quadMeshBuilder;

	quadMeshBuilder.Begin( DrawPrimitiveType::TRIANGLES );
	quadMeshBuilder.SetColor( color );
	quadMeshBuilder.SetUVs( Vector2::ZERO );

	quadMeshBuilder.PushVertex( effectivePosition );
	quadMeshBuilder.PushVertex( effectivePosition + ( drawUpVector * dimensions.y ) );
	quadMeshBuilder.PushVertex( effectivePosition + ( drawRightVector * dimensions.x ) );
	quadMeshBuilder.PushVertex( effectivePosition + ( drawRightVector * dimensions.x ) + ( drawUpVector * dimensions.y ) );

	unsigned int indices[ 6 ] = { 0, 2, 1, 1, 2, 3 };
	quadMeshBuilder.PushIndices( 6, indices );

	quadMeshBuilder.End();
	
	return quadMeshBuilder;
}

MeshBuilder Renderer::MakeTexturedOrientedQuadMesh( const Vector3& position, const Vector3& rightVector, const Vector2& pivot, const Vector2& dimensions, const Texture& texture, const Vector2& textureCoordinatesAtMins, const Vector2& textureCoordinatesAtMaxs, const Rgba& tint, bool usesCameraForward /* = true */, const Vector3& upVector /* = Vector3::UP */ ) const
{
	BindTextureAndSampler( 0, texture, *GetDefaultSampler( texture.UsesMipMaps() ) );

	Vector3 drawRightVector = rightVector;
	Vector3 drawUpVector = upVector;
	if ( usesCameraForward )
	{
		Vector3 cameraForward = m_currentCamera->GetForward();
		Vector3 rightVectorOrthoToCameraForward = rightVector - rightVector.GetComponentInDirection( cameraForward );
		rightVectorOrthoToCameraForward.GetNormalized();
		drawUpVector = CrossProduct( cameraForward, rightVectorOrthoToCameraForward );
		drawRightVector = rightVectorOrthoToCameraForward;
	}

	Vector3 pivotDisplacement = ( drawRightVector * ( dimensions.x * pivot.x ) ) +	// Right component
		( drawUpVector * ( dimensions.y * pivot.y ) );	// Up component
	Vector3 effectivePosition = position - pivotDisplacement;

	MeshBuilder quadMeshBuilder;

	quadMeshBuilder.Begin( DrawPrimitiveType::TRIANGLES );
	quadMeshBuilder.SetColor( tint );

	quadMeshBuilder.SetUVs( textureCoordinatesAtMins );
	quadMeshBuilder.PushVertex( effectivePosition );

	quadMeshBuilder.SetUVs( textureCoordinatesAtMins.x, textureCoordinatesAtMaxs.y );
	quadMeshBuilder.PushVertex( effectivePosition + ( drawUpVector * dimensions.y ) );

	quadMeshBuilder.SetUVs( textureCoordinatesAtMaxs.x, textureCoordinatesAtMins.y );
	quadMeshBuilder.PushVertex( effectivePosition + ( drawRightVector * dimensions.x ) );

	quadMeshBuilder.SetUVs( textureCoordinatesAtMaxs );
	quadMeshBuilder.PushVertex( effectivePosition + ( drawRightVector * dimensions.x ) + ( drawUpVector * dimensions.y ) );

	unsigned int indices[ 6 ] = { 0, 2, 1, 1, 2, 3 };
	quadMeshBuilder.PushIndices( 6, indices );

	quadMeshBuilder.End();

	return quadMeshBuilder;
}

MeshBuilder Renderer::MakeTextMesh3D( const std::string& asciiText, const Vector3& position, const Vector3& rightVector, float cellHeight, float characterWidth, const Rgba& tint, const BitmapFont* font, bool usesCameraForward /* = true */, const Vector3& upVector /* = Vector3::UP */ ) const
{
	MeshBuilder textMeshBuilder;
	textMeshBuilder.Begin( DrawPrimitiveType::TRIANGLES );
	textMeshBuilder.SetColor( tint );
	for ( size_t currentCharacterIndex = 0; currentCharacterIndex < asciiText.length(); currentCharacterIndex++ )
	{
		AABB2 characterUvs = font->GetUVsForGlyph( asciiText[ currentCharacterIndex ] );
		Vector3 currentPosition = position + ( rightVector * ( static_cast<float>( currentCharacterIndex ) * characterWidth ) );

		textMeshBuilder.MergeMesh( MakeTexturedOrientedQuadMesh( currentPosition, rightVector, Vector2::ZERO, Vector2( characterWidth, cellHeight ), *( font->m_spriteSheet.GetTexture() ), characterUvs.mins, characterUvs.maxs, tint, usesCameraForward, upVector ) );
	}
	textMeshBuilder.End();

	return textMeshBuilder;
}

void Renderer::DrawTextInBox3D( const std::string& asciiText, const Vector3& position, const Vector3& rightVector, const Vector2& boxDimensions, float cellHeight, const Rgba& boxColor, const Rgba& textTint, float aspectScale, const BitmapFont* font, const Vector2& boxPivot, const Vector2& alignment, bool usesCameraForward /* = true */, const Vector3& upVector /* = Vector3::UP */ ) const
{
	if ( font == nullptr )
	{
		font = m_defaultBitmapFont;
	}

	TokenizedString tokenizedText = TokenizedString( asciiText, "\n" );
	std::vector< std::string > linesOfText = tokenizedText.GetTokens();

	if ( boxColor.a != Rgba::NO_COLOR.a )
	{
		DrawOrientedQuad( position, rightVector, boxPivot, boxDimensions, boxColor, usesCameraForward, upVector );
	}
	DrawTextInBox3DShrinkToFit( linesOfText, GetOrientedQuadPositionWithPivotCorrection( position, rightVector, boxPivot, boxDimensions ), rightVector, boxDimensions, cellHeight, boxColor, textTint, aspectScale, font, alignment, usesCameraForward, upVector );
}

void Renderer::DrawSprite( Sprite* sprite, const Vector3& position, const Vector3& rightVector, float spriteScale, const Rgba& tint, Vector2 uvFlip /* = Vector2::ONE */ )
{
	BindTextureAndSampler( 0, *sprite->GetTexture(), *GetDefaultSampler( sprite->GetTexture()->UsesMipMaps() ) );

	// Compute the right and up vectors for the sprite - it has to always render 
	Vector3 cameraForward = m_currentCamera->GetForward();
	Vector3 rightVectorOrthoToCameraForward = rightVector - rightVector.GetComponentInDirection( cameraForward );
	rightVectorOrthoToCameraForward.GetNormalized();
	Vector3 upVector = CrossProduct( cameraForward, rightVectorOrthoToCameraForward );

	AABB2 spriteUVs = sprite->GetFlippedUVs( uvFlip );
	Vector2 spritePixels = sprite->GetRenderDimensions();
	Vector2 effectiveDimensions = spriteScale * ( spritePixels / sprite->GetPPU() );
	Vector3 pivotDisplacement = ( rightVectorOrthoToCameraForward * ( effectiveDimensions.x * sprite->GetPivot().x ) ) +	// Right component
								( upVector * ( effectiveDimensions.y * sprite->GetPivot().y ) );	// Up component
	Vector3 effectivePosition = position - pivotDisplacement;

	MeshBuilder spriteMeshBuilder;
	spriteMeshBuilder.Begin( DrawPrimitiveType::TRIANGLES );
	spriteMeshBuilder.SetColor( tint );

	Vertex_3DPCU vertices[ 4 ] = {
		Vertex_3DPCU( effectivePosition, tint, spriteUVs.mins ),
		Vertex_3DPCU( ( effectivePosition + ( upVector * effectiveDimensions.y ) ), tint, spriteUVs.GetMinXMaxY() ),
		Vertex_3DPCU( ( effectivePosition + ( rightVectorOrthoToCameraForward * effectiveDimensions.x ) ), tint, spriteUVs.GetMaxXMinY() ),
		Vertex_3DPCU( ( effectivePosition + ( rightVectorOrthoToCameraForward * effectiveDimensions.x ) + ( upVector * effectiveDimensions.y ) ), tint, spriteUVs.maxs )
	};

	unsigned int indices[ 6 ] = { 0, 1, 2, 2, 1, 3 };

	spriteMeshBuilder.PushVertices( 4, vertices );
	spriteMeshBuilder.PushIndices( 6, indices );
	spriteMeshBuilder.End();
	
	Mesh* spriteMesh = spriteMeshBuilder.CreateMesh();
	DrawMesh( spriteMesh );
	delete spriteMesh;
}

void Renderer::DrawSprite( const std::string& spriteName, const Vector3& position, const Vector3& rightVector, float spriteScale, const Rgba& tint, Vector2 uvFlip /* = Vector2::ONE */ )
{
	DrawSprite( m_loadedSpritesByName[ spriteName ], position, rightVector, spriteScale, tint, uvFlip );
}

void Renderer::DrawTextureCube( const TextureCube& textureCube, const Vector3& dimensions )
{
	Mesh* texturedCubeMesh = MakeTexturedCubeMesh( Vector3::ZERO, dimensions, *GetDefaultTexture(), Rgba::WHITE ).CreateMesh();
	glActiveTexture( GL_TEXTURE0 + 8 );
	glBindTexture( GL_TEXTURE_CUBE_MAP, textureCube.m_handle );
	glBindSampler( 8, GetDefaultSampler()->GetHandle() );
	DrawMesh( texturedCubeMesh );
	delete texturedCubeMesh;
}

MeshBuilder Renderer::MakeCubeMesh( const Vector3& center, const Vector3& dimensions, const Rgba& color ) const
{
	BindTextureAndSampler( 0, *m_defaultTexture, *m_defaultSampler );

	float halfDimX = dimensions.x * 0.5f;
	float halfDimY = dimensions.y * 0.5f;
	float halfDimZ = dimensions.z * 0.5f;

	VertexBuilder vertices[ 24 ] = {
		VertexBuilder( Vector3( ( center.x - halfDimX ), ( center.y - halfDimY ), ( center.z - halfDimZ ) ), color, Vector2::ZERO, ( -1.0f * Vector3::UP ), Vector4( Vector3::RIGHT, -1.0f ) ),		// Bottom
		VertexBuilder( Vector3( ( center.x - halfDimX ), ( center.y - halfDimY ), ( center.z + halfDimZ ) ), color, Vector2::ZERO, ( -1.0f * Vector3::UP ), Vector4( Vector3::RIGHT, -1.0f ) ),
		VertexBuilder( Vector3( ( center.x + halfDimX ), ( center.y - halfDimY ), ( center.z - halfDimZ ) ), color, Vector2::ZERO, ( -1.0f * Vector3::UP ), Vector4( Vector3::RIGHT, -1.0f ) ),
		VertexBuilder( Vector3( ( center.x + halfDimX ), ( center.y - halfDimY ), ( center.z + halfDimZ ) ), color, Vector2::ZERO, ( -1.0f * Vector3::UP ), Vector4( Vector3::RIGHT, -1.0f ) ),
		VertexBuilder( Vector3( ( center.x + halfDimX ), ( center.y - halfDimY ), ( center.z - halfDimZ ) ), color, Vector2::ZERO, Vector3::RIGHT, Vector4( Vector3::FORWARD, 1.0f ) ),		// Right
		VertexBuilder( Vector3( ( center.x + halfDimX ), ( center.y - halfDimY ), ( center.z + halfDimZ ) ), color, Vector2::ZERO, Vector3::RIGHT, Vector4( Vector3::FORWARD, 1.0f ) ),
		VertexBuilder( Vector3( ( center.x + halfDimX ), ( center.y + halfDimY ), ( center.z - halfDimZ ) ), color, Vector2::ZERO, Vector3::RIGHT, Vector4( Vector3::FORWARD, 1.0f ) ),
		VertexBuilder( Vector3( ( center.x + halfDimX ), ( center.y + halfDimY ), ( center.z + halfDimZ ) ), color, Vector2::ZERO, Vector3::RIGHT, Vector4( Vector3::FORWARD, 1.0f ) ),
		VertexBuilder( Vector3( ( center.x - halfDimX ), ( center.y - halfDimY ), ( center.z - halfDimZ ) ), color, Vector2::ZERO, ( -1.0f * Vector3::FORWARD ), Vector4( Vector3::RIGHT, 1.0f ) ),		// Front
		VertexBuilder( Vector3( ( center.x + halfDimX ), ( center.y - halfDimY ), ( center.z - halfDimZ ) ), color, Vector2::ZERO, ( -1.0f * Vector3::FORWARD ), Vector4( Vector3::RIGHT, 1.0f ) ),
		VertexBuilder( Vector3( ( center.x - halfDimX ), ( center.y + halfDimY ), ( center.z - halfDimZ ) ), color, Vector2::ZERO, ( -1.0f * Vector3::FORWARD ), Vector4( Vector3::RIGHT, 1.0f ) ),
		VertexBuilder( Vector3( ( center.x + halfDimX ), ( center.y + halfDimY ), ( center.z - halfDimZ ) ), color, Vector2::ZERO, ( -1.0f * Vector3::FORWARD ), Vector4( Vector3::RIGHT, 1.0f ) ),
		VertexBuilder( Vector3( ( center.x - halfDimX ), ( center.y - halfDimY ), ( center.z + halfDimZ ) ), color, Vector2::ZERO, ( -1.0f * Vector3::RIGHT ), Vector4( ( -1.0f * Vector3::FORWARD ), 1.0f ) ),		// Left
		VertexBuilder( Vector3( ( center.x - halfDimX ), ( center.y - halfDimY ), ( center.z - halfDimZ ) ), color, Vector2::ZERO, ( -1.0f * Vector3::RIGHT ), Vector4( ( -1.0f * Vector3::FORWARD ), 1.0f ) ),
		VertexBuilder( Vector3( ( center.x - halfDimX ), ( center.y + halfDimY ), ( center.z + halfDimZ ) ), color, Vector2::ZERO, ( -1.0f * Vector3::RIGHT ), Vector4( ( -1.0f * Vector3::FORWARD ), 1.0f ) ),
		VertexBuilder( Vector3( ( center.x - halfDimX ), ( center.y + halfDimY ), ( center.z - halfDimZ ) ), color, Vector2::ZERO, ( -1.0f * Vector3::RIGHT ), Vector4( ( -1.0f * Vector3::FORWARD ), 1.0f ) ),
		VertexBuilder( Vector3( ( center.x + halfDimX ), ( center.y - halfDimY ), ( center.z + halfDimZ ) ), color, Vector2::ZERO, Vector3::FORWARD, Vector4( ( -1.0f * Vector3::RIGHT ), 1.0f ) ),		// Back
		VertexBuilder( Vector3( ( center.x - halfDimX ), ( center.y - halfDimY ), ( center.z + halfDimZ ) ), color, Vector2::ZERO, Vector3::FORWARD, Vector4( ( -1.0f * Vector3::RIGHT ), 1.0f ) ),
		VertexBuilder( Vector3( ( center.x + halfDimX ), ( center.y + halfDimY ), ( center.z + halfDimZ ) ), color, Vector2::ZERO, Vector3::FORWARD, Vector4( ( -1.0f * Vector3::RIGHT ), 1.0f ) ),
		VertexBuilder( Vector3( ( center.x - halfDimX ), ( center.y + halfDimY ), ( center.z + halfDimZ ) ), color, Vector2::ZERO, Vector3::FORWARD, Vector4( ( -1.0f * Vector3::RIGHT ), 1.0f ) ),
		VertexBuilder( Vector3( ( center.x - halfDimX ), ( center.y + halfDimY ), ( center.z - halfDimZ ) ), color, Vector2::ZERO, Vector3::UP, Vector4( Vector3::RIGHT, 1.0f ) ),		// Top
		VertexBuilder( Vector3( ( center.x + halfDimX ), ( center.y + halfDimY ), ( center.z - halfDimZ ) ), color, Vector2::ZERO, Vector3::UP, Vector4( Vector3::RIGHT, 1.0f ) ),
		VertexBuilder( Vector3( ( center.x - halfDimX ), ( center.y + halfDimY ), ( center.z + halfDimZ ) ), color, Vector2::ZERO, Vector3::UP, Vector4( Vector3::RIGHT, 1.0f ) ),
		VertexBuilder( Vector3( ( center.x + halfDimX ), ( center.y + halfDimY ), ( center.z + halfDimZ ) ), color, Vector2::ZERO, Vector3::UP, Vector4( Vector3::RIGHT, 1.0f ) )
	};
	unsigned int indices[ 36 ] = {
		0, 1, 2, 1, 3, 2,		// Bottom
		4, 5, 6, 5, 7, 6,		// Right
		8, 9, 10, 9, 11, 10,	// Front
		12, 13, 14, 13, 15, 14,	// Left
		16, 17, 18, 17, 19, 18,	// Back
		20, 21, 22, 21, 23, 22	// Top
	};

	MeshBuilder cubeMeshBuilder;
	cubeMeshBuilder.Begin( DrawPrimitiveType::TRIANGLES );
	cubeMeshBuilder.PushVertices( 24, vertices );
	cubeMeshBuilder.PushIndices( 36, indices );
	cubeMeshBuilder.End();

	return cubeMeshBuilder;
}

MeshBuilder Renderer::MakeCubeMesh( const AABB3& bounds, const Rgba& color ) const
{
	Vector3 center = bounds.GetCenter();
	Vector3 dimensions = bounds.GetDimensions();
	return MakeCubeMesh( center, dimensions, color );
}

MeshBuilder Renderer::MakeTexturedCubeMesh( const Vector3& center, const Vector3& dimensions, const Texture& texture, const Rgba& tint, const AABB2& uvTop /* = AABB2::ZERO_TO_ONE */, const AABB2& uvSide /* = AABB2::ZERO_TO_ONE */, const AABB2& uvBottom /* = AABB2::ZERO_TO_ONE */ ) const
{
	BindTextureAndSampler( 0, texture, *GetDefaultSampler( texture.UsesMipMaps() ) );

	float halfDimX = dimensions.x * 0.5f;
	float halfDimY = dimensions.y * 0.5f;
	float halfDimZ = dimensions.z * 0.5f;

	VertexBuilder vertices[ 24 ] = {
		VertexBuilder( Vector3( ( center.x - halfDimX ), ( center.y - halfDimY ), ( center.z - halfDimZ ) ), tint, uvBottom.mins, ( -1.0f * Vector3::UP ), Vector4( Vector3::RIGHT, -1.0f ) ),		// Bottom
		VertexBuilder( Vector3( ( center.x - halfDimX ), ( center.y - halfDimY ), ( center.z + halfDimZ ) ), tint, Vector2( uvBottom.mins.x, uvBottom.maxs.y ), ( -1.0f * Vector3::UP ), Vector4( Vector3::RIGHT, -1.0f ) ),
		VertexBuilder( Vector3( ( center.x + halfDimX ), ( center.y - halfDimY ), ( center.z - halfDimZ ) ), tint, Vector2( uvBottom.maxs.x, uvBottom.mins.y ), ( -1.0f * Vector3::UP ), Vector4( Vector3::RIGHT, -1.0f ) ),
		VertexBuilder( Vector3( ( center.x + halfDimX ), ( center.y - halfDimY ), ( center.z + halfDimZ ) ), tint, uvBottom.maxs, ( -1.0f * Vector3::UP ), Vector4( Vector3::RIGHT, -1.0f ) ),
		VertexBuilder( Vector3( ( center.x + halfDimX ), ( center.y - halfDimY ), ( center.z - halfDimZ ) ), tint, uvSide.mins, Vector3::RIGHT, Vector4( Vector3::FORWARD, 1.0f ) ),		// Right
		VertexBuilder( Vector3( ( center.x + halfDimX ), ( center.y - halfDimY ), ( center.z + halfDimZ ) ), tint, Vector2( uvSide.maxs.x, uvSide.mins.y ), Vector3::RIGHT, Vector4( Vector3::FORWARD, 1.0f ) ),
		VertexBuilder( Vector3( ( center.x + halfDimX ), ( center.y + halfDimY ), ( center.z - halfDimZ ) ), tint, Vector2( uvSide.mins.x, uvSide.maxs.y ), Vector3::RIGHT, Vector4( Vector3::FORWARD, 1.0f ) ),
		VertexBuilder( Vector3( ( center.x + halfDimX ), ( center.y + halfDimY ), ( center.z + halfDimZ ) ), tint, uvSide.maxs, Vector3::RIGHT, Vector4( Vector3::FORWARD, 1.0f ) ),
		VertexBuilder( Vector3( ( center.x - halfDimX ), ( center.y - halfDimY ), ( center.z - halfDimZ ) ), tint, uvSide.mins, ( -1.0f * Vector3::FORWARD ), Vector4( Vector3::RIGHT, 1.0f ) ),		// Front
		VertexBuilder( Vector3( ( center.x + halfDimX ), ( center.y - halfDimY ), ( center.z - halfDimZ ) ), tint, Vector2( uvSide.maxs.x, uvSide.mins.y ), ( -1.0f * Vector3::FORWARD ), Vector4( Vector3::RIGHT, 1.0f ) ),
		VertexBuilder( Vector3( ( center.x - halfDimX ), ( center.y + halfDimY ), ( center.z - halfDimZ ) ), tint, Vector2( uvSide.mins.x, uvSide.maxs.y ), ( -1.0f * Vector3::FORWARD ), Vector4( Vector3::RIGHT, 1.0f ) ),
		VertexBuilder( Vector3( ( center.x + halfDimX ), ( center.y + halfDimY ), ( center.z - halfDimZ ) ), tint, uvSide.maxs, ( -1.0f * Vector3::FORWARD ), Vector4( Vector3::RIGHT, 1.0f ) ),
		VertexBuilder( Vector3( ( center.x - halfDimX ), ( center.y - halfDimY ), ( center.z + halfDimZ ) ), tint, uvSide.mins, ( -1.0f * Vector3::RIGHT ), Vector4( ( -1.0f * Vector3::FORWARD ), 1.0f ) ),		// Left
		VertexBuilder( Vector3( ( center.x - halfDimX ), ( center.y - halfDimY ), ( center.z - halfDimZ ) ), tint, Vector2( uvSide.maxs.x, uvSide.mins.y ), ( -1.0f * Vector3::RIGHT ), Vector4( ( -1.0f * Vector3::FORWARD ), 1.0f ) ),
		VertexBuilder( Vector3( ( center.x - halfDimX ), ( center.y + halfDimY ), ( center.z + halfDimZ ) ), tint, Vector2( uvSide.mins.x, uvSide.maxs.y ), ( -1.0f * Vector3::RIGHT ), Vector4( ( -1.0f * Vector3::FORWARD ), 1.0f ) ),
		VertexBuilder( Vector3( ( center.x - halfDimX ), ( center.y + halfDimY ), ( center.z - halfDimZ ) ), tint, uvSide.maxs, ( -1.0f * Vector3::RIGHT ), Vector4( ( -1.0f * Vector3::FORWARD ), 1.0f ) ),
		VertexBuilder( Vector3( ( center.x + halfDimX ), ( center.y - halfDimY ), ( center.z + halfDimZ ) ), tint, uvSide.mins, Vector3::FORWARD, Vector4( ( -1.0f * Vector3::RIGHT ), 1.0f ) ),		// Back
		VertexBuilder( Vector3( ( center.x - halfDimX ), ( center.y - halfDimY ), ( center.z + halfDimZ ) ), tint, Vector2( uvSide.maxs.x, uvSide.mins.y ), Vector3::FORWARD, Vector4( ( -1.0f * Vector3::RIGHT ), 1.0f ) ),
		VertexBuilder( Vector3( ( center.x + halfDimX ), ( center.y + halfDimY ), ( center.z + halfDimZ ) ), tint, Vector2( uvSide.mins.x, uvSide.maxs.y ), Vector3::FORWARD, Vector4( ( -1.0f * Vector3::RIGHT ), 1.0f ) ),
		VertexBuilder( Vector3( ( center.x - halfDimX ), ( center.y + halfDimY ), ( center.z + halfDimZ ) ), tint, uvSide.maxs, Vector3::FORWARD, Vector4( ( -1.0f * Vector3::RIGHT ), 1.0f ) ),
		VertexBuilder( Vector3( ( center.x - halfDimX ), ( center.y + halfDimY ), ( center.z - halfDimZ ) ), tint, uvTop.mins, Vector3::UP, Vector4( Vector3::RIGHT, 1.0f ) ),		// Top
		VertexBuilder( Vector3( ( center.x + halfDimX ), ( center.y + halfDimY ), ( center.z - halfDimZ ) ), tint, Vector2( uvTop.maxs.x, uvTop.mins.y ), Vector3::UP, Vector4( Vector3::RIGHT, 1.0f ) ),
		VertexBuilder( Vector3( ( center.x - halfDimX ), ( center.y + halfDimY ), ( center.z + halfDimZ ) ), tint, Vector2( uvTop.mins.x, uvTop.maxs.y ), Vector3::UP, Vector4( Vector3::RIGHT, 1.0f ) ),
		VertexBuilder( Vector3( ( center.x + halfDimX ), ( center.y + halfDimY ), ( center.z + halfDimZ ) ), tint, uvTop.maxs, Vector3::UP, Vector4( Vector3::RIGHT, 1.0f ) )
	};
	unsigned int indices[ 36 ] = {
		0, 1, 2, 1, 3, 2,		// Bottom
		4, 5, 6, 5, 7, 6,		// Right
		8, 9, 10, 9, 11, 10,	// Front
		12, 13, 14, 13, 15, 14,	// Left
		16, 17, 18, 17, 19, 18,	// Back
		20, 21, 22, 21, 23, 22	// Top
	};

	MeshBuilder cubeMeshBuilder;
	cubeMeshBuilder.Begin( DrawPrimitiveType::TRIANGLES );
	cubeMeshBuilder.PushVertices( 24, vertices );
	cubeMeshBuilder.PushIndices( 36, indices );
	cubeMeshBuilder.End();

	return cubeMeshBuilder;
}

MeshBuilder Renderer::MakeTexturedCubeMesh( const AABB3& bounds, const Texture& texture, const Rgba& tint, const AABB2& uvTop /* = AABB2::ZERO_TO_ONE */, const AABB2& uvSide /* = AABB2::ZERO_TO_ONE */, const AABB2& uvBottom /* = AABB2::ZERO_TO_ONE */ ) const
{
	Vector3 center = bounds.GetCenter();
	Vector3 dimensions = bounds.GetDimensions();
	return MakeTexturedCubeMesh( center, dimensions, texture, tint, uvTop, uvSide, uvBottom );
}

MeshBuilder Renderer::MakeOBBMesh( const OBB3& bounds, const Rgba& color ) const
{
	Vector3 center = bounds.m_center;
	Vector3 right = bounds.m_right;
	Vector3 up = bounds.m_up;
	Vector3 forward = bounds.m_forward;

	MeshBuilder obbMeshBuilder = MakeOBBMesh( center, right, up, forward, color );
	return obbMeshBuilder;
}

MeshBuilder  Renderer::MakeOBBMesh( const Vector3& center, const Vector3& right, const Vector3& up, const Vector3& forward, const Rgba& color ) const
{
	VertexBuilder vertices[ 24 ] = {
		VertexBuilder( ( center - right - up - forward ), color, Vector2::ZERO, ( -1.0f * up ), Vector4( right, -1.0f ) ),		// Bottom
		VertexBuilder( ( center - right - up + forward ), color, Vector2::ZERO, ( -1.0f * up ), Vector4( right, -1.0f ) ),
		VertexBuilder( ( center + right - up - forward ), color, Vector2::ZERO, ( -1.0f * up ), Vector4( right, -1.0f ) ),
		VertexBuilder( ( center + right - up + forward ), color, Vector2::ZERO, ( -1.0f * up ), Vector4( right, -1.0f ) ),
		VertexBuilder( ( center + right - up - forward ), color, Vector2::ZERO, right, Vector4( forward, 1.0f ) ),		// Right
		VertexBuilder( ( center + right - up + forward ), color, Vector2::ZERO, right, Vector4( forward, 1.0f ) ),
		VertexBuilder( ( center + right + up - forward ), color, Vector2::ZERO, right, Vector4( forward, 1.0f ) ),
		VertexBuilder( ( center + right + up + forward ), color, Vector2::ZERO, right, Vector4( forward, 1.0f ) ),
		VertexBuilder( ( center - right - up - forward ), color, Vector2::ZERO, ( -1.0f * forward ), Vector4( right, 1.0f ) ),		// Front
		VertexBuilder( ( center + right - up - forward ), color, Vector2::ZERO, ( -1.0f * forward ), Vector4( right, 1.0f ) ),
		VertexBuilder( ( center - right + up - forward ), color, Vector2::ZERO, ( -1.0f * forward ), Vector4( right, 1.0f ) ),
		VertexBuilder( ( center + right + up - forward ), color, Vector2::ZERO, ( -1.0f * forward ), Vector4( right, 1.0f ) ),
		VertexBuilder( ( center - right - up + forward ), color, Vector2::ZERO, ( -1.0f * right ), Vector4( ( -1.0f * forward ), 1.0f ) ),		// Left
		VertexBuilder( ( center - right - up - forward ), color, Vector2::ZERO, ( -1.0f * right ), Vector4( ( -1.0f * forward ), 1.0f ) ),
		VertexBuilder( ( center - right + up + forward ), color, Vector2::ZERO, ( -1.0f * right ), Vector4( ( -1.0f * forward ), 1.0f ) ),
		VertexBuilder( ( center - right + up - forward ), color, Vector2::ZERO, ( -1.0f * right ), Vector4( ( -1.0f * forward ), 1.0f ) ),
		VertexBuilder( ( center + right - up + forward ), color, Vector2::ZERO, forward, Vector4( ( -1.0f * right ), 1.0f ) ),		// Back
		VertexBuilder( ( center - right - up + forward ), color, Vector2::ZERO, forward, Vector4( ( -1.0f * right ), 1.0f ) ),
		VertexBuilder( ( center + right + up + forward ), color, Vector2::ZERO, forward, Vector4( ( -1.0f * right ), 1.0f ) ),
		VertexBuilder( ( center - right + up + forward ), color, Vector2::ZERO, forward, Vector4( ( -1.0f * right ), 1.0f ) ),
		VertexBuilder( ( center - right + up - forward ), color, Vector2::ZERO, up, Vector4( right, 1.0f ) ),		// Top
		VertexBuilder( ( center + right + up - forward ), color, Vector2::ZERO, up, Vector4( right, 1.0f ) ),
		VertexBuilder( ( center - right + up + forward ), color, Vector2::ZERO, up, Vector4( right, 1.0f ) ),
		VertexBuilder( ( center + right + up + forward ), color, Vector2::ZERO, up, Vector4( right, 1.0f ) )
	};
	unsigned int indices[ 36 ] = {
		0, 1, 2, 1, 3, 2,		// Bottom
		4, 5, 6, 5, 7, 6,		// Right
		8, 9, 10, 9, 11, 10,	// Front
		12, 13, 14, 13, 15, 14,	// Left
		16, 17, 18, 17, 19, 18,	// Back
		20, 21, 22, 21, 23, 22	// Top
	};

	MeshBuilder obbMeshBuilder;
	obbMeshBuilder.Begin( DrawPrimitiveType::TRIANGLES );
	obbMeshBuilder.PushVertices( 24, vertices );
	obbMeshBuilder.PushIndices( 36, indices );
	obbMeshBuilder.End();

	return obbMeshBuilder;
}

MeshBuilder Renderer::MakeGridMesh( const Vector3& center, const AABB2& bounds, const Vector3& up, const Vector3& right, const IntVector2& numUnits, const Rgba& color ) const
{
	MeshBuilder gridMeshBuilder;
	gridMeshBuilder.Begin( DrawPrimitiveType::LINES );

	Vector2 dimensions = bounds.GetDimensions();
	Vector3 mins = center - ( 0.5f * dimensions.x * right ) - ( 0.5f * dimensions.y * up );

	Vector2 unitDimensions = dimensions;
	if ( ( numUnits.x == 0 ) )
	{
		ConsolePrintf( Rgba::RED, "ERROR: Renderer::MakeGridMesh - number of units in X is zero. Aborting..." );
		return gridMeshBuilder;
	}
	if ( ( numUnits.y == 0 ) )
	{
		ConsolePrintf( Rgba::RED, "ERROR: Renderer::MakeGridMesh - number of units in Y is zero. Aborting..." );
		return gridMeshBuilder;
	}
	unitDimensions /= ConvertIntVector2ToVector2( numUnits );

	for ( int y = 0; y < numUnits.y; y++ )
	{
		for ( int x = 0; x < numUnits.x; x++ )
		{
			MeshBuilder quadMeshBuilder;
			quadMeshBuilder.Begin( DrawPrimitiveType::LINES );

			Vector2 indexAsVec2 = ConvertIntVector2ToVector2( IntVector2( x, y ) );
			Vector3 quadMins = mins + ( unitDimensions.x * indexAsVec2.x * right ) + ( unitDimensions.y * indexAsVec2.y * up );

			Vertex_3DPCU vertices[ 4 ] = 
			{
				Vertex_3DPCU( quadMins, color, Vector2::ZERO ),
				Vertex_3DPCU( ( quadMins + ( unitDimensions.x * right ) ), color, Vector2::ZERO ),
				Vertex_3DPCU( ( quadMins + ( unitDimensions.y * up ) ), color, Vector2::ZERO ),
				Vertex_3DPCU( ( quadMins + ( unitDimensions.x * right ) + ( unitDimensions.y * up ) ), color, Vector2::ZERO )
			};
			quadMeshBuilder.PushVertices( 4, vertices );

			unsigned int indices[ 8 ] =
			{
				0, 1,
				1, 3,
				3, 2,
				2, 0
			};
			quadMeshBuilder.PushIndices( 8, indices );
			quadMeshBuilder.End();

			gridMeshBuilder.MergeMesh( quadMeshBuilder );
		}
	}

	gridMeshBuilder.End();

	return gridMeshBuilder;
}

MeshBuilder Renderer::MakePlaneMesh( const Vector3& bottomLeft, const Vector3& upVector, const Vector3& rightVector, const Vector2& dimensions, const Rgba& tint, const AABB2& uv ) const
{
	Vector3 normal = CrossProduct( upVector, rightVector );

	Vertex_3DLit vertices[ 4 ] =
	{
		Vertex_3DLit( bottomLeft, tint, uv.mins, normal, Vector4( rightVector, 1.0f ) ),
		Vertex_3DLit( ( bottomLeft + ( rightVector * dimensions.x ) ), tint, uv.GetMaxXMinY(), normal, Vector4( rightVector, 1.0f ) ),
		Vertex_3DLit( ( bottomLeft + ( upVector * dimensions.y ) ), tint, uv.GetMinXMaxY(), normal, Vector4( rightVector, 1.0f ) ),
		Vertex_3DLit( ( bottomLeft + ( upVector * dimensions.y ) + ( rightVector * dimensions.x ) ), tint, uv.maxs, normal, Vector4( rightVector, 1.0f ) )
	};

	unsigned int indices[ 6 ] = { 0, 1, 2, 1, 3, 2 };
	
	MeshBuilder planeBuilder;
	planeBuilder.Begin( DrawPrimitiveType::TRIANGLES );
	planeBuilder.PushVertex( VertexBuilder( vertices[ 0 ].m_position, vertices[ 0 ].m_color, vertices[ 0 ].m_UVs, vertices[ 0 ].m_normal, vertices[ 0 ].m_tangent ) );
	planeBuilder.PushVertex( VertexBuilder( vertices[ 1 ].m_position, vertices[ 1 ].m_color, vertices[ 1 ].m_UVs, vertices[ 1 ].m_normal, vertices[ 1 ].m_tangent ) );
	planeBuilder.PushVertex( VertexBuilder( vertices[ 2 ].m_position, vertices[ 2 ].m_color, vertices[ 2 ].m_UVs, vertices[ 2 ].m_normal, vertices[ 2 ].m_tangent ) );
	planeBuilder.PushVertex( VertexBuilder( vertices[ 3 ].m_position, vertices[ 3 ].m_color, vertices[ 3 ].m_UVs, vertices[ 3 ].m_normal, vertices[ 3 ].m_tangent ) );
	planeBuilder.PushIndices( 6, indices );
	planeBuilder.End();

	return planeBuilder;
}

MeshBuilder Renderer::MakeUVSphereMesh( const Vector3& center, float radius, unsigned int numWedges, unsigned int numSlices, const Rgba& tint ) const
{
	MeshBuilder uvSphereBuilder;
	uvSphereBuilder.Begin( DrawPrimitiveType::TRIANGLES );
	uvSphereBuilder.SetColor( tint );

	uvSphereBuilder.AddSurfacePatch( SphereSurfacePatch, FloatRange( 0.0f, 1.0f ), FloatRange( 0.0f, 1.0f ), IntVector2( numWedges, numSlices ), center, radius );

	uvSphereBuilder.End();

	return uvSphereBuilder;
}

MeshBuilder Renderer::MakeBasisMesh( const Vector3& iBasis, const Vector3& jBasis, const Vector3& kBasis, const Vector3& magnitudes ) const
{
	MeshBuilder basisMeshBuilder;
	basisMeshBuilder.Begin( DrawPrimitiveType::LINES, false );

	basisMeshBuilder.SetColor( Rgba::RED );
	basisMeshBuilder.SetUVs( Vector2::ZERO );
	basisMeshBuilder.PushVertex( Vector3::ZERO );
	basisMeshBuilder.SetUVs( Vector2::ONE );
	basisMeshBuilder.PushVertex( magnitudes.x * iBasis );

	basisMeshBuilder.SetColor( Rgba::GREEN );
	basisMeshBuilder.SetUVs( Vector2::ZERO );
	basisMeshBuilder.PushVertex( Vector3::ZERO );
	basisMeshBuilder.SetUVs( Vector2::ONE );
	basisMeshBuilder.PushVertex( magnitudes.y * jBasis );

	basisMeshBuilder.SetColor( Rgba::BLUE );
	basisMeshBuilder.SetUVs( Vector2::ZERO );
	basisMeshBuilder.PushVertex( Vector3::ZERO );
	basisMeshBuilder.SetUVs( Vector2::ONE );
	basisMeshBuilder.PushVertex( magnitudes.z * kBasis );

	basisMeshBuilder.End();

	return basisMeshBuilder;
}

Vector2 Renderer::GetAnchorPointForAlignmentWithTextBox2D( const AABB2& boxBounds, const Vector2& alignment ) const
{
	Vector2 anchorPoint = Vector2( 
		Interpolate( boxBounds.mins.x, boxBounds.maxs.x, alignment.x ),
		Interpolate( boxBounds.maxs.y, boxBounds.mins.y, alignment.y )
	);

	return anchorPoint;
}

Vector3 Renderer::GetAnchorPointForAlignmentWithTextBox3D( const Vector3& boxMins, const Vector3& rightVector, const Vector2& boxDimensions, const Vector2& alignment ) const
{
	Vector3 cameraForward = m_currentCamera->GetForward();
	Vector3 rightVectorOrthoToCameraForward = rightVector - rightVector.GetComponentInDirection( cameraForward );
	rightVectorOrthoToCameraForward.GetNormalized();
	Vector3 upVector = CrossProduct( cameraForward, rightVectorOrthoToCameraForward );

	Vector3 anchorPoint = boxMins;
	anchorPoint = Interpolate( anchorPoint, ( anchorPoint + ( rightVector * boxDimensions.x ) ), alignment.x );
	anchorPoint = Interpolate( anchorPoint, ( anchorPoint + ( upVector * boxDimensions.y ) ), alignment.y );

	return anchorPoint;
}

void Renderer::DrawTextInBox2DOverrun( const std::vector< std::string >& linesOfText, const AABB2& boxBounds, float cellHeight, const Rgba& tint, float aspectScale, const BitmapFont* font, const Vector2& alignment ) const
{
	for ( unsigned int lineIndex = 0; lineIndex < linesOfText.size(); lineIndex++ )
	{
		Vector2 drawMins;
		Vector2 anchorPoint = GetAnchorPointForAlignmentWithTextBox2D( boxBounds, alignment );

		float lineWidth = font->GetStringWidth( linesOfText[ lineIndex ], cellHeight, aspectScale );

		drawMins.x	=	anchorPoint.x -		// left, center or right
						( alignment.x		// 0 for left, 0.5 for center, 1 for right
						* lineWidth );		// drawMins.x will be equal to the bounds.mins.x if left aligned

		// Calculate drawMins.y based on line height, line number, bounds.y and alignment
		drawMins.y	=	( anchorPoint.y +	// top, center or bottom
							( alignment.y *	// 0 for top, 0.5 for center, 1 for bottom 
								( cellHeight * static_cast< float >( linesOfText.size() ) )
							)
						)					// The Y value, as calculated till here, is the maxs.y for the first (topmost) line of text
						- ( static_cast< float >( lineIndex + 1 ) * cellHeight );

		DrawText2D( drawMins, linesOfText[ lineIndex ], cellHeight, tint, aspectScale, font );
	}
}

void Renderer::DrawTextInBox2DShrinkToFit( const std::vector< std::string >& linesOfText, const AABB2& boxBounds, float cellHeight, const Rgba& tint, float aspectScale, const BitmapFont* font, const Vector2& alignment ) const
{
	float maxLineWidth = 0.0f;
	for ( unsigned int lineIndex = 0; lineIndex < linesOfText.size(); lineIndex++ )
	{
		float lineWidth = font->GetStringWidth( linesOfText[ lineIndex ], cellHeight, aspectScale );
		if ( lineWidth > maxLineWidth )
		{
			maxLineWidth = lineWidth;
		}
	}
	if ( maxLineWidth > boxBounds.GetDimensions().x )
	{
		cellHeight *= boxBounds.GetDimensions().x / maxLineWidth;		// Compress text based on max horizontal width
	}
	float totalTextHeight = cellHeight * static_cast< float > ( linesOfText.size() );
	if ( totalTextHeight > boxBounds.GetDimensions().y )
	{
		cellHeight *= boxBounds.GetDimensions().y / totalTextHeight;
	}

	for ( unsigned int lineIndex = 0; lineIndex < linesOfText.size(); lineIndex++ )
	{
		Vector2 drawMins;
		Vector2 anchorPoint = GetAnchorPointForAlignmentWithTextBox2D( boxBounds, alignment );

		float lineWidth = font->GetStringWidth( linesOfText[ lineIndex ], cellHeight, aspectScale );

		drawMins.x	=	anchorPoint.x -		// left, center or right
			( alignment.x		// 0 for left, 0.5 for center, 1 for right
				* lineWidth );		// drawMins.x will be equal to the bounds.mins.x if left aligned

									// Calculate drawMins.y based on line height, line number, bounds.y and alignment
		drawMins.y	=	( anchorPoint.y +	// top, center or bottom
			( alignment.y *	// 0 for top, 0.5 for center, 1 for bottom 
			( cellHeight * static_cast< float >( linesOfText.size() ) )
				)
			)					// The Y value, as calculated till here, is the maxs.y for the first (topmost) line of text
			- ( static_cast< float >( lineIndex + 1 ) * cellHeight );

		DrawText2D( drawMins, linesOfText[ lineIndex ], cellHeight, tint, aspectScale, font );
	}
}

void Renderer::DrawTextInBox2DWordWrap( std::vector< std::string >& linesOfText, const AABB2& boxBounds, float cellHeight, const Rgba& tint, float aspectScale, const BitmapFont* font, const Vector2& alignment ) const
{
	for ( unsigned int lineIndex = 0; lineIndex < linesOfText.size(); lineIndex++ )		// CAUTION: This loop may resize the vector "linesOfText"!
	{
		float lineWidth = font->GetStringWidth( linesOfText[ lineIndex ], cellHeight, aspectScale );

		if ( lineWidth > boxBounds.GetDimensions().x )
		{
			TokenizedString lineOfTextTokenized = TokenizedString( linesOfText[ lineIndex ], " " );
			std::vector< std::string > wordsInLine = lineOfTextTokenized.GetTokens();
			float currentLength = font->GetStringWidth( wordsInLine[ 0 ], cellHeight, aspectScale );	// We don't have to do anything if the first word is too long
			size_t currentCharLength = wordsInLine[ 0 ].size();

			for ( unsigned int wordIndex = 1; wordIndex < wordsInLine.size(); wordIndex++ )		// Start from 1 since we've already calculated the first word's length
			{
				float newLength = currentLength + font->GetStringWidth( ( " " + wordsInLine[ wordIndex ] ), cellHeight, aspectScale );
				size_t newCharLength = currentCharLength + 1 + wordsInLine[ wordIndex ].size();
				if ( newLength > boxBounds.GetDimensions().x )
				{
					// Push the substring from the beginning of this word to the end to the next index of the linesOfText vector
					std::string overflowSubstring = linesOfText[ lineIndex ].substr( currentCharLength + 1 );
					linesOfText[ lineIndex ] = linesOfText[ lineIndex ].substr( 0, currentCharLength );		// Exclude the space preceding the word as well
					linesOfText.insert( ( linesOfText.begin() + ( lineIndex + 1 ) ), overflowSubstring );
					break;
				}
				else
				{
					currentLength = newLength;
					currentCharLength = newCharLength;
				}
			}
		}
	}

	float totalTextHeight = cellHeight * static_cast< float > ( linesOfText.size() );
	if ( totalTextHeight > boxBounds.GetDimensions().y )
	{
		cellHeight *= boxBounds.GetDimensions().y / totalTextHeight;
	}

	for ( unsigned int lineIndex = 0; lineIndex < linesOfText.size(); lineIndex++ )
	{
		Vector2 drawMins;
		Vector2 anchorPoint = GetAnchorPointForAlignmentWithTextBox2D( boxBounds, alignment );

		float lineWidth = font->GetStringWidth( linesOfText[ lineIndex ], cellHeight, aspectScale );

		drawMins.x	=	anchorPoint.x -		// left, center or right
			( alignment.x		// 0 for left, 0.5 for center, 1 for right
				* lineWidth );		// drawMins.x will be equal to the bounds.mins.x if left aligned

									// Calculate drawMins.y based on line height, line number, bounds.y and alignment
		drawMins.y	=	( anchorPoint.y +	// top, center or bottom
			( alignment.y *	// 0 for top, 0.5 for center, 1 for bottom 
			( cellHeight * static_cast< float >( linesOfText.size() ) )
				)
			)					// The Y value, as calculated till here, is the maxs.y for the first (topmost) line of text
			- ( static_cast< float >( lineIndex + 1 ) * cellHeight );

		DrawText2D( drawMins, linesOfText[ lineIndex ], cellHeight, tint, aspectScale, font );
	}
}

void Renderer::DrawTextInBox3DShrinkToFit( const std::vector<std::string>& linesOfText, const Vector3& position, const Vector3& rightVector, const Vector2& boxDimensions, float cellHeight, const Rgba& boxColor, const Rgba& tint, float aspectScale, const BitmapFont* font, const Vector2& alignment, bool usesCameraForward /* = true */, const Vector3& upVector /* = Vector3::UP */ ) const
{
	float maxLineWidth = 0.0f;
	for ( unsigned int lineIndex = 0; lineIndex < linesOfText.size(); lineIndex++ )
	{
		float lineWidth = font->GetStringWidth( linesOfText[ lineIndex ], cellHeight, aspectScale );
		if ( lineWidth > maxLineWidth )
		{
			maxLineWidth = lineWidth;
		}
	}
	if ( maxLineWidth > boxDimensions.x )
	{
		cellHeight *= boxDimensions.x / maxLineWidth;		// Compress text based on max horizontal width
	}
	float totalTextHeight = cellHeight * static_cast< float > ( linesOfText.size() );
	if ( totalTextHeight > boxDimensions.y )
	{
		cellHeight *= boxDimensions.y / totalTextHeight;
	}

	Vector3 drawUpVector = ( usesCameraForward )? GetCurrentViewportUpVector() : upVector;
	Vector3 anchorPoint = GetAnchorPointForAlignmentWithTextBox3D( position, rightVector, boxDimensions, alignment );
	
	for ( unsigned int lineIndex = 0; lineIndex < linesOfText.size(); lineIndex++ )
	{
		float lineWidth = font->GetStringWidth( linesOfText[ lineIndex ], cellHeight, aspectScale );

		Vector3 drawPosition = anchorPoint - ( rightVector * ( alignment.x * lineWidth ) );
		drawPosition = ( drawPosition + ( drawUpVector * ( alignment.y * ( cellHeight * static_cast< float >( linesOfText.size() ) ) ) ) ) - ( drawUpVector * ( cellHeight * static_cast< float >( lineIndex + 1 ) ) );

		DrawText3D( linesOfText[ lineIndex ], drawPosition, rightVector, cellHeight, tint, aspectScale, font, usesCameraForward, upVector );
	}
}

#pragma endregion

void Renderer::ChangeRenderColor( const Rgba& color ) const
{
	//glColor4ub( color.r, color.g, color.b, color.a );
	UNIMPLEMENTED()
}

void Renderer::PushMatrix() const
{
	//glPushMatrix();
	UNIMPLEMENTED()
}

void Renderer::PopMatrix() const
{
	//glPopMatrix();
	UNIMPLEMENTED()
}

void Renderer::Translate( float xTranslation, float yTranslation, float zTranslation ) const
{
	//glTranslatef( xTranslation, yTranslation, zTranslation );
	UNIMPLEMENTED()
}

void Renderer::Rotate( float angleDegrees, float xAxis, float yAxis, float zAxis ) const
{
	//glRotatef( angleDegrees, xAxis, yAxis, zAxis );
	UNIMPLEMENTED()
}

void Renderer::Scale( float xScale, float yScale, float zScale ) const
{
	//glScalef( xScale, yScale, zScale );
	UNIMPLEMENTED()
}

void Renderer::SetProjectionMatrix( const Matrix44& projectionMatrix ) const
{
	m_currentCamera->SetProjection( projectionMatrix );
}

void Renderer::SetOrtho3D( const Vector3& mins, const Vector3& maxs ) const
{
	// Range-mapping the view space into clip coordinates
	float orthoMatrix[ 16 ] = {
		( 2.0f / ( maxs.x - mins.x ) ), 0.0f, 0.0f, 0.0f,
		0.0f, ( 2.0f / ( maxs.y - mins.y ) ), 0.0f, 0.0f,
		0.0f, 0.0f, ( 2.0f / ( maxs.z - mins.z ) ), 0.0f,
		-( ( mins.x + maxs.x ) / ( maxs.x - mins.x ) ), -( ( mins.y + maxs.y ) / ( maxs.y - mins.y ) ), -( ( mins.z + maxs.z ) / ( maxs.z - mins.z ) ), 1.0f
	};

	m_currentCamera->SetProjection( Matrix44( orthoMatrix ) );
}

void Renderer::SetOrtho( const Vector2& bottomLeft, const Vector2& topRight ) const
{
	SetOrtho3D( Vector3( bottomLeft.x, bottomLeft.y, -1.0f ), Vector3( topRight.x, topRight.y, 1.0f ) );
}
void Renderer::SetPolygonMode( RendererPolygonMode newPolygonMode )
{
	m_defaultShaderPass->SetFillMode( newPolygonMode );
}

void Renderer::SetBlendMode( RendererBlendMode newBlendMode )
{
	switch ( newBlendMode )
	{
		case RendererBlendMode::ALPHA:
		{
			m_defaultShaderPass->EnableAlphaBlending( RendererBlendOperation::OPERATION_ADD, RendererBlendFactor::FACTOR_SRC_ALPHA, RendererBlendFactor::FACTOR_ONE_MINUS_SRC_ALPHA );
			break;
		}
		case RendererBlendMode::ADDITIVE:
		{
			m_defaultShaderPass->EnableAlphaBlending( RendererBlendOperation::OPERATION_ADD, RendererBlendFactor::FACTOR_SRC_ALPHA, RendererBlendFactor::FACTOR_ONE );
			break;
		}
		default:
		{
			break;
		}
	}
}

void Renderer::SetCamera( Camera* camera )
{
	if ( m_currentCamera == nullptr )
	{
		m_currentCamera = camera;
	}
	else if ( camera != m_currentCamera )
	{
		m_currentCamera->GetFrameBuffer()->MarkDirty();

		if ( camera == nullptr )
		{
			m_currentCamera = m_defaultCamera;
		}
		else
		{
			m_currentCamera = camera;
		}
	}
	m_currentCamera->Finalize();

	// Map viewport bounds to render target
	Vector2 targetDimensions = Vector2( Window::GetWidthF(), Window::GetHeightF() );
	if ( m_currentCamera->GetFrameBuffer()->GetColorTargetCount() > 0U )
	{
		targetDimensions = m_currentCamera->GetFrameBuffer()->m_colorTargets[ 0 ]->GetDimensionsF();
	}
	else if ( m_currentCamera->GetFrameBuffer()->m_depthStencilTarget != nullptr )
	{
		targetDimensions = m_currentCamera->GetFrameBuffer()->m_depthStencilTarget->GetDimensionsF();
	}

	AABB2 viewport = m_currentCamera->GetViewport();
	
	viewport.mins = Vector2( ( viewport.mins.x * targetDimensions.x ), ( viewport.mins.y * targetDimensions.y ) );
	viewport.maxs = Vector2( ( viewport.maxs.x * targetDimensions.x ), ( viewport.maxs.y * targetDimensions.y ) );
	IntVector2 viewportDimensions = ConvertVector2ToIntVector2( viewport.GetDimensions() );
	glViewport( static_cast< int >( viewport.mins.x ), static_cast< int >( viewport.mins.y ), viewportDimensions.x, viewportDimensions.y );

	// Pass the camera info to the GPU
	if ( m_currentCameraUBO == nullptr )
	{
		m_currentCameraUBO = new UniformBuffer;
	}
	m_currentCameraUBO->SetDataAndCopyToGPU< CameraUBO >( m_currentCamera->MakeUniformBufferStruct() );
	glBindBufferBase( GL_UNIFORM_BUFFER, UNIFORM_BUFFER_CAMERA_BIND, m_currentCameraUBO->m_handle );
}

void Renderer::UseDefaultCamera()
{
	SetCamera( m_defaultCamera );
}

void Renderer::ClearScreen( const Rgba& clearColor ) const
{
	float rNormalized, gNormalized, bNormalized, aNormalized;
	clearColor.GetAsFloats( rNormalized, gNormalized, bNormalized, aNormalized );

	glClearColor( rNormalized, gNormalized, bNormalized, aNormalized );
	glClear( GL_COLOR_BUFFER_BIT );
}

#pragma region Screenshot

void Renderer::TakeScreenshotAtEndOfFrame()
{
	m_takeScreenshotThisFrame = true;
}

/* Screenshot standalone struct/function */
struct SaveScreenshotThreadArgs
{

public:
	GLubyte* m_imageData = nullptr;
	IntVector2 m_imageDimensions = IntVector2::ZERO;
};

void SaveScreenshotWorker( void* saveScreenshotDataVoid )
{
	SaveScreenshotThreadArgs* saveScreenshotData = reinterpret_cast< SaveScreenshotThreadArgs* >( saveScreenshotDataVoid );

	CreateDirectoryIfNotExists( "Screenshots" );
	stbi_flip_vertically_on_write( 1 );

	stbi_write_png(
		"Screenshots/Screenshot.png",
		saveScreenshotData->m_imageDimensions.x, saveScreenshotData->m_imageDimensions.y,
		4,
		saveScreenshotData->m_imageData,
		0
	);

	std::string timeStamp = GetTimestampForFilename();
	stbi_write_png(
		( "Screenshots/Screenshot_" + timeStamp  + ".png" ).c_str(),
		saveScreenshotData->m_imageDimensions.x, saveScreenshotData->m_imageDimensions.y,
		4,
		saveScreenshotData->m_imageData,
		0
	);

	ConsolePrintf( "Screenshot saved." );

	delete[] saveScreenshotData->m_imageData;
	delete saveScreenshotData;
}

void Renderer::SaveScreenshot() const
{
	IntVector2 imageDimensions = m_currentCamera->GetFrameBuffer()->m_colorTargets[ 0 ]->m_dimensions;	// Uses the primary color target, as this is always the final image

	// GL API is not thread-safe
	GLubyte* imageData = new unsigned char[ 4 * imageDimensions.x * imageDimensions.y ];
	glReadPixels( 0, 0, imageDimensions.x, imageDimensions.y, GL_RGBA, GL_UNSIGNED_BYTE, imageData );

	SaveScreenshotThreadArgs* args = new SaveScreenshotThreadArgs();
	args->m_imageData = imageData;
	args->m_imageDimensions = imageDimensions;
	ThreadCreateAndDetach( SaveScreenshotWorker, args );
}

#pragma endregion
