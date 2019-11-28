#pragma once

#include "Container/SparseArray.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
TBasicSparseArray<T>::TBasicSparseArray() NOEXCEPT
:   _size(0)
,   _freeIndex(InvalidIndex)
,   _highestIndex(0)
,   _uniqueKey(0)
,   _numChunks(0)
,   _chunks(nullptr)
{}
//----------------------------------------------------------------------------
template <typename T>
TBasicSparseArray<T>::~TBasicSparseArray() {
    Assert_NoAssume(CheckInvariants());
}
//----------------------------------------------------------------------------
template <typename T>
TBasicSparseArray<T>::TBasicSparseArray(TBasicSparseArray&& rvalue) NOEXCEPT
:   TBasicSparseArray() {
    Swap(*this, rvalue);
}
//----------------------------------------------------------------------------
template <typename T>
auto TBasicSparseArray<T>::operator =(TBasicSparseArray&& rvalue) -> TBasicSparseArray& {
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
template <typename T>
auto TBasicSparseArray<T>::begin() NOEXCEPT -> iterator {
    return iterator::Begin(*this);
}
//----------------------------------------------------------------------------
template <typename T>
auto TBasicSparseArray<T>::end() NOEXCEPT -> iterator {
    return iterator::End(*this);
}
//----------------------------------------------------------------------------
template <typename T>
auto TBasicSparseArray<T>::begin() const NOEXCEPT -> const_iterator {
    return const_iterator::Begin(*this);
}
//----------------------------------------------------------------------------
template <typename T>
auto TBasicSparseArray<T>::end() const NOEXCEPT -> const_iterator {
    return const_iterator::End(*this);
}
//----------------------------------------------------------------------------
template <typename T>
auto TBasicSparseArray<T>::IndexOf(const_reference data) const -> FDataId {
    Assert_NoAssume(CheckInvariants());
    Assert(AliasesToContainer(&data));

    auto it = reinterpret_cast<const FDataItem*>(&data);
    Assert(not UnpackId_(it->Id).empty());

    return it->Id;
}
//----------------------------------------------------------------------------
template <typename T>
auto TBasicSparseArray<T>::Iterator(FDataId id) -> iterator {
    Assert_NoAssume(CheckInvariants());
    Assert(_numChunks);

    const FUnpackedId_ unpacked = UnpackId_(id);
    FDataItem* const it = At_(unpacked.Index);

    // checks validity of weak ref before returning data
    return ((it->Id == id)
        ? iterator{ *this, unpacked.Index }
        : iterator::End(*this) );
}
//----------------------------------------------------------------------------
template <typename T>
auto TBasicSparseArray<T>::At(size_t index) -> reference {
    Assert_NoAssume(CheckInvariants());
    Assert(_numChunks);

    pointer const it = At_(index);
    AssertRelease(it);

    return (*it);
}
//----------------------------------------------------------------------------
template <typename T>
auto TBasicSparseArray<T>::Find(FDataId id) -> pointer {
    Assert_NoAssume(CheckInvariants());
    Assert(_numChunks);

    const FUnpackedId_ unpacked = UnpackId_(id);
    FDataItem* const it = At_(unpacked.Index);

    // checks validity of weak ref before returning data
    return (it->Id == id ? &it->Data : nullptr);
}
//----------------------------------------------------------------------------
template <typename T>
void TBasicSparseArray<T>::Swap(TBasicSparseArray& other) {
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
template <typename T>
bool TBasicSparseArray<T>::CheckInvariants() const {
    if (_size > _highestIndex)
        return false;
    if (_highestIndex > ckOffset_(_numChunks))
        return false;
    if (0 == _numChunks && _highestIndex)
        return false;
    if ((!_numChunks) ^ (!_chunks))
        return false;
    if (_freeIndex != InvalidIndex && _freeIndex >= _highestIndex)
        return false;

    return true;
}
//----------------------------------------------------------------------------
template <typename T>
bool TBasicSparseArray<T>::AliasesToContainer(const_pointer p) const {
    Assert(p);
    Assert_NoAssume(CheckInvariants());

    if (0 == _numChunks) {
        Assert(nullptr == _chunks);
        return false;
    }
    else if (1 == _numChunks) {
        Assert(_chunks);
        return AliasesToChunk_(reinterpret_cast<const FDataItem* const&>(_chunks), 0, p);
    }
    else {
        forrange(i, 0, _numChunks) {
            if (AliasesToChunk_(_chunks[i].Get(), i, p))
                return true;
        }
        return false;
    }
}
//----------------------------------------------------------------------------
template <typename T>
bool TBasicSparseArray<T>::AliasesToContainer(iterator it) const {
    return (it.Owner() == this && it >= begin() && end() >= it);
}
//----------------------------------------------------------------------------
template <typename T>
bool TBasicSparseArray<T>::AliasesToContainer(const_iterator it) const {
    return (it.Owner() == this && it >= begin() && end() >= it);
}
//----------------------------------------------------------------------------
template <typename T>
auto TBasicSparseArray<T>::At_(size_t index) -> FDataItem* {
    Assert(index < _highestIndex);
    Assert_NoAssume(index < capacity());

    const size_t cls = ckIndex_(index);
    Assert_NoAssume(0 == cls || _numChunks > 1);

    // store the chunk address directly instead of allocating a chunk array
    // avoid allocation for all TSparseArray<> when <= 1 chunk
    FDataItem* const chunk = (_numChunks == 1
        ? reinterpret_cast<FDataItem*&>(_chunks)
        : _chunks[cls].Get() );

    const size_t off = ckOffset_(cls);
    Assert_NoAssume(index - off < ckAllocation_(cls));

    return std::addressof(chunk[index - off]);
}
//----------------------------------------------------------------------------
template <typename T>
bool TBasicSparseArray<T>::AliasesToChunk_(const FDataItem* chunk, size_t cls, const void* ptr) const {
    Assert(chunk);
    Assert_NoAssume(cls < _numChunks);

    const size_t n = ckAllocation_(cls);
    return ((chunk <= ptr) & (chunk + n > ptr));
}
//----------------------------------------------------------------------------
template <typename T>
size_t TBasicSparseArray<T>::GrabFirstFreeBlock_() {
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
template <typename T>
auto TSparseArrayIterator<T>::GotoFirstItem_() -> TSparseArrayIterator& {
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
template <typename T>
auto TSparseArrayIterator<T>::GotoNextItem_() -> TSparseArrayIterator& {
    Assert(_owner);
    Assert(_index < _owner->_highestIndex);

    ++_index;

    return GotoFirstItem_();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
TSparseArray<T, _Allocator>::~TSparseArray() {
    ClearReleaseMemory_LeaveDirty_();
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
TSparseArray<T, _Allocator>::TSparseArray(const TSparseArray& other)
:   TSparseArray(allocator_traits::SelectOnCopy(other)) {
    Assign(other);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
auto TSparseArray<T, _Allocator>::operator =(const TSparseArray& other) -> TSparseArray& {
    Assign(other);
    return (*this);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
TSparseArray<T, _Allocator>::TSparseArray(TSparseArray&& rvalue)
:   _Allocator(allocator_traits::SelectOnMove(std::move(rvalue))) {
    Assign(std::move(rvalue));
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
auto TSparseArray<T, _Allocator>::operator =(TSparseArray&& rvalue) -> TSparseArray& {
    Assign(std::move(rvalue));
    return (*this);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
auto TSparseArray<T, _Allocator>::Add() -> reference {
    Assert_NoAssume(CheckInvariants());

    const FUnpackedId_ unpacked = AllocateItem_();
    Assert(not unpacked.empty());

    FDataItem* const it = At_(unpacked.Index);
    it->Id = PackId_(unpacked);
    Meta::Construct(&it->Data);

    return it->Data;
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
template <typename _It>
void TSparseArray<T, _Allocator>::AddRange(_It first, _It last) {
    Assert_NoAssume(CheckInvariants());

    using iterator_t = Meta::TIteratorTraits<_It>;
    AddRange_(first, last, typename iterator_t::iterator_category{});
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
template <typename... _Args>
auto TSparseArray<T, _Allocator>::Emplace(_Args&&... args) -> FDataId {
    Assert_NoAssume(CheckInvariants());

    const FUnpackedId_ unpacked = AllocateItem_();
    Assert(not unpacked.empty());

    FDataItem* const it = At_(unpacked.Index);
    it->Id = PackId_(unpacked);
    Meta::Construct(&it->Data, std::forward<_Args>(args)...);

    return (it->Id);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
template <typename... _Args>
auto TSparseArray<T, _Allocator>::EmplaceIt(_Args&&... args) -> iterator {
    Assert_NoAssume(CheckInvariants());

    const FUnpackedId_ unpacked = AllocateItem_();
    Assert(not unpacked.empty());

    FDataItem* const it = At_(unpacked.Index);
    it->Id = PackId_(unpacked);
    Meta::Construct(&it->Data, std::forward<_Args>(args)...);

    return iterator{ *this, unpacked.Index };
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TSparseArray<T, _Allocator>::Assign(const TSparseArray& other) {
    Clear_ReleaseMemory();

    allocator_traits::Copy(this, other);

    if (other.size()) {
        Reserve(other.size()); // better than Assign(begin(), end())
        AddRange(other.begin(), other.end());
    }
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TSparseArray<T, _Allocator>::Assign(TSparseArray&& rvalue) {
    IF_CONSTEXPR(allocator_traits::propagate_on_container_move_assignment::value) {
        Clear_ReleaseMemory();
        allocator_traits::Move(this, std::move(rvalue));
        TBasicSparseArray<T>::operator =(std::move(rvalue));
    }
    else {
        Clear();
        Reserve(rvalue.size());
        AddRange(MakeMoveIterator(rvalue.begin()), MakeMoveIterator(rvalue.end()));
        rvalue.Clear();
    }
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
bool TSparseArray<T, _Allocator>::Remove(FDataId id) {
    Assert_NoAssume(CheckInvariants());

    const FUnpackedId_ unpacked = UnpackId_(id);
    Assert(not unpacked.empty());

    FDataItem* const it = At_(unpacked.Index);
    if (it->Id != id)
        return false; // weak ref is invalid, can't remove

    ReleaseItem_(unpacked, it);

    return true;
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TSparseArray<T, _Allocator>::Remove(const_iterator it) {
    Assert(AliasesToContainer(it));

    const auto* const pitem = reinterpret_cast<const FDataItem*>(std::addressof(*it));

    const FUnpackedId_ unpacked = UnpackId_(pitem->Id);
    Assert(not unpacked.empty());

    ReleaseItem_(unpacked, const_cast<FDataItem*>(pitem));
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TSparseArray<T, _Allocator>::Remove(reference data) {
    Assert(AliasesToContainer(&data));

    auto* it = reinterpret_cast<FDataItem*>(&data);

    const FUnpackedId_ unpacked = UnpackId_(it->Id);
    Assert(not unpacked.empty());
    Assert(unpacked.Index < _highestIndex);

    ReleaseItem_(unpacked, it);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TSparseArray<T, _Allocator>::Clear() {
    Assert_NoAssume(CheckInvariants());

    forrange(i, 0, _highestIndex) {
        FDataItem* const it = At_(i);

        const FUnpackedId_ unpacked = UnpackId_(it->Id);
        if (not unpacked.empty()) {
            Assert(unpacked.Index < _highestIndex);
            ReleaseItem_(unpacked, it);
        }
    }

    // reset internal free list
    _freeIndex = InvalidIndex;
    _highestIndex = 0;
    _uniqueKey = 0;

    Assert(0 == _size);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TSparseArray<T, _Allocator>::Clear_ReleaseMemory() {
    if (_chunks) {
        ClearReleaseMemory_LeaveDirty_();

        _size = 0;
        _freeIndex = InvalidIndex;
        _highestIndex = 0;
        _uniqueKey = 0;
        _numChunks = 0;
        _chunks = nullptr;
    }
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TSparseArray<T, _Allocator>::Reserve(size_t n) {
    Assert(n);

    n = allocator_traits::template SnapSizeT<T>(n);

    const size_t cls = ckIndex_(n - 1);
    if (cls >= _numChunks) {
        const size_t csz = ckSize_(cls) - ckOffset_(_numChunks);

        auto* const newChunk= static_cast<FDataItem*>(allocator_traits::Allocate(*this, csz * sizeof(FDataItem)).Data);

        if ((!_numChunks) & (!cls)) {
            _numChunks = 1;
            reinterpret_cast<FDataItem*&>(_chunks) = newChunk;
            return;
        }

        Assert(cls); // must fill at least 2 chunks

        FDataChunkRef fakeChunk;
        fakeChunk.Reset(newChunk, false/* don't delete those slices of the allocation */);

        TMemoryView<FDataChunkRef> aliased;
        if (_numChunks == 1) {
            aliased = allocator_traits::template AllocateT<FDataChunkRef>(*this, cls + 1);
            aliased[0].Reset(reinterpret_cast<FDataItem*&>(_chunks), true/* first chunk is always deletable when alone */);
        }
        else {
            aliased = { _chunks, _numChunks };
            ReallocateAllocatorBlock_AssumePOD(
                allocator_traits::Get(*this),
                aliased, _numChunks, cls + 1);
            aliased = { aliased.data(), cls + 1 };
        }

        forrange(i, _numChunks, cls) {
            const size_t s = ckAllocation_(i);
            aliased[i] = fakeChunk;
            fakeChunk.Set(fakeChunk.Get() + s);
            Assert_NoAssume(fakeChunk.Get() < newChunk + csz);
        }

        // the last one indicates that the previous slices are contiguous, including this one
        fakeChunk.SetFlag0(true); // mark only the last chunk as deletable
        Assert_NoAssume(fakeChunk.Get() + ckAllocation_(cls) == newChunk + csz);
        aliased[cls] = fakeChunk;

        _chunks = aliased.data();
        _numChunks = checked_cast<u16>(aliased.size());

        Assert_NoAssume(CheckInvariants());
    }
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
bool TSparseArray<T, _Allocator>::Equals(const TSparseArray& other) const {
    return ( (&other == this) || (other._size == _size and
        std::equal(begin(), end(), other.begin(), other.end())) );
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
auto TSparseArray<T, _Allocator>::AllocateItem_() -> FUnpackedId_ {
    const size_t cls = ckIndex_(_size);

    if (cls == _numChunks)
        GrabNewChunk_(cls);
    Assert_NoAssume(cls < _numChunks);

    ++_size;
    AssertRelease(_size < InvalidIndex); // can't exceed max size :'(

    const size_t key = (_uniqueKey = (++_uniqueKey | 1)); // can't be 0, reserved for internal free list
    const size_t index = (_highestIndex < capacity()
        ? _highestIndex++ // use uninitialized block instead of free list IFP
        : GrabFirstFreeBlock_() );

    return FUnpackedId_{
#if PLATFORM_BIGENDIAN // so when Key == 0 we can use Index as a valid size_t
        key, index
#else
        index, key
#endif
        };
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TSparseArray<T, _Allocator>::ReleaseItem_(FUnpackedId_ id, FDataItem* it) {
    Assert_NoAssume(not id.empty());
    Assert(it);
    Assert(_size);
    Assert(_numChunks);

    Meta::Destroy(&it->Data);
    ONLY_IF_ASSERT(FPlatformMemory::Memdeadbeef(&it->Data, sizeof(it->Data)));

    const size_t oldIndex = id.Index;
    Assert_NoAssume(PackId_(id) == it->Id); // check ref validity

    // try sorting by ascending index order to keep the container packed
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
template <typename T, typename _Allocator>
NO_INLINE void TSparseArray<T, _Allocator>::GrabNewChunk_(size_t cls) {
    const size_t sz = ckAllocation_(cls);

    FDataChunkRef newChunk;
    newChunk.Reset(static_cast<FDataItem*>(allocator_traits::Allocate(*this, sz * sizeof(FDataItem)).Data), true/* should be deleted */);

    if (Likely(0 == _numChunks)) {
        Assert(nullptr == _chunks);
        Assert_NoAssume(0 == cls);

        _numChunks++; // don't allocate for the first chunk, pack ptr* in ptr**
        reinterpret_cast<FDataItem*&>(_chunks) = newChunk.Get();

        return;
    }
    else if (1 == _numChunks) {
        Assert(_chunks);

        // allocate another array for storing the list of chunks with random access
        auto* const arr = static_cast<FDataChunkRef*>(allocator_traits::Allocate(*this, 2 * sizeof(FDataChunkRef)).Data);
        arr[0].Reset(reinterpret_cast<FDataItem*&>(_chunks), true); // the first chunk is saved without extra allocation (first is always deletable if alone)
        _chunks = arr;
    }
    else {
        Assert(_chunks);

        // reallocate to add a new slot when growing, always call realloc but should rarely effectively allocate a new block
        TMemoryView<FDataChunkRef> aliased(_chunks, _numChunks);
        ReallocateAllocatorBlock_AssumePOD(allocator_traits::Get(*this), aliased, _numChunks, _numChunks + 1);
        _chunks = aliased.data();
    }

    _chunks[_numChunks++] = newChunk;

    Assert_NoAssume(CheckInvariants());
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TSparseArray<T, _Allocator>::ReleaseChunk_(FDataItem* chunk, size_t off, size_t sz) {
    Assert(chunk);
    Assert(sz);

    IF_CONSTEXPR(Meta::has_trivial_destructor<T>::value == false) {
        for (size_t i = 0; (i < sz) & (off + i < _highestIndex); ++i) {
            auto& it = reinterpret_cast<FDataItem*>(chunk)[i];
            if (not UnpackId_(it.Id).empty())
                Meta::Destroy(&it.Data);
        }
    }
    else {
        UNUSED(off);
    }

    allocator_traits::DeallocateT(*this, chunk, sz);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TSparseArray<T, _Allocator>::ClearReleaseMemory_LeaveDirty_() {
    Assert_NoAssume(CheckInvariants());

    if (0 == _numChunks) {
        Assert(0 == _size);
        Assert(0 == _highestIndex);
        Assert(InvalidIndex == _freeIndex);
        Assert(0 == _uniqueKey);
        Assert(nullptr == _chunks);
    }
    else if (1 == _numChunks) { // see GrabNewChunk_()
        Assert(ckSize_(1) >= _highestIndex);
        Assert(_highestIndex >= _size);
        Assert(_chunks);

        ReleaseChunk_(reinterpret_cast<FDataItem*&>(_chunks), 0, ckAllocation_(0));
    }
    else { // using an additional allocation for storing pointer
        Assert(_chunks);
        Assert_NoAssume(_size <= _highestIndex);
        Assert_NoAssume(capacity() >= _highestIndex);

        size_t fst = INDEX_NONE;
        forrange(i, 0, _numChunks) {
            // merge contiguous chunks before deleting
            if (_chunks[i].Flag0()) {
                const size_t sz = ckSize_(i);
                if (INDEX_NONE != fst) {
                    const size_t off = ckOffset_(fst);
                    Assert_NoAssume(_chunks[fst].Get() + (sz - off) == _chunks[i].Get() + (sz - ckOffset_(i)));
                    ReleaseChunk_(_chunks[fst].Get(), off, sz - off);
                }
                else {
                    const size_t off = ckOffset_(i);
                    ReleaseChunk_(_chunks[i].Get(), off, sz - off);
                }
            }
            else if (INDEX_NONE == fst) {
                fst = i;
            }
            else {
                Assert_NoAssume(_chunks[i].Get() == _chunks[i - 1].Get() + ckAllocation_(i - 1));
            }
        }

        // release the extra allocation made for storage
        allocator_traits::DeallocateT(*this, _chunks, _numChunks);
    }
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
template <typename _It>
void TSparseArray<T, _Allocator>::AddRange_(_It first, _It last, std::input_iterator_tag) {
    for (; first != last; ++first)
        Emplace(*first);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
template <typename _It, typename _IteratorTag>
void TSparseArray<T, _Allocator>::AddRange_(_It first, _It last, _IteratorTag) {
    Reserve(_size + std::distance(first, last));
    AddRange_(first, last, std::input_iterator_tag{});
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Allocator, typename _It>
void Assign(TSparseArray<T, _Allocator>& v, _It first, _It last) {
    v.Assign(first, last);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Append(TSparseArray<T, _Allocator>& v, const TMemoryView<const T>& elts) {
    v.AddRange(elts.begin(), elts.end());
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator, typename U>
bool Contains(const TSparseArray<T, _Allocator>& v, const U& elt) {
    return (v.end() != std::find_if(v.begin(), v.end(), [&](const auto& value) NOEXCEPT{
        return (value == elt);
    }));
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Add_AssertUnique(TSparseArray<T, _Allocator>& v, const T& elt) {
    Assert_NoAssume(not Contains(v, elt));
    v.Emplace(elt);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Add_AssertUnique(TSparseArray<T, _Allocator>& v, T&& elt) {
    Assert_NoAssume(not Contains(v, elt));
    v.Emplace(std::move(elt));
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
bool Add_Unique(TSparseArray<T, _Allocator>& v, T&& elt) {
    if (Contains(v, elt)) {
        return false;
    }
    else {
        v.Emplace(std::move(elt));
        return true;
    }
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator, typename... _Args>
auto Emplace_Back(TSparseArray<T, _Allocator>& v, _Args&&... args) -> typename TSparseArray<T, _Allocator>::iterator {
    return v.EmplaceIt(std::forward<_Args>(args)...);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Erase_DontPreserveOrder(TSparseArray<T, _Allocator>& v, const typename TSparseArray<T, _Allocator>::const_iterator& it) {
    // this is just for parity with TVector<>, so we can use TSparseArray<> with TAssociativeVector<>,
    // TSparseArray<> has already a fast erase algorithm and don't need this trick.
    v.Remove(it);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Remove_AssertExists(TSparseArray<T, _Allocator>& v, const T& elt) {
    Verify(Remove_ReturnIfExists(v, elt));
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
bool Remove_ReturnIfExists(TSparseArray<T, _Allocator>& v, const T& elt) {
    const auto it = std::find_if(v.begin(), v.end(), [&](const auto& value) NOEXCEPT{
        return (value == elt);
    });
    if (it != v.end()) {
        v.Remove(it);
        return true;
    }
    else {
        return false;
    }
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Clear(TSparseArray<T, _Allocator>& v) {
    v.Clear();
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Clear_ReleaseMemory(TSparseArray<T, _Allocator>& v) {
    v.Clear_ReleaseMemory();
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Reserve(TSparseArray<T, _Allocator>& v, size_t capacity) {
    v.Reserve(capacity);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
hash_t hash_value(const TSparseArray<T, _Allocator>& v) {
    return hash_range(v.begin(), v.end());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
