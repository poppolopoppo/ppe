#pragma once

#include "Core/Core.h"

#include "Core/Allocator/Allocation.h"
#include "Core/Container/Hash.h"
#include "Core/Container/Pair.h"

#include <unordered_map>

#include <iosfwd>

// http://codecapsule.com/2013/11/17/robin-hood-hashing-backward-shift-deletion/
// https://github.com/goossaert/hashmap/blob/master/backshift_hashmap.cc
// http://codecapsule.com/2013/08/11/hopscotch-hashing/
// http://www.sebastiansylvan.com/post/robin-hood-hashing-should-be-your-default-hash-table-implementation/
// http://stackoverflow.com/questions/245878/how-do-i-choose-between-a-hash-table-and-a-trie-prefix-tree

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <
    typename _Key,
    typename _Value,
    typename _Hasher = Hash<_Key>,
    typename _EqualTo = Meta::EqualTo<_Key>,
    typename _Allocator = ALLOCATOR(Container, Pair<_Key COMMA _Value>)
>
using HashMap = std::unordered_map<_Key, _Value, _Hasher, _EqualTo, _Allocator>;
//----------------------------------------------------------------------------
#define HASHMAP(_DOMAIN, _KEY, _VALUE) \
    ::Core::HashMap<_KEY, _VALUE, ::Core::Hash<_KEY>, ::Core::Meta::EqualTo<_KEY>, ALLOCATOR(_DOMAIN, ::Core::Pair<_KEY COMMA _VALUE>)>
//----------------------------------------------------------------------------
#define HASHMAP_THREAD_LOCAL(_DOMAIN, _KEY, _VALUE) \
    ::Core::HashMap<_KEY, _VALUE, ::Core::Hash<_KEY>, ::Core::Meta::EqualTo<_KEY>, THREAD_LOCAL_ALLOCATOR(_DOMAIN, ::Core::Pair<_KEY COMMA _VALUE>)>
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
hash_t hash_value(const HashMap<_Key, _Value, _Hasher, _EqualTo, _Allocator>& hashMap) {
    return hash_range(hashMap.begin(), hashMap.end());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
bool TryGetValue(const HashMap<_Key, _Value, _Hasher, _EqualTo, _Allocator>& hashmap, const _Key& key, _Value *value) {
    Assert(value);

    const auto it = hashmap.find(key);
    if (hashmap.end() == it)
        return false;

    *value = it->second;
    return true;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
bool Insert_ReturnIfExists(HashMap<_Key, _Value, _Hasher, _EqualTo, _Allocator>& hashmap, const _Key& key, const _Value& value) {
    const auto it = hashmap.lower_bound(key);
    if (it != hashmap.end() && !(hashmap.key_comp()(key, it->first)) ) {
        Assert(it->second == value);
        return true;
    }
    else {
        hashmap.insert(it, MakePair(key, value));
        return false;
    }
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
bool Insert_ReturnIfExistsAndCopyValue(HashMap<_Key, _Value, _Hasher, _EqualTo, _Allocator>& hashmap, const _Key& key, _Value* pvalue) {
    Assert(pvalue);
    const auto it = hashmap.lower_bound(key);
    if (it != hashmap.end() && !(hashmap.key_comp()(key, it->first)) ) {
        *pvalue = it->second;
        return true;
    }
    else {
        hashmap.insert(it, MakePair(key, value));
        return false;
    }
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
void Insert_AssertUnique(HashMap<_Key, _Value, _Hasher, _EqualTo, _Allocator>& hashmap, const _Key& key, const _Value& value) {
#ifdef WITH_CORE_ASSERT
    const auto it = hashmap.lower_bound(key);
    if (it != hashmap.end() && !(hashmap.key_comp()(key, it->first)) )
        AssertNotReached();
    else
        hashmap.insert(it, MakePair(key, value));
#else
    hashmap[key] = value;
#endif
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
bool Remove_ReturnIfExists(HashMap<_Key, _Value, _Hasher, _EqualTo, _Allocator>& hashmap, const _Key& key) {
    const auto it = hashmap.find(key);
    if (hashmap.end() == it) {
        return false;
    }
    else {
        hashmap.erase(it);
        return true;
    }
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
void Remove_AssertExists(HashMap<_Key, _Value, _Hasher, _EqualTo, _Allocator>& hashmap, const _Key& key) {
    if (!Remove_ReturnIfExists(hashmap, key))
        AssertNotReached();
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
_Value Remove_ReturnValue(HashMap<_Key, _Value, _Hasher, _EqualTo, _Allocator>& hashmap, const _Key& key) {
    const auto it = hashmap.find(key);
    AssertRelease(hashmap.end() != it);
    _Value result(std::move(it->second));
    hashmap.erase(it);
    return std::move(result);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
void Remove_AssertExistsAndSameValue(HashMap<_Key, _Value, _Hasher, _EqualTo, _Allocator>& hashmap, const _Key& key, const _Value& value) {
    const auto it = hashmap.find(key);
    if (hashmap.end() == it) {
        AssertNotReached();
    }
    else {
        Assert(it->second == value);
        hashmap.erase(it);
    }
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
std::basic_ostream<_Char, _Traits>& operator <<(std::basic_ostream<_Char, _Traits>& oss, const HashMap<_Key, _Value, _Hasher, _EqualTo, _Allocator>& hashMap) {
    oss << "{ ";
    for (const auto& it : hashMap)
        oss << '(' << it.first << ", " << it.second << "), ";
    return oss << '}';
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
