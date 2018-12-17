#include "Game/TheGame.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Entities/ActorDefinition.hpp"
#include "Game/Entities/ItemDefinition.hpp"
#include "Game/Entities/ProjectileDefinition.hpp"
#include "Game/Entities/PortalDefinition.hpp"
#include "Game/World/AdventureDefinition.hpp"
#include "Game/World/MapDefinition.hpp"
#include "Game/World/TileDefinition.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "ThirdParty/tinyXML2/tinyxml2.h"

TheGame::TheGame()
{
	InitializeTileSpriteSheet();
	CreateTileDefinitionsFromXml();
	CreateProjectileDefinitionsFromXml();
	CreateItemDefinitionsFromXml();
	CreateLootDefinitionsFromXml();
	CreateActorDefinitionsFromXml();
	CreateMapDefinitionsFromXml();
	CreatePortalDefinitionsFromXml();
	CreateAdventureDefinitionsFromXml();
	g_mainCamera = new Camera( g_gameConfigBlackboard.GetValue( "cameraNumTilesInViewVertically", 10.0f ), AABB2( Vector2::ZERO, Vector2::ZERO ) );
}

TheGame::~TheGame()
{
	DeleteAdventureDefinitions();
	DeleteEntityDefinitions();
	DeleteLootDefinitions();
	DeleteMapDefinitions();
	DeleteTileDefinitions();

	delete m_currentAdventure;
	m_currentAdventure = nullptr;

	delete g_mainCamera;
	g_mainCamera = nullptr;

	delete g_tileSpriteSheet;
	g_tileSpriteSheet = nullptr;
}

void TheGame::StartTransitionToState( GameState newGameState )
{
	m_transitionToState = newGameState;
	m_numSecondsIntoStateTransition = 0.0f;
}


Adventure* TheGame::GetCurrentAdventure() const
{
	return m_currentAdventure;
}

void TheGame::PushDialogue( Dialogue* newDialogue )
{
	if ( newDialogue == nullptr )
	{
		return;
	}
	m_dialogueQueue.push_back( newDialogue );
}

void TheGame::PushDialogue( const std::string& text, const AABB2& bounds, DialogueStyle dialogueStyle = DialogueStyle::DIALOGUE_STYLE_NONE )
{
	Dialogue* newDialogue = new Dialogue( text, bounds, dialogueStyle );
	m_dialogueQueue.push_back( newDialogue );
}

void TheGame::Update( float& deltaSeconds )
{
	HandleDebugKeyboardInput( deltaSeconds );
	HandleStateTransitionKeyboardInput();		// State changes can happen here?
	HandleStateTransitionXboxControllerInput();
	UpdateStateVariables( deltaSeconds );

	g_mainCamera->Update();

	switch ( m_currentGameState )
	{
		case	GameState::ATTRACT		:	Update_Attract( deltaSeconds );	break;
		case	GameState::PLAYING		:	Update_Playing( deltaSeconds );	break;
		case	GameState::DIALOGUE		:	Update_Dialogue( deltaSeconds );	break;
		case	GameState::INVENTORY	:	Update_Inventory( deltaSeconds );	break;
		case	GameState::PAUSED		:	Update_Paused( deltaSeconds );	break;
		case	GameState::DEFEAT		:	Update_Defeat( deltaSeconds );	break;
		case	GameState::VICTORY		:	Update_Victory( deltaSeconds );	break;
		default							:	break;
	}

	/*
	if ( m_transitionToState != GameState::NONE && !m_isFinishedTransitioning )
	{
		switch ( m_transitionToState )
		{
			case	GameState::ATTRACT	:	Update_Attract( deltaSeconds );	break;
			case	GameState::PLAYING	:	Update_Playing( deltaSeconds );	break;
			case	GameState::PAUSED	:	Update_Paused( deltaSeconds );	break;
			case	GameState::DEFEAT	:	Update_Defeat( deltaSeconds );	break;
			case	GameState::VICTORY	:	Update_Victory( deltaSeconds );	break;
			default						:	break;
		}
	}
	*/
}

void TheGame::Update_Attract( float deltaSeconds )
{
	UNUSED( deltaSeconds );
	if ( m_transitionToState == GameState::NONE )
	{
		HandleAttractScreenKeyboardInput();
		HandleAttractScreenXboxControllerInput();
	}
	if ( m_menuSelectionIndex >= static_cast< int >( AdventureDefinition::s_definitions.size() ) )
	{
		m_menuSelectionIndex = static_cast< int >( AdventureDefinition::s_definitions.size() - 1 );
	}
}

void TheGame::Update_Playing( float deltaSeconds )
{
	if ( !m_dialogueQueue.empty() )
	{
		m_currentGameState = GameState::DIALOGUE;		// No transition; immediately switch
		m_dialogueReturnToState = GameState::PLAYING;
	}
	else if ( m_currentAdventure->HasPlayerWon() )
	{
		m_transitionToState = GameState::VICTORY;
	}
	else if ( m_currentAdventure->GetCurrentMap()->GetPlayerEntity()->IsDead() )
	{
		m_transitionToState = GameState::DEFEAT;
	}
	else if ( !m_currentAdventure->HasPlayerWon() )
	{
		m_currentAdventure->Update( deltaSeconds );
		if ( m_currentAdventure->HasPlayerWon() )
		{
			m_dialogueReturnToState = GameState::PLAYING;
			PushDialogue( m_currentAdventure->GetAdventureDefinition()->GetDialogue(AdventureDialogueType::ADVENTURE_DIALOGUE_TYPE_VICTORY ) );
		}
		else if ( m_currentAdventure->GetCurrentMap()->GetPlayerEntity()->IsDead() )
		{
			m_dialogueReturnToState = GameState::PLAYING;
			PushDialogue( m_currentAdventure->GetAdventureDefinition()->GetDialogue(AdventureDialogueType::ADVENTURE_DIALOGUE_TYPE_DEFEAT ) );
		}
	}
}

void TheGame::Update_Dialogue( float deltaSeconds )
{
	if ( !m_dialogueQueue.empty() )
	{
		Dialogue* currentDialogue = m_dialogueQueue[ 0 ];
		currentDialogue->Update( deltaSeconds );

		if ( currentDialogue->ShouldBeDismissed() )
		{
			m_dialogueQueue.pop_front();
			currentDialogue->ResetDismissalState();
		}
	}
	if ( m_dialogueQueue.empty() )
	{
		m_currentGameState = m_dialogueReturnToState;		// No transition; immediately switch
	}
}

void TheGame::Update_Inventory( float deltaSeconds )
{
	UNUSED( deltaSeconds );
	if ( !m_dialogueQueue.empty() )
	{
		m_currentGameState = GameState::DIALOGUE;		// No transition; immediately switch
		m_dialogueReturnToState = GameState::INVENTORY;
	}
	else
	{
		Player* player = m_currentAdventure->GetCurrentMap()->GetPlayerEntity();
		if ( player->GetInventory().size() == 0 )		// Handle automatic switch to Equipment state if the Inventory is empty
		{
			m_inventoryMenuState = InventoryMenuSelectionState::INVENTORY_MENU_STATE_EQUIPMENT;
			if ( m_menuSelectionIndex > static_cast< int >( EquipSlot::NUM_SLOTS - 1 ) )
			{
				m_menuSelectionIndex = static_cast< int >( EquipSlot::NUM_SLOTS - 1 );
			}
		}

		if ( m_inventoryMenuState == InventoryMenuSelectionState::INVENTORY_MENU_STATE_EQUIPMENT && m_menuSelectionIndex > static_cast< int >( EquipSlot::NUM_SLOTS - 1 ) )
		{
			m_menuSelectionIndex = static_cast< int >( EquipSlot::NUM_SLOTS - 1 );
		}
		else if ( m_inventoryMenuState == InventoryMenuSelectionState::INVENTORY_MENU_STATE_INVENTORY && player->GetInventory().size() > 0 && static_cast< unsigned int >( m_menuSelectionIndex ) > ( player->GetInventory().size() - 1 ) )
		{
			m_menuSelectionIndex = player->GetInventory().size() - 1;
		}

		if ( m_transitionToState == GameState::NONE )
		{
			HandleInventoryScreenKeyboardInput();
			HandleInventoryScreenXboxControllerInput();
		}
	}
}

void TheGame::Update_Paused( float deltaSeconds )
{
	UNUSED( deltaSeconds );
}

void TheGame::Update_Defeat( float deltaSeconds )
{
	UNUSED( deltaSeconds );
}

void TheGame::Update_Victory( float deltaSeconds )
{
	UNUSED( deltaSeconds );
}

