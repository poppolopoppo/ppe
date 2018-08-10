#pragma once

#include "Core.h"

#include "Container/HashTable.h"
#include "IO/TextWriter_fwd.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define HASHMAP(_DOMAIN, _KEY, _VALUE) \
    ::PPE::THashMap<_KEY, _VALUE, ::PPE::Meta::THash<_KEY>, ::PPE::Meta::TEqualTo<_KEY>, ALLOCATOR(_DOMAIN, ::PPE::TPair<_KEY COMMA _VALUE>)>
//----------------------------------------------------------------------------
#define HASHMAP_MEMOIZE(_DOMAIN, _KEY, _VALUE) \
    HASHMAP(_DOMAIN, ::PPE::THashMemoizer<_KEY>, _VALUE)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <
    typename _Key,
    typename _Value,
    typename _Hasher = Meta::THash<_Key>,
    typename _EqualTo = Meta::TEqualTo<_Key>,
    typename _Allocator = ALLOCATOR(Container, TPair<_Key COMMA _Value>)
>
using THashMap = TBasicHashTable< details::THashMapTraits_<_Key, _Value>, _Hasher, _EqualTo, _Allocator >;
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
FTextWriter& operator <<(FTextWriter& oss, const THashMap<_Key, _Value, _Hasher, _EqualTo, _Allocator>& hashMap) {
    oss << "{ ";
    for (const auto& it : hashMap)
        oss << '(' << it.first << ", " << it.second << "), ";
    return oss << '}';
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
FWTextWriter& operator <<(FWTextWriter& oss, const THashMap<_Key, _Value, _Hasher, _EqualTo, _Allocator>& hashMap) {
    oss << L"{ ";
    for (const auto& it : hashMap)
        oss << L'(' << it.first << L", " << it.second << L"), ";
    return oss << L'}';
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
bool TryGetValue(const THashMap<_Key, _Value, _Hasher, _EqualTo, _Allocator>& hashmap, const _Key& key, _Value *value) {
    Assert(value);
    const auto it = hashmap.find(key);
    if (hashmap.end() == it) {
        return false;
    }
    else {
        *value = it->second;
        return true;
    }
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
bool Insert_ReturnIfExists(THashMap<_Key, _Value, _Hasher, _EqualTo, _Allocator>& hashmap, const _Key& key, const _Value& value) {
    return (not hashmap.try_emplace(key, value).second);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
void Insert_AssertUnique(THashMap<_Key, _Value, _Hasher, _EqualTo, _Allocator>& hashmap, const _Key& key, const _Value& value) {
    if (not hashmap.try_emplace(key, value).second)
        AssertNotReached();
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
void Insert_AssertUnique(THashMap<_Key, _Value, _Hasher, _EqualTo, _Allocator>& hashmap, _Key&& rkey, _Value&& rvalue) {
    if (not hashmap.try_emplace(std::move(rkey), std::move(rvalue)).second)
        AssertNotReached();
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
bool Remove_ReturnIfExists(THashMap<_Key, _Value, _Hasher, _EqualTo, _Allocator>& hashmap, const _Key& key) {
    return hashmap.erase(key);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
void Remove_AssertExists(THashMap<_Key, _Value, _Hasher, _EqualTo, _Allocator>& hashmap, const _Key& key) {
    if (!Remove_ReturnIfExists(hashmap, key))
        AssertNotReached();
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
_Value Remove_ReturnValue(THashMap<_Key, _Value, _Hasher, _EqualTo, _Allocator>& hashmap, const _Key& key) {
    typename THashMap<_Key, _Value, _Hasher, _EqualTo, _Allocator>::value_type value;
    if (not hashmap.erase(key, &value))
        AssertNotReached();
    return value.second;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
void Remove_AssertExistsAndSameValue(THashMap<_Key, _Value, _Hasher, _EqualTo, _Allocator>& hashmap, const _Key& key, const _Value& value) {
#ifdef WITH_PPE_ASSERT
    const auto it = hashmap.find(key);
    if (hashmap.end() == it) {
        AssertNotReached();
    }
    else {
        Assert(it->second == value);
        hashmap.erase(it);
    }
#else
    NOOP(value);
    hashmap.erase(key);
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
