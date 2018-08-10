#pragma once

#include "Core.h"

#include "Container/HashTable.h"
#include "IO/TextWriter_fwd.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define HASHSET(_DOMAIN, T) \
    ::PPE::THashSet<T, ::PPE::Meta::THash<T>, ::PPE::Meta::TEqualTo<T>, ALLOCATOR(_DOMAIN, T)>
//----------------------------------------------------------------------------
#define HASHSET_MEMOIZE(_DOMAIN, T) \
    HASHSET(_DOMAIN, ::PPE::THashMemoizer<T>)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <
    typename _Key,
    typename _Hasher = Meta::THash<_Key>,
    typename _EqualTo = Meta::TEqualTo<_Key>,
    typename _Allocator = ALLOCATOR(Container, _Key)
>
using THashSet = TBasicHashTable< details::THashSetTraits_<_Key>, _Hasher, _EqualTo, _Allocator >;
//----------------------------------------------------------------------------
template <typename T, typename _Hasher, typename _EqualTo, typename _Allocator>
FTextWriter& operator <<(FTextWriter& oss, const THashSet<T, _Hasher, _EqualTo, _Allocator>& hashSet) {
    oss << "[ ";
    for (const auto& it : hashSet)
        oss << *it << ", ";
    return oss << ']';
}
//----------------------------------------------------------------------------
template <typename T, typename _Hasher, typename _EqualTo, typename _Allocator>
FWTextWriter& operator <<(FWTextWriter& oss, const THashSet<T, _Hasher, _EqualTo, _Allocator>& hashSet) {
    oss << L"[ ";
    for (const auto& it : hashSet)
        oss << *it << L", ";
    return oss << L']';
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Hasher, typename _EqualTo, typename _Allocator>
bool Insert_ReturnIfExists(THashSet<T, _Hasher, _EqualTo, _Allocator>& hashset, const T& elt) {
    return hashset.insert_ReturnIfExists(elt);
}
//----------------------------------------------------------------------------
template <typename T, typename _Hasher, typename _EqualTo, typename _Allocator>
void Insert_AssertUnique(THashSet<T, _Hasher, _EqualTo, _Allocator>& hashset, const T& elt) {
    hashset.insert_AssertUnique(elt);
}
//----------------------------------------------------------------------------
template <typename T, typename _Hasher, typename _EqualTo, typename _Allocator>
bool Remove_ReturnIfExists(THashSet<T, _Hasher, _EqualTo, _Allocator>& hashset, const T& elt) {
    return hashset.erase(elt);
}
//----------------------------------------------------------------------------
template <typename T, typename _Hasher, typename _EqualTo, typename _Allocator>
void Remove_AssertExists(THashSet<T, _Hasher, _EqualTo, _Allocator>& hashset, const T& elt) {
    hashset.erase_AssertExists(elt);
}
//----------------------------------------------------------------------------
template <typename T, typename _Hasher, typename _EqualTo, typename _Allocator>
void Remove_AssertExistsAndSameValue(THashSet<T, _Hasher, _EqualTo, _Allocator>& hashset, const T& elt) {
#ifdef WITH_PPE_ASSERT
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
} //!namespace PPE
