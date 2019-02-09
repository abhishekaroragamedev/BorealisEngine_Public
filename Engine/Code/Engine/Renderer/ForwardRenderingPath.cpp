#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Window.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/ForwardRenderingPath.hpp"
#include "Engine/Renderer/Framebuffer.hpp"
#include "Engine/Renderer/GLUtils.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/Mesh.hpp"
#include "Engine/OculusRift/OVRContext.hpp"
#include "Engine/OculusRift/OVRHeadset.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Renderable.hpp"
#include "Engine/Renderer/Sampler.hpp"
#include "Engine/Renderer/Scene.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Tools/Profiler/ProfileScope.hpp"
#include <algorithm>
#include <map>
#include <set>

DrawCall::DrawCall( Mesh* mesh, Material* material, Matrix44 modelMatrix /* = Matrix44::IDENTITY */, RenderQueue queue /* = RenderQueue::RENDER_QUEUE_OPAQUE */, int layer /* = 0 */, unsigned int passIndex /* = 0U */ )
	:	m_mesh( mesh ),
		m_material( material ),
		m_modelMatrix( modelMatrix ),
		m_queue( queue ),
		m_layer( layer ),
		m_passIndex( passIndex )
{
	
}

/* static */
Sampler* ForwardRenderingPath::s_clampSampler;
Sampler* ForwardRenderingPath::s_shadowSampler;

Texture* ForwardRenderingPath::s_cameraOriginalColorTarget;
Texture* ForwardRenderingPath::s_cameraOriginalDepthTarget;
Texture* ForwardRenderingPath::s_colorTargetMS;
Texture* ForwardRenderingPath::s_bloomTargetMS;
Texture* ForwardRenderingPath::s_depthTargetMS;
FrameBuffer* ForwardRenderingPath::s_framebufferMS;

Texture* ForwardRenderingPath::s_bloomTarget;
Texture* ForwardRenderingPath::s_bloomScratchTarget;
Material* ForwardRenderingPath::s_bloomBlurMaterial;

Material* ForwardRenderingPath::s_shadowMapMaterial;

/* static */
void ForwardRenderingPath::Startup()
{
	SamplerOptions samplerOptions;
	samplerOptions.m_wrapMode = SamplerWrapMode::SAMPLER_WRAP_CLAMP_EDGE;
	ForwardRenderingPath::s_clampSampler = new Sampler( samplerOptions );

	samplerOptions.m_wrapMode = SamplerWrapMode::SAMPLER_WRAP_CLAMP_BORDER;
	ForwardRenderingPath::s_shadowSampler = new Sampler( samplerOptions );
	ForwardRenderingPath::s_shadowSampler->UseForShadows();

	IntVector2 targetDimensions = ( g_gameConfigBlackboard.GetValue( "vrEnabled", false ) )?	OVRContext::GetHeadset()->GetResolution()	:	IntVector2( Window::GetWidth(), Window::GetHeight() );
	ForwardRenderingPath::s_bloomTarget = Renderer::GetInstance()->CreateRenderTarget( targetDimensions.x, targetDimensions.y );
	ForwardRenderingPath::s_bloomScratchTarget = Renderer::GetInstance()->CreateRenderTarget( targetDimensions.x, targetDimensions.y );
	ForwardRenderingPath::s_bloomBlurMaterial = Renderer::GetInstance()->CreateOrGetMaterial( "Bloom" );
	ForwardRenderingPath::s_bloomBlurMaterial->SetTextureAndSampler( 1, ForwardRenderingPath::s_bloomTarget, Renderer::GetInstance()->GetDefaultSampler() );

	ForwardRenderingPath::s_shadowMapMaterial = Renderer::GetInstance()->CreateOrGetMaterial( "ShadowMap" );
	ForwardRenderingPath::s_shadowMapMaterial->SetTextureAndSampler( 0, Renderer::GetInstance()->GetDefaultTexture(), Renderer::GetInstance()->GetDefaultSampler() );
}

