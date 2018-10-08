#pragma once

#include "Engine/Core/XmlUtilities.hpp"
#include "Engine/Renderer/RendererTypes.hpp"
#include "Engine/Renderer/ShaderProgram.hpp"
#include "ThirdParty/tinyXML2/tinyxml2.h"
#include <vector>

class MaterialProperty;
class Texture;
class Vector2;

enum RenderQueue : int
{
	RENDER_QUEUE_INVALID = -1,
	RENDER_QUEUE_OPAQUE,
	RENDER_QUEUE_ALPHA,
	NUM_RENDER_QUEUES
};

struct RenderState
{

public:
	RendererCullMode m_cullMode = RendererCullMode::CULL_MODE_BACK;
	RendererPolygonMode m_fillMode = RendererPolygonMode::POLYGON_MODE_FILL;
	RendererWindOrder m_frontFace = RendererWindOrder::WIND_COUNTER_CLOCKWISE;

	DepthTestCompare m_depthCompare = DepthTestCompare::COMPARE_LESS;
	bool m_depthWrite = true;

	RendererBlendOperation m_colorBlendOperation = RendererBlendOperation::OPERATION_ADD;
	RendererBlendFactor m_colorSourceFactor = RendererBlendFactor::FACTOR_SRC_ALPHA;
	RendererBlendFactor m_colorDestinationFactor = RendererBlendFactor::FACTOR_ONE_MINUS_SRC_ALPHA;

	RendererBlendOperation m_alphaBlendOperation = RendererBlendOperation::OPERATION_ADD;
	RendererBlendFactor m_alphaSourceFactor = RendererBlendFactor::FACTOR_ONE;
	RendererBlendFactor m_alphaDestinationFactor = RendererBlendFactor::FACTOR_ONE;

};

class ShaderPass
{
	friend class Material;
	friend class Shader;
	friend class Renderer;

public:
	ShaderPass( ShaderProgram* shaderProgram );
	ShaderPass( const tinyxml2::XMLElement& shaderPassElement );
	~ShaderPass();

	void LoadFromXML( const tinyxml2::XMLElement& shaderPassElement );

	void SetProgram( ShaderProgram* shaderProgram );

	void EnableColorBlending( RendererBlendOperation operation, RendererBlendFactor sourceFactor, RendererBlendFactor destinationFactor );
	void EnableAlphaBlending( RendererBlendOperation operation, RendererBlendFactor sourceFactor, RendererBlendFactor destinationFactor );
	void DisableColorBlending();
	void DisableAlphaBlending();
	void DisableBlending();

	void SetDepth( DepthTestCompare compare, bool write );
	void DisableDepth();

	void SetCullMode( RendererCullMode cullMode );
	void SetFillMode( RendererPolygonMode fillMode );
	void SetFrontFace( RendererWindOrder frontFace );
	void SetShaderProgram( ShaderProgram* shaderProgram );

	unsigned int GetProgramHandle() const;
	ShaderProgram* GetProgram() const;
	bool UsesLights() const;
	RenderQueue GetRenderQueue() const;
	int GetLayer() const;

private:
	void PopulateDefaultPropertiesFromXML( const tinyxml2::XMLElement& materialPropertiesElement );

public:
	static ShaderPass* AcquireResource( const std::string& filePath );

private:
	ShaderProgram* m_program = nullptr;
	RenderState m_state;
	bool m_usesLights = false;
	RenderQueue m_renderQueue = RenderQueue::RENDER_QUEUE_OPAQUE;
	int m_layer = 0;
	std::vector< MaterialProperty* > m_defaultProperties;
	std::vector< const Texture* > m_defaultTextures;
	Vector2 m_specular = Vector2( 0.0f, 1.0f );

}; 
