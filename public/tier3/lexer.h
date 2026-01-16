#ifndef BRB_LEXER_H
#define BRB_LEXER_H

#include "tier0/platform.h"

abstract_class ILexerWord
{
public:
	virtual int GetType() = 0;
};

abstract_class ILexer
{
public:
	virtual ILexerWord *ParseTokens( CUtlVector<Token_t> tokens ) = 0;
};

#endif
