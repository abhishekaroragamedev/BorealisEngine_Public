#pragma once

#include "Game/Dialogue.hpp"
#include "Game/Entities/EntityDefinition.hpp"
#include "Game/Entities/ProjectileDefinition.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/IntRange.hpp"
#include "Engine/Math/Vector2.hpp"
#include "ThirdParty/tinyXML2/tinyxml2.h"
#include <map>
#include <string>

enum EquipSlot
{
	EQUIP_SLOT_NONE = -1,
	EQUIP_SLOT_HEAD,
	EQUIP_SLOT_CHEST,
	EQUIP_SLOT_FEET,
	EQUIP_SLOT_WEAPON,
	EQUIP_SLOT_SPELL,
	NUM_SLOTS
};

enum WeaponType
{
	WEAPON_TYPE_NONE = -1,
	WEAPON_TYPE_MELEE,
	WEAPON_TYPE_PROJECTILE,
	NUM_WEAPON_TYPES
};

enum ItemDialogueType
{
	ITEM_DIALOGUE_TYPES_NONE = -1,
	ITEM_DIALOGUE_TYPE_TAKE,
	ITEM_DIALOGUE_TYPE_DROP,
	ITEM_DIALOGUE_TYPE_EQUIP,
	ITEM_DIALOGUE_TYPE_UNEQUIP,
	ITEM_DIALOGUE_TYPE_USE,
	NUM_ITEM_DIALOGUE_TYPES
};

class ItemDefinition: public EntityDefinition
{

	friend class TheGame;

public:
	~ItemDefinition();

private:
	explicit ItemDefinition( const tinyxml2::XMLElement& itemDefinitionElement );

public:
	int GetStatModifier( StatID statID ) const;
	const int* GetStatModifiers() const;
	EquipSlot GetEquipSlot() const;
	Tags GetActorNeedsTags() const;
	Tags GetUsageTags() const;
	ProjectileDefinition* GetProjectileSpawned() const;
	float GetMeleeRange() const;
	WeaponType GetWeaponType() const;
	std::string GetDescription() const;
	bool CanBeDropped() const;
	Dialogue* GetDialogue( ItemDialogueType dialogueType ) const;

private:
	void PopulateDataFromXml( const tinyxml2::XMLElement& itemDefinitionElement ) override;
	void PopulateStatsFromXml( const tinyxml2::XMLElement& statsElement );
	void PopulateDialoguesFromXml( const tinyxml2::XMLElement& dialogElement );

public:
	static std::map< std::string, ItemDefinition* > s_definitions;
	static EquipSlot GetEquipSlotFromEquipSlotName( const std::string& equipSlotName );
	static std::string GetEquipSlotNameFromEquipSlot( EquipSlot equipSlot );

private:
	static constexpr char STATS_XML_NODE_NAME[] = "Stats";
	static constexpr char DESCRIPTION_XML_NODE_NAME[] = "Description";
	static constexpr char ATTACK_XML_NODE_NAME[] = "Attack";
	static constexpr char DIALOGUE_XML_NODE_NAME[] = "Dialogue";
	static constexpr char DIALOGUE_TAKE_XML_NODE_NAME[] = "Take";
	static constexpr char DIALOGUE_DROP_XML_NODE_NAME[] = "Drop";
	static constexpr char DIALOGUE_EQUIP_XML_NODE_NAME[] = "Equip";
	static constexpr char DIALOGUE_UNEQUIP_XML_NODE_NAME[] = "Unequip";
	static constexpr char DIALOGUE_USE_XML_NODE_NAME[] = "Use";
	static constexpr char SLOT_XML_ATTRIBUTE_NAME[] = "slot";
	static constexpr char CAN_BE_DROPPED_XML_ATTRIBUTE_NAME[] = "canBeDropped";
	static constexpr char NEEDS_TAGS_ATTRIBUTE_NAME[] = "needsTags";
	static constexpr char USAGE_TAGS_ATTRIBUTE_NAME[] = "usageTags";
	static constexpr char PROJECTILE_XML_ATTRIBUTE_NAME[] = "projectile";
	static constexpr char MELEE_RANGE_XML_ATTRIBUTE_NAME[] = "meleeRange";
	static constexpr char HEALTH_XML_ATTRIBUTE_NAME[] = "health";
	static constexpr char STRENGTH_XML_ATTRIBUTE_NAME[] = "strength";
	static constexpr char RESILIENCE_XML_ATTRIBUTE_NAME[] = "resilience";
	static constexpr char AGILITY_XML_ATTRIBUTE_NAME[] = "agility";
	static constexpr char DEXTERITY_XML_ATTRIBUTE_NAME[] = "dexterity";
	static constexpr char AIM_XML_ATTRIBUTE_NAME[] = "aim";
	static constexpr char ACCURACY_XML_ATTRIBUTE_NAME[] = "accuracy";
	static constexpr char EVASIVENESS_XML_ATTRIBUTE_NAME[] = "evasiveness";
	static constexpr char POISON_USAGE_XML_ATTRIBUTE_NAME[] = "poison";
	static constexpr char FIRE_USAGE_XML_ATTRIBUTE_NAME[] = "fire";
	static constexpr char ICE_USAGE_XML_ATTRIBUTE_NAME[] = "ice";
	static constexpr char ELECTRICITY_USAGE_XML_ATTRIBUTE_NAME[] = "electricity";

private:
	int m_statModifiers[ StatID::NUM_STATS ];
	EquipSlot m_equipSlot = EquipSlot::EQUIP_SLOT_NONE;
	Tags m_actorNeedsTags;
	Tags m_usageTags;
	ProjectileDefinition* m_projectileSpawned = nullptr;
	float m_meleeRange = 0.0f;
	bool m_canBeDropped = true;		// Items are droppable unless specified in XML
	WeaponType m_weaponType = WeaponType::WEAPON_TYPE_NONE;
	std::string m_description = "";
	Dialogue* m_itemDialogues[ ItemDialogueType::NUM_ITEM_DIALOGUE_TYPES ];

};

ItemDefinition* ParseXmlAttribute( const tinyxml2::XMLElement& element, const char* attributeName, ItemDefinition* defaultValue );
