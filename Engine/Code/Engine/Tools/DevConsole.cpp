#include "Engine/Core/Clock.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ThreadSafeTypes.hpp"
#include "Engine/Core/Window.hpp"
#include "Engine/FileUtils/File.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Tools/Command.hpp"
#include "Engine/Tools/DevConsole.hpp"
#include "Engine/Tools/Profiler/ProfileScope.hpp"
#include <stdarg.h>
#include <stdio.h>
#include <windows.h>

ColoredString::ColoredString( const std::string& str, const Rgba& color )
	:	m_string( str ),
		m_color( color )
{

}

bool CaptureKeyboardInput( unsigned int msg, size_t wparam, size_t lparam );

DevConsole* g_devConsole = nullptr;
ThreadSafeQueue< ColoredString > g_consolePrintfQueue;

void DevConsole::CreateInstance( Renderer* renderer )
{
	g_devConsole = new DevConsole( renderer );
}

DevConsole* DevConsole::GetInstance()
{
	return g_devConsole;
}

void DevConsole::DeleteInstance()
{
	delete g_devConsole;
	g_devConsole = nullptr;
}

Vector2 DevConsole::s_dimensionScaling = Vector2::ONE;
const Rgba DevConsole::NORMAL_OVERLAY_COLOR = Rgba( 0, 0, 0, 255 );
const Rgba DevConsole::SCROLLED_OVERLAY_COLOR = Rgba( 0, 100, 0, 255 );

DevConsole::DevConsole( Renderer* renderer/*=nullptr*/ )
{
	m_maxVisibleOutputLines = static_cast< unsigned int >( 1.9f / TYPED_TEXT_CELL_HEIGHT );
	m_devConsoleCamera = new Camera( renderer->GetDefaultColorTarget() );
	m_devConsoleCamera->SetProjectionOrtho( 2.0f, Window::GetAspect(), -1.0f, 1.0f );

	DevConsole::s_dimensionScaling = Vector2( Window::GetAspect(), 1.0f );

	LogHookRegister( DevConsoleLog );
	CommandRegister( "help", HelpCommand, "Displays registered command names and descriptions." );
	CommandRegister( "clear", ClearCommand, "Clears the console output window." );
	CommandRegister( "save_log", SaveLogCommand, "Saves the current console output to file." );
	CommandRegister( "echo_with_color", EchoWithColorCommand, "echo_with_color \"<message>\" \"<r,g,b,a>\" - prints a colored string." );
	CommandRegister( "clear_history", ClearCommandHistoryCommand, "Clears the command history." );
	CommandRegister( "log_test", LogTestCommand, "Prints the contents of the provided file to Log.txt." );
	CommandRegister( "log_flush_test", LogFlushTestCommand, "Flushes the log file and hits a breakpoint to allow verification of the log file output." );
	CommandRegister( "log_show_tag", LogShowTagCommand, "Enable logging for messages with a given tag." );
	CommandRegister( "log_hide_tag", LogHideTagCommand, "Disable logging for messages with a given tag." );
	CommandRegister( "log_show_all", LogShowAllCommand, "Enable logging for all messages." );
	CommandRegister( "log_hide_all", LogHideAllCommand, "Disable logging for all messages." );
	CommandRegister( "log_timestamp", LogTimestampCommand, "Enable/Disable timestamps for logs. Enabled by default." );
	RegisterClockCommands();

	CommandStartup();

	AddLineToOutputText( std::string( ENGINE_NAME ) + " " + std::string( ENGINE_VERSION ), Rgba::CYAN );
	AddLineToOutputText( std::string( ENGINE_AUTHOR ) + std::string( ", " ) + std::string( ENGINE_YEARS ), Rgba::CYAN );
}

DevConsole::~DevConsole()
{
	CommandShutdown();

	delete m_devConsoleCamera;
	m_devConsoleCamera = nullptr;
}

void DevConsole::Update(const InputSystem& inputSystem, const Renderer& renderer )
{
	UpdateTextFromConsolePrintf();
	if ( m_isOpen )
	{
		UpdateTypedTextCursor();
		HandleKeyboardInput( inputSystem );
		UpdateSelectedTextRange();
	}
}

void DevConsole::UpdateTextFromConsolePrintf()
{
	ColoredString newLine;
	while ( g_consolePrintfQueue.Dequeue( &newLine ) )
	{
		AddLineToOutputText( newLine.m_string, newLine.m_color );
	}
}

void DevConsole::UpdateTypedTextCursor()
{
	static float s_typedTextCursorFlashTimeElapsedSeconds = 0.0f;
	s_typedTextCursorFlashTimeElapsedSeconds += GetMasterDeltaSecondsF();
	if ( s_typedTextCursorFlashTimeElapsedSeconds >= TYPED_TEXT_CURSOR_FLASH_TIME_SECONDS )
	{
		s_typedTextCursorFlashTimeElapsedSeconds = 0.0f;
		m_renderTypedTextCursor = !m_renderTypedTextCursor;
	}
}

void DevConsole::UpdateSelectedTextRange()
{
	if ( m_selectedTextIndices.GetRangeMagnitude() == 0 )
	{
		m_selectedTextIndices.max = ( int )m_typedTextCursorPosition;
		m_selectedTextIndices.min = ( int )m_typedTextCursorPosition;
	}
}

void DevConsole::HandleKeyboardInput( const InputSystem& inputSystem )
{
	HandleBackspaceAndDeleteKeyInput( inputSystem );
	HandleHorizontalArrowKeyInput( inputSystem );
	HandleVerticalArrowKeyInputForScroll( inputSystem );
	HandleVerticalArrowKeyInputForHistory( inputSystem );
	HandleClipboardInput( inputSystem );
	HandleOtherInput( inputSystem );
	HandleReturnKeyInput( inputSystem );
	HandleEscapeKeyInput( inputSystem );
}

void DevConsole::HandleBackspaceAndDeleteKeyInput( const InputSystem& inputSystem )
{
	if ( inputSystem.WasKeyJustPressed( InputSystem::KEYBOARD_BACKSPACE ) )
	{
		if ( m_selectedTextIndices.GetRangeMagnitude() > 0 )		// Delete selected text
		{
			EraseSelectedText();
		}
		else if ( m_typedTextCursorPosition > 0 )		// Delete preceding character
		{
			m_typedText.erase( --m_typedTextCursorPosition, 1 );
		}
	}
	if ( inputSystem.WasKeyJustPressed( InputSystem::KEYBOARD_DELETE ) )
	{
		if ( m_selectedTextIndices.GetRangeMagnitude() > 0 )		// Delete selected text
		{
			EraseSelectedText();
		}
		else if ( m_typedTextCursorPosition < m_typedText.length() )		// Delete next character
		{
			m_typedText.erase( m_typedTextCursorPosition, 1 );
		}
	}
}

void DevConsole::HandleHorizontalArrowKeyInput( const InputSystem& inputSystem )
{
	static float s_horizontalArrowKeyDelaySeconds = HORIZONTAL_ARROW_KEY_INPUT_DELAY_REQUIRED;
	s_horizontalArrowKeyDelaySeconds += HORIZONTAL_ARROW_KEY_INPUT_DELAY_INCREMENT_FACTOR;

	if ( ( s_horizontalArrowKeyDelaySeconds > HORIZONTAL_ARROW_KEY_INPUT_DELAY_REQUIRED ) && inputSystem.IsKeyDown( InputSystem::KEYBOARD_LEFT_ARROW ) )
	{
		if ( m_typedTextCursorPosition > 0 )
		{
			m_typedTextCursorPosition--;
		}
		if ( inputSystem.IsKeyDown( InputSystem::KEYBOARD_SHIFT ) )
		{
			if ( m_selectedTextIndices.Contains( ( int )m_typedTextCursorPosition ) && ( ( unsigned int )( m_selectedTextIndices.max - 1 ) == m_typedTextCursorPosition ) )
			{
				m_selectedTextIndices.max--;
			}
			else if ( m_selectedTextIndices.min > 0 && ( ( unsigned int )( m_selectedTextIndices.min - 1 ) == m_typedTextCursorPosition ) )
			{
				m_selectedTextIndices.min--;
			}
		}
		else
		{
			m_selectedTextIndices.min = ( int )m_typedTextCursorPosition;
			m_selectedTextIndices.max = ( int )m_typedTextCursorPosition;
		}

		s_horizontalArrowKeyDelaySeconds = 0.0f;
	}
	if ( ( s_horizontalArrowKeyDelaySeconds > HORIZONTAL_ARROW_KEY_INPUT_DELAY_REQUIRED ) && inputSystem.IsKeyDown( InputSystem::KEYBOARD_RIGHT_ARROW ) )
	{
		if ( m_typedTextCursorPosition < m_typedText.length() )
		{
			m_typedTextCursorPosition++;
		}
		if ( inputSystem.IsKeyDown( InputSystem::KEYBOARD_SHIFT ) )
		{
			if ( m_selectedTextIndices.Contains( ( int )m_typedTextCursorPosition ) && ( ( unsigned int )( m_selectedTextIndices.min + 1 ) == m_typedTextCursorPosition ) )
			{
				m_selectedTextIndices.min++;
			}
			else if ( ( unsigned int ) m_selectedTextIndices.max < m_typedText.length() && ( ( unsigned int )( m_selectedTextIndices.max + 1 ) == m_typedTextCursorPosition )  )
			{
				m_selectedTextIndices.max++;
			}
		}
		else
		{
			m_selectedTextIndices.min = ( int )m_typedTextCursorPosition;
			m_selectedTextIndices.max = ( int )m_typedTextCursorPosition;
		}

		s_horizontalArrowKeyDelaySeconds = 0.0f;
	}
}

