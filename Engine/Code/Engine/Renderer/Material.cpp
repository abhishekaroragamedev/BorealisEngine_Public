#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Vector4.hpp"
#include "Engine/Renderer/GLFunctionBinding.hpp"
#include "Engine/Renderer/Light.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/PropertyBlock.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Sampler.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Renderer/ShaderPass.hpp"
#include "Engine/Renderer/Texture.hpp"

MaterialProperty::MaterialProperty( const std::string& name, MaterialPropertyType type, std::map<unsigned int, std::vector<PropertyBlock *>> propertyBlocksByProgram )
	:	m_name( name ),
		m_type( type ),
		m_propertyBlocksByProgram( propertyBlocksByProgram )
{
	for ( std::map< unsigned int, std::vector< PropertyBlock* > >::iterator blockIterator = m_propertyBlocksByProgram.begin(); blockIterator != m_propertyBlocksByProgram.end(); blockIterator++ )
	{
		for ( size_t blockIndex = 0; blockIndex < blockIterator->second.size(); blockIndex++ )
		{
			if ( !blockIterator->second[ blockIndex ]->GetInfo()->ContainsProperty( name ) )
			{
				blockIterator->second.erase( blockIterator->second.begin() + blockIndex ); // Only hold on to those property blocks that contain this property
				blockIndex--;
			}
		}
	}
}

MaterialProperty::~MaterialProperty()
{

}

std::string MaterialProperty::GetName() const
{
	return m_name;
}

MaterialPropertyType MaterialProperty::GetType() const
{
	return m_type;
}

#pragma region MaterialProperty Types

MaterialPropertyFloat::MaterialPropertyFloat( const std::string& name, float value, std::map<unsigned int, std::vector<PropertyBlock *>> propertyBlocksByProgram )
	:	MaterialProperty( name, MaterialPropertyType::MATERIAL_PROPERTY_FLOAT, propertyBlocksByProgram ),
		m_value( value )
{
	for ( std::map< unsigned int, std::vector< PropertyBlock* > >::iterator blockIterator = m_propertyBlocksByProgram.begin(); blockIterator != m_propertyBlocksByProgram.end(); blockIterator++ )
	{
		for ( PropertyBlock* block : blockIterator->second )
		{
			block->SetCPUData( m_name, sizeof( float ), &m_value );
		}
	}
}

MaterialPropertyFloat::~MaterialPropertyFloat()
{

}

float MaterialPropertyFloat::GetValue() const
{
	return m_value;
}

void MaterialPropertyFloat::SetValue( float newValue )
{
	m_value = newValue;
	for ( std::map< unsigned int, std::vector< PropertyBlock* > >::iterator blockIterator = m_propertyBlocksByProgram.begin(); blockIterator != m_propertyBlocksByProgram.end(); blockIterator++ )
	{
		for ( PropertyBlock* block : blockIterator->second )
		{
			block->SetCPUData( m_name, sizeof( float ), &m_value );
		}
	}
}

MaterialPropertyVector2::MaterialPropertyVector2( const std::string& name, const Vector2& value, std::map<unsigned int, std::vector<PropertyBlock *>> propertyBlocksByProgram )
	:	MaterialProperty( name, MaterialPropertyType::MATERIAL_PROPERTY_VECTOR_2, propertyBlocksByProgram ),
		m_value( value )
{
	for ( std::map< unsigned int, std::vector< PropertyBlock* > >::iterator blockIterator = m_propertyBlocksByProgram.begin(); blockIterator != m_propertyBlocksByProgram.end(); blockIterator++ )
	{
		for ( PropertyBlock* block : blockIterator->second )
		{
			block->SetCPUData( m_name, sizeof( Vector2 ), &m_value );
		}
	}
}

MaterialPropertyVector2::MaterialPropertyVector2( const std::string& name, float xValue, float yValue, std::map<unsigned int, std::vector<PropertyBlock *>> propertyBlocksByProgram )
	:	MaterialProperty( name, MaterialPropertyType::MATERIAL_PROPERTY_VECTOR_2, propertyBlocksByProgram ),
		m_value( Vector2( xValue, yValue ) )
{
	for ( std::map< unsigned int, std::vector< PropertyBlock* > >::iterator blockIterator = m_propertyBlocksByProgram.begin(); blockIterator != m_propertyBlocksByProgram.end(); blockIterator++ )
	{
		for ( PropertyBlock* block : blockIterator->second )
		{
			block->SetCPUData( m_name, sizeof( Vector2 ), &m_value );
		}
	}
}

