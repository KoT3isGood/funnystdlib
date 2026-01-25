#ifndef LSP_H
#define LSP_H

#include "tier0/platform.h"

abstract_class IBasicLSP
{
	virtual void GenerateConfig() = 0;
};

#endif
