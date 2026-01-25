#include "tier0/platform.h"
#include "tier0/rand.h"
#include "tier0/commandline.h"
#include "tier1/interface.h"
#include "tier1/utlvector.h"
#include "tier1/utlstring.h"
#include "tier2/ifilesystem.h"
#include "tier2/fileformats/ini.h"
#include "public/c.h"
#include "public/helper.h"
#include "public/ld.h"
#include "public/target.h"
#include "runner.h"
#include "winerunner.h"
#include "c.h"
#include "signal.h"
#include "libgen.h"
#include "builder.h"

CUtlString owndir;
static char **pszBuildDir;

void build_tier0()
{

};

int build()
{
	BuildFile_t *pBuildFile = ProjectBuilder()->BuildProject("main",CUtlString("%s/build.cpp",*pszBuildDir));

	/*

	LinkProject_t linkScriptProject = ccompiler->Compile(&compileScriptProject);
	linkScriptProject.linkType = ELINK_DYNAMIC_LIBRARY;
	linkScriptProject.m_target = Target_t::HostTarget();

	CUtlString script = linker->Link(&linkScriptProject);


	void *scriptDLL = Plat_LoadLibrary(script);
	pBuildFactory = Sys_GetFactory(scriptDLL);


	pBuildFactory = NULL;
	pBuildFileInfo = (BuildFileInfo_t*)pBuildFactory(BUILD_FILE_INFO_INTERFACE_NAME, NULL);
	if (!pBuildFactory)
		Plat_FatalErrorFunc("Failed to find build file info interface\n");

	auto PreinitFn = (void(*)())Plat_GetProc(scriptDLL, "Preinit");
	if (PreinitFn)
		PreinitFn();

	for (auto &build: pBuildFileInfo->m_stages)
	{
		build->m_pMainFn();
	};
	*/

	ccompiler->GenerateLinterData();

	return 0;
};


void IEngine_Signal(int sig)
{
	switch (sig)
	{
	case SIGSEGV:
	case SIGILL:
	case SIGABRT:
		Plat_Backtrace();
		Plat_FatalErrorFunc("Fault\n");
		break;
	case SIGINT:
		Plat_Exit(0);
		break;
	default:
		break;
	};
	Plat_Exit(0);
};

IRunner *runner;
IWineRunner *winerunner;
ICCompiler *ccompiler;
ILinker *linker;


int main(int c, char **v)
{	

	char path[1024];

	CUtlString buildcppDir = Plat_GetWorkingDir();
	owndir = buildcppDir;
	char *szBuildcppDir = buildcppDir.GetString();
	
findbuild:
	FILE* file = V_fopen("build.cpp", "rb");
	if (!file)
	{
		buildcppDir = buildcppDir.GetDirectory();
		if (buildcppDir=="/")
		{
			V_printf("build.cpp not found\n");
			return 0;
		}
		Plat_SetWorkingDir(szBuildcppDir);
		goto findbuild;
	} else {
		V_fclose(file);
	}

	#ifdef __linux
	signal(SIGHUP, IEngine_Signal);
	signal(SIGINT, IEngine_Signal);
	signal(SIGQUIT, IEngine_Signal);
	signal(SIGILL, IEngine_Signal);
	signal(SIGTRAP, IEngine_Signal);
	signal(SIGIOT, IEngine_Signal);
	signal(SIGBUS, IEngine_Signal);
	signal(SIGFPE, IEngine_Signal);
	signal(SIGSEGV, IEngine_Signal);
	signal(SIGTERM, IEngine_Signal);
	#endif
	

	void *pLibFPC = Plat_LoadLibrary("libfpc.so");
	CreateInterfaceFn pLibFPCFactory = Sys_GetFactory(pLibFPC);
	
	filesystem2 = (IFileSystem2*)pLibFPCFactory(FILE_SYSTEM_2_INTERFACE_NAME, NULL);
	pszBuildDir = (char**)pLibFPCFactory(FILE_SYSTEM_2_BUILD_DIRECTORY_INTERFACE_VERSION, NULL);
	*pszBuildDir = szBuildcppDir;
	runner = (IRunner*)pLibFPCFactory(RUNNER_INTERFACE_NAME, NULL);
	winerunner = (IWineRunner*)pLibFPCFactory(WINE_RUNNER_INTERFACE_NAME, NULL);
	ccompiler = (ICCompiler*)pLibFPCFactory(CLANG_C_COMPILER_INTERFACE_NAME, NULL);
	linker = (ILinker*)pLibFPCFactory(CLANG_LINKER_INTERFACE_NAME, NULL);
	


	pLibFPCFactory(LIBFPC_INIT_INTERFACE_VERSION, NULL);

	
	void *pFilesystem = Plat_LoadLibrary("libfilesystem_std.so");
	CreateInterfaceFn pFilesystemFactory = Sys_GetFactory(pFilesystem);

	filesystem = (IFileSystem*)pFilesystemFactory(FILESYSTEM_INTERFACE_VERSION, NULL);
	filesystem->Init();

	g_pConfig = INIManager()->ReadFile(".fpccfg");

	IINIFile **ppConfig = (IINIFile**)pLibFPCFactory(LIBFPC_CONFIG_INTERFACE_VERSION, NULL);
	*ppConfig = g_pConfig;

	CommandLine()->CreateCommandLine(c, v);
	Plat_InitRandom();

	if (CommandLine()->CheckParam("-v") || CommandLine()->CheckParam("--version"))
	{
		V_printf("fpc version v1\n");
		V_printf("built " __DATE__  " " __TIME__ "\n");
		V_printf("built for %s\n", Target_t::HostTarget().GetTriplet().GetString());
		return 0;
	}

	if (CommandLine()->CheckParam("build"))
		build();
	const char *szDeployDevice = CommandLine()->ParamValue("deploy");
	Plat_ShutdownRandom();
	return 0;
};