MaterialPropertyVector2::~MaterialPropertyVector2()
{

}

Vector2 MaterialPropertyVector2::GetValue() const
{
	return m_value;
}

void MaterialPropertyVector2::SetValue( const Vector2& newValue )
{
	m_value = newValue;
	for ( std::map< unsigned int, std::vector< PropertyBlock* > >::iterator blockIterator = m_propertyBlocksByProgram.begin(); blockIterator != m_propertyBlocksByProgram.end(); blockIterator++ )
	{
		for ( PropertyBlock* block : blockIterator->second )
		{
			block->SetCPUData( m_name, sizeof( Vector2 ), &m_value );
		}
	}
}

MaterialPropertyVector3::MaterialPropertyVector3( const std::string& name, const Vector3& value, std::map<unsigned int, std::vector<PropertyBlock *>> propertyBlocksByProgram )
	:	MaterialProperty( name, MaterialPropertyType::MATERIAL_PROPERTY_VECTOR_3, propertyBlocksByProgram ),
		m_value( value )
{
	for ( std::map< unsigned int, std::vector< PropertyBlock* > >::iterator blockIterator = m_propertyBlocksByProgram.begin(); blockIterator != m_propertyBlocksByProgram.end(); blockIterator++ )
	{
		for ( PropertyBlock* block : blockIterator->second )
		{
			block->SetCPUData( m_name, sizeof( Vector3 ), &m_value );
		}
	}
}

MaterialPropertyVector3::MaterialPropertyVector3( const std::string& name, const Rgba& color, std::map<unsigned int, std::vector<PropertyBlock *>> propertyBlocksByProgram )
	:	MaterialProperty( name, MaterialPropertyType::MATERIAL_PROPERTY_VECTOR_3, propertyBlocksByProgram ),
		m_value( color.GetRGBAsFloats() )
{
	for ( std::map< unsigned int, std::vector< PropertyBlock* > >::iterator blockIterator = m_propertyBlocksByProgram.begin(); blockIterator != m_propertyBlocksByProgram.end(); blockIterator++ )
	{
		for ( PropertyBlock* block : blockIterator->second )
		{
			block->SetCPUData( m_name, sizeof( Vector3 ), &m_value );
		}
	}
}

MaterialPropertyVector3::MaterialPropertyVector3( const std::string& name, float xValue, float yValue, float zValue, std::map<unsigned int, std::vector<PropertyBlock *>> propertyBlocksByProgram )
	:	MaterialProperty( name, MaterialPropertyType::MATERIAL_PROPERTY_VECTOR_3, propertyBlocksByProgram ),
		m_value( Vector3( xValue, yValue, zValue ) )
{
	for ( std::map< unsigned int, std::vector< PropertyBlock* > >::iterator blockIterator = m_propertyBlocksByProgram.begin(); blockIterator != m_propertyBlocksByProgram.end(); blockIterator++ )
	{
		for ( PropertyBlock* block : blockIterator->second )
		{
			block->SetCPUData( m_name, sizeof( Vector3 ), &m_value );
		}
	}
}

MaterialPropertyVector3::~MaterialPropertyVector3()
{

}

Vector3 MaterialPropertyVector3::GetValue() const
{
	return m_value;
}

void MaterialPropertyVector3::SetValue( const Vector3& newValue )
{
	m_value = newValue;
	for ( std::map< unsigned int, std::vector< PropertyBlock* > >::iterator blockIterator = m_propertyBlocksByProgram.begin(); blockIterator != m_propertyBlocksByProgram.end(); blockIterator++ )
	{
		for ( PropertyBlock* block : blockIterator->second )
		{
			block->SetCPUData( m_name, sizeof( Vector3 ), &m_value );
		}
	}
}

