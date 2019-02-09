#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Vector4.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/ShaderPass.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Tools/DevConsole.hpp"

ShaderPass::ShaderPass( ShaderProgram* shaderProgram )
	:	m_program( shaderProgram )
{

}

ShaderPass::ShaderPass( const tinyxml2::XMLElement& shaderPassElement )
{
	LoadFromXML( shaderPassElement );
}

ShaderPass::~ShaderPass()
{

}

void ShaderPass::LoadFromXML( const tinyxml2::XMLElement& shaderPassElement )
{
	m_usesLights = ParseXmlAttribute( shaderPassElement, "usesLights", m_usesLights );
	m_renderQueue = static_cast< RenderQueue >( ParseXmlAttribute( shaderPassElement, "queue", m_renderQueue ) );
	m_layer = ParseXmlAttribute( shaderPassElement, "layer", m_layer );

	m_state.m_isCompute = ParseXmlAttribute( shaderPassElement, "compute", false );

	m_state.m_cullMode = GetCullModeFromName( ParseXmlAttribute( shaderPassElement, "cull", "" ) );
	m_state.m_fillMode = GetFillModeFromName( ParseXmlAttribute( shaderPassElement, "fill", "" ) );
	m_state.m_frontFace = GetWindOrderFromName( ParseXmlAttribute( shaderPassElement, "frontFace", "" ) );

	for ( const tinyxml2::XMLElement* childElement = shaderPassElement.FirstChildElement(); childElement != nullptr; childElement = childElement->NextSiblingElement() )
	{
		if ( std::string( childElement->Name() ) == "Program" )
		{
			if ( m_program == nullptr )
			{
				m_program = new ShaderProgram( *childElement, m_state.m_isCompute );
			}
			else
			{
				m_program->LoadFromXML( *childElement, m_state.m_isCompute );
			}
		}
		else if ( std::string( childElement->Name() ) == "Blend" )
		{
			for ( const tinyxml2::XMLElement* grandChildElement = childElement->FirstChildElement(); grandChildElement != nullptr; grandChildElement = grandChildElement->NextSiblingElement() )
			{
				if ( std::string( grandChildElement->Name() ) == "Color" )
				{
					m_state.m_blendOptions.m_colorBlendOperation = GetBlendOperationFromName( ParseXmlAttribute( *grandChildElement, "operation", "" ) );
					m_state.m_blendOptions.m_colorSourceFactor = GetBlendFactorFromName( ParseXmlAttribute( *grandChildElement, "source", "" ) );
					m_state.m_blendOptions.m_colorDestinationFactor = GetBlendFactorFromName( ParseXmlAttribute( *grandChildElement, "destination", "" ) );
				}
				else if ( std::string( grandChildElement->Name() ) == "Alpha" )
				{
					m_state.m_blendOptions.m_alphaBlendOperation = GetBlendOperationFromName( ParseXmlAttribute( *grandChildElement, "operation", "" ) );
					m_state.m_blendOptions.m_alphaSourceFactor = GetBlendFactorFromName( ParseXmlAttribute( *grandChildElement, "source", "" ) );
					m_state.m_blendOptions.m_alphaDestinationFactor = GetBlendFactorFromName( ParseXmlAttribute( *grandChildElement, "destination", "" ) );
				}
			}
		}
		else if ( std::string( childElement->Name() ) == "TargetBlend" )
		{
			PopulateColorTargetBlendOptionsFromXML( *childElement );
		}
		else if ( std::string( childElement->Name() ) == "Depth" )
		{
			m_state.m_depthCompare = GetDepthCompareFromName( ParseXmlAttribute( *childElement, "compare", "" ) );
			m_state.m_depthWrite = ParseXmlAttribute( *childElement, "write", true );
		}
		else if ( std::string( childElement->Name() ) == "Defaults" )
		{
			PopulateDefaultPropertiesFromXML( *childElement );
		}
	}
}

