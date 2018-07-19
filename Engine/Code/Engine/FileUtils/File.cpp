#include "Engine/Core/EngineCommon.hpp"
#include "Engine/FileUtils/File.hpp"
#include "Engine/Tools/DevConsole.hpp"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <cstdlib>

bool CreateDirectoryIfNotExists( const char* directoryPath )
{
	bool success = CreateDirectoryA( directoryPath, NULL );
	success = success || ( GetLastError() == ERROR_ALREADY_EXISTS );
	return success;
}

bool CreateFileIfNotExists( const char* filePath )
{
	void* handle = CreateFileA( filePath,
		( GENERIC_READ | GENERIC_WRITE ),
		( FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE ),
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);
	
	bool success = ( handle != INVALID_HANDLE_VALUE ) || ( GetLastError() == ERROR_ALREADY_EXISTS );
	if ( success )
	{
		CloseHandle( handle );
	}

	return success;
}

FILE* OpenFile( const char* fileName, const char* mode )
{
	FILE *filePointer = nullptr;
	fopen_s( &filePointer, fileName, mode ); 
	return filePointer;
}

size_t AppendToFile( FILE* filePointer, const char* buffer, size_t bufferLength )
{
	fseek( filePointer, 0L, SEEK_END );
	size_t numSuccess = fwrite( buffer, sizeof( char ), bufferLength, filePointer );
	return numSuccess;
}

void CloseFile( FILE* filePointer )
{
	fclose( filePointer );
}

void* FileReadToNewBuffer( char const *fileName )
{
	FILE *filePointer = nullptr;
	fopen_s( &filePointer, fileName, "r" ); 
	if ( filePointer == nullptr )
	{
		return nullptr; 
	}
	
	size_t size = 0U; 
	fseek( filePointer, 0L, SEEK_END );
	size = ftell( filePointer );
	
	fseek( filePointer, 0L, SEEK_SET ); 
	
	byte_t* buffer = ( byte_t* ) malloc( size + 1U ); 
	
	size_t read = fread( buffer, 1, size, filePointer );
	fclose( filePointer );
	
	buffer[ read ] = NULL;

	return buffer;  
}

bool FileWriteFromBuffer( const char* fileName, const char* buffer, unsigned int bufferLength )
{
	FILE *filePointer = nullptr;
	fopen_s( &filePointer, fileName, "w" );

	if ( filePointer == nullptr )
	{
		ConsolePrintf( Rgba::RED, "ERROR: FileWriteFromBuffer: Failed to open file %s for write.", fileName );
		return false;
	}

	size_t numSuccess = fwrite( buffer, sizeof( char ), bufferLength, filePointer );
	fclose( filePointer );

	if( numSuccess == bufferLength )
	{
		ConsolePrintf( Rgba::GREEN, "FileWriteFromBuffer: Successfully wrote to file %s.", fileName );
		return true;
	}
	else
	{
		ConsolePrintf( Rgba::RED, "ERROR: FileWriteFromBuffer: Failed to write correct length to file %s.", fileName );
		return false;
	}
}

bool FileFlush( FILE* filePointer )
{
	int result = fflush( filePointer );
	bool success = result == 0;
	return success;
}

std::string GetDirectoryFromFilePath( const char* filePath )
{
	std::string directory = std::string( filePath );
	directory = directory.substr( 0, directory.find_last_of( "/" ) );
	return directory;
}
