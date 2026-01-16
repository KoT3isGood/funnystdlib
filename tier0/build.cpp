#include "helper.h"
#include "c.h"
#include "ld.h"
#include "tier1/utlstring.h"

CUtlVector<CUtlString> tier0_CompiledFiles = {
	"lib.cpp",
	"mem.cpp",
	"platform.cpp",
	"network.cpp",
	"commandline.cpp",
	"rand.cpp",
};

DECLARE_BUILD_STAGE(tier0)
{
	CProject_t compileProject = {};
	LinkProject_t ldProject = {};
	CUtlString szOutputProject = "";

	compileProject.m_szName = "tier0";
	compileProject.files = tier0_CompiledFiles;
	compileProject.includeDirectories = {"../public"};
	compileProject.bFPIC = true;
	ldProject = ccompiler->Compile(&compileProject);

	ldProject.linkType = ELINK_DYNAMIC_LIBRARY;
	szOutputProject = linker->Link(&ldProject);

	ADD_OUTPUT_OBJECT("tier0", szOutputProject);

	return 0;
};
