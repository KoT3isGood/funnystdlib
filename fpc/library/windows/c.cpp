#include "tier0/mem.h"
#include "winerunner.h"
#include "c.h"
#include "helper.h"
#include "obj.h"
#include "target.h"
#include "tier0/lib.h"
#include "tier0/mem.h"
#include "tier0/platform.h"
#include "tier0/commandline.h"
#include "tier1/interface.h"
#include "tier1/utlstring.h"
#include "tier1/utlvector.h"
#include "libgen.h"
#include "ctype.h"


class CMSVCCompiler : public ICCompiler
{
public:
	virtual LinkProject_t Compile( CProject_t *pProject ) override;

	virtual void GenerateLinterData() override;
protected:

	virtual CUtlVector<CUtlString> BuildCommandLine( CProject_t *pProject, const char *szFileName, const char *szOutputFileName ) override;
	
	// Returns executable which should the OS run
	virtual const char *GetCompilerExecutable( CProject_t *pProject ) override;

	// returns object file format, eg .obj or .o
	virtual const char *GetOutputObjectFormat() override;

	virtual void IncludeDirectory( CUtlVector<CUtlString> &cmd, const char *szName ) override;
	virtual void IncludeFile( CUtlVector<CUtlString> &cmd, const char *szName ) override;
	virtual void Macro( CUtlVector<CUtlString> &cmd, const char *szName ) override;
	virtual void Macro( CUtlVector<CUtlString> &cmd, const char *szName, const char *szValue ) override;
	
	virtual void SetTarget( CUtlVector<CUtlString> &cmd, CProject_t *pProject ) override;
	virtual void SetSysroot( CUtlVector<CUtlString> &cmd, CProject_t *pProject , const char *szSysroot ) override;
	virtual void SetOutputFile( CUtlVector<CUtlString> &cmd, const char *szName ) override;
	virtual void CompileFile( CUtlVector<CUtlString> &cmd, const char *szName ) override;
	
	virtual void EnableDebugSymbols( CUtlVector<CUtlString> &cmd ) override;
	virtual void EnablePIE( CUtlVector<CUtlString> &cmd ) override;
	virtual void EnablePIC( CUtlVector<CUtlString> &cmd ) override;

	virtual void SetStdC( CUtlVector<CUtlString> &cmd, ECVersion ) override;
	virtual void SetStdCPP( CUtlVector<CUtlString> &cmd, ECPPVersion ) override;
};

const char *CMSVCCompiler::GetOutputObjectFormat()
{
	return ".obj";
}

CUtlVector<CUtlString> CMSVCCompiler::BuildCommandLine( CProject_t *pProject, const char *szFileName, const char *szOutputFileName )
{
	CUtlVector<CUtlString> cmd;
	cmd = ICCompiler::BuildCommandLine(pProject, szFileName, szOutputFileName);
	cmd.AppendHead("/c");
	cmd.AppendTail("/nologo");
	return cmd;
}


const char *CMSVCCompiler::GetCompilerExecutable( CProject_t *pProject )
{
	IINISection *pSection = NULL;
	const char *szLinker = "cl.exe";
	if (!g_pConfig)
		return szLinker;


	pSection = g_pConfig->GetSection(pProject->m_target.GetTriplet());
	if (!pSection)
		return szLinker;


	szLinker = pSection->GetStringValue("cl");
	if (szLinker == NULL)
		return "cl.exe";
	return szLinker;
}


void CMSVCCompiler::IncludeDirectory( CUtlVector<CUtlString> &cmd, const char *szName )
{
	cmd.AppendTail(CUtlString("/I%s",szName));
}

void CMSVCCompiler::IncludeFile( CUtlVector<CUtlString> &cmd, const char *szName )
{
}

void CMSVCCompiler::Macro( CUtlVector<CUtlString> &cmd, const char *szName )
{
	cmd.AppendTail(CUtlString("/D%s", (char*)szName));
}

void CMSVCCompiler::Macro( CUtlVector<CUtlString> &cmd, const char *szName, const char *szValue )
{
	cmd.AppendTail(CUtlString("/D%s=%s", (char*)szName, (char*)szValue));
}

void CMSVCCompiler::EnableDebugSymbols( CUtlVector<CUtlString> &cmd )
{
}

void CMSVCCompiler::SetTarget( CUtlVector<CUtlString> &cmd, CProject_t *pProject )
{
	cmd.AppendTail("/utf-8");
	cmd.AppendTail("/EHsc");
}

