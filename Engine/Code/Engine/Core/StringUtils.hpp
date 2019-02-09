#pragma once
//-----------------------------------------------------------------------------------------------
#include <stdarg.h>
#include <string>
#include <vector>

//-----------------------------------------------------------------------------------------------
const int STRINGF_STACK_LOCAL_TEMP_LENGTH = 2048;

//-----------------------------------------------------------------------------------------------
const std::string Stringf( const char* format, ... );
const std::string Stringv( const char* format, va_list args );
const std::string Stringf( const int maxLength, const char* format, ... );
const std::string Stringv( const int maxLength, const char* format, va_list args );

class TokenizedString
{

public:
	explicit TokenizedString( const std::string& stringToTokenize, const std::string& delimiter, bool allowQuotedStrings = false );	// Only supports double quotes, ""
	explicit TokenizedString( const char* stringToTokenize, const char* delimiter, bool allowQuotedStrings = false );	// Only supports double quotes, ""
	~TokenizedString();

	std::vector< std::string > GetTokens() const;
	std::vector< std::string > GetTokensNoNull();	// Returns all tokens that aren't null strings, spaces or tabs
	std::string GetDelimiter() const;

private:
	void SplitIntoTokens( const std::string& stringToTokenize );
	void ConcatenateQuotedStringTokens();		// Limited support: Only quotes at the ends of strings that have already been split using the provided delimiter

private:
	std::vector< std::string > m_tokens;
	std::string m_delimiter = "";

};
