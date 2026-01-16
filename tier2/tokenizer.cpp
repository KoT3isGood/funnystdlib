#include "tier2/tokenizer.h"

static bool IsWordSymbol( char c )
{
	if (V_isalnum(c))
		return true;
	switch (c)
	{
	case '_':
		return true;
	default:
		return false;
	}
}

CUtlVector<Token_t> Tokenize( const char *psz )
{
	return Tokenize(psz, IsWordSymbol);
}

CUtlVector<Token_t> Tokenize( const char *psz, fnIsAlphabetSymbol fnIsAlphabetSymbol )
{
	CUtlVector<Token_t> tokens = {};
	size_t i = 0;
	char c;
	uint32_t nCurrentLine = 0;
	uint32_t nCurrentCharacter = 0;
	bool bIsQuoted = false;
	bool bIsSlash = false;
	CUtlString szStringValue;

	while (true)
	{
		c = psz[i];
		i++;
		if (c == '\0')
			break;
		if (c == '\n')
		{
			nCurrentCharacter = 0;
			nCurrentLine++;
		}
		

		if (bIsQuoted)
		{
			// text within ""
			if (bIsSlash)
			{
				// reading \n \t etc.
				bIsSlash = false;
				switch (c)
				{
				case '\\':
					szStringValue.AppendTail('\\');
					continue;
				case '"':
					szStringValue.AppendTail('"');
					continue;
				case 'n':
					szStringValue.AppendTail('\n');
					continue;
				case 't':
					szStringValue.AppendTail('\t');
					continue;
				// allow for strings to go to next line with by ending with backward slash
				case '\n':
					szStringValue.AppendTail('\n');
					continue;
				default:
					szStringValue.AppendTail(c);
					continue;
				}
			} else {
				// react to special symbols witin a string
				switch (c)
				{

				// enables reading \n \t etc.
				case '\\':
					bIsSlash = true;
					continue;

				case '\n':
					// newline
					// we do want to end it to prevent parsing errors
					// and there is no standart
					V_printf("line %i: new line was found but the string wasn't finished, did you forget to place \" in the end of the line?\n", nCurrentLine);
				case '"':
					// end string
					bIsQuoted = false;
					if (szStringValue == 0)
						continue;
					tokens.AppendTail({szStringValue, true, nCurrentLine});
					szStringValue = 0;
					continue;
				default:
					szStringValue.AppendTail(c);
					continue;
				}
			}
			
		} else {
			// " makes it use a string
			if (c == '"')
			{
				bIsQuoted = true;
			}

			if (fnIsAlphabetSymbol(c))
			{
				szStringValue.AppendTail(c);
				continue;
			} else {
				if (szStringValue != 0)
					tokens.AppendTail({szStringValue, false, nCurrentLine});
				szStringValue = 0;
				if (V_isgraph(c) && c != '"')
					tokens.AppendTail({CUtlString("%c", c), false, nCurrentLine});
			}
		}
	};
	return tokens;
};
