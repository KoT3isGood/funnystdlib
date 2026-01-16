#include "helper.h"
#include "c.h"
#include "ld.h"
#include "tier1/utlstring.h"

ADD_DEPENDENCY_BUILD_FILE(tier0, "../tier0/build.cpp")
ADD_DEPENDENCY_BUILD_FILE(tier1, "../tier1/build.cpp")
ADD_DEPENDENCY_BUILD_FILE(tier2, "../tier2/build.cpp")
ADD_DEPENDENCY_BUILD_FILE(http, "../http/build.cpp")

DECLARE_BUILD_STAGE(appleauth)
{
	CProject_t compileProject = {};
	LinkProject_t ldProject = {};

	compileProject.m_szName = "appleauth";
	compileProject.files = {
		"auth.cpp"
	};
	compileProject.includeDirectories = {"../public"};
	compileProject.bFPIC = true;
	ldProject = ccompiler->Compile(&compileProject);
	ldProject.objects.AppendTail({GET_PROJECT_LIBRARY(tier1, "tier1")});
	ldProject.objects.AppendTail({GET_PROJECT_LIBRARY(tier2, "tier2")});
	ldProject.libraries = {
		"crypto",
	};
	ldProject.linkType = ELINK_DYNAMIC_LIBRARY;

	CUtlString szOutputProject = linker->Link(&ldProject);

	ADD_OUTPUT_OBJECT("appleauth", szOutputProject)

	return 0;
};

