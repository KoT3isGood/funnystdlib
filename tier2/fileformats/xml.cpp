#include "tier2/fileformats/xml.h"
#include "tier2/tokenizer.h"

class CXMLObject: public IXMLObject
{
public:
	virtual EXMLObjectType GetType() override;
	virtual const char *GetValue() override;
	virtual void SetType( EXMLObjectType eType ) override;
	virtual void SetValue( const char *psz ) override;
	virtual CUtlVector<IXMLObject*> &GetChildren() override;
	virtual CUtlVector<XMLParam_t> &GetParams() override;
	virtual void Free() override;

	EXMLObjectType m_eType;
	CUtlString m_szValue;
	CUtlVector<IXMLObject*> m_children;
	CUtlVector<XMLParam_t> m_params;
};

EXMLObjectType CXMLObject::GetType()
{
	return m_eType;
}

const char *CXMLObject::GetValue()
{
	return m_szValue.GetString();
}

void CXMLObject::SetType( EXMLObjectType eType )
{
	m_eType = eType;
}

void CXMLObject::SetValue( const char *psz )
{
	m_szValue = psz;
}


CUtlVector<IXMLObject*> &CXMLObject::GetChildren()
{
	return m_children;
}

CUtlVector<XMLParam_t> &CXMLObject::GetParams()
{
	return m_params;
}

void CXMLObject::Free()
{

}


class CXMLManager: public IXMLManager
{
public:
	virtual XMLFile_t ReadString( const char *szData ) override;
	virtual CUtlString WriteString( IXMLObject *pObject ) override;

	virtual IXMLObject *CreateObject() override;
	virtual void FreeObject( IXMLObject *pObject ) override;

	IXMLObject *ReadTagParams( CUtlString szTag );
	XMLFile_t ReadDoctype( const char *szData );

	bool BIsValidAtStart( char c );
	bool BIsValidAtMid( char c );
	void SkipWhiteSpaces( const char *&psz );
};

static char XML_TAG[] = "<";
static char XML_TAG_END[] = ">";
static char XML_SELF_CLOSE_END[] = "/>";
static char XML_CLOSE_TAG[] = "</";
static char XML_DOCTYPE[] = "<!DOCTYPE";
static char XML_PREPROCESSOR_BEGIN[] = "<?";
static char XML_PREPROCESSOR_END[] = "?>";

