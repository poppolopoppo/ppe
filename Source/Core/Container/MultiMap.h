#pragma once

#include "Core/Core.h"

#include "Core/Allocator/Allocation.h"
#include "Core/Allocator/NodeBasedContainerAllocator.h"
#include "Core/Container/Pair.h"

#include <map>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <
    typename _Key,
    typename _Value,
    typename _Predicate = Meta::Less<_Key>,
    typename _Allocator = NODEBASED_CONTAINER_ALLOCATOR(Container, Pair<_Key COMMA _Value>)
>
using MultiMap = std::multimap<_Key, _Value, _Predicate, _Allocator >;
//----------------------------------------------------------------------------
#define MULTIMAP(_DOMAIN, _KEY, _VALUE) \
    ::Core::MultiMap<_KEY, _VALUE, ::Core::Meta::Less<_KEY>, NODEBASED_CONTAINER_ALLOCATOR(_DOMAIN, ::Core::Pair<_KEY COMMA _VALUE>) >
//----------------------------------------------------------------------------
#define MULTIMAP_THREAD_LOCAL(_DOMAIN, _KEY, _VALUE) \
    ::Core::MultiMap<_KEY, _VALUE, ::Core::Meta::Less<_KEY>, THREAD_LOCAL_NODEBASED_CONTAINER_ALLOCATOR(_DOMAIN, ::Core::Pair<_KEY COMMA _VALUE>) >
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Pred, typename _Allocator>
hash_t hash_value(const MultiMap<_Key, _Value, _Pred, _Allocator>& multiMap) {
    return hash_range(multiMap.begin(), multiMap.end());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Pred, typename _Allocator>
bool TryGetValue(const MultiMap<_Key, _Value, _Pred, _Allocator>& multiMap, const _Key& key, _Value *value) {
    Assert(value);

    const auto it = multiMap.find(key);
    if (multiMap.end() == it)
        return false;

    *value = it->second;
    return true;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Pred, typename _Allocator>
bool Insert_ReturnIfExists(MultiMap<_Key, _Value, _Pred, _Allocator>& multiMap, const _Key& key, const _Value& value) {
    const auto it = multiMap.find(key);
    if (multiMap.end() == it) {
        multiMap.insert(std::make_pair(key, value));
        return false;
    }
    else {
        Assert(it->second == value);
        return true;
    }
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Pred, typename _Allocator>
void Insert_AssertUnique(MultiMap<_Key, _Value, _Pred, _Allocator>& multiMap, const _Key& key, const _Value& value) {
    if (Insert_ReturnIfExists(multiMap, key, value))
        AssertNotReached();
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Pred, typename _Allocator>
bool Remove_ReturnIfExists(MultiMap<_Key, _Value, _Pred, _Allocator>& multiMap, const _Key& key) {
    const auto it = multiMap.find(key);
    if (multiMap.end() == it) {
        return false;
    }
    else {
        multiMap.erase(it);
        return true;
    }
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Pred, typename _Allocator>
void Remove_AssertExists(MultiMap<_Key, _Value, _Pred, _Allocator>& multiMap, const _Key& key) {
    if (!Remove_ReturnIfExists(multiMap, key))
        AssertNotReached();
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Pred, typename _Allocator>
void RemoveKeyValue_AssertExists(MultiMap<_Key, _Value, _Pred, _Allocator>& multiMap, const _Key& key, const _Value& value) {

    const auto range = multiMap.equal_range(key);
    if (multiMap.end() == range.first) {
        Assert(multiMap.end() == range.second);
        AssertNotReached();
        return;
    }

    for (auto it = range.first; it != range.second; ++it)
        if (it->second == value) {
            multiMap.erase(it);
            return;
        }

    AssertNotReached();
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Pred, typename _Allocator>
size_t FillMatchingValues_ReturnCount(_Value *pValues, size_t capacity, const MultiMap<_Key, _Value, _Pred, _Allocator>& multiMap, const _Key& key) {
    Assert(pValues);
    Assert(capacity > 0);

    const auto range = multiMap.equal_range(key);
    if (multiMap.end() == range.first) {
        Assert(multiMap.end() == range.second);
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
    typename _Pred,
    typename _Allocator,
    typename _Char,
    typename _Traits
>
std::basic_ostream<_Char, _Traits>& operator <<(std::basic_ostream<_Char, _Traits>& oss, const MultiMap<_Key, _Value, _Pred, _Allocator>& multiMap) {
    oss << "{ ";
    for (const auto& it : multiMap)
        oss << '(' << it.first << ", " << it.second << "), ";
    return oss << '}';
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
