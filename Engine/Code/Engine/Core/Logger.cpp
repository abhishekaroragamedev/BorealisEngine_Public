#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Logger.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/Threading.hpp"
#include "Engine/Core/ThreadSafeTypes.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/FileUtils/File.hpp"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <algorithm>

constexpr char LOG_DIRECTORY[] = "Log";
constexpr char LOG_FILENAME[] = "Log/Log.txt";
constexpr char LOG_TIMESTAMP_FILENAME_PREFIX[] = "Log/Log_";
constexpr char LOG_TIMESTAMP_FILENAME_SUFFIX[] = ".txt";
constexpr char LOG_FORMAT[] = "%s %s: %s";
constexpr int LOG_MAX_HISTORY = 10;

/* LOG SYSTEM DATA TYPES */
std::string Log::GetLogString() const
{
	std::string formattedString = Stringf( LOG_FORMAT, m_tag.c_str(), m_timestamp.c_str(), m_text.c_str() );
	return formattedString;
}

struct LogHook
{
public:
	LogHook() {}
	LogHook( log_cb callback, void* args = nullptr )
		:	m_callback( callback ),
			m_args( args )
	{

	}

public:
	log_cb m_callback = nullptr;
	void* m_args = nullptr;
};

/* INTERNAL FUNCTION DECLARATIONS */
static void LogThreadWorker( void* );
static void LogWriteToFileDefault( const Log& log, void* );

/* LOGGER CLASS AND INSTANCE */
class Logger
{

public:
	Logger();
	~Logger();

	void Flush();

	void Stop();
	void Resume();

	bool IsRunning() const;
	bool IsLogWhitelisted( const Log& log );

public:
	bool m_isRunning = true;
	ThreadHandle m_threadHandle = nullptr;

	ThreadSafeQueue< Log > m_logs;
	ThreadSafeVector< LogHook > m_logHooks;
	
	// If true, only tags in m_filters will be shown. If false, tags in m_filters will not be shown.
	bool m_areFiltersWhitelist = false;
	ThreadSafeSet< std::string > m_filters;

};

static Logger* g_logSystem = nullptr;
FILE* g_logFilePointer = nullptr;
FILE* g_logTimestampedFilePointer = nullptr;
bool g_shouldFlush = false;
bool g_timestampEnabled = true;

Logger::Logger()
{
	m_threadHandle = ThreadCreate( LogThreadWorker );
}

Logger::~Logger()
{
	if ( m_threadHandle != nullptr )
	{
		ThreadJoin( m_threadHandle );
		m_threadHandle = nullptr;
	}
}

void Logger::Flush()
{
	Log log;
	while ( m_logs.Dequeue( &log ) )
	{
		if ( IsLogWhitelisted( log ) )
		{
			if ( g_timestampEnabled )
			{
				log.m_timestamp = "[" + GetTimestamp() + "]";
			}

			size_t numHooks = m_logHooks.Size();
			LogHook hook;
			for ( size_t hookIndex = 0; hookIndex < numHooks; hookIndex++ )
			{
				bool success = m_logHooks.Get( hookIndex, &hook );
				if ( success )
				{
					hook.m_callback( log, hook.m_args );
				}
			}
		}
	}
}

void Logger::Stop()
{
	m_isRunning = false;
	ThreadJoin( m_threadHandle );
	m_threadHandle = nullptr;
}

void Logger::Resume()
{
	if ( m_threadHandle == nullptr )
	{
		m_threadHandle = ThreadCreate( LogThreadWorker );
	}
	m_isRunning = true;
}

bool Logger::IsRunning() const
{
	return m_isRunning;
}

bool Logger::IsLogWhitelisted( const Log& log )
{
	std::string logTag = log.m_tag;
	return (
		( !m_areFiltersWhitelist && !m_filters.Contains( logTag ) ) ||
		( m_areFiltersWhitelist && m_filters.Contains( logTag ) )
	);
}

/* INTERNAL FUNCTION DEFINITIONS */
static void LogThreadWorker( void* )
{
	while ( g_logSystem->IsRunning() )
	{
		g_logSystem->Flush();

		if ( g_shouldFlush )				// LogFlush() has notified the Logger thread that it needs to flush
		{
			FileFlush( g_logFilePointer );
			FileFlush( g_logTimestampedFilePointer );
			g_shouldFlush = false;			// Notify LogFlush() that flushing is done
		}

		ThreadSleep( 10 );
	}

	g_logSystem->Flush();
}