XMLFile_t CXMLManager::ReadString( const char *szString )
{
	CUtlString szTagStack = {};
	CUtlString szRootObjectName;
	IXMLObject *pRootObject = CreateObject();
	IXMLObject *pLastObject = pRootObject;
	CUtlVector<IXMLObject*> stack = {pRootObject};

	enum EXMLParsingMode
	{
		XML_PARSING_MODE_TEXT,
		XML_PARSING_MODE_PREPROCESS_TAG,
		XML_PARSING_MODE_TAG,
		XML_PARSING_MODE_CLOSE_TAG,
		XML_PARSING_MODE_DOCTYPE,
		XML_PARSING_MODE_COMMENT,
	} eMode = XML_PARSING_MODE_TEXT;

	const char *psz = szString;
	CUtlString szTag = "";
	CUtlString szText = "";
	XMLFile_t stFile = {};
	while (*psz)
	{
		switch (eMode)
		{
		case XML_PARSING_MODE_TEXT:
			if (!V_strncmp(XML_DOCTYPE, psz, sizeof(XML_DOCTYPE)-1))
			{
				psz+=sizeof(XML_DOCTYPE)-1;
				szTag = NULL;
				
				eMode = XML_PARSING_MODE_DOCTYPE;
				if (szText != NULL)
				{
					IXMLObject *pText = CreateObject();
					pText->SetType(XML_OBJECT_TEXT);
					pText->SetValue(szText);
					pLastObject->GetChildren().AppendTail(pText);
					szText = NULL;
				}

				break;
			}
			if (!V_strncmp(XML_PREPROCESSOR_BEGIN, psz, sizeof(XML_PREPROCESSOR_BEGIN)-1))
			{
				psz+=sizeof(XML_PREPROCESSOR_BEGIN)-1;
				szTag = NULL;

				eMode = XML_PARSING_MODE_PREPROCESS_TAG;
				if (szText != NULL)
				{
					IXMLObject *pText = CreateObject();
					pText->SetType(XML_OBJECT_TEXT);
					pText->SetValue(szText);
					pLastObject->GetChildren().AppendTail(pText);
					szText = NULL;
				}
				break;
			}
			if (!V_strncmp(XML_CLOSE_TAG, psz, sizeof(XML_CLOSE_TAG)-1))
			{
				psz+=sizeof(XML_CLOSE_TAG)-1;
				szTag = NULL;

				eMode = XML_PARSING_MODE_CLOSE_TAG;
				if (szText != NULL)
				{
					IXMLObject *pText = CreateObject();
					pText->SetType(XML_OBJECT_TEXT);
					pText->SetValue(szText);
					pLastObject->GetChildren().AppendTail(pText);
					szText = NULL;
				}
				break;
			}
			if (!V_strncmp(XML_TAG, psz, sizeof(XML_TAG)-1))
			{
				psz+=sizeof(XML_TAG)-1;
				szTag = NULL;

				eMode = XML_PARSING_MODE_TAG;
				if (szText != NULL)
				{
					IXMLObject *pText = CreateObject();
					pText->SetType(XML_OBJECT_TEXT);
					pText->SetValue(szText);
					pLastObject->GetChildren().AppendTail(pText);
					szText = NULL;
				}
				break;
			}
			szText.AppendTail(*psz);
			psz++;
			break;
		case XML_PARSING_MODE_PREPROCESS_TAG:
			if (!V_strncmp(XML_PREPROCESSOR_END, psz, sizeof(XML_PREPROCESSOR_END)-1))
			{
				psz+=sizeof(XML_PREPROCESSOR_END)-1;
				eMode = XML_PARSING_MODE_TEXT;
				V_printf("preprocess: %s\n", szTag.GetString());
				break;
			}
			szTag.AppendTail(*psz);
			psz++;
			break;
		case XML_PARSING_MODE_TAG:
			if (!V_strncmp(XML_SELF_CLOSE_END, psz, sizeof(XML_SELF_CLOSE_END)-1))
			{
				psz+=sizeof(XML_SELF_CLOSE_END)-1;
				eMode = XML_PARSING_MODE_TEXT;
				IXMLObject *pObject = ReadTagParams(szTag);
				pLastObject->GetChildren().AppendTail(pObject);
				
				break;
			}

			if (!V_strncmp(XML_TAG_END, psz, sizeof(XML_TAG_END)-1))
			{
				psz+=sizeof(XML_TAG_END)-1;
				eMode = XML_PARSING_MODE_TEXT;
				IXMLObject *pObject = ReadTagParams(szTag);
				stack.AppendTail(pObject);
				pLastObject->GetChildren().AppendTail(pObject);
				pLastObject = pObject;
				break;
			}

			szTag.AppendTail(*psz);
			psz++;
			break;
		case XML_PARSING_MODE_CLOSE_TAG:
			if (!V_strncmp(XML_TAG_END, psz, sizeof(XML_TAG_END)-1))
			{
				psz+=sizeof(XML_TAG_END)-1;
				eMode = XML_PARSING_MODE_TEXT;


				if (!V_strcmp(pLastObject->GetValue(), szTag))
				{
					stack.RemoveTail();
					pLastObject = stack[stack.GetSize()-1];
				}
				else 
					V_printf("\"%s\" != \"%s\"\n",pLastObject->GetValue(), szTag.GetString());
				break;
			}
			szTag.AppendTail(*psz);
			psz++;
			break;
		case XML_PARSING_MODE_DOCTYPE:
			if (!V_strncmp(XML_TAG_END, psz, sizeof(XML_TAG_END)-1))
			{
				psz+=sizeof(XML_TAG_END)-1;
				eMode = XML_PARSING_MODE_TEXT;

				stFile = ReadDoctype(szTag);
				break;
			}
			szTag.AppendTail(*psz);
			psz++;
			break;
		case XML_PARSING_MODE_COMMENT:
			break;
		};
	};
	stFile.m_pRoot = pRootObject;
	return stFile;
}
CUtlString CXMLManager::WriteString( IXMLObject *pObject )
{
};

