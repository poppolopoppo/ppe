#pragma once

#include "Core.h"

#include "Allocator/Allocation.h"
#include "Allocator/StlAllocator.h"
#include "Container/Pair.h"
#include "IO/TextWriter_fwd.h"

#include <map>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <
    typename _Key,
    typename _Value,
    typename _Predicate = Meta::TLess<_Key>,
    typename _Allocator = ALLOCATOR(Container)
>
using TMap = std::map<
    _Key,
    _Value,
    _Predicate,
    TStlAllocator< std::pair<const _Key, _Value>, _Allocator>
>;
//----------------------------------------------------------------------------
#define MAP(_DOMAIN, _KEY, _VALUE) \
    ::PPE::TMap<_KEY, _VALUE, ::PPE::Meta::TLess<_KEY>, ALLOCATOR(_DOMAIN) >
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
template <typename _Key, typename _Value, typename _Predicate, typename _Allocator>
FTextWriter& operator <<(FTextWriter& oss, const TMap<_Key, _Value, _Predicate, _Allocator>& map) {
    oss << "{ ";
    for (const auto& it : map)
        oss << '(' << it.first << ", " << it.second << "), ";
    return oss << '}';
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Predicate, typename _Allocator>
FWTextWriter& operator <<(FWTextWriter& oss, const TMap<_Key, _Value, _Predicate, _Allocator>& map) {
    oss << L"{ ";
    for (const auto& it : map)
        oss << L'(' << it.first << L", " << it.second << L"), ";
    return oss << L'}';
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
