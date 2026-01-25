#include "ld.h"
#include "helper.h"
#include "libgen.h"
#include "target.h"
#include "tier0/platform.h"
#include "tier1/interface.h"
#include "tier1/utlstring.h"

class CClangLinker : public ILinker
{
public:
	//virtual CUtlString Link( LinkProject_t *pProject ) override;
	virtual bool IsLibraryExists( CUtlString szName ) override;
protected:
	//virtual CUtlVector<CUtlString> BuildLinkCommandLine( LinkProject_t *pProject, const char *szFileName, const char *szOutputFileName );
	//virtual CUtlVector<CUtlString> BuildArchiveCommandLine( LinkProject_t *pProject, const char *szFileName, const char *szOutputFileName );
	
	// Returns executable which should the OS run
	virtual const char *GetCompilerExecutable( LinkProject_t *pProject ) override;
	
	virtual void SetTarget( CUtlVector<CUtlString> &cmd, LinkProject_t *pProject ) override;
	virtual void SetSysroot( CUtlVector<CUtlString> &cmd, LinkProject_t *pProject , const char *szSysroot ) override;
	virtual void SetOutputFile( CUtlVector<CUtlString> &cmd, const char *szName ) override;


	// sets rpath
	// for windows should be ignored
	virtual void SetDefaultLibraryPaths( CUtlVector<CUtlString> &cmd, LinkProject_t *pProject ) override;

	virtual void UseStdLib( CUtlVector<CUtlString> &cmd, bool bUse ) override;

	// windows doesn't use it as well
	virtual void UseDynamicLookup( CUtlVector<CUtlString> &cmd, bool bUse ) override;

	// includes whole file
	virtual void UseFullFile( CUtlVector<CUtlString> &cmd, LinkProject_t *pProject ) override;

	// includes used stuff in a file
	virtual void UsePartialFile( CUtlVector<CUtlString> &cmd, LinkProject_t *pProject ) override;
	
	virtual void LinkFile( CUtlVector<CUtlString> &cmd, const char *szName ) override;
	virtual void LinkLibraryObject( CUtlVector<CUtlString> &cmd, const char *szName ) override;
	virtual void LinkLibrary( CUtlVector<CUtlString> &cmd, const char *szName ) override;
	virtual void LinkLibraryPath( CUtlVector<CUtlString> &cmd, const char *szName ) override;
};



const char *CClangLinker::GetCompilerExecutable( LinkProject_t *pProject )
{
	IINISection *pSection = NULL;
	const char *szLinker = "clang++";
	if (!g_pConfig)
		return szLinker;


	pSection = g_pConfig->GetSection(pProject->m_target.GetTriplet());
	if (!pSection)
		return szLinker;


	szLinker = pSection->GetStringValue("CLANG_LINKER_INTERFACE_NAME");
	if (szLinker == NULL)
		return "clang++";
	return szLinker;
}



void CClangLinker::SetTarget( CUtlVector<CUtlString> &cmd, LinkProject_t *pProject )
{	
	if (pProject->linkType == ELINK_DYNAMIC_LIBRARY)
		cmd.AppendTail("-shared");

	if (pProject->m_target.kernel == TARGET_KERNEL_IOS)
		cmd.AppendTail("-fuse-ld=lld");

	cmd.AppendTail("-target");
	cmd.AppendTail(pProject->m_target.GetTriplet());
}

void CClangLinker::SetSysroot( CUtlVector<CUtlString> &cmd, LinkProject_t *pProject , const char *szName )
{
	if (szName != NULL)
	{
		cmd.AppendTail("--sysroot");
		cmd.AppendTail(szName);
		return;
	}

	if (!g_pConfig)
		return;


	IINISection *pSection = g_pConfig->GetSection(pProject->m_target.GetTriplet());
	if (!pSection)
		return;

	const char *szSysroot = pSection->GetStringValue("sysroot");
	if (szSysroot)
	{
		cmd.AppendTail("--sysroot");
		cmd.AppendTail(szSysroot);
	}
}

void CClangLinker::SetOutputFile( CUtlVector<CUtlString> &cmd, const char *szName )
{
	cmd.AppendTail("-o");
	cmd.AppendTail(szName);
}

void CClangLinker::SetDefaultLibraryPaths( CUtlVector<CUtlString> &cmd, LinkProject_t *pProject )
{
	if (pProject->m_target.kernel & TARGET_KERNEL_LINUX)
	{
		cmd.AppendTail("-Wl,--disable-new-dtags");
		cmd.AppendTail("-Wl,-rpath,$ORIGIN");
	}
}


void CClangLinker::UseStdLib( CUtlVector<CUtlString> &cmd, bool bUse )
{
	if (!bUse)
		cmd.AppendTail("-nostdlib");
}


void CClangLinker::UseDynamicLookup( CUtlVector<CUtlString> &cmd, bool bUse )
{

}