MaterialPropertyVector4::MaterialPropertyVector4( const std::string& name, const Vector4& value, std::map<unsigned int, std::vector<PropertyBlock *>> propertyBlocksByProgram )
	:	MaterialProperty( name, MaterialPropertyType::MATERIAL_PROPERTY_VECTOR_4, propertyBlocksByProgram ),
		m_value( value )
{
	for ( std::map< unsigned int, std::vector< PropertyBlock* > >::iterator blockIterator = m_propertyBlocksByProgram.begin(); blockIterator != m_propertyBlocksByProgram.end(); blockIterator++ )
	{
		for ( PropertyBlock* block : blockIterator->second )
		{
			block->SetCPUData( m_name, sizeof( Vector4 ), &m_value );
		}
	}
}

MaterialPropertyVector4::MaterialPropertyVector4( const std::string& name, const Rgba& color, std::map<unsigned int, std::vector<PropertyBlock *>> propertyBlocksByProgram )
	:	MaterialProperty( name, MaterialPropertyType::MATERIAL_PROPERTY_VECTOR_4, propertyBlocksByProgram ),
		m_value( color.GetAsFloats() )
{
	for ( std::map< unsigned int, std::vector< PropertyBlock* > >::iterator blockIterator = m_propertyBlocksByProgram.begin(); blockIterator != m_propertyBlocksByProgram.end(); blockIterator++ )
	{
		for ( PropertyBlock* block : blockIterator->second )
		{
			block->SetCPUData( m_name, sizeof( Vector4 ), &m_value );
		}
	}
}

MaterialPropertyVector4::MaterialPropertyVector4( const std::string& name, const Vector3& xyzValue, float wValue, std::map<unsigned int, std::vector<PropertyBlock *>> propertyBlocksByProgram )
	:	MaterialProperty( name, MaterialPropertyType::MATERIAL_PROPERTY_VECTOR_4, propertyBlocksByProgram ),
		m_value( Vector4( xyzValue, wValue ) )
{
	for ( std::map< unsigned int, std::vector< PropertyBlock* > >::iterator blockIterator = m_propertyBlocksByProgram.begin(); blockIterator != m_propertyBlocksByProgram.end(); blockIterator++ )
	{
		for ( PropertyBlock* block : blockIterator->second )
		{
			block->SetCPUData( m_name, sizeof( Vector4 ), &m_value );
		}
	}
}

MaterialPropertyVector4::MaterialPropertyVector4( const std::string& name, float xValue, float yValue, float zValue, float wValue, std::map<unsigned int, std::vector<PropertyBlock *>> propertyBlocksByProgram )
	:	MaterialProperty( name, MaterialPropertyType::MATERIAL_PROPERTY_VECTOR_4, propertyBlocksByProgram ),
		m_value( Vector4( xValue, yValue, zValue, wValue ) )
{
	for ( std::map< unsigned int, std::vector< PropertyBlock* > >::iterator blockIterator = m_propertyBlocksByProgram.begin(); blockIterator != m_propertyBlocksByProgram.end(); blockIterator++ )
	{
		for ( PropertyBlock* block : blockIterator->second )
		{
			block->SetCPUData( m_name, sizeof( Vector4 ), &m_value );
		}
	}
}

MaterialPropertyVector4::~MaterialPropertyVector4()
{

}

Vector4 MaterialPropertyVector4::GetValue() const
{
	return m_value;
}

void MaterialPropertyVector4::SetValue( const Vector4& newValue )
{
	m_value = newValue;
	for ( std::map< unsigned int, std::vector< PropertyBlock* > >::iterator blockIterator = m_propertyBlocksByProgram.begin(); blockIterator != m_propertyBlocksByProgram.end(); blockIterator++ )
	{
		for ( PropertyBlock* block : blockIterator->second )
		{
			block->SetCPUData( m_name, sizeof( Vector4 ), &m_value );
		}
	}
}

MaterialPropertyRgba::MaterialPropertyRgba( const std::string& name, const Rgba& color, std::map<unsigned int, std::vector<PropertyBlock *>> propertyBlocksByProgram )
	:	MaterialPropertyVector4( name, color, propertyBlocksByProgram )
{

}

MaterialPropertyRgba::MaterialPropertyRgba( const std::string& name, unsigned char rValue, unsigned char gValue, unsigned char bValue, unsigned char aValue, std::map<unsigned int, std::vector<PropertyBlock *>> propertyBlocksByProgram )
	:	MaterialPropertyVector4( name, Rgba( rValue, gValue, bValue, aValue ), propertyBlocksByProgram )
{

}

