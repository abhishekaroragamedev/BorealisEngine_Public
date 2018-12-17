#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/XmlUtilities.hpp"
#include "Engine/Renderer/SpriteAnimationDefinition.hpp"

SpriteAnimationDefinition::SpriteAnimationDefinition( const SpriteSheet& spriteSheet, float defaultFps, const SpriteAnimMode& playbackMode, const std::vector< int >& spriteIndices )
	:	m_spriteSheet( &spriteSheet ),
		m_fps( defaultFps ),
		m_playbackMode( playbackMode ),
		m_spriteIndices( spriteIndices )
{
	m_durationSeconds = static_cast< float >( m_spriteIndices.size() ) / m_fps;
	SetDurationForEachSpriteSeconds();
}

SpriteAnimationDefinition::SpriteAnimationDefinition( const tinyxml2::XMLElement& spriteAnimElement, float defaultFps, Renderer& renderer )
	:	m_fps( defaultFps )
{

	int playbackModeInt = ParseXmlAttribute( spriteAnimElement, PLAYBACK_MODE_ATTRIBUTE_NAME, static_cast<int>( m_playbackMode ) );
	ASSERT_OR_DIE( playbackModeInt > SpriteAnimMode::SPRITE_ANIM_MODE_INVALID && playbackModeInt < SpriteAnimMode::NUM_SPRITE_ANIM_MODES, "SpriteAnimation constructor - Invalid SpriteAnimMode value found in XML. Please enter 0, 1 or 2. Aborting..." );
	m_playbackMode = static_cast< SpriteAnimMode >( playbackModeInt );

	m_name = ParseXmlAttribute( spriteAnimElement, NAME_XML_ATTRIBUTE_NAME, m_name );
	m_spriteIndices = ParseXmlAttribute( spriteAnimElement, SPRITE_INDICES_XML_ATTRIBUTE_NAME, m_spriteIndices );
	m_fps = ParseXmlAttribute( spriteAnimElement, SPRITE_FPS_XML_ATTRIBUTE_NAME, m_fps );
	m_durationSeconds = static_cast< float >( m_spriteIndices.size() ) / m_fps;
	m_orientationOffsetDegrees = ParseXmlAttribute( spriteAnimElement, SPRITE_ORIENTATION_OFFSET_XML_ATTRIBUTE_NAME, m_orientationOffsetDegrees );
	PopulateSpriteSheetInfo( spriteAnimElement, renderer );

	SetDurationForEachSpriteSeconds();
}

SpriteAnimationDefinition::~SpriteAnimationDefinition()
{
	delete m_spriteSheet;
	m_spriteSheet = nullptr;
}

void SpriteAnimationDefinition::PopulateSpriteSheetInfo( const tinyxml2::XMLElement& spriteAnimElement, Renderer& renderer )
{
	std::string overriddenSpriteSheetName = ParseXmlAttribute( spriteAnimElement, SPRITE_SHEET_XML_ATTRIBUTE_NAME, "" );
	if ( overriddenSpriteSheetName != "" )
	{
		Texture* spriteSheetTexture = renderer.CreateOrGetTexture( std::string( SPRITE_SHEET_DIRECTORY_PATH ) + overriddenSpriteSheetName );
		IntVector2 spriteSheetLayout = ParseXmlAttribute( spriteAnimElement, SPRITE_LAYOUT_XML_ATTRIBUTE_NAME, IntVector2::ZERO );
		m_spriteSheet = new SpriteSheet( *spriteSheetTexture, spriteSheetLayout.x, spriteSheetLayout.y );
	}
	else	// Use the default spritesheet
	{
		const tinyxml2::XMLElement* parentElement = spriteAnimElement.Parent()->ToElement();
		std::string defaultSpriteSheetName = ParseXmlAttribute( *parentElement, SPRITE_SHEET_XML_ATTRIBUTE_NAME, "" );
		Texture* spriteSheetTexture = renderer.CreateOrGetTexture( std::string( SPRITE_SHEET_DIRECTORY_PATH ) + defaultSpriteSheetName );
		IntVector2 spriteSheetLayout = ParseXmlAttribute( *parentElement, SPRITE_LAYOUT_XML_ATTRIBUTE_NAME, IntVector2::ZERO );
		m_spriteSheet = new SpriteSheet( *spriteSheetTexture, spriteSheetLayout.x, spriteSheetLayout.y );
	}
}

float SpriteAnimationDefinition::GetDurationSeconds() const
{
	return m_durationSeconds;
}

float SpriteAnimationDefinition::GetOrientationOffset() const
{
	return m_orientationOffsetDegrees;
}

void SpriteAnimationDefinition::SetDurationForEachSpriteSeconds()
{
	float numberOfSpritesFloat = static_cast<float>( GetNumberOfSpritesInAnimation() );
	m_singleSpriteDurationSeconds = m_durationSeconds / numberOfSpritesFloat;
}


float SpriteAnimationDefinition::GetSingleSpriteDurationSeconds() const
{
	return m_singleSpriteDurationSeconds;
}

int SpriteAnimationDefinition::GetStartSpriteIndex() const
{
	return m_spriteIndices[ 0 ];
}

int SpriteAnimationDefinition::GetEndSpriteIndex() const
{
	return m_spriteIndices[ m_spriteIndices.size() - 1 ];
}

std::string SpriteAnimationDefinition::GetName() const
{
	return m_name;
}

std::vector< int > SpriteAnimationDefinition::GetSpriteIndices() const
{
	return m_spriteIndices;
}

int SpriteAnimationDefinition::GetNumberOfSpritesInAnimation() const
{
	return m_spriteIndices.size();
}

const SpriteSheet* SpriteAnimationDefinition::GetSpriteSheet() const
{
	return m_spriteSheet;
}

const Texture* SpriteAnimationDefinition::GetTexture() const
{
	return m_spriteSheet->GetTexture();
}

SpriteAnimMode SpriteAnimationDefinition::GetPlaybackMode() const
{
	return m_playbackMode;
}
