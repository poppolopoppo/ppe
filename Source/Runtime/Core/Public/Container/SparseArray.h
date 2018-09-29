#pragma once

#include "Core.h"

#include "Allocator/Allocation.h"
#include "Allocator/MallocBinned.h" // SnapSize()
#include "HAL/PlatformMemory.h" // Memswap
#include "Memory/MemoryView.h"
#include "Meta/Iterator.h"

// Inspired from DataArray<T>
// http://greysphere.tumblr.com/post/31601463396/data-arrays

#define SPARSEARRAY(_DOMAIN, T, _ChunkSize) \
    ::PPE::TAlignedSparseArray<COMMA_PROTECT(T), _ChunkSize, \
        ALLOCATOR(_DOMAIN, ::PPE::TSparseArrayItem<COMMA_PROTECT(T)>) >

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
    // Choose this parameter wisely :
    // - too high and you would waste lot of memory (can't shrink) ;
    // - too low and you would allocate very often.
    // The value is specified as a template parameter to adapt to your specific usage
    STATIC_CONST_INTEGRAL(size_t, ChunkSize, _ChunkSize);

    using FDataId = FSparseDataId;
    using FDataItem = TSparseArrayItem<T>;
    using FDataChunk = Meta::TArray<FDataItem, _ChunkSize>;

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

    TBasicSparseArray(const TBasicSparseArray&) = delete; // because this container is very specific about lifetime
    TBasicSparseArray& operator =(const TBasicSparseArray&) = delete;

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
    const_iterator cend() const NOEXCEPT { return end(); }

    FDataId IndexOf(const_reference data) const;

    pointer Find(FDataId id);
    const_pointer Find(FDataId id) const {
        return const_cast<TBasicSparseArray*>(this)->Find(id);
    }

    void Swap(TBasicSparseArray& other);

    bool CheckInvariants() const;

    bool AliasesToContainer(const_pointer p) const;
    bool AliasesToContainer(iterator it) const;
    bool AliasesToContainer(const_iterator it) const;

    static size_t Nth(FDataId id) {
        return UnpackId_(id).Index;
    }

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
        return const_cast<TBasicSparseArray*>(this)->At_(index);
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
        Assert(id.Packed.Index == index);
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
    template <typename U, size_t _Sz>
    friend class TSparseArrayIterator;

public:
    using FSparseArray = TBasicSparseArray<Meta::TRemoveConst<T>, _ChunkSize>;
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

    template <typename U>
    TSparseArrayIterator(const TSparseArrayIterator<U, _ChunkSize>& other) {
        operator =(other);
    }
    template <typename U>
    TSparseArrayIterator& operator =(const TSparseArrayIterator<U, _ChunkSize>& other) {
        _index = other._index;
        _owner = other._owner;
        return (*this);
    }

    pointer data() const {
        Assert(_owner);
        return std::addressof(_owner->At_(_index)->Data);
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
        std::swap(_owner, other._owner);
    }
    inline friend void swap(TSparseArrayIterator& lhs, TSparseArrayIterator& rhs) {
        lhs.Swap(rhs);
    }

    static TSparseArrayIterator Begin(const FSparseArray& owner) {
        return (not owner.empty() // skip linear search when empty
            ? TSparseArrayIterator(owner, 0).GotoFirstItem_()
            : End(owner) );
    }
    static TSparseArrayIterator End(const FSparseArray& owner) {
        return TSparseArrayIterator(owner, owner._highestIndex);
    }

    inline friend bool operator ==(const TSparseArrayIterator& lhs, const TSparseArrayIterator& rhs) {
        Assert(lhs._owner == rhs._owner);
        return (lhs._index == rhs._index);
    }
    inline friend bool operator !=(const TSparseArrayIterator& lhs, const TSparseArrayIterator& rhs) {
        return (not operator ==(lhs, rhs));
    }

    inline friend bool operator < (const TSparseArrayIterator& lhs, const TSparseArrayIterator& rhs) {
        Assert(lhs._owner == rhs._owner);
        return (lhs._index < rhs._index);
    }
    inline friend bool operator >=(const TSparseArrayIterator& lhs, const TSparseArrayIterator& rhs) {
        return (not operator < (lhs, rhs));
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
    using allocator_traits = std::allocator_traits<_Allocator>;

    static_assert(std::is_same<FDataItem, typename allocator_traits::value_type>::value,
        "allocator value_type mismatch");

    TSparseArray() NOEXCEPT : parent_type() {}
    ~TSparseArray();

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

    using parent_type::IndexOf;
    using parent_type::Find;

    using parent_type::CheckInvariants;
    using parent_type::AliasesToContainer;

    const allocator_type& get_allocator() const {
        return static_cast<const allocator_type&>(*this);
    }

    reference Add();

    template <typename... _Args>
    FDataId Emplace(_Args&&... args);

    bool Remove(FDataId id);
    void Remove(iterator it);
    void Remove(reference data);

    void Clear(); // won't release memory

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

    using typename parent_type::FUnpackedId_;
    using parent_type::PackId_;
    using parent_type::UnpackId_;
    using parent_type::At_;

    FUnpackedId_ Allocate_();
    void Release_(FUnpackedId_ id, FDataItem* it);

    void GrabNewChunk_();

    void ReleaseChunk_(FDataChunk* chunk, size_t offset);
    void ClearReleaseMemory_LeaveDirty_();

    allocator_type& get_allocator_() {
        return static_cast<allocator_type&>(*this);
    }
};
//----------------------------------------------------------------------------
template <typename T, size_t _ChunkSize, typename _Allocator>
using TAlignedSparseArray = TSparseArray<T,
    FMallocBinned::SnapSizeConstexpr(
        _ChunkSize * sizeof(TSparseArrayItem<T>)) /
        sizeof(TSparseArrayItem<T>),
    _Allocator
>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#include "Container/SparseArray-inl.h"
