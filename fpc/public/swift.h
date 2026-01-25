#ifndef SWIFT_H
#define SWIFT_H

#include "tier0/platform.h"
#include "tier1/utlstring.h"
#include "tier1/utlvector.h"
#include "runner.h"
#include "ld.h"
#include "c.h"
#include "target.h"
#include "helper.h"

struct SwiftProject_t : public CPUProject_t
{
public:
	// Compiled files
	CUtlVector<CUtlString> files = {};

	// Included directories
	CUtlVector<CUtlString> includeDirectories = {};
	
	// Included files
	CUtlVector<CUtlString> includeObjcFiles = {};

	// Stuff for embedded objc
	// Included files
	// They are included on top of the file
	CUtlVector<CUtlString> includeFiles = {};

	// Stuff for embedded objc
	// Defined macros
	CUtlVector<C_Macro_t> macros = {};

	// Stuff for embedded objc
	// Target C version
	ECVersion cVersion;
	
	// Stuff for embedded objc
	// Target C++ version
	ECPPVersion cppVersion;
};

#define SWIFT_COMPILER_INTERFACE_VERSION "SwiftCompiler001"

class ISwiftCompiler
{
public:

	// Compiles all files into objects, returns linker project,
	// which can be linked into executable or library.
	virtual LinkProject_t Compile( SwiftProject_t *pProject );
	
	//virtual void GenerateLinterData() = 0;
protected:
	// Compiler internals
	
	// Returns file name of the
	CUtlString GetOutputObjectName( SwiftProject_t *pProject, unsigned int hash, CUtlString szFileName );

	virtual CUtlVector<CUtlString> BuildCommandLine( SwiftProject_t *pProject, const char *szFileName, const char *szOutputFileName );
	
	// Returns executable which should the OS run
	virtual const char *GetCompilerExecutable( SwiftProject_t *pProject ) = 0;

	// returns object file format, eg .obj or .o
	virtual const char *GetOutputObjectFormat() = 0;

	virtual void IncludeDirectory( CUtlVector<CUtlString> &cmd, const char *szName ) = 0;
	virtual void IncludeFile( CUtlVector<CUtlString> &cmd, const char *szName ) = 0;
	virtual void Macro( CUtlVector<CUtlString> &cmd, const char *szName ) = 0;
	virtual void Macro( CUtlVector<CUtlString> &cmd, const char *szName, const char *szValue ) = 0;
	
	virtual void SetTarget( CUtlVector<CUtlString> &cmd, SwiftProject_t *pProject ) = 0;
	virtual void SetSysroot( CUtlVector<CUtlString> &cmd, SwiftProject_t *pProject , const char *szSysroot ) = 0;
	virtual void SetOutputFile( CUtlVector<CUtlString> &cmd, const char *szName ) = 0;
	
	virtual void CompileFile( CUtlVector<CUtlString> &cmd, const char *szName ) = 0;

	virtual void EnableDebugSymbols( CUtlVector<CUtlString> &cmd ) = 0;
	virtual void EnablePIE( CUtlVector<CUtlString> &cmd ) = 0;
	virtual void EnablePIC( CUtlVector<CUtlString> &cmd ) = 0;	
};


extern ISwiftCompiler *swiftcompiler;

#endif