/* static */
void ForwardRenderingPath::Shutdown()
{
	//delete ForwardRenderingPath::s_bloomBlurMaterial; // Owned by the Renderer, created on startup
	delete ForwardRenderingPath::s_framebufferMS;
	delete ForwardRenderingPath::s_bloomTarget;
	delete ForwardRenderingPath::s_bloomScratchTarget;
	delete ForwardRenderingPath::s_colorTargetMS;
	delete ForwardRenderingPath::s_depthTargetMS;
	delete ForwardRenderingPath::s_bloomTargetMS;
	delete ForwardRenderingPath::s_clampSampler;
	delete ForwardRenderingPath::s_shadowSampler;
}

/* static */
void ForwardRenderingPath::RenderScene( Scene* scene )
{
	PROFILE_SCOPE( "ForwardRenderingPath::RenderScene()" );
	ForwardRenderingPath::LightsPreRender( scene );

	scene->SortCameras();

	for ( Camera* camera : scene->m_cameras )
	{
		ForwardRenderingPath::RenderSceneToCamera( scene, camera );
	}
}

/* static */
void ForwardRenderingPath::LightsPreRender( Scene* scene )
{
	for ( Light* light : scene->m_lights )
	{
		light->UpdateUBOFromTransform();

		if ( light->m_lightUBO.m_castsShadows )
		{
			LightPreRender( light, scene );
		}
	}
}

/* static */
void ForwardRenderingPath::LightPreRender( Light* light, Scene* scene )
{
	Camera* shadowMapCamera = nullptr;
	if ( light->IsSpotLight() )
	{
		shadowMapCamera = light->MakePerspectiveCameraForShadowMapAndSetLightMatrices();
	}
	else
	{
		ASSERT_OR_DIE( ( scene->m_shadowViewingCamera != nullptr ), "Error: ForwardRenderingPath::LightPreRender(): No camera set to view shadows in scene, when shadows are enabled for a directional light. Aborting..." )

		Vector3 lightPosition = Vector3::ZERO;
		float orthoSize = 0.0f;
		float farZ = 0.0f;
		scene->m_shadowViewingCamera->GetOrthoCameraPropertiesForShadows( light->m_lightUBO.m_direction, lightPosition, orthoSize, farZ );

		float aspect = scene->m_shadowViewingCamera->GetFrameBuffer()->m_colorTargets[ 0 ]->GetDimensionsF().x / scene->m_shadowViewingCamera->GetFrameBuffer()->m_colorTargets[ 0 ]->GetDimensionsF().y;
		shadowMapCamera = light->MakeOrthoCameraForShadowMapAndSetLightMatrices(
			lightPosition,
			orthoSize,
			aspect,
			0.1f,
			farZ
		);
	}
	shadowMapCamera->SetDepthStencilTarget( scene->m_shadowMap );

	Renderer::GetInstance()->SetCamera( shadowMapCamera );
	Renderer::GetInstance()->WriteDepthImmediate( true );
	Renderer::GetInstance()->ClearDepth();

	for ( Renderable* renderable : scene->m_renderables )
	{
		if ( renderable->CastsShadows() && renderable->GetMaterial()->GetRenderQueue( 0 ) == RenderQueue::RENDER_QUEUE_OPAQUE && renderable->IsVisible() )
		{
			Renderable* renderableCopy = new Renderable( renderable->GetMesh(), ForwardRenderingPath::s_shadowMapMaterial, false, false, renderable->GetModelMatrix() );
			Renderer::GetInstance()->DrawRenderable( *renderableCopy );
			delete renderableCopy;
		}
	}

	shadowMapCamera->SetDepthStencilTarget( nullptr );

	Renderer::GetInstance()->UseDefaultCamera();
	delete shadowMapCamera;
}

