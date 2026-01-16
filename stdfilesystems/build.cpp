#include "helper.h"
#include "c.h"
#include "ld.h"
#include "tier1/utlstring.h"

ADD_DEPENDENCY_BUILD_FILE(tier0, "../tier0/build.cpp")
ADD_DEPENDENCY_BUILD_FILE(tier1, "../tier1/build.cpp")

CUtlVector<CUtlString> fs_CompiledFiles = {
	"filesystem_libc.cpp",
};

DECLARE_BUILD_STAGE(filesystem_std)
{
	CProject_t compileProject = {};
	LinkProject_t ldProject = {};
	CUtlString szOutputProject = "";

	compileProject.m_szName = "filesystem_std";
	compileProject.files = fs_CompiledFiles;
	compileProject.includeDirectories = {"../public"};
	compileProject.bFPIC = true;
	ldProject = ccompiler->Compile(&compileProject);

	ldProject.linkType = ELINK_DYNAMIC_LIBRARY;
	ldProject.libraryObjects = {
		 GET_PROJECT_LIBRARY(tier0, "tier0"),
		 GET_PROJECT_LIBRARY(tier1, "tier1"),
	};
	szOutputProject = linker->Link(&ldProject);

	ADD_OUTPUT_OBJECT("fs", szOutputProject);

	return 0;
};
