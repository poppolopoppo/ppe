#pragma once

#include "Core_fwd.h"

#include "Allocator/Allocation.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
class TRawStorage;
//----------------------------------------------------------------------------
#define RAWSTORAGE(_DOMAIN, T) \
    ::PPE::TRawStorage<T, ALLOCATOR(_DOMAIN)>
//----------------------------------------------------------------------------
#define RAWSTORAGE_ALIGNED(_DOMAIN, T, _ALIGNMENT) \
    ::PPE::TRawStorage<T, ALIGNED_ALLOCATOR(_DOMAIN, _ALIGNMENT)>
    //----------------------------------------------------------------------------
#define RAWSTORAGE_INSITU(_DOMAIN, T, N) \
    ::PPE::TRawStorage<T, INLINE_STACK_ALLOCATOR(_DOMAIN, T, N)>
//----------------------------------------------------------------------------
#define RAWSTORAGE_STACK(T) \
    ::PPE::TRawStorage<T, STACKLOCAL_ALLOCATOR()>
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE