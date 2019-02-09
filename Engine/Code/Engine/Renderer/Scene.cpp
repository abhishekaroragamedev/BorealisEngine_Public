#include "Engine/Core/Window.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/Renderable.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Scene.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Tools/DevConsole.hpp"
#include <algorithm>

Scene::Scene()
{
	m_shadowMap = Renderer::GetInstance()->CreateRenderTarget( Window::GetWidth(), Window::GetHeight(), TextureFormat::TEXTURE_FORMAT_D24S8 );
}

Scene::~Scene()
{
	delete m_shadowMap;
	m_shadowMap = nullptr;
}

void Scene::AddCamera( Camera* camera )
{
	if ( std::find( m_cameras.begin(), m_cameras.end(), camera ) == m_cameras.end() )
	{
		m_cameras.push_back( camera );
	}
	else
	{
		ConsolePrintf( Rgba::YELLOW, "WARNING: Scene::AddCamera() - Camera to be added already exists. Skipping..." );
	}
}

void Scene::AddLight( Light* light )
{
	if ( std::find( m_lights.begin(), m_lights.end(), light ) == m_lights.end() )
	{
		m_lights.push_back( light );
	}
	else
	{
		ConsolePrintf( Rgba::YELLOW, "WARNING: Scene::AddLight() - Light to be added already exists. Skipping..." );
	}
}

void Scene::AddLights( Light* lights, unsigned int count )
{
	for ( unsigned int index = 0U; index < count; index++ )
	{
		if ( std::find( m_lights.begin(), m_lights.end(), ( lights + index ) ) == m_lights.end() )
		{
			m_lights.push_back( ( lights + index ) );
		}
		else
		{
			ConsolePrintf( Rgba::YELLOW, "WARNING: Scene::AddLights() - Light to be added already exists. Skipping..." );
		}
	}
}

void Scene::SetShadowCastingLight( Light* light )
{
	if ( m_shadowCastingLight != nullptr )
	{
		m_shadowCastingLight->m_lightUBO.m_castsShadows = false;
	}

	if ( std::find( m_lights.begin(), m_lights.end(), light ) == m_lights.end() )
	{
		AddLight( light );
	}
	m_shadowCastingLight = light;
	light->m_lightUBO.m_castsShadows = true;
}

void Scene::SetShadowViewingCamera( Camera* camera )
{
	if ( m_shadowViewingCamera != nullptr )
	{
		m_shadowViewingCamera->DisableShadowViewing();
	}

	if ( std::find( m_cameras.begin(), m_cameras.end(), camera ) == m_cameras.end() )
	{
		AddCamera( camera );
	}
	m_shadowViewingCamera = camera;
}

void Scene::AddRenderable( Renderable* renderable )
{
	if ( std::find( m_renderables.begin(), m_renderables.end(), renderable ) == m_renderables.end() )
	{
		m_renderables.push_back( renderable );
	}
	else
	{
		ConsolePrintf( Rgba::YELLOW, "WARNING: Scene::AddRenderable() - Renderable to be added already exists. Skipping..." );
	}
}

void Scene::RemoveCamera( Camera* camera )
{
	if ( camera == m_shadowViewingCamera )
	{
		m_shadowViewingCamera = nullptr;
	}

	std::vector< Camera* >::iterator cameraIterator = std::find( m_cameras.begin(), m_cameras.end(), camera );
	if ( cameraIterator != m_cameras.end() )
	{
		m_cameras.erase( cameraIterator );
	}
	else
	{
		ConsolePrintf( Rgba::YELLOW, "WARNING: Scene::RemoveCamera() - Camera to be removed doesn't exist. Skipping..." );
	}
}

void Scene::RemoveLight( Light* light )
{
	bool lightFound = false;
	for ( size_t lightIndex = 0; lightIndex < m_lights.size(); lightIndex++ )
	{
		if ( m_lights[ lightIndex ] == light )
		{
			m_lights.erase( m_lights.begin() + lightIndex );

			if ( m_shadowCastingLight == light )
			{
				if ( !m_lights.empty() )
				{
					m_shadowCastingLight = m_lights[ 0 ];
					m_shadowCastingLight->m_lightUBO.m_castsShadows = true;
				}
				else
				{
					m_shadowCastingLight = nullptr;
				}
			}
			light->m_lightUBO.m_castsShadows = false;
			lightFound = true;
			break;
		}
	}
	if ( !lightFound )
	{
		ConsolePrintf( Rgba::YELLOW, "WARNING: Scene::RemoveLight() - Light to be removed doesn't exist. Skipping..." );
	}
}

