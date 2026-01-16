#include "tier2/fileformats/json.h"
#include "tier2/tokenizer.h"

abstract_class CJSONArray: public IJSONArray 
{
public:
	virtual uint32_t GetCount() override;
	virtual IJSONValue *GetParameter( uint32_t i ) override;

	virtual void SetArray( uint32_t uCount, IJSONValue **ppValues ) override;

	virtual void CopyTo( IJSONArray *pObject ) override;
	virtual void Free() override;

	CUtlVector<IJSONValue*> m_values;
};

uint32_t CJSONArray::GetCount()
{
	return m_values.GetSize();
}

IJSONValue *CJSONArray::GetParameter( uint32_t i )
{
	return m_values[i];
}


void CJSONArray::SetArray( uint32_t uCount, IJSONValue **ppValues )
{
	m_values.Resize(uCount);
	for ( uint32_t u = 0; u < uCount; u++)
		m_values[u] = ppValues[u];
}

void CJSONArray::CopyTo( IJSONArray *pObject )
{

}

void CJSONArray::Free()
{
	for ( auto &value: m_values)
		JSONManager()->FreeValue(value);
	m_values = {};
}

abstract_class CJSONValue: public IJSONValue
{
public:
	virtual EJSONParameterType GetType( void ) override;
	virtual const char *GetStringValue() override;
	virtual float GetNumberValue() override;
	virtual bool GetBooleanValue() override;
	virtual IJSONArray *GetArray() override;
	virtual IJSONObject *GetObject() override;

	virtual void MakeNULL() override;	
	virtual void SetStringValue( const char *szString ) override;	
	virtual void SetNumberValue( float fValue ) override;	
	virtual void SetBooleanValue( bool bValue ) override;
	virtual void SetArrayValue( IJSONArray *pValue ) override;	
	virtual void SetObjectValue( IJSONObject *pValue ) override;

	virtual void CopyTo( IJSONValue *pObject ) override;
	virtual void Free() override;

	EJSONParameterType m_eType;
	CUtlString m_szString;
	float m_fValue;
	bool m_bValue;
	IJSONArray *m_pArray;
	IJSONObject *m_pObject;
};

EJSONParameterType CJSONValue::GetType( void )
{
	return m_eType;
}

const char * CJSONValue::GetStringValue()
{
	return m_szString;
}

float  CJSONValue::GetNumberValue()
{
	return m_fValue;
}

bool CJSONValue::GetBooleanValue()
{
	return m_fValue;
}

IJSONArray *CJSONValue::GetArray()
{
	return m_pArray;
}

IJSONObject *CJSONValue::GetObject()
{
	return m_pObject;
}


void CJSONValue::MakeNULL()
{
	if ( GetType() == JSON_PARAMETER_OBJECT )
	{
		JSONManager()->FreeObject(m_pObject);
		m_pObject = NULL;
	}
	if ( GetType() == JSON_PARAMETER_ARRAY )
	{

		JSONManager()->FreeArray(m_pArray);
		m_pArray = NULL;
	}
	if ( GetType() == JSON_PARAMETER_STRING )
		m_szString = NULL;
}
	
void CJSONValue::SetStringValue( const char *szString )
{
	MakeNULL();
	m_eType = JSON_PARAMETER_STRING;
	m_szString = szString;
}
	
void CJSONValue::SetNumberValue( float fValue )
{
	MakeNULL();
	m_eType = JSON_PARAMETER_NUMBER;
	m_fValue = fValue;
}
	
void CJSONValue::SetBooleanValue( bool bValue )
{
	MakeNULL();
	m_eType = JSON_PARAMETER_BOOLEAN;
}

void CJSONValue::SetArrayValue( IJSONArray *pValue )
{
	MakeNULL();
	m_eType = JSON_PARAMETER_ARRAY;
	m_pArray = pValue;
}
	
void CJSONValue::SetObjectValue( IJSONObject *pValue )
{
	MakeNULL();
	m_eType = JSON_PARAMETER_OBJECT;
	m_pObject = pValue;
}


void CJSONValue::CopyTo( IJSONValue *pObject )
{
	MakeNULL();
}

void CJSONValue::Free()
{
	MakeNULL();
}


abstract_class CJSONObject: public IJSONObject
{
public:
	virtual IJSONValue *GetValue( const char *szName ) override;
	virtual void SetValue( const char *szName, IJSONValue *pValue ) override;

	virtual void CopyTo( IJSONObject *pObject ) override;
	virtual void Free() override;

	struct JSONObjectParam_t
	{
		CUtlString m_szName;
		IJSONValue *m_pValue;
	};
	CUtlVector<JSONObjectParam_t> m_params;
};

IJSONValue *CJSONObject::GetValue( const char *szName )
{
	int i;
	JSONObjectParam_t *pParam;
	for ( i = 0; i < m_params.GetSize(); i++ )
	{
		if ( m_params[i].m_szName != szName )
			continue;
		return m_params[i].m_pValue;
	}
	return NULL;

}

