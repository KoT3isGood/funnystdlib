#ifndef TIER2_JSON_H
#define TIER2_JSON_H

#include "tier0/platform.h"
#include "tier1/utlstring.h"

class IJSONObject;
class IJSONArray;
class IJSONValue;

enum EJSONParameterType
{
	JSON_PARAMETER_NULL,
	JSON_PARAMETER_STRING,
	JSON_PARAMETER_NUMBER,
	JSON_PARAMETER_BOOLEAN,
	JSON_PARAMETER_ARRAY,
	JSON_PARAMETER_OBJECT,
};

enum EJSONParameterNotes
{
	JSON_PARAMETER_NOTE_NONE = 0,
	JSON_PARAMETER_NOTE_BINARY,
};

abstract_class IJSONArray
{
public:
	virtual uint32_t GetCount() = 0;
	virtual IJSONValue *GetParameter( uint32_t i ) = 0;

	virtual void SetArray( uint32_t uCount, IJSONValue **ppValues ) = 0;

	virtual void CopyTo( IJSONArray *pObject ) = 0;
	virtual void Free() = 0;
};

abstract_class IJSONValue
{
public:
	virtual EJSONParameterType GetType( void ) = 0;
	virtual 
	virtual const char *GetStringValue() = 0;
	virtual float GetNumberValue() = 0;
	virtual bool GetBooleanValue() = 0;
	virtual IJSONArray *GetArray() = 0;
	virtual IJSONObject *GetObject() = 0;

	virtual void MakeNULL() = 0;	
	virtual void SetStringValue( const char *szString ) = 0;	
	virtual void SetNumberValue( float fValue ) = 0;	
	virtual void SetBooleanValue( bool bValue ) = 0;
	virtual void SetArrayValue( IJSONArray *pValue ) = 0;	
	virtual void SetObjectValue( IJSONObject *pValue ) = 0;

	virtual void CopyTo( IJSONValue *pObject ) = 0;
	virtual void Free() = 0;
};

abstract_class IJSONObject
{
public:
	virtual IJSONValue *GetValue( const char *szName ) = 0;
	virtual void SetValue( const char *szName, IJSONValue *pValue ) = 0;

	virtual void CopyTo( IJSONObject *pObject ) = 0;
	virtual void Free() = 0;
};

abstract_class IJSONManager
{
public:
	virtual IJSONObject *CreateObject(  ) = 0;
	virtual void FreeObject( IJSONObject *pObject ) = 0;
	virtual IJSONArray *CreateArray(  ) = 0;
	virtual void FreeArray( IJSONArray *pArray ) = 0;
	virtual IJSONValue *CreateValue(  ) = 0;
	virtual void FreeValue( IJSONValue *pValue ) = 0;

	virtual IJSONValue *ReadString( const char *szString ) = 0;
	virtual CUtlString WriteString( IJSONValue *pValue ) = 0;
};

IJSONManager *JSONManager();

#endif
