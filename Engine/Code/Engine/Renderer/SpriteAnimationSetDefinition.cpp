#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/XmlUtilities.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/SpriteAnimationSetDefinition.hpp"

SpriteAnimationSetDefinition::SpriteAnimationSetDefinition( const tinyxml2::XMLElement& spriteAnimSetElement, Renderer& renderer, const std::string& defaultAnimName /* = "Idle" */ )
	:	m_defaultAnimName( defaultAnimName )
{
	m_fps = ParseXmlAttribute( spriteAnimSetElement, SPRITE_FPS_XML_ATTRIBUTE_NAME, m_fps );
	PopulateSpriteAnimations( spriteAnimSetElement, renderer );
}

SpriteAnimationSetDefinition::~SpriteAnimationSetDefinition()
{
	for ( std::map< std::string, SpriteAnimationDefinition* >::iterator mapIterator = m_spriteAnimationDefinitionsByName.begin(); mapIterator != m_spriteAnimationDefinitionsByName.end(); mapIterator++ )
	{
		delete mapIterator->second;
		mapIterator->second = nullptr;
	}
}

SpriteAnimationDefinition* SpriteAnimationSetDefinition::GetSpriteAnimationDefinitionFromName( const std::string& spriteAnimationDefinitionName )
{
	ASSERT_OR_DIE( ( m_spriteAnimationDefinitionsByName.find( spriteAnimationDefinitionName ) != m_spriteAnimationDefinitionsByName.end() ), "SpriteAnimationSetDefinition::GetSpriteAnimationDefinitionFromName - key for sprite animation not found. Aborting..." );
	return m_spriteAnimationDefinitionsByName[ spriteAnimationDefinitionName ];
}

SpriteAnimationDefinition* SpriteAnimationSetDefinition::GetDefaultSpriteAnimationDefinition()
{
	return m_spriteAnimationDefinitionsByName[ m_defaultAnimName ];
}

std::string SpriteAnimationSetDefinition::GetDefaultSpriteAnimationDefinitionName() const
{
	return m_defaultAnimName;
}

void SpriteAnimationSetDefinition::PopulateSpriteAnimations(  const tinyxml2::XMLElement& spriteAnimSetElement, Renderer& renderer )
{
	if ( !spriteAnimSetElement.NoChildren() )
	{
		for ( const tinyxml2::XMLElement* spriteAnimElement = spriteAnimSetElement.FirstChildElement(); spriteAnimElement != nullptr; spriteAnimElement = spriteAnimElement->NextSiblingElement() )
		{
			std::string spriteAnimationName = ParseXmlAttribute( *spriteAnimElement, SPRITE_ANIM_DEF_NAME_XML_ATTRIBUTE_NAME, "" );
			m_spriteAnimationDefinitionsByName[ spriteAnimationName ] = new SpriteAnimationDefinition( *spriteAnimElement,  m_fps, renderer );
		}
	}
}
