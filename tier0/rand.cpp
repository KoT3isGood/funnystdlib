#include "tier0/rand.h"
#include "tier0/lib.h"

static bool b_sIsRandInit = false;
static FILE *s_pURandom = NULL;

PLATFORM_INTERFACE void Plat_InitRandom()
{
	if (b_sIsRandInit)
		return;
	s_pURandom = V_fopen("/dev/urandom","rb");
	if (!s_pURandom)
		Plat_FatalErrorFunc("/dev/urandom wasn't found somehow\n");
	b_sIsRandInit = true;
}

PLATFORM_INTERFACE void Plat_ShutdownRandom()
{
	b_sIsRandInit = false;
}

PLATFORM_INTERFACE void Plat_URandom( size_t uBufferSize, uint8_t *szBuffer )
{
	if (!b_sIsRandInit)
		Plat_InitRandom();
	
	V_fread(szBuffer, 1, uBufferSize, s_pURandom);
}