void CJSONObject::SetValue( const char *szName, IJSONValue *pValue )
{
	int i;
	JSONObjectParam_t *pParam;
	for ( i = 0; i < m_params.GetSize(); i++ )
	{
		if ( m_params[i].m_szName != szName )
			continue;


		// Scary
		if ( pValue == NULL )
			m_params.RemoveAt(i);
		else
			m_params[i].m_pValue = pValue;
		return;
	}
	if ( pValue == NULL )
		return;
	m_params.AppendTail({szName, pValue});
}


void CJSONObject::CopyTo( IJSONObject *pObject )
{

}

void CJSONObject::Free()
{
	for ( auto &param: m_params )
	{
		JSONManager()->FreeValue(param.m_pValue);
	}
}

abstract_class CJSONManager: public IJSONManager 
{
public:
	virtual IJSONObject *CreateObject(  ) override;
	virtual void FreeObject( IJSONObject *pObject ) override;
	virtual IJSONArray *CreateArray(  ) override;
	virtual void FreeArray( IJSONArray *pArray ) override;
	virtual IJSONValue *CreateValue(  ) override;
	virtual void FreeValue( IJSONValue *pValue ) override;

	virtual IJSONValue *ReadString( const char *szString ) override;
	virtual CUtlString WriteString( IJSONValue *pValue ) override;

private:
	CUtlString RealWriteString( IJSONValue *pValue, uint32_t uOffest );
	CUtlString GetAsJsonString( CUtlString szString );

	static bool ExpectedToken( Token_t &token, const char *szValue );
	static CUtlString GetQuotedToken( Token_t &token );
	IJSONObject *ParseObject( Token_t *&pToken, const Token_t *pEnding  );
	IJSONArray *ParseArray( Token_t *&pToken, const Token_t *pEnding  );
	IJSONValue *ParseValue( Token_t *&pToken, const Token_t *pEnding  );
};

IJSONObject *CJSONManager::CreateObject(  )
{
	return new CJSONObject;
}

void CJSONManager::FreeObject( IJSONObject *pObject )
{
	pObject->Free();
	delete (CJSONObject*)pObject;
}

IJSONArray *CJSONManager::CreateArray(  )
{
	return new CJSONArray;
}

void CJSONManager::FreeArray( IJSONArray *pArray )
{
	pArray->Free();
	delete (CJSONObject*)pArray;
}

IJSONValue *CJSONManager::CreateValue(  )
{
	return new CJSONValue;
}

void CJSONManager::FreeValue( IJSONValue *pValue )
{
	pValue->Free();
	delete (CJSONValue*)pValue;
}

#define NEXT_TOKEN() \
pToken++; \
if (pToken == pEnding) \
	goto eof \

bool CJSONManager::ExpectedToken( Token_t &token, const char *szValue )
{
	if ( token.m_szValue == szValue && !token.m_bIsQuoted )
		return true;
	return false;
}
CUtlString CJSONManager::GetQuotedToken( Token_t &token )
{
	if ( token.m_bIsQuoted )
		return token.m_szValue;
	return NULL;
}

IJSONObject *CJSONManager::ParseObject( Token_t *&pToken, const Token_t *pEnding )
{
	IJSONObject *pObject;
	CUtlString szParamName;
	IJSONValue *pValue;

	if ( !ExpectedToken(*(pToken), "{") )
		return NULL;
	NEXT_TOKEN();
	pObject = CreateObject();

	// object might be empty
	if ( ExpectedToken(*pToken, "}") )
	{
		NEXT_TOKEN();
		return pObject;
	}

	while(true)
	{
		szParamName = NULL;
		pValue = NULL;
		if ( GetQuotedToken(*pToken) == NULL )
			goto not_quoted;
		
		szParamName = pToken->m_szValue;
		NEXT_TOKEN();

		if ( !ExpectedToken(*pToken, ":") )
			goto not_colon;
		NEXT_TOKEN();

		pValue = ParseValue(pToken, pEnding);
		pObject->SetValue(szParamName, pValue);

		if ( !ExpectedToken(*pToken, ",") )
		{
			if ( !ExpectedToken(*pToken, "}") )
			{
				goto not_comma;
			}
			return pObject;
		}
		NEXT_TOKEN();
	}
	return pObject;
not_comma:
	V_printf("%i: comma (,) or } was expected but got %s\n", pToken->m_iLine, pToken->m_szValue.GetString());
	return NULL;
not_colon:
	V_printf("%i: colon (:) was expected but got %s\n", pToken->m_iLine, pToken->m_szValue.GetString());
	return NULL;

not_quoted:
	V_printf("%s was not quoted\n", szParamName.GetString());
	return NULL;

eof:
	V_printf("EOF\n");
	return NULL;
}

