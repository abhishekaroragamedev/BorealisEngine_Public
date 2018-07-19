#pragma once

#include "Engine/Core/StringUtils.hpp"
#include <string>
#include <vector>

// A command is a single submitted commmand
// NOT the definition
// Comments will be using a Command constructed as follows; 
// Command cmd = Command( "echo_with_color (255,255,0) \"Hello \\"World\\"\" ); 
class Command
{

public:
	Command( const char* str ); 
	~Command();

	std::string GetName() const; // would return "echo_with_color"

						   // Gets the next string in the argument list.
						   // Breaks on whitespace.  Quoted strings should be treated as a single return 

	std::string GetNextString();   // would return after each call...
								   // first:  "(255,255,0)""
								   // second: "Hello \"world\""
								   // third+: ""

								   // [OPTIONAL] I like to add helpers for getting arguments
								   // and I just add them as I need them.
								   // Each takes the output variable, and returns whether it succeeded
								   // bool GetNextInt( int *out_val ); 
								   // bool GetNextColor( RGBA *out_val );
								   // bool GetNextVector2( Vector2 *out_val ); 
								   // bool GetNextVector3( Vector3 *out_val );
								   // ... 

private:
	std::string m_name = "";
	TokenizedString m_argumentTokens;
	int m_currentArgumentIndex = 0;
};

// Command callbacks take a Command.
typedef bool (*command_cb)( Command& cmd );

// Allows for setting up the system and cleaning up.
// Optional, but may help with some tasks. 
void CommandStartup();
void CommandShutdown(); 

// Registers a command with the system
// Example, say we had a global function named...
//    void Help( Command &cmd ) { /* ... */ }  
// 
// We then, during some startup, call
//    CommandRegister( "help", Help ); 
void CommandRegister( const char* name, command_cb cb, const char* description = "" ); 
void CommandUnregister( const char* name );

// Will construct a Command object locally, and if 
// a callback is associated with its name, will call it and 
// return true, otherwise returns false.
// Name is case-insensitive
bool CommandRun( const char* command ); 

std::vector< std::string > GetCommandHistory(); 
void ClearCommandHistory();

bool HelpCommand( Command& command );