void TheGame::UpdateStateVariables( float deltaSeconds )
{
	m_numSecondsInCurrentGameState += deltaSeconds;

	if ( m_transitionToState != GameState::NONE )
	{
		m_numSecondsIntoStateTransition += deltaSeconds;

		if ( m_numSecondsIntoStateTransition > g_gameConfigBlackboard.GetValue( "stateTransitionTime", 1.0f ) )
		{
			m_isFinishedTransitioning = true;
			m_currentGameState = m_transitionToState;
			m_transitionToState = GameState::NONE;
			m_numSecondsIntoStateTransition = 0.0f;
			m_numSecondsInCurrentGameState = 0.0f;
		}
	}
	else
	{
		m_isFinishedTransitioning = false;
	}
}

void TheGame::Render() const	// TODO: Enable developer mode
{
	g_renderer->ClearScreen( Rgba::BLACK );
	g_mainCamera->SetRendererOrtho();

	switch ( m_currentGameState )
	{
		case	GameState::ATTRACT		:	Render_Attract();	break;
		case	GameState::PLAYING		:	Render_Playing();	break;
		case	GameState::DIALOGUE		:	
		{
			if ( m_dialogueReturnToState == GameState::PLAYING )
			{
				Render_Playing();
			}
			else if ( m_dialogueReturnToState == GameState::INVENTORY )
			{
				Render_Inventory();
			}
			Render_Dialogue();
			break;
		}
		case	GameState::INVENTORY	:	Render_Inventory();	break;
		case	GameState::PAUSED		:	Render_Paused();	break;
		case	GameState::DEFEAT		:	Render_Defeat();	break;
		case	GameState::VICTORY		:	Render_Victory();	break;
		default							:	break;
	}

	if ( m_transitionToState != GameState::NONE && !m_isFinishedTransitioning )
	{
		switch ( m_transitionToState )
		{
			case	GameState::ATTRACT		:	Render_Attract();	break;
			case	GameState::PLAYING		:	Render_Playing();	break;
			case	GameState::INVENTORY	:	Render_Inventory();	break;
			case	GameState::PAUSED		:	Render_Paused();	break;
			case	GameState::DEFEAT		:	Render_Defeat();	break;
			case	GameState::VICTORY		:	Render_Victory();	break;
			default							:	break;
		}
	}
}

void TheGame::Render_Attract() const
{
	float alphaFraction = GetRenderAlphaForState( GameState::ATTRACT );

	Rgba backgroundColor = Rgba( 0, 100, 0, 255 );
	Rgba textColor = Rgba::WHITE;

	AABB2 currentOrtho = g_mainCamera->GetViewportBounds();
	g_renderer->DrawAABB( currentOrtho, backgroundColor.GetWithAlpha( alphaFraction ) );
	g_renderer->DrawTextInBox2D( "Adventure", currentOrtho, 1.0f, textColor.GetWithAlpha( alphaFraction ), 1.0f, g_renderer->CreateOrGetBitmapFont( FIXED_FONT_NAME ), TextDrawMode::TEXT_DRAW_OVERRUN, Vector2( 0.5f, 0.0f ) );
	g_renderer->DrawTextInBox2D( "by Abhishek Arora", currentOrtho, 0.2f, textColor.GetWithAlpha( alphaFraction ), 1.0f, g_renderer->CreateOrGetBitmapFont( FIXED_FONT_NAME ), TextDrawMode::TEXT_DRAW_OVERRUN, Vector2( 0.5f, 0.1f ) );
	g_renderer->DrawTextInBox2D( "Controls: (Keyboard/Xbox Controller)\n\nMenu navigation: WASD/D-Pad\nMove character: WASD/Left Analog Stick\nAttack(Melee/Weapon): Space/A\nCast Spell: X/X\nPick up item: E/B\nEquip/Use item: E/A\nUnequip/Drop item: R/Y\nAdvance Dialogue: Space/Y\nInventory: I/Start\nPause: P/Back\nF1 - Toggle Developer Mode", currentOrtho, 0.3f, textColor.GetWithAlpha( alphaFraction ), 1.0f, g_renderer->CreateOrGetBitmapFont( FIXED_FONT_NAME ), TextDrawMode::TEXT_DRAW_SHRINK_TO_FIT, Vector2( 0.0f, 0.5f ) );

	AABB2 adventureListBoxBounds = currentOrtho;
	float adventureListLineHeight = 0.3f;
	adventureListBoxBounds.mins.x = Interpolate( currentOrtho.mins.x, currentOrtho.maxs.x, 0.7f );
	adventureListBoxBounds.maxs.x = Interpolate( currentOrtho.mins.x, currentOrtho.maxs.x, 1.0f );
	adventureListBoxBounds.mins.y = Interpolate( currentOrtho.mins.y, currentOrtho.maxs.y, 0.4f );
	adventureListBoxBounds.maxs.y = adventureListBoxBounds.mins.y + ( adventureListLineHeight * ( g_gameConfigBlackboard.GetValue( "menuAdventureViewNumLines", 5.0f ) + 2.0f ) );
	RenderAdventureList( adventureListBoxBounds, Rgba::WHITE, adventureListLineHeight, alphaFraction );
	adventureListBoxBounds.Translate( adventureListLineHeight * Vector2::UP );		// Make the title render right above the box, and not inside it
	g_renderer->DrawTextInBox2D( "Select Adventure:", adventureListBoxBounds, adventureListLineHeight, textColor.GetWithAlpha( alphaFraction ), 1.0f, g_renderer->CreateOrGetBitmapFont( FIXED_FONT_NAME ), TextDrawMode::TEXT_DRAW_OVERRUN, Vector2( 1.0f, 0.0f ) );
}

void TheGame::RenderAdventureList( const AABB2& adventureListContainerBounds, const Rgba& textColor, float lineHeight, float alphaFraction ) const
{
	Vector2 alignment = Vector2( 0.5f, 0.0f );		// The Adventure list will render in the center of its container

	g_renderer->DrawLineBorder( adventureListContainerBounds, Rgba::WHITE, g_gameConfigBlackboard.GetValue( "menuBorderThickness", 0.3f ) );
	AABB2 currentLineDrawBounds = adventureListContainerBounds;

	IntRange displayedItemRange = ComputeIndexRangeForListPaginationBasedOnGameState();
	if ( displayedItemRange.min > 0 )
	{
		g_renderer->DrawTextInBox2D( "^", currentLineDrawBounds, lineHeight, textColor.GetWithAlpha( alphaFraction ), 1.0f, g_renderer->CreateOrGetBitmapFont( FIXED_FONT_NAME ), TextDrawMode::TEXT_DRAW_OVERRUN, alignment );
	}

	currentLineDrawBounds.maxs.y -= lineHeight;
	std::vector< AdventureDefinition* > adventureDefinitions = AdventureDefinition::GetDefinitionsAsVector();

	for ( int itemIndex = displayedItemRange.min; itemIndex <= displayedItemRange.max; itemIndex++ )
	{
		std::string currentLine = adventureDefinitions[ itemIndex ]->GetName();
		g_renderer->DrawTextInBox2D( currentLine, currentLineDrawBounds, lineHeight, textColor.GetWithAlpha( alphaFraction ), 1.0f, g_renderer->CreateOrGetBitmapFont( FIXED_FONT_NAME ), TextDrawMode::TEXT_DRAW_OVERRUN, alignment );

		if ( m_menuSelectionIndex == itemIndex )
		{
			RenderSelectedItemBoundingBox( currentLine, currentLineDrawBounds, lineHeight, alignment, alphaFraction );
		}

		currentLineDrawBounds.maxs.y -= lineHeight;
	}

	if ( displayedItemRange.max < static_cast< int >( adventureDefinitions.size() - 1 ) )
	{
		g_renderer->DrawTextInBox2D( "v", currentLineDrawBounds, lineHeight, textColor.GetWithAlpha( alphaFraction ), 1.0f, g_renderer->CreateOrGetBitmapFont( FIXED_FONT_NAME ), TextDrawMode::TEXT_DRAW_OVERRUN, alignment );
	}
}

void TheGame::Render_Playing() const
{
	float alphaFraction = GetRenderAlphaForState( GameState::PLAYING );
	Rgba textColor = Rgba::WHITE;

	m_currentAdventure->Render( alphaFraction, m_developerModeEnabled );
	Render_HUD( alphaFraction );
}

