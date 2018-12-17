#pragma once

#include "Game/Entities/ItemDefinition.hpp"
#include "Game/Entities/Entity.hpp"
#include "Engine/Math/Vector2.hpp"
#include <string>

 class Actor;

class Item : public Entity
{

	friend class Map;

public:
	~Item();

	ItemDefinition* GetItemDefinition() const;
	void Update( float deltaSeconds ) override;
	void AddToInventory( Actor* heldByActor );
	void RemoveFromInventory( const Vector2& dropLocation, const Map* mapToDropIn );
	Actor* GetHoldingActor() const;
	bool IsInInventory() const;
	void Render( float renderAlpha, bool developerModeEnabled ) const override;
	void RenderInHUD( const AABB2& bounds, float renderAlpha ) const;
	void RenderSprite( const std::string& currentAnimationName, int currentSpriteIndex, const AABB2& renderBounds, const Rgba& renderTint, float renderAlpha ) const;
	SpriteAnimationSet& GetSpriteAnimationSet() const;

protected:
	explicit Item( const std::string instanceName, const Vector2& position, ItemDefinition* itemDefinition, const Map& map );

protected:
	ItemDefinition* m_itemDefinition = nullptr;
	bool m_isInInventory = false;
	Actor* m_heldByActor = nullptr;

};
