#pragma once

#include "Core.h"

#include "Allocator/Allocation.h"
#include "HAL/PlatformMemory.h" // Memswap
#include "Memory/MemoryView.h"
#include "Meta/Iterator.h"

// Inspired from DataArray<T>
// http://greysphere.tumblr.com/post/31601463396/data-arrays

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Important properties :
// - act as a keyed container
// - dynamic growth
// - won't invalidate it's storage validity
// - an internal free list is used to get an available slot
//----------------------------------------------------------------------------
using FSparseDataId = size_t;
//----------------------------------------------------------------------------
template <typename T>
struct TSparseArrayItem {
    T Data;
    FSparseDataId Id;
};
//----------------------------------------------------------------------------
template <typename T, size_t _ChunkSize>
class TSparseArrayIterator;
//----------------------------------------------------------------------------
template <typename T, size_t _ChunkSize>
class TBasicSparseArray {
public:
    // Choose this wisely this parameter wisely :
    // - too high and you would waste lot of memory (can't shrink) ;
    // - too low and you would allocate very often.
    // The value is specified as a template parameter to adapt to specific usages.
    STATIC_CONST_INTEGRAL(size_t, ChunkSize, _ChunkSize);

    using FDataId = FSparseDataId;
    using FDataItem = TSparseArrayItem<T>;
    using FDataChunk = Meta::TArray<T, _ChunkSize>;

    using value_type = T;
    using pointer = std::add_pointer_t<T>;
    using const_pointer = std::add_pointer_t<std::add_const_t<T>>;
    using reference = std::add_lvalue_reference_t<T>;
    using const_reference = std::add_lvalue_reference_t<std::add_const_t<T>>;

    using size_type = size_t;
    using difference_type = ptrdiff_t;

    using iterator = TSparseArrayIterator<T, _ChunkSize>;
    using const_iterator = TSparseArrayIterator<std::add_const_t<T>, _ChunkSize>;

    TBasicSparseArray() NOEXCEPT;
    ~TBasicSparseArray();

    TBasicSparseArray(const TSparseArray&) = delete; // because this container is very specific about lifetime
    TBasicSparseArray& operator =(const TSparseArray&) = delete;

    TBasicSparseArray(TBasicSparseArray&& rvalue) NOEXCEPT; // but you can still move it
    TBasicSparseArray& operator =(TBasicSparseArray&& rvalue);

    bool empty() const { return (0 == _size); }
    size_type size() const { return _size; }
    size_type capacity() const { return (ChunkSize * _numChunks); }

    iterator begin() NOEXCEPT;
    iterator end() NOEXCEPT;

    const_iterator begin() const NOEXCEPT;
    const_iterator end() const NOEXCEPT;

    const_iterator cbegin() const NOEXCEPT { return begin(); }
    const_iterator cend() const NOEXCEPT { return begin(); }

    FDataId Id(const_reference data) const;

    pointer Find(FDataId id);
    const_pointer Find(FDataId id) const {
        return const_cast<TSparseArray*>(this)->Find(id);
    }

    void Swap(TSparseArray& other) { parent_type::Swap(other); }

    bool AliasesToContainer(const_pointer p) const;
    bool AliasesToContainer(iterator it) const;
    bool AliasesToContainer(const_iterator it) const;

protected:
    friend class iterator;
    friend class const_iterator;

    STATIC_ASSERT(Meta::IsAligned(sizeof(intptr_t), sizeof(FDataChunk)));
    STATIC_CONST_INTEGRAL(u32, InvalidIndex, CODE3264(0xFFFFul, 0xFFFFFFFFul));
    STATIC_CONST_INTEGRAL(size_t, NumPtrsPerChunk, sizeof(FDataChunk) / sizeof(intptr_t));

    u32 _size;
    u32 _freeIndex;
    u32 _highestIndex;
    u16 _uniqueKey;
    u16 _numChunks;
    FDataChunk** _chunks;

    FDataItem* At_(size_t index);
    FORCE_INLINE const FDataItem* At_(size_t index) const {
        return const_cast<TSparseArray*>(this)->at_(index);
    }

    size_t GrabFirstFreeBlock_();

    struct FUnpackedId_ {
#if PLATFORM_BIGENDIAN // so when Key == 0 we can use Index as a valid size_t
        size_t Key : 16;
        size_t Index : sizeof(size_t) * 8 - 16;
#else
        size_t Index : sizeof(size_t) * 8 - 16;
        size_t Key : 16;
#endif
        bool empty() const { return (0 == Key); }
    };
    union UDataId {
        FUnpackedId_ Unpacked;
        FDataId Packed;
    };

