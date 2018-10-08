// Loads a file to a memory buffer
// Free using free(ptr); 
#include <stdio.h>
#include <string>

bool CreateDirectoryIfNotExists( const char* directoryPath );	// Returns true if creation was successful or if directory already exists
bool CreateFileIfNotExists( const char* filePath );				// Returns true if creation was successful or if file already exists

FILE* OpenFile( const char* fileName, const char* mode );
size_t AppendToFile( FILE* filePointer, const char* buffer, size_t bufferLength );
void CloseFile( FILE* filePointer );

void* FileReadToNewBuffer( const char* fileName );
bool FileWriteFromBuffer( const char* fileName, const char* buffer, unsigned int bufferLength );

bool FileFlush( FILE* filePointer );

std::string GetDirectoryFromFilePath( const char* filePath );
