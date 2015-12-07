#pragma once

#include "Core/Container/Vector.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Append(Vector<T, _Allocator>& v, const MemoryView<const T>& elts) {
    v.insert(v.end(), elts.begin(), elts.end());
}
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
bool Remove_ReturnIfExists(Vector<T, _Allocator>& v, const T& elt) {
    auto it = std::find(v.begin(), v.end(), elt);
    if (it == v.end())
        return false;

    v.erase(it);
    return true;
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Remove_DontPreserveOrder(Vector<T, _Allocator>& v, const T& elt) {
    using std::swap;
    auto it = std::find(v.begin(), v.end(), elt);
    Assert(it != v.end());
    if (v.size() > 1)
        swap(*it, v.back());
    v.pop_back();
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
bool Remove_ReturnIfExists_DontPreserveOrder(Vector<T, _Allocator>& v, const T& elt) {
    using std::swap;
    auto it = std::find(v.begin(), v.end(), elt);
    if (v.end() == it)
        return false;

    if (v.size() > 1)
        swap(*it, v.back());
    v.pop_back();

    return true;
}
//----------------------------------------------------------------------------
// Fast erase : swap last elem with elem to erase and pop_back() the vector
template <typename T, typename _Allocator>
void Erase_DontPreserveOrder(Vector<T, _Allocator>& v, size_t index) {
    typedef Vector<T, _Allocator> vector_type;
    using std::swap;
    const size_t k = checked_cast<size_t>(v.size());
    Assert(index < k);
    if (k > 1 && index != k - 1)
        swap(v[index], v[k - 1]);
    v.pop_back();
}
//----------------------------------------------------------------------------
// Fast erase : swap last elem with elem to erase and pop_back() the vector
template <typename T, typename _Allocator>
void Erase_DontPreserveOrder(Vector<T, _Allocator>& v, const typename Vector<T, _Allocator>::const_iterator& it) {
    typedef Vector<T, _Allocator> vector_type;
    using std::swap;
    const size_t k = checked_cast<size_t>(v.size());
    Assert(k);
    const typename vector_type::const_iterator begin = v.begin();
    const size_t i = std::distance(begin, it);
    Assert(i < k);
    if (k > 1 && i != k - 1)
        swap(v[i], v[k - 1]);
    v.pop_back();
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Clear_ReleaseMemory(Vector<T, _Allocator>& v) {
    v.clear();
    v.shrink_to_fit();
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
hash_t hash_value(const Vector<T, _Allocator>& vector) {
    hash_t h(CORE_HASH_VALUE_SEED);
    for (const T& it : vector)
        hash_combine(h, hash_value(it));
    return h;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _InSituCount, typename _Allocator >
auto VectorInSitu<T, _InSituCount, _Allocator>::operator =(const VectorInSitu& other) -> VectorInSitu& {
    vector_type::clear();
    vector_type::reserve(_InSituCount);
    vector_type::insert(vector_type::end(), other.begin(), other.end());
    Assert(UseInSitu() || false == other.UseInSitu());
    return *this;
}
//----------------------------------------------------------------------------
template <typename T, size_t _InSituCount, typename _Allocator >
auto VectorInSitu<T, _InSituCount, _Allocator>::operator =(VectorInSitu&& rvalue) -> VectorInSitu& {
    const size_t count = rvalue.size();
    if (count && rvalue.UseInSitu() ) {
        vector_type::resize(count);
        std::move(rvalue.begin(), rvalue.end(), vector_type::begin());
        rvalue.clear();
    }
    else {
        vector_type::operator =(std::move(rvalue));
    }
    rvalue.reserve(_InSituCount); // to keep its insitu data
    Assert(rvalue.capacity() >= _InSituCount);
    return *this;
}
//----------------------------------------------------------------------------
template <typename T, size_t _InSituCount, typename _Allocator >
void swap(VectorInSitu<T, _InSituCount, _Allocator>& lhs, VectorInSitu<T, _InSituCount, _Allocator>& rhs) {
    typedef typename VectorInSitu<T, _InSituCount, _Allocator>::storage_type    storage_type;
    typedef typename VectorInSitu<T, _InSituCount, _Allocator>::vector_type     vector_type;

    const bool lhsInSitu = lhs.UseInSitu();
    const bool rhsInSitu = rhs.UseInSitu();

    if (false == (lhsInSitu || rhsInSitu)) {
        Assert(static_cast<storage_type&>(lhs).InSituEmpty());
        Assert(static_cast<storage_type&>(rhs).InSituEmpty());

        std::swap(  static_cast<vector_type&>(lhs),
                    static_cast<vector_type&>(rhs) );
    }
    else {
        Assert(lhsInSitu || rhsInSitu);

        VectorInSitu<T, _InSituCount, _Allocator>* v0, *v1;
        if (lhsInSitu) { v0 = &lhs; v1 = &rhs; }
        else { v0 = &rhs; v1 = &lhs; }

        VectorInSitu<T, _InSituCount, _Allocator> tmp(std::move(*v0));
        *v0 = std::move(*v1);
        *v1 = std::move(tmp);

        if (lhsInSitu) {
            Assert(rhs.UseInSitu());
            rhs.reserve(_InSituCount);
        }
        else {
            Assert(static_cast<storage_type&>(rhs).InSituEmpty());
        }

        if (rhsInSitu) {
            Assert(lhs.UseInSitu());
            lhs.reserve(_InSituCount);
        }
        else {
            Assert(static_cast<storage_type&>(lhs).InSituEmpty());
        }
    }
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