void DevConsole::HandleVerticalArrowKeyInputForScroll( const InputSystem& inputSystem )
{
	m_isScrolling = false;
	if ( inputSystem.IsKeyDown( InputSystem::KEYBOARD_PAGE_UP ) )
	{
		if ( m_lowestVisibleOutputLineIndex > ( m_maxVisibleOutputLines - 1 ) )
		{
			m_lowestVisibleOutputLineIndex--;
		}
		m_isScrolling = true;
	}
	if ( inputSystem.IsKeyDown( InputSystem::KEYBOARD_PAGE_DOWN ) )
	{
		if ( m_lowestVisibleOutputLineIndex < ( m_outputTextLines.size() - 1 ) )
		{
			m_lowestVisibleOutputLineIndex++;
		}
		m_isScrolling = true;
	}
}

void DevConsole::HandleVerticalArrowKeyInputForHistory( const InputSystem& inputSystem )
{
	std::vector< std::string > currentHistory = GetCommandHistory();
	size_t historySize = currentHistory.size();
	if ( historySize == 0 )
	{
		return;
	}
	if ( inputSystem.WasKeyJustPressed( InputSystem::KEYBOARD_UP_ARROW ) )
	{
		m_commandHistoryIndex = ( m_commandHistoryIndex + 1 ) % ( (int)historySize );
		m_typedText = currentHistory[ m_commandHistoryIndex ];
		m_typedTextCursorPosition = m_typedText.length();
	}
	if ( inputSystem.WasKeyJustPressed( InputSystem::KEYBOARD_DOWN_ARROW ) )
	{
		m_commandHistoryIndex--;
		if ( m_commandHistoryIndex < 0 )
		{
			m_commandHistoryIndex = ( (int)historySize ) - 1;
		}
		m_typedText = currentHistory[ m_commandHistoryIndex ];
		m_typedTextCursorPosition = m_typedText.length();
	}
}

void DevConsole::HandleClipboardInput( const InputSystem& inputSystem )
{
	bool clipboardSuccess = true;
	std::string errorMessage = "";

	if ( inputSystem.WasKeyJustPressed( InputSystem::KEYBOARD_CTRL_C ) && ( m_selectedTextIndices.GetRangeMagnitude() > 0 ) )
	{
		clipboardSuccess = CopyTextToClipboard( errorMessage );
	}
	else if ( inputSystem.WasKeyJustPressed( InputSystem::KEYBOARD_CTRL_V ) )
	{
		clipboardSuccess = PasteTextFromClipboard( errorMessage );
	}
	else if ( inputSystem.WasKeyJustPressed( InputSystem::KEYBOARD_CTRL_X ) && ( m_selectedTextIndices.GetRangeMagnitude() > 0 ) )
	{
		clipboardSuccess = CopyTextToClipboard( errorMessage );
		EraseSelectedText();
	}

	if ( !clipboardSuccess )
	{
		ConsolePrintf( Rgba::RED, std::string( "ERROR: DevConsole::HandleClipboardInput - " + errorMessage ).c_str() );
	}
}

