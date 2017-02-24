#pragma once

#include "Core/Core.h"

#include "Core/Container/HashTable.h"

#include <iosfwd>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define HASHSET(_DOMAIN, T) \
    ::Core::THashSet<T, ::Core::THash<T>, ::Core::Meta::TEqualTo<T>, ALLOCATOR(_DOMAIN, T)>
//----------------------------------------------------------------------------
#define HASHSET_THREAD_LOCAL(_DOMAIN, T) \
    ::Core::THashSet<T, ::Core::THash<T>, ::Core::Meta::TEqualTo<T>, THREAD_LOCAL_ALLOCATOR(_DOMAIN, T)>
//----------------------------------------------------------------------------
#define HASHSET_MEMOIZE(_DOMAIN, T) \
    HASHSET(_DOMAIN, ::Core::THashMemoizer<T>)
//----------------------------------------------------------------------------
#define HASHSET_MEMOIZE_THREAD_LOCAL(_DOMAIN, T) \
    HASHSET_THREAD_LOCAL(_DOMAIN, ::Core::THashMemoizer<T>)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <
    typename _Key,
    typename _Hasher = THash<_Key>,
    typename _EqualTo = Meta::TEqualTo<_Key>,
    typename _Allocator = ALLOCATOR(Container, _Key)
>
using THashSet = TBasicHashTable< details::THashSetTraits_<_Key>, _Hasher, _EqualTo, _Allocator >;
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
template <typename T, typename _Hasher, typename _EqualTo, typename _Allocator>
bool Insert_ReturnIfExists(THashSet<T, _Hasher, _EqualTo, _Allocator>& hashset, const T& elt) {
    return (not hashset.insert(elt).second);
}
//----------------------------------------------------------------------------
template <typename T, typename _Hasher, typename _EqualTo, typename _Allocator>
void Insert_AssertUnique(THashSet<T, _Hasher, _EqualTo, _Allocator>& hashset, const T& elt) {
    if (not hashset.insert(elt).second)
        AssertNotReached();
}
//----------------------------------------------------------------------------
template <typename T, typename _Hasher, typename _EqualTo, typename _Allocator>
bool Remove_ReturnIfExists(THashSet<T, _Hasher, _EqualTo, _Allocator>& hashset, const T& elt) {
    return hashset.erase(elt);
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
#ifdef WITH_CORE_ASSERT
    const auto it = hashset.find(key);
    if (hashset.end() == it) {
        AssertNotReached();
    }
    else {
        Assert(it->second == value);
        hashset.erase(it);
    }
#else
    hashset.erase(it);
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