MaterialPropertyRgba::~MaterialPropertyRgba()
{

}

Rgba MaterialPropertyRgba::GetValueAsRgba() const
{
	return Rgba( m_value );
}

void MaterialPropertyRgba::SetValue( const Vector4& newValue )
{
	m_value = newValue;
	for ( std::map< unsigned int, std::vector< PropertyBlock* > >::iterator blockIterator = m_propertyBlocksByProgram.begin(); blockIterator != m_propertyBlocksByProgram.end(); blockIterator++ )
	{
		for ( PropertyBlock* block : blockIterator->second )
		{
			block->SetCPUData( m_name, sizeof( Vector4 ), &m_value );
		}
	}
}

void MaterialPropertyRgba::SetValue( const Rgba& newValue )
{
	SetValue( newValue.GetAsFloats() );
}

MaterialPropertyMatrix44::MaterialPropertyMatrix44( const std::string& name, const Matrix44& value, std::map<unsigned int, std::vector<PropertyBlock *>> propertyBlocksByProgram )
	:	MaterialProperty( name, MaterialPropertyType::MATERIAL_PROPERTY_MATRIX44, propertyBlocksByProgram ),
		m_value( value )
{
	for ( std::map< unsigned int, std::vector< PropertyBlock* > >::iterator blockIterator = m_propertyBlocksByProgram.begin(); blockIterator != m_propertyBlocksByProgram.end(); blockIterator++ )
	{
		for ( PropertyBlock* block : blockIterator->second )
		{
			block->SetCPUData( m_name, sizeof( Matrix44 ), &m_value );
		}
	}
}

MaterialPropertyMatrix44::~MaterialPropertyMatrix44()
{

}

Matrix44 MaterialPropertyMatrix44::GetValue() const
{
	return m_value;
}

void MaterialPropertyMatrix44::SetValue( const Matrix44& newValue )
{
	m_value = newValue;
	for ( std::map< unsigned int, std::vector< PropertyBlock* > >::iterator blockIterator = m_propertyBlocksByProgram.begin(); blockIterator != m_propertyBlocksByProgram.end(); blockIterator++ )
	{
		for ( PropertyBlock* block : blockIterator->second )
		{
			block->SetCPUData( m_name, sizeof( Matrix44 ), &m_value );
		}
	}
}

#pragma endregion

Material::Material( Shader* shader, bool isInstance /* = false */ )
	:	m_shader( shader ),
		m_isInstance( isInstance )
{
	m_lightProperties = new ObjectLightPropertiesUBO;
	CreatePropertyBlocksPerProgram();
	InitializeMaterialPropertiesFromShader();
	ErasePropertyBlocksForUnownedBlocks();
}

Material::Material( const Material* sourceMaterial )
	:	m_shader( sourceMaterial->GetShader() ),
		m_lightProperties( new ObjectLightPropertiesUBO( *sourceMaterial->m_lightProperties ) ),
		m_samplers( sourceMaterial->m_samplers ),
		m_textures( sourceMaterial->m_textures ),
		m_isInstance( true )
{
	CreatePropertyBlocksPerProgram();
	DuplicateMaterialProperties( *sourceMaterial );
	ErasePropertyBlocksForUnownedBlocks();
}

Material::Material( const Material& copyMaterial )
	:	m_shader( copyMaterial.GetShader() ),
		m_lightProperties( new ObjectLightPropertiesUBO( *copyMaterial.m_lightProperties ) ),
		m_samplers( copyMaterial.m_samplers ),
		m_textures( copyMaterial.m_textures ),
		m_isInstance( true )
{
	CreatePropertyBlocksPerProgram();
	DuplicateMaterialProperties( copyMaterial );
	ErasePropertyBlocksForUnownedBlocks();
}

Material::Material( const tinyxml2::XMLElement& materialElement )
{
	LoadFromXML( materialElement );
}

Material::~Material()
{
	DeleteAllMaterialData();
}

