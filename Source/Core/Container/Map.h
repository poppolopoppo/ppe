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
    typename _Predicate = Meta::TLess<_Key>,
    typename _Allocator = NODEBASED_CONTAINER_ALLOCATOR(Container, TPair<_Key COMMA _Value>)
>
using TMap = std::map<_Key, _Value, _Predicate, _Allocator >;
//----------------------------------------------------------------------------
#define MAP(_DOMAIN, _KEY, _VALUE) \
    ::Core::TMap<_KEY, _VALUE, ::Core::Meta::TLess<_KEY>, NODEBASED_CONTAINER_ALLOCATOR(_DOMAIN, ::Core::TPair<_KEY COMMA _VALUE>) >
//----------------------------------------------------------------------------
#define MAP_THREAD_LOCAL(_DOMAIN, _KEY, _VALUE) \
    ::Core::TMap<_KEY, _VALUE, ::Core::Meta::TLess<_KEY>, THREAD_LOCAL_NODEBASED_CONTAINER_ALLOCATOR(_DOMAIN, ::Core::TPair<_KEY COMMA _VALUE>) >
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Pred, typename _Allocator>
hash_t hash_value(const TMap<_Key, _Value, _Pred, _Allocator>& map) {
    return hash_range(map.begin(), map.end());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Predicate, typename _Allocator>
bool TryGetValue(const TMap<_Key, _Value, _Predicate, _Allocator>& map, const _Key& key, _Value *value) {
    Assert(value);

    const auto it = map.find(key);
    if (map.end() == it)
        return false;

    *value = it->second;
    return true;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Predicate, typename _Allocator>
bool Insert_ReturnIfExists(TMap<_Key, _Value, _Predicate, _Allocator>& map, const _Key& key, const _Value& value) {
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
void Insert_AssertUnique(TMap<_Key, _Value, _Predicate, _Allocator>& map, const _Key& key, const _Value& value) {
    if (Insert_ReturnIfExists(map, key, value))
        AssertNotReached();
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Predicate, typename _Allocator>
bool Remove_ReturnIfExists(TMap<_Key, _Value, _Predicate, _Allocator>& map, const _Key& key) {
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
void Remove_AssertExists(TMap<_Key, _Value, _Predicate, _Allocator>& map, const _Key& key) {
    if (!Remove_ReturnIfExists(map, key))
        AssertNotReached();
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Predicate, typename _Allocator>
void Remove_AssertExistsAndSameValue(TMap<_Key, _Value, _Predicate, _Allocator>& map, const _Key& key, const _Value& value) {
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
std::basic_ostream<_Char, _Traits>& operator <<(std::basic_ostream<_Char, _Traits>& oss, const TMap<_Key, _Value, _Predicate, _Allocator>& map) {
    oss << "{ ";
    for (const auto& it : map)
        oss << '(' << it.first << ", " << it.second << "), ";
    return oss << '}';
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