void ShaderPass::PopulateDefaultPropertiesFromXML( const tinyxml2::XMLElement& materialPropertiesElement )
{
	for ( const tinyxml2::XMLElement* childElement = materialPropertiesElement.FirstChildElement(); childElement != nullptr; childElement = childElement->NextSiblingElement() )
	{
		if ( std::string( childElement->Name() ) == "Texture" || std::string( childElement->Name() ) == "texture" )
		{
			std::string textureFileName = ParseXmlAttribute( *childElement, "fileName", "Data/Images/Texture_Default.png" );
			unsigned int mipLevel = ParseXmlAttribute( *childElement, "mip", 1U );
			m_defaultTextures.push_back( Renderer::GetInstance()->CreateOrGetTexture( textureFileName, mipLevel ) );
		}
		else if ( std::string( childElement->Name() ) == "Float" || std::string( childElement->Name() ) == "float" )
		{
			std::string floatPropertyName = ParseXmlAttribute( *childElement, "name", "" );
			float floatValue = ParseXmlAttribute( *childElement, "defaultValue", 0.0f );
			m_defaultProperties.push_back( new MaterialPropertyFloat( floatPropertyName, floatValue, std::map< unsigned int, std::vector< PropertyBlock* > >() ) );
		}
		else if ( std::string( childElement->Name() ) == "Vec2" || std::string( childElement->Name() ) == "vec2" )
		{
			std::string vec2PropertyName = ParseXmlAttribute( *childElement, "name", "" );
			Vector2 vec2Value = ParseXmlAttribute( *childElement, "defaultValue", Vector2::ZERO );
			m_defaultProperties.push_back( new MaterialPropertyVector2( vec2PropertyName, vec2Value, std::map< unsigned int, std::vector< PropertyBlock* > >() ) );
		}
		else if ( std::string( childElement->Name() ) == "Vec3" || std::string( childElement->Name() ) == "vec3" )
		{
			std::string vec3PropertyName = ParseXmlAttribute( *childElement, "name", "" );
			Vector3 vec3Value = ParseXmlAttribute( *childElement, "defaultValue", Vector3::ZERO );
			m_defaultProperties.push_back( new MaterialPropertyVector3( vec3PropertyName, vec3Value, std::map< unsigned int, std::vector< PropertyBlock* > >() ) );
		}
		else if ( std::string( childElement->Name() ) == "Vec4" || std::string( childElement->Name() ) == "vec4" )
		{
			std::string vec4PropertyName = ParseXmlAttribute( *childElement, "name", "" );
			Vector4 vec4Value = ParseXmlAttribute( *childElement, "defaultValue", Vector4::ZERO_DISPLACEMENT );
			m_defaultProperties.push_back( new MaterialPropertyVector4( vec4PropertyName, vec4Value, std::map< unsigned int, std::vector< PropertyBlock* > >() ) );
		}
		else if ( std::string( childElement->Name() ) == "Specular" || std::string( childElement->Name() ) == "specular" )
		{
			float amount = ParseXmlAttribute( *childElement, "amount", 0.0f );
			float power = ParseXmlAttribute( *childElement, "power", 1.0f );
			m_specular = Vector2( amount, power );
		}
	}
}

void ShaderPass::PopulateColorTargetBlendOptionsFromXML( const tinyxml2::XMLElement& targetsBlendElement )
{
	for ( const tinyxml2::XMLElement* childElement = targetsBlendElement.FirstChildElement(); childElement != nullptr; childElement = childElement->NextSiblingElement() )
	{
		if ( std::string( childElement->Name() ) == "ColorTarget" )
		{
			RenderBlendOptions optionsForTarget;

			for ( const tinyxml2::XMLElement* grandChildElement = childElement->FirstChildElement(); grandChildElement != nullptr; grandChildElement = grandChildElement->NextSiblingElement() )
			{
				if ( std::string( grandChildElement->Name() ) == "Color" )
				{
					optionsForTarget.m_colorBlendOperation = GetBlendOperationFromName( ParseXmlAttribute( *grandChildElement, "operation", "" ) );
					optionsForTarget.m_colorSourceFactor = GetBlendFactorFromName( ParseXmlAttribute( *grandChildElement, "source", "" ) );
					optionsForTarget.m_colorDestinationFactor = GetBlendFactorFromName( ParseXmlAttribute( *grandChildElement, "destination", "" ) );
				}
				else if ( std::string( grandChildElement->Name() ) == "Alpha" )
				{
					optionsForTarget.m_alphaBlendOperation = GetBlendOperationFromName( ParseXmlAttribute( *grandChildElement, "operation", "" ) );
					optionsForTarget.m_alphaSourceFactor = GetBlendFactorFromName( ParseXmlAttribute( *grandChildElement, "source", "" ) );
					optionsForTarget.m_alphaDestinationFactor = GetBlendFactorFromName( ParseXmlAttribute( *grandChildElement, "destination", "" ) );
				}
			}

			m_state.m_targetBlendOptions.push_back( optionsForTarget );
		}
	}
}

