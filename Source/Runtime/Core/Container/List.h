#pragma once

#include "Core/Core.h"

#include "Core/Allocator/Allocation.h"
#include "Core/Allocator/NodeBasedContainerAllocator.h"
#include "Core/IO/TextWriter_fwd.h"

#include <list>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <
    typename T,
    typename _Allocator = NODEBASED_CONTAINER_ALLOCATOR(Container, T)
>
using TList = std::list< T, _Allocator >;
//----------------------------------------------------------------------------
#define LIST(_DOMAIN, T) \
    ::Core::TList<T, NODEBASED_CONTAINER_ALLOCATOR(_DOMAIN, T) >
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
} //!namespace Core
