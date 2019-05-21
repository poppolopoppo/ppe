#pragma once

#include "Core.h"

#include "Allocator/Allocation.h"
#include "Allocator/StlAllocator.h"

#include <deque>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Allocator = ALLOCATOR(Container, T)>
using TDeque = std::deque<T, TStlAllocator<T, _Allocator> >;
//----------------------------------------------------------------------------
#define DEQUE(_DOMAIN, T) \
    ::PPE::TDeque<COMMA_PROTECT(T), ALLOCATOR(_DOMAIN, COMMA_PROTECT(T)) >
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
