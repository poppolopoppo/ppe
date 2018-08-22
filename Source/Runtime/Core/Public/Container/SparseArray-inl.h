#pragma once

#include "Container/SparseArray.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _ChunkSize>
TBasicSparseArray<T, _ChunkSize>::TBasicSparseArray() NOEXCEPT
:   _size(0)
,   _freeIndex(InvalidIndex)
,   _highestIndex(0)
,   _uniqueKey(0)
,   _numChunks(0)
,   _chunks(nullptr)
{}
//----------------------------------------------------------------------------
template <typename T, size_t _ChunkSize>
TBasicSparseArray<T, _ChunkSize>::~TBasicSparseArray() {
#ifdef WITH_PPE_ASSERT
    Assert_NoAssume(CheckInvariants());
    FPlatformMemory::Memdeadbeef(this, sizeof(*this)); // crash if used after destruction
#endif
}
//----------------------------------------------------------------------------
template <typename T, size_t _ChunkSize>
TBasicSparseArray<T, _ChunkSize>::TBasicSparseArray(TBasicSparseArray&& rvalue) NOEXCEPT
:   TBasicSparseArray() {
    Swap(*this, rvalue);
}
//----------------------------------------------------------------------------
template <typename T, size_t _ChunkSize>
auto TBasicSparseArray<T, _ChunkSize>::operator =(TBasicSparseArray&& rvalue) -> TBasicSparseArray& {
    Assert(0 == _size);
    Assert(InvalidIndex == _freeIndex);
    AssertRelease(0 == _highestIndex); // can't move over a used container (restriction for pointer validity)
    Assert(0 == _uniqueKey);
    Assert(0 == _numChunks);
    Assert(nullptr == _numChunks);

    Swap(*this, rvalue);

    return (*this);
}
//----------------------------------------------------------------------------
template <typename T, size_t _ChunkSize>
auto TBasicSparseArray<T, _ChunkSize>::begin() NOEXCEPT -> iterator {
    return iterator::Begin(*this);
}
//----------------------------------------------------------------------------
template <typename T, size_t _ChunkSize>
auto TBasicSparseArray<T, _ChunkSize>::end() NOEXCEPT -> iterator {
    return iterator::End(*this);
}
//----------------------------------------------------------------------------
template <typename T, size_t _ChunkSize>
auto TBasicSparseArray<T, _ChunkSize>::begin() const NOEXCEPT -> const_iterator {
    return const_iterator::Begin(*this);
}
//----------------------------------------------------------------------------
template <typename T, size_t _ChunkSize>
auto TBasicSparseArray<T, _ChunkSize>::end() const NOEXCEPT -> const_iterator {
    return const_iterator::End(*this);
}
//----------------------------------------------------------------------------
template <typename T, size_t _ChunkSize>
auto TBasicSparseArray<T, _ChunkSize>::IndexOf(const_reference data) const -> FDataId {
    Assert_NoAssume(CheckInvariants());
    Assert(AliasesToContainer(&data));

    auto it = reinterpret_cast<const FDataItem*>(&data);
    Assert(not UnpackId_(it->Id).empty());

    return it->Id;
}
//----------------------------------------------------------------------------
template <typename T, size_t _ChunkSize>
auto TBasicSparseArray<T, _ChunkSize>::Find(FDataId id) -> pointer {
    Assert_NoAssume(CheckInvariants());
    Assert(_numChunks);

    const FUnpackedId_ unpacked = UnpackId_(id);
    FDataItem* const it = At_(unpacked.Index);

    // checks validity of weak ref before returning data
    return (it->Id == id ? &it->Data : nullptr);
}
//----------------------------------------------------------------------------
template <typename T, size_t _ChunkSize>
void TBasicSparseArray<T, _ChunkSize>::Swap(TBasicSparseArray& other) {
    Assert_NoAssume(CheckInvariants());
    Assert_NoAssume(other.CheckInvariants());
#if 0
    std::swap(_size, other._size);
    std::swap(_freeIndex, other._freeIndex);
    std::swap(_highestIndex, other._highestIndex);
    std::swap(_uniqueKey, other._uniqueKey);
    std::swap(_numChunks, other._numChunks);
    std::swap(_chunks, other._chunks);
#else
    FPlatformMemory::Memswap(this, &other, sizeof(*this));
#endif
}
//----------------------------------------------------------------------------
template <typename T, size_t _ChunkSize>
bool TBasicSparseArray<T, _ChunkSize>::CheckInvariants() const {
    if (_size > _highestIndex)
        return false;
    if (_freeIndex >= _highestIndex)
        return false;
    if (_highestIndex && 0 == _numChunks)
        return false;
    if (_numChunks && nullptr == _chunks)
        return false;

    return true;
}
//----------------------------------------------------------------------------
template <typename T, size_t _ChunkSize>
bool TBasicSparseArray<T, _ChunkSize>::AliasesToContainer(const_pointer p) const {
    Assert(p);

    if (0 == _numChunks) {
        Assert(nullptr == _chunks);
        return false;
    }
    else if (1 == _numChunks) {
        Assert(_chunks);
        return AliasesToChunk_(reinterpret_cast<const FDataChunk*>(_chunks), p);
    }
    else {
        forrange(it, _chunks, _chunks + _numChunks) {
            if (AliasesToChunk_(*it, p))
                return true;
        }
        return false;
    }
}
//----------------------------------------------------------------------------
template <typename T, size_t _ChunkSize>
bool TBasicSparseArray<T, _ChunkSize>::AliasesToContainer(iterator it) const {
    return (it._owner == this && begin() <= it && it <= end());
}
//----------------------------------------------------------------------------
template <typename T, size_t _ChunkSize>
bool TBasicSparseArray<T, _ChunkSize>::AliasesToContainer(const_iterator it) const {
    return (it._owner == this && begin() <= it && it <= end());
}
//----------------------------------------------------------------------------
template <typename T, size_t _ChunkSize>
auto TBasicSparseArray<T, _ChunkSize>::At_(size_t index) -> FDataItem* {
    Assert(index < _highestIndex);
    Assert(index < _numChunks * ChunkSize);

    // store the chunk address directly instead of allocating a chunk array
    // avoid allocation for all TSparseArray<> witch <= 1 chunk
    FDataChunk& chunk = (_numChunks == 1
        ? *reinterpret_cast<FDataChunk*>(_chunks)
        : *_chunks[index / ChunkSize] );

    STATIC_ASSERT(Meta::IsPow2(_ChunkSize));
    return std::addressof(chunk[index & _ChunkSize]);
}
//----------------------------------------------------------------------------
template <typename T, size_t _ChunkSize>
size_t TBasicSparseArray<T, _ChunkSize>::GrabFirstFreeBlock_() {
    Assert(InvalidIndex != _freeIndex);

    const size_t result = _freeIndex;
    FDataItem* const head = At_(result);

    Assert(UnpackId_(head->Id).empty()); // check that this a deleted key
    _freeIndex = checked_cast<u32>(head->Id); // allow to treat Id as a valid size_t

    return result;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _ChunkSize>
auto TSparseArrayIterator<T, _ChunkSize>::GotoFirstItem_() -> TSparseArrayIterator& {
    Assert(_owner);
    Assert(_index <= _owner->_highestIndex);

    for (;;) {
        if (_index == _owner->_highestIndex ||
            not FSparseArray::UnpackId_(_owner->At_(_index)->Id).empty())
            break;

        ++_index;
    }

    return (*this);
}
//----------------------------------------------------------------------------
template <typename T, size_t _ChunkSize>
auto TSparseArrayIterator<T, _ChunkSize>::GotoNextItem_() -> TSparseArrayIterator& {
    Assert(_owner);
    Assert(_index < _owner->_highestIndex);

    ++_index;

    return GotoFirstItem_();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _ChunkSize, typename _Allocator>
TSparseArray<T, _ChunkSize, _Allocator>::~TSparseArray() {
    ClearReleaseMemory_LeaveDirty_();
}
//----------------------------------------------------------------------------
template <typename T, size_t _ChunkSize, typename _Allocator>
TSparseArray<T, _ChunkSize, _Allocator>::TSparseArray(TSparseArray&& rvalue) NOEXCEPT
:   TBasicSparseArray(std::move(rvalue)) {
    AllocatorPropagateMove(get_allocator_(), std::move(rvalue.get_allocator_()));
}
//----------------------------------------------------------------------------
template <typename T, size_t _ChunkSize, typename _Allocator>
auto TSparseArray<T, _ChunkSize, _Allocator>::operator =(TSparseArray&& rvalue) -> TSparseArray& {
    parent_type::operator =(std::move(rvalue));
    AllocatorPropagateMove(get_allocator_(), std::move(rvalue.get_allocator_()));
    return (*this);
}
//----------------------------------------------------------------------------
template <typename T, size_t _ChunkSize, typename _Allocator>
auto TSparseArray<T, _ChunkSize, _Allocator>::Add() -> reference {
    const FUnpackedId_ unpacked = Allocate_();
    Assert(not unpacked.empty());

    FDataItem* const it = At_(unpacked.Index);
    it->Id = PackId_(unpacked);
    allocator_traits::construct(get_allocator_(), &it->Data);

    return it->Data;
}
//----------------------------------------------------------------------------
template <typename T, size_t _ChunkSize, typename _Allocator>
template <typename... _Args>
auto TSparseArray<T, _ChunkSize, _Allocator>::Emplace(_Args&&... args) -> FDataId {
    const FUnpackedId_ unpacked = Allocate_();
    Assert(not unpacked.empty());

    FDataItem* const it = At_(unpacked.Index);
    it->Id = PackId_(unpacked);
    allocator_traits::construct(get_allocator_(), &it->Data, std::forward<_Args>(args)...);

    return it->Id;
}
//----------------------------------------------------------------------------
template <typename T, size_t _ChunkSize, typename _Allocator>
bool TSparseArray<T, _ChunkSize, _Allocator>::Remove(FDataId id) {
    const FUnpackedId_ unpacked = UnpackId_(id);
    Assert(not unpacked.empty());

    FDataItem* const it = At_(unpacked.Index);
    if (it->Id != id)
        return false; // weak ref is invalid, can't remove

    allocator_traits::destroy(get_allocator_(), &it->Data);

    Release_(unpacked, it);
    return true;
}
//----------------------------------------------------------------------------
template <typename T, size_t _ChunkSize, typename _Allocator>
void TSparseArray<T, _ChunkSize, _Allocator>::Remove(iterator it) {
    Assert(AliasesToContainer(it));

    auto* const pitem = reinterpret_cast<FDataItem*>(std::addressof(*it));

    const FUnpackedId_ unpacked = UnpackId_(pitem->Id);
    Assert(not unpacked.empty());

    allocator_traits::destroy(get_allocator_(), &pitem->Data);

    Release_(unpacked, pitem);
}
//----------------------------------------------------------------------------
template <typename T, size_t _ChunkSize, typename _Allocator>
void TSparseArray<T, _ChunkSize, _Allocator>::Remove(reference data) {
    Assert(AliasesToContainer(&data));

    auto* it = reinterpret_cast<FDataItem*>(&data);

    const FUnpackedId_ unpacked = UnpackId_(it->Id);
    Assert(not unpacked.empty());
    Assert(unpacked.Index < _highestIndex);

    allocator_traits::destroy(get_allocator_(), &it->Data);

    Release_(unpacked, it);
}
//----------------------------------------------------------------------------
template <typename T, size_t _ChunkSize, typename _Allocator>
void TSparseArray<T, _ChunkSize, _Allocator>::Clear() {
    forrange(i, 0, _highestIndex) {
        FDataItem* const it = At_(i);

        const FUnpackedId_ unpacked = UnpackId_(it->Id);
        Assert(not unpacked.empty());
        Assert(unpacked.Index < _highestIndex);

        allocator_traits::destroy(get_allocator_(), &it->Data);

        Release_(unpacked, it);
    }

    Assert(0 == _size);
}
//----------------------------------------------------------------------------
template <typename T, size_t _ChunkSize, typename _Allocator>
auto TSparseArray<T, _ChunkSize, _Allocator>::Allocate_() -> FUnpackedId_ {
    if (_numChunks * _ChunkSize == _size)
        GrabNewChunk_();
    Assert(_numChunks * _ChunkSize > _size);

    ++_size;
    AssertRelease(_size < InvalidIndex); // can't exceed max size :'(

    return FUnpackedId_{
        _uniqueKey = (++_uniqueKey | 1), // can't be 0, reserved for internal free list
        (_highestIndex < _numChunks * _ChunkSize
            ? _highestIndex++ // use uninitialized block instead of free list IFP
            : GrabFirstFreeBlock_()) };
}
//----------------------------------------------------------------------------
template <typename T, size_t _ChunkSize, typename _Allocator>
void TSparseArray<T, _ChunkSize, _Allocator>::Release_(FUnpackedId_ id, FDataItem* it) {
    Assert_NoAssume(not id.empty());
    Assert(it);
    Assert(_size);
    Assert(_numChunks);

    // assuming it.Data was already destroyed !

    const size_t oldIndex = id.Index;
    Assert_NoAssume(PackId_(id) == it->Id); // check ref validity

    // tries to keep the container packed by sorting ascending index order
    if (_freeIndex < oldIndex) {
        Assert(InvalidIndex != _freeIndex);

        FDataItem* const ft = At_(_freeIndex);

        // don't sort completely, so no guarantee on final sort order
        // but this is better than doing nothing
        it->Id = ft->Id;
        ft->Id = oldIndex;
    }
    else {
        it->Id = _freeIndex;
        _freeIndex = checked_cast<u32>(oldIndex);
    }
    Assert(UnpackId_(it->Id).empty());

    --_size; // won't ever shrink allocation
}
//----------------------------------------------------------------------------
template <typename T, size_t _ChunkSize, typename _Allocator>
NO_INLINE void TSparseArray<T, _ChunkSize, _Allocator>::GrabNewChunk_() {
    Assert_NoAssume(SafeAllocatorSnapSize(get_allocator(), _ChunkSize) == _ChunkSize); // checks that we're not wasting memory

    FDataItem* const newChunk = allocator_traits::allocate(get_allocator_(), _ChunkSize);

    if (Likely(0 == _numChunks)) {
        Assert(nullptr == _chunks);

        _numChunks++; // don't allocate for the first chunk, pack ptr* in ptr**
        _chunks = reinterpret_cast<FDataChunk**>(newChunk);

        return;
    }
    else if (1 == _numChunks) {
        Assert(_chunks);

        FDataChunk* const firstChunk = reinterpret_cast<FDataChunk*>(_chunks); // the first chunk is saved without extra allocation

        _chunks = (FDataChunk**)allocator_traits::allocate(get_allocator_(), _ChunkSize);
        _chunks[0] = firstChunk;
    }
    else if (Unlikely(0 == _numChunks % NumPtrsPerChunk)) {
        Assert(_chunks);
        Assert(Meta::RoundToNext(_numChunks, NumPtrsPerChunk) == _numChunks);

        const TMemoryView<FDataChunk*> oldChunks(_chunks, _numChunks);
        const TMemoryView<FDataItem> aliasedChunks = oldChunks.Cast<FDataItem>();

        _chunks = (FDataChunk**)Relocate_AssumePod( // assume POD to do a simple memcpy (not used a FDataItem*)
            get_allocator_(),
            aliasedChunks,
            aliasedChunks.size() + _ChunkSize, // a new FDataChunk worth of free space
            aliasedChunks.size() );
    }

    _chunks[_numChunks++] = reinterpret_cast<FDataChunk*>(_chunks);
}
//----------------------------------------------------------------------------
template <typename T, size_t _ChunkSize, typename _Allocator>
void TSparseArray<T, _ChunkSize, _Allocator>::ReleaseChunk_(FDataChunk* chunk) {
    Assert(chunk);

    allocator_type& al = get_allocator_();

    for (FDataItem& it : *chunk) {
        if (not UnpackId_(it.Id).empty())
            allocator_traits::destroy(al, &it.Data);
    }

    allocator_traits::deallocate(al, reinterpret_cast<FDataItem*>(chunk), _ChunkSize);
}
//----------------------------------------------------------------------------
template <typename T, size_t _ChunkSize, typename _Allocator>
void TSparseArray<T, _ChunkSize, _Allocator>::ClearReleaseMemory_LeaveDirty_() {
    if (0 == _numChunks) {
        Assert(0 == _size);
        Assert(0 == _highestIndex);
        Assert(InvalidIndex == _freeIndex);
        Assert(0 == _uniqueKey);
        Assert(nullptr == _chunks);
    }
    else if (1 == _numChunks) { // see GrabNewChunk_()
        Assert(_ChunkSize >= _size);
        Assert(_ChunkSize >= _highestIndex);
        Assert(_chunks);

        ReleaseChunk_(reinterpret_cast<FDataChunk*>(_chunks));
    }
    else { // using an additional allocation for storing pointer
        Assert(_ChunkSize < _highestIndex);
        Assert(_chunks);

        forrange(it, _chunks, _chunks + _numChunks)
            ReleaseChunk_(*it);

        // release the extra allocation made for storage
        const size_t numChunksReserved = Meta::RoundToNext(_numChunks, NumPtrsPerChunk);
        const TMemoryView<FDataChunk*> storage(_chunks, numChunksReserved);
        const TMemoryView<FDataItem> aliased = storage.Cast<FDataItem>();

        allocator_traits::deallocate(get_allocator_(), aliased.data(), aliased.size());
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
