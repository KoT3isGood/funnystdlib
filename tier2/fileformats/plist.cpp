#include "tier2/fileformats/xml.h"
#include "tier2/fileformats/plist.h"

class CPropertyListManager: public IPropertyListManager
{
public:
	IJSONObject *ReadDict( IXMLObject *pDict );
	virtual IJSONObject *ReadString( const char *psz ) override;
};

IJSONObject *CPropertyListManager::ReadString( const char *psz )
{
	XMLFile_t file = XMLManager()->ReadString(psz);
	IJSONObject *pObject;

	IXMLObject *pList = file.m_pRoot->FindFirstTag(file.m_szRootObjectName);
	if (pList == NULL)
		return NULL;

	IXMLObject *pDict = pList->FindFirstTag("dict");
	if (pDict == NULL)
		return NULL;


	pObject = ReadDict(pDict);
	return pObject;
}
IJSONObject *CPropertyListManager::ReadDict( IXMLObject *pDict )
{
	CUtlString szKeyToCreate = NULL;
	IJSONObject *pObject = JSONManager()->CreateObject();
	for ( auto &c: pDict->GetChildren() )
	{
		if (c->GetType() != XML_OBJECT_ELEMENT)
			continue;
		if ( !V_strcmp(c->GetValue(), "key"))
		{
			szKeyToCreate = c->GetChildren()[0]->GetValue();
		}
		if ( !V_strcmp(c->GetValue(), "string") || !V_strcmp(c->GetValue(), "data") )
		{
			if ( szKeyToCreate == NULL )
				continue;
			IJSONValue *pVal = JSONManager()->CreateValue();
			if (c->GetChildren().GetSize()>0)
				pVal->SetStringValue(c->GetChildren()[0]->GetValue());
			else 	
				pVal->SetStringValue("");
			pObject->SetValue(szKeyToCreate, pVal);
			szKeyToCreate = NULL;
		}
		if ( !V_strcmp(c->GetValue(), "integer") )
		{
			if ( szKeyToCreate == NULL )
				continue;
			IJSONValue *pVal = JSONManager()->CreateValue();
			if (c->GetChildren().GetSize()>0)
				pVal->SetNumberValue(atol(c->GetChildren()[0]->GetValue()));
			else 	
				pVal->SetNumberValue(0);
			pObject->SetValue(szKeyToCreate, pVal);
			szKeyToCreate = NULL;
		}
		if ( !V_strcmp(c->GetValue(), "dict"))
		{
			if ( szKeyToCreate == NULL )
				continue;
			IJSONValue *pVal = JSONManager()->CreateValue();
			pVal->SetObjectValue(ReadDict(c));
			pObject->SetValue(szKeyToCreate, pVal);
			szKeyToCreate = NULL;
		}
	}
	return pObject;
}

IPropertyListManager *PropertyListManager()
{
	static CPropertyListManager mgr = {};
	return &mgr;
}
