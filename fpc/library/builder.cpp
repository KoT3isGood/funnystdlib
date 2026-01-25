#include "builder.h"
#include "ld.h"
#include "c.h"

CUtlVector<BuildFileInfo_t> buildFileInfos = {};


class CProjectBuilder : public IProjectBuilder
{
public:
	virtual BuildFile_t *BuildProject( const char *szProjectName, const char *szPath ) override;

	BuildFile_t *m_pBuildFiles = NULL;
};


static CProjectBuilder s_projectBuilder;

EXPOSE_INTERFACE_GLOBALVAR(CProjectBuilder, IProjectBuilder, PROJECT_BUILDER_INTERFACE_NAME, s_projectBuilder)

IProjectBuilder *ProjectBuilder()
{
	return &s_projectBuilder;
};

CUtlString FPC_GetProjectObject( const char *szName, const char *szObjectName )
{
	for (auto b: buildFileInfos)
	{
		for (auto s: b.m_stages)
		{
			if (strcmp(s->m_psz, szName))
				continue;
			for (auto o: s->m_outputs)
			{
				if (strcmp(o.m_szName, szObjectName))
					continue;

				return o.m_szPath;
			}
		}
	}
	return NULL;
}

BuildFile_t *CProjectBuilder::BuildProject( const char *szProjectName, const char *szPath )
{
	CProject_t stCompileProject = {};
	LinkProject_t stLinkProject = {};
	CUtlString szBuildLibrary;
	void *pLibrary;
	CreateInterfaceFn pBuildFactory;
	BuildFileInfo_t *pBuildFileInfo;
	BuildFileInfo_t stBuildFileInfo;
	BuildFile_t *pBuildFile = NULL;
	CBuildStage **ppExecutedBuildStage = NULL;
	GetProjectObjectFn *pfnGetProjectObject = NULL;
	CUtlString szWd = CUtlString(szPath).GetDirectory();

	Plat_SetWorkingDir(szWd);

	
	stCompileProject.m_szName = szProjectName;
	stCompileProject.files = {
		szPath,
	};
	stCompileProject.includeDirectories = {CUtlString("%s/public",filesystem2->OwnDirectory())};
	stCompileProject.bFPIC = true;
	stCompileProject.m_target = Target_t::HostTarget();

	stLinkProject = ccompiler->Compile(&stCompileProject);
	stLinkProject.linkType = ELINK_DYNAMIC_LIBRARY;
	stLinkProject.m_target = Target_t::HostTarget();
	stLinkProject.objects.AppendHead({CUtlString("%s/libfpcbuild.a",filesystem2->OwnDirectory())});
	stLinkProject.libraryObjects.AppendHead({CUtlString("%s/libfpc.so",filesystem2->OwnDirectory())});
	stLinkProject.libraryObjects.AppendHead({CUtlString("%s/libtier2.a",filesystem2->OwnDirectory())});
	stLinkProject.libraryObjects.AppendHead({CUtlString("%s/libtier1.a",filesystem2->OwnDirectory())});
	szBuildLibrary = linker->Link(&stLinkProject);

	pLibrary = Plat_LoadLibrary(szBuildLibrary);
	if ( !pLibrary )
		return NULL;

	pBuildFactory = Sys_GetFactory(pLibrary);
	if (!pBuildFactory)
	{
		V_printf("Failed to find CreateInterface\n");
		Plat_UnloadLibrary(szBuildLibrary);
		return NULL;
	}

	pBuildFileInfo = (BuildFileInfo_t*)pBuildFactory(BUILD_FILE_INFO_INTERFACE_VERSION, NULL);
	ppExecutedBuildStage = (CBuildStage**)pBuildFactory(BUILD_CURRENT_STAGE_INTERFACE_VERSION, NULL);
	pfnGetProjectObject = (GetProjectObjectFn*)pBuildFactory(BUILD_GET_PROJECT_OBJECT_INTERFACE_VERSION, NULL);
	if (!pBuildFileInfo || !ppExecutedBuildStage || !pfnGetProjectObject)
	{
		V_printf("Required interfaces are not present\n");
		Plat_UnloadLibrary(szBuildLibrary);
		return NULL;
	}
	stBuildFileInfo = *pBuildFileInfo;
	buildFileInfos.AppendTail(stBuildFileInfo);
	*pfnGetProjectObject = FPC_GetProjectObject;
	for (auto a: stBuildFileInfo.m_dependantFiles)
	{
		BuildProject("something", CUtlString("%s/%s",szWd.GetString(),a));
	}
	
	Plat_SetWorkingDir(szWd);


	for (auto &build: stBuildFileInfo.m_stages)
	{
		*ppExecutedBuildStage = build;
		build->m_pMainFn();
		for ( auto &o: build->m_outputs)
		{
			o.m_szPath.AppendHead("/");
			o.m_szPath.AppendHead(Plat_GetWorkingDir());
		};
	};

	pBuildFile = new BuildFile_t;
	pBuildFile->m_szOutputFile = szBuildLibrary;
	pBuildFile->m_pLibrary = pLibrary;
	pBuildFile->m_pNext = m_pBuildFiles;
	m_pBuildFiles = pBuildFile;
	

	return pBuildFile;
}