static void DeleteOldLogFiles()
{
	std::vector< WIN32_FIND_DATAA > fileInfo;

	WIN32_FIND_DATAA findData;
	HANDLE searchHandle = FindFirstFileA(
		"Log/Log*.txt",
		&findData
	);

	if ( searchHandle == INVALID_HANDLE_VALUE && ::GetLastError() == ERROR_FILE_NOT_FOUND )
	{
		// There are no log files
		return;
	}

	fileInfo.push_back( findData );

	while ( FindNextFileA( searchHandle, &findData ) )
	{
		fileInfo.push_back( findData );
	}

	FindClose( searchHandle );

	if ( fileInfo.size() <= static_cast< size_t >( LOG_MAX_HISTORY ) )
	{
		// Haven't reached max history yet
		return;
	}

	std::sort( fileInfo.begin(), fileInfo.end(), []( const WIN32_FIND_DATAA& a, const WIN32_FIND_DATAA& b ) {
		LONG result = CompareFileTime( &a.ftCreationTime, &b.ftCreationTime );
		return ( result > 0L );
	} );

	for ( size_t fileInfoIndex = static_cast< size_t >( LOG_MAX_HISTORY ); fileInfoIndex < fileInfo.size(); fileInfoIndex++ )
	{
		std::string filePath = std::string( LOG_DIRECTORY ) + "/" + std::string( fileInfo[ fileInfoIndex ].cFileName );
		DeleteFileA( filePath.c_str() );
	}
}

static void LogWriteToFileDefault( const Log& log, void* )
{
	if ( g_logFilePointer == nullptr )
	{
		CreateDirectoryIfNotExists( LOG_DIRECTORY );
		CreateFileIfNotExists( LOG_FILENAME );

		g_logFilePointer = OpenFile( LOG_FILENAME, "w" );
	}
	if ( g_logTimestampedFilePointer == nullptr )
	{
		std::string timeStamp = GetTimestampForFilename();
		std::string fullFilename = std::string( LOG_TIMESTAMP_FILENAME_PREFIX ) + "_" + timeStamp + std::string( LOG_TIMESTAMP_FILENAME_SUFFIX );
		g_logTimestampedFilePointer = OpenFile( fullFilename.c_str(), "w" );

		DeleteOldLogFiles();
	}

	std::string textToLog = log.GetLogString();
	textToLog += "\n";
	AppendToFile( g_logFilePointer, textToLog.c_str(), textToLog.length() );
	AppendToFile( g_logTimestampedFilePointer, textToLog.c_str(), textToLog.length() );
}

/* EXTERNAL FUNCTIONS */
void LogSystemStartup()
{
	g_logSystem = new Logger();
	LogHookRegister( LogWriteToFileDefault );
}

void LogSystemShutdown()
{
	g_logSystem->Stop();
	delete g_logSystem;
	g_logSystem = nullptr;

	if ( g_logFilePointer != nullptr )
	{
		CloseFile( g_logFilePointer );
		g_logFilePointer = nullptr;
	}
	if ( g_logTimestampedFilePointer != nullptr )
	{
		CloseFile( g_logTimestampedFilePointer );
		g_logTimestampedFilePointer = nullptr;
	}
}

void LogFlush()
{
	g_shouldFlush = true;	// Notify the Logger thread that it needs to flush
	while ( g_shouldFlush )
	{
		// Wait on the Logger thread to finish
		Sleep( 1 );
	}
}

void LogPrintf( const char* format, ... )
{
	char textLiteral[ STRINGF_STACK_LOCAL_TEMP_LENGTH ];
	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, STRINGF_STACK_LOCAL_TEMP_LENGTH, _TRUNCATE, format, variableArgumentList );	
	va_end( variableArgumentList );
	textLiteral[ STRINGF_STACK_LOCAL_TEMP_LENGTH - 1 ] = '\0';

	Log newLog = Log( "Log", textLiteral );
	g_logSystem->m_logs.Enqueue( newLog );
}