IXMLObject *CXMLManager::ReadTagParams( CUtlString szTag )
{
	const char *psz = szTag;
	CUtlVector<XMLParam_t> params;
	CUtlString szTagName = NULL;
	CUtlString szParamName = NULL;
	IXMLObject *pObject = CreateObject();
	// Read tag name
	if (!BIsValidAtStart(*psz))
	{
		V_printf("tag must start with letter or _");
		FreeObject(pObject);
		return NULL;
	}

	szTagName.AppendTail(*psz++);
	while (BIsValidAtMid(*psz))
	{
		szTagName.AppendTail(*psz);
		psz++;
	}
	pObject->SetType(XML_OBJECT_ELEMENT);
	pObject->SetValue(szTagName);

	while (*psz)
	{
		while (V_isspace(*psz))
		{
			psz++;
			continue;
		}
		if (*psz && (!V_isalpha(*psz) || *psz == '_'))
		{
			V_printf("parameter must start with letter or _\n");
			return {};
		}
		while (*psz && (*psz != ' ' || *psz != '='))
		{
			psz++;
			continue;
		}
		while (V_isspace(*psz))
		{
			psz++;
			continue;
		}
		szParamName.AppendTail(*psz);
	};
	return pObject;
};

static bool XMLGetWordSymbol( char c )
{
	if (V_isalnum(c))
		return true;
	switch(c)
	{
	case ':':
		return true;
	case '.':
		return true;
	case '_':
		return true;
	case '-':
		return true;
	default:
		break;
	}
	return false;
};

XMLFile_t CXMLManager::ReadDoctype( const char *szData )
{
	const char *psz = szData;
	XMLFile_t stFile = {};
	CUtlString dtdType = NULL;
	CUtlVector<Token_t> tokens;
	int i = 0;

	if (!V_isspace(*psz))
		return stFile;

	SkipWhiteSpaces(psz);

	tokens = Tokenize(psz, XMLGetWordSymbol);
	if ( tokens.GetSize() < i + 1 )
		return stFile;
	if ( tokens[i].m_bIsQuoted )
		return stFile;

	stFile.m_szRootObjectName = tokens[i++].m_szValue;

	// parse ddt
	if ( tokens.GetSize() < i + 1 )
		return stFile;

	if ( tokens[i].m_szValue == "PUBLIC" && !tokens[i].m_bIsQuoted )
	{		
		i++;
		if ( tokens.GetSize() < i + 2 )
			return stFile;
		if (!tokens[i].m_bIsQuoted || !tokens[i+1].m_bIsQuoted)
			return stFile;
		stFile.m_szFPI = tokens[i++].m_szValue;
		stFile.m_szURI = tokens[i++].m_szValue;
	} else if ( tokens[i].m_szValue == "SYSTEM" && !tokens[i].m_bIsQuoted )
	{
	} else
	{

	}
	
	

	// read root element
	return stFile;
}

bool CXMLManager::BIsValidAtStart( char c )
{
	if (V_isalpha(c))
		return true;
	if (c == '_')
		return true;
	return false;
}

bool CXMLManager::BIsValidAtMid( char c )
{
	if (V_isalnum(c))
		return true;
	if (c == '_')
		return true;
	if (c == '-')
		return true;
	if (c == '.')
		return true;
	return false;
}
void CXMLManager::SkipWhiteSpaces( const char *&psz )
{
	while (V_isspace(*psz))
	{
		psz++;
		continue;
	}
}

IXMLObject *CXMLManager::CreateObject()
{
	return new CXMLObject;
}

void CXMLManager::FreeObject( IXMLObject *pObject )
{
	pObject->Free();
	delete (CXMLObject*)pObject;
}


IXMLManager *XMLManager()
{
	static CXMLManager mgr;
	return &mgr;
};
