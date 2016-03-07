#pragma once

#include "Core/Core.h"

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

    template <size_t _Dim> // enables type promotion between T[] and MemoryView<T>
    MemoryView(value_type (&staticArray)[_Dim]) : MemoryView(staticArray, _Dim) {}

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

    reverse_iterator rbegin() const { return reverse_iterator(begin()); }
    reverse_iterator rend() const { return reverse_iterator(end()); }

    reference at(size_type index) const;
    reference operator [](size_type index) const { return at(index); }

    reference front() const { return at(0); }
    reference back() const { return at(_size - 1); }

    MemoryView<T> SubRange(size_t offset, size_t count) const;
    MemoryView< typename std::add_const<T>::type > SubRangeConst(size_t offset, size_t count) const;

    MemoryView<T> CutStartingAt(size_t offset) const { return SubRange(offset, _size - offset); }
    MemoryView< typename std::add_const<T>::type > CutStartingAtConst(size_t offset) const { return SubRangeConst(offset, _size - offset); }

    MemoryView<T> CutBefore(size_t offset) const { return SubRange(0, offset); }
    MemoryView< typename std::add_const<T>::type > CutBeforeConst(size_t offset) const { return SubRangeConst(0, offset); }

    MemoryView<T> ShiftBack() const { Assert(_size > 0); return MemoryView<T>(_storage, _size - 1); }
    MemoryView<T> ShiftFront() const { Assert(_size > 0); return MemoryView<T>(_storage + 1, _size - 1); }

    MemoryView<T> GrowBack() const { Assert(_size > 0); return MemoryView<T>(_storage, _size + 1); }
    MemoryView<T> GrowFront() const { Assert(_size > 0); return MemoryView<T>(_storage - 1, _size + 1); }

    template <typename U>
    bool IsSubRangeOf(const MemoryView<U>& parent) const {
        return ((void*)parent.data() <= (void*)_storage &&
                (void*)(parent.data()+parent.size()) >= (void*)(_storage+_size));
    }

    template <typename _Pred>
    iterator find_if(const _Pred& pred) const { return std::find_if(begin(), end(), pred); }

    bool AliasesToContainer(iterator it) const { return (_storage <= &*it && _storage + _size > &*it); }

    template <typename U>
    MemoryView<U> Cast() const;

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
    typedef MemoryView< typename traits_type::value_type > view_type;
    STATIC_ASSERT(std::is_same<typename traits_type::iterator_category, std::random_access_iterator_tag>::value);
    return view_type(std::addressof(*first), std::distance(first, last));
}
//----------------------------------------------------------------------------
template <typename _VectorLike>
MemoryView<typename _VectorLike::value_type> MakeView(_VectorLike& container) {
    if (container.begin() != container.end())
        return MemoryView<typename _VectorLike::value_type>(&*container.begin(), container.end() - container.begin());
    else
        return MemoryView<typename _VectorLike::value_type>();
}
//----------------------------------------------------------------------------
template <typename _VectorLike>
MemoryView<const typename _VectorLike::value_type> MakeView(const _VectorLike& container) {
    if (container.begin() != container.end())
        return MemoryView<const typename _VectorLike::value_type>(&*container.begin(), container.end() - container.begin());
    else
        return MemoryView<const typename _VectorLike::value_type>();
}
//----------------------------------------------------------------------------
template <typename _VectorLike>
MemoryView<const typename _VectorLike::value_type> MakeConstView(const _VectorLike& container) {
    if (container.begin() != container.end())
        return MemoryView<const typename _VectorLike::value_type>(&*container.begin(), container.end() - container.begin());
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
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
void Copy(const MemoryView<T>& dst, const MemoryView<const T>& src) {
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
