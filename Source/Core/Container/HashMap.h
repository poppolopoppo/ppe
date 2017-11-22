#pragma once

#include "Core/Core.h"

#include "Core/Container/HashTable.h"

#include <iosfwd>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define HASHMAP(_DOMAIN, _KEY, _VALUE) \
    ::Core::THashMap<_KEY, _VALUE, ::Core::Meta::THash<_KEY>, ::Core::Meta::TEqualTo<_KEY>, ALLOCATOR(_DOMAIN, ::Core::TPair<_KEY COMMA _VALUE>)>
//----------------------------------------------------------------------------
#define HASHMAP_THREAD_LOCAL(_DOMAIN, _KEY, _VALUE) \
    ::Core::THashMap<_KEY, _VALUE, ::Core::Meta::THash<_KEY>, ::Core::Meta::TEqualTo<_KEY>, THREAD_LOCAL_ALLOCATOR(_DOMAIN, ::Core::TPair<_KEY COMMA _VALUE>)>
//----------------------------------------------------------------------------
#define HASHMAP_MEMOIZE(_DOMAIN, _KEY, _VALUE) \
    HASHMAP(_DOMAIN, ::Core::THashMemoizer<_KEY>, _VALUE)
//----------------------------------------------------------------------------
#define HASHMAP_MEMOIZE_THREAD_LOCAL(_DOMAIN, _KEY, _VALUE) \
    HASHMAP_THREAD_LOCAL(_DOMAIN, ::Core::THashMemoizer<_KEY>, _VALUE)
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
template <
    typename _Key,
    typename _Value,
    typename _Hasher,
    typename _EqualTo,
    typename _Allocator,
    typename _Char,
    typename _Traits
>
std::basic_ostream<_Char, _Traits>& operator <<(std::basic_ostream<_Char, _Traits>& oss, const THashMap<_Key, _Value, _Hasher, _EqualTo, _Allocator>& hashMap) {
    oss << "{ ";
    for (const auto& it : hashMap)
        oss << '(' << it.first << ", " << it.second << "), ";
    return oss << '}';
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
    THashMap<_Key, _Value, _Hasher, _EqualTo, _Allocator>::value_type value;
    if (not hashmap.erase(key, &value))
        AssertNotReached();
    return value.second;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
void Remove_AssertExistsAndSameValue(THashMap<_Key, _Value, _Hasher, _EqualTo, _Allocator>& hashmap, const _Key& key, const _Value& value) {
#ifdef WITH_CORE_ASSERT
    const auto it = hashmap.find(key);
    if (hashmap.end() == it) {
        AssertNotReached();
    }
    else {
        Assert(it->second == value);
        hashmap.erase(it);
    }
#else
    hashmap.erase(key);
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
