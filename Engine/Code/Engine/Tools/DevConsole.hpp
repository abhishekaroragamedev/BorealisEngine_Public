#pragma once

#include "Engine/Core/Logger.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/IntRange.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Tools/Command.hpp"
#include <string>
#include <vector>

class InputSystem;
class Renderer;

constexpr float CONSOLE_OVERLAY_RENDER_ALPHA = 0.5f;
constexpr float CONSOLE_SCROLLBAR_THICKNESS = 0.025f;
constexpr float HORIZONTAL_ARROW_KEY_INPUT_DELAY_REQUIRED = 1.0f;
constexpr float HORIZONTAL_ARROW_KEY_INPUT_DELAY_INCREMENT_FACTOR = 0.1f;
constexpr float SELECTED_TEXT_OVERLAY_RENDER_ALPHA = 0.75f;
constexpr float TYPED_TEXT_CURSOR_FLASH_TIME_SECONDS = 0.5f;
constexpr float TYPED_TEXT_PADDING = -0.01f;
constexpr float TYPED_TEXT_CELL_HEIGHT = 0.03f;
constexpr float TYPED_TEXT_CURSOR_WIDTH = 0.01f;
constexpr size_t OUTPUT_TEXT_MAX_LINES = 1000;

typedef void (*devconsole_cb)( const std::string&, void* );

struct ColoredString
{
public:
	ColoredString() {}
	ColoredString( const std::string& str, const Rgba& color );

	std::string m_string = "";
	Rgba m_color = Rgba::WHITE;
};

class DevConsole 
{

private:
	DevConsole( Renderer* renderer = nullptr ); 
	~DevConsole(); 

public:
	// Handles all input
	void Update( const InputSystem& inputSystem, const Renderer& renderer );
	// Renders the display
	void Render( Renderer& renderer ) const; 
	bool IsOpen() const;
	std::string GetOutputText() const;
	void ToggleOpenState();
	void AddLineToOutputText( const char* lineToAdd, const Rgba& color = Rgba::WHITE );
	void AddLineToOutputText( const std::string& lineToAdd, const Rgba& color = Rgba::WHITE );
	void AddCharacterToBuffer( char charToAdd );
	void AddStringToBuffer( const std::string& stringToAdd );
	void ClearOutput();

public:
	static void CreateInstance( Renderer* renderer );
	static DevConsole* GetInstance();
	static void DeleteInstance();

private:
	void UpdateTextFromConsolePrintf();
	void HandleKeyboardInput( const InputSystem& inputSystem );
	void HandleReturnKeyInput( const InputSystem& inputSystem );
	void HandleEscapeKeyInput( const InputSystem& inputSystem );
	void HandleBackspaceAndDeleteKeyInput( const InputSystem& inputSystem );
	void HandleHorizontalArrowKeyInput( const InputSystem& inputSystem );
	void HandleVerticalArrowKeyInputForScroll( const InputSystem& inputSystem );
	void HandleVerticalArrowKeyInputForHistory( const InputSystem& inputSystem );
	void HandleClipboardInput( const InputSystem& inputSystem );
	void HandleOtherInput( const InputSystem& inputSystem );
	bool CopyTextToClipboard( std::string& errorMessage );
	bool PasteTextFromClipboard( std::string& errorMessage );
	void UpdateTypedTextCursor();
	void UpdateSelectedTextRange();
	void CorrectCursorPosition();
	void EraseSelectedText();
	bool IsScrollLevelAtBottom() const;
	void RenderTextInputBox( const Renderer& renderer ) const;
	void RenderSelectedTextOverlay( const Renderer& renderer ) const;
	void RenderOutput( const Renderer& renderer ) const;
	void RenderOutputText( const Renderer& renderer, const AABB2& outputBounds ) const;
	void RenderOutputTextScrollbar( const Renderer& renderer, const AABB2& outputBounds ) const;
	void RenderRCSWidget( const Renderer& renderer ) const;	// RCS - Remote Command Service

private:
	static const Rgba NORMAL_OVERLAY_COLOR;
	static const Rgba SCROLLED_OVERLAY_COLOR;
	static Vector2 s_dimensionScaling;

private:
	bool m_isOpen = false;
	bool m_renderTypedTextCursor = false;
	bool m_isScrolling = false;
	std::string m_typedText = "";
	size_t m_typedTextCursorPosition = 0;
	size_t m_lowestVisibleOutputLineIndex = 0;
	size_t  m_maxVisibleOutputLines = 0;
	int m_commandHistoryIndex = -1;
	std::vector< ColoredString > m_outputTextLines;
	IntRange m_selectedTextIndices;
	Camera* m_devConsoleCamera = nullptr;

};


// Global functions

// I like this as a C function or a static 
// method so that it is easy for systems to check if the dev
// console is open.  
bool IsDevConsoleOpen(); 

// Should add a line of coloured text to the output 
void ConsolePrintf( const Rgba& color, const char* format, ... ); 

// Same as previous, be defaults to a color visible easily on your console
void ConsolePrintf( char const *format, ... );

bool ClearCommand( Command& command );
bool SaveLogCommand( Command& command );
bool EchoWithColorCommand( Command& command );
bool ClearCommandHistoryCommand( Command& command );
bool CloneProcessCommand( Command& command );

bool LogTestCommand( Command& command );
bool LogFlushTestCommand( Command& command );
bool LogShowTagCommand( Command& command );
bool LogHideTagCommand( Command& command );
bool LogShowAllCommand( Command& command );
bool LogHideAllCommand( Command& command );
bool LogTimestampCommand( Command& command );

// For hooks into the DevConsole output
void DevConsoleHook( devconsole_cb callback, void* userData );
void DevConsoleUnhook( devconsole_cb callback, void* userData );
void DevConsoleHooksEnable();
void DevConsoleHooksDisable();

// DevConsole is hooked into the Log System
void DevConsoleLog( const Log& log, void* );
