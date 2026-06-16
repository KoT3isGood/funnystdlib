//================= Copyright kotofyt, All rights reserved ==================//
// C++ only supports std::initializer_list, so we need to get around compiler.
//
// fuck C++ once
// fuck C++ twice
// fuck C++ thrice
//===========================================================================//

#ifndef TIER1_UTL_INITIALIZER_LIST_H
#define TIER1_UTL_INITIALIZER_LIST_H

#include "stddef.h"
#include "initializer_list"

// from GCC 
template<typename T>
using CUtlInitializerList = std::initializer_list<T>;


#endif
