#include "Game/Dialogue.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/XmlUtilities.hpp"
#include "Engine/Math/MathUtils.hpp"

Dialogue::Dialogue( const std::string text, const AABB2& bounds, DialogueStyle style = DialogueStyle::DIALOGUE_STYLE_NONE )
	:	m_text( text ),
		m_dialogueRelativeBounds( bounds ),
		m_style( style )
{
	
}

Dialogue::~Dialogue()
{

}

std::string Dialogue::GetText() const
{
	return m_text;
}

AABB2 Dialogue::GetRelativeBounds() const
{
	return m_dialogueRelativeBounds;
}

DialogueStyle Dialogue::GetStyle() const
{
	return m_style;
}

bool Dialogue::ShouldBeDismissed() const
{
	return m_shouldBeDismissed;
}

void Dialogue::ResetDismissalState()
{
	m_shouldBeDismissed = false;
	m_ageInSeconds = 0.0f;
}

void Dialogue::Update( float deltaSeconds )
{
	m_ageInSeconds += deltaSeconds;
	HandleKeyboardInput();
	HandleXboxControllerInput();
}

void Dialogue::Render( float renderAlpha )
{
	AABB2 currentOrtho = g_mainCamera->GetViewportBounds();
	AABB2 actualDialogBounds = currentOrtho;
	actualDialogBounds.mins.x = Interpolate( currentOrtho.mins.x, currentOrtho.maxs.x, m_dialogueRelativeBounds.mins.x );
	actualDialogBounds.maxs.x = Interpolate( currentOrtho.mins.x, currentOrtho.maxs.x, m_dialogueRelativeBounds.maxs.x );
	actualDialogBounds.mins.y = Interpolate( currentOrtho.mins.y, currentOrtho.maxs.y, m_dialogueRelativeBounds.mins.y );
	actualDialogBounds.maxs.y = Interpolate( currentOrtho.mins.y, currentOrtho.maxs.y, m_dialogueRelativeBounds.maxs.y );

	// Get config variables
	Rgba boxBackgroundColor = g_gameConfigBlackboard.GetValue( "dialogBoxColor", Rgba::WHITE ).GetWithAlpha( renderAlpha );
	Rgba textColor = g_gameConfigBlackboard.GetValue( "dialogTextColor", Rgba::WHITE ).GetWithAlpha( renderAlpha );
	float borderPadding = g_gameConfigBlackboard.GetValue( "dialogBorderPadding", 0.2f );
	float borderThickness = g_gameConfigBlackboard.GetValue( "dialogBorderThickness", 0.5f );
	float textLineHeight = g_gameConfigBlackboard.GetValue( "dialogTextLineHeight", 0.2f );
	
	// Box sizes for the border and the text
	AABB2 boxBorderBounds = AABB2( actualDialogBounds );
	boxBorderBounds.AddPaddingToSides( -borderPadding, -borderPadding );
	AABB2 textBounds = AABB2( boxBorderBounds );
	boxBorderBounds.AddPaddingToSides( -borderPadding, -borderPadding );

	g_renderer->DrawAABB( actualDialogBounds, boxBackgroundColor );
	g_renderer->DrawLineBorder( boxBorderBounds, textColor, borderThickness );
	g_renderer->DrawTextInBox2D( GetTextToRender(), textBounds, textLineHeight, textColor, 1.0f, g_renderer->CreateOrGetBitmapFont( FIXED_FONT_NAME ), TextDrawMode::TEXT_DRAW_WORD_WRAP, Vector2( 0.5f, 0.5f ) );
}

std::string Dialogue::GetTextToRender() const
{
	if ( m_style == DialogueStyle::DIALOGUE_STYLE_PROGRESSIVE )
	{
		std::string textToRender = m_text;
		for ( unsigned int indexToEraseFrom = GetMaxLetterIndexForRenderExclusive(); indexToEraseFrom < m_text.size(); indexToEraseFrom++ )
		{
			if ( textToRender[ indexToEraseFrom ] != '\n' )
			{
				textToRender[ indexToEraseFrom ] = ' ';
			}
		}

		return textToRender;
	}
	else
	{
		return m_text;
	}
}

unsigned int Dialogue::GetMaxLetterIndexForRenderExclusive() const
{
	if ( m_style == DialogueStyle::DIALOGUE_STYLE_PROGRESSIVE )
	{
		float dialogLettersPerSecond = g_gameConfigBlackboard.GetValue( "dialogProgressiveLettersPerSecond", 10.0f );
		float numLettersToRenderAsFloat = dialogLettersPerSecond * m_ageInSeconds;

		unsigned int numLettersToRender = static_cast< unsigned int >( numLettersToRenderAsFloat );
		numLettersToRender = ClampInt( numLettersToRender, 0, m_text.size() );
		return numLettersToRender;
	}
	else
	{
		return m_text.size();
	}
}

void Dialogue::HandleKeyboardInput()
{
	if ( g_inputSystem->WasKeyJustPressed( InputSystem::KEYBOARD_SPACE ) )
	{
		m_shouldBeDismissed = true;
	}
}

void Dialogue::HandleXboxControllerInput()
{
	if ( g_inputSystem->GetController( 0 ).IsConnected() )
	{
		if ( g_inputSystem->GetController( 0 ).WasKeyJustPressed( InputXboxControllerMappings::GAMEPAD_Y ) )
		{
			m_shouldBeDismissed = true;
		}
	}
}

std::string Dialogue::GetDialogueStyleNameFromStyle( DialogueStyle style )
{
	switch( style )
	{
		case DialogueStyle::DIALOGUE_STYLE_NONE				:		return "";
		case DialogueStyle::DIALOGUE_STYLE_PROGRESSIVE		:		return "Progressive";
		default		:		return "";
	}
}

DialogueStyle Dialogue::GetDialogueStyleFromStyleName( const std::string& styleName )
{
	if ( styleName == "Progressive" )
	{
		return DialogueStyle::DIALOGUE_STYLE_PROGRESSIVE;
	}
	return DialogueStyle::DIALOGUE_STYLE_NONE;
}

Dialogue* ParseXmlAttribute( const tinyxml2::XMLElement& element, Dialogue* defaultValue )
{
	UNUSED( defaultValue );
	std::string dialogueText = ParseXmlAttribute( element, Dialogue::DIALOGUE_TEXT_XML_ATTRIBUTE_NAME, "" );
	AABB2 dialogueBounds = ParseXmlAttribute( element, Dialogue::DIALOGUE_BOUNDS_XML_ATTRIBUTE_NAME, AABB2() );
	DialogueStyle dialogueStyle = Dialogue::GetDialogueStyleFromStyleName( ParseXmlAttribute( element, Dialogue::DIALOGUE_STYLE_XML_ATTRIBUTE_NAME, "" ) );
	return new Dialogue( dialogueText, dialogueBounds, dialogueStyle );
}
