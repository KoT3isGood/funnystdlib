#include "c.h"
#include "ld.h"
#include "helper.h"
#include "tier0/platform.h"
#include "tier1/interface.h"
#include "tier2/ifilesystem.h"

DECLARE_BUILD_STAGE(zsign)
{	
	if (!filesystem)
	{
		void *pFilesystem = Plat_LoadLibrary("libfilesystem_std.so");
		CreateInterfaceFn pFilesystemFactory = Sys_GetFactory(pFilesystem);

		filesystem = (IFileSystem*)pFilesystemFactory(FILESYSTEM_INTERFACE_VERSION, NULL);
	}
	IDirectoryHandle *pDir = filesystem->OpenDir("zsign");
	if (!pDir)
		return 0;
	filesystem->CloseDir(pDir);

	CProject_t compileProject = {};
	LinkProject_t ldProject = {};
	CUtlString szOutputProject = "";

	compileProject.m_szName = "zsign";
	compileProject.m_target = Target_t::HostTarget();
	compileProject.bFPIC = true;
	compileProject.files = {
		"zsign/src/common/archive.cpp",
		"zsign/src/common/base64.cpp",
		"zsign/src/common/fs.cpp",
		"zsign/src/common/json.cpp",
		"zsign/src/common/log.cpp",
		"zsign/src/common/sha.cpp",
		"zsign/src/common/timer.cpp",
		"zsign/src/common/util.cpp",
		"zsign/src/archo.cpp",
		"zsign/src/bundle.cpp",
		"zsign/src/macho.cpp",
		"zsign/src/openssl.cpp",
		"zsign/src/signing.cpp",
		"zsign/src/zsign.cpp",
	};
	compileProject.includeDirectories = {
		"zsign/src/common",
		"zsign/src",
	};
	ldProject = ccompiler->Compile(&compileProject);

	ldProject.linkType = ELINK_DYNAMIC_LIBRARY;
	szOutputProject = linker->Link(&ldProject);

	ADD_OUTPUT_OBJECT("zsign", szOutputProject);

	return 0;
}