bool DevConsole::CopyTextToClipboard( std::string& errorMessage )
{
	bool clipboardSuccess = OpenClipboard( *( ( HWND* ) Window::GetInstance()->GetHandle() ) );
	if ( clipboardSuccess )
	{
		clipboardSuccess = EmptyClipboard();
		if ( !clipboardSuccess )
		{
			errorMessage = "Could not empty Clipboard.";
		}

		std::string selectedText = m_typedText.substr( m_selectedTextIndices.min, m_selectedTextIndices.GetRangeMagnitude() );
		const char* selectedTextPrimitive = selectedText.c_str();

		HGLOBAL memoryHandle = GlobalAlloc( GMEM_MOVEABLE, ( ( selectedText.length() + 1 ) * sizeof(char ) ) );
		LPVOID lockedMemory = GlobalLock( memoryHandle );
		memcpy( lockedMemory, selectedTextPrimitive, ( selectedText.length() * sizeof( char ) ) );
		GlobalUnlock( memoryHandle );
		SetClipboardData( CF_TEXT, memoryHandle );
		clipboardSuccess = CloseClipboard();
		if ( !clipboardSuccess )
		{
			errorMessage = "Could not close Clipboard.";
		}
	}
	else
	{
		errorMessage = "Could not open Clipboard.";
	}
	return clipboardSuccess;
}

bool DevConsole::PasteTextFromClipboard( std::string& errorMessage )
{
	bool clipboardSuccess = IsClipboardFormatAvailable( CF_TEXT );
	if ( clipboardSuccess )
	{
		clipboardSuccess = OpenClipboard( *( ( HWND* ) Window::GetInstance()->GetHandle() ) );
		if ( clipboardSuccess )
		{
			HGLOBAL memoryHandle = GetClipboardData( CF_TEXT );
			if ( memoryHandle == NULL )
			{
				clipboardSuccess = false;
				errorMessage = "Could not access clipboard memory.";
				CloseClipboard();
			}
			else
			{
				LPTSTR clipboardContent = ( LPTSTR ) GlobalLock( memoryHandle );
				if ( clipboardContent == NULL )
				{
					clipboardSuccess = false;
					errorMessage = "Clipboard is empty.";
					CloseClipboard();
				}
				else		// SUCCESS
				{
					AddStringToBuffer( std::string( ( char* ) clipboardContent ) );
					GlobalUnlock( memoryHandle );

					clipboardSuccess = CloseClipboard();
					if ( !clipboardSuccess )
					{
						errorMessage = "Could not close Clipboard.";
					}
				}
			}
		}
		else
		{
			errorMessage = "Could not open Clipboard.";
		}
	}
	else
	{
		errorMessage = "Invalid clipboard format found. Cannot paste into console window.";
	}
	return clipboardSuccess;
}

void DevConsole::HandleOtherInput( const InputSystem& inputSystem )
{
	if ( inputSystem.WasKeyJustPressed( InputSystem::KEYBOARD_CTRL_A ) && ( m_typedText.length() > 0 ) )
	{
		m_selectedTextIndices.min = 0;
		m_selectedTextIndices.max = ( int )m_typedText.length();
		m_typedTextCursorPosition = m_typedText.length();
	}
}

void DevConsole::HandleReturnKeyInput( const InputSystem& inputSystem )
{
	if ( inputSystem.WasKeyJustPressed( InputSystem::KEYBOARD_RETURN ) )
	{
		if ( !m_typedText.empty() )
		{
			AddLineToOutputText( "Running command \'" + m_typedText + "\'..." );
			bool commandSuccess = CommandRun( m_typedText.c_str() );
			if ( commandSuccess )
			{
				AddLineToOutputText( "COMMAND SUCCEEDED: \'" + m_typedText + "\'", Rgba::GREEN );
			}
			else
			{
				AddLineToOutputText( "COMMAND FAILED: \'" + m_typedText + "\'", Rgba::RED );
			}
			m_typedText.clear();
			m_typedTextCursorPosition = 0;
			m_commandHistoryIndex = -1;
			m_selectedTextIndices.min = 0;
			m_selectedTextIndices.max = 0;
		}
	}
}

void DevConsole::HandleEscapeKeyInput( const InputSystem& inputSystem )
{
	if ( inputSystem.WasKeyJustPressed( InputSystem::KEYBOARD_ESCAPE ) )		// ESCAPE
	{
		if ( m_selectedTextIndices.GetRangeMagnitude() > 0 )		// Deselect text
		{
			m_selectedTextIndices.min = ( int )m_typedTextCursorPosition;
			m_selectedTextIndices.max = ( int )m_typedTextCursorPosition;
		}
		else if ( !m_typedText.empty() )		// Clear typed text
		{
			m_typedText.clear();
			m_typedTextCursorPosition = 0;
			m_commandHistoryIndex = -1;
		}
		else		// Close the console
		{
			ToggleOpenState();
		}
	}
}

void DevConsole::CorrectCursorPosition()
{
	m_typedTextCursorPosition = m_selectedTextIndices.min;
}

