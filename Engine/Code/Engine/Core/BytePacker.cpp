#include "Engine/Core/BytePacker.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <cstdlib>
#include <string>

BytePacker::BytePacker( Endianness byteOrder /* = Endianness::LITTLE_ENDIAN */, unsigned int options /* = 3U */ )
	:	m_endianness( byteOrder ),
		m_options( options )
{
	/*
	if ( OwnsMemory() )
	{
		// Don't know how much, though!
	}
	*/
}

BytePacker::BytePacker( size_t bufferSize, Endianness byteOrder /* = Endianness::LITTLE_ENDIAN */, unsigned int options /* = 3U */ )
	:	m_endianness( byteOrder ),
		m_bufferSize( bufferSize ),
		m_options( options )
{
	if ( OwnsMemory() )
	{
		m_buffer = reinterpret_cast< char* >( malloc( m_bufferSize ) );
		m_readHead = m_buffer;
		m_writeHead = m_buffer;
	}
}

BytePacker::BytePacker( size_t bufferSize, void* buffer, Endianness byteOrder /* = Endianness::LITTLE_ENDIAN */, unsigned int options /* = 3U */ )
	:	m_endianness( byteOrder ),
		m_bufferSize( bufferSize ),
		m_buffer( reinterpret_cast< char* >( buffer ) ),
		m_options( options )
{
	m_readHead = m_buffer;
	m_writeHead = m_buffer;
}

BytePacker::~BytePacker()
{
	if ( OwnsMemory() && m_buffer != NULL )
	{
		free( (void*) m_buffer );
		m_buffer = NULL;
		m_bufferSize = 0;
		m_readHead = NULL;
		m_writeHead = NULL;
	}
}

void BytePacker::TryInitAndGrow( size_t sizeToGrow )
{
	if ( !OwnsMemory() )
	{
		return;
	}

	if ( m_buffer == NULL )
	{
		m_bufferSize = sizeToGrow;
		m_buffer = reinterpret_cast< char* >( malloc( m_bufferSize ) );
		memset( m_buffer, 0, m_bufferSize );

		m_writeHead = m_buffer;
		m_readHead = m_buffer;
	}
	else if ( CanGrow() && ( ( m_buffer + m_bufferSize ) - m_writeHead ) < static_cast< int >( sizeToGrow ) )
	{
		Grow( sizeToGrow );
	}
}

void BytePacker::Grow( size_t sizeToGrow )
{
	if ( CanGrow() )
	{
		char* newMemory = reinterpret_cast< char* >( malloc( m_bufferSize + sizeToGrow ) );
		memset( newMemory, 0, m_bufferSize + sizeToGrow );

		size_t writeHeadDistance = 0;
		size_t readHeadDistance = 0;

		if ( m_buffer != NULL )
		{
			writeHeadDistance = m_writeHead - m_buffer;
			readHeadDistance = m_readHead - m_buffer;

			memcpy( newMemory, m_buffer, writeHeadDistance );
			free( ( void* ) m_buffer );
			m_buffer = NULL;
		}
		else
		{
			m_bufferSize = 0;
		}

		m_bufferSize += sizeToGrow;
		m_buffer = newMemory;

		m_writeHead = m_buffer + writeHeadDistance;
		m_readHead = m_buffer + readHeadDistance;
	}
}

void BytePacker::Clear()
{
	if ( m_buffer != NULL )
	{
		free( (void*) m_buffer );
		m_buffer = NULL;
	}

	m_bufferSize = 0;
	m_readHead = NULL;
	m_writeHead = NULL;
}

bool BytePacker::WriteBytes( size_t byteCount, const void* data )
{
	TryInitAndGrow( byteCount );

	size_t writableSize = GetWritableByteCount();
	size_t sizeWritten = Min( writableSize, byteCount );
	if ( sizeWritten < byteCount )
	{
		return false;
	}

	memcpy( m_writeHead, data, sizeWritten );
	ToEndianness( sizeWritten, m_writeHead, m_endianness );

	AdvanceWriteHead( sizeWritten );

	return true;
}

size_t BytePacker::ReadBytes( void* out_data, size_t maxByteCount )
{
	size_t readableSize = GetReadableByteCount();
	size_t sizeRead = Min( readableSize, maxByteCount );
	memcpy( out_data, m_readHead, sizeRead );
	FromEndianness( sizeRead, out_data, m_endianness );

	AdvanceReadHead( sizeRead );

	return readableSize;
}

