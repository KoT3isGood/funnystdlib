#include "helper.h"
#include "runner.h"
#include "c.h"
#include "ld.h"
#include "tier2/ifilesystem.h"
#include "config.h"

IFileSystem *filesystem;

void *LibFpcInit()
{
	filesystem2 = (IFileSystem2*)CreateInterface(FILE_SYSTEM_2_INTERFACE_NAME, NULL);
	runner = (IRunner*)CreateInterface(RUNNER_INTERFACE_NAME, NULL);

	/* configure default compilers */

	CreateInterfaceFn pLibFPCFactory = Sys_GetFactory("fpc");
	IConfigManager *mgr = (IConfigManager*)pLibFPCFactory(CONFIG_MANAGER_INTERFACE_VERSION, NULL);
	if (mgr)
	{
	
		CUtlString szCC = mgr->GetProperty(NULL, "ccompiler", Target_t::DefaultTarget());
		if (szCC)
		{
			ccompiler = (ICCompiler*)CreateInterface(szCC, NULL);
		}
		
		CUtlString szLink = mgr->GetProperty(NULL, "linker", Target_t::DefaultTarget());
		if (szLink)
		{
			linker = (ILinker*)CreateInterface(szLink, NULL);
		}
		

	}
	if (!ccompiler)
		ccompiler = (ICCompiler*)CreateInterface(CLANG_C_COMPILER_INTERFACE_NAME, NULL);
	if (!linker)
		linker = (ILinker*)CreateInterface(CLANG_LINKER_INTERFACE_NAME, NULL);

	CreateInterfaceFn pFilesystemFactory = Sys_GetFactory("filesystem_std");
	filesystem = (IFileSystem*)pFilesystemFactory(FILESYSTEM_INTERFACE_VERSION, NULL);
	filesystem->Init();
	return NULL;
}

EXPOSE_INTERFACE_FN(LibFpcInit, LibFpcInit, LIBFPC_INIT_INTERFACE_VERSION);
