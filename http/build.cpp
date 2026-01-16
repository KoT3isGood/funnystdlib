#include "helper.h"
#include "c.h"
#include "ld.h"
#include "tier1/utlstring.h"
#include "tier0/commandline.h"

ADD_DEPENDENCY_BUILD_FILE(tier0, "../tier0/build.cpp")
ADD_DEPENDENCY_BUILD_FILE(tier1, "../tier1/build.cpp")
ADD_DEPENDENCY_BUILD_FILE(tier2, "../tier2/build.cpp")

DECLARE_BUILD_STAGE(funnyhttp)
{
	CProject_t compileProject = {};
	LinkProject_t ldProject = {};

	compileProject.m_szName = "funnyhttp";
	compileProject.files = {
		"client.cpp"
	};
	compileProject.includeDirectories = {"../public"};
	compileProject.bFPIC = true;
	ldProject = ccompiler->Compile(&compileProject);
	ldProject.objects.AppendTail({GET_PROJECT_LIBRARY(tier1, "tier1")});
	ldProject.objects.AppendTail({GET_PROJECT_LIBRARY(tier2, "tier2")});
	ldProject.libraries = {
		"ssl",
		"crypto",
	};
	ldProject.linkType = ELINK_DYNAMIC_LIBRARY;

	CUtlString szOutputDir = linker->Link(&ldProject);

	ADD_OUTPUT_OBJECT("funnyhttp", szOutputDir)

	return 0;
};

