#pragma once

#include "Engine/Core/Endianness.hpp"
#include <stdint.h>

#define BIT_SHIFT( x ) 1<<x

enum BytePackerOptionBit : unsigned int
{
	BYTEPACKER_OWNS_MEMORY	= BIT_SHIFT( 0 ),
	BYTEPACKER_CAN_GROW		= BIT_SHIFT( 1 )
};
typedef unsigned int BytePackerOptions; 

class BytePacker 
{

public:
	BytePacker( Endianness byteOrder = Endianness::LITTLE_ENDIAN, unsigned int options = 3U );
	BytePacker( size_t bufferSize, Endianness byteOrder = Endianness::LITTLE_ENDIAN, unsigned int options = 3U );
	BytePacker( size_t bufferSize, void* buffer, Endianness byteOrder = Endianness::LITTLE_ENDIAN, unsigned int options = 3U );
	virtual ~BytePacker();

	template <typename T>
	bool Peek( T* out_size ) const
	{
		size_t readableSize = GetReadableByteCount();
		size_t neededSize = sizeof( T );
		size_t readSize = Min( neededSize, readableSize );

		if ( readSize < neededSize )
		{
			return false;
		}

		memcpy( out_size, m_readHead, readSize );
		return true;
	}

	virtual bool WriteBytes( size_t byteCount, const void* data );	// Appends
	virtual size_t ReadBytes( void* out_data, size_t maxByteCount ); 

	virtual size_t WriteSize( size_t size );
	virtual size_t ReadSize( size_t* out_size );

	virtual bool WriteString( const char* str ); 
	virtual size_t ReadString( char* out_str, size_t maxStrSize );	// max_str_size should be enough to contain the null terminator as well; 

	void AdvanceWriteHead( size_t amount );
	void AdvanceReadHead( size_t amount );

	char* GetWriteHead() const;
	char* GetReadHead() const;

	void SetEndianness( Endianness endianness );
	// bool SetReadableByteCount( size_t byteCount );
	void ResetWrite();	// Resets writing to the beginning of the buffer.  Make sure read head stays valid (<= write_head)
	void ResetRead();	// Resets reading to the beginning of the buffer

	Endianness GetEndianness() const;
	bool CanGrow() const;
	bool OwnsMemory() const;
	size_t GetWrittenByteCount() const;		// How much have I written to this buffer
	size_t GetWritableByteCount() const;	// How much more can I write to this buffer (if growable, this returns UINFINITY)
	size_t GetReadByteCount() const;
	size_t GetReadableByteCount() const;	// How much more data can I read;

	void Grow( size_t sizeToGrow );
	void TryInitAndGrow( size_t sizeToGrow );
	void Clear();

public:
	char* m_buffer = NULL;
	size_t m_bufferSize = 0;
	Endianness m_endianness = Endianness::LITTLE_ENDIAN;
	unsigned int m_options;

protected:
	char* m_readHead = NULL;
	char* m_writeHead = NULL;

};
