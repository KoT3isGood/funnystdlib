#include "helper.h"
#include "c.h"
#include "ld.h"
#include "tier1/utlstring.h"

CUtlVector<CUtlString> tier2_CompiledFiles = {
	"fileformats/ini.cpp",
	"fileformats/json.cpp",
	"fileformats/xml.cpp",
	"fileformats/plist.cpp",
	"tokenizer.cpp",
	"filesystem.cpp",
};


DECLARE_BUILD_STAGE(tier2)
{
	CProject_t compileProject = {};
	LinkProject_t ldProject = {};

	compileProject.m_szName = "tier2";
	compileProject.files = tier2_CompiledFiles;
	compileProject.includeDirectories = {"../public"};
	compileProject.bFPIC = true;
	ldProject = ccompiler->Compile(&compileProject);
	ldProject.linkType = ELINK_STATIC_LIBRARY;

	CUtlString szOutputProject = linker->Link(&ldProject);

	ADD_OUTPUT_OBJECT("tier2", szOutputProject)

	return 0;
};
