#pragma once

/* DATA */
struct ThreadHandleType;				// Just a void
typedef ThreadHandleType* ThreadHandle;	// Just a void* with a purpose - only to be used with Thread Management

typedef void ( *thread_cb )( void* data );

/* FUNCTIONS */
ThreadHandle ThreadCreate( thread_cb callback, void* data = nullptr, const char* name = nullptr );
void ThreadCreateAndDetach( thread_cb callback, void* data = nullptr, const char* name = nullptr );

void ThreadJoin( ThreadHandle handle );
void ThreadDetach( ThreadHandle handle );

void ThreadSleep( unsigned int milliseconds );
void ThreadYield();

void ThreadSetName( const char* name );	// For Debug
