#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/IntRange.hpp"
#include "Engine/Tools/DevConsole.hpp"
#include <stdarg.h>


//-----------------------------------------------------------------------------------------------
const std::string Stringf( const char* format, ... )
{
	char textLiteral[ STRINGF_STACK_LOCAL_TEMP_LENGTH ];
	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, STRINGF_STACK_LOCAL_TEMP_LENGTH, _TRUNCATE, format, variableArgumentList );	
	va_end( variableArgumentList );
	textLiteral[ STRINGF_STACK_LOCAL_TEMP_LENGTH - 1 ] = '\0';

	return std::string( textLiteral );
}

const std::string Stringv( const char* format, va_list args )
{
	char textLiteral[ STRINGF_STACK_LOCAL_TEMP_LENGTH ];
	vsnprintf_s( textLiteral, STRINGF_STACK_LOCAL_TEMP_LENGTH, _TRUNCATE, format, args );	
	textLiteral[ STRINGF_STACK_LOCAL_TEMP_LENGTH - 1 ] = '\0';

	return std::string( textLiteral );
}

//-----------------------------------------------------------------------------------------------
const std::string Stringf( const int maxLength, const char* format, ... )
{
	char textLiteralSmall[ STRINGF_STACK_LOCAL_TEMP_LENGTH ];
	char* textLiteral = textLiteralSmall;
	if( maxLength > STRINGF_STACK_LOCAL_TEMP_LENGTH )
		textLiteral = new char[ maxLength ];

	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, maxLength, _TRUNCATE, format, variableArgumentList );	
	va_end( variableArgumentList );
	textLiteral[ maxLength - 1 ] = '\0';

	std::string returnValue( textLiteral );
	if( maxLength > STRINGF_STACK_LOCAL_TEMP_LENGTH )
	{
		delete[] textLiteral;
	}

	return returnValue;
}

const std::string Stringv( const int maxLength, const char* format, va_list args )
{
	char textLiteralSmall[ STRINGF_STACK_LOCAL_TEMP_LENGTH ];
	char* textLiteral = textLiteralSmall;
	if( maxLength > STRINGF_STACK_LOCAL_TEMP_LENGTH )
	{
		textLiteral = new char[ maxLength ];
	}

	vsnprintf_s( textLiteral, maxLength, _TRUNCATE, format, args );

	textLiteral[ maxLength - 1 ] = '\0';

	std::string returnValue( textLiteral );
	if( maxLength > STRINGF_STACK_LOCAL_TEMP_LENGTH )
	{
		delete[] textLiteral;
	}

	return returnValue;
}

TokenizedString::TokenizedString( const std::string& stringToTokenize, const std::string& delimiter, bool allowQuotedStrings/* = false*/ )
	:	m_delimiter( delimiter )
{
	SplitIntoTokens( stringToTokenize );
	if ( allowQuotedStrings )
	{
		ConcatenateQuotedStringTokens();
	}
}

TokenizedString::TokenizedString( const char* stringToTokenize, const char* delimiter, bool allowQuotedStrings/* = false*/ )
	:	m_delimiter( delimiter )
{
	SplitIntoTokens( std::string( stringToTokenize ) );
	if ( allowQuotedStrings )
	{
		ConcatenateQuotedStringTokens();
	}
}

TokenizedString::~TokenizedString()
{

}

std::vector< std::string > TokenizedString::GetTokensNoNull()
{
	for ( size_t index = 0; index < m_tokens.size(); index++ )
	{
		if ( m_tokens[ index ] == "" || m_tokens[ index ] == " " || m_tokens[ index ] == "\t"  )
		{
			m_tokens.erase( m_tokens.begin() + index );
			index--;
		}
	}

	return m_tokens;
}

std::vector< std::string > TokenizedString::GetTokens() const
{
	return m_tokens;
}

std::string TokenizedString::GetDelimiter() const
{
	return m_delimiter;
}

void TokenizedString::SplitIntoTokens( const std::string& stringToTokenize )
{
	size_t delimiterPosition = 0;
	std::string token = stringToTokenize;

	while ( delimiterPosition != std::string::npos )
	{
		size_t newDelimiterPosition = stringToTokenize.find_first_of( m_delimiter, delimiterPosition );

		if ( newDelimiterPosition != std::string::npos )
		{
			token = stringToTokenize.substr( delimiterPosition, ( newDelimiterPosition - delimiterPosition ) );
			m_tokens.push_back( token );
			delimiterPosition = newDelimiterPosition + 1;
		}
		else
		{
			token = stringToTokenize.substr( delimiterPosition, ( stringToTokenize.size() - delimiterPosition ) );
			delimiterPosition = std::string::npos;
		}
	}
	
	m_tokens.push_back( token );
}

void TokenizedString::ConcatenateQuotedStringTokens()
{
	std::vector< IntRange > rangesToConcatenate;
	int rangeMin = -1;

	for ( int tokenIndex = 0; tokenIndex < (int)m_tokens.size(); tokenIndex++ )
	{
		if ( ( rangeMin == -1 ) )		// Look for a left quote at the end of the string
		{
			if ( !m_tokens[ tokenIndex ].empty() && ( m_tokens[ tokenIndex ][ 0 ] == '\"' ) )
			{
				m_tokens[ tokenIndex ].erase( 0, 1 );
				if ( !m_tokens[ tokenIndex ].empty() )
				{
					if ( m_tokens[ tokenIndex ][ m_tokens[ tokenIndex ].length() - 1 ] == '\"' )	// If this string is surrounded by quotes, eliminate them
					{
						m_tokens[ tokenIndex ].erase( ( m_tokens[ tokenIndex ].length() - 1 ), 1 );
						rangeMin = - 1;
					}
					else
					{
						rangeMin = tokenIndex;
					}
				}
				else
				{
					rangeMin = -1;
				}
			}
		}
		else if ( !m_tokens[ tokenIndex ].empty() && m_tokens[ tokenIndex ][ m_tokens[ tokenIndex ].length() - 1 ] == '\"' )		// Look for a right quote at the end of the string
		{
			m_tokens[ tokenIndex ].erase( ( m_tokens[ tokenIndex ].length() - 1 ), 1 );
			if ( !m_tokens[ tokenIndex ].empty() )
			{
				rangesToConcatenate.push_back( IntRange( rangeMin, tokenIndex ) );
			}
			rangeMin = -1;
		}
	}

	for ( int rangeIndex = ( int )( rangesToConcatenate.size() - 1 ); rangeIndex >= 0; rangeIndex-- )	// Go in reverse so that indices are not changed
	{
		for ( int indexToConcatenate = rangesToConcatenate[ rangeIndex ].max; indexToConcatenate > rangesToConcatenate[ rangeIndex ].min; indexToConcatenate-- )
		{
			std::string stringToAppend = m_tokens[ indexToConcatenate ];
			m_tokens.erase( ( m_tokens.begin() + indexToConcatenate ) );
			m_tokens[ indexToConcatenate - 1 ].append( m_delimiter + stringToAppend );
		}
	}
}