void CClangLinker::UseFullFile( CUtlVector<CUtlString> &cmd, LinkProject_t *pProject )
{
	if (pProject->m_target.kernel != TARGET_KERNEL_IOS)
		cmd.AppendTail("-Wl,--whole-archive");
}


void CClangLinker::UsePartialFile( CUtlVector<CUtlString> &cmd, LinkProject_t *pProject )
{
	if (pProject->m_target.kernel != TARGET_KERNEL_IOS)
		cmd.AppendTail("-Wl,--no-whole-archive");
}


void CClangLinker::LinkFile( CUtlVector<CUtlString> &cmd, const char *szName )
{
	cmd.AppendTail(szName);
}

void CClangLinker::LinkLibraryObject( CUtlVector<CUtlString> &cmd, const char *szName )
{
	CUtlString szDir = CUtlString(szName).GetDirectory();
	CUtlString szFileName = CUtlString(szName).GetFileName();
	if (!V_strncmp(szFileName, "lib",3))
		szFileName.RemoveHead(3);
	if (!V_strncmp(szFileName.GetFileExtension(), "so",2))
		szFileName.RemoveTail(3);
	if (!V_strncmp(szFileName.GetFileExtension(), "a",1))
		szFileName.RemoveTail(2);
	cmd.AppendTail("-L");
	cmd.AppendTail(szDir);
	cmd.AppendTail("-l");
	cmd.AppendTail(szFileName);
}

void CClangLinker::LinkLibrary( CUtlVector<CUtlString> &cmd, const char *szName )
{
	cmd.AppendTail("-l");
	cmd.AppendTail(szName);
}

void CClangLinker::LinkLibraryPath( CUtlVector<CUtlString> &cmd, const char *szName )
{
	cmd.AppendTail("-L");
	cmd.AppendTail(szName);
}

