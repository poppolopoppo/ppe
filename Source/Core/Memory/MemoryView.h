#pragma once

#include "Core/Core.h"

#include <algorithm>
#include <initializer_list>
#include <iosfwd>
#include <iterator>
#include <type_traits>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class MemoryView {
public:
    template <typename U>
    friend class MemoryView;

    typedef T value_type;
    typedef typename std::add_pointer<T>::type pointer;
    typedef typename std::add_pointer<const T>::type const_pointer;
    typedef typename std::add_lvalue_reference<T>::type reference;
    typedef typename std::add_lvalue_reference<const T>::type const_reference;

    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

    typedef CheckedArrayIterator<value_type> iterator;
    typedef std::random_access_iterator_tag iterator_category;
    typedef std::reverse_iterator<iterator> reverse_iterator;

    MemoryView();
    MemoryView(pointer storage, size_type size);
    ~MemoryView();

    // enables type promotion between {T(),T(),T()} and MemoryView<T>
    MemoryView(std::initializer_list<value_type> list)
        : MemoryView(&*list.begin(), std::distance(list.begin(), list.end())) {}

    // enables type promotion between T[] and MemoryView<T>
    template <size_t _Dim>
    MemoryView(value_type (&staticArray)[_Dim])
        : MemoryView(staticArray, _Dim) {}

    MemoryView(const iterator& first, const iterator& last)
        : MemoryView(std::addressof(*first), std::distance(first, last)) {}

    MemoryView(MemoryView&& rvalue);
    MemoryView& operator =(MemoryView&& rvalue);

    MemoryView(const MemoryView& other);
    MemoryView& operator =(const MemoryView& other);

    template <typename U>
    MemoryView(const MemoryView<U>& other);
    template <typename U>
    MemoryView& operator =(const MemoryView<U>& other);

    pointer Pointer() const { return _storage; }
    size_t SizeInBytes() const { return _size * sizeof(T); }

    pointer data() const { return _storage; }
    size_type size() const { return _size; }
    bool empty() const { return 0 == _size; }

    iterator begin() const { return MakeCheckedIterator(_storage, _size, 0); }
    iterator end() const { return MakeCheckedIterator(_storage, _size, _size); }

    iterator cbegin() const { return begin(); }
    iterator cend() const { return end(); }

    reverse_iterator rbegin() const { return reverse_iterator(end()); }
    reverse_iterator rend() const { return reverse_iterator(begin()); }

    reference at(size_type index) const;
    reference operator [](size_type index) const { return at(index); }

    reference front() const { return at(0); }
    reference back() const { return at(_size - 1); }

    void CopyTo(const MemoryView<typename std::remove_const<T>::type>& dst) const;

    template <size_t _Dim>
    void CopyTo(typename std::remove_const<T>::type (&dst)[_Dim]) const {
        Assert(_Dim >= _size);
        CopyTo(MakeView(dst).CutBefore(_size));
    }

    MemoryView<T> SubRange(size_t offset, size_t count) const;
    MemoryView< typename std::add_const<T>::type > SubRangeConst(size_t offset, size_t count) const;

    MemoryView<T> CutStartingAt(size_t offset) const { return SubRange(offset, _size - offset); }
    MemoryView< typename std::add_const<T>::type > CutStartingAtConst(size_t offset) const { return SubRangeConst(offset, _size - offset); }

    MemoryView<T> CutStartingAt(const iterator& it) const {
        Assert(AliasesToContainer(it));
        return (end() != it
            ? MemoryView(std::addressof(*it), std::distance(it, end()))
            : MemoryView(_storage+_size, size_type(0)) );
    }

    MemoryView<T> CutStartingAt(const reverse_iterator& it) const {
        Assert(AliasesToContainer(it));
        return (rend() != it
            ? MemoryView(std::addressof(*it), _storage + _size - std::addressof(*it))
            : MemoryView(_storage, size_type(0)) );
    }

    MemoryView<T> CutBefore(size_t offset) const { return SubRange(0, offset); }
    MemoryView< typename std::add_const<T>::type > CutBeforeConst(size_t offset) const { return SubRangeConst(0, offset); }

    MemoryView<T> CutBefore(const iterator& it) const {
        Assert(AliasesToContainer(it));
        return MemoryView<T>(_storage, std::distance(begin(), it));
    }

    MemoryView<T> CutBefore(const reverse_iterator& it) const {
        Assert(AliasesToContainer(it));
        return MemoryView<T>(_storage, std::addressof(*it) - _storage);
    }

    MemoryView<T> ShiftBack() const { Assert(_size > 0); return MemoryView<T>(_storage, _size - 1); }
    MemoryView<T> ShiftFront() const { Assert(_size > 0); return MemoryView<T>(_storage + 1, _size - 1); }

    MemoryView<T> GrowBack() const { return MemoryView<T>(_storage, _size + 1); }
    MemoryView<T> GrowFront() const { return MemoryView<T>(_storage - 1, _size + 1); }

    template <typename U>
    bool IsSubRangeOf(const MemoryView<U>& parent) const {
        return ((void*)parent.data() <= (void*)_storage &&
                (void*)(parent.data()+parent.size()) >= (void*)(_storage+_size));
    }

    template <typename _Pred>
    iterator FindIf(const _Pred& pred) const { return std::find_if(begin(), end(), pred); }
    template <typename _Pred>
    size_type FindFirst(const _Pred& pred) const { return std::distance(begin(), FindIf(pred)); }

    template <typename _Pred>
    reverse_iterator FindIfR(const _Pred& pred) const { return std::find_if(rbegin(), rend(), pred); }
    template <typename _Pred>
    size_type FindLast(const _Pred& pred) const { return std::distance(rbegin(), FindIfR(pred)); }

    template <typename _Pred>
    iterator FindIfNot(const _Pred& pred) const { return std::find_if_not(begin(), end(), pred); }
    template <typename _Pred>
    size_type FindFirstNot(const _Pred& pred) const { return std::distance(begin(), FindIfNot(pred)); }

    template <typename _Pred>
    reverse_iterator FindIfNotR(const _Pred& pred) const { return std::find_if_not(rbegin(), rend(), pred); }
    template <typename _Pred>
    size_type FindLastNot(const _Pred& pred) const { return std::distance(rbegin(), FindIfNotR(pred)); }

    iterator FindSubRange(const MemoryView<T>& subrange) const;

    template <typename _Pred>
    MemoryView SplitIf(const _Pred& pred) const { return MemoryView(_storage, FindFirst(pred)); }
    template <typename _Pred>
    MemoryView SplitIfNot(const _Pred& pred) const { return MemoryView(_storage, FindFirstNot(pred)); }

    bool AliasesToContainer(const iterator& it) const { return (begin() <= it && it <= end()); }
    bool AliasesToContainer(const reverse_iterator& it) const { return (rbegin() <= it && it <= rend()); }

    template <typename U>
    MemoryView<U> Cast() const;

    MemoryView<typename std::add_const<value_type>::type> AddConst() const {
        return MemoryView<typename std::add_const<value_type>::type>(_storage, _size);
    }

    MemoryView<typename std::remove_const<value_type>::type> RemoveConst() const {
        typedef MemoryView<typename std::remove_const<value_type>::type> nonconst_type;
        return nonconst_type(const_cast<typename nonconst_type::pointer>(_storage), _size);
    }

    friend void swap(MemoryView& lhs, MemoryView& rhs) {
        std::swap(lhs._storage, rhs._storage);
        std::swap(lhs._size, rhs._size);
    }

    friend bool operator ==(const MemoryView& lhs, const MemoryView& rhs) {
        return  lhs._storage == rhs._storage &&
                lhs._size == rhs._size;
    }

    friend bool operator !=(const MemoryView& lhs, const MemoryView& rhs) {
        return false == operator ==(lhs, rhs);
    }

protected:
    pointer _storage;
    size_type _size;
};
//----------------------------------------------------------------------------
template <typename T>
MemoryView<T>::MemoryView()
: _storage(nullptr), _size(0) {}
//----------------------------------------------------------------------------
template <typename T>
MemoryView<T>::MemoryView(pointer storage, size_type size)
: _storage(storage), _size(size) {
    Assert(storage || 0 == size);
}
//----------------------------------------------------------------------------
template <typename T>
MemoryView<T>::~MemoryView() {}
//----------------------------------------------------------------------------
template <typename T>
MemoryView<T>::MemoryView(MemoryView&& rvalue)
:   _storage(std::move(rvalue._storage))
,   _size(std::move(rvalue._size)) {
    rvalue._storage = nullptr;
    rvalue._size = 0;
}
//----------------------------------------------------------------------------
template <typename T>
MemoryView<T>& MemoryView<T>::operator =(MemoryView&& rvalue) {
    _storage = std::move(rvalue._storage);
    _size = std::move(rvalue._size);
    rvalue._storage = nullptr;
    rvalue._size = 0;
    return (*this);
}
//----------------------------------------------------------------------------
template <typename T>
MemoryView<T>::MemoryView(const MemoryView& other)
:   _storage(other._storage), _size(other._size) {}
//----------------------------------------------------------------------------
template <typename T>
MemoryView<T>& MemoryView<T>::operator =(const MemoryView& other) {
    _storage = other._storage;
    _size = other._size;
    return (*this);
}
//----------------------------------------------------------------------------
template <typename T>
template <typename U>
MemoryView<T>::MemoryView(const MemoryView<U>& other)
:   _storage(other._storage), _size(other._size) {}
//----------------------------------------------------------------------------
template <typename T>
template <typename U>
MemoryView<T>& MemoryView<T>::operator =(const MemoryView<U>& other) {
    _storage = other._storage;
    _size = other._size;
    return (*this);
}
//----------------------------------------------------------------------------
template <typename T>
auto MemoryView<T>::at(size_type index) const -> reference {
    Assert(index < _size);
    return _storage[index];
}
//----------------------------------------------------------------------------
template <typename T>
auto MemoryView<T>::FindSubRange(const MemoryView<T>& subrange) const -> iterator {
    Assert(!subrange.empty());
    const auto last = end();
    for (auto it = begin(); it != last; ++it) {
        if (std::equal(begin(), end(), subrange.begin(), subrange.end()))
            return it;
    }
    return last;
}
//----------------------------------------------------------------------------
template <typename T>
void MemoryView<T>::CopyTo(const MemoryView<typename std::remove_const<T>::type>& dst) const {
    Assert(dst.size() == size());
    std::copy(begin(), end(), dst.begin());
}
//----------------------------------------------------------------------------
template <typename T>
MemoryView<T> MemoryView<T>::SubRange(size_t offset, size_t count) const {
    Assert(offset <= _size);
    Assert(offset + count <= _size);
    return MemoryView(_storage + offset, count);
}
//----------------------------------------------------------------------------
template <typename T>
MemoryView< typename std::add_const<T>::type > MemoryView<T>::SubRangeConst(size_t offset, size_t count) const {
    Assert(offset <= _size);
    Assert(offset + count <= _size);
    return MemoryView< typename std::add_const<T>::type >(_storage + offset, count);
}
//----------------------------------------------------------------------------
template <typename T>
template <typename U>
MemoryView<U> MemoryView<T>::Cast() const {
    STATIC_ASSERT(  (0 == (sizeof(T) % sizeof(U)) ) ||
                    (0 == (sizeof(U) % sizeof(T)) ) );

    return MemoryView<U>(reinterpret_cast<U *>(_storage), (_size * sizeof(T)) / sizeof(U));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
MemoryView<T> MakeView(T(&staticArray)[_Dim]) {
    return MemoryView<T>(&staticArray[0], _Dim);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
MemoryView<typename std::add_const<T>::type > MakeConstView(T(&staticArray)[_Dim]) {
    return MemoryView<typename std::add_const<T>::type >(&staticArray[0], _Dim);
}
//----------------------------------------------------------------------------
template <typename _It>
typename std::enable_if<
    Meta::is_iterator<_It>::value,
    MemoryView< typename std::iterator_traits<_It>::value_type >
>::type MakeView(_It first, _It last) {
    typedef std::iterator_traits<_It> traits_type;
    STATIC_ASSERT(std::is_same<typename traits_type::iterator_category, std::random_access_iterator_tag>::value);
    return MemoryView< typename traits_type::value_type >(std::addressof(*first), std::distance(first, last));
}
//----------------------------------------------------------------------------
template <typename _VectorLike>
MemoryView<typename _VectorLike::value_type> MakeView(_VectorLike& container) {
    if (container.begin() != container.end())
        return MemoryView<typename _VectorLike::value_type>(&*std::begin(container), std::distance(std::begin(container), std::end(container)) );
    else
        return MemoryView<typename _VectorLike::value_type>();
}
//----------------------------------------------------------------------------
template <typename _VectorLike>
MemoryView<const typename _VectorLike::value_type> MakeView(const _VectorLike& container) {
    if (container.begin() != container.end())
        return MemoryView<const typename _VectorLike::value_type>(&*std::begin(container), std::distance(std::begin(container), std::end(container)) );
    else
        return MemoryView<const typename _VectorLike::value_type>();
}
//----------------------------------------------------------------------------
template <typename _VectorLike>
MemoryView<const typename _VectorLike::value_type> MakeConstView(const _VectorLike& container) {
    if (container.begin() != container.end())
        return MemoryView<const typename _VectorLike::value_type>(&*std::begin(container), std::distance(std::begin(container), std::end(container)) );
    else
        return MemoryView<const typename _VectorLike::value_type>();
}
//----------------------------------------------------------------------------
template <typename T>
MemoryView< T > MakeView(T* pbegin, T* pend) {
    Assert(pend >= pbegin);
    return MemoryView< T >(pbegin, std::distance(pbegin, pend));
}
//----------------------------------------------------------------------------
template <typename T>
MemoryView<typename std::add_const<T>::type > MakeConstView(T* pbegin, T* pend) {
    Assert(pend >= pbegin);
    return MemoryView<typename std::add_const<T>::type >(pbegin, std::distance(pbegin, pend));
}
//----------------------------------------------------------------------------
template <typename T>
MemoryView<u8> MakeRawView(T& assumePod) {
    return MemoryView<u8>(reinterpret_cast<u8*>(&assumePod), sizeof(T));
}
//----------------------------------------------------------------------------
template <typename T>
MemoryView<const u8> MakeRawView(const T& assumePod) {
    return MemoryView<const u8>(reinterpret_cast<const u8*>(&assumePod), sizeof(T));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename U, typename V>
void Copy(const MemoryView<U>& dst, const MemoryView<V>& src) {
    Assert(dst.size() == src.size());
    std::copy(src.begin(), src.end(), dst.begin());
}
//----------------------------------------------------------------------------
template <typename T>
void Move(const MemoryView<T>& dst, const MemoryView<T>& src) {
    Assert(dst.size() == src.size());
    std::move(src.begin(), src.end(), dst.begin());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits, typename T >
std::basic_ostream<_Char, _Traits>& operator <<(
    std::basic_ostream<_Char, _Traits>& oss,
    const MemoryView<T>& view) {
    for (const auto& it : view)
        oss << it;
    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
