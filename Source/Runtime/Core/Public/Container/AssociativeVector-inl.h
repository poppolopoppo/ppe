#pragma once

#include "Container/AssociativeVector.h"

#include <algorithm>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
TAssociativeVector<_Key, _Value, _EqualTo, _Vector>::TAssociativeVector() {}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
TAssociativeVector<_Key, _Value, _EqualTo, _Vector>::TAssociativeVector(allocator_type&& alloc)
    : _vector(std::move(alloc))
{}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
TAssociativeVector<_Key, _Value, _EqualTo, _Vector>::TAssociativeVector(Meta::FForceInit forceInit)
    : _vector(forceInit)
{}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
TAssociativeVector<_Key, _Value, _EqualTo, _Vector>::TAssociativeVector(size_type capacity) {
    _vector.reserve(capacity);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
TAssociativeVector<_Key, _Value, _EqualTo, _Vector>::~TAssociativeVector() {}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
TAssociativeVector<_Key, _Value, _EqualTo, _Vector>::TAssociativeVector(vector_type&& vector)
:   _vector(std::move(vector)) {}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
auto TAssociativeVector<_Key, _Value, _EqualTo, _Vector>::operator =(vector_type&& vector) -> TAssociativeVector& {
    _vector = std::move(vector);
    return *this;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
TAssociativeVector<_Key, _Value, _EqualTo, _Vector>::TAssociativeVector(const vector_type& vector)
:   _vector(vector) {}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
auto TAssociativeVector<_Key, _Value, _EqualTo, _Vector>::operator =(const vector_type& vector) -> TAssociativeVector& {
    _vector = vector;
    return *this;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
TAssociativeVector<_Key, _Value, _EqualTo, _Vector>::TAssociativeVector(TAssociativeVector&& rvalue)
:   _vector(std::move(rvalue._vector)) {}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
auto TAssociativeVector<_Key, _Value, _EqualTo, _Vector>::operator =(TAssociativeVector&& rvalue) -> TAssociativeVector& {
    _vector = std::move(rvalue._vector);
    return *this;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
TAssociativeVector<_Key, _Value, _EqualTo, _Vector>::TAssociativeVector(const TAssociativeVector& other)
:   _vector(other._vector) {}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
auto TAssociativeVector<_Key, _Value, _EqualTo, _Vector>::operator =(const TAssociativeVector& other) -> TAssociativeVector& {
    _vector = other._vector;
    return *this;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
void TAssociativeVector<_Key, _Value, _EqualTo, _Vector>::reserve(size_type capacity) {
    _vector.reserve(capacity);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
void TAssociativeVector<_Key, _Value, _EqualTo, _Vector>::clear() {
    _vector.clear();
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
void TAssociativeVector<_Key, _Value, _EqualTo, _Vector>::clear_ReleaseMemory() {
    _vector.clear_ReleaseMemory();
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
_Value& TAssociativeVector<_Key, _Value, _EqualTo, _Vector>::Add(_Key&& rkey) {
    Assert(end() == Find(rkey));
    _vector.emplace_back();
    _vector.back().first = std::move(rkey);
    return _vector.back().second;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
_Value& TAssociativeVector<_Key, _Value, _EqualTo, _Vector>::Add(const _Key& key) {
    Assert(end() == Find(key));
    _vector.emplace_back();
    _vector.back().first = key;
    return _vector.back().second;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
auto TAssociativeVector<_Key, _Value, _EqualTo, _Vector>::Find(const _Key& key) -> iterator {
    return std::find_if(_vector.begin(), _vector.end(), [&key](const value_type& it) {
        return key_equal()(it.first, key);
    });
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
auto TAssociativeVector<_Key, _Value, _EqualTo, _Vector>::FindOrAdd(const _Key& key, bool* pAdded) -> iterator {
    iterator it = Find(key);
    if (end() == it) {
        if (pAdded)
            *pAdded = true;
        _vector.emplace_back();
        _vector.back().first = key;
        return (_vector.end() - 1);
    }
    else {
        if (pAdded)
            *pAdded = false;
        return it;
    }
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
auto TAssociativeVector<_Key, _Value, _EqualTo, _Vector>::Find(const _Key& key) const -> const_iterator {
    return std::find_if(_vector.begin(), _vector.end(), [&key](const value_type& it) {
        return key_equal()(it.first, key);
    });
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
auto TAssociativeVector<_Key, _Value, _EqualTo, _Vector>::FindAfter(const _Key& key, const iterator& after) -> iterator {
    const iterator end = _vector.end();
    const iterator first = (after == end ? end : after + 1);
    return std::find_if(first, end, [&key](const value_type& it) {
        return key_equal()(it.first, key);
    });
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
auto TAssociativeVector<_Key, _Value, _EqualTo, _Vector>::FindAfter(const _Key& key, const const_iterator& after) const -> const_iterator {
    const const_iterator end = _vector.end();
    const const_iterator first = (after == end ? end : after + 1);
    return std::find_if(first, end, [&key](const value_type& it) {
        return key_equal()(it.first, key);
    });
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
bool TAssociativeVector<_Key, _Value, _EqualTo, _Vector>::Find(const _Key& key, _Value *pvalue) const {
    const const_iterator it = Find(key);
    if (end() == it)
        return false;
    Assert(pvalue);
    *pvalue = it->second;
    return true;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
template <class... _Args>
bool TAssociativeVector<_Key, _Value, _EqualTo, _Vector>::Emplace_ReturnIfExists(_Key&& key, _Args&&... args) {
    if (end() == Find(key)) {
        _vector.emplace_back(std::move(key), mapped_type(std::forward<_Args>(args)...));
        return false;
    }
    else {
        return true;
    }
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
template <class... _Args>
void TAssociativeVector<_Key, _Value, _EqualTo, _Vector>::Emplace_KeepOldIFN(_Key&& key, _Args&&... args) {
    Emplace_ReturnIfExists(std::move(key), std::forward<_Args>(args)...);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
template <class... _Args>
void TAssociativeVector<_Key, _Value, _EqualTo, _Vector>::Emplace_AssertUnique(_Key&& key, _Args&&... args) {
    Assert(end() == Find(key));
    _vector.emplace_back(std::move(key), mapped_type(std::forward<_Args>(args)...));
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
bool TAssociativeVector<_Key, _Value, _EqualTo, _Vector>::Insert_ReturnIfExists(_Key&& key, _Value&& rvalue) {
    if (end() == Find(key)) {
        _vector.emplace_back(std::move(key), std::move(rvalue));
        return false;
    }
    else {
        return true;
    }
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
void TAssociativeVector<_Key, _Value, _EqualTo, _Vector>::Insert_KeepOldIFN(_Key&& key, _Value&& rvalue) {
    Insert_ReturnIfExists(std::move(key), std::move(rvalue));
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
void TAssociativeVector<_Key, _Value, _EqualTo, _Vector>::Insert_AssertUnique(_Key&& key, _Value&& rvalue) {
    Assert(end() == Find(key));
    _vector.emplace_back(std::move(key), std::move(rvalue));
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
bool TAssociativeVector<_Key, _Value, _EqualTo, _Vector>::Insert_ReturnIfExists(const _Key& key, const _Value& value) {
    if (end() == Find(key)) {
        _vector.emplace_back(key, value);
        return false;
    }
    else {
        return true;
    }
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
void TAssociativeVector<_Key, _Value, _EqualTo, _Vector>::Insert_KeepOldIFN(const _Key& key, const _Value& value) {
    Insert_ReturnIfExists(key, value);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
void TAssociativeVector<_Key, _Value, _EqualTo, _Vector>::Insert_AssertUnique(const _Key& key, const _Value& value) {
    Assert(end() == Find(key));
    _vector.emplace_back(key, value);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
_Value& TAssociativeVector<_Key, _Value, _EqualTo, _Vector>::Get(const _Key& key) {
    const iterator it = Find(key);
    AssertRelease(end() != it);
    return it->second;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
_Value* TAssociativeVector<_Key, _Value, _EqualTo, _Vector>::GetIFP(const _Key& key) {
    const iterator it = Find(key);
    return (end() != it ? &it->second : nullptr);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
bool TAssociativeVector<_Key, _Value, _EqualTo, _Vector>::TryGet(const _Key& key, _Value *value) const {
    Assert(value);

    const const_iterator it = Find(key);
    if (end() == it)
        return false;

    *value = it->second;
    return true;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
const _Value& TAssociativeVector<_Key, _Value, _EqualTo, _Vector>::At(const _Key& key) const {
    const const_iterator it = Find(key);
    Assert(_vector.end() != it);
    return it->second;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
bool TAssociativeVector<_Key, _Value, _EqualTo, _Vector>::Erase(const _Key& key) {
    const const_iterator it = Find(key);
    if (end() == it)
        return false;
    Erase(it);
    return true;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
void TAssociativeVector<_Key, _Value, _EqualTo, _Vector>::Erase(const const_iterator& it) {
    Erase_DontPreserveOrder(_vector, it);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
void TAssociativeVector<_Key, _Value, _EqualTo, _Vector>::Remove_AssertExists(const _Key& key, const _Value& valueForDebug) {
    UNUSED(valueForDebug);
    const const_iterator it = Find(key);
    Assert(end() != it);
    Assert(valueForDebug == it->second);
    Erase(it);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
void TAssociativeVector<_Key, _Value, _EqualTo, _Vector>::Remove_AssertExists(const _Key& key) {
    const const_iterator it = Find(key);
    Assert(end() != it);
    Erase(it);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
FTextWriter& operator <<(FTextWriter& oss, const TAssociativeVector<_Key, _Value, _EqualTo, _Vector>& associativeVector) {
    if (associativeVector.empty()) {
        return oss << "{}";
    }
    else {
        auto it = associativeVector.begin();
        oss << "{(" << it->first << ", " << it->second << ')';
        ++it;
        for (const auto end = associativeVector.end(); it != end; ++it)
            oss << ",(" << it->first << ", " << it->second << ')';
        return oss << '}';
    }
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
FWTextWriter& operator <<(FWTextWriter& oss, const TAssociativeVector<_Key, _Value, _EqualTo, _Vector>& associativeVector) {
    if (associativeVector.empty()) {
        return oss << L"{}";
    }
    else {
        auto it = associativeVector.begin();
        oss << L"{(" << it->first << L", " << it->second << L')';
        ++it;
        for (const auto end = associativeVector.end(); it != end; ++it)
            oss << L",(" << it->first << L", " << it->second << L')';
        return oss << L'}';
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE