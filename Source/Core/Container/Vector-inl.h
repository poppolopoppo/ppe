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
    const typename Vector<T, _Allocator>::const_iterator begin = v.begin();
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
size_t hash_value(const Vector<T, _Allocator>& vector) {
    return hash_value_seq(vector.begin(), vector.end());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <size_t _SizeInBytes>
struct VectorInSituStorage {
    typedef typename std::aligned_storage< _SizeInBytes, 16 >::type
        storage_type;

    VectorInSituStorage() {}

    VectorInSituStorage(const VectorInSituStorage& ) = delete;
    VectorInSituStorage& operator =(const VectorInSituStorage& ) = delete;

    void *get() { return &_storage; }
    const void *get() const { return &_storage; }

    storage_type _storage;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _SizeInBytes, typename _Allocator>
class VectorInSituAllocator : public _Allocator {
public:
    template <typename, size_t, typename>
    friend class VectorInSituAllocator;

    typedef _Allocator fallback_type;

    typedef T *pointer;
    typedef std::size_t size_type;

    typedef VectorInSituStorage<_SizeInBytes> storage_type;
        
    template<typename U>
    struct rebind
    {
        typedef VectorInSituAllocator<
            U, _SizeInBytes, 
            typename _Allocator::template rebind<U>::other
        >   other;
    };

    VectorInSituAllocator(storage_type& insitu) throw() : _pinsitu(&insitu) {}

    template <typename U, typename A>
    VectorInSituAllocator(const VectorInSituAllocator<U, _SizeInBytes, A>& other)
        : _pinsitu(other._pinsitu) {}

    VectorInSituAllocator& operator =(const VectorInSituAllocator& ) { return *this; }

    pointer allocate(size_type n);
    pointer allocate(size_type n, const void* /*hint*/) { return allocate(n); }
    void deallocate(pointer p, size_type n);

    template <typename U, typename A>
    friend bool operator ==(const VectorInSituAllocator& lhs, const VectorInSituAllocator<U, _SizeInBytes, A>& rhs) {
        return lhs._pinsitu == rhs._pinsitu;
    }

    template <typename U, typename A>
    friend bool operator !=(const VectorInSituAllocator& lhs, const VectorInSituAllocator<U, _SizeInBytes, A>& rhs) {
        return !operator ==(lhs, rhs);
    }

private:
    storage_type *_pinsitu;
};
//----------------------------------------------------------------------------
template <typename T, size_t _SizeInBytes, typename _Allocator>
auto VectorInSituAllocator<T, _SizeInBytes, _Allocator>::allocate(size_type n) -> pointer {
    if (n * sizeof(T) == _SizeInBytes) {
        Assert(_pinsitu);
        return reinterpret_cast<T *>(_pinsitu->get());
    }
    else {
        return fallback_type::allocate(n);
    }
}
//----------------------------------------------------------------------------
template <typename T, size_t _SizeInBytes, typename _Allocator>
void VectorInSituAllocator<T, _SizeInBytes, _Allocator>::deallocate(pointer p, size_type n) {
    if (p != _pinsitu->get())
        fallback_type::deallocate(p, n);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _InSituCount, typename _Allocator >
auto VectorInSitu<T, _InSituCount, _Allocator>::operator =(VectorInSitu&& rvalue) -> VectorInSitu& {
    const size_t count = rvalue.size();
    if (count && &rvalue.front() == rvalue._inSituData.get()) {
        vector_type::resize(count);
        T *psrc = &rvalue.front();
        T *const pend = &rvalue.back();
        for (T *pdst = &vector_type::front(); psrc != pend; ++pdst, ++psrc)
            *pdst = std::move(*psrc);
        rvalue.clear();
    }
    else {
        vector_type::operator =(std::move(rvalue));
        rvalue.reserve(_InSituCount); // to keep its insitu data
    }
    Assert(rvalue.capacity() == _InSituCount);
    return *this;
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