/* static */
void ForwardRenderingPath::RenderSceneToCamera( Scene* scene, Camera* camera )
{
	ForwardRenderingPath::CameraPreRender( camera );

	std::set< int > layerNumbers; // Sorted by default
	std::map< int, std::vector< DrawCall > > drawCallsByLayer;

	for ( Renderable* renderable : scene->m_renderables )
	{
		if ( !renderable->IsVisible() )
		{
			continue;
		}
		
		for ( unsigned int passIndex = 0U; passIndex < renderable->GetMaterial()->GetShader()->GetPassCount(); passIndex++ )
		{
			unsigned int lightCount = 0U;

			RenderQueue renderableQueue = renderable->GetMaterial()->GetShader()->GetPass( passIndex )->GetRenderQueue();
			int renderableLayer = renderable->GetMaterial()->GetShader()->GetPass( passIndex )->GetLayer();

			DrawCall drawCall = DrawCall( renderable->GetMesh(), renderable->GetMaterial(), renderable->GetModelMatrix(), renderableQueue, renderableLayer, passIndex );

			if ( renderable->GetMaterial()->GetShader()->GetPass( passIndex )->UsesLights() )
			{
				lightCount = scene->GetMostContributingLightsForRenderable( renderable, drawCall.m_lights );

				Light* shadowCastingLight = scene->m_shadowCastingLight;
				if ( shadowCastingLight != nullptr )
				{
					Texture* shadowMap = scene->m_shadowMap;
					renderable->GetMaterial()->SetTextureAndSampler( TEXTURE_BIND_POINT_SHADOW_MAP, shadowMap, ForwardRenderingPath::s_shadowSampler );
				}
			}

			layerNumbers.insert( renderableLayer );
			drawCallsByLayer[ renderable->GetMaterial()->GetShader()->GetPass( passIndex )->GetLayer() ].push_back( drawCall );
		}
	}

	for ( int layer : layerNumbers )
	{
		// Sort based on queues within layer
		ForwardRenderingPath::SortDrawCallsByQueue( drawCallsByLayer[ layer ] );
		ForwardRenderingPath::SortAlphaDrawCallsByCameraDistance( drawCallsByLayer[ layer ], camera );

		for ( DrawCall drawCall : drawCallsByLayer[ layer ] )
		{
			Renderer::GetInstance()->BindLights( drawCall.m_lights );
			Renderer::GetInstance()->SetCamera( camera );
			Renderer::GetInstance()->Draw( drawCall );
		}
	}

	ForwardRenderingPath::CameraPostRender( camera );
}

/* static */
void ForwardRenderingPath::CameraPreRender( Camera* camera )
{
	if ( camera->IsMSAAEnabled() )
	{
		// Swap out camera's targets for MSAA targets
		ForwardRenderingPath::TryInitializeMSTargetsFromCamera( camera );

		ForwardRenderingPath::s_cameraOriginalColorTarget = camera->GetFrameBuffer()->m_colorTargets[ 0 ];
		ForwardRenderingPath::s_cameraOriginalDepthTarget = camera->GetFrameBuffer()->m_depthStencilTarget;

		camera->SetColorTarget( ForwardRenderingPath::s_colorTargetMS );
		camera->SetDepthStencilTarget( ForwardRenderingPath::s_depthTargetMS );
	}

	if ( camera->IsBloomEnabled() )
	{
		ForwardRenderingPath::AddBloomTarget( camera );
	}

	Renderer::GetInstance()->SetCamera( camera );

	if ( camera->ShouldClearColor() )
	{
		Renderer::GetInstance()->ClearColor();
	}
	if ( camera->ShouldClearDepth() )
	{
		Renderer::GetInstance()->WriteDepthImmediate( true );
		Renderer::GetInstance()->ClearDepth( 1.0f );
	}

	if ( camera->HasSkybox() )
	{
		if ( camera->IsBloomEnabled() )
		{
			ForwardRenderingPath::RemoveBloomTarget( camera );
		}

		camera->RenderSkybox();

		if ( camera->IsBloomEnabled() )
		{
			ForwardRenderingPath::AddBloomTarget( camera );
		}
	}
}