void DevConsole::EraseSelectedText()
{
	m_typedText.erase( m_selectedTextIndices.min, m_selectedTextIndices.GetRangeMagnitude() );
	CorrectCursorPosition();
	m_selectedTextIndices.min = ( int )m_typedTextCursorPosition;
	m_selectedTextIndices.max = ( int )m_typedTextCursorPosition;
}

bool DevConsole::IsScrollLevelAtBottom() const
{
	return ( ( m_outputTextLines.size() == 0 ) || m_lowestVisibleOutputLineIndex == ( m_outputTextLines.size() - 1 ) );
}

void DevConsole::ToggleOpenState()
{
	m_isOpen = !m_isOpen;
	if ( m_isOpen )
	{
		Window::GetInstance()->RegisterHandler( CaptureKeyboardInput );
	}
	else
	{
		Window::GetInstance()->UnregisterHandler( CaptureKeyboardInput );
	}
}

void DevConsole::AddLineToOutputText( const char* lineToAdd, const Rgba& color )
{
	AddLineToOutputText( std::string( lineToAdd ) );
}

void DevConsole::AddLineToOutputText( const std::string& lineToAdd, const Rgba& color )
{
	TokenizedString lines = TokenizedString( lineToAdd, "\n" );
	for ( std::string line : lines.GetTokens() )
	{
		m_outputTextLines.push_back( ColoredString( ( line + "\n" ), color ) );
		if ( !m_isScrolling )
		{
			m_lowestVisibleOutputLineIndex = m_outputTextLines.size() - 1;
		}
	}
}

void DevConsole::AddCharacterToBuffer( char charToAdd )
{
	if ( m_selectedTextIndices.GetRangeMagnitude() > 0 )
	{
		EraseSelectedText();
	}
	m_typedText.insert( m_typedTextCursorPosition, 1, charToAdd );
	m_typedTextCursorPosition++;
}

void DevConsole::AddStringToBuffer( const std::string& stringToAdd )
{
	if ( m_selectedTextIndices.GetRangeMagnitude() > 0 )
	{
		EraseSelectedText();
	}
	m_typedText.insert( m_typedTextCursorPosition, stringToAdd );
	m_typedTextCursorPosition += stringToAdd.length();
}

void DevConsole::ClearOutput()
{
	m_outputTextLines.clear();
	m_lowestVisibleOutputLineIndex = 0;
}

bool DevConsole::IsOpen() const
{
	return m_isOpen;
}

std::string DevConsole::GetOutputText() const
{
	std::string outputText = "";
	for ( ColoredString coloredString : m_outputTextLines )
	{
		outputText += coloredString.m_string;
	}
	return outputText;
}

void DevConsole::Render( Renderer& renderer ) const
{
	if ( m_isOpen )
	{
		PROFILE_SCOPE( "DevConsole::Render()" );
		renderer.UseShaderProgram( DEFAULT_SHADER_NAME );
		renderer.BindModelMatrixToCurrentShader( Matrix44::IDENTITY );
		renderer.EnableDepth( DepthTestCompare::COMPARE_ALWAYS, false );
		renderer.SetCamera( m_devConsoleCamera );
		Rgba overlayColor = ( IsScrollLevelAtBottom() )? DevConsole::NORMAL_OVERLAY_COLOR.GetWithAlpha( CONSOLE_OVERLAY_RENDER_ALPHA ) : DevConsole::SCROLLED_OVERLAY_COLOR.GetWithAlpha( CONSOLE_OVERLAY_RENDER_ALPHA );
		renderer.DrawAABB( AABB2( ( -1.0f *  DevConsole::s_dimensionScaling ), DevConsole::s_dimensionScaling ), overlayColor );
		RenderTextInputBox( renderer );
		RenderSelectedTextOverlay( renderer );
		RenderOutput( renderer );
	}
}