void TheGame::Render_HUD( float alphaFraction ) const
{
	AABB2 currentOrtho = g_mainCamera->GetViewportBounds();
	Player* player = m_currentAdventure->GetCurrentMap()->GetPlayerEntity();
	
	// Render weapon sprite in bottom right corner
	Item* currentWeapon = player->GetEquippedItem( EquipSlot::EQUIP_SLOT_WEAPON );
	if ( currentWeapon != nullptr )
	{
		AABB2 weaponSpriteBounds = currentOrtho;
		weaponSpriteBounds.mins.x = Interpolate( currentOrtho.mins.x, currentOrtho.maxs.x, 0.95f );
		weaponSpriteBounds.maxs.x = Interpolate( currentOrtho.mins.x, currentOrtho.maxs.x, 1.0f );
		weaponSpriteBounds.mins.y = Interpolate( currentOrtho.mins.y, currentOrtho.maxs.y, 0.0f );
		weaponSpriteBounds.maxs.y = weaponSpriteBounds.mins.y + weaponSpriteBounds.GetDimensions().x;	// If the screen aspect is not 1, account for it
		currentWeapon->RenderInHUD( weaponSpriteBounds, alphaFraction );
	}
	// Render spell sprite in bottom right corner, to the left of the weapon sprite
	Item* currentSpell = player->GetEquippedItem( EquipSlot::EQUIP_SLOT_SPELL );
	if ( currentSpell != nullptr )
	{
		AABB2 spellSpriteBounds = currentOrtho;
		spellSpriteBounds.mins.x = Interpolate( currentOrtho.mins.x, currentOrtho.maxs.x, 0.9f );
		spellSpriteBounds.maxs.x = Interpolate( currentOrtho.mins.x, currentOrtho.maxs.x, 0.95f );
		spellSpriteBounds.mins.y = Interpolate( currentOrtho.mins.y, currentOrtho.maxs.y, 0.0f );
		spellSpriteBounds.maxs.y = spellSpriteBounds.mins.y + spellSpriteBounds.GetDimensions().x;	// If the screen aspect is not 1, account for it
		currentSpell->RenderInHUD( spellSpriteBounds, alphaFraction );
	}
}

void TheGame::Render_Dialogue() const
{
	float playOverlayAlpha = g_gameConfigBlackboard.GetValue( "dialogModeGameOverlayAlpha", 0.5f );
	AABB2 currentOrtho = g_mainCamera->GetViewportBounds();
	g_renderer->DrawAABB( currentOrtho, Rgba::BLACK.GetWithAlpha( playOverlayAlpha ) );

	float dialogueStateAlphaFraction = GetRenderAlphaForState( GameState::DIALOGUE );
	Dialogue* currentDialogue = m_dialogueQueue[ 0 ];
	currentDialogue->Render( dialogueStateAlphaFraction );
}

void TheGame::Render_Inventory() const
{
	float alphaFraction = GetRenderAlphaForState( GameState::INVENTORY );

	AABB2 currentOrtho = g_mainCamera->GetViewportBounds();
	Vector2 borderPaddingVector = Vector2( g_gameConfigBlackboard.GetValue( "menuBorderPadding", 0.5f ), g_gameConfigBlackboard.GetValue( "menuBorderPadding", 0.5f ) );
	AABB2 borderBounds = AABB2( ( currentOrtho.mins + borderPaddingVector ), ( currentOrtho.maxs - borderPaddingVector ) );
	AABB2 contentBounds = AABB2( ( borderBounds.mins + borderPaddingVector ), ( borderBounds.maxs - borderPaddingVector ) );

	Rgba backgroundColor = Rgba::BLACK;
	Rgba textColor = Rgba::WHITE;
	Rgba borderColor = Rgba::WHITE;

	float lineHeight = 0.4f;
	float listLineHeight = 0.2f;

	g_renderer->DrawAABB( currentOrtho, backgroundColor.GetWithAlpha( alphaFraction ) );
	g_renderer->DrawLineBorder( borderBounds, borderColor, g_gameConfigBlackboard.GetValue( "menuBorderThickness", 0.3f ) );

	RenderStatList( contentBounds, textColor.GetWithAlpha( alphaFraction ), listLineHeight, alphaFraction );
	RenderEquipmentList( contentBounds, textColor.GetWithAlpha( alphaFraction ), listLineHeight, alphaFraction );
	RenderItemList( contentBounds, textColor.GetWithAlpha( alphaFraction ), listLineHeight, alphaFraction );
	RenderSelectedItemDescription( contentBounds, textColor.GetWithAlpha( alphaFraction ), lineHeight, alphaFraction );
	RenderSelectedItemInstructions( contentBounds, textColor.GetWithAlpha( alphaFraction ), lineHeight, alphaFraction );

	g_renderer->DrawTextInBox2D( "Press I or Start to Resume", contentBounds, lineHeight, textColor.GetWithAlpha( alphaFraction ), 0.5f, g_renderer->CreateOrGetBitmapFont( FIXED_FONT_NAME ), TextDrawMode::TEXT_DRAW_SHRINK_TO_FIT, Vector2( 0.5f, 1.0f ) );
}

void TheGame::RenderStatList( const AABB2& screenBounds, const Rgba& textColor, float lineHeight, float alphaFraction ) const
{
	Vector2 alignment = Vector2( 0.0f, 0.0f );		// The Stat list will render at the top-left of the screen

	AABB2 contentBoundsForStatList = screenBounds;
	g_renderer->DrawTextInBox2D( "Stats", screenBounds, lineHeight, textColor.GetWithAlpha( alphaFraction ), 1.0f, g_renderer->CreateOrGetBitmapFont( FIXED_FONT_NAME ), TextDrawMode::TEXT_DRAW_OVERRUN, alignment );
	contentBoundsForStatList.maxs.y -= lineHeight;
	contentBoundsForStatList.mins.y = contentBoundsForStatList.maxs.y - ( lineHeight * static_cast< float >( StatID::NUM_STATS ) );
	contentBoundsForStatList.maxs.x = Interpolate( screenBounds.mins.x, screenBounds.maxs.x, 0.33f );

	Player* playerEntity = m_currentAdventure->GetCurrentMap()->GetPlayerEntity();
	int playerStats[ StatID::NUM_STATS ];
	playerEntity->GetStats( playerStats );
	AABB2 currentLineDrawBounds = contentBoundsForStatList;

	for ( int statID = 0; statID < StatID::NUM_STATS; statID++ )
	{
		Rgba lineRenderColor = GetLineRenderColorForStatListLine( StatID( statID ), playerEntity, textColor );
		std::string currentLine = Entity::GetNameForStatID( StatID( statID ) ) + " " + std::to_string( m_currentAdventure->GetCurrentMap()->GetPlayerEntity()->GetStat( StatID( statID ) ) ) + "/" + std::to_string( m_currentAdventure->GetCurrentMap()->GetPlayerEntity()->GetBaseStat( StatID( statID ) ) );
		g_renderer->DrawTextInBox2D( ( "\n" + currentLine ), currentLineDrawBounds, lineHeight, lineRenderColor.GetWithAlpha( alphaFraction ), 1.0f, g_renderer->CreateOrGetBitmapFont( FIXED_FONT_NAME ), TextDrawMode::TEXT_DRAW_OVERRUN, alignment );

		currentLineDrawBounds.maxs.y -= lineHeight;
	}
}

Rgba TheGame::GetLineRenderColorForStatListLine( StatID statID, const Player* playerEntity, const Rgba& defaultColor ) const
{
	Rgba lineRenderColor = Rgba( defaultColor );
	if ( statID == StatID::STAT_HEALTH )
	{
		if ( playerEntity->GetTagSet().HasTags( Entity::GetNameForStatID( StatID::STAT_POISON_USAGE ) ) )
		{
			lineRenderColor = Rgba::GREEN;
		}
		else if ( playerEntity->GetTagSet().HasTags( Entity::GetNameForStatID( StatID::STAT_FIRE_USAGE ) ) )
		{
			lineRenderColor = Rgba::ORANGE;
		}
	}
	else if ( statID == StatID::STAT_STRENGTH && playerEntity->GetTagSet().HasTags( Entity::GetNameForStatID( StatID::STAT_FIRE_USAGE ) ) )
	{
		lineRenderColor = Rgba::ORANGE;
	}
	else if ( statID == StatID::STAT_DEXTERITY && playerEntity->GetTagSet().HasTags( Entity::GetNameForStatID( StatID::STAT_ICE_USAGE ) ) )
	{
		lineRenderColor = Rgba::CYAN;
	}
	else if ( statID == StatID::STAT_AGILITY && playerEntity->GetTagSet().HasTags( Entity::GetNameForStatID( StatID::STAT_ELECTRICITY_USAGE ) ) )
	{
		lineRenderColor = Rgba::YELLOW;
	}

	return lineRenderColor;
}