void LogWarningf( const char* format, ... )
{
	char textLiteral[ STRINGF_STACK_LOCAL_TEMP_LENGTH ];
	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, STRINGF_STACK_LOCAL_TEMP_LENGTH, _TRUNCATE, format, variableArgumentList );	
	va_end( variableArgumentList );
	textLiteral[ STRINGF_STACK_LOCAL_TEMP_LENGTH - 1 ] = '\0';

	Log newLog = Log( "WARNING", textLiteral );
	g_logSystem->m_logs.Enqueue( newLog );
}

void LogErrorf( const char* format, ... )
{
	char textLiteral[ STRINGF_STACK_LOCAL_TEMP_LENGTH ];
	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, STRINGF_STACK_LOCAL_TEMP_LENGTH, _TRUNCATE, format, variableArgumentList );	
	va_end( variableArgumentList );
	textLiteral[ STRINGF_STACK_LOCAL_TEMP_LENGTH - 1 ] = '\0';

	Log newLog = Log( "ERROR", textLiteral );
	g_logSystem->m_logs.Enqueue( newLog );
}

void LogTaggedPrintf( const char* tag, const char* format, ... )
{
	char textLiteral[ STRINGF_STACK_LOCAL_TEMP_LENGTH ];
	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, STRINGF_STACK_LOCAL_TEMP_LENGTH, _TRUNCATE, format, variableArgumentList );	
	va_end( variableArgumentList );
	textLiteral[ STRINGF_STACK_LOCAL_TEMP_LENGTH - 1 ] = '\0';

	Log newLog = Log( tag, textLiteral );
	g_logSystem->m_logs.Enqueue( newLog );
}

void LogTestWorker( void* args )
{
	const char* fileName = reinterpret_cast< const char* >( args );
	char* fileContents = reinterpret_cast<char* >( FileReadToNewBuffer( fileName ) );

	if ( fileContents == nullptr )
	{
		LogTaggedPrintf( "Invalid filename: %s", fileName );
		return;
	}

	TokenizedString tokens = TokenizedString( fileContents, "\n" );
	std::vector< std::string > lines = tokens.GetTokens();
	size_t numLines = lines.size();
	for ( int lineNumber = 0; lineNumber < numLines; lineNumber++ )
	{
		std::string line = lines[ lineNumber ];
		const char* lineCStr = line.c_str();
		LogTaggedPrintf( "LogTest", "[%ld,%d] %s", ::GetCurrentThreadId(), lineNumber, lineCStr );
	}

	delete[] fileName;
	free( fileContents );
}

void LogTest( const char* srcFileName, int numThreads )
{
	// Create a non-const version of the filename to pass to the thread
	size_t fileNameLength = strlen( srcFileName );

	while( numThreads > 0 )
	{
		char* fileNameMutableCopy = new char[ fileNameLength + 1 ];
		memcpy( fileNameMutableCopy, srcFileName, fileNameLength );
		fileNameMutableCopy[ fileNameLength ] = '\0';

		ThreadCreateAndDetach( LogTestWorker, fileNameMutableCopy );
		numThreads--;
	}
}

void LogFlushTest()
{
	LogTaggedPrintf( "LogFlushTest", "You will hit a breakpoint!" );
	LogFlush();

	DebugBreak();
}

void LogShowTag( const char* tag )
{
	std::string tagString = std::string( tag );
	if ( g_logSystem->m_areFiltersWhitelist )
	{
		g_logSystem->m_filters.Insert( tagString );
	}
	else
	{
		g_logSystem->m_filters.Erase( tagString );
	}
}

void LogHideTag( const char* tag )
{
	std::string tagString = std::string( tag );
	if ( g_logSystem->m_areFiltersWhitelist )
	{
		g_logSystem->m_filters.Erase( tagString );
	}
	else
	{
		g_logSystem->m_filters.Insert( tagString );
	}
}

void LogShowAll()
{
	g_logSystem->m_areFiltersWhitelist = false;
	g_logSystem->m_filters.Clear();
}

void LogHideAll()
{
	g_logSystem->m_areFiltersWhitelist = true;
	g_logSystem->m_filters.Clear();
}

void LogSetTimestampEnabled( bool enabled )
{
	g_timestampEnabled = enabled;
}

void LogHookRegister( log_cb callback, void* args /*= nullptr*/ )
{
	LogHook hookToAdd = LogHook( callback, args );
	g_logSystem->m_logHooks.Push( hookToAdd );	// Blocking call
}