void DevConsole::RenderTextInputBox( const Renderer& renderer ) const
{
	AABB2 textInputBoxBounds = AABB2( ( -0.95f * DevConsole::s_dimensionScaling ), Vector2( ( 0.95f * DevConsole::s_dimensionScaling.x ), ( -0.9f * DevConsole::s_dimensionScaling.y ) ) );
	renderer.DrawLineBorder( textInputBoxBounds, Rgba::WHITE, 1.5f );
	textInputBoxBounds.AddPaddingToSides( TYPED_TEXT_PADDING, TYPED_TEXT_PADDING );
	if ( m_renderTypedTextCursor )
	{
		AABB2 cursorBounds = textInputBoxBounds;
		cursorBounds.mins.x += static_cast< float >( m_typedTextCursorPosition ) * TYPED_TEXT_CELL_HEIGHT;
		cursorBounds.maxs.x = cursorBounds.mins.x + TYPED_TEXT_CURSOR_WIDTH;
		renderer.DrawAABB( cursorBounds, Rgba::YELLOW );
	}
	renderer.DrawTextInBox2D( m_typedText, textInputBoxBounds, TYPED_TEXT_CELL_HEIGHT, Rgba::YELLOW, 1.0f, nullptr, TextDrawMode::TEXT_DRAW_OVERRUN, Vector2( 0.0f, 0.5f ) );
}

void DevConsole::RenderSelectedTextOverlay( const Renderer& renderer ) const
{
	if ( m_selectedTextIndices.GetRangeMagnitude() > 0 )
	{
		AABB2 selectedTextBounds = AABB2( ( -0.95f * DevConsole::s_dimensionScaling ), Vector2( ( 0.95f * ( DevConsole::s_dimensionScaling.x ) ), ( -0.9f * DevConsole::s_dimensionScaling.y ) ) );
		selectedTextBounds.AddPaddingToSides( TYPED_TEXT_PADDING, TYPED_TEXT_PADDING );
		selectedTextBounds.mins.x += static_cast< float >( m_selectedTextIndices.min ) * TYPED_TEXT_CELL_HEIGHT;
		selectedTextBounds.maxs.x = selectedTextBounds.mins.x + static_cast< float >( m_selectedTextIndices.GetRangeMagnitude() ) * TYPED_TEXT_CELL_HEIGHT;
		renderer.DrawAABB( selectedTextBounds, Rgba::YELLOW.GetWithAlpha( SELECTED_TEXT_OVERLAY_RENDER_ALPHA ) );
	}
}

void DevConsole::RenderOutput( const Renderer& renderer ) const
{
	AABB2 textOutputBoxBounds = AABB2( Vector2( ( -0.95f * DevConsole::s_dimensionScaling.x ), ( -0.85f * DevConsole::s_dimensionScaling.y ) ), Vector2( ( 0.95f * DevConsole::s_dimensionScaling.x ), ( 0.9f * DevConsole::s_dimensionScaling.y ) ) );
	renderer.DrawLine( Vector2( -DevConsole::s_dimensionScaling.x, textOutputBoxBounds.mins.y ), Vector2( DevConsole::s_dimensionScaling.x, textOutputBoxBounds.mins.y ), Rgba::WHITE, Rgba::WHITE, 1.5f );
	textOutputBoxBounds.AddPaddingToSides( TYPED_TEXT_PADDING, TYPED_TEXT_PADDING );
	RenderOutputText( renderer, textOutputBoxBounds );
	RenderOutputTextScrollbar( renderer, textOutputBoxBounds );
}

void DevConsole::RenderOutputText( const Renderer& renderer, const AABB2& outputBounds ) const
{
	if ( m_outputTextLines.size() == 0 )
	{
		return;
	}

	AABB2 currentLineBounds = outputBounds;
	currentLineBounds.maxs.y = currentLineBounds.mins.y + TYPED_TEXT_CELL_HEIGHT;
	int numLinesToPrint = Max( static_cast< int >( m_lowestVisibleOutputLineIndex - m_maxVisibleOutputLines ), 0 );
	for ( int lineIndex = static_cast< int >( m_lowestVisibleOutputLineIndex ); lineIndex >= numLinesToPrint; lineIndex-- )	// Iterate backwards so that the most recent line ends up on the bottom
	{
		renderer.DrawTextInBox2D( m_outputTextLines[ lineIndex ].m_string, currentLineBounds, TYPED_TEXT_CELL_HEIGHT, m_outputTextLines[ lineIndex ].m_color, 1.0f, nullptr, TextDrawMode::TEXT_DRAW_OVERRUN, Vector2( 0.0f, 1.0f ) );
		
		currentLineBounds.Translate( Vector2( 0.0f, TYPED_TEXT_CELL_HEIGHT ) );
		if ( currentLineBounds.maxs.y > DevConsole::s_dimensionScaling.x )	// Stop rendering text that's off-screen
		{
			break;
		}
	}
}

