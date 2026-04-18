#ifndef FPC_RUST_H
#define FPC_RUST_H
#include "helper.h"
#include "ld.h"

enum ERustEdition
{
	k_ERustEdition_Default = 0,
	k_ERustEdition_2015,
	k_ERustEdition_2018,
	k_ERustEdition_2021,
	k_ERustEdition_2024,
	k_ERustEdition_Future,
};

enum ERustLink
{
	k_ERustLink_Default = 0,
	k_ERustLink_Bin,
	k_ERustLink_Lib,
	k_ERustLink_DyLib,
	k_ERustLink_StaticLib,
	k_ERustLink_CDyLib,
	k_ERustLink_RLib,
	k_ERustLink_Proc_Macro,
};

struct RustValueC_t
{
	const char *szName;
	const char *szValue;
};

struct RustValue_t
{
	CUtlString szName;
	CUtlString szPath;
};

struct RustProject_t: public CPUProject_t
{
	CUtlString m_szRoot;
	CUtlVector<CUtlString> m_files = {};
	const char *m_szCrateName;
	ERustEdition m_eEdition;
	ERustLink m_eLink;
	CUtlVector<RustValueC_t> m_cfg;
	CUtlVector<RustValueC_t> m_codegen;
	CUtlVector<RustValue_t> m_externs;
	CUtlVector<CUtlString> m_libraryDirectories;
};

class CRustCompiler
{
public:
	LinkProject_t Compile( RustProject_t *pProject );
	virtual void GenerateLinterData();
private:
	CUtlVector<CUtlString> BuildCommandLine( RustProject_t *pProject, const char *szFileName, const char *szOutputFileName );
	CUtlString GetOutputObjectName( RustProject_t *pProject, unsigned int hash, CUtlString szFileName );
	const char *GetCompilerExecutable( RustProject_t *pProject );
	const char *GetOutputObjectFormat( RustProject_t *pProject );

};

#define RUST_COMPILER_INTERFACE_VERSION "RustCompiler001"

#endif
