#pragma once

#include "Core/Container/RawStorage.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
RawStorage<T, _Allocator>::RawStorage()
:   _storage(nullptr), _size(0) {}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
RawStorage<T, _Allocator>::RawStorage(size_type size)
:   RawStorage() {
    Resize_DiscardData(size);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
RawStorage<T, _Allocator>::RawStorage(allocator_type&& allocator)
:   allocator_type(std::move(allocator))
,   _storage(nullptr), _size(0) {}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
RawStorage<T, _Allocator>::RawStorage(size_type size, allocator_type&& allocator)
:   allocator_type(std::move(allocator))
,   _storage(nullptr), _size(0) {
    Resize_DiscardData(size);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
template <typename _It>
RawStorage<T, _Allocator>::RawStorage(_It&& begin, _It&& end)
:   _storage(nullptr), _size(0) {
    insert(this->end(), begin, end);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
RawStorage<T, _Allocator>::~RawStorage() {
    Clear_ReleaseMemory();
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
RawStorage<T, _Allocator>::RawStorage(RawStorage&& rvalue)
:   allocator_type(std::move(rvalue))
,   _storage(std::move(rvalue._storage))
,   _size(std::move(rvalue._size)) {
    rvalue._storage = nullptr;
    rvalue._size = 0;
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
RawStorage<T, _Allocator>& RawStorage<T, _Allocator>::operator =(RawStorage&& rvalue) {
    Clear_ReleaseMemory();
    allocator_type::operator =(std::move(rvalue));
    _storage = std::move(rvalue._storage);
    _size = std::move(rvalue._size);
    rvalue._storage = nullptr;
    rvalue._size = 0;
    return (*this);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
RawStorage<T, _Allocator>::RawStorage(const RawStorage& other)
:   RawStorage(other.begin(), other.end() ) {
    allocator_type::operator =(other);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
RawStorage<T, _Allocator>& RawStorage<T, _Allocator>::operator =(const RawStorage& other) {
    Clear_ReleaseMemory();
    allocator_type::operator =(other);
    insert(end(), other.begin(), other.end());
    return (*this);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
auto RawStorage<T, _Allocator>::at(size_type index) -> reference {
    Assert(index < _size);
    return _storage[index];
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
auto RawStorage<T, _Allocator>::at(size_type index) const -> const_reference {
    Assert(index < _size);
    return _storage[index];
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void RawStorage<T, _Allocator>::Swap(RawStorage& other) {
    std::swap(other._storage, _storage);
    std::swap(other._size, _size);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void RawStorage<T, _Allocator>::Resize(size_type size, bool keepData) {
    if (_size == size)
        return;

    T *const storage = _storage;
    _storage = nullptr;

    if (0 == size) {
        if (storage)
            allocator_type::deallocate(storage, _size);
    }
    else if (keepData) {
        _storage = Relocate_AssumePod(static_cast<allocator_type&>(*this), MemoryView<T>(storage, _size), size, _size);
        AssertRelease(_storage);
    }
    else {
        if (storage)
            allocator_type::deallocate(storage, _size);

        _storage = allocator_type::allocate(size);
        AssertRelease(_storage);
    }

    _size = size;
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
template <typename _It>
void RawStorage<T, _Allocator>::insert(iterator after, _It&& begin, _It&& end) {
    Assert(after >= this->begin() && after <= this->end());

    const size_t insertCount = std::distance(begin, end);
    const size_t insertOffset = std::distance(this->begin(), after);

    if (insertOffset + insertCount > _size)
        Resize_KeepData(insertOffset + insertCount);

    for (size_t i = insertOffset; begin != end; ++begin, ++i)
        _storage[i] = *begin;
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void RawStorage<T, _Allocator>::Clear_ReleaseMemory() {
    if (nullptr == _storage) {
        Assert(0 == _size);
        return;
    }

    allocator_type::deallocate(_storage, _size);
    _storage = nullptr;
    _size = 0;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
