#pragma once

#include "Game/Entities/EntityDefinition.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Disc2.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/SpriteAnimationSet.hpp"
#include "ThirdParty/tinyXML2/tinyxml2.h"
#include <string>

class Item;
class Map;

class Entity
{

public:
	~Entity();

public:
	virtual void Update( float deltaSeconds );
	void ApplyCorrectiveTranslation( const Vector2& translation );
	void SetPosition( const Vector2& newPosition );
	virtual void Render( float renderAlpha, bool developerModeEnabled ) const;
	std::string GetInstanceName() const;
	Vector2 GetLocation() const;
	Vector2 GetVelocity() const;
	float GetOrientationDegrees() const;
	EntityDefinition* GetEntityDefinition() const;
	Map* GetMap() const;
	Tags GetTagSet() const;
	std::vector< Item* > GetInventory() const;

public:
	static StatID GetStatIDForName( const std::string& statIDName );
	static std::string GetNameForStatID( StatID statID );

protected:
	explicit Entity( const std::string instanceName, const Vector2& position, EntityDefinition* entityDefinition, const Map& map );

protected:
	void Rotate( float deltaSeconds );
	virtual void Translate( float deltaSeconds );		// For entities to use their Agility stat, if needed

protected:
	static constexpr char NAME_XML_ATTRIBUTE_NAME[] = "name";
	static constexpr char POSITION_ATTRIBUTE_NAME[] = "position";

protected:
	std::string m_instanceName = "";
	EntityDefinition* m_entityDefinition = nullptr;
	SpriteAnimationSet* m_spriteAnimationSet = nullptr;
	Vector2 m_position = Vector2::ZERO;
	Vector2 m_velocity = Vector2::ZERO;
	float m_orientation = 0.0f;
	float m_rotationSpeed = 0.0f;
	Map* m_map = nullptr;
	Tags m_tags;
	std::vector< Item* > m_inventory;

};
