#include "Engine/Core/Image.hpp"
#include "Engine/Core/Threading.hpp"
#include "Engine/Core/ThreadSafeTypes.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/TextureAsync.hpp"
#include "ThirdParty/stb/stb_image.h"

// Must do this as a blocking call since STBI's flip state must be consistent throughout for each image loaded (multithreaded)
SpinLock g_stbiFlipSpinLock;
Image LoadFlippedImageBlocking( const char* filename )
{
	g_stbiFlipSpinLock.Enter();

	/* BEGIN CRITICAL SECTION */
	stbi_set_flip_vertically_on_load( 1 );
	Image image = Image( filename );
	stbi_set_flip_vertically_on_load( 0 );
	/* END CRITICAL SECTION */

	g_stbiFlipSpinLock.Leave();

	return image;
}

void CreateTextureFromFileAsync( void* args )
{
	TextureAsyncLoadData* loadData = reinterpret_cast< TextureAsyncLoadData* >( args );

	Image image = LoadFlippedImageBlocking( loadData->m_filename );

	TextureAsyncLoadRequest* loadInfo = new TextureAsyncLoadRequest( image, loadData->m_textureToLoad, loadData->m_mipLevels );
	Renderer::GetInstance()->AddToTextureLoadQueue( loadInfo );

	delete loadData->m_filename;
	delete loadData;
}
