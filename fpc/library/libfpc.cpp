#include "helper.h"
#include "runner.h"
#include "c.h"
#include "ld.h"
#include "tier2/ifilesystem.h"

IFileSystem *filesystem;

void *LibFpcInit()
{
	filesystem2 = (IFileSystem2*)CreateInterface(FILE_SYSTEM_2_INTERFACE_NAME, NULL);
	runner = (IRunner*)CreateInterface(RUNNER_INTERFACE_NAME, NULL);
	ccompiler = (ICCompiler*)CreateInterface(CLANG_C_COMPILER_INTERFACE_NAME, NULL);
	linker = (ILinker*)CreateInterface(CLANG_LINKER_INTERFACE_NAME, NULL);

	CreateInterfaceFn pFilesystemFactory = Sys_GetFactory("filesystem_std");
	filesystem = (IFileSystem*)pFilesystemFactory(FILESYSTEM_INTERFACE_VERSION, NULL);
	filesystem->Init();
	return NULL;
}

EXPOSE_INTERFACE_FN(LibFpcInit, LibFpcInit, LIBFPC_INIT_INTERFACE_VERSION);