    static FDataId PackId_(u16 key, size_t index) {
        STATIC_ASSERT(sizeof(FUnpackedId_) == sizeof(FDataId));
#ifdef WITH_PPE_ASSERT
        const UDataId id{ key, index };
        Assert(id.Packed.Key == key);
        Assert(id.Packed.Index == Index);
        return id.Packed;
#else
        return UDataId{ key, index }.Packed;
#endif
    }

    static FDataId PackId_(FUnpackedId_ unpacked) {
        UDataId id;
        id.Unpacked = unpacked;
        return id.Packed;
    }

    static FUnpackedId_ UnpackId_(FDataId packed) {
        UDataId id;
        id.Packed = packed;
        return id.Unpacked;
    }

    static bool AliasesToChunk_(const FDataChunk* chunk, const void* p) {
        return ((const void*)chunk <= p && p < (const void*)(chunk + 1));
    }
};
//----------------------------------------------------------------------------
template <typename T, size_t _ChunkSize>
class TSparseArrayIterator : public Meta::TIterator<T> {
public:
    using FSparseArray = TBasicSparseArray<T, _ChunkSize>;
    using FDataId = typename FSparseArray::FDataId;
    using FDataItem = typename FSparseArray::FDataItem;
    using FDataChunk = typename FSparseArray::FDataChunk;

    using parent_type = Meta::TIterator<T>;

    using typename parent_type::iterator_category;
    using typename parent_type::difference_type;
    using typename parent_type::value_type;
    using typename parent_type::pointer;
    using typename parent_type::reference;

    TSparseArrayIterator() NOEXCEPT
        : _index(0)
        , _owner(nullptr)
    {}

    TSparseArrayIterator(const TSparseArrayIterator&) = default;
    TSparseArrayIterator& operator =(const TSparseArrayIterator&) = default;

    TSparseArrayIterator(TSparseArrayIterator&&) = default;
    TSparseArrayIterator& operator =(TSparseArrayIterator&&) = default;

    pointer data() const {
        Assert(_chunks);
        STATIC_ASSERT(Meta::IsPow2(_ChunkSize));
        return std::addressof(_chunks[_index / _ChunkSize][_index & _ChunkSize].Data);
    }

    pointer operator->() const { return data(); }
    reference operator*() const { return (*data()); }

    TSparseArrayIterator& operator++() /* prefix */ { return GotoNextItem_(); }
    TSparseArrayIterator operator++(int) /* postfix */ {
        TSparseArrayIterator tmp(*this);
        ++(*this);
        return tmp;
    }

    void Swap(TSparseArrayIterator& other) {
        std::swap(_index, other._index);
        std::swap(_chunks, other._chunks);
    }
    inline friend void swap(TSparseArrayIterator& lhs, TSparseArrayIterator& rhs) {
        lhs.Swap(rhs);
    }

    inline friend bool operator ==(const TSparseArrayIterator& lhs, const TSparseArrayIterator& rhs) {
        return (lhs._index == rhs._index && lhs._chunks == rhs._chunks);
    }
    inline friend bool operator !=(const TSparseArrayIterator& lhs, const TSparseArrayIterator& rhs) {
        return (not operator ==(lhs, rhs));
    }

    inline friend bool operator < (const TSparseArrayIterator& lhs, const TSparseArrayIterator& rhs) {
        Assert(lhs._chunks == rhs._chunks);
        return (lhs._index < rhs._index);
    }
    inline friend bool operator >=(const TSparseArrayIterator& lhs, const TSparseArrayIterator& rhs) {
        return (not operator < (lhs, rhs));
    }

    static TSparseArrayIterator Begin(const FSparseArray& owner) {
        return TSparseArrayIterator(owner, 0).GotoFirstItem_();
    }
    static TSparseArrayIterator End(const FSparseArray& owner) {
        return TSparseArrayIterator(owner, owner._highestIndex).GotoFirstItem_();
    }

private:
    size_t _index;
    const FSparseArray* _owner;

    TSparseArrayIterator(const FSparseArray& owner, size_t index)
    :   _index(index)
    ,   _owner(&owner) {
        Assert(index <= _owner->_highestIndex);
    }

