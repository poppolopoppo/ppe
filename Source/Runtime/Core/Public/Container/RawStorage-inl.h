#pragma once

#include "Container/RawStorage.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
TRawStorage<T, _Allocator>::TRawStorage() NOEXCEPT
:   _storage(nullptr)
,   _sizeInBytes(0) {}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
TRawStorage<T, _Allocator>::TRawStorage(size_type size)
:   TRawStorage() {
    Resize_DiscardData(size);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
TRawStorage<T, _Allocator>::TRawStorage(allocator_type&& allocator) NOEXCEPT
:   allocator_type(std::move(allocator))
,   _storage(nullptr)
,   _sizeInBytes(0) {}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
TRawStorage<T, _Allocator>::TRawStorage(allocator_type&& allocator, size_type size)
:   allocator_type(std::move(allocator))
,   _storage(nullptr)
,   _sizeInBytes(0) {
    Resize_DiscardData(size);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
template <typename _It>
TRawStorage<T, _Allocator>::TRawStorage(_It&& begin, _It&& end)
:   _storage(nullptr)
,   _sizeInBytes(0) {
    insert(this->end(), begin, end);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
TRawStorage<T, _Allocator>::~TRawStorage() {
    STATIC_ASSERT(Meta::has_trivial_destructor<T>::value);
    clear_ReleaseMemory();
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
TRawStorage<T, _Allocator>::TRawStorage(TRawStorage&& rvalue) NOEXCEPT
:   allocator_type(allocator_traits::SelectOnMove(std::move(rvalue)))
,   _storage(std::move(rvalue._storage))
,   _sizeInBytes(std::move(rvalue._sizeInBytes)) {
    rvalue._storage = nullptr;
    rvalue._sizeInBytes = 0;
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
TRawStorage<T, _Allocator>& TRawStorage<T, _Allocator>::operator =(TRawStorage&& rvalue) NOEXCEPT {
    clear_ReleaseMemory();
    allocator_traits::Move(this, std::move(rvalue));
    _storage = std::move(rvalue._storage);
    _sizeInBytes = std::move(rvalue._sizeInBytes);
    rvalue._storage = nullptr;
    rvalue._sizeInBytes = 0;
    return (*this);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
TRawStorage<T, _Allocator>::TRawStorage(const TRawStorage& other)
:   allocator_type(allocator_traits::SelectOnCopy(other))
,   _storage(nullptr)
,   _sizeInBytes(0) {
    insert(this->end(), other.begin(), other.end());
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
TRawStorage<T, _Allocator>& TRawStorage<T, _Allocator>::operator =(const TRawStorage& other) {
    clear_ReleaseMemory();
    allocator_traits::Copy(this, other);
    insert(end(), other.begin(), other.end());
    return (*this);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
auto TRawStorage<T, _Allocator>::at(size_type index) -> reference {
    Assert(index < size());
    return _storage[index];
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
auto TRawStorage<T, _Allocator>::at(size_type index) const -> const_reference {
    Assert(index < size());
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
void TRawStorage<T, _Allocator>::Swap(TRawStorage& other) NOEXCEPT {
    std::swap(other._storage, _storage);
    std::swap(other._sizeInBytes, _sizeInBytes);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TRawStorage<T, _Allocator>::Resize(size_type size, bool keepData) {
    if (_sizeInBytes == size * sizeof(value_type))
        return;

    T *const storage = _storage;
    _storage = nullptr;

    if (0 == size) {
        if (storage)
            allocator_traits::Deallocate(*this, FAllocatorBlock{ _storage, _sizeInBytes });
    }
    else if (keepData) {
        TMemoryView<T> blk(storage, this->size());
        ReallocateAllocatorBlock_AssumePOD(allocator_traits::Get(*this), blk, this->size(), size);
        AssertRelease(blk.data());
        _storage = blk.data();
    }
    else {
        if (storage)
            allocator_traits::Deallocate(*this, FAllocatorBlock{ storage, _sizeInBytes });

        _storage = allocator_traits::template AllocateT<value_type>(*this, size).data();
        AssertRelease(_storage);
    }

    _sizeInBytes = size * sizeof(value_type);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
template <typename _It>
void TRawStorage<T, _Allocator>::insert(iterator after, _It&& begin, _It&& end) {
    Assert(after >= this->begin() && after <= this->end());

    const size_t insertCount = std::distance(begin, end);
    const size_t insertOffset = std::distance(this->begin(), after);

    if (insertOffset + insertCount > size())
        Resize_KeepData(insertOffset + insertCount);

    for (size_t i = insertOffset; begin != end; ++begin, ++i)
        _storage[i] = *begin;
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TRawStorage<T, _Allocator>::clear_ReleaseMemory() {
    if (nullptr == _storage) {
        Assert(0 == _sizeInBytes);
        return;
    }

    allocator_traits::Deallocate(*this, FAllocatorBlock{ _storage, _sizeInBytes });
    _storage = nullptr;
    _sizeInBytes = 0;
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
bool TRawStorage<T, _Allocator>::Equals(const TRawStorage& other) const {
    if (_sizeInBytes != other._sizeInBytes)
        return false;
    if (_storage == other._storage)
        return true;

    Assert(_sizeInBytes == other._sizeInBytes);
    forrange(i, 0, size())
        if (_storage[i] != other._storage[i])
            return false;

    return true;
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
bool TRawStorage<T, _Allocator>::AcquireDataUnsafe(FAllocatorBlock b) NOEXCEPT {
    Assert_NoAssume(Meta::IsAlignedPow2(sizeof(value_type), b.SizeInBytes));

    if (allocator_traits::Acquire(*this, b)) {
        clear_ReleaseMemory();

        _storage = static_cast<pointer>(b.Data);
        _sizeInBytes = b.SizeInBytes;

        return true;
    }
    else {
        return false;
    }
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
FAllocatorBlock TRawStorage<T, _Allocator>::StealDataUnsafe() NOEXCEPT {
    FAllocatorBlock b{ _storage, _sizeInBytes };
    if (allocator_traits::Steal(*this, b)) {
        // won't delete the block since it's been stolen !

        _storage = nullptr;
        _sizeInBytes = 0;

        return b;
    }
    else {
        return FAllocatorBlock::Null();
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
