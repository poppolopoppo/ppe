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
    typename _Predicate = std::less<_Key>,
    typename _Allocator = NODEBASED_CONTAINER_ALLOCATOR(Container, Pair<_Key COMMA _Value>)
>
using Map = std::map<_Key, _Value, _Predicate, _Allocator >;
//----------------------------------------------------------------------------
#define MAP(_DOMAIN, _KEY, _VALUE) \
    ::Core::Map<_KEY, _VALUE, std::less<_KEY>, NODEBASED_CONTAINER_ALLOCATOR(_DOMAIN, ::Core::Pair<_KEY COMMA _VALUE>) >
//----------------------------------------------------------------------------
#define MAP_THREAD_LOCAL(_DOMAIN, _KEY, _VALUE) \
    ::Core::Map<_KEY, _VALUE, std::less<_KEY>, THREAD_LOCAL_NODEBASED_CONTAINER_ALLOCATOR(_DOMAIN, ::Core::Pair<_KEY COMMA _VALUE>) >
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Pred, typename _Allocator>
size_t hash_value(const Map<_Key, _Value, _Pred, _Allocator>& map) {
    return hash_value_seq(map.begin(), map.end());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Predicate, typename _Allocator>
bool TryGetValue(const Map<_Key, _Value, _Predicate, _Allocator>& map, const _Key& key, _Value *value) {
    Assert(value);

    const auto it = map.find(key);
    if (map.end() == it)
        return false;

    *value = it->second;
    return true;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Predicate, typename _Allocator>
bool Insert_ReturnIfExists(Map<_Key, _Value, _Predicate, _Allocator>& map, const _Key& key, const _Value& value) {
    const auto it = map.find(key);
    if (map.end() == it) {
        map.insert(std::make_pair(key, value));
        return false;
    }
    else {
        Assert(it->second == value);
        return true;
    }
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Predicate, typename _Allocator>
void Insert_AssertUnique(Map<_Key, _Value, _Predicate, _Allocator>& map, const _Key& key, const _Value& value) {
    if (Insert_ReturnIfExists(map, key, value))
        AssertNotReached();
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Predicate, typename _Allocator>
bool Remove_ReturnIfExists(Map<_Key, _Value, _Predicate, _Allocator>& map, const _Key& key) {
    const auto it = map.find(key);
    if (map.end() == it) {
        return false;
    }
    else {
        map.erase(it);
        return true;
    }
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Predicate, typename _Allocator>
void Remove_AssertExists(Map<_Key, _Value, _Predicate, _Allocator>& map, const _Key& key) {
    if (!Remove_ReturnIfExists(map, key))
        AssertNotReached();
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Predicate, typename _Allocator>
void Remove_AssertExistsAndSameValue(Map<_Key, _Value, _Predicate, _Allocator>& map, const _Key& key, const _Value& value) {
    const auto it = map.find(key);
    if (map.end() == it) {
        AssertNotReached();
    }
    else {
        Assert(it->second == value);
        map.erase(it);
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <
    typename _Key,
    typename _Value,
    typename _Predicate,
    typename _Allocator,
    typename _Char,
    typename _Traits
>
std::basic_ostream<_Char, _Traits>& operator <<(std::basic_ostream<_Char, _Traits>& oss, const Map<_Key, _Value, _Predicate, _Allocator>& map) {
    oss << "{ ";
    for (const auto& it : map)
        oss << '(' << it.first << ", " << it.second << "), ";
    return oss << '}';
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