void ShaderPass::SetProgram( ShaderProgram* shaderProgram )
{
	m_program = shaderProgram;
}

void ShaderPass::EnableColorBlending( RendererBlendOperation operation, RendererBlendFactor sourceFactor, RendererBlendFactor destinationFactor )
{
	m_state.m_blendOptions.m_colorBlendOperation = operation;
	m_state.m_blendOptions.m_colorSourceFactor = sourceFactor;
	m_state.m_blendOptions.m_colorDestinationFactor = destinationFactor;
}

void ShaderPass::EnableAlphaBlending( RendererBlendOperation operation, RendererBlendFactor sourceFactor, RendererBlendFactor destinationFactor )
{
	m_state.m_blendOptions.m_alphaBlendOperation = operation;
	m_state.m_blendOptions.m_alphaSourceFactor = sourceFactor;
	m_state.m_blendOptions.m_alphaDestinationFactor = destinationFactor;
}

void ShaderPass::DisableColorBlending()
{
	m_state.m_blendOptions.m_colorBlendOperation = RendererBlendOperation::OPERATION_ADD;
	m_state.m_blendOptions.m_colorSourceFactor = RendererBlendFactor::FACTOR_ONE;
	m_state.m_blendOptions.m_colorDestinationFactor = RendererBlendFactor::FACTOR_ZERO;
}

void ShaderPass::DisableAlphaBlending()
{
	m_state.m_blendOptions.m_alphaBlendOperation = RendererBlendOperation::OPERATION_ADD;
	m_state.m_blendOptions.m_alphaSourceFactor = RendererBlendFactor::FACTOR_ONE;
	m_state.m_blendOptions.m_alphaDestinationFactor = RendererBlendFactor::FACTOR_ONE;
}

void ShaderPass::DisableBlending()
{
	DisableColorBlending();
	DisableAlphaBlending();
}

void ShaderPass::SetDepth( DepthTestCompare compare, bool write )
{
	m_state.m_depthCompare = compare;
	m_state.m_depthWrite = write;
}

void ShaderPass::DisableDepth()
{
	m_state.m_depthCompare = DepthTestCompare::COMPARE_ALWAYS;
	m_state.m_depthWrite = false;
}

void ShaderPass::SetCullMode( RendererCullMode cullMode )
{
	m_state.m_cullMode = cullMode;
}

void ShaderPass::SetFillMode( RendererPolygonMode fillMode )
{
	m_state.m_fillMode = fillMode;
}

void ShaderPass::SetFrontFace( RendererWindOrder frontFace )
{
	m_state.m_frontFace = frontFace;
}

void ShaderPass::SetShaderProgram( ShaderProgram* shaderProgram )
{
	m_program = shaderProgram;
}

unsigned int ShaderPass::GetProgramHandle() const
{
	return m_program->GetHandle();
}

ShaderProgram* ShaderPass::GetProgram() const
{
	return m_program;
}

bool ShaderPass::UsesLights() const
{
	return m_usesLights;
}

RenderQueue ShaderPass::GetRenderQueue() const
{
	return m_renderQueue;
}

int ShaderPass::GetLayer() const
{
	return m_layer;
}

/* static */
ShaderPass* ShaderPass::AcquireResource( const std::string& filePath )
{
	tinyxml2::XMLDocument shaderPassDoc;

	tinyxml2::XMLError loadStatus = shaderPassDoc.LoadFile( filePath.c_str() );
	if ( loadStatus == tinyxml2::XMLError::XML_SUCCESS )
	{
		return new ShaderPass( *shaderPassDoc.FirstChildElement() );
	}
	else
	{
		ConsolePrintf( Rgba::RED, "ERROR: ShaderPass::AcquireResource() - Could not load XML file %s.", filePath.c_str() );
		return nullptr;
	}
}
