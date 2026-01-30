#include "ld.h"
#include "helper.h"
#include "libgen.h"
#include "target.h"
#include "tier0/platform.h"
#include "tier1/interface.h"
#include "tier1/utlstring.h"
#include "tier2/fileformats/ini.h"
#include "winerunner.h"

class CMSVCLinker : public ILinker
{
public:
	virtual bool IsLibraryExists( CUtlString szName ) override;
protected:
	// Returns executable which should the OS run
	virtual const char *GetCompilerExecutable( LinkProject_t *pProject ) override;
	
	virtual void SetTarget( CUtlVector<CUtlString> &cmd, LinkProject_t *pProject ) override;
	virtual void SetSysroot( CUtlVector<CUtlString> &cmd, LinkProject_t *pProject , const char *szSysroot ) override;
	virtual void SetOutputFile( CUtlVector<CUtlString> &cmd, const char *szOutput) override;


	// sets rpath
	// for windows should be ignored
	virtual void SetDefaultLibraryPaths( CUtlVector<CUtlString> &cmd, LinkProject_t *pProject ) override;

	virtual void UseStdLib( CUtlVector<CUtlString> &cmd, bool bUse ) override;

	// windows doesn't use it as well
	virtual void UseDynamicLookup( CUtlVector<CUtlString> &cmd, bool bUse ) override;

	// includes whole file
	virtual void UseFullFile( CUtlVector<CUtlString> &cmd, LinkProject_t *pProject  ) override;

	// includes used stuff in a file
	virtual void UsePartialFile( CUtlVector<CUtlString> &cmd, LinkProject_t *pProject  ) override;
	
	virtual void LinkFile( CUtlVector<CUtlString> &cmd, const char *szName ) override;
	virtual void LinkLibraryObject( CUtlVector<CUtlString> &cmd, const char *szName ) override;
	virtual void LinkLibrary( CUtlVector<CUtlString> &cmd, const char *szName ) override;
	virtual void LinkLibraryPath( CUtlVector<CUtlString> &cmd, const char *szName ) override;
};

const char *CMSVCLinker::GetCompilerExecutable( LinkProject_t *pProject )
{
	IINISection *pSection = NULL;
	const char *szLinker = "link";
	if (!g_pConfig)
		return szLinker;


	pSection = g_pConfig->GetSection(pProject->m_target.GetTriplet());
	if (!pSection)
		return szLinker;


	szLinker = pSection->GetStringValue("MSVC_LINKER_INTERFACE_NAME");
	if (szLinker == NULL)
		return "link";
	return szLinker;
}

void CMSVCLinker::SetTarget( CUtlVector<CUtlString> &cmd, LinkProject_t *pProject )
{
	if (pProject->m_target.kernel != TARGET_KERNEL_WINDOWS)
		return;
	switch (pProject->m_eWindowsSubsystem)
	{
	case WINDOWS_SUBSYSTEM_NATIVE:
		cmd.AppendTail("/subsystem:native");
	default:
		break;
	}
	switch (pProject->linkType)
	{
	case ELINK_DYNAMIC_LIBRARY:
	case ELINK_STATIC_LIBRARY:
	case ELINK_EXECUTABLE:
		break;
	case ELINK_KERNEL_DRIVER:
		cmd.AppendTail("/driver");
		cmd.AppendTail(CUtlString("/entry:%s", pProject->szEntry));
		break;
	}
}

void CMSVCLinker::SetSysroot( CUtlVector<CUtlString> &cmd, LinkProject_t *pProject , const char *szSysroot )
{

}

void CMSVCLinker::SetOutputFile( CUtlVector<CUtlString> &cmd, const char *szName )
{

}

void CMSVCLinker::SetDefaultLibraryPaths( CUtlVector<CUtlString> &cmd, LinkProject_t *pProject )
{

}


void CMSVCLinker::UseStdLib( CUtlVector<CUtlString> &cmd, bool bUse )
{

}


void CMSVCLinker::UseDynamicLookup( CUtlVector<CUtlString> &cmd, bool bUse )
{

}


void CMSVCLinker::UseFullFile( CUtlVector<CUtlString> &cmd, LinkProject_t *pProject  )
{

}


void CMSVCLinker::UsePartialFile( CUtlVector<CUtlString> &cmd, LinkProject_t *pProject  )
{

}


void CMSVCLinker::LinkFile( CUtlVector<CUtlString> &cmd, const char *szName )
{
	cmd.AppendTail(szName);
}

void CMSVCLinker::LinkLibraryObject( CUtlVector<CUtlString> &cmd, const char *szName )
{
	cmd.AppendTail(szName);
}

void CMSVCLinker::LinkLibrary( CUtlVector<CUtlString> &cmd, const char *szName )
{
	cmd.AppendTail(szName);
}

void CMSVCLinker::LinkLibraryPath( CUtlVector<CUtlString> &cmd, const char *szName )
{
	cmd.AppendTail(CUtlString("/libpath:%s", szName));

}

EXPOSE_INTERFACE(CMSVCLinker, ILinker, MSVC_LINKER_INTERFACE_NAME);

bool CMSVCLinker::IsLibraryExists( CUtlString szName )
{
	szName = CUtlString("%s.dll", szName.GetString());
	void *pLib = Plat_LoadLibrary(szName.GetString());
	if (!pLib)
		return false;
	Plat_UnloadLibrary(pLib);
	return true;
}
