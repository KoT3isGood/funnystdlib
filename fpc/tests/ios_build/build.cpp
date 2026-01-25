#include "helper.h"
#include "c.h"
#include "ld.h"
#include "tier1/utlstring.h"
#include "appletool.h"

DECLARE_BUILD_STAGE(ios_build)
{
	V_printf("Cool\n");
	CProject_t compileProject = {};
	compileProject.m_szName = "ios_app";
	compileProject.m_target = Target_t::HostTarget();
	compileProject.m_target.kernel = TARGET_KERNEL_IOS;
	compileProject.m_target.cpu = TARGET_CPU_AARCH64;
	compileProject.files = {
		"main.c",
	};
	LinkProject_t ldProject = ccompiler->Compile(&compileProject);
	CUtlString szOutput = linker->Link(&ldProject);

	AppleManifest_t manifest = {};
	manifest.SetPackageName("FPC Testing facility");
	manifest.SetPackageID("com.example.testfpc");
	manifest.SetPackageExecutable(szOutput);
	CUtlString szIpa = AppleTool()->BuildPackage( manifest, manifest.BuildManifest() );
	CUtlString szPackage = AppleTool()->SignPackage(szIpa, NULL);

	return 0;
}