EXPOSE_INTERFACE(CClangLinker, ILinker, CLANG_LINKER_INTERFACE_NAME);
/*
CUtlString CClangLinker::Link( LinkProject_t *pProject )
{
	if (pProject->m_szName == 0)
	{
		Plat_FatalErrorFunc("m_szName must be present\n");
	}

	// Find a name for the file
	CUtlString szFileName;
	unsigned int hash = pProject->GenerateProjectHash();
	switch(pProject->linkType)
	{
	case ELINK_EXECUTABLE:
		if (pProject->m_target.kernel == TARGET_KERNEL_WINDOWS)
			szFileName = CUtlString("%s.exe", pProject->m_szName.GetString());	
		else if (pProject->m_target.kernel == TARGET_KERNEL_ANDROID)
			szFileName = CUtlString("lib%s.so", pProject->m_szName.GetString());
		else
			szFileName = CUtlString("%s", pProject->m_szName.GetString());
		break;
	case ELINK_STATIC_LIBRARY:
		szFileName = CUtlString("lib%s.a", pProject->m_szName.GetString());
		break;
	case ELINK_DYNAMIC_LIBRARY:
		if (pProject->m_target.kernel == TARGET_KERNEL_DARWIN)
			szFileName = CUtlString("lib%s.dylib", pProject->m_szName.GetString());
		if (pProject->m_target.kernel == TARGET_KERNEL_LINUX)
			szFileName = CUtlString("lib%s.so", pProject->m_szName.GetString());
		break;
	case ELINK_KERNEL_DRIVER:
		Plat_FatalErrorFunc("TODO: not supported\n");
		break;
	}

	CUtlString szTarget = pProject->m_target.GetTriplet();
	CUtlString szOutputFile = CUtlString("%s/%s/ld/%u_%s/%s",FPC_TEMPORAL_DIRNAME, szTarget.GetString(), hash, pProject->m_szName.GetString(), szFileName.GetString());
	CUtlString szOutputDir = szOutputFile;
	szOutputDir = dirname(szOutputDir);
	filesystem2->MakeDirectory(szOutputDir);
	if (pProject->linkType == ELINK_STATIC_LIBRARY)
	{
		V_printf("  AR       %s\n", pProject->m_szName.GetString());
		bool shouldRecompile = false;
		CUtlVector<CUtlString> args;
		for (auto object: pProject->objects)
		{
			if (filesystem2->ShouldRecompile(object.m_szObjectFile,szOutputFile))
			{
				shouldRecompile = true;
				break;
			}
		}
		if (!shouldRecompile)
			goto compiled;
		args = {
			"rcs",
			szOutputFile
		};
		for (auto object: pProject->objects)
			args.AppendTail(object.m_szObjectFile);
		runner->Run("ar", args);
		runner->Wait();

	} else {
		V_printf("  LINK     %s\n", pProject->m_szName.GetString());
		bool shouldRecompile = false;
		CUtlVector<CUtlString> args;

		// Check if any of the files have changed
		for (auto object: pProject->objects)
		{
			if (filesystem2->ShouldRecompile(object.m_szObjectFile,szOutputFile))
			{
				shouldRecompile = true;
				break;
			}
		}
		if (!shouldRecompile)
			goto compiled;


		CUtlString szTarget = pProject->m_target.GetTriplet();		
		CUtlString szCompiledTarget = szTarget; 
		if (pProject->m_target.kernel == TARGET_KERNEL_ANDROID)
		{
			szCompiledTarget = CUtlString("%s%u", szTarget.GetString(), pProject->m_androidmanifest.m_nTargetVersion);
		}
		args = {
			"-o",
			szOutputFile,
			"-target",
			szCompiledTarget,
		};

		// Disable stdlib
		if (pProject->bNoStdLib)
		{
			args.AppendTail("-nostdlib");
		}

		// Sysroots
		if (pProject->m_target.kernel == TARGET_KERNEL_DARWIN)
		{
			args.AppendTail("-isysroot");
			args.AppendTail("/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk");

			// Shouldn't be here
			args.AppendTail("-Wl,-export_dynamic");
			args.AppendTail("-undefined");
			args.AppendTail("dynamic_lookup");
		}
		else if (pProject->m_target.kernel == TARGET_KERNEL_IOS)
		{
			args.AppendTail("-isysroot");
			args.AppendTail("/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS.sdk");
			// Shouldn't be here ?
			args.AppendTail("-fembed-bitcode");
			args.AppendTail("-Wl,-rpath,@executable_path");
			args.AppendTail("-Wl,-all_load");
			args.AppendTail("-w");
			args.AppendTail("-miphoneos-version-min=18.0 ");	
		} 
		else if (pProject->m_target.kernel == TARGET_KERNEL_ANDROID)
		{
			// args.AppendTail(CUtlString("--sysroot=%s/sysroot", pProject->m_target.szSysroot));
			
			// Shouldn't be here ?
			args.AppendTail("-static-libstdc++");
		}
		else if (pProject->m_target.szSysroot)
		{
			// args.AppendTail(CUtlString("--sysroot=%s", pProject->m_target.szSysroot));
		}

		// Magic for the systems
		if (pProject->m_target.kernel == TARGET_KERNEL_WINDOWS)
		{
			args.AppendTail("-fuse-ld=ld");
		}
		if (pProject->m_target.kernel == TARGET_KERNEL_LINUX || pProject->m_target.kernel == TARGET_KERNEL_ANDROID)
		{
			args.AppendTail("-Wl,--disable-new-dtags");
			args.AppendTail("-Wl,-rpath,$ORIGIN");
		}

		// Dynamic libraries
		// Android can't run executables
		if (pProject->linkType == ELINK_DYNAMIC_LIBRARY || pProject->m_target.kernel == TARGET_KERNEL_ANDROID)
		{
			args.AppendTail("-shared");
		}

		// All the objects
		if (pProject->m_target.kernel == TARGET_KERNEL_WINDOWS || pProject->m_target.kernel == TARGET_KERNEL_LINUX || pProject->m_target.kernel == TARGET_KERNEL_ANDROID)
			args.AppendTail("-Wl,--whole-archive");
		for (auto object: pProject->objects)
			args.AppendTail(object.m_szObjectFile);
		if (pProject->m_target.kernel == TARGET_KERNEL_WINDOWS || pProject->m_target.kernel == TARGET_KERNEL_LINUX || pProject->m_target.kernel == TARGET_KERNEL_ANDROID)
			args.AppendTail("-Wl,--no-whole-archive");
		for (auto object: pProject->libraryObjects)
			args.AppendTail(object);

		// Libraries
		for (auto lib: pProject->libraries)
		{
			args.AppendTail("-l");
			args.AppendTail(lib);
		}
		for (auto lib: pProject->libraryDirectories)
		{
			args.AppendTail("-L");
			args.AppendTail(lib);
		}

		// Apple frameworks
		for (auto &directory: pProject->frameworkDirectories)
		{
			args.AppendTail("-F");
			args.AppendTail(directory);
		}
		for (auto &framework: pProject->frameworks)
		{
			args.AppendTail("-framework");
			args.AppendTail(framework);
		}

		IINISection *pSection = NULL;
		const char *szLinker;
		const char *szSysroot;
		if (!g_pConfig)
			goto use_default_linker; 


		pSection = g_pConfig->GetSection(pProject->m_target.GetTriplet());
		if (!pSection)
			goto use_default_linker;
		szSysroot = pSection->GetStringValue("sysroot");
		if (szSysroot)
		{
			args.AppendTail("--sysroot");
			args.AppendTail(szSysroot);
		}

		szLinker = pSection->GetStringValue("CLANG_LINKER_INTERFACE_NAME");
		if (szLinker)
		{
			runner->Run(szLinker, args);
		}

		runner->Wait();	

		goto compiled;
use_default_linker:
		runner->Run("clang++", args);
		runner->Wait();	
	}
compiled:
	return szOutputFile;
};
*/
bool CClangLinker::IsLibraryExists( CUtlString szName )
{
	szName = CUtlString("lib%s.so", szName.GetString());
	void *pLib = Plat_LoadLibrary(szName.GetString());
	if (!pLib)
		return false;
	Plat_UnloadLibrary(pLib);
	return true;
}
