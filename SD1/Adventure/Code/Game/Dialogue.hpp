#pragma once

#include "Engine/Math/AABB2.hpp"
#include "ThirdParty/tinyXML2/tinyxml2.h"
#include <string>

enum DialogueStyle
{
	DIALOGUE_STYLE_NONE = -1,
	DIALOGUE_STYLE_PROGRESSIVE,
	NUM_DIALOGUE_STYLES
};

class Dialogue
{

public:
	explicit Dialogue( const std::string text, const AABB2& bounds, DialogueStyle style );
	~Dialogue();

	std::string GetText() const;
	AABB2 GetRelativeBounds() const;
	DialogueStyle GetStyle() const;
	bool ShouldBeDismissed() const;
	void ResetDismissalState();
	void Update( float deltaSeconds );
	void Render( float renderAlpha );

public:
	static std::string GetDialogueStyleNameFromStyle( DialogueStyle style );
	static DialogueStyle GetDialogueStyleFromStyleName( const std::string& styleName );
	static constexpr char DIALOGUE_TEXT_XML_ATTRIBUTE_NAME[] = "text";
	static constexpr char DIALOGUE_BOUNDS_XML_ATTRIBUTE_NAME[] = "bounds";
	static constexpr char DIALOGUE_STYLE_XML_ATTRIBUTE_NAME[] = "style";

private:
	void HandleKeyboardInput();
	void HandleXboxControllerInput();
	std::string GetTextToRender() const;
	unsigned int GetMaxLetterIndexForRenderExclusive() const;

private:
	std::string m_text = "";
	AABB2 m_dialogueRelativeBounds;
	DialogueStyle m_style = DialogueStyle::DIALOGUE_STYLE_NONE;
	float m_ageInSeconds = 0.0f;
	bool m_shouldBeDismissed = false;

};

Dialogue* ParseXmlAttribute( const tinyxml2::XMLElement& element, Dialogue* defaultValue );