void DevConsole::RenderOutputTextScrollbar( const Renderer& renderer, const AABB2& outputBounds ) const
{
	if ( m_outputTextLines.size() < m_maxVisibleOutputLines )
	{
		return;
	}

	float scrollbarBoundsMinY = -0.85f + 1.9f * ( static_cast< float >( ( m_outputTextLines.size() - 1 ) - m_lowestVisibleOutputLineIndex ) / static_cast< float >( m_outputTextLines.size() ) );
	float scrollbarLength = 1.9f * static_cast< float >( m_maxVisibleOutputLines ) / static_cast< float >( m_outputTextLines.size() ) ;
	float scrollbarBoundsMaxY = ( scrollbarBoundsMinY + scrollbarLength ) * DevConsole::s_dimensionScaling.y;
	float scrollbarBoundsMaxX = 0.95f * DevConsole::s_dimensionScaling.x;
	float scrollbarBoundsMinX = scrollbarBoundsMaxX - CONSOLE_SCROLLBAR_THICKNESS;
	AABB2 scrollbarBounds = AABB2( Vector2( scrollbarBoundsMinX, scrollbarBoundsMinY ), Vector2( scrollbarBoundsMaxX, scrollbarBoundsMaxY ) );
	renderer.DrawAABB( scrollbarBounds, Rgba::WHITE );
}

bool IsDevConsoleOpen()
{
	return DevConsole::GetInstance()->IsOpen();
}

void ConsolePrintf( const Rgba& color, const char* format, ... )
{
	va_list args;
	va_start( args, format );
	std::string finalString = Stringv( format, args );
	va_end( args );

	ColoredString coloredString = ColoredString( finalString, color );
	g_consolePrintfQueue.Enqueue( coloredString );
}

void ConsolePrintf( char const *format, ... )
{
	va_list args;
	va_start( args, format );
	std::string finalString = Stringv( format, args );
	va_end( args );

	ColoredString coloredString = ColoredString( finalString, Rgba::WHITE );
	g_consolePrintfQueue.Enqueue( coloredString );
}

bool CaptureKeyboardInput( unsigned int msg, size_t wparam, size_t lparam )
{
	if ( msg == WM_CHAR )
	{
		unsigned char asKey = (unsigned char) wparam;
		// In order - Tilde, Backspace, Enter, Escape, Ctrl+A, Ctrl+C, Ctrl+V, Ctrl+X
		if ( asKey == '`' || asKey == '\b' || asKey == '\r' || asKey == '\x1b' || asKey == InputSystem::KEYBOARD_CTRL_A || asKey == InputSystem::KEYBOARD_CTRL_C || asKey == InputSystem::KEYBOARD_CTRL_V || asKey == InputSystem::KEYBOARD_CTRL_X )
		{
			return true;
		}
		DevConsole::GetInstance()->AddCharacterToBuffer( asKey );
		return false;
	}
	return true;
}

#pragma region Command Callbacks

bool ClearCommand( Command& clearCommand )
{
	if ( clearCommand.GetName() == "clear" )
	{
		g_devConsole->GetInstance()->ClearOutput();
		return true;
	}
	return false;
}

bool SaveLogCommand( Command& saveLogCommand )
{
	if ( saveLogCommand.GetName() == "save_log" )
	{
		std::string fileName = saveLogCommand.GetNextString();
		if ( fileName == "" )
		{
			ConsolePrintf( Rgba::RED, "ERROR: save_log: No filename provided" );
			return false;
		}

		std::string outputText = DevConsole::GetInstance()->GetOutputText();
		const char* buffer = outputText.c_str();

		if( FileWriteFromBuffer( fileName.c_str(), buffer, static_cast< unsigned int >( outputText.length() ) ) )
		{
			ConsolePrintf( Rgba::GREEN, "save_log: Successfully wrote log contents to file." );
			return true;
		}
		else
		{
			ConsolePrintf( Rgba::RED, "ERROR: save_log: File write unsuccessful:." );
		}
	}
	return false;
}

bool EchoWithColorCommand( Command& echoWithColorCommand )
{
	if ( echoWithColorCommand.GetName() == "echo_with_color" )
	{
		std::string echoString = echoWithColorCommand.GetNextString();
		if ( echoString == "" )
		{
			ConsolePrintf( Rgba::RED, "ERROR: echo_with_color: No string provided" );
			return false;
		}

		std::string echoColorString = echoWithColorCommand.GetNextString();
		if ( !Rgba::IsValidString( echoColorString ) )
		{
			ConsolePrintf( Rgba::RED, "ERROR: echo_with_color: Invalid color provided; must be in format \'R,G,B,A\', where \',A\' is optional, and 0 <= RGBA <= 255" );
			return false;
		}

		Rgba echoColor = Rgba();
		echoColor.SetFromText( echoColorString );

		ConsolePrintf( echoColor, echoString.c_str() );
		return true;
	}
	return false;
}