IJSONArray *CJSONManager::ParseArray( Token_t *&pToken, const Token_t *pEnding  )
{
	IJSONArray *pObject;
	CUtlString szParamName;
	IJSONValue *pValue;
	CUtlVector<IJSONValue*> values;

	if ( !ExpectedToken(*(pToken), "[") )
		return NULL;
	NEXT_TOKEN();
	pObject = CreateArray();

	// object might be empty
	if ( ExpectedToken(*pToken, "]") )
	{
		NEXT_TOKEN();
		return pObject;
	}

	while(true)
	{
		pValue = ParseValue(pToken, pEnding);
		values.AppendTail(pValue);

		if ( !ExpectedToken(*pToken, ",") )
		{
			if ( !ExpectedToken(*pToken, "]") )
			{
				goto not_comma;
			}

			pObject->SetArray(values.GetSize(), values.GetData());
			return pObject;
		}
		NEXT_TOKEN();
	}
	return pObject;
not_comma:
	V_printf("%i: comma (,) or } was expected but got %s\n", pToken->m_iLine, pToken->m_szValue.GetString());
	return NULL;

eof:
	V_printf("EOF\n");
	pObject->SetArray(values.GetSize(), values.GetData());
	return pObject;
}

IJSONValue *CJSONManager::ParseValue( Token_t *&pToken, const Token_t *pEnding )
{
	IJSONObject *pObject = ParseObject(pToken, pEnding);
	IJSONArray *pArray = ParseArray(pToken, pEnding);
	IJSONValue *pValue = CreateValue();
	if (pObject)
	{
		pValue->SetObjectValue(pObject);
		return pValue;
	}
	if (pArray)
	{
		pValue->SetArrayValue(pArray);
		return pValue;
	}
	if ( GetQuotedToken(*pToken) != NULL )
	{
		pValue->SetStringValue(pToken->m_szValue);
		NEXT_TOKEN();
		return pValue;
	}
	return NULL;
eof:
	V_printf("EOF\n");
	return NULL;
}


IJSONValue *CJSONManager::ReadString( const char *psz )
{
	CUtlVector<Token_t> tokens;
	CUtlVector<char> stack;
	IJSONValue *pGlobalObject = NULL;

	tokens = Tokenize(psz);
	Token_t *pCurrentToken = tokens.GetData();
	pGlobalObject = ParseValue(pCurrentToken, tokens.end().m_pCurrent);

	return pGlobalObject;
};

CUtlString CJSONManager::WriteString( IJSONValue *pValue )
{
	return RealWriteString(pValue, 0);
}
CUtlString CJSONManager::GetAsJsonString( CUtlString szString )
{
	CUtlString szOut = "\"";
	for ( int i = 0; i < szString.GetLenght(); i++ )
	{
		char c = szString[i];
		switch (c)
		{
		case '\"':
			szOut.AppendTail("\\\"");
			break;
		case '\n':
			szOut.AppendTail("\\n");
			break;
		case '\r':
			szOut.AppendTail("\\r");
			break;
		case '\t':
			szOut.AppendTail("\\t");
			break;
		case '\\':
			szOut.AppendTail("\\\\");
			break;
		default:
			szOut.AppendTail(c);
			break;
		}
	};
	szOut.AppendTail("\"");
	return szOut;
}

CUtlString CJSONManager::RealWriteString( IJSONValue *pValue, uint32_t uOffset )
{
	IJSONArray *pArray;
	CJSONObject *pObject;
	CUtlString szString = "";
	if (pValue)
	switch (pValue->GetType())
	{
	case JSON_PARAMETER_NULL:
		return CUtlString("null", pValue->GetNumberValue());
	case JSON_PARAMETER_BOOLEAN:
		if (pValue->GetBooleanValue())
			return "true";
		return "false";
	case JSON_PARAMETER_NUMBER:
		return CUtlString("%f", pValue->GetNumberValue());
	case JSON_PARAMETER_STRING:
		return GetAsJsonString(pValue->GetStringValue());
	case JSON_PARAMETER_ARRAY:
		pArray = pValue->GetArray();
		szString.AppendTail("[\n");
		for ( uint32_t i = 0; i < pArray->GetCount(); i++ )
		{
			for ( uint32_t j = 0; j <= uOffset; j++)
				szString.AppendTail("\t");
			szString.AppendTail(RealWriteString( pArray->GetParameter(i), uOffset+1 ));
			if ( i != pArray->GetCount() - 1 )
				szString.AppendTail(",");
			szString.AppendTail("\n");
		}
		for ( uint32_t j = 0; j < uOffset; j++)
			szString.AppendTail("\t");
		szString.AppendTail("]");
		return szString;
	case JSON_PARAMETER_OBJECT:
		pObject = (CJSONObject*)pValue->GetObject();
		szString.AppendTail("{\n");
		for ( uint32_t i = 0; i < pObject->m_params.GetSize(); i++ )
		{
			for ( uint32_t j = 0; j <= uOffset; j++)
				szString.AppendTail("\t");

			szString.AppendTail(GetAsJsonString(pObject->m_params[i].m_szName));
			szString.AppendTail(": ");

			szString.AppendTail(RealWriteString( pObject->m_params[i].m_pValue, uOffset+1 ));
			if ( i != pObject->m_params.GetSize() - 1 )
				szString.AppendTail(",");
			szString.AppendTail("\n");
		}
		for ( uint32_t j = 0; j < uOffset; j++)
			szString.AppendTail("\t");
		szString.AppendTail("}");
		return szString;
	}
}

IJSONManager *JSONManager()
{
	static CJSONManager mgr;
	return &mgr;
}
