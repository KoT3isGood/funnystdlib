#include "tier1/interface.h"
#include "tier1/utlvector.h"
#include "tier1/utlstring.h"

static CInterfaceRegistry *s_pInterfaceRegistries;

CInterfaceRegistry::CInterfaceRegistry( InstantiateInterfaceFn fn, const char *szName )
	: m_szName(szName)
{
	m_CreateFn = fn;
	m_pNext = s_pInterfaceRegistries;
	s_pInterfaceRegistries = this;  
};

DLL_EXPORT void *CreateInterface( const char *szName, int *pReturnCode )
{
	CInterfaceRegistry *pRegistry = s_pInterfaceRegistries;
	while (pRegistry)
	{
		if (!V_strcmp(szName, pRegistry->m_szName))
		{
			if (pReturnCode)
				*pReturnCode = 0;
			return pRegistry->m_CreateFn();
		}
		pRegistry = pRegistry->m_pNext;
	}
	if (pReturnCode)
		*pReturnCode = 1;
	return 0;
}

CreateInterfaceFn Sys_GetFactory( void *lib )
{
	return (CreateInterfaceFn)Plat_GetProc(lib, "CreateInterface");
}
CreateInterfaceFn Sys_GetFactory( const char *szLibrary )
{
	void *pLib = NULL;
	CUtlString szLib = szLibrary;
#if defined(__linux__)
	szLib = CUtlString("lib%s.so", szLib.GetFileName().GetString());
	pLib = Plat_LoadLibrary(szLib);
#endif
#if defined(__WIN32__)
	szLib = CUtlString("%s.dll", szLib.GetFileName().GetString());
	pLib = Plat_LoadLibrary(szLib);
#endif
	if ( !pLib )
		return NULL;
	return Sys_GetFactory(pLib);
}
