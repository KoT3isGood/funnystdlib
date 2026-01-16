//================= Copyright kotofyt, All rights reserved ==================//
// Purpose: Multi-platform implementation of some of the functions which
// are provided by each OS differently.
//===========================================================================//

#ifndef TIER0_PLATFORM_H
#define TIER0_PLATFORM_H

#include "stdint.h"
#include "stddef.h"

#ifdef __WIN32__
#define  DLL_EXPORT extern "C"
#define  DLL_IMPORT extern "C"

#define  DLL_CLASS_EXPORT
#define  DLL_CLASS_IMPORT

#define  DLL_GLOBAL_EXPORT   extern
#define  DLL_GLOBAL_IMPORT   extern
#else
#define  DLL_EXPORT   extern "C" __attribute__ ((visibility("default")))
#define  DLL_IMPORT   extern "C"

#define  DLL_CLASS_EXPORT __attribute__ ((visibility("default")))
#define  DLL_CLASS_IMPORT

#define  DLL_GLOBAL_EXPORT   __attribute ((visibility("default")))
#define  DLL_GLOBAL_IMPORT   extern

#endif

#ifdef TIER0_STATIC

#define GLOBAL_USED __attribute__((used)) 

#else

#define GLOBAL_USED __attribute__((used))

#ifdef TIER0_IMPLEMENTATION
#define PLATFORM_INTERFACE DLL_EXPORT
#define PLATFORM_OVERLOAD DLL_GLOBAL_EXPORT
#define PLATFORM_CLASS DLL_CLASS_EXPORT
#else
#define PLATFORM_INTERFACE DLL_IMPORT
#define PLATFORM_OVERLOAD DLL_GLOBAL_IMPORT
#define PLATFORM_CLASS DLL_CLASS_IMPORT
#endif

#endif

#if defined(__linux__)
#define POSIX
#endif

#define SWAP16(x) (uint16_t)((((x) >> 8) & 0x00FF) | \
                             (((x) << 8) & 0xFF00))

#define SWAP32(x) (uint32_t)((((x) >> 24) & 0x000000FF) | \
                             (((x) >> 8)  & 0x0000FF00) | \
                             (((x) << 8)  & 0x00FF0000) | \
                             (((x) << 24) & 0xFF000000))

#define SWAP64(x) ((uint64_t)( \
    (((x) >> 56) & 0x00000000000000FFULL) | \
    (((x) >> 40) & 0x000000000000FF00ULL) | \
    (((x) >> 24) & 0x0000000000FF0000ULL) | \
    (((x) >> 8)  & 0x00000000FF000000ULL) | \
    (((x) << 8)  & 0x000000FF00000000ULL) | \
    (((x) << 24) & 0x0000FF0000000000ULL) | \
    (((x) << 40) & 0x00FF000000000000ULL) | \
    (((x) << 56) & 0xFF00000000000000ULL) ))

#define abstract_class class

PLATFORM_INTERFACE void Plat_FatalErrorFunc( const char *szFormat, ... );

typedef void( *ListDirCallbackFn )( const char *szPath );
PLATFORM_INTERFACE void Plat_ListDirRecursive( const char *szPath, ListDirCallbackFn file, ListDirCallbackFn dir );
PLATFORM_INTERFACE void Plat_ListDir( const char *szPath, ListDirCallbackFn file, ListDirCallbackFn dir );
PLATFORM_INTERFACE char *Plat_GetExtension( const char *szPath );

PLATFORM_INTERFACE void Plat_MakeDir( const char *szPath, int iPermissions );

PLATFORM_INTERFACE void Plat_TrapSignals( void (*pfn)() );
PLATFORM_INTERFACE void Plat_Backtrace( void );

PLATFORM_INTERFACE void *Plat_LoadLibrary( const char *psz );
PLATFORM_INTERFACE void *Plat_GetProc( void *lib, const char *psz );
PLATFORM_INTERFACE void Plat_UnloadLibrary( void *psz );

PLATFORM_INTERFACE void Plat_SetEnv( const char *szVar, const char *psz );
PLATFORM_INTERFACE const char *Plat_GetEnv( const char *szVar );

PLATFORM_INTERFACE void Plat_SetWorkingDir(  const char *psz );
PLATFORM_INTERFACE const char *Plat_GetWorkingDir( void );

PLATFORM_INTERFACE const char *Plat_GetExecutablePath( void );
PLATFORM_INTERFACE const char *Plat_GetParentDir( const char *psz );

PLATFORM_INTERFACE const char *Plat_GetUNIXExecutable( const char *psz );
PLATFORM_INTERFACE const char *Plat_GetWindowsExecutable( const char *psz );
PLATFORM_INTERFACE const char *Plat_GetPlatformExecutable( const char *psz );

PLATFORM_INTERFACE const char *Plat_GetDarwinSharedLib( const char *psz );
PLATFORM_INTERFACE const char *Plat_GetUNIXSharedLib( const char *psz );
PLATFORM_INTERFACE const char *Plat_GetWindowsSharedLib( const char *psz );
PLATFORM_INTERFACE const char *Plat_GetPlatformSharedLib( const char *psz );

PLATFORM_INTERFACE double Plat_GetTime( void );
PLATFORM_INTERFACE void Plat_Exit( int status );

#endif
