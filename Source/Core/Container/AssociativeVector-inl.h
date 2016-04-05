#pragma once

#include "Core/Container/AssociativeVector.h"

#include <algorithm>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
AssociativeVector<_Key, _Value, _EqualTo, _Vector>::AssociativeVector() {}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
AssociativeVector<_Key, _Value, _EqualTo, _Vector>::AssociativeVector(size_type capacity) {
    _vector.reserve(capacity);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
AssociativeVector<_Key, _Value, _EqualTo, _Vector>::~AssociativeVector() {}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
AssociativeVector<_Key, _Value, _EqualTo, _Vector>::AssociativeVector(vector_type&& vector)
:   _vector(std::move(vector)) {}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
auto AssociativeVector<_Key, _Value, _EqualTo, _Vector>::operator =(vector_type&& vector) -> AssociativeVector& {
    _vector = std::move(vector);
    return *this;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
AssociativeVector<_Key, _Value, _EqualTo, _Vector>::AssociativeVector(const vector_type& vector)
:   _vector(vector) {}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
auto AssociativeVector<_Key, _Value, _EqualTo, _Vector>::operator =(const vector_type& vector) -> AssociativeVector& {
    _vector = vector;
    return *this;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
AssociativeVector<_Key, _Value, _EqualTo, _Vector>::AssociativeVector(AssociativeVector&& rvalue)
:   _vector(std::move(rvalue._vector)) {}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
auto AssociativeVector<_Key, _Value, _EqualTo, _Vector>::operator =(AssociativeVector&& rvalue) -> AssociativeVector& {
    _vector = std::move(rvalue._vector);
    return *this;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
AssociativeVector<_Key, _Value, _EqualTo, _Vector>::AssociativeVector(const AssociativeVector& other)
:   _vector(other._vector) {}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
auto AssociativeVector<_Key, _Value, _EqualTo, _Vector>::operator =(const AssociativeVector& other) -> AssociativeVector& {
    _vector = other._vector;
    return *this;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
void AssociativeVector<_Key, _Value, _EqualTo, _Vector>::reserve(size_type capacity) {
    _vector.reserve(capacity);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
void AssociativeVector<_Key, _Value, _EqualTo, _Vector>::clear() {
    _vector.clear();
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
void AssociativeVector<_Key, _Value, _EqualTo, _Vector>::Clear_ReleaseMemory() {
    _vector.clear();
    _vector.shrink_to_fit();
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
auto AssociativeVector<_Key, _Value, _EqualTo, _Vector>::Find(const _Key& key) -> iterator {
    const iterator end = _vector.end();
    for (iterator it = _vector.begin(); it != end; ++it)
        if (key_equal()(it->first, key))
            return it;
    return end;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
auto AssociativeVector<_Key, _Value, _EqualTo, _Vector>::Find(const _Key& key) const -> const_iterator {
    const const_iterator end = _vector.end();
    for (const_iterator it = _vector.begin(); it != end; ++it)
        if (key_equal()(it->first, key))
            return it;
    return end;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
auto AssociativeVector<_Key, _Value, _EqualTo, _Vector>::FindAfter(const _Key& key, const iterator& after) -> iterator {
    const iterator end = _vector.end();
    for (iterator it = (after == end ? end : after + 1); it != end; ++it)
        if (key_equal()(it->first, key))
            return it;
    return end;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
auto AssociativeVector<_Key, _Value, _EqualTo, _Vector>::FindAfter(const _Key& key, const const_iterator& after) const -> const_iterator {
    const const_iterator end = _vector.end();
    for (const_iterator it = (after == end ? end : after + 1); it != end; ++it)
        if (key_equal()(it->first, key))
            return it;
    return end;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
bool AssociativeVector<_Key, _Value, _EqualTo, _Vector>::Find(const _Key& key, _Value *pvalue) const {
    const const_iterator it = Find(key);
    if (end() == it)
        return false;
    Assert(pvalue);
    *pvalue = it->second;
    return true;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
bool AssociativeVector<_Key, _Value, _EqualTo, _Vector>::Insert_ReturnIfExists(_Key&& key, _Value&& rvalue) {
    if (end() != Find(key))
        return true;
    _vector.emplace_back(std::forward<_Key>(key), std::forward<_Value>(rvalue));
    return false;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
void AssociativeVector<_Key, _Value, _EqualTo, _Vector>::Insert_KeepOldIFN(_Key&& key, _Value&& rvalue) {
    Insert_ReturnIfExists(std::forward<_Key>(key), std::forward<_Value>(rvalue));
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
void AssociativeVector<_Key, _Value, _EqualTo, _Vector>::Insert_AssertUnique(_Key&& key, _Value&& rvalue) {
#ifdef _DEBUG
    if (Insert_ReturnIfExists(std::forward<_Key>(key), std::forward<_Value>(rvalue)))
        Assert(false);
#else
    _vector.emplace_back(std::forward<_Key>(key), std::forward<_Value>(rvalue));
#endif
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
bool AssociativeVector<_Key, _Value, _EqualTo, _Vector>::Insert_ReturnIfExists(const _Key& key, const _Value& value) {
    if (end() != Find(key))
        return true;
    _vector.emplace_back(key, value);
    return false;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
void AssociativeVector<_Key, _Value, _EqualTo, _Vector>::Insert_KeepOldIFN(const _Key& key, const _Value& value) {
    Insert_ReturnIfExists(key, value);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
void AssociativeVector<_Key, _Value, _EqualTo, _Vector>::Insert_AssertUnique(const _Key& key, const _Value& value) {
#ifdef _DEBUG
    if (Insert_ReturnIfExists(key, value))
        Assert(false);
#else
    _vector.emplace_back(key, value);
#endif
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
_Value& AssociativeVector<_Key, _Value, _EqualTo, _Vector>::Get(const _Key& key) {
    const iterator it = Find(key);
    AssertRelease(end() != it);
    return it->second;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
bool AssociativeVector<_Key, _Value, _EqualTo, _Vector>::TryGet(const _Key& key, _Value *value) const {
    Assert(value);

    const const_iterator it = Find(key);
    if (end() == it)
        return false;

    *value = it->second;
    return true;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
const _Value& AssociativeVector<_Key, _Value, _EqualTo, _Vector>::At(const _Key& key) const {
    const const_iterator it = Find(key);
    Assert(_vector.end() != it);
    return it->second;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
bool AssociativeVector<_Key, _Value, _EqualTo, _Vector>::Erase(const _Key& key) {
    const const_iterator it = Find(key);
    if (end() == it)
        return false;
    Erase(it);
    return true;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
void AssociativeVector<_Key, _Value, _EqualTo, _Vector>::Erase(const const_iterator& it) {
    Erase_DontPreserveOrder(_vector, it);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
void AssociativeVector<_Key, _Value, _EqualTo, _Vector>::Remove_AssertExists(const _Key& key, const _Value& valueForDebug) {
    UNUSED(valueForDebug);
    const const_iterator it = Find(key);
    Assert(end() != it);
    Assert(valueForDebug == it->second);
    Erase(it);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
void AssociativeVector<_Key, _Value, _EqualTo, _Vector>::Remove_AssertExists(const _Key& key) {
    const const_iterator it = Find(key);
    Assert(end() != it);
    Erase(it);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