/* static */
void ForwardRenderingPath::TryInitializeMSTargetsFromCamera( Camera* camera )
{
	if ( ForwardRenderingPath::s_framebufferMS == nullptr )
	{
		ForwardRenderingPath::s_framebufferMS = new FrameBuffer;
	}
	if ( ( ForwardRenderingPath::s_colorTargetMS == nullptr ) || ( ForwardRenderingPath::s_colorTargetMS->GetNumMSAASamples() != camera->GetNumMSAASamples() ) )
	{
		IntVector2 dimensions = camera->GetFrameBuffer()->m_colorTargets[ 0 ]->GetDimensions();
		ForwardRenderingPath::s_colorTargetMS = Renderer::GetInstance()->CreateMSAATarget( dimensions.x, dimensions.y, TextureFormat::TEXTURE_FORMAT_RGBA8, camera->GetNumMSAASamples() );
	}
	if ( ( ForwardRenderingPath::s_depthTargetMS == nullptr ) || ( ForwardRenderingPath::s_depthTargetMS->GetNumMSAASamples() != camera->GetNumMSAASamples() ) )
	{
		IntVector2 dimensions = camera->GetFrameBuffer()->m_colorTargets[ 0 ]->GetDimensions();
		ForwardRenderingPath::s_depthTargetMS = Renderer::GetInstance()->CreateMSAATarget( dimensions.x, dimensions.y, TextureFormat::TEXTURE_FORMAT_D24S8, camera->GetNumMSAASamples() );
	}
	if ( ( ForwardRenderingPath::s_bloomTargetMS == nullptr ) || ( ForwardRenderingPath::s_bloomTargetMS->GetNumMSAASamples() != camera->GetNumMSAASamples() ) )
	{
		IntVector2 dimensions = camera->GetFrameBuffer()->m_colorTargets[ 0 ]->GetDimensions();
		ForwardRenderingPath::s_bloomTargetMS = Renderer::GetInstance()->CreateMSAATarget( dimensions.x, dimensions.y, TextureFormat::TEXTURE_FORMAT_RGBA8, camera->GetNumMSAASamples() );
	}
}

/* static */
void ForwardRenderingPath::CameraPostRender( Camera* camera )
{
	ForwardRenderingPath::CameraPostRenderMSAA( camera );
	ForwardRenderingPath::CameraPostRenderBloom( camera );
}

/* static */
void ForwardRenderingPath::CameraPostRenderMSAA( Camera* camera )
{
	if ( camera->IsMSAAEnabled() )
	{
		camera->SetColorTarget( ForwardRenderingPath::s_cameraOriginalColorTarget );
		camera->SetDepthStencilTarget( ForwardRenderingPath::s_cameraOriginalDepthTarget );

		ForwardRenderingPath::s_framebufferMS->SetColorTarget( ForwardRenderingPath::s_colorTargetMS );
		ForwardRenderingPath::s_framebufferMS->SetDepthStencilTarget( ForwardRenderingPath::s_depthTargetMS );

		if ( camera->IsBloomEnabled() )
		{
			camera->SetColorTarget( ForwardRenderingPath::s_bloomTarget, CAMERA_BLOOM_TARGET_INDEX );
			ForwardRenderingPath::s_framebufferMS->SetColorTarget( ForwardRenderingPath::s_bloomTargetMS, CAMERA_BLOOM_TARGET_INDEX );
		}
		else
		{
			if ( camera->GetFrameBuffer()->GetColorTargetCount() > 1U )
			{
				camera->RemoveColorTarget( CAMERA_BLOOM_TARGET_INDEX );
			}
			if ( ForwardRenderingPath::s_framebufferMS->GetColorTargetCount() > 1U )
			{
				ForwardRenderingPath::s_framebufferMS->RemoveColorTarget( CAMERA_BLOOM_TARGET_INDEX );
			}
		}

		ForwardRenderingPath::s_framebufferMS->Finalize();
		camera->Finalize();
		Renderer::GetInstance()->ClearColor();

		Renderer::GetInstance()->CopyFrameBuffer( camera->GetFrameBuffer(), ForwardRenderingPath::s_framebufferMS );

		ForwardRenderingPath::s_cameraOriginalColorTarget = nullptr;
		ForwardRenderingPath::s_cameraOriginalDepthTarget = nullptr;
	}
}

