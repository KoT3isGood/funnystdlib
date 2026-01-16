#ifndef TIER2_TOKENIZER_H
#define TIER2_TOKENIZER_H
#include "tier0/platform.h"
#include "tier1/utlstring.h"
#include "tier1/utlvector.h"

struct Token_t
{
	CUtlString m_szValue;
	bool m_bIsQuoted;

	uint32_t m_iLine;
	uint32_t m_iCharacter;
};

typedef bool( *fnIsAlphabetSymbol )( char c );
CUtlVector<Token_t> Tokenize( const char *szString );
CUtlVector<Token_t> Tokenize( const char *szString, fnIsAlphabetSymbol pfnIsAlphabetSymbol );


#endif
