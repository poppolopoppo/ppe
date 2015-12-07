#pragma once

#include "Core/Core.h"

#include "Core/Allocator/Allocation.h"
#include "Core/Container/Hash.h"
#include "Core/Container/Pair.h"

#include <unordered_map>

#include <iosfwd>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <
    typename _Key,
    typename _Value,
    typename _Hasher = Hash<_Key>,
    typename _EqualTo = EqualTo<_Key>,
    typename _Allocator = ALLOCATOR(Container, Pair<_Key COMMA _Value>)
>
using HashMultiMap = std::unordered_multimap<_Key, _Value, _Hasher, _EqualTo, _Allocator>;
//----------------------------------------------------------------------------
#define HASHMULTIMAP(_DOMAIN, _KEY, _VALUE) \
    ::Core::HashMultiMap<_KEY, _VALUE, ::Core::Hash<_KEY>, ::Core::EqualTo<_KEY>, ALLOCATOR(_DOMAIN, ::Core::Pair<_KEY COMMA _VALUE>)>
//----------------------------------------------------------------------------
#define HASHMULTIMAP_THREAD_LOCAL(_DOMAIN, _KEY, _VALUE) \
    ::Core::HashMultiMap<_KEY, _VALUE, ::Core::Hash<_KEY>, ::Core::EqualTo<_KEY>, THREAD_LOCAL_ALLOCATOR(_DOMAIN, ::Core::Pair<_KEY COMMA _VALUE>)>
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
hash_t hash_value(const HashMultiMap<_Key, _Value, _Hasher, _EqualTo, _Allocator>& hashMultiMap) {
    return hash_value_seq(hashMultiMap.begin(), hashMultiMap.end());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
bool TryGetValue(const HashMultiMap<_Key, _Value, _Hasher, _EqualTo, _Allocator>& hashMultiMap, const _Key& key, _Value *value) {
    Assert(value);

    const auto it = hashMultiMap.find(key);
    if (hashMultiMap.end() == it)
        return false;

    *value = it->second;
    return true;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
bool Insert_ReturnIfExists(HashMultiMap<_Key, _Value, _Hasher, _EqualTo, _Allocator>& hashMultiMap, const _Key& key, const _Value& value) {
    const auto it = hashMultiMap.find(key);
    if (hashMultiMap.end() == it) {
        hashMultiMap.insert(std::make_pair(key, value));
        return false;
    }
    else {
        Assert(it->second == value);
        return true;
    }
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
void Insert_AssertUnique(HashMultiMap<_Key, _Value, _Hasher, _EqualTo, _Allocator>& hashMultiMap, const _Key& key, const _Value& value) {
    if (Insert_ReturnIfExists(hashMultiMap, key, value))
        AssertNotReached();
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
bool Remove_ReturnIfExists(HashMultiMap<_Key, _Value, _Hasher, _EqualTo, _Allocator>& hashMultiMap, const _Key& key) {
    const auto it = hashMultiMap.find(key);
    if (hashMultiMap.end() == it) {
        return false;
    }
    else {
        hashMultiMap.erase(it);
        return true;
    }
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
void Remove_AssertExists(HashMultiMap<_Key, _Value, _Hasher, _EqualTo, _Allocator>& hashMultiMap, const _Key& key) {
    if (!Remove_ReturnIfExists(hashMultiMap, key))
        AssertNotReached();
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
void RemoveKeyValue_AssertExists(HashMultiMap<_Key, _Value, _Hasher, _EqualTo, _Allocator>& hashMultiMap, const _Key& key, const _Value& value) {

    const auto range = hashMultiMap.equal_range(key);
    if (hashMultiMap.end() == range.first) {
        Assert(hashMultiMap.end() == range.second);
        AssertNotReached();
        return;
    }

    for (auto it = range.first; it != range.second; ++it)
        if (it->second == value) {
            hashMultiMap.erase(it);
            return;
        }

    AssertNotReached();
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
size_t FillMatchingValues_ReturnCount(_Value *pValues, size_t capacity, const HashMultiMap<_Key, _Value, _Hasher, _EqualTo, _Allocator>& hashMultiMap, const _Key& key) {
    Assert(pValues);
    Assert(capacity > 0);

    const auto range = hashMultiMap.equal_range(key);
    if (hashMultiMap.end() == range.first) {
        Assert(hashMultiMap.end() == range.second);
        return 0;
    }

    const size_t count = std::distance(range.first, range.second);
    AssertRelease(count < capacity);

    for (auto it = range.first; it != range.second; ++it)
        *(pValues++) = it->second;

    return count;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
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
std::basic_ostream<_Char, _Traits>& operator <<(std::basic_ostream<_Char, _Traits>& oss, const HashMultiMap<_Key, _Value, _Hasher, _EqualTo, _Allocator>& hashMultiMap) {
    oss << "{ ";
    for (const auto& it : hashMultiMap)
        oss << '(' << it.first << ", " << it.second << "), ";
    return oss << '}';
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