size_t BytePacker::WriteSize( size_t size )
{
	size_t totalWritten = 0;

	char byteMask = static_cast< char >( 127 );	// 7 1s
	char byteToWrite = static_cast< char >( 0 );
	while ( size > 0 )
	{
		byteToWrite = size & byteMask;
		size >>= 7;
		byteToWrite |= ( size > 0 )? 128 : 0;	// Make the most significant bit either 1 or 0 depending on whether more needs to be written or not

		size_t writableSize = GetWritableByteCount();
		size_t availableBytes = m_bufferSize - GetWrittenByteCount();
		if ( availableBytes < 1 )
		{
			TryInitAndGrow( 1 );
		}
			
		memcpy( m_writeHead, &byteToWrite, 1 );	// Writing will be independent of endianness, so no need to 
		AdvanceWriteHead( 1 );

		totalWritten += 1;
	}

	return totalWritten;
}

size_t BytePacker::ReadSize( size_t* out_size )
{
	size_t totalRead = 0;
	char shouldRead = static_cast< char >( 1 );
	char byteRead;
	char byteMask = static_cast< char >( 127 );	// 7 1s
	int numBytesRead = 0;
	size_t value = 0;

	while ( shouldRead )
	{
		size_t readableSize = GetReadableByteCount();
		size_t sizeRead = Min( readableSize, sizeof( byteRead ) );
		memcpy( &byteRead, m_readHead, sizeRead );

		char newBytes = byteRead & byteMask;
		size_t valueRead = newBytes;
		valueRead <<= numBytesRead;
		value |= valueRead;

		AdvanceReadHead( sizeRead );
		totalRead += sizeRead;
		numBytesRead++;
		shouldRead = byteRead & ( ~byteMask );	// Most significant bit contains a one if there are more bytes
	}

	*out_size = value;

	return totalRead;
}

bool BytePacker::WriteString( const char* str )
{
	WriteSize( strlen( str ) );

	TryInitAndGrow( strlen( str ) );

	size_t writableSize = GetWritableByteCount();
	size_t totalStrLength = strlen( str );
	size_t sizeWritten = Min( writableSize, totalStrLength );
	if ( sizeWritten < totalStrLength )
	{
		return false;
	}

	memcpy( m_writeHead, str, sizeWritten );

	AdvanceWriteHead( sizeWritten );

	return true;
}

size_t BytePacker::ReadString( char* out_str, size_t maxStrSize )
{
	size_t strSize = 0;
	ReadSize( &strSize );
	size_t sizeRead = Min( strSize, maxStrSize );
	memcpy( out_str, m_readHead, sizeRead );

	AdvanceReadHead( sizeRead );

	return sizeRead;
}

void BytePacker::AdvanceWriteHead( size_t amount )
{
	m_writeHead += amount;
}

void BytePacker::AdvanceReadHead( size_t amount )
{
	m_readHead += amount;
}

char* BytePacker::GetWriteHead() const
{
	return m_writeHead;
}

char* BytePacker::GetReadHead() const
{
	return m_readHead;
}

/* SETTERS */
void BytePacker::SetEndianness( Endianness endianness )
{
	if ( endianness != m_endianness )
	{
		m_endianness = endianness;

		if ( m_bufferSize > 0 )
		{
			ToEndianness( m_bufferSize, m_buffer, m_endianness );
		}
	}
}

void BytePacker::ResetWrite()
{
	m_writeHead = m_buffer;
}

void BytePacker::ResetRead()
{
	m_readHead = m_buffer;
}

/* GETTERS */
Endianness BytePacker::GetEndianness() const
{
	return m_endianness;
}

bool BytePacker::CanGrow() const
{
	return ( m_options | BYTEPACKER_OWNS_MEMORY && m_options | BYTEPACKER_CAN_GROW );
}

bool BytePacker::OwnsMemory() const
{
	return ( m_options | BYTEPACKER_OWNS_MEMORY );
}

size_t BytePacker::GetWrittenByteCount() const
{
	if ( m_buffer != NULL && m_writeHead != NULL )
	{
		return m_writeHead - m_buffer;	// How far ahead the write head is from the start of the buffer
	}

	return 0;
}

size_t BytePacker::GetWritableByteCount() const
{
	if ( m_buffer != NULL && m_writeHead != NULL )
	{
		if ( CanGrow() )
		{
			return SIZE_MAX;
		}

		size_t writtenBytes = GetWrittenByteCount();
		return ( m_bufferSize - writtenBytes );
	}

	return 0;
}

size_t BytePacker::GetReadByteCount() const
{
	if ( m_buffer != NULL && m_readHead != NULL )
	{
		return m_readHead - m_buffer;	// How far ahead the write head is from the start of the buffer
	}

	return 0;
}

size_t BytePacker::GetReadableByteCount() const
{
	if ( m_buffer != NULL && m_readHead != NULL )
	{
		size_t readBytes = GetReadByteCount();
		return ( m_bufferSize - readBytes );
	}

	return 0;
}
