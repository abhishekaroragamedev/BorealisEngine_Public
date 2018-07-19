#pragma once

#include "Engine/Math/AABB3.hpp"
#include "Engine/Renderer/Light.hpp"
#include <vector>

class Camera;
class Renderable;
class Texture;

class Scene
{

	friend class ForwardRenderingPath;

public:
	Scene();
	~Scene();

	void AddCamera( Camera* camera );
	void AddLight( Light* light );
	void AddLights( Light* lights, unsigned int count );
	void SetShadowCastingLight( Light* light );
	void SetShadowViewingCamera( Camera* camera );
	void AddRenderable( Renderable* renderable );

	void RemoveCamera( Camera* camera );
	void RemoveLight( Light* light );
	void RemoveRenderable( Renderable* Renderable );

	void SortCameras();

	unsigned int GetCameraCount() const;
	unsigned int GetLightCount() const;
	unsigned int GetRenderableCount() const;
	unsigned int GetMostContributingLightsForRenderable( Renderable* renderable, LightUBO out_lights[ MAX_LIGHTS ] ) const;	// If there is a shadow casting light in the scene, this light will always be the first light returned

private:
	std::vector< Camera* > m_cameras;
	std::vector< Light* > m_lights;
	Light* m_shadowCastingLight = nullptr;
	Texture* m_shadowMap = nullptr;
	Camera* m_shadowViewingCamera = nullptr;
	std::vector< Renderable* > m_renderables;

};