void Material::LoadFromXML( const tinyxml2::XMLElement& materialElement, bool isReload /* = false */ )
{
	if ( isReload )
	{
		DeleteAllMaterialData();
	}

	std::string shaderName = ParseXmlAttribute( materialElement, "shader", DEFAULT_SHADER_NAME );
	m_shader = Renderer::GetInstance()->CreateOrGetShader( shaderName );

	m_isInstance = false;
	m_lightProperties = new ObjectLightPropertiesUBO();

	unsigned int textureCount = 0U;

	CreatePropertyBlocksPerProgram();
	for ( const tinyxml2::XMLElement* childElement = materialElement.FirstChildElement(); childElement != nullptr; childElement = childElement->NextSiblingElement() )
	{
		if ( std::string( childElement->Name() ) == "Texture" )
		{
			std::string textureFileName = ParseXmlAttribute( *childElement, "fileName", "Texture_Default.png" );
			unsigned int mipLevel = ParseXmlAttribute( *childElement, "mip", 1U );
			SetTextureAndSampler(	textureCount,
				Renderer::GetInstance()->CreateOrGetTexture( textureFileName, mipLevel ),
				Renderer::GetInstance()->GetDefaultSampler( ( mipLevel > 1U ) )	// Use a MipMap Linear sampler if mip level > 1
			);
			textureCount++;
		}
		else if ( std::string( childElement->Name() ) == "Float" )
		{
			std::string floatPropertyName = ParseXmlAttribute( *childElement, "name", "" );
			float floatValue = ParseXmlAttribute( *childElement, "defaultValue", 0.0f );
			SetProperty( floatPropertyName, floatValue );
		}
		else if ( std::string( childElement->Name() ) == "Vec2" )
		{
			std::string vec2PropertyName = ParseXmlAttribute( *childElement, "name", "" );
			Vector2 vec2Value = ParseXmlAttribute( *childElement, "defaultValue", Vector2::ZERO );
			SetProperty( vec2PropertyName, vec2Value );
		}
		else if ( std::string( childElement->Name() ) == "Vec3" )
		{
			std::string vec3PropertyName = ParseXmlAttribute( *childElement, "name", "" );
			Vector3 vec3Value = ParseXmlAttribute( *childElement, "defaultValue", Vector3::ZERO );
			SetProperty( vec3PropertyName, vec3Value );
		}
		else if ( std::string( childElement->Name() ) == "Vec4" )
		{
			std::string vec4PropertyName = ParseXmlAttribute( *childElement, "name", "" );
			Vector4 vec4Value = ParseXmlAttribute( *childElement, "defaultValue", Vector4::ZERO_DISPLACEMENT );
			SetProperty( vec4PropertyName, vec4Value );
		}
		else if ( std::string( childElement->Name() ) == "Specular" )
		{
			float amount = ParseXmlAttribute( *childElement, "amount", 0.0f );
			float power = ParseXmlAttribute( *childElement, "power", 1.0f );
			SetSpecularProperties( amount, power );
		}
	}
	ErasePropertyBlocksForUnownedBlocks();
}

void Material::InitializeMaterialPropertiesFromShader()
{
	for ( unsigned int passIndex = 0U; passIndex < m_shader->GetPassCount(); passIndex++ )
	{
		for ( const Texture* defaultTexture : m_shader->GetPass( passIndex )->m_defaultTextures )
		{
			AddTextureAndSampler( defaultTexture, Renderer::GetInstance()->GetDefaultSampler() );
		}
		
		m_properties.insert( m_properties.end(), m_shader->GetPass( passIndex )->m_defaultProperties.begin(), m_shader->GetPass( passIndex )->m_defaultProperties.end() );
		
		SetSpecularProperties( m_shader->GetPass( passIndex )->m_specular.x, m_shader->GetPass( passIndex )->m_specular.y );
	}
}

