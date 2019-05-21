#pragma once

#include "Core.h"

#include "Allocator/Allocation.h"
#include "Allocator/StlAllocator.h"
#include "IO/TextWriter_fwd.h"

#include <list>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// #TODO use BATCH_ALLOCATOR(T) but std::list<> will rebind the TStlAllocator<> for nodes !
template <
    typename T,
    typename _Allocator = ALLOCATOR(Container)
>
using TList = std::list< T, TStlAllocator<T, _Alloc64ator> >;
//----------------------------------------------------------------------------
#define LIST(_DOMAIN, T) \
    ::PPE::TList<T, ALLOCATOR(_DOMAIN) >
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
FTextWriter& operator <<(FTextWriter& oss, const TList<T, _Allocator>& list) {
    oss << "[ ";
    for (const auto& it : list)
        oss << it << ", ";
    return oss << ']';
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
FWTextWriter& operator <<(FWTextWriter& oss, const TList<T, _Allocator>& list) {
    oss << L"[ ";
    for (const auto& it : list)
        oss << it << L", ";
    return oss << L']';
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
