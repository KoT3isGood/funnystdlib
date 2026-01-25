#include "swift.h"

class CSwiftCompiler: public ISwiftCompiler
{
public:

	// Compiles all files into objects, returns linker project,
	// which can be linked into executable or library.
	//virtual LinkProject_t Compile( SwiftProject_t *pProject );
	
	//virtual void GenerateLinterData() ;
	
protected:
	// Compiler internals

	virtual CUtlVector<CUtlString> BuildCommandLine( SwiftProject_t *pProject, const char *szFileName, const char *szOutputFileName ) override;
	
	// Returns executable which should the OS run
	virtual const char *GetCompilerExecutable( SwiftProject_t *pProject ) override;

	// returns object file format, eg .obj or .o
	virtual const char *GetOutputObjectFormat() override;

	virtual void IncludeDirectory( CUtlVector<CUtlString> &cmd, const char *szName ) override;
	virtual void IncludeFile( CUtlVector<CUtlString> &cmd, const char *szName ) override;
	virtual void Macro( CUtlVector<CUtlString> &cmd, const char *szName ) override;
	virtual void Macro( CUtlVector<CUtlString> &cmd, const char *szName, const char *szValue ) override;
	
	virtual void SetTarget( CUtlVector<CUtlString> &cmd, SwiftProject_t *pProject ) override;
	virtual void SetSysroot( CUtlVector<CUtlString> &cmd, SwiftProject_t *pProject , const char *szSysroot ) override;
	virtual void SetOutputFile( CUtlVector<CUtlString> &cmd, const char *szName ) override;
	
	virtual void CompileFile( CUtlVector<CUtlString> &cmd, const char *szName ) override;

	virtual void EnableDebugSymbols( CUtlVector<CUtlString> &cmd ) override;
	virtual void EnablePIE( CUtlVector<CUtlString> &cmd ) override;
	virtual void EnablePIC( CUtlVector<CUtlString> &cmd ) override;
};

const char *CSwiftCompiler::GetOutputObjectFormat()
{
	return ".o";
}

CUtlVector<CUtlString> CSwiftCompiler::BuildCommandLine( SwiftProject_t *pProject, const char *szFileName, const char *szOutputFileName )
{
	CUtlVector<CUtlString> cmd;
	cmd = ISwiftCompiler::BuildCommandLine(pProject, szFileName, szOutputFileName);
	cmd.AppendHead("-c");
	return cmd;
}


const char *CSwiftCompiler::GetCompilerExecutable( SwiftProject_t *pProject )
{
	return "swiftc";
}


void CSwiftCompiler::IncludeDirectory( CUtlVector<CUtlString> &cmd, const char *szName )
{
	cmd.AppendTail("-I");
	cmd.AppendTail(szName);
}

void CSwiftCompiler::IncludeFile( CUtlVector<CUtlString> &cmd, const char *szName )
{
}

void CSwiftCompiler::Macro( CUtlVector<CUtlString> &cmd, const char *szName )
{
}

void CSwiftCompiler::Macro( CUtlVector<CUtlString> &cmd, const char *szName, const char *szValue )
{
	cmd.AppendTail("-D");
	cmd.AppendTail(CUtlString("%s=%s", (char*)szName, (char*)szValue));
}

void CSwiftCompiler::EnableDebugSymbols( CUtlVector<CUtlString> &cmd )
{
	cmd.AppendTail("-g");
}

void CSwiftCompiler::SetTarget( CUtlVector<CUtlString> &cmd, SwiftProject_t *pProject )
{
	cmd.AppendTail("-target");
	cmd.AppendTail(pProject->m_target.GetTriplet());
}

void CSwiftCompiler::CompileFile( CUtlVector<CUtlString> &cmd, const char *szName )
{
	cmd.AppendTail(szName);
}
void CSwiftCompiler::SetOutputFile( CUtlVector<CUtlString> &cmd, const char *szName )
{
	cmd.AppendTail("-o");
	cmd.AppendTail(szName);
}
void CSwiftCompiler::EnablePIE( CUtlVector<CUtlString> &cmd )
{
	cmd.AppendTail("-fPIE");
}

void CSwiftCompiler::EnablePIC( CUtlVector<CUtlString> &cmd )
{
	cmd.AppendTail("-fPIC");
}
void CSwiftCompiler::SetSysroot( CUtlVector<CUtlString> &cmd, SwiftProject_t *pProject, const char *szName )
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

EXPOSE_INTERFACE(CSwiftCompiler, ISwiftCompiler, SWIFT_COMPILER_INTERFACE_VERSION)

