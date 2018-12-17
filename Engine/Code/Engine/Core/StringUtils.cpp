#include "Engine/Core/StringUtils.hpp"
#include <stdarg.h>


//-----------------------------------------------------------------------------------------------
const int STRINGF_STACK_LOCAL_TEMP_LENGTH = 2048;


//-----------------------------------------------------------------------------------------------
const std::string Stringf( const char* format, ... )
{
	char textLiteral[ STRINGF_STACK_LOCAL_TEMP_LENGTH ];
	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, STRINGF_STACK_LOCAL_TEMP_LENGTH, _TRUNCATE, format, variableArgumentList );	
	va_end( variableArgumentList );
	textLiteral[ STRINGF_STACK_LOCAL_TEMP_LENGTH - 1 ] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

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
	textLiteral[ maxLength - 1 ] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	std::string returnValue( textLiteral );
	if( maxLength > STRINGF_STACK_LOCAL_TEMP_LENGTH )
		delete[] textLiteral;

	return returnValue;
}

TokenizedString::TokenizedString( const std::string& stringToTokenize, const std::string& delimiter )
	:	m_delimiter( delimiter )
{
	SplitIntoTokens( stringToTokenize );
}

TokenizedString::TokenizedString( const char* stringToTokenize, const char* delimiter )
	:	m_delimiter( delimiter )
{
	SplitIntoTokens( std::string( stringToTokenize ) );
}

TokenizedString::~TokenizedString()
{

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