void TheGame::RenderEquipmentList( const AABB2& screenBounds, const Rgba& textColor, float lineHeight, float alphaFraction ) const
{
	Vector2 alignment = Vector2( 0.5f, 0.0f );		// The Equipment list will render at the top-center of the screen

	AABB2 contentBoundsForEquipmentList = screenBounds;
	g_renderer->DrawTextInBox2D( "Equipment", screenBounds, lineHeight, textColor.GetWithAlpha( alphaFraction ), 1.0f, g_renderer->CreateOrGetBitmapFont( FIXED_FONT_NAME ), TextDrawMode::TEXT_DRAW_OVERRUN, alignment );
	contentBoundsForEquipmentList.maxs.y -= lineHeight;
	contentBoundsForEquipmentList.mins.y = contentBoundsForEquipmentList.maxs.y - ( lineHeight * static_cast< float >( EquipSlot::NUM_SLOTS ) );
	contentBoundsForEquipmentList.mins.x = Interpolate( screenBounds.mins.x, screenBounds.maxs.x, 0.33f );
	contentBoundsForEquipmentList.maxs.x = Interpolate( screenBounds.maxs.x, screenBounds.mins.x, 0.33f );

	Item** playerEquipment = m_currentAdventure->GetCurrentMap()->GetPlayerEntity()->GetEquipment();
	AABB2 currentLineDrawBounds = contentBoundsForEquipmentList;

	for ( int equipSlotIndex = 0; equipSlotIndex < EquipSlot::NUM_SLOTS; equipSlotIndex++ )
	{
		std::string equipmentSlotItemName = "";
		if ( playerEquipment[ equipSlotIndex ] != nullptr )
		{
			equipmentSlotItemName = playerEquipment[ equipSlotIndex ]->GetInstanceName();
		}
		std::string currentLine = ItemDefinition::GetEquipSlotNameFromEquipSlot( EquipSlot( equipSlotIndex ) ) + " " + equipmentSlotItemName;
		g_renderer->DrawTextInBox2D( ( "\n" + currentLine ), currentLineDrawBounds, lineHeight, textColor.GetWithAlpha( alphaFraction ), 1.0f, g_renderer->CreateOrGetBitmapFont( FIXED_FONT_NAME ), TextDrawMode::TEXT_DRAW_OVERRUN, alignment );

		currentLineDrawBounds.maxs.y -= lineHeight;
		if ( m_inventoryMenuState == InventoryMenuSelectionState::INVENTORY_MENU_STATE_EQUIPMENT && m_menuSelectionIndex == equipSlotIndex )
		{
			RenderSelectedItemBoundingBox( currentLine, currentLineDrawBounds, lineHeight, alignment, alphaFraction );		// Use the currentLineDrawBounds after the Y decrement because of the preceding \n
		}
	}
}

void TheGame::RenderItemList( const AABB2& screenBounds, const Rgba& textColor, float lineHeight, float alphaFraction ) const
{
	Vector2 alignment = Vector2( 1.0f, 0.0f );		// The Item list will render at the top-right of the screen

	AABB2 contentBoundsForInventoryList = screenBounds;
	g_renderer->DrawTextInBox2D( "Inventory", screenBounds, lineHeight, textColor.GetWithAlpha( alphaFraction ), 1.0f, g_renderer->CreateOrGetBitmapFont( FIXED_FONT_NAME ), TextDrawMode::TEXT_DRAW_SHRINK_TO_FIT, alignment );
	contentBoundsForInventoryList.maxs.y -= lineHeight;
	contentBoundsForInventoryList.mins.y = contentBoundsForInventoryList.maxs.y - ( ( lineHeight * g_gameConfigBlackboard.GetValue( "menuInventoryViewNumLines", 3.0f ) ) + 3.0f );		// Allowing two extra lines for arrows, and one for space so that the upper arrow is visible
	contentBoundsForInventoryList.mins.x = Interpolate( screenBounds.mins.x, screenBounds.maxs.x, 0.66f );

	std::vector< Item* > playerInventory = m_currentAdventure->GetCurrentMap()->GetPlayerEntity()->GetInventory();
	AABB2 currentLineDrawBounds = contentBoundsForInventoryList;
	currentLineDrawBounds.maxs.y -= lineHeight;		// Add a blank line

	IntRange displayedItemRange = ComputeIndexRangeForListPaginationBasedOnGameState();
	if ( displayedItemRange.min > 0 )
	{
		g_renderer->DrawTextInBox2D( "^", currentLineDrawBounds, lineHeight, textColor.GetWithAlpha( alphaFraction ), 1.0f, g_renderer->CreateOrGetBitmapFont( FIXED_FONT_NAME ), TextDrawMode::TEXT_DRAW_OVERRUN, alignment );
	}

	currentLineDrawBounds.maxs.y -= lineHeight;

	for ( int itemIndex = displayedItemRange.min; itemIndex <= displayedItemRange.max; itemIndex++ )
	{
		std::string currentLine = playerInventory[ itemIndex ]->GetInstanceName();
		g_renderer->DrawTextInBox2D( currentLine, currentLineDrawBounds, lineHeight, textColor.GetWithAlpha( alphaFraction ), 1.0f, g_renderer->CreateOrGetBitmapFont( FIXED_FONT_NAME ), TextDrawMode::TEXT_DRAW_OVERRUN, alignment );

		if ( m_inventoryMenuState == InventoryMenuSelectionState::INVENTORY_MENU_STATE_INVENTORY && m_menuSelectionIndex == itemIndex )
		{
			RenderSelectedItemBoundingBox( currentLine, currentLineDrawBounds, lineHeight, alignment, alphaFraction );
		}

		currentLineDrawBounds.maxs.y -= lineHeight;
	}

	if ( displayedItemRange.max < static_cast< int >( playerInventory.size() - 1 ) )
	{
		g_renderer->DrawTextInBox2D( "v", currentLineDrawBounds, lineHeight, textColor.GetWithAlpha( alphaFraction ), 1.0f, g_renderer->CreateOrGetBitmapFont( FIXED_FONT_NAME ), TextDrawMode::TEXT_DRAW_OVERRUN, alignment );
	}
}

void TheGame::RenderSelectedItemBoundingBox( const std::string& currentLine, const AABB2& selectedItemBounds, float lineHeight, const Vector2& alignment, float alphaFraction ) const
{
	AABB2 modifiedSelectedItemBounds = AABB2( selectedItemBounds );
	modifiedSelectedItemBounds.mins.y = selectedItemBounds.maxs.y - lineHeight;
	float lineLength = g_renderer->CreateOrGetBitmapFont( FIXED_FONT_NAME )->GetStringWidth( currentLine, lineHeight, 1.0f );
	float differenceInLength = selectedItemBounds.GetDimensions().x - lineLength; // Assumes we are compressing the box to fit around the text
	modifiedSelectedItemBounds.mins.x = selectedItemBounds.mins.x + ( alignment.x * differenceInLength );
	modifiedSelectedItemBounds.maxs.x = selectedItemBounds.maxs.x - ( ( 1.0f - alignment.x ) * differenceInLength );
	g_renderer->DrawLineBorder( modifiedSelectedItemBounds, Rgba::YELLOW.GetWithAlpha( alphaFraction ), g_gameConfigBlackboard.GetValue( "menuBorderThickness", 0.3f ) );
}

