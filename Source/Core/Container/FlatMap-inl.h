#pragma once

#include "Core/Container/FlatMap.h"

#include <algorithm>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Less, typename _Vector>
TFlatMap<_Key, _Value, _EqualTo, _Less, _Vector>::TFlatMap() {
    typedef typename std::iterator_traits< iterator >::iterator_category iterator_category;
    STATIC_ASSERT(std::is_same< iterator_category, std::random_access_iterator_tag >::value);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Less, typename _Vector>
TFlatMap<_Key, _Value, _EqualTo, _Less, _Vector>::TFlatMap(size_type capacity) : TFlatMap() {
    _vector.reserve(capacity);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Less, typename _Vector>
TFlatMap<_Key, _Value, _EqualTo, _Less, _Vector>::~TFlatMap() {}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Less, typename _Vector>
TFlatMap<_Key, _Value, _EqualTo, _Less, _Vector>::TFlatMap(TFlatMap&& rvalue)
:   _vector(std::move(rvalue._vector)) {}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Less, typename _Vector>
auto TFlatMap<_Key, _Value, _EqualTo, _Less, _Vector>::operator =(TFlatMap&& rvalue) -> TFlatMap& {
    _vector = std::move(rvalue._vector);
    return *this;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Less, typename _Vector>
TFlatMap<_Key, _Value, _EqualTo, _Less, _Vector>::TFlatMap(const TFlatMap& other)
:   _vector(other._vector) {}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Less, typename _Vector>
auto TFlatMap<_Key, _Value, _EqualTo, _Less, _Vector>::operator =(const TFlatMap& other) -> TFlatMap& {
    _vector = other._vector;
    return *this;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Less, typename _Vector>
TFlatMap<_Key, _Value, _EqualTo, _Less, _Vector>::TFlatMap(std::initializer_list<value_type> values) {
    insert(std::begin(values), std::end(values));
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Less, typename _Vector>
auto TFlatMap<_Key, _Value, _EqualTo, _Less, _Vector>::operator =(std::initializer_list<value_type> values) -> TFlatMap& {
    _vector.clear();
    insert(std::begin(values), std::end(values));
    return *this;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Less, typename _Vector>
void TFlatMap<_Key, _Value, _EqualTo, _Less, _Vector>::reserve(size_type capacity) {
    _vector.reserve(capacity);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Less, typename _Vector>
void TFlatMap<_Key, _Value, _EqualTo, _Less, _Vector>::clear() {
    _vector.clear();
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Less, typename _Vector>
void TFlatMap<_Key, _Value, _EqualTo, _Less, _Vector>::clear_ReleaseMemory() {
    _vector.clear();
    _vector.shrink_to_fit();
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Less, typename _Vector>
auto TFlatMap<_Key, _Value, _EqualTo, _Less, _Vector>::Find(const _Key& key) -> iterator {
    const iterator end = _vector.end();
    const iterator it = std::lower_bound(_vector.begin(), end, key, FKeyLess_());
    return ((it != end && FKeyEqual_()(*it, key)) ? it : end);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Less, typename _Vector>
auto TFlatMap<_Key, _Value, _EqualTo, _Less, _Vector>::FindOrAdd(const _Key& key, bool* pAdded) -> iterator {
    const iterator end = _vector.end();
    const iterator it = std::lower_bound(_vector.begin(), end, key, FKeyLess_());
    if (it != end && FKeyEqual_()(*it, key)) {
        if (pAdded)
            *pAdded = false;
        return it;
    }
    else {
        if (pAdded)
            *pAdded = true;
        return _vector.insert(it, value_type{ key, mapped_type() });
    }
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Less, typename _Vector>
auto TFlatMap<_Key, _Value, _EqualTo, _Less, _Vector>::Find(const _Key& key) const -> const_iterator {
    const const_iterator end = _vector.end();
    const const_iterator it = std::lower_bound(_vector.begin(), end, key, FKeyLess_());
    return ((it != end && FKeyEqual_()(*it, key)) ? it : end);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Less, typename _Vector>
auto TFlatMap<_Key, _Value, _EqualTo, _Less, _Vector>::FindAfter(const _Key& key, const iterator& after) -> iterator {
    const iterator end = _vector.end();
    const iterator first = (after == end ? end : ++iterator(after));
    const iterator it = std::lower_bound(first, end, key, FKeyLess_());
    return ((it != end && FKeyEqual_()(*it, key)) ? it : end);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Less, typename _Vector>
auto TFlatMap<_Key, _Value, _EqualTo, _Less, _Vector>::FindAfter(const _Key& key, const const_iterator& after) const -> const_iterator {
    const const_iterator end = _vector.end();
    const const_iterator first = (after == end ? end : ++const_iterator(after));
    const const_iterator it = std::lower_bound(first, end, key, FKeyLess_());
    return ((it != end && FKeyEqual_()(*it, key)) ? it : end);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Less, typename _Vector>
bool TFlatMap<_Key, _Value, _EqualTo, _Less, _Vector>::Find(const _Key& key, _Value *pvalue) const {
    const const_iterator it = Find(key);
    if (end() == it)
        return false;
    Assert(pvalue);
    *pvalue = it->second;
    return true;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Less, typename _Vector>
template <class... _Args>
bool TFlatMap<_Key, _Value, _EqualTo, _Less, _Vector>::Emplace_ReturnIfExists(_Key&& key, _Args&&... args) {
    const iterator end = _vector.end();
    const iterator it = std::lower_bound(_vector.begin(), end, key, FKeyLess_());
    if (it != end && FKeyEqual_()(*it, key)) {
        return true;
    }
    else {
        _vector.insert(it, value_type{ std::move(key), mapped_type(std::forward<_Args>(args)...) });
        return false;
    }
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Less, typename _Vector>
template <class... _Args>
void TFlatMap<_Key, _Value, _EqualTo, _Less, _Vector>::Emplace_KeepOldIFN(_Key&& key, _Args&&... args) {
    Emplace_ReturnIfExists(std::move(key), std::forward<_Args>(args)...);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Less, typename _Vector>
template <class... _Args>
void TFlatMap<_Key, _Value, _EqualTo, _Less, _Vector>::Emplace_AssertUnique(_Key&& key, _Args&&... args) {
    if (Emplace_ReturnIfExists(std::move(key), std::forward<_Args>(args)...))
        AssertNotReached();
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Less, typename _Vector>
bool TFlatMap<_Key, _Value, _EqualTo, _Less, _Vector>::Insert_ReturnIfExists(_Key&& key, _Value&& rvalue) {
    return Emplace_ReturnIfExists(std::move(key), std::move(rvalue));
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Less, typename _Vector>
void TFlatMap<_Key, _Value, _EqualTo, _Less, _Vector>::Insert_KeepOldIFN(_Key&& key, _Value&& rvalue) {
    Insert_ReturnIfExists(std::move(key), std::move(rvalue));
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Less, typename _Vector>
void TFlatMap<_Key, _Value, _EqualTo, _Less, _Vector>::Insert_AssertUnique(_Key&& key, _Value&& rvalue) {
    if (Insert_ReturnIfExists(std::move(key), std::move(rvalue)))
        AssertNotReached();
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Less, typename _Vector>
bool TFlatMap<_Key, _Value, _EqualTo, _Less, _Vector>::Insert_ReturnIfExists(const _Key& key, const _Value& value) {
    const iterator end = _vector.end();
    const iterator it = std::lower_bound(_vector.begin(), end, key, FKeyLess_());
    if (it != end && FKeyEqual_()(*it, key)) {
        return true;
    }
    else {
        _vector.insert(it, value_type{ key, value });
        return false;
    }
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Less, typename _Vector>
void TFlatMap<_Key, _Value, _EqualTo, _Less, _Vector>::Insert_KeepOldIFN(const _Key& key, const _Value& value) {
    Insert_ReturnIfExists(key, value);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Less, typename _Vector>
void TFlatMap<_Key, _Value, _EqualTo, _Less, _Vector>::Insert_AssertUnique(const _Key& key, const _Value& value) {
    if (Insert_ReturnIfExists(key, value))
        AssertNotReached();
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Less, typename _Vector>
_Value& TFlatMap<_Key, _Value, _EqualTo, _Less, _Vector>::Get(const _Key& key) {
    const iterator it = Find(key);
    AssertRelease(end() != it);
    return it->second;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Less, typename _Vector>
_Value* TFlatMap<_Key, _Value, _EqualTo, _Less, _Vector>::GetIFP(const _Key& key) {
    const iterator it = Find(key);
    return (end() != it ? &it->second : nullptr);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Less, typename _Vector>
bool TFlatMap<_Key, _Value, _EqualTo, _Less, _Vector>::TryGet(const _Key& key, _Value *value) const {
    Assert(value);

    const const_iterator it = Find(key);
    if (end() == it)
        return false;

    *value = it->second;
    return true;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Less, typename _Vector>
const _Value& TFlatMap<_Key, _Value, _EqualTo, _Less, _Vector>::At(const _Key& key) const {
    const const_iterator it = Find(key);
    Assert(_vector.end() != it);
    return it->second;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Less, typename _Vector>
bool TFlatMap<_Key, _Value, _EqualTo, _Less, _Vector>::Erase(const _Key& key) {
    const const_iterator it = Find(key);
    if (end() == it)
        return false;
    Erase(it);
    return true;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Less, typename _Vector>
void TFlatMap<_Key, _Value, _EqualTo, _Less, _Vector>::Erase(const const_iterator& it) {
    _vector.erase(it);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Less, typename _Vector>
void TFlatMap<_Key, _Value, _EqualTo, _Less, _Vector>::Remove_AssertExists(const _Key& key, const _Value& valueForDebug) {
    UNUSED(valueForDebug);
    const const_iterator it = Find(key);
    Assert(end() != it);
    Assert(valueForDebug == it->second);
    Erase(it);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Less, typename _Vector>
void TFlatMap<_Key, _Value, _EqualTo, _Less, _Vector>::Remove_AssertExists(const _Key& key) {
    const const_iterator it = Find(key);
    Assert(end() != it);
    Erase(it);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Less, typename _Vector>
template <typename _It>
void TFlatMap<_Key, _Value, _EqualTo, _Less, _Vector>::insert(_It&& begin, _It&& end) {
    _vector.insert(_vector.end(), begin, end);
    std::sort(_vector.begin(), _vector.end(), [](const value_type& lhs, const value_type& rhs) {
        return key_less()(lhs.first, rhs.first);
    });
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
