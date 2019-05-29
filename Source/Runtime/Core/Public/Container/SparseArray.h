#pragma once

#include "Core.h"

#include "Allocator/Allocation.h"
#include "Allocator/MallocBinned.h" // SnapSize()
#include "HAL/PlatformMemory.h" // Memswap
#include "Memory/MemoryView.h"
#include "Meta/Iterator.h"
#include "Meta/PointerWFlags.h"

// Inspired from DataArray<T>
// http://greysphere.tumblr.com/post/31601463396/data-arrays

#define SPARSEARRAY_INLINE_ALLOCATOR(_Domain, T, N) \
    ::PPE::TSparseArrayInlineAllocator< MEMORYDOMAIN_TAG(_Domain), T, N >

#define SPARSEARRAY(_DOMAIN, T) \
    ::PPE::TSparseArray< COMMA_PROTECT(T), ALLOCATOR(_DOMAIN) >
#define SPARSEARRAY_INSITU(_DOMAIN, T, N) \
    ::PPE::TSparseArray< COMMA_PROTECT(T), SPARSEARRAY_INLINE_ALLOCATOR(_DOMAIN, COMMA_PROTECT(T), N) >

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Important properties :
// - act as a keyed container
// - dynamic growth
// - won't invalidate it's storage validity
// - an internal free list is used to get an available slot
// - uses more than exponential growth : s1 = s0 * 3
//----------------------------------------------------------------------------
using FSparseDataId = size_t;
//----------------------------------------------------------------------------
template <typename T>
struct TSparseArrayItem {
    T Data;
    FSparseDataId Id;
};
//----------------------------------------------------------------------------
template <typename T>
class TSparseArrayIterator;
//----------------------------------------------------------------------------
template <typename T>
class TBasicSparseArray {
public:
    using FDataId = FSparseDataId;
    using FDataItem = TSparseArrayItem<T>;
    using FDataChunkRef = Meta::TPointerWFlags<FDataItem>;

    using value_type = T;
    using pointer = Meta::TAddPointer<T>;
    using const_pointer = Meta::TAddPointer<Meta::TAddConst<T>>;
    using reference = Meta::TAddReference<T>;
    using const_reference = Meta::TAddReference<Meta::TAddConst<T>>;

    using size_type = size_t;
    using difference_type = ptrdiff_t;

    using iterator = TSparseArrayIterator<T>;
    using const_iterator = TSparseArrayIterator<Meta::TAddConst<T>>;

    TBasicSparseArray() NOEXCEPT;
    ~TBasicSparseArray();

    TBasicSparseArray(const TBasicSparseArray&) = delete; // because this container is very specific about lifetime
    TBasicSparseArray& operator =(const TBasicSparseArray&) = delete;

    TBasicSparseArray(TBasicSparseArray&& rvalue) NOEXCEPT; // but you can still move it
    TBasicSparseArray& operator =(TBasicSparseArray&& rvalue);

    bool empty() const { return (0 == _size); }
    size_type size() const { return _size; }
    size_type capacity() const { return ckOffset_(_numChunks); }

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
    friend class TSparseArrayIterator<T>;
    friend class TSparseArrayIterator<Meta::TAddConst<T>>;

    STATIC_CONST_INTEGRAL(u32, InvalidIndex, CODE3264(0xFFFFul, 0xFFFFFFFFul));

    u32 _size;
    u32 _freeIndex;
    u32 _highestIndex;
    u16 _uniqueKey;
    u16 _numChunks;
    FDataChunkRef* _chunks;

    FDataItem* At_(size_t index);
    FORCE_INLINE const FDataItem* At_(size_t index) const {
        return const_cast<TBasicSparseArray*>(this)->At_(index);
    }

    bool AliasesToChunk_(const FDataItem* chunk, size_t cls, const void* ptr) const;
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

    // each time a sparse must grow it allocate a new chunk twice bigger than previous one
    STATIC_CONST_INTEGRAL(size_t, MinChunkSize_, 8);
    STATIC_CONST_INTEGRAL(size_t, MinChunkExp_, 3); // first chunk is always 8 elements (2^3)
    static size_t ckIndex_(size_t i) NOEXCEPT {
        return (FPlatformMaths::CeilLog2(Meta::RoundToNext(i | 1, MinChunkSize_)) - MinChunkExp_);
    }
    static size_t ckSize_(size_t o) NOEXCEPT {
        return (size_t(1) << (MinChunkExp_ + o));
    }
    static size_t ckOffset_(size_t o) NOEXCEPT {
        return (o ? ckSize_(o - 1) : 0);
    }
    static size_t ckAllocation_(size_t o) NOEXCEPT {
        return (ckSize_(o) - ckOffset_(o));
    }
};
//----------------------------------------------------------------------------
template <typename T>
class TSparseArrayIterator : public Meta::TIterator<T, std::input_iterator_tag> {
    using parent_type = Meta::TIterator<T, std::input_iterator_tag>;

    template <typename U>
    friend class TSparseArrayIterator;

public:
    using FSparseArray = TBasicSparseArray<Meta::TRemoveConst<T>>;
    using FDataId = typename FSparseArray::FDataId;
    using FDataItem = typename FSparseArray::FDataItem;

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
    TSparseArrayIterator(const TSparseArrayIterator<U>& other) {
        operator =(other);
    }
    template <typename U>
    TSparseArrayIterator& operator =(const TSparseArrayIterator<U>& other) {
        _index = other._index;
        _owner = other._owner;
        return (*this);
    }

