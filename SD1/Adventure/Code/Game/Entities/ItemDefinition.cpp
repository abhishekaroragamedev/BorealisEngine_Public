#include "Game/Entities/ItemDefinition.hpp"
#include "Engine/Core/XmlUtilities.hpp"

std::map< std::string, ItemDefinition* > ItemDefinition::s_definitions;

ItemDefinition::ItemDefinition( const tinyxml2::XMLElement& itemDefinitionElement ) : EntityDefinition( itemDefinitionElement )
{
	PopulateDataFromXml( itemDefinitionElement );
}

ItemDefinition::~ItemDefinition()
{

}

void ItemDefinition::PopulateDataFromXml( const tinyxml2::XMLElement& itemDefinitionElement )
{
	EntityDefinition::PopulateDataFromXml( itemDefinitionElement );
	m_equipSlot = ItemDefinition::GetEquipSlotFromEquipSlotName( ParseXmlAttribute( itemDefinitionElement, SLOT_XML_ATTRIBUTE_NAME, ItemDefinition::GetEquipSlotNameFromEquipSlot( m_equipSlot ) ) );
	m_actorNeedsTags = ParseXmlAttribute( itemDefinitionElement, NEEDS_TAGS_ATTRIBUTE_NAME, m_actorNeedsTags );
	m_usageTags = ParseXmlAttribute( itemDefinitionElement, USAGE_TAGS_ATTRIBUTE_NAME, m_usageTags );
	m_canBeDropped = ParseXmlAttribute( itemDefinitionElement, CAN_BE_DROPPED_XML_ATTRIBUTE_NAME, m_canBeDropped );

	if ( !itemDefinitionElement.NoChildren() )
	{
		for ( const tinyxml2::XMLElement* itemDefSubElement = itemDefinitionElement.FirstChildElement(); itemDefSubElement != nullptr; itemDefSubElement = itemDefSubElement->NextSiblingElement() )
		{
			if ( std::string( itemDefSubElement->Name() ) == std::string( STATS_XML_NODE_NAME ) )
			{
				PopulateStatsFromXml( *itemDefSubElement );
			}
			else if ( std::string( itemDefSubElement->Name() ) == std::string( ATTACK_XML_NODE_NAME ) )
			{
				m_projectileSpawned = ParseXmlAttribute( *itemDefSubElement, PROJECTILE_XML_ATTRIBUTE_NAME, m_projectileSpawned );
				m_meleeRange = ParseXmlAttribute( *itemDefSubElement, MELEE_RANGE_XML_ATTRIBUTE_NAME, m_meleeRange );
				
				if ( m_meleeRange > 0.0f )
				{
					m_weaponType = WeaponType::WEAPON_TYPE_MELEE;
				}
				else if ( m_projectileSpawned != nullptr )
				{
					m_weaponType = WeaponType::WEAPON_TYPE_PROJECTILE;
				}
			}
			else if ( std::string( itemDefSubElement->Name() ) == std::string( DESCRIPTION_XML_NODE_NAME ) )
			{
				m_description = std::string( itemDefSubElement->GetText() );
			}
			else if ( std::string( itemDefSubElement->Name() ) == std::string( DIALOGUE_XML_NODE_NAME ) )
			{
				PopulateDialoguesFromXml( *itemDefSubElement );
			}
		}
	}

	s_definitions[ m_name ] = this;
}

void ItemDefinition::PopulateStatsFromXml( const tinyxml2::XMLElement& statsElement )
{
	m_statModifiers[ StatID::STAT_HEALTH ] = ParseXmlAttribute( statsElement, HEALTH_XML_ATTRIBUTE_NAME, 0 );
	m_statModifiers[ StatID::STAT_STRENGTH ] = ParseXmlAttribute( statsElement, STRENGTH_XML_ATTRIBUTE_NAME, 0 );
	m_statModifiers[ StatID::STAT_RESILIENCE ] = ParseXmlAttribute( statsElement, RESILIENCE_XML_ATTRIBUTE_NAME, 0 );
	m_statModifiers[ StatID::STAT_AGILITY ] = ParseXmlAttribute( statsElement, AGILITY_XML_ATTRIBUTE_NAME, 0 );
	m_statModifiers[ StatID::STAT_DEXTERITY ] = ParseXmlAttribute( statsElement, DEXTERITY_XML_ATTRIBUTE_NAME, 0 );
	m_statModifiers[ StatID::STAT_AIM ] =  ParseXmlAttribute( statsElement, AIM_XML_ATTRIBUTE_NAME, 0 );
	m_statModifiers[ StatID::STAT_ACCURACY ] = ParseXmlAttribute( statsElement, ACCURACY_XML_ATTRIBUTE_NAME, 0 );
	m_statModifiers[ StatID::STAT_EVASIVENESS ] = ParseXmlAttribute( statsElement, EVASIVENESS_XML_ATTRIBUTE_NAME, 0 );
	m_statModifiers[ StatID::STAT_POISON_USAGE ] = ParseXmlAttribute( statsElement, POISON_USAGE_XML_ATTRIBUTE_NAME, 0 );
	m_statModifiers[ StatID::STAT_FIRE_USAGE ] = ParseXmlAttribute( statsElement, FIRE_USAGE_XML_ATTRIBUTE_NAME, 0 );
	m_statModifiers[ StatID::STAT_ICE_USAGE ] = ParseXmlAttribute( statsElement, ICE_USAGE_XML_ATTRIBUTE_NAME, 0 );
	m_statModifiers[ StatID::STAT_ELECTRICITY_USAGE ] = ParseXmlAttribute( statsElement, ELECTRICITY_USAGE_XML_ATTRIBUTE_NAME, 0 );
}

void ItemDefinition::PopulateDialoguesFromXml( const tinyxml2::XMLElement& dialogueElement )
{
	for ( const tinyxml2::XMLElement* dialogueSubElement = dialogueElement.FirstChildElement(); dialogueSubElement != nullptr; dialogueSubElement = dialogueSubElement->NextSiblingElement() )
	{
		if ( std::string( dialogueSubElement->Name() ) == std::string( DIALOGUE_TAKE_XML_NODE_NAME ) )
		{
			m_itemDialogues[ ItemDialogueType::ITEM_DIALOGUE_TYPE_TAKE ] = ParseXmlAttribute( *dialogueSubElement, nullptr );
		}
		else if ( std::string( dialogueSubElement->Name() ) == std::string( DIALOGUE_DROP_XML_NODE_NAME ) )
		{
			m_itemDialogues[ ItemDialogueType::ITEM_DIALOGUE_TYPE_DROP ] = ParseXmlAttribute( *dialogueSubElement, nullptr );
		}
		else if ( std::string( dialogueSubElement->Name() ) == std::string( DIALOGUE_EQUIP_XML_NODE_NAME ) )
		{
			m_itemDialogues[ ItemDialogueType::ITEM_DIALOGUE_TYPE_EQUIP ] = ParseXmlAttribute( *dialogueSubElement, nullptr );
		}
		else if ( std::string( dialogueSubElement->Name() ) == std::string( DIALOGUE_UNEQUIP_XML_NODE_NAME ) )
		{
			m_itemDialogues[ ItemDialogueType::ITEM_DIALOGUE_TYPE_UNEQUIP ] = ParseXmlAttribute( *dialogueSubElement, nullptr );
		}
		else if ( std::string( dialogueSubElement->Name() ) == std::string( DIALOGUE_USE_XML_NODE_NAME ) )
		{
			m_itemDialogues[ ItemDialogueType::ITEM_DIALOGUE_TYPE_USE ] = ParseXmlAttribute( *dialogueSubElement, nullptr );
		}
	}
}

int ItemDefinition::GetStatModifier( StatID statID ) const
{
	return m_statModifiers[ statID ];
}

const int* ItemDefinition::GetStatModifiers() const
{
	return m_statModifiers;
}

EquipSlot ItemDefinition::GetEquipSlot() const
{
	return m_equipSlot;
}

Tags ItemDefinition::GetActorNeedsTags() const
{
	return m_actorNeedsTags;
}

Tags ItemDefinition::GetUsageTags() const
{
	return m_usageTags;
}

ProjectileDefinition* ItemDefinition::GetProjectileSpawned() const
{
	return m_projectileSpawned;
}

float ItemDefinition::GetMeleeRange() const
{
	return m_meleeRange;
}

WeaponType ItemDefinition::GetWeaponType() const
{
	return m_weaponType;
}

std::string ItemDefinition::GetDescription() const
{
	return m_description;
}

bool ItemDefinition::CanBeDropped() const
{
	return m_canBeDropped;
}

Dialogue* ItemDefinition::GetDialogue( ItemDialogueType dialogueType ) const
{
	return m_itemDialogues[ dialogueType ];
}

EquipSlot ItemDefinition::GetEquipSlotFromEquipSlotName( const std::string& equipSlotName )
{
	if ( equipSlotName == "head" )
	{
		return EquipSlot::EQUIP_SLOT_HEAD;
	}
	else if ( equipSlotName == "chest" )
	{
		return EquipSlot::EQUIP_SLOT_CHEST;
	}
	else if ( equipSlotName == "feet" )
	{
		return EquipSlot::EQUIP_SLOT_FEET;
	}
	else if ( equipSlotName == "weapon" )
	{
		return EquipSlot::EQUIP_SLOT_WEAPON;
	}
	else if ( equipSlotName == "spell" )
	{
		return EquipSlot::EQUIP_SLOT_SPELL;
	}
	else
	{
		return EquipSlot::EQUIP_SLOT_NONE;
	}
}

std::string ItemDefinition::GetEquipSlotNameFromEquipSlot( EquipSlot equipSlot )
{
	switch ( equipSlot )
	{
		case EquipSlot::EQUIP_SLOT_HEAD		:	return "head";
		case EquipSlot::EQUIP_SLOT_CHEST	:	return "chest";
		case EquipSlot::EQUIP_SLOT_FEET		:	return "feet";
		case EquipSlot::EQUIP_SLOT_WEAPON	:	return "weapon";
		case EquipSlot::EQUIP_SLOT_SPELL	:	return "spell";
		default								:	return "";
	}
}

ItemDefinition* ParseXmlAttribute( const tinyxml2::XMLElement& element, const char* attributeName, ItemDefinition* defaultValue )
{
	const char* itemDefinitionText = element.Attribute( attributeName );
	if ( itemDefinitionText == nullptr )
	{
		return defaultValue;
	}

	std::string itemDefinitionKey = std::string( itemDefinitionText );
	if ( ItemDefinition::s_definitions.find( itemDefinitionKey ) == ItemDefinition::s_definitions.end() )
	{
		return defaultValue;
	}

	return ItemDefinition::s_definitions[ itemDefinitionKey ];
}