void CMSVCCompiler::SetSysroot( CUtlVector<CUtlString> &cmd, CProject_t *pProject , const char *szName )
{
	if (szName != NULL)
	{
		cmd.AppendTail(CUtlString("/winsysroot:%s", szName));
		return;
	}

	if (!g_pConfig)
		return;


	IINISection *pSection = g_pConfig->GetSection(pProject->m_target.GetTriplet());
	if (!pSection)
		return;

	const char *szSysroot = pSection->GetStringValue("xwin");
	if (szSysroot)
	{
		cmd.AppendTail(CUtlString("/I%s/crt/include", szSysroot));
		cmd.AppendTail(CUtlString("/I%s/sdk/include/cppwinrt", szSysroot));
		cmd.AppendTail(CUtlString("/I%s/sdk/include/shared", szSysroot));
		cmd.AppendTail(CUtlString("/I%s/sdk/include/ucrt", szSysroot));
		cmd.AppendTail(CUtlString("/I%s/sdk/include/um", szSysroot));
		cmd.AppendTail(CUtlString("/I%s/sdk/include/winrt", szSysroot));
	}

}

void CMSVCCompiler::CompileFile( CUtlVector<CUtlString> &cmd, const char *szName )
{
	cmd.AppendTail(szName);
}
void CMSVCCompiler::SetOutputFile( CUtlVector<CUtlString> &cmd, const char *szName )
{
	cmd.AppendTail(CUtlString("/Fo%s",szName));
}
void CMSVCCompiler::EnablePIE( CUtlVector<CUtlString> &cmd )
{
}

void CMSVCCompiler::EnablePIC( CUtlVector<CUtlString> &cmd )
{
}

void CMSVCCompiler::SetStdC( CUtlVector<CUtlString> &cmd, ECVersion )
{
}

void CMSVCCompiler::SetStdCPP( CUtlVector<CUtlString> &cmd, ECPPVersion v )
{
	switch (v)
	{
	case CPPVERSION_17: cmd.AppendTail("/std:c++17"); break;
	case CPPVERSION_20: cmd.AppendTail("/std:c++20"); break;
	case CPPVERSION_23: cmd.AppendTail("/std:c++23"); break;
	default: break;
	}
}


EXPOSE_INTERFACE(CMSVCCompiler, ICCompiler, MSVC_C_COMPILER_INTERFACE_NAME);

void CMSVCCompiler::GenerateLinterData()
{
	/*
	FILE* f = V_fopen("compile_commands.json", "wb");
	V_fprintf(f, "[\n");
	uint32_t i = 0;
	for (auto &file: g_clangFiles)
	{
		V_fprintf(f, "\t{\n");
		V_fprintf(f, "\t\t\"arguments\": [\n");
		for (auto &arg: file.m_szArguments)
			V_fprintf(f, "\t\t\t\"%s\",\n",arg.GetString());
	
		V_fseek(f, -2, SEEK_CUR);
		V_fprintf(f, "\n\t\t],\n");
		V_fprintf(f, "\t\t\"file\": \"%s\",\n", file.m_szName.GetString());
		V_fprintf(f, "\t\t\"directory\": \"%s\"\n", filesystem2->BuildDirectory());
		V_fprintf(f, "\t},\n");
	};
	V_fseek(f, -2, SEEK_CUR);
	V_fprintf(f, "\n]\n");
	V_fclose(f);
	*/
};

struct ClangFile_t
{
	CUtlString m_szName;
	CUtlString m_szDir;
	CUtlVector<CUtlString> m_szArguments;
};

LinkProject_t CMSVCCompiler::Compile( CProject_t *pProject )
{
	if (pProject->m_szName == 0)
	{
		Plat_FatalErrorFunc("m_szName must be present\n");
	}

	LinkProject_t proj = {};
	proj.m_szName = pProject->m_szName;
	proj.m_target = pProject->m_target;
	proj.m_androidmanifest = pProject->m_androidmanifest;
	unsigned int hash = pProject->GenerateProjectHash();

	// Get output directories
	for (auto &file: pProject->files)
	{
		CUtlString szOutputFile = GetOutputObjectName(pProject, hash, file);
		CUtlString szOutputDir = szOutputFile;
		szOutputDir = dirname(szOutputDir);
		filesystem2->MakeDirectory(szOutputDir);
	}

	// Run CC
	for (auto &file: pProject->files)
	{
		
		bool bAreDependenciesUpdated = false;
		CUtlString szOutputFile = GetOutputObjectName(pProject, hash, file);
		CUtlVector<CUtlString> args;
		
		args = BuildCommandLine(pProject, file, szOutputFile);

		if (!filesystem2->ShouldRecompile(file, szOutputFile))
			goto skipcompile;
		else
			V_printf("  CC       %s\n", file.GetString());

		runner->Run(GetCompilerExecutable(pProject), args);
skipcompile:
		proj.objects.AppendTail((Object_t){szOutputFile});

		extern CUtlVector<ClangFile_t> g_clangFiles;
		ClangFile_t cfile = {};
		cfile.m_szName = file;
		cfile.m_szDir = Plat_GetWorkingDir();
		cfile.m_szArguments = args;
		cfile.m_szArguments.AppendHead(GetCompilerExecutable(pProject));

		g_clangFiles.AppendTail(cfile);
	}
	runner->Wait();	
	return proj;
}
