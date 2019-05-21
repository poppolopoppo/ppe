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
using TMultiMap = std::multimap<
    _Key,
    _Value,
    _Predicate,
    TStlAllocator<std::pair<_Key, _Value>, _Allocator>
>;
//----------------------------------------------------------------------------
#define MULTIMAP(_DOMAIN, _KEY, _VALUE) \
    ::PPE::TMultiMap<_KEY, _VALUE, ::PPE::Meta::TLess<_KEY>, ALLOCATOR(_DOMAIN) >
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Pred, typename _Allocator>
hash_t hash_value(const TMultiMap<_Key, _Value, _Pred, _Allocator>& multiMap) {
    return hash_range(multiMap.begin(), multiMap.end());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Pred, typename _Allocator>
bool TryGetValue(const TMultiMap<_Key, _Value, _Pred, _Allocator>& multiMap, const _Key& key, _Value *value) {
    Assert(value);

    const auto it = multiMap.find(key);
    if (multiMap.end() == it)
        return false;

    *value = it->second;
    return true;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Pred, typename _Allocator>
bool Insert_ReturnIfExists(TMultiMap<_Key, _Value, _Pred, _Allocator>& multiMap, const _Key& key, const _Value& value) {
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
void Insert_AssertUnique(TMultiMap<_Key, _Value, _Pred, _Allocator>& multiMap, const _Key& key, const _Value& value) {
    if (Insert_ReturnIfExists(multiMap, key, value))
        AssertNotReached();
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Pred, typename _Allocator>
bool Remove_ReturnIfExists(TMultiMap<_Key, _Value, _Pred, _Allocator>& multiMap, const _Key& key) {
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
void Remove_AssertExists(TMultiMap<_Key, _Value, _Pred, _Allocator>& multiMap, const _Key& key) {
    if (!Remove_ReturnIfExists(multiMap, key))
        AssertNotReached();
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Pred, typename _Allocator>
void RemoveKeyValue_AssertExists(TMultiMap<_Key, _Value, _Pred, _Allocator>& multiMap, const _Key& key, const _Value& value) {

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
size_t FillMatchingValues_ReturnCount(_Value *pValues, size_t capacity, const TMultiMap<_Key, _Value, _Pred, _Allocator>& multiMap, const _Key& key) {
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
template <typename _Key, typename _Value, typename _Pred, typename _Allocator>
FTextWriter& operator <<(FTextWriter& oss, const TMultiMap<_Key, _Value, _Pred, _Allocator>& multiMap) {
    oss << "{ ";
    for (const auto& it : multiMap)
        oss << '(' << it.first << ", " << it.second << "), ";
    return oss << '}';
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Pred, typename _Allocator>
FWTextWriter& operator <<(FWTextWriter& oss, const TMultiMap<_Key, _Value, _Pred, _Allocator>& multiMap) {
    oss << L"{ ";
    for (const auto& it : multiMap)
        oss << L'(' << it.first << L", " << it.second << L"), ";
    return oss << L'}';
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
