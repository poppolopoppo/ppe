#pragma once

#include "Core/Core.h"

#include <iosfwd>
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
    typedef typename std::add_reference<T>::type reference;
    typedef typename std::add_reference<const T>::type const_reference;

    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

    typedef pointer iterator;
    typedef std::random_access_iterator_tag iterator_category;

    MemoryView();
    MemoryView(pointer storage, size_type size);
    ~MemoryView();

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

    size_type size() const { return _size; }
    bool empty() const { return 0 == _size; }

    iterator begin() const { return _storage; }
    iterator end() const { return _storage + _size; }

    reference at(size_type index) const;
    reference operator [](size_type index) const { return at(index); }

    reference front() const { return at(0); }
    reference back() const { return at(_size - 1); }

    MemoryView<T> SubRange(size_t offset, size_t count) const;
    MemoryView< typename std::add_const<T>::type > SubRangeConst(size_t offset, size_t count) const;

    void Swap(MemoryView& other);

    template <typename U>
    MemoryView<U> Cast() const;

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
    Assert(offset + count <= _size);
    return MemoryView(_storage + offset, count);
}
//----------------------------------------------------------------------------
template <typename T>
MemoryView< typename std::add_const<T>::type > MemoryView<T>::SubRangeConst(size_t offset, size_t count) const {
    Assert(offset + count <= _size);
    return MemoryView< typename std::add_const<T>::type >(_storage + offset, count);
}
//----------------------------------------------------------------------------
template <typename T>
void MemoryView<T>::Swap(MemoryView& other) {
    std::swap(other._storage, _storage);
    std::swap(other._size, _size);
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
template <typename T>
void swap(MemoryView<T>& lhs, MemoryView<T>& rhs) {
    lhs.Swap(rhs);
}
//----------------------------------------------------------------------------
template <typename T>
bool operator ==(const MemoryView<T>& lhs, const MemoryView<T>& rhs) {
    return lhs.Pointer() == rhs.Pointer() && lhs.SizeInBytes() == rhs.SizeInBytes();
}
//----------------------------------------------------------------------------
template <typename T>
bool operator !=(const MemoryView<T>& lhs, const MemoryView<T>& rhs) {
    return !operator ==(lhs, rhs);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
MemoryView<T> MakeView(T(&staticArray)[_Dim]) {
    return MemoryView<T>(&staticArray[0], _Dim);
}
//----------------------------------------------------------------------------
template <typename _Category, typename _Ty>
MemoryView<_Ty> MakeView(
    std::iterator<_Category, _Ty, ptrdiff_t> begin,
    std::iterator<_Category, _Ty, ptrdiff_t> end) {
    return MemoryView<_Ty>(&*begin, std::distance(begin, end));
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
template <typename T>
MemoryView< T > MakeView(T* begin, T* end) {
    Assert(end >= begin);
    return MemoryView< T >(begin, std::distance(begin, end));
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
