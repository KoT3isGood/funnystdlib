#include "rust.h"
#include "tier2/fileformats/json.h"
#include "config.h"

struct RustCrate_t
{
	CUtlString m_szRoot;
	ERustEdition m_eEdition;
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
		s_crates.AppendTail({pProject->m_szRoot, pProject->m_eEdition});
	}
	proj.objects.AppendTail((Object_t){szOutputFile});
	

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
	cmd.AppendTail("--crate-type=staticlib");
	cmd.AppendTail("-o");
	cmd.AppendTail(szOutputFileName);
	cmd.AppendTail(szFileName);
	cmd.AppendTail("--target");
	cmd.AppendTail(pProject->m_target.GetTriplet());
	if (pProject->m_target.optimization == TARGET_DEBUG)
	{
		cmd.AppendTail("-g");
	}

	for ( auto &c: pProject->m_codegen )
	{
		cmd.AppendTail("-C");
		if (c.szValue)
			cmd.AppendTail(CUtlString("%s=%s", c.szName, c.szValue).GetString());
		else
			cmd.AppendTail(c.szName);
	}


	return cmd;
}

const char *CRustCompiler::GetOutputObjectFormat( RustProject_t *pProject )
{
	if (pProject->m_target.abi == TARGET_ABI_MSVC)
		return ".lib";
	return ".a";
}

CUtlString CRustCompiler::GetOutputObjectName( RustProject_t *pProject, unsigned int hash, CUtlString szFileName )
{
	CUtlString szTarget = pProject->m_target.GetTriplet();

	return CUtlString(
			"%s/%s/rustc/%s/%s/%s%s",
			FPC_TEMPORAL_DIRNAME, 
			szTarget.GetString(), 
			pProject->m_szName.GetString(), 
			filesystem2->BuildDirectory(), 
			szFileName.GetString(), 
			GetOutputObjectFormat(pProject));
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
			
		pObject->SetValue("root_module", pVRoot);
		pObject->SetValue("edition", pVEdition);
		pObject->SetValue("deps", pVDeps);
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
