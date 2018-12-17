#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Threading.hpp"
#include "Engine/Renderer/GLFunctionBinding.hpp"
#include "Engine/Renderer/GLUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/TextureCube.hpp"

/* ASYNC */
struct TextureCubeAsyncLoadData
{
public:
	TextureCube* m_textureCubeToLoad = nullptr;
	const char* m_filename = nullptr;
};

void CreateTextureCubeFromFileAsync( void* args )
{
	TextureCubeAsyncLoadData* loadData = reinterpret_cast< TextureCubeAsyncLoadData* >( args );

	Image image = Image( loadData->m_filename );

	TextureCubeAsyncLoadRequest* loadInfo = new TextureCubeAsyncLoadRequest( image, loadData->m_textureCubeToLoad );
	Renderer::GetInstance()->AddToTextureCubeLoadQueue( loadInfo );

	delete loadData->m_filename;
	delete loadData;
}

/* INTERNAL FUNCTIONS */
static void BindImageToSide( TextureCubeSide side, const Image& image, unsigned int size, unsigned int ox, unsigned int oy, GLenum channels, GLenum pixel_layout ) 
{
	const void* ptr = image.GetTexelReference( ox, oy ); 
	glTexSubImage2D( Renderer::GetInstance()->GetGLCubeSide( side ),
		0,          // mip_level
		0, 0,       // offset
		size, size, 
		channels, 
		pixel_layout, 
		ptr ); 

	GL_CHECK_ERROR(); 
}

static void BindImage( TextureCubeSide side, const Image& image, GLenum channels, GLenum pixel_layout ) 
{
	Image copy = Image( image ); 
	BindImageToSide( side, copy, copy.GetDimensions().x, 0, 0, channels, pixel_layout ); 
}

TextureCube::TextureCube()
{

}

TextureCube::~TextureCube()
{
	Cleanup(); 
}

void TextureCube::Cleanup()
{
	if ( IsValid() )
	{
		glDeleteTextures( 1, &m_handle );
		m_handle = NULL; 
	}

	m_size = 0; 
	m_format = TextureFormat::TEXTURE_FORMAT_INVALID; 
}

bool TextureCube::MakeFromImages( const Image* images )
{
	if (m_handle == NULL) {
		glGenTextures( 1, &m_handle ); 
		if ( !IsValid() )
		{
			return false;
		}
	}

	m_size = images[0].GetDimensions().x; 
	m_format = TextureFormat::TEXTURE_FORMAT_RGBA8; 

	GLenum internal_format; 
	GLenum channels; 
	GLenum pixel_layout; 
	TextureFormatToGLFormats( &internal_format, &channels, &pixel_layout, m_format ); 

	// bind it; 
	glBindTexture( GL_TEXTURE_CUBE_MAP, m_handle ); 

	glTexStorage2D( GL_TEXTURE_CUBE_MAP, 1, internal_format, m_size, m_size ); 
	glPixelStorei( GL_UNPACK_ROW_LENGTH, 0 ); 

	// bind the image to the side; 
	BindImage( TextureCubeSide::TEXCUBE_RIGHT,  images[0], channels, pixel_layout ); 
	BindImage( TextureCubeSide::TEXCUBE_LEFT,   images[1], channels, pixel_layout ); 
	BindImage( TextureCubeSide::TEXCUBE_TOP,    images[2], channels, pixel_layout ); 
	BindImage( TextureCubeSide::TEXCUBE_BOTTOM, images[3], channels, pixel_layout ); 
	BindImage( TextureCubeSide::TEXCUBE_FRONT,  images[4], channels, pixel_layout ); 
	BindImage( TextureCubeSide::TEXCUBE_BACK,   images[5], channels, pixel_layout ); 

	return GLSucceeded(); 
}

bool TextureCube::MakeFromImage( Image const &image )
{
	unsigned int width = image.GetDimensions().x; 
	unsigned int size = width / 4; 

	glGenTextures( 1, &m_handle ); 
	if ( !IsValid() )
	{
		return false;
	}

	m_size = size; 
	m_format = TextureFormat::TEXTURE_FORMAT_RGBA8; 
	Image copy = Image( image );  

	GLenum internal_format; 
	GLenum channels; 
	GLenum pixel_layout; 
	TextureFormatToGLFormats( &internal_format, &channels, &pixel_layout, m_format ); 
	GL_CHECK_ERROR(); 

	// bind it; 
	glBindTexture( GL_TEXTURE_CUBE_MAP, m_handle ); 
	glTexStorage2D( GL_TEXTURE_CUBE_MAP, 1, internal_format, m_size, m_size ); 
	GL_CHECK_ERROR(); 

	glPixelStorei( GL_UNPACK_ROW_LENGTH, copy.GetDimensions().x ); 
	GL_CHECK_ERROR(); 

	// bind the image to the side; 
	BindImageToSide( TextureCubeSide::TEXCUBE_RIGHT,  copy, m_size, m_size * 2, m_size * 1, channels, pixel_layout ); 
	BindImageToSide( TextureCubeSide::TEXCUBE_LEFT,   copy, m_size, m_size * 0, m_size * 1, channels, pixel_layout ); 
	BindImageToSide( TextureCubeSide::TEXCUBE_TOP,    copy, m_size, m_size * 1, m_size * 0, channels, pixel_layout ); 
	BindImageToSide( TextureCubeSide::TEXCUBE_BOTTOM, copy, m_size, m_size * 1, m_size * 2, channels, pixel_layout ); 
	BindImageToSide( TextureCubeSide::TEXCUBE_FRONT,  copy, m_size, m_size * 1, m_size * 1, channels, pixel_layout ); 
	BindImageToSide( TextureCubeSide::TEXCUBE_BACK,   copy, m_size, m_size * 3, m_size * 1, channels, pixel_layout ); 

	glPixelStorei( GL_UNPACK_ROW_LENGTH, 0 ); 

	return GLSucceeded(); 
}

bool TextureCube::MakeFromImage( const char* filename, bool loadAsync /* = true */ )
{
	if ( loadAsync )
	{
		TextureCube* defaultTextureCube = Renderer::GetInstance()->GetDefaultTextureCube();
		if ( defaultTextureCube == nullptr )
		{
			ERROR_AND_DIE( "ERROR: TextureCube::MakeFromImage(): Cannot load Default Texture Cube async! Aborting..." );
		}

		char* filenameCopy = new char[ strlen( filename ) + 1 ];
		memcpy( filenameCopy, filename, strlen( filename ) );
		filenameCopy[ strlen( filename ) ] = '\0';

		TextureCubeAsyncLoadData* loadData = new TextureCubeAsyncLoadData();
		loadData->m_textureCubeToLoad = this;
		loadData->m_filename = filenameCopy;

		ThreadCreateAndDetach( CreateTextureCubeFromFileAsync, loadData );
		m_handle = defaultTextureCube->m_handle;
		m_size = defaultTextureCube->m_size;
		m_format = defaultTextureCube->m_format;

		return true;
	}
	else
	{
		Image image = Image( std::string( filename ) );
		return MakeFromImage( image ); 
	}
}