void Material::DuplicateMaterialProperties( const Material& materialToCopy )
{
	for ( MaterialProperty* property : materialToCopy.m_properties )
	{
		switch( property->m_type )
		{
			case MaterialPropertyType::MATERIAL_PROPERTY_FLOAT	:
			{
				MaterialPropertyFloat* existingProperty = reinterpret_cast< MaterialPropertyFloat* >( property );
				m_properties.push_back( new MaterialPropertyFloat( existingProperty->m_name, existingProperty->m_value, m_propertyBlocksByProgram ) );
				break;
			}
			case MaterialPropertyType::MATERIAL_PROPERTY_VECTOR_2	:
			{
				MaterialPropertyVector2* existingProperty = reinterpret_cast< MaterialPropertyVector2* >( property );
				m_properties.push_back( new MaterialPropertyVector2( existingProperty->m_name, existingProperty->m_value, m_propertyBlocksByProgram ) );
				break;
			}
			case MaterialPropertyType::MATERIAL_PROPERTY_VECTOR_3	:
			{
				MaterialPropertyVector3* existingProperty = reinterpret_cast< MaterialPropertyVector3* >( property );
				m_properties.push_back( new MaterialPropertyVector3( existingProperty->m_name, existingProperty->m_value, m_propertyBlocksByProgram ) );
				break;
			}
			case MaterialPropertyType::MATERIAL_PROPERTY_VECTOR_4	:
			{
				MaterialPropertyVector4* existingProperty = reinterpret_cast< MaterialPropertyVector4* >( property );
				m_properties.push_back( new MaterialPropertyVector4( existingProperty->m_name, existingProperty->m_value, m_propertyBlocksByProgram ) );
				break;
			}
		}
	}
}

void Material::CreatePropertyBlocksPerProgram()
{
	for ( const ShaderProgram* program : m_shader->GetPrograms() )
	{
		const ShaderProgramInfo* programInfo = program->GetInfo();
		
		for ( size_t blockIndex = 0; blockIndex < programInfo->m_propertyBlockInfos.size(); blockIndex++ )
		{
			m_propertyBlocksByProgram[ program->GetHandle() ].push_back( new PropertyBlock( &programInfo->m_propertyBlockInfos[ blockIndex ] ) );
		}
	}
}

void Material::ErasePropertyBlocksForUnownedBlocks()
{
	std::vector< std::string > propertyNames;
	for ( MaterialProperty* property : m_properties )
	{
		propertyNames.push_back( property->GetName() );
	}

	for ( std::map< unsigned int, std::vector< PropertyBlock* > >::iterator blockIterator = m_propertyBlocksByProgram.begin(); blockIterator != m_propertyBlocksByProgram.end(); blockIterator++ )
	{
		std::vector< PropertyBlock* >& propertyBlocks = blockIterator->second;
		
		for ( size_t blockIndex = 0; blockIndex < propertyBlocks.size(); blockIndex++ )
		{
			bool blockContainsProperty = false;
			for ( std::string propertyName : propertyNames )
			{
				if ( propertyBlocks[ blockIndex ]->GetInfo()->ContainsProperty( propertyName ) )
				{
					blockContainsProperty = true;
					break;
				}
			}
			if ( !blockContainsProperty )
			{
				propertyBlocks.erase( propertyBlocks.begin() + blockIndex );
				blockIndex--;
			}
		}
	}
}

Material* Material::GetInstance() const
{
	return new Material( this );
}

Shader* Material::GetShader() const
{
	return m_shader;
}

unsigned int Material::GetProgramHandle( unsigned int passIndex /* = 0U */ ) const
{
	return m_shader->GetPass( passIndex )->GetProgramHandle();
}

MaterialProperty* Material::GetProperty( const std::string& name ) const
{
	MaterialProperty* foundProperty = nullptr;

	for ( MaterialProperty* property : m_properties )
	{
		if ( property->GetName() == name )
		{
			foundProperty = property;
			break;
		}
	}

	return foundProperty;
}

MaterialProperty* Material::GetProperty( unsigned int propertyIndex ) const
{
	return m_properties[ propertyIndex ];
}

float Material::GetSpecularAmount() const
{
	return m_lightProperties->m_specularAmount;
}

float Material::GetSpecularPower() const
{
	return m_lightProperties->m_specularPower;
}

int Material::GetPropertyIndex( const std::string& name ) const
{
	int foundPropertyIndex = -1;

	for ( size_t propertyIndex = 0; propertyIndex < m_properties.size(); propertyIndex++ )
	{
		if ( m_properties[ propertyIndex ]->GetName() == name )
		{
			foundPropertyIndex = static_cast< int >( propertyIndex );
			break;
		}
	}

	return foundPropertyIndex;
}

const Texture* Material::GetTexture( unsigned int index ) const
{
	return m_textures[ index ];
}

const Sampler* Material::GetSampler( unsigned int index ) const
{
	return m_samplers[ index ];
}