void Scene::RemoveRenderable( Renderable* renderable )
{
	std::vector< Renderable* >::iterator renderableIterator = std::find( m_renderables.begin(), m_renderables.end(), renderable );
	if ( renderableIterator != m_renderables.end() )
	{
		m_renderables.erase( renderableIterator );
	}
	else
	{
		ConsolePrintf( Rgba::YELLOW, "WARNING: Scene::RemoveRenderable() - Renderable to be removed doesn't exist. Skipping..." );
	}
}

void Scene::SortCameras()
{
	std::sort( m_cameras.begin(), m_cameras.end(), [] ( const Camera* first, const Camera* second ) { return ( first->GetSortOrder() - second->GetSortOrder() ); } );
}

unsigned int Scene::GetCameraCount() const
{
	return static_cast< unsigned int >( m_cameras.size() );
}

unsigned int Scene::GetLightCount() const
{
	return static_cast< unsigned int >( m_lights.size() );
}

unsigned int Scene::GetRenderableCount() const
{
	return static_cast< unsigned int >( m_renderables.size() );
}

// Just needed for Scene::GetMostContributingLightsForRenderable
class RenderableLightIntensity
{

public:
	RenderableLightIntensity( Light* light, Renderable* renderable )
		:	m_light( light ),
			m_renderable( renderable )
	{
		Vector3 boundsCenter = m_renderable->GetMesh()->GetBounds().GetCenter();
		Matrix44 modelMatrix = m_renderable->GetModelMatrix();
		Vector3 transformedBoundsCenter = modelMatrix.TransformPosition( boundsCenter );

		m_intensity = m_light->GetIntensityAtPosition( transformedBoundsCenter );
		m_intensity = ClampFloat( m_intensity, 0.0f, m_light->m_lightUBO.m_colorAndIntensity.w );
	}

	bool RenderableLightIntensity::operator>( const RenderableLightIntensity& compareTo )
	{
		return ( m_intensity > compareTo.m_intensity );
	}

public:
	Light* m_light = nullptr;
	Renderable* m_renderable = nullptr;
	float m_intensity = 0.0f;

};

unsigned int Scene::GetMostContributingLightsForRenderable( Renderable* renderable, LightUBO out_lights[ MAX_LIGHTS ] ) const
{
	std::vector< RenderableLightIntensity > renderableLights;

	unsigned int lightStartIndex = 0U;
	if ( m_shadowCastingLight != nullptr )
	{
		out_lights[ 0 ] = m_shadowCastingLight->m_lightUBO;
		lightStartIndex = 1U;
	}

	for ( Light* light : m_lights )
	{
		if ( light == m_shadowCastingLight )
		{
			continue;
		}

		RenderableLightIntensity newRenderableLight = RenderableLightIntensity( light, renderable );
		if ( newRenderableLight.m_intensity > 0.0f )
		{
			if ( static_cast< unsigned int >( renderableLights.size() ) < ( MAX_LIGHTS - lightStartIndex ) )
			{
				renderableLights.push_back( newRenderableLight );
			}
			else
			{
				float minIntensity = FLT_MAX;
				size_t minIndex = 0;
				for ( size_t lightIndex = 0; lightIndex < renderableLights.size(); lightIndex++ )
				{
					if ( renderableLights[ lightIndex ].m_intensity < minIntensity )
					{
						minIntensity = renderableLights[ lightIndex ].m_intensity;
						minIndex = lightIndex; 
					}
				}

				if ( newRenderableLight.m_intensity > minIntensity )
				{
					renderableLights[ minIndex ] = newRenderableLight;
				}
			}
		}
	}

	for ( unsigned int lightIndex = 0U; lightIndex < static_cast< unsigned int >( renderableLights.size() ); lightIndex++ )
	{
		out_lights[ lightIndex + lightStartIndex ] = renderableLights[ lightIndex ].m_light->m_lightUBO;
	}

	return Min( static_cast< int >( static_cast< unsigned int >( renderableLights.size() ) + lightStartIndex ), MAX_LIGHTS );
}
