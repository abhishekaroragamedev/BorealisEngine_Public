#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/FileUtils/File.hpp"
#include "Engine/Tools/Command.hpp"
#include "Engine/Tools/DevConsole.hpp"

constexpr int NUM_COMMAND_DEFINITIONS = 128;
constexpr int NUM_COMMANDS_IN_HISTORY = 32;
constexpr char COMMAND_DEFAULT_DESCRIPTION[] = "No description provided.";
constexpr char COMMAND_HISTORY_FILENAME[] = "cmdhistory.txt";

class CommandDefinition
{

public:
	std::string m_name = "";
	std::string m_description = "";
	command_cb m_callback = nullptr;

	void operator=( const CommandDefinition& comparedCommand )
	{
		m_name = comparedCommand.m_name;
		m_description = comparedCommand.m_description;
		m_callback = comparedCommand.m_callback;
	}

	void Clear()
	{
		m_name = "";
		m_description = std::string( COMMAND_DEFAULT_DESCRIPTION );
		m_callback = nullptr;
	}

	bool IsRegistered()
	{
		return ( m_callback != nullptr );
	}

};

CommandDefinition g_Definitions[ NUM_COMMAND_DEFINITIONS ];
int g_definitionCount = 0;

std::vector< std::string > g_commandHistory;
void AddToCommandHistory( const char* command );
void AddToCommandHistory( const std::string& command );
void TruncateHistoryIfLimitExceeded();
void RemoveDuplicateCommands( const std::string& commandStr );
void LoadCommandHistoryFromFile();
void WriteCommandHistoryToFile();

#pragma region Command Class Functions

Command::Command( const char* str )
	:	m_argumentTokens( std::string( str ), " ", true )
{
	std::vector< std::string > argumentTokenVector = m_argumentTokens.GetTokens();
	m_name = argumentTokenVector[ 0 ];
	m_currentArgumentIndex = 1;
}

Command::~Command()
{

}

std::string Command::GetName() const
{
	return m_name;
}

std::string Command::GetNextString()
{
	if ( ( (size_t)m_currentArgumentIndex ) < m_argumentTokens.GetTokens().size() )
	{
		return m_argumentTokens.GetTokens()[ m_currentArgumentIndex++ ];
	}

	return "";
}

#pragma endregion

#pragma region Command Helper Functions

void CommandStartup()
{
	LoadCommandHistoryFromFile();
}

void CommandShutdown()
{
	WriteCommandHistoryToFile();
}

void CommandRegister( const char* name, command_cb cb, const char* description /*= ""*/ )
{
	g_Definitions[ g_definitionCount ].m_name = std::string( name );
	g_Definitions[ g_definitionCount ].m_description = std::string( description );
	g_Definitions[ g_definitionCount ].m_callback = cb;
	g_definitionCount++;
}

void CommandUnregister( const char* name )
{
	std::string nameAsString = std::string( name );
	if ( nameAsString.size() == 0 )
	{
		return;
	}

	for ( int commandIndex = 0; commandIndex < g_definitionCount; commandIndex++ )
	{
		if ( nameAsString == g_Definitions[ commandIndex ].m_name )
		{
			g_Definitions[ commandIndex ] = g_Definitions[ g_definitionCount - 1 ];
			g_Definitions[ g_definitionCount - 1 ].Clear();
			g_definitionCount--;
		}
	}
}

bool CommandRun( const char* command )
{
	Command commandInstance = Command( command );
	AddToCommandHistory( command );

	for ( int commandIndex = 0; commandIndex < g_definitionCount; commandIndex++ )
	{
		if ( commandInstance.GetName() == g_Definitions[ commandIndex ].m_name )
		{
			return g_Definitions[ commandIndex ].m_callback( commandInstance );
		}
	}

	ConsolePrintf( Rgba::RED, "ERROR: Invalid command entered. Enter \"help\" to view available commands." );
	return false;
}

#pragma endregion

#pragma region Command History Functions

std::vector< std::string > GetCommandHistory()
{
	return g_commandHistory;
}

void AddToCommandHistory( const char* command )
{
	std::string commandStr = std::string( command );
	RemoveDuplicateCommands( commandStr );
	TruncateHistoryIfLimitExceeded();
	g_commandHistory.insert( g_commandHistory.begin(), commandStr );
}

void AddToCommandHistory( const std::string& command )
{
	RemoveDuplicateCommands( command );
	TruncateHistoryIfLimitExceeded();
	g_commandHistory.insert( g_commandHistory.begin(), command );
}

