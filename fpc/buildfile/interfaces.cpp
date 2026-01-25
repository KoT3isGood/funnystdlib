#include "tier1/interface.h"
#include "helper.h"

static BuildFileInfo_t buildfileinfo = {};
CBuildStage *g_pCurrentStage = NULL;
GetProjectObjectFn g_pfnGetProjectObject = NULL;

BuildFileInfo_t *GetBuildFileInfo()
{
	return &buildfileinfo;
}


CBuildStage::CBuildStage( const char *psz, int(*pMainFn)() )
{
	m_psz = psz;
	m_pMainFn = pMainFn;
	if (psz == 0 || pMainFn == 0)
		Plat_FatalErrorFunc("Name and function pointer must be set\n");
	
	GetBuildFileInfo()->m_stages.AppendTail(this);
};

CBuildDependentFile::CBuildDependentFile( const char *psz )
{
	// TODO: This doesn't affect recursion, which is bad

	GetBuildFileInfo()->m_dependantFiles.AppendTail(psz);
}

CUtlString FPC_GetProjectObject( const char *szName, const char *szObjectName )
{
	CUtlString szOutputString;
	szOutputString = g_pfnGetProjectObject(szName, szObjectName);
	return szOutputString;
}


EXPOSE_INTERFACE_GLOBALVAR(BuildFileInfo_t, BuildFileInfo_t, BUILD_FILE_INFO_INTERFACE_VERSION, buildfileinfo);
EXPOSE_INTERFACE_GLOBALVAR(CBuildStage, CBuildStage, BUILD_CURRENT_STAGE_INTERFACE_VERSION, g_pCurrentStage);
EXPOSE_INTERFACE_GLOBALVAR(GetProjectObjectFn, GetProjectObjectFn, BUILD_GET_PROJECT_OBJECT_INTERFACE_VERSION, g_pfnGetProjectObject);
