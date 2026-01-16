#ifndef TIER0_RAND_H
#define TIER0_RAND_H

#include "tier0/platform.h"

PLATFORM_INTERFACE void Plat_InitRandom();
PLATFORM_INTERFACE void Plat_ShutdownRandom();
PLATFORM_INTERFACE void Plat_URandom( size_t uBufferSize, uint8_t *szBuffer );

#endif