void RemoveDuplicateCommands( const std::string& commandStr )
{
	for ( int commandIndex = ( int )( g_commandHistory.size() - 1 ); commandIndex >= 0; commandIndex-- )
	{
		if ( commandStr == g_commandHistory[ commandIndex ] )
		{
			g_commandHistory.erase( g_commandHistory.begin() + commandIndex );
		}
	}
}

void TruncateHistoryIfLimitExceeded()
{
	for ( int commandIndex = ( int )( g_commandHistory.size() - 1 ); commandIndex >= ( NUM_COMMANDS_IN_HISTORY - 1 ); commandIndex-- )
	{
		g_commandHistory.erase( g_commandHistory.begin() + commandIndex );
	}
}

void LoadCommandHistoryFromFile()
{
	const char* fileContents = reinterpret_cast< const char* >( FileReadToNewBuffer( COMMAND_HISTORY_FILENAME ) );

	if ( fileContents != nullptr )
	{
		g_commandHistory.clear();

		std::string fileContentsAsString = std::string( fileContents );
		unsigned int currentIndex = 0;
		int currentNumCommands = 0;
		while ( ( currentIndex < fileContentsAsString.length() ) && ( currentNumCommands < NUM_COMMANDS_IN_HISTORY ) )
		{
			std::string currentLine = fileContentsAsString.substr( currentIndex, ( fileContentsAsString.find_first_of( '\n', currentIndex ) - currentIndex ) );
			
			if ( currentLine == "" )
			{
				currentIndex++;
				continue;
			}
			
			AddToCommandHistory( currentLine );
			currentIndex += static_cast< unsigned int >( currentLine.length() );
			currentNumCommands++;
		}

		std::reverse( g_commandHistory.begin(), g_commandHistory.end() );
	}
}

void WriteCommandHistoryToFile()
{
	if ( g_commandHistory.size() > 0 )
	{
		std::string cumulativeBuffer = "";
		for ( std::string command : g_commandHistory )
		{
			if ( command != "" )
			{
				cumulativeBuffer += command + "\n";
			}
		}
		cumulativeBuffer[ cumulativeBuffer.length() - 1 ] = NULL;
		const char* bufferToWrite = cumulativeBuffer.c_str();
		FileWriteFromBuffer( COMMAND_HISTORY_FILENAME, bufferToWrite, static_cast< unsigned int >( cumulativeBuffer.length() ) );
	}
}

void ClearCommandHistory()
{
	g_commandHistory.clear();
}

bool HelpCommand( Command& helpCommand )
{
	if ( helpCommand.GetName() == "help" )
	{
		ConsolePrintf( "Available commands:" );
		
		for ( CommandDefinition commandDefinition : g_Definitions )
		{
			if (commandDefinition.IsRegistered() )
			{
				std::string commandDescription = commandDefinition.m_description;
				if ( commandDescription == "" )
				{
					commandDescription = COMMAND_DEFAULT_DESCRIPTION;
				}

				ConsolePrintf( ( commandDefinition.m_name + ": " + commandDescription ).c_str() );
			}
		}

		ConsolePrintf( "Supported controls:" );
		ConsolePrintf( "~ - Open/Close console" );
		ConsolePrintf( "ENTER - Runs command" );
		ConsolePrintf( "ESC - Cancels selection/clears input window/resets history index/closes console" );
		ConsolePrintf( "LEFT/RIGHT Arrows - Navigate through input text" );
		ConsolePrintf( "BACKSPACE - Delete selection/previous character" );
		ConsolePrintf( "DELETE - Delete selection/next character" );
		ConsolePrintf( "SHIFT + LEFT/RIGHT Arrows - Select text" );
		ConsolePrintf( "UP/DOWN Arrows - Browse command history" );
		ConsolePrintf( "PAGE UP/PAGE DOWN - Scroll through output window" );
		ConsolePrintf( "CTRL + A - Select all input text" );
		ConsolePrintf( "CTRL + C - Copy" );
		ConsolePrintf( "CTRL + V - Paste" );
		ConsolePrintf( "CTRL + X - Cut" );
		ConsolePrintf( "F5 - Screenshot" );
		ConsolePrintf( "F9 - Reload Shaders" );
		ConsolePrintf( "F11 - Toggle master clock pause state" );
		ConsolePrintf( "[ ] - Run through built-in time scale hotkey values" );
		return true;
	}
	return false;
}

#pragma endregion