void TheGame::RenderSelectedItemDescription( const AABB2& screenBounds, const Rgba& textColor, float lineHeight, float alphaFraction ) const
{
	Item* selectedItem = nullptr;
	
	if ( m_inventoryMenuState == InventoryMenuSelectionState::INVENTORY_MENU_STATE_INVENTORY )
	{
		selectedItem = m_currentAdventure->GetCurrentMap()->GetPlayerEntity()->GetItemAtIndex( m_menuSelectionIndex );
	}
	else if ( m_inventoryMenuState == InventoryMenuSelectionState::INVENTORY_MENU_STATE_EQUIPMENT )
	{
		selectedItem = m_currentAdventure->GetCurrentMap()->GetPlayerEntity()->GetEquippedItem( EquipSlot( m_menuSelectionIndex ) );
	}

	if ( selectedItem != nullptr )
	{
		AABB2 descriptionBoxBounds = screenBounds;
		descriptionBoxBounds.mins.x = Interpolate( screenBounds.mins.x, screenBounds.maxs.x, 0.33f );
		descriptionBoxBounds.maxs.x = Interpolate( screenBounds.mins.x, screenBounds.maxs.x, 0.66f );
		descriptionBoxBounds.mins.y = Interpolate( screenBounds.mins.y, screenBounds.maxs.y, 0.25f );
		descriptionBoxBounds.maxs.y = Interpolate( screenBounds.mins.y, screenBounds.maxs.y, 0.5f );
		g_renderer->DrawTextInBox2D( selectedItem->GetItemDefinition()->GetDescription(), descriptionBoxBounds, lineHeight, textColor.GetWithAlpha( alphaFraction ), 0.5f, g_renderer->CreateOrGetBitmapFont( FIXED_FONT_NAME ), TextDrawMode::TEXT_DRAW_WORD_WRAP, Vector2( 0.5f, 0.5f ) );

		AABB2 descriptionSpriteBounds = screenBounds;
		descriptionSpriteBounds.mins = Interpolate( screenBounds.mins, screenBounds.maxs, 0.5f );
		descriptionSpriteBounds.maxs = Interpolate( screenBounds.mins, screenBounds.maxs, 0.55f );
		descriptionSpriteBounds.maxs.y = descriptionSpriteBounds.mins.y + descriptionSpriteBounds.GetDimensions().x;	// If the screen aspect is not 1, account for it
		descriptionSpriteBounds.Translate( descriptionSpriteBounds.GetDimensions() * -0.5f );
		descriptionSpriteBounds.Translate( 0.1f * Vector2::UP );
		selectedItem->RenderInHUD( descriptionSpriteBounds, alphaFraction );

		AABB2 statChangeBoxBounds = screenBounds;
		statChangeBoxBounds.maxs.x = Interpolate( screenBounds.mins.x, screenBounds.maxs.x, 0.33f );
		statChangeBoxBounds.mins.y = Interpolate( screenBounds.mins.y, screenBounds.maxs.y, 0.25f );
		statChangeBoxBounds.maxs.y = Interpolate( screenBounds.mins.y, screenBounds.maxs.y, 0.5f );
		std::string statText = "";
		for ( int statIndex = 0; statIndex < StatID::NUM_STATS; statIndex++ )
		{
			StatID statID = StatID( statIndex );
			int statModifierValue = selectedItem->GetItemDefinition()->GetStatModifier( statID );
			if ( statModifierValue != 0 )
			{
				
				statText += Entity::GetNameForStatID( statID ) +
							" : " +
							( ( statModifierValue > 0 ) ? "+" : "" ) +		// Add a "+" if the item increases a stat, since a negative value will automatically add a '-' when converted to string
							std::to_string( selectedItem->GetItemDefinition()->GetStatModifier( statID ) ) +
							"\n";
			}
		}

		g_renderer->DrawTextInBox2D( statText, statChangeBoxBounds, lineHeight, textColor.GetWithAlpha( alphaFraction ), 0.5f, g_renderer->CreateOrGetBitmapFont( FIXED_FONT_NAME ), TextDrawMode::TEXT_DRAW_WORD_WRAP, Vector2( 0.0f, 0.5f ) );
	}
}

void TheGame::RenderSelectedItemInstructions( const AABB2& screenBounds, const Rgba& textColor, float lineHeight, float alphaFraction ) const
{
	AABB2 itemInstructionsBoxBounds = screenBounds;
	itemInstructionsBoxBounds.mins.x = Interpolate( screenBounds.mins.x, screenBounds.maxs.x, 0.66f );
	itemInstructionsBoxBounds.mins.y = Interpolate( screenBounds.mins.y, screenBounds.maxs.y, 0.25f );
	itemInstructionsBoxBounds.maxs.y = Interpolate( screenBounds.mins.y, screenBounds.maxs.y, 0.5f );
	std::string itemInstructionsText = "";

	Player* player = m_currentAdventure->GetCurrentMap()->GetPlayerEntity();
	if ( m_inventoryMenuState == InventoryMenuSelectionState::INVENTORY_MENU_STATE_EQUIPMENT )
	{
		if ( player->GetEquipment()[ EquipSlot( m_menuSelectionIndex ) ] != nullptr )
		{
			itemInstructionsText += "R/Controller-Y : Unequip";
		}
	}
	else if ( m_inventoryMenuState == InventoryMenuSelectionState::INVENTORY_MENU_STATE_INVENTORY )
	{
		Item* selectedItem = player->GetItemAtIndex( m_menuSelectionIndex );
		if ( selectedItem != nullptr )
		{
			if ( selectedItem->GetItemDefinition()->GetEquipSlot() != EquipSlot::EQUIP_SLOT_NONE )
			{
				itemInstructionsText += "E/Controller-A : Equip\n";
			}
			else
			{
				itemInstructionsText += "E/Controller-A : Use\n";
			}
			itemInstructionsText += "R/Controller-Y : Drop";
		}
	}

	g_renderer->DrawTextInBox2D( itemInstructionsText, itemInstructionsBoxBounds, lineHeight, textColor.GetWithAlpha( alphaFraction ), 0.5f, g_renderer->CreateOrGetBitmapFont( FIXED_FONT_NAME ), TextDrawMode::TEXT_DRAW_WORD_WRAP, Vector2( 1.0f, 0.5f ) );
}

void TheGame::Render_Paused() const
{
	float alphaFraction = GetRenderAlphaForState( GameState::PAUSED );

	Rgba backgroundColor = Rgba::BLACK;
	Rgba textColor = Rgba::WHITE;

	AABB2 currentOrtho = g_mainCamera->GetViewportBounds();
	g_renderer->DrawAABB( currentOrtho, backgroundColor.GetWithAlpha( alphaFraction ) );
	g_renderer->DrawTextInBox2D( m_currentAdventure->GetAdventureDefinition()->GetName() + "\n\nObjective:\n" + m_currentAdventure->GetAdventureDefinition()->GetTitle(), currentOrtho, 0.5f, textColor.GetWithAlpha( alphaFraction ), 0.5f, g_renderer->CreateOrGetBitmapFont( FIXED_FONT_NAME ), TextDrawMode::TEXT_DRAW_SHRINK_TO_FIT, Vector2( 0.5f, 0.2f ) );
	g_renderer->DrawTextInBox2D( "Game Paused", currentOrtho, 1.0f, textColor.GetWithAlpha( alphaFraction ), 1.0f, g_renderer->CreateOrGetBitmapFont( FIXED_FONT_NAME ), TextDrawMode::TEXT_DRAW_SHRINK_TO_FIT, Vector2( 0.5f, 0.5f ) );
	g_renderer->DrawTextInBox2D( "Press P or Start to Resume\nPress Back to Return to Menu", currentOrtho, 0.5f, textColor.GetWithAlpha( alphaFraction ), 0.5f, g_renderer->CreateOrGetBitmapFont( FIXED_FONT_NAME ), TextDrawMode::TEXT_DRAW_SHRINK_TO_FIT, Vector2( 0.5f, 0.8f ) );
}

IntRange TheGame::ComputeIndexRangeForListPaginationBasedOnGameState() const
{
	IntRange indexRange;
	int listItemCount = 0;
	int maxVisibleListItems = 0;
	int selectedItemIndex = m_menuSelectionIndex;

	if ( m_currentGameState == GameState::ATTRACT || m_transitionToState == GameState::ATTRACT )		// This works for both these states since there will never be transitions directly between them; they always go through either the Playing or Paused states to get to each other
	{
		listItemCount = static_cast< int >( AdventureDefinition::s_definitions.size() );
		maxVisibleListItems = g_gameConfigBlackboard.GetValue( "menuAdventureViewNumLines", 5 );
	}
	else if ( m_currentGameState == GameState::INVENTORY || m_transitionToState == GameState::INVENTORY )
	{
		listItemCount = static_cast< int >( m_currentAdventure->GetCurrentMap()->GetPlayerEntity()->GetInventory().size() );
		maxVisibleListItems = g_gameConfigBlackboard.GetValue( "menuInventoryViewNumLines", 3 );
		if ( m_inventoryMenuState == InventoryMenuSelectionState::INVENTORY_MENU_STATE_EQUIPMENT )		// In this case, m_menuSelectionIndex represents the index in the Equipment list, not the Inventory list. Since  only the inventory list is paginated, we default this to zero
		{
			selectedItemIndex = 0;
		}
	}

	indexRange.min = Max( 0, ( selectedItemIndex - ( maxVisibleListItems / 2 ) ) );
	indexRange.max = Min( ( listItemCount - 1 ), ( indexRange.min + ( maxVisibleListItems - 1 ) ) );

	return indexRange;
}

void TheGame::Render_Defeat() const
{
	float alphaFraction = GetRenderAlphaForState( GameState::DEFEAT );

	Rgba backgroundColor = Rgba::RED;
	Rgba textColor = Rgba::WHITE;

	AABB2 currentOrtho = g_mainCamera->GetViewportBounds();
	g_renderer->DrawAABB( currentOrtho, backgroundColor.GetWithAlpha( alphaFraction ) );
	g_renderer->DrawTextInBox2D( "You died", currentOrtho, 1.0f, textColor.GetWithAlpha( alphaFraction ), 1.0f, g_renderer->CreateOrGetBitmapFont( FIXED_FONT_NAME ), TextDrawMode::TEXT_DRAW_SHRINK_TO_FIT, Vector2( 0.5f, 0.5f ) );
	g_renderer->DrawTextInBox2D( "Press Space or Start to Respawn\nPress Back to Quit To Menu", currentOrtho, 0.5f, textColor.GetWithAlpha( alphaFraction ), 0.5f, g_renderer->CreateOrGetBitmapFont( FIXED_FONT_NAME ), TextDrawMode::TEXT_DRAW_SHRINK_TO_FIT, Vector2( 0.5f, 0.8f ) );
}

