#pragma once

#include <string>

struct Log
{
public:
	Log() {}
	Log( const std::string& tag, const std::string& text )
		:	m_tag( tag ),
		m_text( text )
	{

	}

	std::string GetLogString() const;

public:
	std::string m_tag = "";
	std::string m_timestamp = "";
	std::string m_text = "";
};

typedef void (*log_cb)( const Log& log, void* args );

void LogSystemStartup();
void LogSystemShutdown();

void LogFlush();

void LogPrintf( const char* format, ... );
void LogWarningf( const char* format, ... );
void LogErrorf( const char* format, ... );
void LogTaggedPrintf( const char* tag, const char* format, ... );

void LogTest( const char* srcFileName, int numThreads );	// Prints the contents of the provided file to Log.txt
void LogFlushTest();

void LogShowTag( const char* tag );
void LogHideTag( const char* tag );
void LogShowAll();
void LogHideAll();

void LogSetTimestampEnabled( bool enabled );

void LogHookRegister( log_cb callback, void* args = nullptr );
