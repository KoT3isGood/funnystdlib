#ifndef TIER2_XML_H
#define TIER2_XML_H

#include "tier0/platform.h"
#include "tier1/utlstring.h"

struct XMLParam_t
{
	CUtlString m_szName;
	CUtlString m_szValue;
};

enum EXMLObjectType 
{
	XML_OBJECT_ELEMENT,
	XML_OBJECT_TEXT,
	XML_OBJECT_COMMENT,
	XML_OBJECT_CDATA,
	XML_OBJECT_PROCESSING_INSTRUCTION,
	XML_OBJECT_DOCUMENT_TYPE,
};

abstract_class IXMLObject
{
public:
	virtual EXMLObjectType GetType() = 0;
	virtual const char *GetValue() = 0;
	virtual void SetType( EXMLObjectType eType ) = 0;
	virtual void SetValue( const char *psz ) = 0;

	virtual CUtlVector<XMLParam_t> &GetParams() = 0;

	virtual CUtlVector<IXMLObject*> &GetChildren() = 0;

	virtual void Free() = 0;

	inline IXMLObject *FindFirstTag( const char *szName ) 
	{ 
		for ( auto &c: GetChildren() ) 
		{ 
			if ( c->GetType() != XML_OBJECT_ELEMENT ) continue;
			if ( V_strcmp(c->GetValue(), szName) ) continue;
			return c;
		}
		return NULL;
	};
};

struct XMLFile_t
{
	IXMLObject *m_pRoot;
	CUtlString m_szRootObjectName;
	CUtlString m_szFPI;
	CUtlString m_szURI;
};

abstract_class IXMLManager
{
public:
	virtual XMLFile_t ReadString( const char *szData ) = 0;
	virtual CUtlString WriteString( IXMLObject *pObject ) = 0;

	virtual IXMLObject *CreateObject() = 0;
	virtual void FreeObject( IXMLObject *pObject ) = 0;
};

IXMLManager *XMLManager();

#endif
