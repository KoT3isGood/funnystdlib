#include "c.h"
#include "ld.h"
#include "helper.h"
#include "tier0/platform.h"
#include "tier1/interface.h"

ADD_DEPENDENCY_BUILD_FILE(tier0, "../tier0/build.cpp");
ADD_DEPENDENCY_BUILD_FILE(tier1, "../tier1/build.cpp");
ADD_DEPENDENCY_BUILD_FILE(tier2, "../tier2/build.cpp");
ADD_DEPENDENCY_BUILD_FILE(appleauth, "../appleauth/build.cpp");


CUtlVector<CUtlString> g_fpcFiles = {

	"main.cpp",
	"library/helper.cpp",
	"library/target.cpp",
	"library/builder.cpp",
};

CUtlVector<CUtlString> g_libFpcFiles = {	
	"library/libfpc.cpp",
	"library/helper.cpp",
	"library/target.cpp",
	"library/builder.cpp",
	"library/runner.cpp",
	"library/winerunner.cpp",
	"library/deploy.cpp",
	
	"library/c.cpp",
	"library/ld.cpp",
	"library/swift.cpp",

	"library/android/apktool.cpp",
	"library/apple/appletool.cpp",
	
	"library/clang/c.cpp",
	"library/clang/ld.cpp",

	"library/windows/c.cpp",
	"library/windows/ld.cpp",
	
	"library/apple/swift.cpp",
	
};

CUtlVector<CUtlString> g_IncludeDirectories = {
	"public",
	"../public",
};

DECLARE_BUILD_STAGE(libfpcbuild)
{
	CProject_t compileProject = {};
	LinkProject_t ldProject = {};

	compileProject.m_szName = "fpcbuild";
	compileProject.files = g_libFpcFiles;
	compileProject.includeDirectories = g_IncludeDirectories;
	compileProject.bFPIC = true;
	ldProject = ccompiler->Compile(&compileProject);
	ldProject.linkType = ELINK_STATIC_LIBRARY;

	CUtlString outputProject = linker->Link(&ldProject);


	ADD_OUTPUT_OBJECT("fpcbuild", outputProject)

	return 0;
}
DECLARE_BUILD_STAGE(libfpc)
{
	if (linker->IsLibraryExists("clang"))
		g_libFpcFiles.AppendTail("library/clang/c_libclang.cpp");
	else
		V_printf("Warning: to support included files libclang must be installed.\n");
	CProject_t compileProject = {};
	LinkProject_t ldProject = {};

	compileProject.m_szName = "fpc";
	compileProject.files = g_libFpcFiles;
	compileProject.includeDirectories = g_IncludeDirectories;
	compileProject.bFPIC = true;
	compileProject.macros = {
		{"FPC_ARCH", CUtlString("\"%s\"",Target_t::StringFromCPU(compileProject.m_target.cpu))},
		{"FPC_OS", CUtlString("\"%s\"",Target_t::StringFromKernel(compileProject.m_target.kernel))},
		{"FPC_ABI", CUtlString("\"%s\"",Target_t::StringFromABI(compileProject.m_target.abi))},
	};
	ldProject = ccompiler->Compile(&compileProject);
	ldProject.linkType = ELINK_DYNAMIC_LIBRARY;
	ldProject.libraryObjects = {
		 GET_PROJECT_LIBRARY(tier0, "tier0"),
		 GET_PROJECT_LIBRARY(tier1, "tier1"),
		 GET_PROJECT_LIBRARY(tier2, "tier2"),
	};

	if (linker->IsLibraryExists("clang"))
		ldProject.libraries.AppendTail("clang");

	CUtlString outputProject = linker->Link(&ldProject);


	ADD_OUTPUT_OBJECT("fpc", outputProject)

	return 0;
};

DECLARE_BUILD_STAGE(fpc)
{
	CProject_t compileProject = {};
	LinkProject_t ldProject = {};

	compileProject.m_szName = "fpc";
	compileProject.files = g_fpcFiles;
	compileProject.bFPIC = true;
	compileProject.includeDirectories = g_IncludeDirectories;
	compileProject.macros = {
		{"FPC_ARCH", CUtlString("\"%s\"",Target_t::StringFromCPU(compileProject.m_target.cpu))},
		{"FPC_OS", CUtlString("\"%s\"",Target_t::StringFromKernel(compileProject.m_target.kernel))},
		{"FPC_ABI", CUtlString("\"%s\"",Target_t::StringFromABI(compileProject.m_target.abi))},
	};
	ldProject = ccompiler->Compile(&compileProject);
	ldProject.libraryObjects = {
		 GET_PROJECT_LIBRARY(tier0, "tier0"),
		 GET_PROJECT_LIBRARY(tier1, "tier1"),
		 GET_PROJECT_LIBRARY(tier2, "tier2"),
	};

	CUtlString outputProject = linker->Link(&ldProject);


	ADD_OUTPUT_OBJECT("fpc", outputProject);

	return 0;
};


DECLARE_BUILD_STAGE(install)
{
	CUtlString szExe = GET_PROJECT_LIBRARY(fpc, "fpc");
	CUtlString szLibFpc = GET_PROJECT_LIBRARY(libfpc, "fpc");
	CUtlString szTier0 = GET_PROJECT_LIBRARY(tier0, "tier0");
	CUtlString szTier1 = GET_PROJECT_LIBRARY(tier1, "tier1");
	CUtlString szTier2 = GET_PROJECT_LIBRARY(tier2, "tier2");
	CUtlString szHttp = GET_PROJECT_LIBRARY(funnyhttp, "funnyhttp");
	CUtlString szAppleAuth = GET_PROJECT_LIBRARY(appleauth, "appleauth");
	
	filesystem2->CopyFile("build/fpc_temp", szExe);
	filesystem2->CopyFile("build/libfpc_temp.so", szLibFpc);
	filesystem2->CopyFile("build", szHttp);
	filesystem2->CopyFile("build", szAppleAuth);

	/*
	filesystem2->CopyFile("build/libtier1.a", szTier1);
	filesystem2->CopyFile("build/libtier2.a", szTier2);
	filesystem2->CopyFile("build/libtier0_temp.so", szTier0);
	*/

	return 0;
};