    TSparseArrayIterator& GotoFirstItem_();
    TSparseArrayIterator& GotoNextItem_();
};
//----------------------------------------------------------------------------
template <typename T, size_t _ChunkSize = 8, typename _Allocator = ALLOCATOR(Container, TSparseArrayItem<T>) >
class TSparseArray : public TBasicSparseArray<T, _ChunkSize>, private _Allocator {
public:
    using parent_type = TBasicSparseArray<T, _ChunkSize>;

    using parent_type::ChunkSize;

    using typename parent_type::FDataId;
    using typename parent_type::FDataItem;
    using typename parent_type::FDataChunk;

    using typename parent_type::value_type;
    using typename parent_type::pointer;
    using typename parent_type::const_pointer;
    using typename parent_type::reference;
    using typename parent_type::const_reference;

    using typename parent_type::size_type;
    using typename parent_type::difference_type;

    using typename parent_type::iterator;
    using typename parent_type::const_iterator;

    using allocator_type = _Allocator;
    using allocator_traits = std::allocator_traits_t<_Allocator>;

    using iterator = TSparseArrayIterator<T, _ChunkSize>;
    using const_iterator = TSparseArrayIterator<std::add_const_t<T>, _ChunkSize>;

    TSparseArray() NOEXCEPT : parent_type() {}

    explicit TSparseArray(Meta::FForceInit) noexcept
        : allocator_type(Meta::MakeForceInit<allocator_type>()) // used for non default-constructible allocators
    {}

    explicit TSparseArray(allocator_type&& alloc) : allocator_type(std::move(alloc)) {}
    explicit TSparseArray(const allocator_type& alloc) : allocator_type(alloc) {}

    TSparseArray(const TSparseArray&) = delete; // because this container is very specific about lifetime
    TSparseArray& operator =(const TSparseArray&) = delete;

    TSparseArray(TSparseArray&& rvalue) NOEXCEPT;
    TSparseArray& operator =(TSparseArray&& rvalue);

    using parent_type::empty;
    using parent_type::size;
    using parent_type::capacity;

    using parent_type::begin;
    using parent_type::end;

    using parent_type::cbegin;
    using parent_type::cend;

    using parent_type::Id;
    using parent_type::Find;
    using parent_type::AliasesToContainer;

    const allocator_type& get_allocator() const { return static_cast<const allocator_type&>(*this); }

    reference Add();

    template <typename... _Args>
    FDataId Emplace(_Args&&... args);

    bool Remove(FDataId id);
    void Remove(reference data);

    void Swap(TSparseArray& other) { parent_type::Swap(other); }
    inline friend void swap(TSparseArray& lhs, TSparseArray& rhs) {
        lhs.Swap(rhs);
    }

private:
    using parent_type::InvalidIndex;
    using parent_type::NumPtrsPerChunk;

    using parent_type::_size;
    using parent_type::_freeIndex;
    using parent_type::_highestIndex;
    using parent_type::_uniqueKey;
    using parent_type::_numChunks;
    using parent_type::_chunks;

    using parent_type::GrabFirstFreeBlock_;

    using parent_type::FUnpackedId_;
    using parent_type::PackId_;
    using parent_type::UnpackId_;
    using parent_type::At_;

    FUnpackedId_ Allocate_();
    void Release_(FUnpackedId_ id, FDataItem* it);

    void GrabNewChunk_();