/* static */
void ForwardRenderingPath::CameraPostRenderBloom( Camera* camera )
{
	if ( camera->IsBloomEnabled() )
	{
		RemoveBloomTarget( camera );
		ForwardRenderingPath::ApplyBloomEffect( camera );
	}
}

/* static */
void ForwardRenderingPath::ApplyBloomEffect( Camera* camera )
{
	Texture* originalCameraTarget = camera->GetFrameBuffer()->m_colorTargets[ 0U ];
	camera->GetFrameBuffer()->m_colorTargets[ 0U ] = originalCameraTarget;

	Material* currentBloomMaterial = ForwardRenderingPath::s_bloomBlurMaterial;
	currentBloomMaterial->SetTextureAndSampler( 0, originalCameraTarget, Renderer::GetInstance()->GetDefaultSampler() );
	currentBloomMaterial->SetTextureAndSampler( 1, ForwardRenderingPath::s_bloomTarget, s_clampSampler );
	Renderer::GetInstance()->ApplyEffect( currentBloomMaterial, camera, ForwardRenderingPath::s_bloomScratchTarget );
	Renderer::GetInstance()->FinishEffects( originalCameraTarget );
}

/* static */
void ForwardRenderingPath::AddBloomTarget( Camera* camera )
{
	if ( camera->IsMSAAEnabled() )
	{
		camera->GetFrameBuffer()->SetColorTarget( ForwardRenderingPath::s_bloomTargetMS, CAMERA_BLOOM_TARGET_INDEX );
	}
	else
	{
		camera->GetFrameBuffer()->SetColorTarget( ForwardRenderingPath::s_bloomTarget, CAMERA_BLOOM_TARGET_INDEX );
	}
	camera->Finalize();
}

/* static */
void ForwardRenderingPath::RemoveBloomTarget( Camera* camera )
{
	camera->RemoveColorTarget( CAMERA_BLOOM_TARGET_INDEX );
	camera->Finalize();
}

/* static */
void ForwardRenderingPath::SortDrawCallsByQueue( std::vector< DrawCall >& drawCalls )
{
	std::sort( drawCalls.begin(), drawCalls.end(), []( const DrawCall& a, const DrawCall& b ) { return a.m_queue < b.m_queue; } );
}

/* static */
void ForwardRenderingPath::SortAlphaDrawCallsByCameraDistance( std::vector< DrawCall >& allDrawCalls, Camera* camera )
{
	size_t alphaStartIndex = SIZE_MAX;
	size_t alphaEndIndex = SIZE_MAX;
	for ( size_t drawCallIndex = 0; drawCallIndex < allDrawCalls.size(); drawCallIndex++ )
	{
		if ( allDrawCalls[ drawCallIndex ].m_queue == RenderQueue::RENDER_QUEUE_ALPHA )
		{
			if ( alphaStartIndex == SIZE_MAX )
			{
				alphaStartIndex = drawCallIndex;
			}
			alphaEndIndex = drawCallIndex;
		}
		else if ( alphaStartIndex != SIZE_MAX )
		{
			std::sort( ( allDrawCalls.begin() + alphaStartIndex ), ( allDrawCalls.begin() + alphaEndIndex ), [ camera ]( const DrawCall& a, const DrawCall& b ) {
				float aDistanceSquared = ( a.m_modelMatrix.GetTranslation() - camera->GetPosition() ).GetLengthSquared();
				float bDistanceSquared = ( b.m_modelMatrix.GetTranslation() - camera->GetPosition() ).GetLengthSquared();
				return bDistanceSquared < aDistanceSquared; // The farther away the object, the sooner it should render
			} );
			break;
		}
	}
}