unsigned int Material::GetPropertyCount() const
{
	return static_cast< unsigned int >( m_properties.size() );
}

unsigned int Material::GetTextureCount() const
{
	return static_cast< unsigned int >( m_textures.size() );
}

unsigned int Material::GetSamplerCount() const
{
	return static_cast< unsigned int >( m_samplers.size() );
}

bool Material::IsInstance() const
{
	return m_isInstance;
}

bool Material::UsesLights( unsigned int passIndex ) const
{
	return m_shader->GetPass( passIndex )->UsesLights();
}

RenderQueue Material::GetRenderQueue( unsigned int passIndex ) const
{
	return m_shader->GetPass( passIndex )->GetRenderQueue();
}

int Material::GetLayer( unsigned int passIndex ) const
{
	return m_shader->GetPass( passIndex )->GetLayer();
}

void Material::DeleteAllMaterialData()
{
	delete m_lightProperties;
	m_lightProperties = nullptr;

	for ( std::map< unsigned int, std::vector< PropertyBlock* > >::iterator blockIterator = m_propertyBlocksByProgram.begin(); blockIterator != m_propertyBlocksByProgram.end(); blockIterator++ )
	{
		for ( PropertyBlock* block : blockIterator->second )
		{
			delete block;
			block = nullptr;
		}
	}
	m_propertyBlocksByProgram.clear();

	DeleteAllProperties();
}

void Material::DeleteAllProperties()
{
	for ( size_t propertyIndex = 0; propertyIndex < m_properties.size(); propertyIndex++ )
	{
		delete m_properties[ propertyIndex ];
		m_properties[ propertyIndex ] = nullptr;
	}
	m_properties.clear();
}

void Material::DeleteExistingProperty( const std::string& name )
{
	int existingPropertyIndex = GetPropertyIndex( name );
	if ( existingPropertyIndex != -1 )
	{
		delete m_properties[ existingPropertyIndex ];
		m_properties.erase( m_properties.begin() + existingPropertyIndex );
	}
}

void Material::BindPropertyBlocks( unsigned int passIndex )
{
	for ( PropertyBlock* block : m_propertyBlocksByProgram[ m_shader->GetPass( passIndex )->GetProgramHandle() ] )
	{
		block->CopyToGPU();
		block->Bind();
	}
}

void Material::AddProperty( MaterialProperty* property )
{
	if ( property != nullptr )
	{
		m_properties.push_back( property );
	}
}

void Material::SetProperty( const std::string& name, float value )
{
	MaterialProperty* existingProperty = GetProperty( name );
	if ( existingProperty != nullptr )
	{
		if ( existingProperty->GetType() == MaterialPropertyType::MATERIAL_PROPERTY_FLOAT )
		{
			reinterpret_cast< MaterialPropertyFloat* >( existingProperty )->SetValue( value );
		}
		else
		{
			DeleteExistingProperty( name );
			AddProperty( new MaterialPropertyFloat( name, value, m_propertyBlocksByProgram ) );
		}
	}
	else
	{
		AddProperty( new MaterialPropertyFloat( name, value, m_propertyBlocksByProgram ) );
	}
}

void Material::SetProperty( const std::string& name, const Vector2& value )
{
	MaterialProperty* existingProperty = GetProperty( name );
	if ( existingProperty != nullptr )
	{
		if ( existingProperty->GetType() == MaterialPropertyType::MATERIAL_PROPERTY_VECTOR_2 )
		{
			reinterpret_cast< MaterialPropertyVector2* >( existingProperty )->SetValue( value );
		}
		else
		{
			DeleteExistingProperty( name );
			AddProperty( new MaterialPropertyVector2( name, value, m_propertyBlocksByProgram ) );
		}
	}
	else
	{
		AddProperty( new MaterialPropertyVector2( name, value, m_propertyBlocksByProgram ) );
	}
}

void Material::SetProperty( const std::string& name, const Vector3& value )
{
	MaterialProperty* existingProperty = GetProperty( name );
	if ( existingProperty != nullptr )
	{
		if ( existingProperty->GetType() == MaterialPropertyType::MATERIAL_PROPERTY_VECTOR_3 )
		{
			reinterpret_cast< MaterialPropertyVector3* >( existingProperty )->SetValue( value );
		}
	}
	else
	{
		AddProperty( new MaterialPropertyVector3( name, value, m_propertyBlocksByProgram ) );
	}
}

