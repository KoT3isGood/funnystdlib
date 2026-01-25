//================= Copyright kotofyt, All rights reserved ==================//
// Purpose: Helper functions for compilers, filesystem2 and build stages.
//===========================================================================//

#ifndef HELPER_H
#define HELPER_H

#include "apktool.h"
#include "tier1/utlstring.h"
#include "target.h"
#include "tier2/fileformats/ini.h"
#include "tier1/interface.h"

#define FPC_TEMPORAL_DIRNAME ".fpc"

//-----------------------------------------------------------------------------
// A base for all projects
//-----------------------------------------------------------------------------
struct BaseProject_t
{
public:
	CUtlString m_szName;
	// Creates a hash for the project
	unsigned int GenerateProjectHash( void );
};

//-----------------------------------------------------------------------------
// A base for cpu projects
//-----------------------------------------------------------------------------
struct CPUProject_t : public BaseProject_t
{
public:
	Target_t m_target = Target_t::DefaultTarget();

	// Is compiled as position independent executable
	bool bFPIE = false;
	
	// Is compiled as position independent code
	// Use for shared libraries
	bool bFPIC = false;

	// TODO: rework manifests
	// Android manifest
	AndroidManifest_t m_androidmanifest = {};
};

//-----------------------------------------------------------------------------
// A base for shader projects
//-----------------------------------------------------------------------------
struct ShaderProject_t : public BaseProject_t
{
public:
	EShaderTarget m_eTarget;
};

//-----------------------------------------------------------------------------
// File system manager.
//-----------------------------------------------------------------------------
#define FILE_SYSTEM_2_INTERFACE_NAME "FileSystem2_001"
#define FILE_SYSTEM_2_BUILD_DIRECTORY_INTERFACE_VERSION "FileSystem2BuildDirectory_001"

abstract_class IFileSystem2
{
public:
	// Returns a directory of fpc executable
	virtual char *OwnDirectory() = 0;

	// Returns directory of build.cpp
	virtual char *BuildDirectory() = 0;

	// Creates new directory at path
	virtual void MakeDirectory( const char *psz ) = 0;
	
	// UNIX-style file copy
	virtual void CopyFile( const char *szDestination, const char *szOrigin ) = 0;
	
	// UNIX-style recursive directory copy
	virtual void CopyDirectory( const char *szDestination, const char *szOrigin ) = 0;

	// Compares timestamps of 2 files
	virtual bool ShouldRecompile( const char *szSource, const char *szOutput ) = 0;

	virtual char *GetWindowsPath( const char *szPath ) = 0;
	virtual char *GetPOSIXPath( const char *szPath ) = 0;
};

extern IFileSystem2 *filesystem2;

struct StageOutput_t
{
	const char *m_szName;
	CUtlString m_szPath;
};

//-----------------------------------------------------------------------------
// Build stage.
//-----------------------------------------------------------------------------
class CBuildStage
{
public:
	CBuildStage( const char *psz, int(*pMainFn)() );
	const char *m_psz;
	CUtlString m_szPath;
	CUtlVector<StageOutput_t> m_outputs;
	int(*m_pMainFn)();
};


class CBuildDependentFile
{
public:
	CBuildDependentFile( const char *psz );
};

//-----------------------------------------------------------------------------
// Declares new build stage.
// example:
//	DECLARE_BUILD_STAGE(your_build_stage_name)
//	{
//		return 0;
//	}
//-----------------------------------------------------------------------------
#define DECLARE_BUILD_STAGE(sz) \
static int __build_stage_##sz(); \
static CBuildStage __##sz##_build_stage(#sz, __build_stage_##sz); \
static int __build_stage_##sz()

#define ADD_DEPENDENCY_BUILD_FILE(name, path) \
static CBuildDependentFile __##name##DependencyFile(path);

#define ADD_OUTPUT_OBJECT(sz, path) \
	g_pCurrentStage->m_outputs.AppendTail((StageOutput_t){sz, path});

#define DEPEND_ON_PROJECT(sz) \

#define GET_PROJECT_LIBRARY(sz, szLib) \
	FPC_GetProjectObject(#sz, szLib)

#define GET_PROJECT_OBJECT(sz, szLib) \
	FPC_GetProjectObject(#sz, szLib)
	

struct BuildFileInfo_t
{
	CUtlVector<const char*> m_dependantFiles;
	CUtlVector<CBuildStage*> m_stages;
};

BuildFileInfo_t *GetBuildFileInfo();
extern CBuildStage *g_pCurrentStage;

CUtlString FPC_GetProjectObject( const char *szName, const char *szObjectName );


#define BUILD_FILE_INFO_INTERFACE_VERSION "BuildFileInfo001"
#define BUILD_CURRENT_STAGE_INTERFACE_VERSION "BuildCurrentStage001"

typedef CUtlString(*GetProjectObjectFn)( const char *szName, const char *szObjectName );
#define BUILD_GET_PROJECT_OBJECT_INTERFACE_VERSION "GetProjectObject001"

#define LIBFPC_INIT_INTERFACE_VERSION "LibFPCInit001"


extern IINIFile *g_pConfig;
#define LIBFPC_CONFIG_INTERFACE_VERSION "LibFPCConfig001"

#endif
