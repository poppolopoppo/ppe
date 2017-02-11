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
    typename _Hasher = THash<T>,
    typename _EqualTo = Meta::TEqualTo<T>,
    typename _Allocator = ALLOCATOR(Container, T)
>
using THashSet = std::unordered_set<T, _Hasher, _EqualTo, _Allocator>;
//----------------------------------------------------------------------------
#define HASHSET(_DOMAIN, T) \
    ::Core::THashSet<T, THash<T>, ::Core::Meta::TEqualTo<T>, ALLOCATOR(_DOMAIN, T)>
//----------------------------------------------------------------------------
#define HASHSET_THREAD_LOCAL(_DOMAIN, T) \
    ::Core::THashSet<T, THash<T>, ::Core::Meta::TEqualTo<T>, THREAD_LOCAL_ALLOCATOR(_DOMAIN, T)>
//----------------------------------------------------------------------------
#define HASHSET_MEMOIZE(_DOMAIN, T) \
    HASHSET(_DOMAIN, ::Core::THashMemoizer<T>)
//----------------------------------------------------------------------------
#define HASHSET_MEMOIZE_THREAD_LOCAL(_DOMAIN, T) \
    HASHSET_THREAD_LOCAL(_DOMAIN, ::Core::THashMemoizer<T>)
//----------------------------------------------------------------------------
template <typename T, typename _Hasher, typename _EqualTo, typename _Allocator>
hash_t hash_value(const THashSet<T, _Hasher, _EqualTo, _Allocator>& hashSet) {
    return hash_range(hashSet.begin(), hashSet.end());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Hasher, typename _EqualTo, typename _Allocator>
bool Insert_ReturnIfExists(THashSet<T, _Hasher, _EqualTo, _Allocator>& hashset, const T& elt) {
    const auto it = hashset.lower_bound(elt);
    if (it != hashset.end() && !(hashset.key_comp()(elt, *it)) ) {
        return true;
    }
    else {
        hashset.insert(it, elt);
        return false;
    }
}
//----------------------------------------------------------------------------
template <typename T, typename _Hasher, typename _EqualTo, typename _Allocator>
void Insert_AssertUnique(THashSet<T, _Hasher, _EqualTo, _Allocator>& hashset, const T& elt) {
#ifdef WITH_CORE_ASSERT
    const auto it = hashset.lower_bound(elt);
    if (it != hashset.end() && !(hashset.key_comp()(elt, *it)) )
        AssertNotReached();
    else
        hashset.insert(it, elt);
#else
    hashset.insert(elt);
#endif
}
//----------------------------------------------------------------------------
template <typename T, typename _Hasher, typename _EqualTo, typename _Allocator>
bool Remove_ReturnIfExists(THashSet<T, _Hasher, _EqualTo, _Allocator>& hashset, const T& elt) {
    const auto it = hashset.find(elt);
    if (hashset.end() == it) {
        return false;
    }
    else {
        hashset.erase(it);
        return true;
    }
}
//----------------------------------------------------------------------------
template <typename T, typename _Hasher, typename _EqualTo, typename _Allocator>
void Remove_AssertExists(THashSet<T, _Hasher, _EqualTo, _Allocator>& hashset, const T& elt) {
    if (!Remove_ReturnIfExists(hashset, elt))
        AssertNotReached();
}
//----------------------------------------------------------------------------
template <typename T, typename _Hasher, typename _EqualTo, typename _Allocator>
void Remove_AssertExistsAndSameValue(THashSet<T, _Hasher, _EqualTo, _Allocator>& hashset, const T& elt) {
    const auto it = hashset.find(key);
    if (hashset.end() == it) {
        AssertNotReached();
    }
    else {
        Assert(it->second == value);
        hashset.erase(it);
    }
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
std::basic_ostream<_Char, _Traits>& operator <<(std::basic_ostream<_Char, _Traits>& oss, const THashSet<T, _Hasher, _EqualTo, _Allocator>& hashSet) {
    oss << "[ ";
    for (const auto& it : hashSet)
        oss << *it << ", ";
    return oss << ']';
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
