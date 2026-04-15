#include "rust.h"
#include "tier2/fileformats/json.h"
#include "config.h"

struct RustCrate_t
{
	CUtlString m_szName;
	CUtlString m_szRoot;
	bool m_bIsProcMacro;
	ERustEdition m_eEdition;
	CUtlVector<ExternRustCrate_t> m_externs;
};
static CUtlVector<RustCrate_t> s_crates;

COMPILER_LANGUAGE(rust, 0, "rust")
COMPILER_VALUE(rust, rustc, "rustc", "rustc executable");

LinkProject_t CRustCompiler::Compile( RustProject_t *pProject )
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


	CUtlString szOutputFile = GetOutputObjectName(pProject, hash, pProject->m_szRoot);

	filesystem2->MakeDirectory(szOutputFile.GetDirectory());


	bool bRecompile = false;
	// Run Rust
	for (auto &file: pProject->m_files)
	{
		if (filesystem2->ShouldRecompile(file, szOutputFile))
			bRecompile = true;
	}
	if (bRecompile)
	{
		V_printf("  RUSTC    %s\n", pProject->m_szRoot.GetString());
		CUtlVector<CUtlString> args;
		args = BuildCommandLine(pProject, pProject->m_szRoot, szOutputFile);
		runner->Run(GetCompilerExecutable(pProject), args);
		runner->Wait();	
	}
	proj.objects.AppendTail((Object_t){szOutputFile});
<<<<<<< HEAD

=======
>>>>>>> cf88683 (added json flags)
	s_crates.AppendTail({
			pProject->m_szName,
			pProject->m_szRoot, 
			pProject->m_eLink == k_ERustLink_Proc_Macro ? true : false, 
			pProject->m_eEdition, 
			pProject->m_externs});
	

	return proj;

}

CUtlVector<CUtlString> CRustCompiler::BuildCommandLine( RustProject_t *pProject, const char *szFileName, const char *szOutputFileName )
{
	CUtlVector<CUtlString> cmd;
	cmd.AppendTail("--edition");
	switch(pProject->m_eEdition)
	{
	case k_ERustEdition_Default: cmd.AppendTail("latest"); break;
	case k_ERustEdition_2015: cmd.AppendTail("2015"); break;
	case k_ERustEdition_2018: cmd.AppendTail("2018"); break;
	case k_ERustEdition_2021: cmd.AppendTail("2021"); break;
	case k_ERustEdition_2024: cmd.AppendTail("2024"); break;
	case k_ERustEdition_Future: cmd.AppendTail("latest"); break;
	}
	switch(pProject->m_eLink)
	{
	case k_ERustLink_Default: break;
	case k_ERustLink_Bin: cmd.AppendTail("--crate-type=bin"); break;
	case k_ERustLink_Lib: cmd.AppendTail("--crate-type=lib"); break;
	case k_ERustLink_DyLib: cmd.AppendTail("--crate-type=dylib"); break;
	case k_ERustLink_StaticLib: cmd.AppendTail("--crate-type=staticlib"); break;
	case k_ERustLink_CDyLib: cmd.AppendTail("--crate-type=cdylib"); break;
	case k_ERustLink_RLib: cmd.AppendTail("--crate-type=rlib"); break;
	case k_ERustLink_Proc_Macro: cmd.AppendTail("--crate-type=proc-macro"); break;
	}
	cmd.AppendTail("-o");
	cmd.AppendTail(szOutputFileName);
	cmd.AppendTail(szFileName);
	if (pProject->m_eLink != k_ERustLink_Proc_Macro)
	{
		cmd.AppendTail("--target");
		cmd.AppendTail(pProject->m_target.GetTriplet());
	}
	if (pProject->m_target.optimization == TARGET_DEBUG)
	{
		cmd.AppendTail("-g");
	}

	for ( auto &d: pProject->m_libraryDirectories )
	{
		cmd.AppendTail("-L");
		cmd.AppendTail(d.GetString());
	}

	for ( auto &c: pProject->m_codegen )
	{
		cmd.AppendTail("-C");
		if (c.szValue)
			cmd.AppendTail(CUtlString("%s=%s", c.szName, c.szValue).GetString());
		else
			cmd.AppendTail(c.szName);
	}

	for ( auto &e: pProject->m_externs )
	{
		cmd.AppendTail("--extern");
		cmd.AppendTail(CUtlString("%s=%s", e.szName.GetString(), e.szPath.GetString()).GetString());
	}


	return cmd;
}

const char *CRustCompiler::GetOutputObjectFormat( RustProject_t *pProject )
{
	switch (pProject->m_eLink)
	{

	case k_ERustLink_Default: goto bin;
	case k_ERustLink_Bin: goto bin;
	case k_ERustLink_Lib: goto lib;
	case k_ERustLink_DyLib: goto dll;
	case k_ERustLink_StaticLib: goto lib;
	case k_ERustLink_CDyLib: goto dll;
	case k_ERustLink_RLib: return "rlib";
	case k_ERustLink_Proc_Macro: goto dll; 

	}
bin:
	return pProject->m_target.GetExecutableFileFormat();
dll:
	return pProject->m_target.GetDynamicLibraryFileFormat();
lib:
	return pProject->m_target.GetStaticLibraryFileFormat();
}

