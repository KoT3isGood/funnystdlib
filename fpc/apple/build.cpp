#include "c.h"
#include "swift.h"
#include "ld.h"
#include "helper.h"
#include "tier0/platform.h"
#include "tier1/interface.h"

ADD_DEPENDENCY_BUILD_FILE(tier1, "../../tier1/build.cpp");

DECLARE_BUILD_STAGE(xtool)
{
	if (!linker->IsLibraryExists("swiftCore"))
	{
		V_printf("Swift is not installed\n");
		return 0;
	}


	CUtlString outputProject = CUtlString("fpcxtool/.build/%s/debug/libfpcxtool.so",Target_t::HostTarget().GetTriplet().GetString());
	V_printf("SWIFT %s\n", outputProject.GetString());

	CUtlVector<CUtlString> args = {"build"};
	runner->Run("swift","fpcxtool", args);
	runner->Wait();

	ADD_OUTPUT_OBJECT("xtool", outputProject);

	return 0;
};

DECLARE_BUILD_STAGE(xtool_cpp)
{
	CProject_t compileProject = {};
	LinkProject_t ldProject = {};
	CUtlString szOutputProject = "";

	compileProject.m_szName = "xtool";
	compileProject.files = {
		"fpcxtool/fpcxtool.cpp"
	};
	compileProject.includeDirectories = {"../public","../../public"};
	compileProject.bFPIC = true;
	ldProject = ccompiler->Compile(&compileProject);

	ldProject.linkType = ELINK_DYNAMIC_LIBRARY;
	ldProject.objects.AppendTail({GET_PROJECT_LIBRARY(xtool, "xtool")});
	ldProject.libraryObjects = {
		GET_PROJECT_LIBRARY(tier1, "tier1"),
	};

	szOutputProject = linker->Link(&ldProject);
	ADD_OUTPUT_OBJECT("xtool", szOutputProject);
	return 0;
}