void Material::SetProperty( const std::string& name, const Vector4& value )
{
	MaterialProperty* existingProperty = GetProperty( name );
	if ( existingProperty != nullptr )
	{
		if ( existingProperty->GetType() == MaterialPropertyType::MATERIAL_PROPERTY_VECTOR_4 )
		{
			reinterpret_cast< MaterialPropertyVector4* >( existingProperty )->SetValue( value );
		}
		else
		{
			DeleteExistingProperty( name );
			AddProperty( new MaterialPropertyVector4( name, value, m_propertyBlocksByProgram ) );
		}
	}
	else
	{
		AddProperty( new MaterialPropertyVector4( name, value, m_propertyBlocksByProgram ) );
	}
}

void Material::SetProperty( const std::string& name, const Rgba& value )
{
	MaterialProperty* existingProperty = GetProperty( name );
	if ( existingProperty != nullptr )
	{
		if ( existingProperty->GetType() == MaterialPropertyType::MATERIAL_PROPERTY_VECTOR_4 )
		{
			reinterpret_cast< MaterialPropertyRgba* >( existingProperty )->SetValue( value.GetAsFloats() );
		}
		else
		{
			DeleteExistingProperty( name );
			AddProperty( new MaterialPropertyRgba( name, value, m_propertyBlocksByProgram ) );
		}
	}
	else
	{
		AddProperty( new MaterialPropertyRgba( name, value, m_propertyBlocksByProgram ) );
	}
}

void Material::SetProperty( const std::string& name, const Matrix44& value )
{
	MaterialProperty* existingProperty = GetProperty( name );
	if ( existingProperty != nullptr )
	{
		if ( existingProperty->GetType() == MaterialPropertyType::MATERIAL_PROPERTY_MATRIX44 )
		{
			reinterpret_cast< MaterialPropertyMatrix44* >( existingProperty )->SetValue( value );
		}
		else
		{
			DeleteExistingProperty( name );
			AddProperty( new MaterialPropertyMatrix44( name, value, m_propertyBlocksByProgram ) );
		}
	}
	else
	{
		AddProperty( new MaterialPropertyMatrix44( name, value, m_propertyBlocksByProgram ) );
	}
}

void Material::SetSpecularAmount( float specAmount )
{
	m_lightProperties->m_specularAmount = specAmount;
}

void Material::SetSpecularPower( float specPower )
{
	m_lightProperties->m_specularPower = specPower;
}

void Material::SetSpecularProperties( float specAmount, float specPower )
{
	SetSpecularAmount( specAmount );
	SetSpecularPower( specPower );
}

void Material::AddTextureAndSampler( const Texture* texture, const Sampler* sampler )
{
	m_textures.push_back( texture );
	m_samplers.push_back( sampler );
}

void Material::SetTexture( unsigned int index, const Texture* texture )
{
	if ( static_cast< int >( index ) > static_cast< int >( m_textures.size() - 1 ) )
	{
		int currentIndex = static_cast< int >( m_textures.size() - 1 );
		
		while( currentIndex < static_cast< int >( index ) )
		{
			m_textures.push_back( nullptr );
			currentIndex++;
		}
	}
	m_textures[ index ] = texture;
}

void Material::SetSampler( unsigned int index, const Sampler* sampler )
{
	if ( static_cast< int >( index ) > static_cast< int >( m_samplers.size() - 1 ) )
	{
		int currentIndex = static_cast< int >( m_samplers.size() - 1 );
		
		while( currentIndex < static_cast< int >( index ) )
		{
			m_samplers.push_back( Renderer::GetInstance()->GetDefaultSampler() );
			currentIndex++;
		}
	}
	m_samplers[ index ] = sampler;
}

void Material::SetTextureAndSampler( unsigned int index, const Texture* texture, const Sampler* sampler )
{
	SetTexture( index, texture );
	SetSampler( index, sampler );
}

/* static */
Material* Material::FromShader( Shader* shader )
{
	return new Material( shader );
}

/* static */
Material* Material::FromSource( const Material* material )
{
	return new Material( material );
}
