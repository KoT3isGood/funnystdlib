#include "helper.h"
#include "runner.h"
#include "tier0/platform.h"
#include "tier1/utlvector.h"
#include "tier1/utlstring.h"
#include "unistd.h"
#include "libgen.h"
#include "sys/stat.h"
#include "tier1/interface.h"
#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

unsigned int g_hashState = 102851263;
unsigned int BaseProject_t::GenerateProjectHash( void )
{
	unsigned int hash = 5381+g_hashState;
	int c;
	char *szName = m_szName;

	while( (c = *szName++) )
		hash = (hash * 33) + c;

	g_hashState = g_hashState * 1664525 + 1013904223;

	return hash;
};

static char path[1024];
#ifdef __linux__
static ssize_t pathSize = readlink("/proc/self/exe", path, sizeof(path) - 1);
#endif
#ifdef __APPLE__
static uint32_t pathSize = sizeof(path);
static int pathResult = _NSGetExecutablePath(path, &pathSize);
#endif
static char *szPathDir = dirname(path);
static char *s_szBuildDir = 0;
EXPOSE_INTERFACE_GLOBALVAR(szBuildDir, char*, FILE_SYSTEM_2_BUILD_DIRECTORY_INTERFACE_VERSION, s_szBuildDir);

class CPOSIXFileSystem2: public IFileSystem2
{
public:
	virtual char *OwnDirectory() override;
	virtual char *BuildDirectory() override;
	virtual void MakeDirectory( const char *psz ) override;
	virtual void CopyFile( const char *szDestination, const char *szOrigin ) override;
	virtual void CopyDirectory( const char *szDestination, const char *szOrigin ) override;
	virtual bool ShouldRecompile( const char *szSource, const char *szOutput ) override;
	virtual char *GetWindowsPath( const char *szPath ) override;
	virtual char *GetPOSIXPath( const char *szPath ) override;
};

char *CPOSIXFileSystem2::GetWindowsPath( const char *szPath )
{
	char *szNewPath = (char*)V_malloc(V_strlen(szPath)+1);
	int i = 0;
	V_strcpy(szNewPath, szPath);
	while(szNewPath[i])
	{
		if (szNewPath[i] == '/')
			szNewPath[i] = '\\';
		i++;
	}
	return szNewPath;
}

char *CPOSIXFileSystem2::GetPOSIXPath( const char *szPath )
{
	char *szNewPath = (char*)V_malloc(V_strlen(szPath)+1);
	int i = 0;
	V_strcpy(szNewPath, szPath);
	while(szNewPath[i])
	{
		if (szNewPath[i] == '\\')
			szNewPath[i] = '/';
		i++;
	}
	return szNewPath;
}

EXPOSE_INTERFACE(CPOSIXFileSystem2, IFileSystem2, FILE_SYSTEM_2_INTERFACE_NAME);
IFileSystem2 *filesystem2;

char *CPOSIXFileSystem2::OwnDirectory()
{
	return szPathDir;
};
char *CPOSIXFileSystem2::BuildDirectory()
{
	return s_szBuildDir;
};
	
void CPOSIXFileSystem2::CopyFile( const char *szDestination, const char *szOrigin )
{
#ifdef POSIX
	CUtlVector<CUtlString> args = {
		szOrigin,
		szDestination,
	};
	runner->Run(CUtlString("cp"), args);
	runner->Wait();	
#endif
#ifdef __WIN32__
	CUtlVector<CUtlString> args = {
		"/Y",
		szOrigin,
		szDestination,
	};
	runner->Run(CUtlString("xcopy"), args);
	runner->Wait();	
#endif
}
void CPOSIXFileSystem2::CopyDirectory( const char *szDestination, const char *szOrigin )
{
#ifdef POSIX
	CUtlVector<CUtlString> args = {
		"-r",
		szOrigin,
		szDestination,
	};
	runner->Run(CUtlString("cp"), args);
	runner->Wait();	
#endif
#ifdef __WIN32__
	CUtlVector<CUtlString> args = {
		"/Y",
		"/E",
		szOrigin,
		szDestination,
	};
	runner->Run(CUtlString("xcopy"), args);
	runner->Wait();	
#endif
}

void CPOSIXFileSystem2::MakeDirectory( const char *psz )
{
#ifdef POSIX
	CUtlVector<CUtlString> args = {
		"-p",
		psz
	};
	runner->Run("mkdir", args);
	runner->Wait();	
#endif
#ifdef __WIN32__
	CUtlVector<CUtlString> args = {
		psz
	};
	runner->Run("mkdir", args);
	runner->Wait();	
#endif
};

bool CPOSIXFileSystem2::ShouldRecompile(const char *szSource, const char *szOutput)
{
	struct stat srcbuf;
	struct stat outbuf;
	if (stat(szSource, &srcbuf) != 0) {
		return true;
	}
	if (stat(szOutput, &outbuf) != 0) {
		return true;
	}
	return outbuf.st_mtime < srcbuf.st_mtime;
};


IINIFile *g_pConfig;
EXPOSE_INTERFACE_GLOBALVAR(IINIFile, IINIFile, LIBFPC_CONFIG_INTERFACE_VERSION, g_pConfig);
