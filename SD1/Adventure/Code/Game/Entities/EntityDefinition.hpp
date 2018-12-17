#pragma once

#include "Game/Tags.hpp"
#include "Game/Entities/Stats.hpp"
#include "Game/World/TileDefinition.hpp"
#include "Engine/Renderer/SpriteAnimationSetDefinition.hpp"
#include "ThirdParty/tinyxml2/tinyxml2.h"
#include <map>
#include <string>

class EntityDefinition
{

	friend class ActorDefinition;
	friend class ProjectileDefinition;
	friend class PortalDefinition;

public:
	~EntityDefinition();

protected:
	explicit EntityDefinition( const tinyxml2::XMLElement& entityDefinitionElement );

public:
	std::string GetName() const;
	bool CanSee() const;
	bool CanWalk() const;
	bool CanFly() const;
	bool CanSwim() const;
	float GetPhysicsRadius() const;
	float GetFrictionPerSecond() const;
	AABB2 GetDrawBounds() const;
	Tags GetDefaultTags() const;
	SpriteAnimationSetDefinition* GetSpriteAnimationSetDefinition() const;
	const std::map< TileDefinition*, float > GetTileDefinitionToMovementPenaltyMap() const;
	void RenderDeveloperModeVertices( float renderAlpha ) const;

protected:
	virtual void PopulateDataFromXml( const tinyxml2::XMLElement& entityDefinitionElement );

private:
	void PopulatePhysicalDiscVertices();

protected:
	static constexpr char SPRITE_ANIM_SET_XML_NODE_NAME[] = "SpriteAnimSet";
	static constexpr char PHYSICS_XML_NODE_NAME[] = "Physics";
	static constexpr char MOVEMENT_XML_NODE_NAME[] = "Movement";
	static constexpr char TILE_XML_NODE_NAME[] = "Tile";
	static constexpr char NAME_XML_ATTRIBUTE_NAME[] = "name";
	static constexpr char SIGHT_XML_NODE_NAME[] = "See";
	static constexpr char WALKING_XML_NODE_NAME[] = "Walk";
	static constexpr char FLYING_XML_NODE_NAME[] = "Fly";
	static constexpr char SWIMMING_XML_NODE_NAME[] = "Swim";
	static constexpr char TAGS_XML_ATTRIBUTE_NAME[] = "tags";
	static constexpr char TILE_TYPE_XML_ATTRIBUTE_NAME[] = "type";
	static constexpr char TILE_PENALTY_XML_ATTRIBUTE_NAME[] = "penalty";
	static constexpr char IS_PLAYER_CONTROLLED_ENTITY_NAME[] = "Player";
	static constexpr char PHYSICS_RADIUS_XML_ATTRIBUTE_NAME[] = "physicsRadius";
	static constexpr char DRAW_BOUNDS_XML_ATTRIBUTE_NAME[] = "localDrawBounds";
	static constexpr char FRICTION_XML_ATTRIBUTE_NAME[] = "friction";

protected:
	std::string m_name = "";
	bool m_canSee = false;
	bool m_canWalk = false;
	bool m_canFly = false;
	bool m_canSwim = false;
	float m_physicsRadius = 0.0f;
	float m_frictionPerSecond = 1.0f;		// Entities will stop immediately by default (They won't stop if this isn't the case!)
	Vector2 m_physicalDiscVertices[ CIRCLE_NUM_SIDES ];
	AABB2 m_drawBounds;
	SpriteAnimationSetDefinition* m_spriteAnimSetDefinition = nullptr;
	std::map< TileDefinition*, float > m_movementPenaltiesByTileDefinition;
	Tags m_defaultTags;

};
