#pragma once
//-----------------------------------------------------------------------------------------------
#include <string>
#include <vector>

//-----------------------------------------------------------------------------------------------
const std::string Stringf( const char* format, ... );
const std::string Stringf( const int maxLength, const char* format, ... );

class TokenizedString
{

public:
	explicit TokenizedString( const std::string& stringToTokenize, const std::string& delimiter );
	explicit TokenizedString( const char* stringToTokenize, const char* delimiter );
	~TokenizedString();

	std::vector< std::string > GetTokens() const;
	std::string GetDelimiter() const;

private:
	void SplitIntoTokens( const std::string& stringToTokenize );

private:
	std::vector< std::string > m_tokens;
	std::string m_delimiter;

};
