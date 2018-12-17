#include "Game/GameCommon.hpp"
#include "Game/Entities/Actor.hpp"
#include "Game/Entities/Item.hpp"
#include "Engine/Math/MathUtils.hpp"

Item::Item( const std::string instanceName, const Vector2& position, ItemDefinition* itemDefinition, const Map& map )
	:	Entity( instanceName, position, itemDefinition, map ),
		m_itemDefinition( itemDefinition )
{

}

Item::~Item()
{

}

ItemDefinition* Item::GetItemDefinition() const
{
	return m_itemDefinition;
}

SpriteAnimationSet& Item::GetSpriteAnimationSet() const
{
	return *m_spriteAnimationSet;
}

void Item::Update( float deltaSeconds )
{
	Entity::Update( deltaSeconds );
}

void Item::AddToInventory( Actor* heldByActor )
{
	m_heldByActor = heldByActor;
	m_isInInventory = true;
}

void Item::RemoveFromInventory( const Vector2& dropLocation, const Map* mapToDropIn )
{
	if ( m_map != mapToDropIn )
	{
		m_map->RemoveItemFromMap( this );
		m_map = const_cast< Map* >( mapToDropIn );
		m_map->AddExistingItem( this );
	}

	m_position = dropLocation;
	m_heldByActor = nullptr;
	m_isInInventory = false;
}

void Item::Render( float renderAlpha, bool developerModeEnabled ) const
{
	if ( !IsInInventory() )		// Don't render at all if one of these two conditions isn't met
	{
		float angleToRotate =  m_orientation - m_spriteAnimationSet->GetCurrentSpriteAnimation()->GetDefinition()->GetOrientationOffset();

		g_renderer->PushMatrix();
		g_renderer->Translate( m_position.x, m_position.y, 0.0f );
		g_renderer->Rotate( angleToRotate, 0.0f, 0.0f, 1.0f );

		g_renderer->DrawTexturedAABB( m_entityDefinition->GetDrawBounds(), *m_spriteAnimationSet->GetTextureForSpriteAnimation( "Environment" ), m_spriteAnimationSet->GetFrameUVsFromSpriteAnimation( 0, "Environment" ).mins, m_spriteAnimationSet->GetFrameUVsFromSpriteAnimation( 0, "Environment" ).maxs, Rgba::WHITE.GetWithAlpha( renderAlpha ) );
		if ( developerModeEnabled )
		{
			m_entityDefinition->RenderDeveloperModeVertices( renderAlpha );
		}

		g_renderer->PopMatrix();
	}
}

void Item::RenderInHUD( const AABB2& bounds, float renderAlpha ) const
{
	AABB2 boundsForSprite = AABB2( bounds );
	boundsForSprite.AddPaddingToSides( -g_gameConfigBlackboard.GetValue( "hudSpriteBorderPadding", 0.1f ), -g_gameConfigBlackboard.GetValue( "hudSpriteBorderPadding", 0.1f ) );
	g_renderer->DrawTexturedAABB( boundsForSprite, *m_spriteAnimationSet->GetTextureForSpriteAnimation( "Environment" ), m_spriteAnimationSet->GetFrameUVsFromSpriteAnimation( 0, "Environment" ).mins, m_spriteAnimationSet->GetFrameUVsFromSpriteAnimation( 0, "Environment" ).maxs, Rgba::WHITE.GetWithAlpha( renderAlpha ) );
}

void Item::RenderSprite( const std::string& currentAnimationName, int currentSpriteIndex, const AABB2& renderBounds, const Rgba& renderTint, float renderAlpha ) const
{
	AABB2 texCoordsForSprite = m_spriteAnimationSet->GetFrameUVsFromSpriteAnimation( currentSpriteIndex, currentAnimationName );
	const Texture* textureForSprite = m_spriteAnimationSet->GetTextureForSpriteAnimation( currentAnimationName );
	g_renderer->DrawTexturedAABB( renderBounds, *textureForSprite, texCoordsForSprite.mins, texCoordsForSprite.maxs, renderTint.GetWithAlpha( renderAlpha ) );
}

Actor* Item::GetHoldingActor() const
{
	return m_heldByActor;
}

bool Item::IsInInventory() const
{
	return m_isInInventory;
}