void TheGame::Render_Victory() const
{
	float alphaFraction = GetRenderAlphaForState( GameState::VICTORY );

	Rgba backgroundColor = Rgba::BLUE;
	Rgba textColor = Rgba::WHITE;

	AABB2 currentOrtho = g_mainCamera->GetViewportBounds();
	g_renderer->DrawAABB( currentOrtho, backgroundColor.GetWithAlpha( alphaFraction ) );
	g_renderer->DrawTextInBox2D( "Victory!", currentOrtho, 1.0f, textColor.GetWithAlpha( alphaFraction ), 1.0f, g_renderer->CreateOrGetBitmapFont( FIXED_FONT_NAME ), TextDrawMode::TEXT_DRAW_SHRINK_TO_FIT, Vector2( 0.5f, 0.5f ) );
	g_renderer->DrawTextInBox2D( "Press Space or Start to Return to Menu", currentOrtho, 0.5f, textColor.GetWithAlpha( alphaFraction ), 0.5f, g_renderer->CreateOrGetBitmapFont( FIXED_FONT_NAME ), TextDrawMode::TEXT_DRAW_SHRINK_TO_FIT, Vector2( 0.5f, 0.8f ) );
}

float TheGame::GetRenderAlphaForState( GameState state ) const
{
	float alphaFraction = 0.0f;

	if ( m_currentGameState == GameState::DIALOGUE && ( state == GameState::PLAYING || state == GameState::INVENTORY ) )
	{
		return 1.0f;
	}
	else if ( m_currentGameState == state )
	{
		alphaFraction = 1.0f;

		if ( IsStateTransitioning() )
		{
			alphaFraction *= 1.0f - ( m_numSecondsIntoStateTransition / g_gameConfigBlackboard.GetValue( "stateTransitionTime", 1.0f ) );
		}
	}
	else if ( m_transitionToState == state )
	{
		alphaFraction = ( ( m_numSecondsIntoStateTransition ) / g_gameConfigBlackboard.GetValue( "stateTransitionTime", 1.0f ) );
	}

	return alphaFraction;
}

bool TheGame::IsStateTransitioning() const
{
	return ( !m_isFinishedTransitioning && m_transitionToState != GameState::NONE );
}

void TheGame::CreateTileDefinitionsFromXml()
{
	tinyxml2::XMLDocument* tileDefinitionsDocument = new tinyxml2::XMLDocument();
	tileDefinitionsDocument->LoadFile( TILE_XML_FILE_PATH );

	tinyxml2::XMLElement* tileDefinitionsElement = tileDefinitionsDocument->RootElement();
	ASSERT_OR_DIE( !tileDefinitionsElement->NoChildren(), "TheGame::CreateTileDefinitionsFromXml - <TileDefinitions> root node does not have any <TileDefinition> child nodes. Aborting..." );

	for ( const tinyxml2::XMLElement* tileDefinitionElement = tileDefinitionsElement->FirstChildElement(); tileDefinitionElement != nullptr; tileDefinitionElement = tileDefinitionElement->NextSiblingElement() )
	{
		new TileDefinition( *tileDefinitionElement );		// The destructor will deallocate this correctly
	}
}

void TheGame::CreateMapDefinitionsFromXml()
{
	tinyxml2::XMLDocument* mapDefinitionsDocument = new tinyxml2::XMLDocument();
	mapDefinitionsDocument->LoadFile( MAP_XML_FILE_PATH );

	tinyxml2::XMLElement* mapDefinitionsElement = mapDefinitionsDocument->RootElement();
	ASSERT_OR_DIE( !mapDefinitionsElement->NoChildren(), "TheGame::CreateMapDefinitionsFromXml - <MapDefinitions> root node does not have any <MapDefinition> child nodes. Aborting..." );

	for ( const tinyxml2::XMLElement* mapDefinitionElement = mapDefinitionsElement->FirstChildElement(); mapDefinitionElement != nullptr; mapDefinitionElement = mapDefinitionElement->NextSiblingElement() )
	{
		new MapDefinition( *mapDefinitionElement );		// The destructor will deallocate this correctly
	}
}

void TheGame::CreateActorDefinitionsFromXml()
{
	tinyxml2::XMLDocument* actorDefinitionsDocument = new tinyxml2::XMLDocument();
	actorDefinitionsDocument->LoadFile( ACTOR_XML_FILE_PATH );

	tinyxml2::XMLElement* actorDefinitionsElement = actorDefinitionsDocument->RootElement();
	ASSERT_OR_DIE( !actorDefinitionsElement->NoChildren(), "TheGame::CreateActorDefinitionsFromXml - <ActorDefinitions> root node does not have any <ActorDefinition> child nodes. Aborting..." );

	for ( const tinyxml2::XMLElement* actorDefinitionElement = actorDefinitionsElement->FirstChildElement(); actorDefinitionElement != nullptr; actorDefinitionElement = actorDefinitionElement->NextSiblingElement() )
	{
		new ActorDefinition( *actorDefinitionElement );
	}
}

void TheGame::CreateItemDefinitionsFromXml()
{
	tinyxml2::XMLDocument* itemDefinitionsDocument = new tinyxml2::XMLDocument();
	itemDefinitionsDocument->LoadFile( ITEM_XML_FILE_PATH );

	tinyxml2::XMLElement* itemDefinitionsElement = itemDefinitionsDocument->RootElement();
	ASSERT_OR_DIE( !itemDefinitionsElement->NoChildren(), "TheGame::CreateItemDefinitionsFromXml - <ItemDefinitions> root node does not have any <ItemDefinition> child nodes. Aborting..." );

	for ( const tinyxml2::XMLElement* itemDefinitionElement = itemDefinitionsElement->FirstChildElement(); itemDefinitionElement != nullptr; itemDefinitionElement = itemDefinitionElement->NextSiblingElement() )
	{
		new ItemDefinition( *itemDefinitionElement );
	}
}

void TheGame::CreateLootDefinitionsFromXml()
{
	tinyxml2::XMLDocument* lootDefinitionsDocument = new tinyxml2::XMLDocument();
	lootDefinitionsDocument->LoadFile( LOOT_XML_FILE_PATH );

	tinyxml2::XMLElement* lootDefinitionsElement = lootDefinitionsDocument->RootElement();
	ASSERT_OR_DIE( !lootDefinitionsElement->NoChildren(), "TheGame::CreateLootDefinitionsFromXml - <LootDefinitions> root node does not have any <LootDefinition> child nodes. Aborting..." );

	for ( const tinyxml2::XMLElement* lootDefinitionElement = lootDefinitionsElement->FirstChildElement(); lootDefinitionElement != nullptr; lootDefinitionElement = lootDefinitionElement->NextSiblingElement() )
	{
		new LootDefinition( *lootDefinitionElement );
	}
}

void TheGame::CreateProjectileDefinitionsFromXml()
{
	tinyxml2::XMLDocument* projectileDefinitionsDocument = new tinyxml2::XMLDocument();
	projectileDefinitionsDocument->LoadFile( PROJECTILE_XML_FILE_PATH );

	tinyxml2::XMLElement* projectileDefinitionsElement = projectileDefinitionsDocument->RootElement();
	ASSERT_OR_DIE( !projectileDefinitionsElement->NoChildren(), "TheGame::CreateProjectileDefinitionsFromXml - <ProjectileDefinitions> root node does not have any <ProjectileDefinition> child nodes. Aborting..." );

	for ( const tinyxml2::XMLElement* projectileDefinitionElement = projectileDefinitionsElement->FirstChildElement(); projectileDefinitionElement != nullptr; projectileDefinitionElement = projectileDefinitionElement->NextSiblingElement() )
	{
		new ProjectileDefinition( *projectileDefinitionElement );
	}
}

void TheGame::CreatePortalDefinitionsFromXml()
{
	tinyxml2::XMLDocument* portalDefinitionsDocument = new tinyxml2::XMLDocument();
	portalDefinitionsDocument->LoadFile( PORTAL_XML_FILE_PATH );

	tinyxml2::XMLElement* portalDefinitionsElement = portalDefinitionsDocument->RootElement();
	ASSERT_OR_DIE( !portalDefinitionsElement->NoChildren(), "TheGame::CreatePortalDefinitionsFromXml - <PortalDefinitions> root node does not have any <PortalDefinition> child nodes. Aborting..." );

	for ( const tinyxml2::XMLElement* portalDefinition = portalDefinitionsElement->FirstChildElement(); portalDefinition != nullptr; portalDefinition = portalDefinition->NextSiblingElement() )
	{
		new PortalDefinition( *portalDefinition );
	}
}

