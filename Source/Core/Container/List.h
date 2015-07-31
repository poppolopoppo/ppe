#pragma once

#include "Core/Core.h"

#include "Core/Allocator/Allocation.h"
#include "Core/Allocator/NodeBasedContainerAllocator.h"

#include <list>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <
    typename T,
    typename _Allocator = NODEBASED_CONTAINER_ALLOCATOR(Container, T)
>
using List = std::list< T, _Allocator >;
//----------------------------------------------------------------------------
#define LIST(_DOMAIN, T) \
    ::Core::List<T, NODEBASED_CONTAINER_ALLOCATOR(_DOMAIN, T) >
//----------------------------------------------------------------------------
#define LIST_THREAD_LOCAL(_DOMAIN, T) \
    ::Core::List<T, THREAD_LOCAL_NODEBASED_CONTAINER_ALLOCATOR(_DOMAIN, T) >
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <
    typename T,
    typename _Allocator,
    typename _Char,
    typename _Traits
>
std::basic_ostream<_Char, _Traits>& operator <<(std::basic_ostream<_Char, _Traits>& oss, const List<T, _Allocator>& list) {
    oss << "[ ";
    for (const auto& it : list)
        oss << it << ", ";
    return oss << ']';
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
