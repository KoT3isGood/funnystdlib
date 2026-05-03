#ifndef TIER2_INI_H
#define TIER2_INI_H

#include "tier0/platform.h"
#include "tier1/utlstring.h"
#include "tier1/utlvector.h"

enum EIniValueType
{
	INITYPE_STRING,
	INITYPE_BOOLEAN,
	INITYPE_FLOAT,
};


struct IniValueData_t
{
	EIniValueType m_eType;
	CUtlString m_szKey;
	
	bool m_bData;
	float m_fData;
	CUtlString m_szData;
};

abstract_class IINISection
{
public:
	virtual bool GetBoolValue( const char *szKeyName ) = 0;
	virtual int GetIntValue( const char *szKeyName ) = 0;
	virtual const char *GetStringValue( const char *szKeyName ) = 0;
	virtual CUtlString GetUTLStringValue( const char *szKeyName ) = 0;
	virtual CUtlVector<IniValueData_t*> GetValues() = 0;
	virtual const char *GetName() = 0;
};

abstract_class IINIFile
{
public:
	virtual IINISection *GetSection( const char *szSectionName ) = 0;
	virtual CUtlVector<IINISection*> GetSections() = 0;
};

abstract_class IINIManager
{
public:
	virtual IINIFile *ReadFile( const char *psz ) = 0;
	virtual IINIFile *ReadString( const char *psz ) = 0;
	virtual void ReleaseFile( IINIFile *pFile ) = 0;
};

IINIManager *INIManager();

#endif
