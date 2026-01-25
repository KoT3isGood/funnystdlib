#ifndef Apple_TOOL_H
#define Apple_TOOL_H

#include "tier0/platform.h"
#include "tier1/utlstring.h"
#include "legal.h"

struct AppleManifest_t
{
public:
	void SetPackageID( CUtlString szPackageID );
	void SetPackageName( CUtlString szPackageName );
	void SetPackageExecutable( CUtlString szPackageExecutable );

	CUtlString BuildManifest();

	CUtlString m_szPackageName;
	CUtlString m_szPackageID;
	CUtlString m_szPackageExecutable;
};

abstract_class IAppleTool
{
public:
	virtual CUtlString BuildPackage( AppleManifest_t manifest, CUtlString szManifestDir ) = 0;
	virtual CUtlString SignPackage( const char *szIpa, const char *szPassword ) = 0;
};


IAppleTool *AppleTool();

#endif

