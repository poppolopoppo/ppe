#pragma once

#include "Container/RawStorage.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
TRawStorage<T, _Allocator>::TRawStorage()
:   _storage(nullptr), _size(0) {}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
TRawStorage<T, _Allocator>::TRawStorage(size_type size)
:   TRawStorage() {
    Resize_DiscardData(size);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
TRawStorage<T, _Allocator>::TRawStorage(allocator_type&& allocator)
:   allocator_type(std::move(allocator))
,   _storage(nullptr), _size(0) {}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
TRawStorage<T, _Allocator>::TRawStorage(allocator_type&& allocator, size_type size)
:   allocator_type(std::move(allocator))
,   _storage(nullptr), _size(0) {
    Resize_DiscardData(size);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
TRawStorage<T, _Allocator>::TRawStorage(allocator_type&& allocator, const TMemoryView<T>& stolen)
:   allocator_type(std::move(allocator))
,   _storage(stolen.data())
,   _size(stolen.size())
{}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
template <typename _It>
TRawStorage<T, _Allocator>::TRawStorage(_It&& begin, _It&& end)
:   _storage(nullptr), _size(0) {
    insert(this->end(), begin, end);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
TRawStorage<T, _Allocator>::~TRawStorage() {
    clear_ReleaseMemory();
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
TRawStorage<T, _Allocator>::TRawStorage(TRawStorage&& rvalue)
:   allocator_type(std::move(rvalue))
,   _storage(std::move(rvalue._storage))
,   _size(std::move(rvalue._size)) {
    rvalue._storage = nullptr;
    rvalue._size = 0;
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
TRawStorage<T, _Allocator>& TRawStorage<T, _Allocator>::operator =(TRawStorage&& rvalue) {
    clear_ReleaseMemory();
    allocator_type::operator =(std::move(rvalue));
    _storage = std::move(rvalue._storage);
    _size = std::move(rvalue._size);
    rvalue._storage = nullptr;
    rvalue._size = 0;
    return (*this);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
TRawStorage<T, _Allocator>::TRawStorage(const TRawStorage& other)
:   TRawStorage(other.begin(), other.end() ) {
    allocator_type::operator =(other);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
TRawStorage<T, _Allocator>& TRawStorage<T, _Allocator>::operator =(const TRawStorage& other) {
    clear_ReleaseMemory();
    allocator_type::operator =(other);
    insert(end(), other.begin(), other.end());
    return (*this);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
auto TRawStorage<T, _Allocator>::at(size_type index) -> reference {
    Assert(index < _size);
    return _storage[index];
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
auto TRawStorage<T, _Allocator>::at(size_type index) const -> const_reference {
    Assert(index < _size);
    return _storage[index];
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TRawStorage<T, _Allocator>::CopyFrom(const TMemoryView<const T>& src) {
    Resize_DiscardData(src.size());
    src.template Cast<const u8>().CopyTo(MakeView());
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TRawStorage<T, _Allocator>::Swap(TRawStorage& other) {
    std::swap(other._storage, _storage);
    std::swap(other._size, _size);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TRawStorage<T, _Allocator>::Resize(size_type size, bool keepData) {
    if (_size == size)
        return;

    T *const storage = _storage;
    _storage = nullptr;

    if (0 == size) {
        if (storage)
            allocator_type::deallocate(storage, _size);
    }
    else if (keepData) {
        _storage = Relocate_AssumePod(static_cast<allocator_type&>(*this), TMemoryView<T>(storage, _size), size, _size);
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
void TRawStorage<T, _Allocator>::insert(iterator after, _It&& begin, _It&& end) {
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
void TRawStorage<T, _Allocator>::clear_ReleaseMemory() {
    if (nullptr == _storage) {
        Assert(0 == _size);
        return;
    }

    allocator_type::deallocate(_storage, _size);
    _storage = nullptr;
    _size = 0;
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
bool TRawStorage<T, _Allocator>::Equals(const TRawStorage& other) const {
    if (_size != other._size)
        return false;
    else if (_storage == other._storage)
        return true;

    Assert(_size == other._size);
    forrange(i, 0, _size)
        if (_storage[i] != other._storage[i])
            return false;

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE