#include "tier0/mem.h"
#include "stdlib.h"
#include "tier0/platform.h"

//-----------------------------------------------------------------------------
// These functions copy over libc memory functions
//-----------------------------------------------------------------------------

PLATFORM_INTERFACE void* V_malloc(int nSize)
{
	void *pMemory = malloc(nSize);
	if (!pMemory)
		Plat_FatalErrorFunc("Failed to allocate memory");
	return pMemory;
}

PLATFORM_INTERFACE void V_free(void *pMem)
{
	return free(pMem);
}

PLATFORM_INTERFACE void* V_realloc(void *pMem, int nSize)
{
	return realloc(pMem, nSize);
}

void *operator new( size_t nCount )
{
	return V_malloc(nCount);	
}

void *operator new ( size_t nCount, void *pPtr )
{
	return pPtr;
}

void operator delete( void *pMem ) noexcept
{
	return V_free(pMem);	
}