bool ClearCommandHistoryCommand( Command& clearCommandHistoryCommand )
{
	if ( clearCommandHistoryCommand.GetName() == "clear_history" )
	{
		ClearCommandHistory();
		return true;
	}
	return false;
}

bool LogTestCommand( Command& logTestCommand )
{
	if ( logTestCommand.GetName() == "log_test" )
	{
		std::string fileName = logTestCommand.GetNextString();
		if ( fileName == "" )
		{
			ConsolePrintf( Rgba::RED, "ERROR: logTestCommand: No source filename provided" );
			return false;
		}

		std::string numThreadsString = logTestCommand.GetNextString();
		int numThreads = 0;
		if ( numThreadsString == "" )
		{
			ConsolePrintf( Rgba::RED, "ERROR: logTestCommand: Number of threads not provided" );
			return false;
		}
		try
		{
			numThreads = stoi( numThreadsString );
		}
		catch ( std::invalid_argument& invalidArgException )
		{
			UNUSED( invalidArgException );
			ConsolePrintf( Rgba::RED, "ERROR: logTestCommand: Number of threads must be an integer greater than one" );
			return false;
		}

		if ( numThreads < 1 )
		{
			ConsolePrintf( Rgba::RED, "ERROR: logTestCommand: Number of threads must be an integer greater than one" );
			return false;
		}

		const char* fileNameCString = fileName.c_str();
		LogTest( fileNameCString, numThreads );
		return true;
	}
	return false;
}

bool LogFlushTestCommand( Command& logFlushTestCommand )
{
	if ( logFlushTestCommand.GetName() == "log_flush_test" )
	{
		LogFlushTest();
		return true;
	}
	return false;
}

bool LogShowTagCommand( Command& logShowTagCommand )
{
	if ( logShowTagCommand.GetName() == "log_show_tag" )
	{
		std::string tag = logShowTagCommand.GetNextString();
		if ( tag == "" )
		{
			ConsolePrintf( Rgba::RED, "ERROR: log_show_tag: No tag provided" );
			return false;
		}

		LogShowTag( tag.c_str() );
		return true;
	}
	return false;
}

bool LogHideTagCommand( Command& logHideTagCommand )
{
	if ( logHideTagCommand.GetName() == "log_hide_tag" )
	{
		std::string tag = logHideTagCommand.GetNextString();
		if ( tag == "" )
		{
			ConsolePrintf( Rgba::RED, "ERROR: log_hide_tag: No tag provided" );
			return false;
		}

		LogHideTag( tag.c_str() );
		return true;
	}
	return false;
}

bool LogShowAllCommand( Command& logShowAllCommand )
{
	if ( logShowAllCommand.GetName() == "log_show_all" )
	{
		LogShowAll();
		return true;
	}
	return false;
}

bool LogHideAllCommand( Command& logHideAllCommand )
{
	if ( logHideAllCommand.GetName() == "log_hide_all" )
	{
		LogHideAll();
		return true;
	}
	return false;
}

bool LogTimestampCommand( Command& logTimestampCommand )
{
	if ( logTimestampCommand.GetName() == "log_timestamp" )
	{
		std::string enabledString = logTimestampCommand.GetNextString();
		if ( enabledString == "" )
		{
			ConsolePrintf( Rgba::RED, "ERROR: log_timestamp: No setting provided. Type \"enable\" or \"disable\"" );
			return false;
		}

		bool timestampEnabled = true;
		if ( enabledString == "Enabled" || enabledString == "enabled" || enabledString == "ENABLED" )
		{

		}
		else if ( enabledString == "Disabled" || enabledString == "disabled" || enabledString == "DISABLED" )
		{
			timestampEnabled = false;
		}
		else
		{
			ConsolePrintf( Rgba::RED, "ERROR: log_timestamp: Invalid setting provided. Type \"enable\" or \"disable\"" );
			return false;
		}

		LogSetTimestampEnabled( timestampEnabled );

		return true;
	}
	return false;
}

#pragma endregion

#pragma region Log Callbacks

void DevConsoleLog( const Log& log, void* )
{
	std::string fullLogString = log.GetLogString();
	ConsolePrintf( fullLogString.c_str() );
}

#pragma endregion