void TheGame::CreateAdventureDefinitionsFromXml()
{
	tinyxml2::XMLDocument* adventureDefinitionsDocument = new tinyxml2::XMLDocument();
	adventureDefinitionsDocument->LoadFile( ADVENTURE_XML_FILE_PATH );

	tinyxml2::XMLElement* adventureDefinitionsElement = adventureDefinitionsDocument->RootElement();
	ASSERT_OR_DIE( !adventureDefinitionsElement->NoChildren(), "TheGame::CreateAdventureDefinitionsFromXml - <AdventureDefinitions> root node does not have any <AdventureDefinition> child nodes. Aborting..." );

	for ( const tinyxml2::XMLElement* adventureDefinitionElement = adventureDefinitionsElement->FirstChildElement(); adventureDefinitionElement != nullptr; adventureDefinitionElement = adventureDefinitionElement->NextSiblingElement() )
	{
		new AdventureDefinition( *adventureDefinitionElement );		// The destructor will deallocate this correctly
	}
}

void TheGame::InitializeTileSpriteSheet()
{
	Texture* tileTexture = g_renderer->CreateOrGetTexture( TILE_SPRITE_SHEET_FILE_PATH );
	g_tileSpriteSheet = new SpriteSheet( *tileTexture, TILE_SPRITE_SHEET_NUM_TILES_WIDE, TILE_SPRITE_SHEET_NUM_TILES_HIGH );
}

void TheGame::RenderNameInBitmapFont() const
{
	BitmapFont* bitmapFont = g_renderer->CreateOrGetBitmapFont( FIXED_FONT_NAME );
	g_renderer->DrawText2D( Vector2::ZERO, "Abhishek Arora", TILE_SIDE_LENGTH_WORLD_UNITS, Rgba::WHITE, 1.0f, bitmapFont );
}

void TheGame::DeleteAdventureDefinitions()
{
	for ( std::map<std::string, AdventureDefinition*>::iterator adventureDefIterator = AdventureDefinition::s_definitions.begin(); adventureDefIterator != AdventureDefinition::s_definitions.end(); adventureDefIterator++ )
	{
		delete adventureDefIterator->second;
		adventureDefIterator->second = nullptr;
	}
}

void TheGame::DeleteTileDefinitions()
{
	for ( std::map<std::string, TileDefinition*>::iterator tileDefIterator = TileDefinition::s_definitions.begin(); tileDefIterator != TileDefinition::s_definitions.end(); tileDefIterator++ )
	{
		delete tileDefIterator->second;
		tileDefIterator->second = nullptr;
	}
}

void TheGame::DeleteLootDefinitions()
{
	for ( std::map<std::string, LootDefinition*>::iterator lootDefIterator = LootDefinition::s_definitions.begin(); lootDefIterator != LootDefinition::s_definitions.end(); lootDefIterator++ )
	{
		delete lootDefIterator->second;
		lootDefIterator->second = nullptr;
	}
}

void TheGame::DeleteMapDefinitions()
{
	for ( std::map<std::string, MapDefinition*>::iterator mapDefIterator = MapDefinition::s_definitions.begin(); mapDefIterator != MapDefinition::s_definitions.end(); mapDefIterator++ )
	{
		delete mapDefIterator->second;
		mapDefIterator->second = nullptr;
	}
}

void TheGame::DeleteEntityDefinitions()
{
	for ( std::map<std::string, ActorDefinition*>::iterator actorDefIterator = ActorDefinition::s_definitions.begin(); actorDefIterator != ActorDefinition::s_definitions.end(); actorDefIterator++ )
	{
		delete actorDefIterator->second;
		actorDefIterator->second = nullptr;
	}
	for ( std::map<std::string, ItemDefinition*>::iterator itemDefIterator = ItemDefinition::s_definitions.begin(); itemDefIterator != ItemDefinition::s_definitions.end(); itemDefIterator++ )
	{
		delete itemDefIterator->second;
		itemDefIterator->second = nullptr;
	}
	for ( std::map<std::string, ProjectileDefinition*>::iterator projectileDefIterator = ProjectileDefinition::s_definitions.begin(); projectileDefIterator != ProjectileDefinition::s_definitions.end(); projectileDefIterator++ )
	{
		delete projectileDefIterator->second;
		projectileDefIterator->second = nullptr;
	}
	for ( std::map<std::string, PortalDefinition*>::iterator portalDefIterator = PortalDefinition::s_definitions.begin(); portalDefIterator != PortalDefinition::s_definitions.end(); portalDefIterator++ )
	{
		delete portalDefIterator->second;
		portalDefIterator->second = nullptr;
	}
}

void TheGame::HandleDebugKeyboardInput( float& out_deltaSeconds ) const
{
	if ( g_inputSystem->IsKeyDown( InputSystem::KEYBOARD_T ) )
	{
		out_deltaSeconds *= g_gameConfigBlackboard.GetValue( "debugSlowMultiplier", 0.5f );
	}
	else if ( g_inputSystem->IsKeyDown( InputSystem::KEYBOARD_Y ) )
	{
		out_deltaSeconds *= g_gameConfigBlackboard.GetValue( "debugFastMultiplier", 2.0f );
	}
}

void TheGame::HandleStateTransitionKeyboardInput()
{
	if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_F1 ) )
	{
		m_developerModeEnabled = !m_developerModeEnabled;
	}

	if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_P ) )
	{
		if ( m_currentGameState == GameState::PLAYING )
		{
			m_transitionToState = GameState::PAUSED;
		}
		else if ( m_currentGameState == GameState::PAUSED )
		{
			m_transitionToState = GameState::PLAYING;
		}
	}
	if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_I ) )
	{
		if ( m_currentGameState == GameState::PLAYING )
		{
			m_transitionToState = GameState::INVENTORY;
			m_menuSelectionIndex = 0;
		}
		else if ( m_currentGameState == GameState::INVENTORY )
		{
			m_transitionToState = GameState::PLAYING;
		}
	}
	if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_SPACE ) )
	{
		if ( m_currentGameState == GameState::VICTORY )
		{
			m_transitionToState = GameState::ATTRACT;
		} 
		else if ( m_currentGameState == GameState::DEFEAT )
		{
			m_transitionToState = GameState::PLAYING;
			m_currentAdventure->GetCurrentMap()->GetPlayerEntity()->Respawn();
		} 
	}
	if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_ESCAPE ) )
	{
		if ( m_currentGameState == GameState::PLAYING )
		{
			m_transitionToState = GameState::PAUSED;
		}
		else if ( m_currentGameState == GameState::PAUSED || m_currentGameState == GameState::DEFEAT || m_currentGameState == GameState::VICTORY )
		{
			m_transitionToState = GameState::ATTRACT;
		}
	}
}

void TheGame::HandleStateTransitionXboxControllerInput()
{
	if ( g_inputSystem->GetController( 0 ).IsConnected() )
	{
		if ( g_inputSystem->GetController( 0 ).WasKeyJustPressed( InputXboxControllerMappings::GAMEPAD_START ) )
		{
			if ( m_currentGameState == GameState::PAUSED || m_currentGameState == GameState::INVENTORY  )
			{
				m_transitionToState = GameState::PLAYING;
			}
			else if ( m_currentGameState == GameState::DEFEAT )
			{
				m_transitionToState = GameState::PLAYING;
				m_currentAdventure->GetCurrentMap()->GetPlayerEntity()->Respawn();
			} 
			else if ( m_currentGameState == GameState::PLAYING )
			{
				m_transitionToState = GameState::INVENTORY;
				m_menuSelectionIndex = 0;
			}
			else if ( m_currentGameState == GameState::VICTORY )
			{
				m_transitionToState = GameState::ATTRACT;
			} 
		}
		if ( g_inputSystem->GetController( 0 ).WasKeyJustPressed( InputXboxControllerMappings::GAMEPAD_SELECT ) )
		{
			if ( m_currentGameState == GameState::PLAYING )
			{
				m_transitionToState = GameState::PAUSED;
			}
			else if ( m_currentGameState == GameState::PAUSED || m_currentGameState == GameState::DEFEAT || m_currentGameState == GameState::VICTORY )
			{
				m_transitionToState = GameState::ATTRACT;
			}
		}
	}
}

