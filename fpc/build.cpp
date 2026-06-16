#include "c.h"
#include "ld.h"
#include "helper.h"
#include "tier0/platform.h"
#include "tier1/interface.h"

ADD_DEPENDENCY_BUILD_FILE(tier0, "../tier0/build.cpp");
ADD_DEPENDENCY_BUILD_FILE(tier1, "../tier1/build.cpp");
ADD_DEPENDENCY_BUILD_FILE(tier2, "../tier2/build.cpp");
ADD_DEPENDENCY_BUILD_FILE(stdfilesystems, "../stdfilesystems/build.cpp");
ADD_DEPENDENCY_BUILD_FILE(appleauth, "../appleauth/build.cpp");
//ADD_DEPENDENCY_BUILD_FILE(xtool, "apple/build.cpp");


CUtlVector<CUtlString> g_fpcFiles = {

	"main.cpp",
	"library/helper.cpp",
	"library/target.cpp",
	"library/builder.cpp",

	"library/config.cpp",
};

CUtlVector<CUtlString> g_libFpcFiles = {	
	"library/libfpc.cpp",
	"library/helper.cpp",
	"library/target.cpp",
	"library/builder.cpp",
	"library/runner.cpp",
	"library/winerunner.cpp",
	"library/deploy.cpp",
	
	"library/config.cpp",
	
	"library/c.cpp",
	"library/ld.cpp",
	"library/swift.cpp",
	"library/rust.cpp",

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
	compileProject.files = {"buildfile/interfaces.cpp"};
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
	/*
	if (linker->IsLibraryExists("clang"))
		g_libFpcFiles.AppendTail("library/clang/c_libclang.cpp");
	else
		V_printf("Warning: to support included files libclang must be installed.\n");
	*/
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

	/*
	IINISection *pSection = g_pConfig->GetSection("config");
	if (pSection->GetStringValue("SWIFT_LIB_DIR"))
	{
		ldProject.libraries = {
			"FoundationInternationalization",
			"Foundation",
			"swiftRuntime",
			"swift_Concurrency",
			"FoundationEssentials",
			"swiftDispatch",
			"swiftSynchronization",
			"swiftGlibc",
			"swift_StringProcessing",
			"swiftCore",
			"swift_RegexParser",
			"BlocksRuntime",
			"FoundationEssentials",
			"dispatch",
			"swiftSwiftOnoneSupport",
			"_FoundationICU",
			"z",
			"xadi",
		};
		
		ldProject.libraryDirectories = {
			pSection->GetStringValue("SWIFT_LIB_DIR")
		};
	}
	*/

	/*
	if (linker->IsLibraryExists("clang"))
		ldProject.libraries.AppendTail("clang");
	*/

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
	CUtlString szFpcBuild = GET_PROJECT_LIBRARY(libfpcbuild, "fpcbuild");
	CUtlString szTier0 = GET_PROJECT_LIBRARY(tier0, "tier0");
	CUtlString szTier1 = GET_PROJECT_LIBRARY(tier1, "tier1");
	CUtlString szTier2 = GET_PROJECT_LIBRARY(tier2, "tier2");
	CUtlString szHttp = GET_PROJECT_LIBRARY(funnyhttp, "funnyhttp");
	CUtlString szAppleAuth = GET_PROJECT_LIBRARY(appleauth, "appleauth");
	CUtlString szFilesystem = GET_PROJECT_LIBRARY(filesystem_std, "fs");

	if (!CommandLine()->CheckParam("--install"))
		return 0;
	if (!CommandLine()->ParamValue("--install", NULL))
	{
		V_printf("--install expects directory\n");
		return 0;
	}
	CUtlString szOutputDirectory = CUtlString("%s/%s/",CommandLine()->ParamValue("--install"), Target_t::DefaultTarget().GetTriplet().GetString());
	CUtlString szHeaderDirectory = CUtlString("%s/%s/public",CommandLine()->ParamValue("--install"), Target_t::DefaultTarget().GetTriplet().GetString());
	filesystem2->MakeDirectory(szOutputDirectory);
	filesystem2->MakeDirectory(szHeaderDirectory);

	
	filesystem2->CopyFile(szOutputDirectory, szExe);
	filesystem2->CopyFile(szOutputDirectory, szLibFpc);
	filesystem2->CopyFile(szOutputDirectory, szFpcBuild);
	filesystem2->CopyFile(szOutputDirectory, szTier0);
	filesystem2->CopyFile(szOutputDirectory, szTier1);
	filesystem2->CopyFile(szOutputDirectory, szTier2);
	filesystem2->CopyFile(szOutputDirectory, szFilesystem);
	filesystem2->CopyDirectory(szOutputDirectory, "public");
	filesystem2->CopyDirectory(szHeaderDirectory, "../public/tier0");
	filesystem2->CopyDirectory(szHeaderDirectory, "../public/tier1");
	filesystem2->CopyDirectory(szHeaderDirectory, "../public/tier2");

	return 0;
};