    void ReleaseChunk_(FDataChunk* chunk);
    void ClearReleaseMemory_LeaveDirty_();
};
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
TBasicSparseArray<T, _ChunkSize>::~TBasicSparseArray()
{}
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
    return iterator::Begin(*this);
}
//----------------------------------------------------------------------------
template <typename T, size_t _ChunkSize>
auto TBasicSparseArray<T, _ChunkSize>::end() const NOEXCEPT -> const_iterator {
    return iterator::End(*this);
}
//----------------------------------------------------------------------------
template <typename T, size_t _ChunkSize>
auto TBasicSparseArray<T, _ChunkSize>::Id(const_reference data) const -> FDataId {
    Assert(AliasesToContainer(&data));

    auto it = reinterpret_cast<const FDataItem*>(&data);
    Assert(not UnpackId_(it->Id).empty());

    return it->Id;
}
//----------------------------------------------------------------------------
template <typename T, size_t _ChunkSize>
auto TBasicSparseArray<T, _ChunkSize>::Find(FDataId id) -> pointer {
    Assert(_numChunks);

    const FUnpackedId_ unpacked = UnpackId_(id);
    FDataItem* const it = At_(unpacked.Index);

    // checks validity of weak ref before returning data
    return (it->Id == id ? &it->Data : nullptr);
}
//----------------------------------------------------------------------------
template <typename T, size_t _ChunkSize>
void TBasicSparseArray<T, _ChunkSize>::Swap(TBasicSparseArray& other) {
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
        forrange(chunk, _chunks, _chunks + _numChunks) {
            if (AliasesToChunk_(chunk, p))
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
    FDataChunk* const chunk = (_numChunks == 1
        ? reinterpret_cast<FDataChunk*>(_chunks)
        : _chunks[index / ChunkSize]);

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
    _freeIndex = head->Id; // allow to treat Id as a valid size_t

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
            not FSparseArray::UnpackId_(_owner->At_(_index).Id).empty())
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
FDataId TSparseArray<T, _ChunkSize, _Allocator>::Emplace(_Args&&... args) {
    const FUnpackedId_ unpacked = Allocate_();
    Assert(not unpacked.empty());

    FDataItem* const it = At_(unpacked.Index);
    it->Id = PackId_(unpacked);
    allocator_traits::construct(get_allocator_(), &it->Data, std::forward<_Args>(args)...);

    return it->Id;
}
//----------------------------------------------------------------------------
template <typename T, size_t _ChunkSize, typename _Allocator>
void TSparseArray<T, _ChunkSize, _Allocator>::Remove(FDataId id) {
    const FUnpackedId_ unpacked = UnpackId_(id);
    Assert(not unpacked.empty());

    FDataItem* const it = At_(unpacked.Index);
    if (it->Id != id)
        return false; // weak ref is invalid, can't remove

    allocator_traits::destroy(get_allocator_(), &it->Data);

    Release_(unpacked, it);
}
//----------------------------------------------------------------------------
template <typename T, size_t _ChunkSize, typename _Allocator>
void TSparseArray<T, _ChunkSize, _Allocator>::Remove(reference data) {
    Assert(AliasesToContainer(&data));

    auto it = reinterpret_cast<FDataItem*>(&data);

    const FUnpackedId_ unpacked = UnpackId_(it->Id);
    Assert(not unpacked.empty());
    Assert(unpacked.Index < _highestIndex);

    allocator_traits::destroy(get_allocator_(), &it->Data);

    Release_(unpacked, it);
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
        _freeIndex = oldIndex;
    }
    Assert(UnpackId_(it->Id).empty());

    --_size; // won't ever shrink allocation
}
//----------------------------------------------------------------------------
template <typename T, size_t _ChunkSize, typename _Allocator>
NO_INLINE void TSparseArray<T, _ChunkSize, _Allocator>::GrabNewChunk_() {
    Assert_NoAssume(SafeAllocatorSnapSize(_ChunkSize) == _ChunkSize); // checks that we're not wasting memory

    pointer const newChunk = allocator_traits::allocate(get_allocator_(), _ChunkSize);

    if (Likely(0 == _numChunks)) {
        Assert(nullptr == _chunks);

        _numChunks++; // don't allocate for the first chunk, pack ptr* in ptr**
        _chunks = (pointer*)newChunk;

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

        _chunks = Relocate_AssumePod( // assume POD to do a simple memcpy (not used a FDataItem*)
            get_allocator_(),
            aliasedChunks,
            aliasedChunks.size() + _ChunkSize, // a new FDataChunk worth of free space
            aliasedChunks.size() );
    }

    _chunks[_numChunks++] = newChunk;
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

    allocator_traits::deallocate(al, chunk, _ChunkSize);
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

        ReleaseChunk_(reinterpret_cast<FDataChunk*>(_chunks))
    }
    else { // using an additional allocation for storing pointer
        Assert(_ChunkSize < _highestIndex);
        Assert(_chunks);

        forrange(chunk, _chunks, _chunks + _numChunks)
            ReleaseChunk_(chunk);

        // release the extra allocation made for storage
        const size_t numChunksReserved = Meta::RoundToNext(_numChunks, NumPtrsPerChunk);
        const TMemoryView<FDataChunk*> storage(_chunks, numChunksReserved);
        const TMemoryView<FDataItem> aliased = oldChunks.Cast<FDataItem>();

        allocator_traits::deallocate(get_allocator_(), aliased.data(), aliased.size());
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
