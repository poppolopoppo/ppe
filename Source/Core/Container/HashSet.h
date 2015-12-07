#pragma once

#include "Core/Core.h"

#include "Core/Allocator/Allocation.h"
#include "Core/Container/Hash.h"
#include "Core/Container/Pair.h"

#include <unordered_set>

#include <iosfwd>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <
    typename T,
    typename _Hasher = Hash<T>,
    typename _EqualTo = EqualTo<T>,
    typename _Allocator = ALLOCATOR(Container, T)
>
using HashSet = std::unordered_set<T, _Hasher, _EqualTo, _Allocator>;
//----------------------------------------------------------------------------
#define HASHSET(_DOMAIN, T) \
    ::Core::HashSet<T, Hash<T>, EqualTo<T>, ALLOCATOR(_DOMAIN, T)>
//----------------------------------------------------------------------------
#define HASHSET_THREAD_LOCAL(_DOMAIN, T) \
    ::Core::HashSet<T, Hash<T>, EqualTo<T>, THREAD_LOCAL_ALLOCATOR(_DOMAIN, T)>
//----------------------------------------------------------------------------
template <typename T, typename _Hasher, typename _EqualTo, typename _Allocator>
hash_t hash_value(const HashSet<T, _Hasher, _EqualTo, _Allocator>& hashSet) {
    return hash_value_seq(hashSet.begin(), hashSet.end());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <
    typename T,
    typename _Hasher,
    typename _EqualTo,
    typename _Allocator,
    typename _Char,
    typename _Traits
>
std::basic_ostream<_Char, _Traits>& operator <<(std::basic_ostream<_Char, _Traits>& oss, const HashSet<T, _Hasher, _EqualTo, _Allocator>& hashSet) {
    oss << "[ ";
    for (const auto& it : hashSet)
        oss << *it << ", ";
    return oss << ']';
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
