#include "helper.h"
#include "c.h"
#include "ld.h"
#include "tier1/utlstring.h"

CUtlVector<CUtlString> tier1_CompiledFiles = {
	"interface.cpp",
	"appinit.cpp",
	"utlbuffer.cpp",
	"utlmap.cpp",
	"utlstring.cpp",
	"utlvector.cpp",
};

DECLARE_BUILD_STAGE(tier1)
{
	CProject_t compileProject = {};
	LinkProject_t ldProject = {};

	compileProject.m_szName = "tier1";
	compileProject.files = tier1_CompiledFiles;
	compileProject.includeDirectories = {"../public"};
	compileProject.bFPIC = true;
	ldProject = ccompiler->Compile(&compileProject);
	ldProject.linkType = ELINK_STATIC_LIBRARY;

	CUtlString szOutputDir = linker->Link(&ldProject);

	ADD_OUTPUT_OBJECT("tier1", szOutputDir)

	return 0;
};
