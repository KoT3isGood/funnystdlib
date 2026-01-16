#ifndef INTERFACE_H
#define INTERFACE_H

#include "tier0/platform.h"

typedef void *( *CreateInterfaceFn )( const char *szName, int *pReturnCode );
typedef void *( *InstantiateInterfaceFn )( void );

class CInterfaceRegistry
{
public:
	CInterfaceRegistry( InstantiateInterfaceFn fn, const char *szName );

	InstantiateInterfaceFn m_CreateFn;
	const char *m_szName;

	CInterfaceRegistry *m_pNext;
};


#define EXPOSE_INTERFACE( className, interfaceName, versionName ) \
	static void *__Create##className##_interface() { return ( interfaceName* )( new className ); }; \
	static CInterfaceRegistry __Create##className##_registry( __Create##className##_interface, versionName );

#define EXPOSE_INTERFACE_FN( functionName, interfaceName, versionName ) \
	static CInterfaceRegistry __Create##interfaceName##_registry( functionName, versionName );

#define EXPOSE_INTERFACE_GLOBALVAR( className, interfaceName, versionName, globalVarName ) \
	static void *__Create##className##_interface() { return ( interfaceName* )( &globalVarName ); }; \
	static CInterfaceRegistry __Create##className##_registry(  __Create##className##_interface, versionName );


DLL_EXPORT void *CreateInterface( const char *szName, int *pReturnCode );

CreateInterfaceFn Sys_GetFactory( void *pLibrary );
CreateInterfaceFn Sys_GetFactory( const char *szLibrary );

#endif