void TheGame::HandleAttractScreenKeyboardInput()
{
	std::vector< AdventureDefinition* > adventureList = AdventureDefinition::GetDefinitionsAsVector();

	if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_W ) || g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_UP_ARROW ) )
	{
		if ( m_menuSelectionIndex > 0 )
		{
			m_menuSelectionIndex--;
		}
	}
	else if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_S ) || g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_DOWN_ARROW ) )
	{
		if ( m_menuSelectionIndex < static_cast< int >( adventureList.size() - 1 ) )
		{
			m_menuSelectionIndex++;
		}
	}

	if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_SPACE ) )
	{
		m_currentAdventure = new Adventure( adventureList[ m_menuSelectionIndex ] );
		m_transitionToState = GameState::PLAYING;
	}
}

void TheGame::HandleAttractScreenXboxControllerInput()
{
	if ( g_inputSystem->GetController( 0 ).IsConnected() )
	{
		std::vector< AdventureDefinition* > adventureList = AdventureDefinition::GetDefinitionsAsVector();

		if ( g_inputSystem->GetController( 0 ).WasKeyJustPressed( InputXboxControllerMappings::DPAD_UP ) )
		{
			if ( m_menuSelectionIndex > 0 )
			{
				m_menuSelectionIndex--;
			}
		}
		else if ( g_inputSystem->GetController( 0 ).WasKeyJustPressed( InputXboxControllerMappings::DPAD_DOWN ) )
		{
			if ( m_menuSelectionIndex < static_cast< int >( adventureList.size() - 1 ) )
			{
				m_menuSelectionIndex++;
			}
		}

		if ( g_inputSystem->GetController( 0 ).WasKeyJustPressed( InputXboxControllerMappings::GAMEPAD_A ) )
		{
			m_currentAdventure = new Adventure( adventureList[ m_menuSelectionIndex ] );
			m_transitionToState = GameState::PLAYING;
		}
	}
}

void TheGame::HandleInventoryScreenKeyboardInput()
{
	Player* player = m_currentAdventure->GetCurrentMap()->GetPlayerEntity();

	if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_A )  || g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_LEFT_ARROW )  )
	{
		m_inventoryMenuState = InventoryMenuSelectionState::INVENTORY_MENU_STATE_EQUIPMENT;
		if ( m_menuSelectionIndex > static_cast< int >( EquipSlot::NUM_SLOTS - 1 ) )
		{
			m_menuSelectionIndex = static_cast< int >( EquipSlot::NUM_SLOTS - 1 );
		}
	}
	else if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_D ) || g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_RIGHT_ARROW )  )
	{
		if ( player->GetInventory().size() > 0 )
		{
			m_inventoryMenuState = InventoryMenuSelectionState::INVENTORY_MENU_STATE_INVENTORY;
			if ( m_menuSelectionIndex > static_cast< int >( player->GetInventory().size() - 1 ) )
			{
				m_menuSelectionIndex = static_cast< int >( player->GetInventory().size() - 1 );
			}
		}
	}
	else if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_W ) || g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_UP_ARROW )  )
	{
		if ( m_menuSelectionIndex > 0 )
		{
			m_menuSelectionIndex--;
		}
	}
	else if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_S ) || g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_DOWN_ARROW )  )
	{
		if ( m_inventoryMenuState == InventoryMenuSelectionState::INVENTORY_MENU_STATE_INVENTORY )
		{
			if ( m_menuSelectionIndex < static_cast< int >( player->GetInventory().size() - 1 ) )
			{
				m_menuSelectionIndex++;
			}
		}
		else if ( m_inventoryMenuState == InventoryMenuSelectionState::INVENTORY_MENU_STATE_EQUIPMENT  )
		{
			if ( m_menuSelectionIndex < static_cast< int >( EquipSlot::NUM_SLOTS - 1 ) )
			{
				m_menuSelectionIndex++;
			}
		}
	}

	if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_E ) )
	{
		if ( m_inventoryMenuState  == InventoryMenuSelectionState::INVENTORY_MENU_STATE_INVENTORY )
		{
			if ( player->GetInventory()[ m_menuSelectionIndex ]->GetItemDefinition()->GetEquipSlot() != EquipSlot::EQUIP_SLOT_NONE )
			{
				player->EquipItem( player->GetInventory()[ m_menuSelectionIndex ] );
			}
			else
			{
				player->UseItem( player->GetInventory()[ m_menuSelectionIndex ] );
			}
		}
	}

	if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_R ) )
	{
		if ( m_inventoryMenuState  == InventoryMenuSelectionState::INVENTORY_MENU_STATE_EQUIPMENT )
		{
			if ( player->GetEquipment()[ EquipSlot( m_menuSelectionIndex ) ] != nullptr )
			{
				player->UnequipItem( player->GetEquipment()[ EquipSlot( m_menuSelectionIndex ) ] );
			}
		}
		else if ( m_inventoryMenuState  == InventoryMenuSelectionState::INVENTORY_MENU_STATE_INVENTORY )
		{
			player->DropItem( player->GetInventory()[ m_menuSelectionIndex ] );
		}
	}
}

void TheGame::HandleInventoryScreenXboxControllerInput()
{
	if ( g_inputSystem->GetController( 0 ).IsConnected() )
	{
		Player* player = m_currentAdventure->GetCurrentMap()->GetPlayerEntity();
		
		if ( g_inputSystem->GetController( 0 ).WasKeyJustPressed( InputXboxControllerMappings::DPAD_LEFT ) )
		{
			m_inventoryMenuState = InventoryMenuSelectionState::INVENTORY_MENU_STATE_EQUIPMENT;
			if ( m_menuSelectionIndex > static_cast< int >( EquipSlot::NUM_SLOTS - 1 ) )
			{
				m_menuSelectionIndex = static_cast< int >( EquipSlot::NUM_SLOTS - 1 );
			}
		}
		else if ( g_inputSystem->GetController( 0 ).WasKeyJustPressed( InputXboxControllerMappings::DPAD_RIGHT ) )
		{
			if ( player->GetInventory().size() > 0 )
			{
				m_inventoryMenuState = InventoryMenuSelectionState::INVENTORY_MENU_STATE_INVENTORY;
				if ( m_menuSelectionIndex > static_cast< int >( player->GetInventory().size() - 1 ) )
				{
					m_menuSelectionIndex = static_cast< int >( player->GetInventory().size() - 1 );
				}
			}
		}
		else if ( g_inputSystem->GetController( 0 ).WasKeyJustPressed( InputXboxControllerMappings::DPAD_UP ) )
		{
			if ( m_menuSelectionIndex > 0 )
			{
					m_menuSelectionIndex--;
			}
		}
		else if ( g_inputSystem->GetController( 0 ).WasKeyJustPressed( InputXboxControllerMappings::DPAD_DOWN ) )
		{
			if ( m_inventoryMenuState == InventoryMenuSelectionState::INVENTORY_MENU_STATE_INVENTORY )
			{
				if ( m_menuSelectionIndex < static_cast< int >( player->GetInventory().size() - 1 ) )
				{
					m_menuSelectionIndex++;
				}
			}
			else if ( m_inventoryMenuState == InventoryMenuSelectionState::INVENTORY_MENU_STATE_EQUIPMENT )
			{
				if ( m_menuSelectionIndex < static_cast< int >( EquipSlot::NUM_SLOTS - 1 ) )
				{
					m_menuSelectionIndex++;
				}
			}
		}

		if ( g_inputSystem->GetController( 0 ).WasKeyJustPressed( InputXboxControllerMappings::GAMEPAD_A ) )
		{
			if ( m_inventoryMenuState  == InventoryMenuSelectionState::INVENTORY_MENU_STATE_INVENTORY )
			{
				if ( player->GetInventory()[ m_menuSelectionIndex ]->GetItemDefinition()->GetEquipSlot() != EquipSlot::EQUIP_SLOT_NONE )
				{
					player->EquipItem( player->GetInventory()[ m_menuSelectionIndex ] );
				}
				else
				{
					player->UseItem( player->GetInventory()[ m_menuSelectionIndex ] );
				}
			}
		}

		if ( g_inputSystem->GetController( 0 ).WasKeyJustPressed( InputXboxControllerMappings::GAMEPAD_Y ) )
		{
			if ( m_inventoryMenuState  == InventoryMenuSelectionState::INVENTORY_MENU_STATE_EQUIPMENT )
			{
				if ( player->GetEquipment()[ EquipSlot( m_menuSelectionIndex ) ] != nullptr )
				{
					player->UnequipItem( player->GetEquipment()[ EquipSlot( m_menuSelectionIndex ) ] );
				}
			}
			else if ( m_inventoryMenuState  == InventoryMenuSelectionState::INVENTORY_MENU_STATE_INVENTORY )
			{
				player->DropItem( player->GetInventory()[ m_menuSelectionIndex ] );
			}
		}
	}
}
