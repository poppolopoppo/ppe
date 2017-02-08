#pragma once

#include "Core/Container/FlatSet.h"

#include <algorithm>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Key, typename _EqualTo, typename _Less, typename _Vector>
TFlatSet<_Key, _EqualTo, _Less, _Vector>::TFlatSet() {}
//----------------------------------------------------------------------------
template <typename _Key, typename _EqualTo, typename _Less, typename _Vector>
TFlatSet<_Key, _EqualTo, _Less, _Vector>::TFlatSet(size_type capacity) {
    _vector.reserve(capacity);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _EqualTo, typename _Less, typename _Vector>
TFlatSet<_Key, _EqualTo, _Less, _Vector>::~TFlatSet() {}
//----------------------------------------------------------------------------
template <typename _Key, typename _EqualTo, typename _Less, typename _Vector>
TFlatSet<_Key, _EqualTo, _Less, _Vector>::TFlatSet(TFlatSet&& rvalue)
:   _vector(std::move(rvalue._vector)) {}
//----------------------------------------------------------------------------
template <typename _Key, typename _EqualTo, typename _Less, typename _Vector>
auto TFlatSet<_Key, _EqualTo, _Less, _Vector>::operator =(TFlatSet&& rvalue) -> TFlatSet& {
    _vector = std::move(rvalue._vector);
    return *this;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _EqualTo, typename _Less, typename _Vector>
TFlatSet<_Key, _EqualTo, _Less, _Vector>::TFlatSet(const TFlatSet& other)
:   _vector(other._vector) {}
//----------------------------------------------------------------------------
template <typename _Key, typename _EqualTo, typename _Less, typename _Vector>
auto TFlatSet<_Key, _EqualTo, _Less, _Vector>::operator =(const TFlatSet& other) -> TFlatSet& {
    _vector = other._vector;
    return *this;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _EqualTo, typename _Less, typename _Vector>
TFlatSet<_Key, _EqualTo, _Less, _Vector>::TFlatSet(std::initializer_list<value_type> values) {
    insert(std::begin(values), std::end(values));
}
//----------------------------------------------------------------------------
template <typename _Key, typename _EqualTo, typename _Less, typename _Vector>
auto TFlatSet<_Key, _EqualTo, _Less, _Vector>::operator =(std::initializer_list<value_type> values) -> TFlatSet& {
    _vector.clear();
    insert(std::begin(values), std::end(values));
    return *this;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _EqualTo, typename _Less, typename _Vector>
void TFlatSet<_Key, _EqualTo, _Less, _Vector>::reserve(size_type capacity) {
    _vector.reserve(capacity);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _EqualTo, typename _Less, typename _Vector>
void TFlatSet<_Key, _EqualTo, _Less, _Vector>::clear() {
    _vector.clear();
}
//----------------------------------------------------------------------------
template <typename _Key, typename _EqualTo, typename _Less, typename _Vector>
void TFlatSet<_Key, _EqualTo, _Less, _Vector>::Clear_ReleaseMemory() {
    _vector.clear();
    _vector.shrink_to_fit();
}
//----------------------------------------------------------------------------
template <typename _Key, typename _EqualTo, typename _Less, typename _Vector>
auto TFlatSet<_Key, _EqualTo, _Less, _Vector>::Find(const _Key& key) -> iterator {
    const iterator end = _vector.end();
    const iterator it = std::lower_bound(_vector.begin(), end, key, value_less());
    return ((it != end && value_equal()(*it, key)) ? it : end);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _EqualTo, typename _Less, typename _Vector>
auto TFlatSet<_Key, _EqualTo, _Less, _Vector>::FindOrAdd(const _Key& key, bool* pAdded) -> iterator {
    const iterator end = _vector.end();
    const iterator it = std::lower_bound(_vector.begin(), end, key, value_less());
    if (it != end && value_equal()(*it, key)) {
        if (pAdded)
            *pAdded = false;
        return it;
    }
    else {
        if (pAdded)
            *pAdded = true;
        return _vector.insert(it, key);
    }
}
//----------------------------------------------------------------------------
template <typename _Key, typename _EqualTo, typename _Less, typename _Vector>
auto TFlatSet<_Key, _EqualTo, _Less, _Vector>::Find(const _Key& key) const -> const_iterator {
    const const_iterator end = _vector.end();
    const const_iterator it = std::lower_bound(_vector.begin(), end, key, value_less());
    return ((it != end && value_equal()(*it, key)) ? it : end);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _EqualTo, typename _Less, typename _Vector>
auto TFlatSet<_Key, _EqualTo, _Less, _Vector>::FindAfter(const _Key& key, const iterator& after) -> iterator {
    const iterator end = _vector.end();
    const iterator first = (after == end ? end : ++iterator(after));
    const iterator it = std::lower_bound(first, end, key, value_less());
    return ((it != end && value_equal()(*it, key)) ? it : end);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _EqualTo, typename _Less, typename _Vector>
auto TFlatSet<_Key, _EqualTo, _Less, _Vector>::FindAfter(const _Key& key, const const_iterator& after) const -> const_iterator {
    const const_iterator end = _vector.end();
    const const_iterator first = (after == end ? end : ++const_iterator(after));
    const const_iterator it = std::lower_bound(first, end, key, value_less());
    return ((it != end && value_equal()(*it, key)) ? it : end);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _EqualTo, typename _Less, typename _Vector>
bool TFlatSet<_Key, _EqualTo, _Less, _Vector>::Insert_ReturnIfExists(_Key&& key) {
    const iterator end = _vector.end();
    const iterator it = std::lower_bound(_vector.begin(), end, key, value_less());
    if (it != end && value_equal()(*it, key)) {
        return true;
    }
    else {
        _vector.insert(it, std::move(key));
        return false;
    }
}
//----------------------------------------------------------------------------
template <typename _Key, typename _EqualTo, typename _Less, typename _Vector>
void TFlatSet<_Key, _EqualTo, _Less, _Vector>::Insert_KeepOldIFN(_Key&& key) {
    Insert_ReturnIfExists(std::move(key));
}
//----------------------------------------------------------------------------
template <typename _Key, typename _EqualTo, typename _Less, typename _Vector>
void TFlatSet<_Key, _EqualTo, _Less, _Vector>::Insert_AssertUnique(_Key&& key) {
    if (Insert_ReturnIfExists(std::move(key)))
        AssertNotReached();
}
//----------------------------------------------------------------------------
template <typename _Key, typename _EqualTo, typename _Less, typename _Vector>
bool TFlatSet<_Key, _EqualTo, _Less, _Vector>::Insert_ReturnIfExists(const _Key& key) {
    const iterator end = _vector.end();
    const iterator it = std::lower_bound(_vector.begin(), end, key, value_less());
    if (it != end && value_equal()(*it, key)) {
        return true;
    }
    else {
        _vector.insert(it, key);
        return false;
    }
}
//----------------------------------------------------------------------------
template <typename _Key, typename _EqualTo, typename _Less, typename _Vector>
void TFlatSet<_Key, _EqualTo, _Less, _Vector>::Insert_KeepOldIFN(const _Key& key) {
    Insert_ReturnIfExists(key);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _EqualTo, typename _Less, typename _Vector>
void TFlatSet<_Key, _EqualTo, _Less, _Vector>::Insert_AssertUnique(const _Key& key) {
    if (Insert_ReturnIfExists(key))
        AssertNotReached();
}
//----------------------------------------------------------------------------
template <typename _Key, typename _EqualTo, typename _Less, typename _Vector>
bool TFlatSet<_Key, _EqualTo, _Less, _Vector>::Erase(const _Key& key) {
    const const_iterator it = Find(key);
    if (end() == it)
        return false;
    Erase(it);
    return true;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _EqualTo, typename _Less, typename _Vector>
void TFlatSet<_Key, _EqualTo, _Less, _Vector>::Erase(const const_iterator& it) {
    _vector.erase(it);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _EqualTo, typename _Less, typename _Vector>
void TFlatSet<_Key, _EqualTo, _Less, _Vector>::Remove_AssertExists(const _Key& key) {
    const const_iterator it = Find(key);
    Assert(end() != it);
    Erase(it);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _EqualTo, typename _Less, typename _Vector>
template <typename _It>
void TFlatSet<_Key, _EqualTo, _Less, _Vector>::insert(_It&& begin, _It&& end) {
    _vector.insert(_vector.end(), begin, end);
    std::sort(_vector.begin(), _vector.end(), value_less());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