    size_t Index() const { return _index; }
    const FSparseArray* Owner() const { return _owner; }

    pointer data() const {
        Assert(_owner);
        return const_cast<pointer>(// <== better than duping the code
            std::addressof(_owner->At_(_index)->Data) );
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

    template <typename U>
    inline friend bool operator ==(const TSparseArrayIterator& lhs, const TSparseArrayIterator<U>& rhs) {
        Assert_NoAssume(lhs.Owner() == rhs.Owner());
        return (lhs.Index() == rhs.Index());
    }
    template <typename U>
    inline friend bool operator !=(const TSparseArrayIterator& lhs, const TSparseArrayIterator<U>& rhs) {
        return (not operator ==(lhs, rhs));
    }

    template <typename U>
    inline friend bool operator < (const TSparseArrayIterator& lhs, const TSparseArrayIterator<U>& rhs) {
        Assert_NoAssume(lhs.Owner() == rhs.Owner());
        return (lhs.Index() < rhs.Index());
    }
    template <typename U>
    inline friend bool operator >=(const TSparseArrayIterator& lhs, const TSparseArrayIterator<U>& rhs) {
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
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Uses an in situ only one allocation of N blocks, then uses malloc()
//----------------------------------------------------------------------------
namespace details {
template <typename _Tag, typename T, size_t N>
struct TCheckedSparseArrayInlineAllocator_ {
    STATIC_ASSERT(Meta::IsPow2(N));
    STATIC_ASSERT(N >= 8); // this is the minimum allocation size by default TSparseArray
    using type = TSegregatorAllocator<
        sizeof(TSparseArrayItem<T>)* N,
        TInSituAllocator<sizeof(TSparseArrayItem<T>)* N>,
        TDefaultAllocator<_Tag> >;
};
} //!details
//----------------------------------------------------------------------------
#if USE_PPE_MEMORY_DEBUGGING
template <typename _Tag, typename T, size_t N>
using TSparseArrayInlineAllocator = TDefaultAllocator<_Tag>;
#else
template <typename _Tag, typename T, size_t N>
using TSparseArrayInlineAllocator = typename details::TCheckedSparseArrayInlineAllocator_<_Tag, T, N>::type;
#endif //!USE_PPE_MEMORY_DEBUGGING
//----------------------------------------------------------------------------
template <typename T, typename _Allocator = ALLOCATOR(Container) >
class TSparseArray : private _Allocator, public TBasicSparseArray<T> {
public:
    using parent_type = TBasicSparseArray<T>;

    using typename parent_type::FDataId;
    using typename parent_type::FDataItem;
    using typename parent_type::FDataChunkRef;

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
    using allocator_traits = TAllocatorTraits<_Allocator>;

    TSparseArray() NOEXCEPT : parent_type() {}
    ~TSparseArray();

    explicit TSparseArray(Meta::FForceInit) noexcept
        : allocator_type(Meta::MakeForceInit<allocator_type>()) // used for non default-constructible allocators
    {}

    explicit TSparseArray(allocator_type&& alloc) : allocator_type(std::move(alloc)) {}
    explicit TSparseArray(const allocator_type& alloc) : allocator_type(alloc) {}

    // #TODO not sure if it's really relevant to copy this container, particularly with the way it's implemented atm
    TSparseArray(const TSparseArray&);
    TSparseArray& operator =(const TSparseArray&);

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

    template <typename _It>
    void AddRange(_It first, _It last);

    template <typename... _Args>
    FDataId Emplace(_Args&&... args);

    bool Remove(FDataId id);
    void Remove(iterator it);
    void Remove(reference data);

    void Clear(); // won't release memory
    void Clear_ReleaseMemory();
    void Reserve(size_t n);

    void Swap(TSparseArray& other) { parent_type::Swap(other); }
    inline friend void swap(TSparseArray& lhs, TSparseArray& rhs) {
        lhs.Swap(rhs);
    }

private:
    using parent_type::InvalidIndex;

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

    using parent_type::ckIndex_;
    using parent_type::ckSize_;
    using parent_type::ckOffset_;
    using parent_type::ckAllocation_;

    FUnpackedId_ AllocateItem_();
    void ReleaseItem_(FUnpackedId_ id, FDataItem* it);

    void GrabNewChunk_(size_t cls);
    void ReleaseChunk_(FDataItem* chunk, size_t off, size_t sz);

    void ClearReleaseMemory_LeaveDirty_();

    template <typename _It>
    void AddRange_(_It first, _It last, std::input_iterator_tag);
    template <typename _It, typename _IteratorTag>
    void AddRange_(_It first, _It last, _IteratorTag);

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#include "Container/SparseArray-inl.h"

//----------------------------------------------------------------------------
// Use TSparseArray<T> as an in-place allocator :
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
inline void* operator new(size_t sizeInBytes, PPE::TSparseArray<T, _Allocator>& arr) {
    Assert(sizeInBytes == sizeof(T));
    void* const p = &arr.Add();
    Assert(Meta::IsAligned(std::alignment_of_v<T>, p));
    return p;
}
template <typename T, typename _Allocator>
inline void operator delete(void* ptr, PPE::TSparseArray<T, _Allocator>& arr) {
    Assert_NoAssume(arr.AliasesToContainer(static_cast<T*>(ptr)));
    AssertNotImplemented(); // can't move elements around the sparse array
}
