#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Threading.hpp"
#include "Engine/Tools/DevConsole.hpp"
#include <process.h>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

/* INTERNAL DATA */
#define MS_VC_EXCEPTION (0x406d1388)
#define THREAD_DEFAULT_STACK_SIZE_BYTES 1000000
#define THREAD_INVALID_ID 0

struct ThreadArgs
{
public:
	ThreadArgs( const char* name, thread_cb callback, void* args = nullptr )
		:	m_name( name ),
		m_callback( callback ),
		m_args( args )
	{

	}

public:
	const char* m_name = nullptr;
	thread_cb m_callback;
	void* m_args = nullptr;
};

#pragma pack( push, 8 )	// Places the struct's members directly after each other in memory
	struct ThreadNameInfo
	{
	public:
		DWORD m_type = 0x1000;			// Mandatory value
		const char* m_name = nullptr;
		DWORD m_threadID = 0;
		DWORD m_flags = 0;
	};
#pragma pack( pop )

/* INTERNAL FUNCTIONS */

// Master function that is used by Windows to initialize a thread; we invoke our own callback with the passed args for the thread to execute
static DWORD WINAPI ThreadEntryPoint( void* args )	// Will delete the passed structure
{
	ThreadArgs* threadArgs = reinterpret_cast< ThreadArgs* >( args );
	ThreadSetName( threadArgs->m_name );

	thread_cb callback = threadArgs->m_callback;
	void* passArgs = threadArgs->m_args;

	callback( passArgs );
	delete threadArgs;
	return 0;
}

unsigned long ThreadGetCurrentID()
{
	return static_cast< unsigned long >( ::GetCurrentThreadId() );
}

unsigned long ThreadGetID( ThreadHandle handle )
{
	return static_cast< unsigned long >( ::GetThreadId( handle ) );
}

/* EXTERNAL FUNCTION DEFINITIONS */

ThreadHandle ThreadCreate( thread_cb callback, void* data /*= nullptr*/, const char* name /*=nullptr*/ )
{
	ThreadArgs* threadArgs = new ThreadArgs(
		name,
		callback,
		data
	);

	size_t stackSize = THREAD_DEFAULT_STACK_SIZE_BYTES;

	DWORD threadId = 0;
	ThreadHandle handle = reinterpret_cast< ThreadHandle >(
		::CreateThread(
			NULL,
			stackSize,
			ThreadEntryPoint,
			threadArgs,
			0,
			&threadId
		)
	);

	if ( handle == NULL )
	{
		DWORD errorCode = ::GetLastError();
		DebuggerPrintf( "ERROR: ThreadCreate(): Failed to create thread with error code %lu.", errorCode );
		ConsolePrintf( Rgba::RED, "ERROR: ThreadCreate(): Failed to create thread with error code %lu.", errorCode );
	}

	return handle;
}

void ThreadCreateAndDetach( thread_cb callback, void* data /* = nullptr */, const char* name /* = nullptr */ )
{
	ThreadHandle handle = ThreadCreate( callback, data, name );
	if ( handle == NULL )
	{
		DebuggerPrintf( "ERROR: ThreadCreateAndDetach(): Failed to create thread. See last error from ThreadCreate()." );
		ConsolePrintf( Rgba::RED, "ERROR: ThreadCreateAndDetach(): Failed to create thread. See last error from ThreadCreate()." );
	}
	ThreadDetach( handle );
}

void ThreadJoin( ThreadHandle handle )
{
	if ( handle != NULL )
	{
		::WaitForSingleObject( handle, INFINITE );	// Wait for the thread to finish, with an infinite timeout
		::CloseHandle( handle );
	}
	else
	{
		DebuggerPrintf( "ERROR: ThreadJoin(): Failed to join thread - invalid handle." );
		ConsolePrintf( Rgba::RED, "ERROR: ThreadJoin(): Failed to create thread - invalid handle." );
	}
}

void ThreadDetach( ThreadHandle handle )
{
	if ( handle != NULL )
	{
		::CloseHandle( handle );
	}
	else
	{
		DebuggerPrintf( "ERROR: ThreadDetach(): Failed to detach thread - invalid handle." );
		ConsolePrintf( Rgba::RED, "ERROR: ThreadDetach(): Failed to detach thread - invalid handle." );
	}
}

void ThreadSleep( unsigned int milliseconds )
{
	DWORD millisecondsAsDword = static_cast< DWORD >( milliseconds );
	::Sleep( millisecondsAsDword );	// Gives up the current thread's time slice for the minimum time specified
}

void ThreadYield()
{
	::SwitchToThread();				// Allows other threads that are ready to run, if any, on the current processor
}

void ThreadSetName( const char* name )
{
	if ( name == nullptr )
	{
		return;
	}

	unsigned long threadID = ThreadGetCurrentID();
	if ( threadID != NULL )
	{
		ThreadNameInfo info;
		info.m_type = 0x1000;
		info.m_name = name;
		info.m_threadID = ( DWORD ) threadID;
		info.m_flags = 0;

		// This is a Visual Studio-specific way to get the thread name to display
		__try
		{
			RaiseException(
				MS_VC_EXCEPTION,
				0,
				( sizeof( info ) / sizeof( ULONG_PTR ) ),
				reinterpret_cast< ULONG_PTR* >( &info )
			);
		}
		__except( EXCEPTION_CONTINUE_EXECUTION )
		{

		}
	}
	else
	{
		DebuggerPrintf( "ERROR: ThreadSetName(): Could not get current thread ID." );
		ConsolePrintf( Rgba::RED, "ERROR: ThreadSetName(): Could not get current thread ID." );
	}
}
