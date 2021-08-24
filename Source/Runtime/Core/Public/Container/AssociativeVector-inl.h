#pragma once

#include "Container/AssociativeVector.h"

#include <algorithm>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
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
    PPE::Reserve(_vector, capacity);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
TAssociativeVector<_Key, _Value, _EqualTo, _Vector>::TAssociativeVector(vector_type&& vector) NOEXCEPT
:   _vector(std::move(vector)) {}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
auto TAssociativeVector<_Key, _Value, _EqualTo, _Vector>::operator =(vector_type&& vector) NOEXCEPT -> TAssociativeVector& {
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
TAssociativeVector<_Key, _Value, _EqualTo, _Vector>::TAssociativeVector(TAssociativeVector&& rvalue) NOEXCEPT
:   _vector(std::move(rvalue._vector)) {}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
auto TAssociativeVector<_Key, _Value, _EqualTo, _Vector>::operator =(TAssociativeVector&& rvalue) NOEXCEPT -> TAssociativeVector& {
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
    PPE::Reserve(_vector, capacity);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
void TAssociativeVector<_Key, _Value, _EqualTo, _Vector>::clear() {
    PPE::Clear(_vector);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
void TAssociativeVector<_Key, _Value, _EqualTo, _Vector>::clear_ReleaseMemory() {
    PPE::Clear_ReleaseMemory(_vector);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
_Value& TAssociativeVector<_Key, _Value, _EqualTo, _Vector>::Add(_Key&& rkey) {
    Assert(end() == Find(rkey));
    auto it = PPE::Emplace_Back(_vector);
    it->first = std::move(rkey);
    return (it->second);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
_Value& TAssociativeVector<_Key, _Value, _EqualTo, _Vector>::Add(const _Key& key) {
    Assert(end() == Find(key));
    auto it = PPE::Emplace_Back(_vector);
    it->first = key;
    return (it->second);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
_Value& TAssociativeVector<_Key, _Value, _EqualTo, _Vector>::FindOrAdd(const _Key& key) {
    bool added;
    const auto it = FindOrAdd(key, &added);
    return it->second;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
auto TAssociativeVector<_Key, _Value, _EqualTo, _Vector>::Find(const _Key& key) -> iterator {
    return std::find_if(std::begin(_vector), std::end(_vector), [&key](const value_type& it) {
        return key_equal()(it.first, key);
    });
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
auto TAssociativeVector<_Key, _Value, _EqualTo, _Vector>::FindOrAdd(const _Key& key, bool* pAdded) -> iterator {
    iterator it = Find(key);

    if (end() == it) {
        if (pAdded) *pAdded = true;

        it = PPE::Emplace_Back(_vector);
        it->first = key;
    }
    else {
        if (pAdded) *pAdded = false;
    }

    return it;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
auto TAssociativeVector<_Key, _Value, _EqualTo, _Vector>::Find(const _Key& key) const -> const_iterator {
    return std::find_if(std::begin(_vector), std::end(_vector), [&key](const value_type& it) {
        return key_equal()(it.first, key);
    });
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
auto TAssociativeVector<_Key, _Value, _EqualTo, _Vector>::FindAfter(const _Key& key, iterator after) -> iterator {
    const iterator end{ std::end(_vector) };
    const iterator first{ (after == end) ? end : ++after };
    return std::find_if(first, end, [&key](const value_type& it) {
        return key_equal()(it.first, key);
    });
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
auto TAssociativeVector<_Key, _Value, _EqualTo, _Vector>::FindAfter(const _Key& key, const_iterator after) const -> const_iterator {
    const const_iterator end{ std::end(_vector) };
    const const_iterator first{ (after == end) ? end : ++after };
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
template <typename _KeyLike>
auto TAssociativeVector<_Key, _Value, _EqualTo, _Vector>::FindLike(const _KeyLike& keyLike) -> iterator {
    return std::find_if(std::begin(_vector), std::end(_vector), [&keyLike](const value_type& it) {
        return key_equal()(it.first, keyLike);
    });
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
template <typename _KeyLike>
auto TAssociativeVector<_Key, _Value, _EqualTo, _Vector>::FindLike(const _KeyLike& keyLike) const -> const_iterator {
    return std::find_if(std::begin(_vector), std::end(_vector), [&keyLike](const value_type& it) {
        return key_equal()(it.first, keyLike);
    });
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
template <typename _KeyLike>
bool TAssociativeVector<_Key, _Value, _EqualTo, _Vector>::FindLike(const _KeyLike& keyLike, _Value *pvalue) const {
    const const_iterator it = FindLike(keyLike);
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
        PPE::Emplace_Back(_vector, std::move(key), mapped_type(std::forward<_Args>(args)...));
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
bool TAssociativeVector<_Key, _Value, _EqualTo, _Vector>::Emplace_Overwrite(_Key&& key, _Args&&... args) {
    auto it = Find(key);
    if (end() == it) {
        PPE::Emplace_Back(_vector, std::move(key), mapped_type(std::forward<_Args>(args)...));
        return false;
    }
    else {
        it->second = mapped_type(std::forward<_Args>(args)...);
        return true;
    }
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
template <class... _Args>
void TAssociativeVector<_Key, _Value, _EqualTo, _Vector>::Emplace_AssertUnique(_Key&& key, _Args&&... args) {
    Assert(end() == Find(key));
    PPE::Emplace_Back(_vector, std::move(key), mapped_type(std::forward<_Args>(args)...));
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
template <class... _Args>
bool TAssociativeVector<_Key, _Value, _EqualTo, _Vector>::Emplace_ReturnIfExists(const _Key& key, _Args&&... args) {
    return Emplace_ReturnIfExists(_Key(key), std::forward<_Args>(args)...);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
template <class... _Args>
void TAssociativeVector<_Key, _Value, _EqualTo, _Vector>::Emplace_KeepOldIFN(const _Key& key, _Args&&... args) {
    Emplace_ReturnIfExists(_Key(key), std::forward<_Args>(args)...);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
template <class... _Args>
bool TAssociativeVector<_Key, _Value, _EqualTo, _Vector>::Emplace_Overwrite(const _Key& key, _Args&&... args) {
    return Emplace_Overwrite(_Key(key), std::forward<_Args>(args)...);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
template <class... _Args>
void TAssociativeVector<_Key, _Value, _EqualTo, _Vector>::Emplace_AssertUnique(const _Key& key, _Args&&... args) {
    return Emplace_AssertUnique(_Key(key), std::forward<_Args>(args)...);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
bool TAssociativeVector<_Key, _Value, _EqualTo, _Vector>::Insert_ReturnIfExists(_Key&& key, _Value&& rvalue) {
    if (end() == Find(key)) {
        PPE::Emplace_Back(_vector, std::move(key), std::move(rvalue));
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
bool TAssociativeVector<_Key, _Value, _EqualTo, _Vector>::Insert_Overwrite(_Key&& key, _Value&& rvalue) {
    auto it = Find(key);
    if (end() == it) {
        PPE::Emplace_Back(_vector, std::move(key), std::move(rvalue));
        return false;
    }
    else {
        it->second = std::move(rvalue);
        return true;
    }
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
void TAssociativeVector<_Key, _Value, _EqualTo, _Vector>::Insert_AssertUnique(_Key&& key, _Value&& rvalue) {
    Assert(end() == Find(key));
    PPE::Emplace_Back(_vector, std::move(key), std::move(rvalue));
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
bool TAssociativeVector<_Key, _Value, _EqualTo, _Vector>::Insert_ReturnIfExists(const _Key& key, const _Value& value) {
    if (end() == Find(key)) {
        PPE::Emplace_Back(_vector, key, value);
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
bool TAssociativeVector<_Key, _Value, _EqualTo, _Vector>::Insert_Overwrite(const _Key& key, const _Value& value) {
    auto it = Find(key);
    if (end() == it) {
        PPE::Emplace_Back(_vector, key, value);
        return false;
    }
    else {
        it->second = value;
        return true;
    }
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
void TAssociativeVector<_Key, _Value, _EqualTo, _Vector>::Insert_AssertUnique(const _Key& key, const _Value& value) {
    Assert(end() == Find(key));
    PPE::Emplace_Back(_vector, key, value);
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
    AssertRelease(std::end(_vector) != it);
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
