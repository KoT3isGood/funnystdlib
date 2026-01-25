#include "c.h"
#include "c_libclang.h"
#include "helper.h"
#include "obj.h"
#include "target.h"
#include "tier0/lib.h"
#include "tier0/platform.h"
#include "tier0/commandline.h"
#include "tier1/interface.h"
#include "tier1/utlstring.h"
#include "tier1/utlvector.h"
#include "tier2/fileformats/json.h"
#include "libgen.h"
#include "ctype.h"

struct ClangFile_t
{
	CUtlString m_szName;
	CUtlVector<CUtlString> m_szArguments;
};

class CClangCompiler : public ICCompiler
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
	virtual void CompileFile( CUtlVector<CUtlString> &cmd, const char *szName ) override;
	virtual void SetOutputFile( CUtlVector<CUtlString> &cmd, const char *szName ) override;
	
	virtual void EnableDebugSymbols( CUtlVector<CUtlString> &cmd ) override;
	virtual void EnablePIE( CUtlVector<CUtlString> &cmd ) override;
	virtual void EnablePIC( CUtlVector<CUtlString> &cmd ) override;

	virtual void SetSysroot( CUtlVector<CUtlString> &cmd, CProject_t *pProject, const char *szSysroot ) override;
};

const char *CClangCompiler::GetOutputObjectFormat()
{
	return ".o";
}

CUtlVector<CUtlString> CClangCompiler::BuildCommandLine( CProject_t *pProject, const char *szFileName, const char *szOutputFileName )
{
	CUtlVector<CUtlString> cmd;
	cmd = ICCompiler::BuildCommandLine(pProject, szFileName, szOutputFileName);
	cmd.AppendHead("-c");
	return cmd;
}


const char *CClangCompiler::GetCompilerExecutable( CProject_t *pProject )
{
	return "clang";
}


void CClangCompiler::IncludeDirectory( CUtlVector<CUtlString> &cmd, const char *szName )
{
	cmd.AppendTail("-I");
	cmd.AppendTail(szName);
}

void CClangCompiler::IncludeFile( CUtlVector<CUtlString> &cmd, const char *szName )
{
}

void CClangCompiler::Macro( CUtlVector<CUtlString> &cmd, const char *szName )
{
}

void CClangCompiler::Macro( CUtlVector<CUtlString> &cmd, const char *szName, const char *szValue )
{
	cmd.AppendTail("-D");
	cmd.AppendTail(CUtlString("%s=%s", (char*)szName, (char*)szValue));
}

void CClangCompiler::EnableDebugSymbols( CUtlVector<CUtlString> &cmd )
{
	cmd.AppendTail("-g");
}

void CClangCompiler::SetTarget( CUtlVector<CUtlString> &cmd, CProject_t *pProject )
{
	cmd.AppendTail("-target");
	cmd.AppendTail(pProject->m_target.GetTriplet());
	switch ( pProject->m_target.optimization )
	{
	case TARGET_RELEASE_SPEED:
		cmd.AppendTail("-O3");
		break;
	default:
		break;

	}
}

void CClangCompiler::CompileFile( CUtlVector<CUtlString> &cmd, const char *szName )
{
	cmd.AppendTail(szName);
}
void CClangCompiler::SetOutputFile( CUtlVector<CUtlString> &cmd, const char *szName )
{
	cmd.AppendTail("-o");
	cmd.AppendTail(szName);
}
void CClangCompiler::EnablePIE( CUtlVector<CUtlString> &cmd )
{
	cmd.AppendTail("-fPIE");
}

void CClangCompiler::EnablePIC( CUtlVector<CUtlString> &cmd )
{
	cmd.AppendTail("-fPIC");
}
void CClangCompiler::SetSysroot( CUtlVector<CUtlString> &cmd, CProject_t *pProject, const char *szName )
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



EXPOSE_INTERFACE(CClangCompiler, ICCompiler, CLANG_C_COMPILER_INTERFACE_NAME);

CUtlVector<ClangFile_t> g_clangFiles;
IClangBackend *clangbackend = NULL;

LinkProject_t CClangCompiler::Compile( CProject_t *pProject )
{
	if (!clangbackend && CommandLine()->CheckParam("-experimental_header_include"))
		clangbackend = (IClangBackend*)CreateInterface(CLANG_BACKEND_INTERFACE_NAME, NULL);

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
		/*
		CUtlString szTarget = pProject->m_target.GetTriplet();
		CUtlString szCompiledTarget = szTarget; 
		if (pProject->m_target.kernel == TARGET_KERNEL_ANDROID)
		{
			szCompiledTarget = CUtlString("%s%u", szTarget.GetString(), pProject->m_androidmanifest.m_nTargetVersion);
		}
		CUtlString szOutputFile = CUtlString("%s/%s/cc/%u_%s/%s/%s.o",FPC_TEMPORAL_DIRNAME, szTarget.GetString(), hash, pProject->m_szName.GetString(), filesystem2->BuildDirectory(), file.GetString());

		args = {
			"-target",
			szCompiledTarget,
		};
		*/
		
		/*
		if (!strcmp(Plat_GetExtension(file),"cpp"))
			args.AppendTail("-std=c++17"); 
		else if (!strcmp(Plat_GetExtension(file),"mm"))
			;
		else
			args.AppendTail("-std=c99");
		*/
		
		args = BuildCommandLine(pProject, file, szOutputFile);

		/*
		if (pProject->m_target.kernel == TARGET_KERNEL_DARWIN)
		{
			args.AppendTail("-isysroot");
			args.AppendTail("/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk");
		} else if (pProject->m_target.kernel == TARGET_KERNEL_IOS)
		{
			args.AppendTail("-isysroot");
			args.AppendTail("/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS.sdk");
			args.AppendTail("-miphoneos-version-min=18.0 ");
			args.AppendTail("-fembed-bitcode");
		}
		*/

		if (!filesystem2->ShouldRecompile(file, szOutputFile))
			goto skipcompile;
		else
			V_printf("  CC       %s\n", file.GetString());

		runner->Run(GetCompilerExecutable(pProject), args);
skipcompile:
		proj.objects.AppendTail((Object_t){szOutputFile});

		ClangFile_t cfile = {};
		cfile.m_szName = file;
		cfile.m_szArguments = args;
		cfile.m_szArguments.AppendHead(GetCompilerExecutable(pProject));

		g_clangFiles.AppendTail(cfile);
	}
	runner->Wait();	
	return proj;
}

void CClangCompiler::GenerateLinterData()
{
	CUtlVector<IJSONValue*> jsonValues = {};
	FILE* f = V_fopen("compile_commands.json", "wb");
	for ( auto &f: g_clangFiles )
	{
		IJSONObject *pFileObject = JSONManager()->CreateObject();
		IJSONValue *pFileValue = JSONManager()->CreateValue();
		IJSONArray *pArgumentFiles = JSONManager()->CreateArray();
		IJSONValue *pArgumentsValue = JSONManager()->CreateValue();
		IJSONValue *pFileNameValue = JSONManager()->CreateValue();
		IJSONValue *pDirectoryValue = JSONManager()->CreateValue();
		CUtlVector<IJSONValue*> values;

		for (auto &arg: f.m_szArguments)
		{
			IJSONValue *pFileValue = JSONManager()->CreateValue();
			pFileValue->SetStringValue(arg.GetString());
			values.AppendTail(pFileValue);
		}
		pArgumentFiles->SetArray(values.GetSize(), values.GetData());
		pArgumentsValue->SetArrayValue(pArgumentFiles);
		pFileNameValue->SetStringValue(f.m_szName.GetString());
		pDirectoryValue->SetStringValue(filesystem2->BuildDirectory());
		pFileObject->SetValue("arguments", pArgumentsValue);
		pFileObject->SetValue("file", pFileNameValue);
		pFileObject->SetValue("directory", pDirectoryValue);
		pFileValue->SetObjectValue(pFileObject);
		jsonValues.AppendTail(pFileValue);
	};
	IJSONArray *pArray = JSONManager()->CreateArray();
	pArray->SetArray(jsonValues.GetSize(), jsonValues.GetData());
	IJSONValue *pRoot = JSONManager()->CreateValue();
	pRoot->SetArrayValue(pArray);
	CUtlString szCommands = JSONManager()->WriteString(pRoot);
	V_fprintf(f, szCommands.GetString());
	V_fclose(f);
};
