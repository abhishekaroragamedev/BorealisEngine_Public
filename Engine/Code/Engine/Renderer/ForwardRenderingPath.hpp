#pragma once

#include "Engine/Math/Matrix44.hpp"
#include "Engine/Renderer/Light.hpp"
#include "Engine/Renderer/ShaderPass.hpp"
#include <vector>

class Camera;
class FrameBuffer;
class Material;
class Mesh;
class Sampler;
class Scene;
class Texture;

constexpr unsigned int CAMERA_BLOOM_TARGET_INDEX = 1U;

struct DrawCall
{

public:
	DrawCall( Mesh* mesh, Material* material, Matrix44 modelMatrix = Matrix44::IDENTITY, RenderQueue queue = RenderQueue::RENDER_QUEUE_OPAQUE, int layer = 0, unsigned int passIndex = 0U );

public:
	Matrix44 m_modelMatrix = Matrix44::IDENTITY;
	Mesh* m_mesh = nullptr;
	Material* m_material = nullptr;	// The DrawCall will deal with a single Shader Pass instead of a full Material (which may have multiple Shader Passes)
	LightUBO m_lights[ MAX_LIGHTS ];
	RenderQueue m_queue = RenderQueue::RENDER_QUEUE_OPAQUE;
	int m_layer = 0;
	unsigned int m_passIndex = 0U;

};

/* static */
class ForwardRenderingPath
{

public:
	static void Startup();
	static void Shutdown();
	static void RenderScene( Scene* scene );
	static void RenderSceneToCamera( Scene* scene, Camera* camera );

private:
	static void LightsPreRender( Scene* scene );
	static void LightPreRender( Light* light, Scene* scene );
	static void CameraPreRender( Camera* camera );
	static void TryInitializeMSTargetsFromCamera( Camera* camera );
	static void CameraPostRender( Camera* camera );
	static void CameraPostRenderMSAA( Camera* camera );
	static void CameraPostRenderBloom( Camera* camera );
	static void ApplyBloomEffect( Camera* camera );
	static void SortDrawCallsByQueue( std::vector< DrawCall >& drawCalls );
	static void SortAlphaDrawCallsByCameraDistance( std::vector< DrawCall >& allDrawCalls, Camera* camera );
	static void AddBloomTarget( Camera* camera );
	static void RemoveBloomTarget( Camera* camera );

	static Sampler* s_clampSampler;
	static Sampler* s_shadowSampler;

	// Bloom effect
	static Texture* s_bloomTarget;
	static Texture* s_bloomScratchTarget;
	static Material* s_bloomBlurMaterial;

	// MSAA Support Textures added to the camera
	static Texture* s_colorTargetMS;
	static Texture* s_bloomTargetMS;
	static Texture* s_depthTargetMS;
	// Temp Textures to hold camera's original targets if MSAA is enabled
	static Texture* s_cameraOriginalColorTarget;
	static Texture* s_cameraOriginalDepthTarget;
	// Framebuffer to blit MS textures onto normal textures if MSAA is enabled
	static FrameBuffer* s_framebufferMS;

	static Material* s_shadowMapMaterial;

};