#pragma once

#include "Engine/Math/Matrix44.hpp"
#include "ThirdParty/tinyXML2/tinyxml2.h"
#include <map>
#include <string>
#include <vector>

enum RenderQueue;
struct ObjectLightPropertiesUBO;
class Matrix44;
class PropertyBlock;
class Rgba;
class Sampler;
class Shader;
class ShaderPass;
class Texture;
class Vector2;
class Vector3;
class Vector4;

#pragma region MaterialProperty Types

enum MaterialPropertyType
{
	MATERIAL_PROPERTY_INVALID = -1,
	MATERIAL_PROPERTY_FLOAT,
	MATERIAL_PROPERTY_VECTOR_2,
	MATERIAL_PROPERTY_VECTOR_3,
	MATERIAL_PROPERTY_VECTOR_4,
	MATERIAL_PROPERTY_MATRIX44,
	NUM_MATERIAL_PROPERTY_TYPES
};

class MaterialProperty
{

	friend class Material;

public:
	MaterialProperty( const std::string& name, MaterialPropertyType type, std::map< unsigned int, std::vector< PropertyBlock* > > propertyBlocksByProgram );
	~MaterialProperty();

public:
	std::string GetName() const;
	MaterialPropertyType GetType() const;

protected:
	std::string m_name = "";
	MaterialPropertyType m_type = MaterialPropertyType::MATERIAL_PROPERTY_INVALID;
	std::map< unsigned int, std::vector< PropertyBlock* > > m_propertyBlocksByProgram;

};

class MaterialPropertyFloat : public MaterialProperty
{

	friend class Material;

public:
	MaterialPropertyFloat( const std::string& name, float value, std::map< unsigned int, std::vector< PropertyBlock* > > propertyBlocksByProgram );
	~MaterialPropertyFloat();

public:
	float GetValue() const;
	void SetValue( float newValue ); // Sets value and updates Property Block

private:
	float m_value = 0.0f;

};

class MaterialPropertyVector2 : public MaterialProperty
{

	friend class Material;

public:
	MaterialPropertyVector2( const std::string& name, const Vector2& value, std::map< unsigned int, std::vector< PropertyBlock* > > propertyBlocksByProgram );
	MaterialPropertyVector2( const std::string& name, float xValue, float yValue, std::map< unsigned int, std::vector< PropertyBlock* > > propertyBlocksByProgram );
	~MaterialPropertyVector2();

public:
	Vector2 GetValue() const;
	void SetValue( const Vector2& newValue ); // Sets value and updates Property Block

private:
	Vector2 m_value = Vector2::ZERO;

};

class MaterialPropertyVector3 : public MaterialProperty
{

	friend class Material;

public:
	MaterialPropertyVector3( const std::string& name, const Vector3& value, std::map< unsigned int, std::vector< PropertyBlock* > > propertyBlocksByProgram );
	MaterialPropertyVector3( const std::string& name, const Rgba& color, std::map< unsigned int, std::vector< PropertyBlock* > > propertyBlocksByProgram );
	MaterialPropertyVector3( const std::string& name, float xValue, float yValue, float zValue, std::map< unsigned int, std::vector< PropertyBlock* > > propertyBlocksByProgram );
	~MaterialPropertyVector3();

public:
	Vector3 GetValue() const;
	void SetValue( const Vector3& newValue ); // Sets value and updates Property Block

private:
	Vector3 m_value = Vector3::ZERO;

};

class MaterialPropertyVector4 : public MaterialProperty
{

	friend class Material;

public:
	MaterialPropertyVector4( const std::string& name, const Vector4& value, std::map< unsigned int, std::vector< PropertyBlock* > > propertyBlocksByProgram );
	MaterialPropertyVector4( const std::string& name, const Rgba& color, std::map< unsigned int, std::vector< PropertyBlock* > > propertyBlocksByProgram );
	MaterialPropertyVector4( const std::string& name, const Vector3& xyzValue, float wValue, std::map< unsigned int, std::vector< PropertyBlock* > > propertyBlocksByProgram );
	MaterialPropertyVector4( const std::string& name, float xValue, float yValue, float zValue, float wValue, std::map< unsigned int, std::vector< PropertyBlock* > > propertyBlocksByProgram );
	~MaterialPropertyVector4();

public:
	Vector4 GetValue() const;
	void SetValue( const Vector4& newValue ); // Sets value and updates Property Block

protected:
	Vector4 m_value = Vector4::ZERO_DISPLACEMENT;

};

