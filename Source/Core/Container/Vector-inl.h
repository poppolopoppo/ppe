#pragma once

#include "Core/Container/Vector.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
typename Vector<T, _Allocator>::const_iterator FindFirstOf(const Vector<T, _Allocator>& v, const T& elt) {
    return std::find(v.begin(), v.end(), elt);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
bool Contains(const Vector<T, _Allocator>& v, const T& elt) {
    return v.end() != FindFirstOf(v, elt);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
bool FindElementIndexIFP(size_t *pIndex, Vector<T, _Allocator>& v, const T& elt) {
    Assert(pIndex);
    const size_t size = v.size();
    for (size_t i = 0; i < size; ++i)
        if (elt == v[i]) {
            *pIndex = i;
            return true;
        }
    return false;
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Add_AssertUnique(Vector<T, _Allocator>& v, const T& elt) {
    Assert(!Contains(v, elt));
    v.emplace_back(elt);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Add_AssertUnique(Vector<T, _Allocator>& v, T&& elt) {
    Assert(!Contains(v, elt));
    v.emplace_back(std::move(elt));
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Remove_AssertExists(Vector<T, _Allocator>& v, const T& elt) {
    auto it = std::find(v.begin(), v.end(), elt);
    Assert(it != v.end());
    v.erase(it);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Remove_DontPreserveOrder(Vector<T, _Allocator>& v, const T& elt) {
    auto it = std::find(v.begin(), v.end(), elt);
    Assert(it != v.end());
    if (v.size() > 1) swap(*it, v.back());
    v.pop_back();
}
//----------------------------------------------------------------------------
// Fast erase : swap last elem with elem to erase and pop_back() the vector
template <typename T, typename _Allocator>
void Erase_DontPreserveOrder(
    Vector<T, _Allocator>& v,
    const typename Vector<T, _Allocator>::const_iterator& it) {
    typedef Vector<T, _Allocator> vector_type;
    const size_t k = checked_cast<size_t>(v.size());
    Assert(k);
    const size_t i = checked_cast<size_t>(it - v.begin());
    Assert(i < k);
    if (i != k - 1)
        std::swap(v[i], v[k - 1]);
    v.pop_back();
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
size_t hash_value(const Vector<T, _Allocator>& vector) {
    return hash_value_seq(vector.begin(), vector.end());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <
    typename T,
    typename _Allocator,
    typename _Char,
    typename _Traits
>
std::basic_ostream<_Char, _Traits>& operator <<(std::basic_ostream<_Char, _Traits>& oss, const std::vector<T, _Allocator>& vector) {
    oss << "[ ";
    for (const auto& it : vector)
        oss << it << ", ";
    return oss << ']';
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
