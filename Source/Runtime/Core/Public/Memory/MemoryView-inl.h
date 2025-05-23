#pragma once

#include "Memory/MemoryView.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR TMemoryView<T>::TMemoryView(pointer storage, size_type size) NOEXCEPT
:   _storage(storage), _size(size) {
    Assert(storage || 0 == size);
}
//----------------------------------------------------------------------------
// enables type promotion between char[] and TMemoryView<char>
template <>
template <size_t _Dim>
CONSTEXPR TMemoryView<const char>::TMemoryView(const char (&staticString)[_Dim]) NOEXCEPT
:   TMemoryView(staticString, _Dim - 1) {
    STATIC_ASSERT(0 < _Dim);
    Assert('\0' == staticString[_Dim - 1]);
}
//----------------------------------------------------------------------------
// enables type promotion between wchar_t[] and TMemoryView<wchar_t>
template <>
template <size_t _Dim>
CONSTEXPR TMemoryView<const wchar_t>::TMemoryView(const wchar_t (&staticString)[_Dim]) NOEXCEPT
:   TMemoryView(staticString, _Dim - 1) {
    STATIC_ASSERT(0 < _Dim);
    Assert('\0' == staticString[_Dim - 1]);
}
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR TMemoryView<T>::TMemoryView(TMemoryView&& rvalue) NOEXCEPT
:   _storage(std::move(rvalue._storage))
,   _size(std::move(rvalue._size)) {
    rvalue._storage = nullptr;
    rvalue._size = 0;
}
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR TMemoryView<T>& TMemoryView<T>::operator =(TMemoryView&& rvalue) NOEXCEPT {
    _storage = std::move(rvalue._storage);
    _size = std::move(rvalue._size);
    rvalue._storage = nullptr;
    rvalue._size = 0;
    return (*this);
}
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR TMemoryView<T>::TMemoryView(const TMemoryView& other) NOEXCEPT
:   _storage(other._storage), _size(other._size) {}
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR TMemoryView<T>& TMemoryView<T>::operator =(const TMemoryView& other) NOEXCEPT {
    _storage = other._storage;
    _size = other._size;
    return (*this);
}
//----------------------------------------------------------------------------
template <typename T>
template <typename U, Meta::TEnableIf<std::is_assignable_v<T*&, U*>>*>
CONSTEXPR TMemoryView<T>::TMemoryView(const TMemoryView<U>& other) NOEXCEPT
:   _storage(other._storage), _size(other._size) {
    STATIC_ASSERT(sizeof(T) == sizeof(U));
}
//----------------------------------------------------------------------------
template <typename T>
template <typename U, Meta::TEnableIf<std::is_assignable_v<T*&, U*>>*>
CONSTEXPR TMemoryView<T>& TMemoryView<T>::operator =(const TMemoryView<U>& other) NOEXCEPT {
    STATIC_ASSERT(sizeof(T) == sizeof(U));
    _storage = other._storage;
    _size = other._size;
    return (*this);
}
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR auto TMemoryView<T>::at(size_type index) const -> reference {
    Assert(index < _size);
    return _storage[index];
}
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR auto TMemoryView<T>::FindSubRange(const TMemoryView<T>& subrange) const -> iterator {
    const size_t n = subrange.size();
    Assert(0 != n);
    if (size() < n)
        return end();

    const auto sbegin = subrange.begin();
    const auto send = subrange.end();

    const auto last = end() - subrange.size() + 1;
    for (auto it = begin(); it != last; ++it) {
        if (std::equal(it, it + n, sbegin, send))
            return it;
    }

    return end();
}
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR bool TMemoryView<T>::EndsWith(const T& suffix) const {
    return (_size && back() == suffix);
}
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR bool TMemoryView<T>::StartsWith(const T& prefix) const {
    return (_size && front() == prefix);
}
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR bool TMemoryView<T>::EndsWith(const TMemoryView<T>& suffix) const {
    if (suffix.size() > _size)
        return false;

    const auto slice = LastNElements(suffix.size());
    return std::equal(slice.begin(), slice.end(), suffix.begin());
}
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR bool TMemoryView<T>::StartsWith(const TMemoryView<T>& prefix) const {
    if (prefix.size() > _size)
        return false;

    const auto slice = FirstNElements(prefix.size());
    return std::equal(slice.begin(), slice.end(), prefix.begin());
}
//----------------------------------------------------------------------------
template <typename T>
NODISCARD CONSTEXPR TMemoryView<T> TMemoryView<T>::Concat(const TMemoryView& other) const {
    if (Likely(_storage && other._storage)) {
        Assert(_storage + _size == other._storage);
        return TMemoryView<T>{ _storage, _size + other._size };
    }
    else if (_storage) {
        return (*this);
    }
    else {
        return other;
    }
}
//----------------------------------------------------------------------------
template <typename T>
NODISCARD CONSTEXPR TMemoryView<T> TMemoryView<T>::Concat_AssumeNotEmpty(const TMemoryView& other) const {
    Assert(_storage);
    Assert(other._storage);
    Assert_NoAssume(_storage + _size == other._storage);
    return TMemoryView<T>{ _storage, _size + other._size };
}
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR void TMemoryView<T>::CopyTo(const TMemoryView<Meta::TRemoveConst<T>>& dst) const {
    Assert(dst.size() == _size);
    IF_CONSTEXPR(std::is_trivially_copyable_v<T>) {
        //FPlatformMemory::Memcpy(dst.data(), _storage, _size);
        std::memcpy(dst.data(), _storage, _size * sizeof(T));
    }
    else {
        std::copy(begin(), end(), dst.begin());
    }
}
//----------------------------------------------------------------------------
template <typename T>
NODISCARD CONSTEXPR TMemoryView<T> TMemoryView<T>::SubRange(size_t offset, size_t count) const {
    Assert(offset <= _size);
    Assert(offset + count <= _size);
    return TMemoryView(_storage + offset, count);
}
//----------------------------------------------------------------------------
template <typename T>
NODISCARD CONSTEXPR TMemoryView< Meta::TAddConst<T> > TMemoryView<T>::SubRangeConst(size_t offset, size_t count) const {
    Assert(offset <= _size);
    Assert(offset + count <= _size);
    return TMemoryView< Meta::TAddConst<T> >(_storage + offset, count);
}
//----------------------------------------------------------------------------
template <typename T>
NODISCARD CONSTEXPR TMemoryView<T> TMemoryView<T>::SubRange(iterator first, iterator last) const {
    Assert(AliasesToContainer(first));
    Assert(AliasesToContainer(last));
    Assert(first <= last);
    return SubRange(std::distance(begin(), first), std::distance(first, last));
}
//----------------------------------------------------------------------------
template <typename T>
NODISCARD CONSTEXPR TMemoryView< Meta::TAddConst<T> > TMemoryView<T>::SubRangeConst(iterator first, iterator last) const {
    Assert(AliasesToContainer(first));
    Assert(AliasesToContainer(last));
    Assert(first <= last);
    return SubRangeConst(std::addressof(*first), std::distance(first, last));
}
//----------------------------------------------------------------------------
template <typename T>
NODISCARD CONSTEXPR TMemoryView<T> TMemoryView<T>::Slice(size_t index, size_t stride) const {
    Assert(stride <= _size);
    Assert(index < (_size + stride - 1) / stride);
    const size_t offset = index * stride;
    const size_t count = PPE::Min((index + 1) * stride, _size) - offset;
    return SubRange(offset, count);
}
//----------------------------------------------------------------------------
template <typename T>
NODISCARD CONSTEXPR TMemoryView< Meta::TAddConst<T> > TMemoryView<T>::SliceConst(size_t index, size_t stride) const {
    return Slice(index, stride).template Cast< Meta::TAddConst<T> >();
}
//----------------------------------------------------------------------------
template <typename T>
template <typename U>
NODISCARD TMemoryView<U> TMemoryView<T>::Cast() const {
    STATIC_ASSERT(  (0 == (sizeof(T) % sizeof(U)) ) ||
                    (0 == (sizeof(U) % sizeof(T)) ) );
    Assert_NoAssume((_size * sizeof(T)) % sizeof(U) == 0);

    return TMemoryView<U>(reinterpret_cast<U *>(_storage), (_size * sizeof(T)) / sizeof(U));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