class MaterialPropertyRgba : public MaterialPropertyVector4
{

	friend class Material;

public:
	MaterialPropertyRgba( const std::string& name, const Rgba& color, std::map< unsigned int, std::vector< PropertyBlock* > > propertyBlocksByProgram );
	MaterialPropertyRgba( const std::string& name, unsigned char rValue, unsigned char gValue, unsigned char bValue, unsigned char aValue, std::map< unsigned int, std::vector< PropertyBlock* > > propertyBlocksByProgram );
	~MaterialPropertyRgba();

public:
	Rgba GetValueAsRgba() const;
	void SetValue( const Vector4& newValue ); // Sets value and updates Property Block
	void SetValue( const Rgba& newValue ); // Sets value and updates Property Block

};

class MaterialPropertyMatrix44 : public MaterialProperty
{
	friend class Material;

public:
	MaterialPropertyMatrix44( const std::string& name, const Matrix44& value, std::map< unsigned int, std::vector< PropertyBlock* > > propertyBlocksByProgram );
	~MaterialPropertyMatrix44();

public:
	Matrix44 GetValue() const;
	void SetValue( const Matrix44& newValue ); // Sets value and updates Property Block

protected:
	Matrix44 m_value = Matrix44::IDENTITY;
};

#pragma endregion

class Material
{

public:
	explicit Material( Shader* shader, bool isInstance = false );
	explicit Material( const Material* sourceMaterial );
	explicit Material( const Material& copyMaterial );
	explicit Material( const tinyxml2::XMLElement& materialElement );
	~Material();

	Material* GetInstance() const;
	Shader* GetShader() const;
	unsigned int GetProgramHandle( unsigned int passIndex = 0U ) const;
	MaterialProperty* GetProperty( const std::string& name ) const; // Returns nullptr if property not found
	MaterialProperty* GetProperty( unsigned int propertyIndex ) const; // Returns nullptr if property not found
	float GetSpecularAmount() const;
	float GetSpecularPower() const;
	const Texture* GetTexture( unsigned int index ) const;
	const Sampler* GetSampler( unsigned int index ) const;
	unsigned int GetPropertyCount() const;
	unsigned int GetTextureCount() const;
	unsigned int GetSamplerCount() const;
	bool IsInstance() const;
	bool UsesLights( unsigned int passIndex ) const;
	RenderQueue GetRenderQueue( unsigned int passIndex ) const;
	int GetLayer( unsigned int passIndex ) const;

	void BindPropertyBlocks( unsigned int passIndex );

	void AddProperty( MaterialProperty* property );
	void SetProperty( const std::string& name, float value );
	void SetProperty( const std::string& name, const Vector2& value );
	void SetProperty( const std::string& name, const Vector3& value );
	void SetProperty( const std::string& name, const Vector4& value );
	void SetProperty( const std::string& name, const Rgba& value );
	void SetProperty( const std::string& name, const Matrix44& value );

	void SetSpecularAmount( float specAmount );
	void SetSpecularPower( float specPower );
	void SetSpecularProperties( float specAmount, float specPower );

	void AddTextureAndSampler( const Texture* texture, const Sampler* sampler );
	void SetTexture( unsigned int index, const Texture* texture );
	void SetSampler( unsigned int index, const Sampler* sampler );
	void SetTextureAndSampler( unsigned int index, const Texture* texture, const Sampler* sampler );

private:
	void InitializeMaterialPropertiesFromShader();
	void DuplicateMaterialProperties( const Material& materialToCopy );

	void CreatePropertyBlocksPerProgram();
	void ErasePropertyBlocksForUnownedBlocks(); // Removes blocks from m_propertyBlocksByProgram if no properties in m_properties are present in them

	int GetPropertyIndex( const std::string& name ) const; // Returns -1 if property not found

	void DeleteAllProperties();
	void DeleteExistingProperty( const std::string& name );

public:
	static Material* FromShader( Shader* shader );	// Returns a copy
	static Material* FromSource( const Material* material );

public:
	bool m_isInstance = false;
	Shader* m_shader = nullptr;
	std::vector< MaterialProperty* > m_properties;
	std::map< unsigned int, std::vector< PropertyBlock* > > m_propertyBlocksByProgram;
	std::vector< const Texture* > m_textures;
	std::vector< const Sampler* > m_samplers;
	ObjectLightPropertiesUBO* m_lightProperties = nullptr;

};