CUtlString CRustCompiler::GetOutputObjectName( RustProject_t *pProject, unsigned int hash, CUtlString szFileName )
{
	CUtlString szTarget = pProject->m_target.GetTriplet();
	CUtlString szLib = CUtlString(GetOutputObjectFormat(pProject),pProject->m_szName.GetString());
	szLib.AppendHead("/");
	szLib.AppendHead(szFileName.GetDirectory());

	return CUtlString(
			"%s/%s/rustc/%s/%s/%s",
			FPC_TEMPORAL_DIRNAME, 
			szTarget.GetString(), 
			pProject->m_szName.GetString(), 
			filesystem2->BuildDirectory(), 
			szLib.GetString()
			);
}

const char *CRustCompiler::GetCompilerExecutable( RustProject_t *pProject )
{
	return "rustc";
}
void CRustCompiler::GenerateLinterData()
{
	if (s_crates.GetSize()==0)
		return;
	CUtlVector<IJSONValue*> jsonValues = {};
	FILE* f = V_fopen("rust-project.json", "wb");
	for ( auto &c: s_crates )
	{
		IJSONValue *pVObject = JSONManager()->CreateValue();
		IJSONObject *pObject = JSONManager()->CreateObject();
		IJSONValue *pVRoot = JSONManager()->CreateValue();
		IJSONValue *pVEdition = JSONManager()->CreateValue();
		IJSONValue *pVDeps = JSONManager()->CreateValue();
		IJSONArray *pDeps = JSONManager()->CreateArray();
		IJSONValue *pVIsProcMacro = JSONManager()->CreateValue();
		V_printf("%s\n",c.m_szName.GetString());
		
		CUtlVector<IJSONValue*> deps = {};
		for ( auto e: c.m_externs )
		{
			V_printf("%s\n",c.m_szName.GetString());

			int index = -1;
			for ( int i = 0; i < s_crates.GetSize(); i++)
			{
				if (s_crates[i].m_szName == e.szName)
				{
					index = i;
					break;
				}
			}
			if (index<0)
				return;

			IJSONValue *pVName = JSONManager()->CreateValue();
			IJSONValue *pVIndex = JSONManager()->CreateValue();
			IJSONObject *pDep = JSONManager()->CreateObject();
			IJSONValue *pVDep = JSONManager()->CreateValue();


			pVName->SetStringValue(e.szName);
			pVIndex->SetNumberValue(index);
			pVIndex->SetFlag(k_EJSON_Integer);
			pDep->SetValue("name", pVName);
			pDep->SetValue("crate", pVIndex);
			pVDep->SetObjectValue(pDep);
			deps.AppendTail(pVDep);

		}
		pDeps->SetArray(deps.GetSize(), deps.GetData());


		switch(c.m_eEdition)
		{
			case k_ERustEdition_Default: pVEdition->SetStringValue("latest"); break;
			case k_ERustEdition_2015: pVEdition->SetStringValue("2015"); break;
			case k_ERustEdition_2018: pVEdition->SetStringValue("2018"); break;
			case k_ERustEdition_2021: pVEdition->SetStringValue("2021"); break;
			case k_ERustEdition_2024: pVEdition->SetStringValue("2024"); break;
			case k_ERustEdition_Future: pVEdition->SetStringValue("latest"); break;
		}
		pVRoot->SetStringValue(c.m_szRoot.GetAbsolute());	
		pVDeps->SetArrayValue(pDeps);
		pVIsProcMacro->SetBooleanValue(c.m_bIsProcMacro);
			
		pObject->SetValue("root_module", pVRoot);
		pObject->SetValue("edition", pVEdition);
		pObject->SetValue("deps", pVDeps);
		pObject->SetValue("is_proc_macro", pVIsProcMacro);
		pVObject->SetObjectValue(pObject);
		jsonValues.AppendTail(pVObject);

	};
	IJSONArray *pCrates = JSONManager()->CreateArray();
	pCrates->SetArray(jsonValues.GetSize(), jsonValues.GetData());
	IJSONValue *pVCrates = JSONManager()->CreateValue();
	pVCrates->SetArrayValue(pCrates);
	IJSONObject *pObject = JSONManager()->CreateObject();
	pObject->SetValue("crates", pVCrates);

	IJSONValue *pRoot = JSONManager()->CreateValue();
	pRoot->SetObjectValue(pObject);
	CUtlString szCommands = JSONManager()->WriteString(pRoot);
	JSONManager()->FreeValue(pRoot);
	V_fwrite(szCommands.GetString(), 1, szCommands.GetLenght(), f);
	V_fclose(f);
};

static CRustCompiler s_rust_compiler;
EXPOSE_INTERFACE_GLOBALVAR(CRustCompiler, CRustCompiler, RUST_COMPILER_INTERFACE_VERSION, s_rust_compiler)
