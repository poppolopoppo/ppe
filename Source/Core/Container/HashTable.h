#pragma once

#include "Core/Core.h"

#include "Core/Allocator/Allocation.h"
#include "Core/Allocator/InSituAllocator.h"
#include "Core/Container/Hash.h"
#include "Core/Container/Pair.h"

#include <algorithm>
#include <initializer_list>
#include <iterator>
#include <utility>
#include <type_traits>

// https://gist.github.com/ssylvan/5538011

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define HASHTABLE(_DOMAIN, _KEY, _VALUE) \
    ::Core::details::HashTable_<_KEY, _VALUE, ::Core::Hash<_KEY>, ::Core::Meta::EqualTo<_KEY>, ALLOCATOR(_DOMAIN, ::Core::Pair<_KEY COMMA _VALUE>)>
//----------------------------------------------------------------------------
#define HASHTABLE_THREAD_LOCAL(_DOMAIN, _KEY, _VALUE) \
    ::Core::details::HashTable_<_KEY, _VALUE, ::Core::Hash<_KEY>, ::Core::Meta::EqualTo<_KEY>, THREAD_LOCAL_ALLOCATOR(_DOMAIN, ::Core::Pair<_KEY COMMA _VALUE>)>
//----------------------------------------------------------------------------
#define HASHTABLE_SET(_DOMAIN, _KEY) HASHTABLE(_DOMAIN, _KEY, void)
#define HASHTABLE_SET_THREAD_LOCAL(_DOMAIN, _KEY, _VALUE) HASHTABLE_THREAD_LOCAL(_DOMAIN, _KEY, void)
//----------------------------------------------------------------------------
#define HASHTABLE_MAP(_DOMAIN, _KEY, _VALUE) HASHTABLE(_DOMAIN, _KEY, _VALUE)
#define HASHTABLE_MAP_THREAD_LOCAL(_DOMAIN, _KEY, _VALUE) HASHTABLE_THREAD_LOCAL(_DOMAIN, _KEY, _VALUE)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hash, typename _Equal, typename _Allocator>
class HashTable;
//---------------------------------------------------------------------------
struct HashTableStats {
    size_t MinProbe;
    size_t MaxProbe;
    double MeanProbe;
    double DevProbe;
};
//---------------------------------------------------------------------------
namespace details {
template <typename _Key, typename _Value>
struct HashTableTraits_ {
    typedef Pair<_Key, _Value> value_type;
    typedef Pair<typename std::add_const<_Key>::type, _Value> public_type;
    typedef _Value& mapped_reference;
    typedef const _Value& mapped_const_reference;
    static const _Key& Key(const _Key& key) { return key; }
    static const _Key& Key(const value_type& value) { return value.first; }
    static mapped_reference Value(value_type& value) { return value.second; }
    static mapped_reference Value(public_type& value) { return value.second; }
    static mapped_const_reference Value(const value_type& value) { return value.second; }
    static mapped_const_reference Value(const public_type& value) { return value.second; }
    static value_type Make(const _Key& key) { return value_type(key, _Value()); }
    static value_type Make(_Key&& rkey) { return value_type(std::move(rkey), _Value()); }
    static value_type Make(const _Key& key, const _Value& value) { return value_type(key, value); }
    static value_type Make(const _Key& key, _Value&& rvalue) { return value_type(key, std::move(rvalue)); }
    static value_type Make(_Key&& rkey, _Value&& rvalue) { return value_type(std::move(rkey), std::move(rvalue)); }
};
template <typename _Key>
struct HashTableTraits_<_Key, void> {
    typedef _Key value_type;
    typedef typename std::add_const<_Key>::type public_type;
    typedef const _Key& mapped_reference;
    typedef const _Key& mapped_const_reference;
    static const _Key& Key(const value_type& value) { return value; }
    static mapped_reference Value(value_type& value) { return value; }
    static mapped_const_reference Value(const value_type& value) { return value; }
    template <typename... _Args>
    static value_type Make(_Args&&... args) { return value_type(std::forward<_Args>(args)...); }
};
struct HashValueWIndex32_ {
    typedef u16 size_type;
    u16 hash_value;
    u16 data_index;
    bool empty() const { return (0 == *(const u32*)(this)); }
    bool deleted() const { return (u16(-1) == data_index); }
    void MarkAsDeleted() { data_index = u16(-1); }
    static HashValueWIndex32_ Zero() { return HashValueWIndex32_{0,0}; }
    static HashValueWIndex32_ Make(size_t hashValue, size_t dataIndex) {
        return HashValueWIndex32_{ u16(hashValue), checked_cast<u16>(dataIndex)};
    }
};
struct HashValueWIndex64_ {
    typedef u32 size_type;
    u32 hash_value;
    u32 data_index;
    bool empty() const { return (0 == *(const u64*)(this)); }
    bool deleted() const { return (u32(-1) == data_index); }
    void MarkAsDeleted() { data_index = u32(-1); }
    static HashValueWIndex64_ Zero() { return HashValueWIndex64_{0,0}; }
    static HashValueWIndex64_ Make(size_t hashValue, size_t dataIndex) {
        return HashValueWIndex64_{ u32(hashValue), checked_cast<u32>(dataIndex)};
    }
};
static constexpr size_t HashTableCapacityForHash_[32] = {
   0x00000000,0x00000003,0x0000000b,0x00000017,0x00000035,0x00000061,0x000000c1,0x00000185,
   0x00000301,0x00000607,0x00000c07,0x00001807,0x00003001,0x00006011,0x0000c005,0x0001800d,
   0x00030005,0x00060019,0x000c0001,0x00180005,0x0030000b,0x0060000d,0x00c00005,0x01800013,
   0x03000005,0x06000017,0x0c000013,0x18000005,0x30000059,0x60000005,0xc0000001,0xfffffffb,
};
static constexpr size_t HashTableCapacityForValue_[32] = {
   0x00000000,0x00000003,0x00000009,0x00000012,0x00000029,0x0000004b,0x00000095,0x0000012c,
   0x00000250,0x000004a4,0x00000943,0x00001281,0x000024f7,0x000049f9,0x000093db,0x000127b8,
   0x00024f60,0x00049ecc,0x00093d72,0x00127ae5,0x0024f5cb,0x0049eb8f,0x0093d70e,0x0127ae23,
   0x024f5c2d,0x049eb864,0x093d70b3,0x127ae14c,0x24f5c2d4,0x49eb8523,0x93d70a3e,0xc51eb84e,
};
struct HashTableProbe_ {
    static constexpr size_t ShiftSize = 5;
    static constexpr size_t MaskCapacityIndex = (1ul<<ShiftSize)-1;
    static constexpr size_t MaskSize = ~MaskCapacityIndex;
    static constexpr size_t MaxCapacityIndex = MaskCapacityIndex;
    HashTableProbe_(size_t size, size_t capacityIndex, void* hashIndicesRaw, bool useHashIndices64)
        : Size(size)
        , HashCapacity(HashTableCapacityForHash_[capacityIndex])
        , ValuesCapacity(HashTableCapacityForValue_[capacityIndex])
        , HashIndicesRaw(hashIndicesRaw)
        , UseHashIndices64(useHashIndices64) {
        STATIC_ASSERT(MaxCapacityIndex+1 == lengthof(HashTableCapacityForHash_));
        STATIC_ASSERT(sizeof(HashTableCapacityForHash_) == sizeof(HashTableCapacityForValue_));
        Assert(capacityIndex <= MaxCapacityIndex);
        Assert(0 == ValuesCapacity || HashIndicesRaw);
        Assert(Size <= ValuesCapacity);
        Assert(ValuesCapacity <= HashCapacity);
        Assert(IS_ALIGNED(sizeof(size_t), HashIndicesRaw));
    }
    size_t Size;
    size_t HashCapacity;
    size_t ValuesCapacity;
    void* HashIndicesRaw;
    bool UseHashIndices64;
    size_t DesiredPos(size_t hashValue) const { return (hashValue % HashCapacity); }
    size_t DesiredInc(size_t hashValue) const { return (1 + hashValue % (HashCapacity - 1)); }
    size_t NextBucket(size_t bucket, size_t inc) const { return (bucket + inc >= HashCapacity ? (bucket + inc) - HashCapacity : bucket + inc); }
    //size_t ProbeDistance(size_t hashValue, size_t bucket) const { return (((bucket + HashCapacity) - DesiredPos(hashValue)) % HashCapacity); }
    FORCE_INLINE size_t ProbeDistance(size_t hashValue, size_t bucket) const {
        size_t b = DesiredPos(hashValue);
        size_t inc = DesiredInc(hashValue);
        size_t distance = 0;
        for (; bucket != b; b = NextBucket(b, inc), ++distance);
        return distance;
    }
    MemoryView<HashValueWIndex32_> HashIndices32() const { Assert(not UseHashIndices64); return MemoryView<HashValueWIndex32_>(reinterpret_cast<HashValueWIndex32_*>(HashIndicesRaw), HashCapacity); }
    MemoryView<HashValueWIndex64_> HashIndices64() const { Assert(UseHashIndices64); return MemoryView<HashValueWIndex64_>(reinterpret_cast<HashValueWIndex64_*>(HashIndicesRaw), HashCapacity); }
    void EraseBucket(size_t bucket, size_t hashValue) const;
    void SwapDataIndex(size_t bucket0, size_t bucket1) const;
    void ClearBuckets() const;
    HashTableStats ProbingStats() const;
};
template <typename _Key, typename _Value, typename _Hash, typename _Equal, typename _Allocator>
using HashTable_ = HashTable<_Key, _Value, _Hash, _Equal,
    typename _Allocator::template rebind< typename HashTableTraits_<_Key, _Value>::type >::type >;
} //!details
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
class HashTableBase {
public:
    typedef _Key key_type;
    typedef _Value mapped_type;

    typedef details::HashTableTraits_<_Key, _Value> table_traits;
    typedef typename table_traits::value_type value_type;
    typedef typename table_traits::public_type public_type;
    typedef typename table_traits::mapped_reference mapped_reference;
    typedef typename table_traits::mapped_const_reference mapped_const_reference;

    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef typename std::add_pointer<value_type>::type pointer;
    typedef typename std::add_pointer<const value_type>::type const_pointer;

    typedef typename MemoryView<public_type>::iterator iterator;
    typedef typename MemoryView<const value_type>::iterator const_iterator;
    typedef std::random_access_iterator_tag iterator_category;

    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

    size_type size() const { return checked_cast<size_type>(_size_capacityIndex>>probe_type::ShiftSize); }
    bool empty() const { return (0 == (_size_capacityIndex&probe_type::MaskSize)); }
    size_type capacity() const { return details::HashTableCapacityForValue_[_size_capacityIndex&probe_type::MaskCapacityIndex]; }

    size_type bucket_count() const { return details::HashTableCapacityForHash_[_size_capacityIndex&probe_type::MaskCapacityIndex]; }
    size_type max_bucket_count() const { return details::HashTableCapacityForHash_[probe_type::MaxCapacityIndex]; }

    iterator begin() { return MakeView().begin(); }
    iterator end() { return MakeView().end(); }

    const_iterator begin() const { return MakeView().begin(); }
    const_iterator end() const { return MakeView().end(); }

    const_iterator cbegin() const { return MakeView().begin(); }
    const_iterator cend() const { return MakeView().end(); }

    reverse_iterator rbegin() { return reverse_iterator(end()); }
    reverse_iterator rend() { return reverse_iterator(begin()); }

    const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
    const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }

    const_reverse_iterator crbegin() const { return const_reverse_iterator(end()); }
    const_reverse_iterator crend() const { return const_reverse_iterator(begin()); }

    iterator begin(size_type pos) { Assert(pos < size()); return begin() + pos; }
    const_iterator begin(size_type pos) const { Assert(pos < size()); return begin() + pos; }
    const_iterator cbegin(size_type pos) const { Assert(pos < size()); return cbegin() + pos; }

    MemoryView<public_type> MakeView() { return MemoryView<public_type>(reinterpret_cast<public_type*>(_values_hashIndices), size()); }
    MemoryView<const value_type> MakeView() const { return MemoryView<const value_type>(_values_hashIndices, size()); }

    float load_factor() const { return size() * 1.0f / bucket_count(); }

protected:
    typedef details::HashTableProbe_ probe_type;

    HashTableBase() noexcept : _size_capacityIndex(0), _values_hashIndices(nullptr) {}

    size_t GrowIFN_ReturnAllocationCount_(size_type atleast);
    size_t ShrinkToFitIFN_ReturnAllocationCount_(size_type atleast);

    size_type DecSize_(size_type count = 1) {
        Assert((_size_capacityLog2Plus1>>ShiftSize) >= count);
        const size_type newSize = (_size_capacityIndex>>probe_type::ShiftSize) - count;
        SetSize_(newSize);
        return newSize;
    }
    size_type IncSize_(size_type count = 1) {
        const size_type newSize = (_size_capacityIndex>>probe_type::ShiftSize) + count;
        SetSize_(newSize);
        return newSize;
    }

    void SetSize_(size_type newSize = 1) {
        Assert(capacity() >= newSize);
        Assert(((newSize<<probe_type::ShiftSize)>>probe_type::ShiftSize) == newSize);
        _size_capacityIndex = (_size_capacityIndex&probe_type::MaskCapacityIndex)|(newSize<<probe_type::ShiftSize);
    }

    static bool UseHashIndices64Helper_(size_t capacityForValues) { return (capacityForValues>UINT16_MAX); }

    bool UseHashIndices64_() const { return UseHashIndices64Helper_(capacity()); }

#ifdef ARCH_X64
    STATIC_ASSERT(8 == sizeof(size_type));
    static void* HashIndicesRaw_(const void* data, size_type capacityForValues) {
        return (void*)ROUND_TO_NEXT_8((capacityForValues*sizeof(value_type))+size_type(data));
    }
#else
    STATIC_ASSERT(4 == sizeof(size_type));
    static void* HashIndicesRaw_(const void* data, size_type capacityForValues) {
        return (void*)ROUND_TO_NEXT_4((capacityForValues*sizeof(value_type))+size_type(data));
    }
#endif

    static size_type AllocationCountWIndicesFor_(size_type capacityIndex) {
        Assert(capacityIndex <= probe_type::MaxCapacityIndex);
        const size_type hashesCount = details::HashTableCapacityForHash_[capacityIndex];
        const size_type valuesCount = details::HashTableCapacityForValue_[capacityIndex];
        const size_type indexsize = UseHashIndices64Helper_(valuesCount)
            ? sizeof(details::HashValueWIndex64_)
            : sizeof(details::HashValueWIndex32_);
        const size_type ioffset = (size_type)HashIndicesRaw_(nullptr, valuesCount);
        const size_type iend = ioffset + indexsize*hashesCount;
        return (iend+sizeof(value_type)-1)/sizeof(value_type);
    }

    details::HashTableProbe_ MakeProbe_() const {
        return details::HashTableProbe_(
            (_size_capacityIndex>>probe_type::ShiftSize),
            (_size_capacityIndex&probe_type::MaskCapacityIndex),
            HashIndicesRaw_(_values_hashIndices, capacity()),
            UseHashIndices64_() );
    }

    bool AliasesToContainer_(const details::HashTableProbe_& probe) const {
        Assert(size() == probe.Size);
        Assert(capacity() == probe.ValuesCapacity);
        Assert((void*)_values_hashIndices <= (void*)HashIndicesRaw_(_values_hashIndices, capacity()));
        const MemoryView<const value_type> this_data_windices(_values_hashIndices, AllocationCountWIndicesFor_(_size_capacityIndex&probe_type::MaskCapacityIndex));
        return (probe.UseHashIndices64
            ? probe.HashIndices64().IsSubRangeOf(this_data_windices)
            : probe.HashIndices32().IsSubRangeOf(this_data_windices) );
    }

    size_type _size_capacityIndex;
    pointer _values_hashIndices;
};
//----------------------------------------------------------------------------
template <
    typename _Key
,   typename _Value
,   typename _Hash = Hash<_Key>
,   typename _Equal = Meta::EqualTo<_Key>
,   typename _Allocator = ALLOCATOR(Container, typename details::HashTableTraits_<_Key COMMA _Value>::value_type)
>   class HashTable : public HashTableBase<_Key, _Value>, _Allocator {
public:
    typedef _Hash hasher;
    typedef _Equal key_equal;

    typedef _Allocator allocator_type;
    typedef std::allocator_traits<allocator_type> allocator_traits;

    typedef HashTableBase<_Key, _Value> base_type;

    using typename base_type::table_traits;

    using typename base_type::key_type;
    using typename base_type::value_type;
    using typename base_type::reference;
    using typename base_type::const_reference;
    using typename base_type::pointer;
    using typename base_type::const_pointer;

    using typename base_type::size_type;
    using typename base_type::difference_type;

    using typename base_type::public_type;
    using typename base_type::mapped_reference;
    using typename base_type::mapped_const_reference;

    using typename base_type::iterator;
    using typename base_type::const_iterator;
    using typename base_type::reverse_iterator;
    using typename base_type::const_reverse_iterator;

    using base_type::size;
    using base_type::empty;
    using base_type::capacity;

    using base_type::begin;
    using base_type::end;

    using base_type::cbegin;
    using base_type::cend;

    using base_type::MakeView;

    STATIC_ASSERT(std::is_same<typename allocator_traits::pointer, pointer>::value);
    STATIC_ASSERT(std::is_same<typename allocator_traits::const_pointer, const_pointer>::value);

    STATIC_ASSERT(std::is_same<typename allocator_traits::size_type, size_type>::value);
    STATIC_ASSERT(std::is_same<typename allocator_traits::difference_type, difference_type>::value);

    HashTable() noexcept {}
    ~HashTable() { Assert(CheckInvariants()); clear_ReleaseMemory(); }

    explicit HashTable(allocator_type&& alloc) : allocator_type(std::move(alloc)) {}
    explicit HashTable(const allocator_type& alloc) : allocator_type(alloc) {}

    explicit HashTable(size_type capacity) : HashTable() { reserve_AssumeEmpty(capacity); }
    HashTable(size_type capacity, const allocator_type& alloc) : HashTable(alloc) { reserve_AssumeEmpty(capacity); }

    HashTable(const HashTable& other) : HashTable(allocator_traits::select_on_container_copy_construction(other)) { assign(other.begin(), other.end()); }
    HashTable(const HashTable& other, const allocator_type& alloc) : HashTable(alloc) { assign(other.begin(), other.end()); }
    HashTable& operator=(const HashTable& other);

    HashTable(HashTable&& rvalue) noexcept : HashTable(static_cast<allocator_type&&>(rvalue)) { assign(std::move(rvalue)); }
    HashTable(HashTable&& rvalue, const allocator_type& alloc) noexcept : HashTable(alloc) { assign_rvalue_(std::move(rvalue), std::false_type()); }
    HashTable& operator=(HashTable&& rvalue) noexcept;

    HashTable(std::initializer_list<value_type> ilist) : HashTable() { assign(ilist.begin(), ilist.end()); }
    HashTable(std::initializer_list<value_type> ilist, const allocator_type& alloc) : HashTable(alloc) { assign(ilist.begin(), ilist.end()); }
    HashTable& operator=(std::initializer_list<value_type> ilist) { assign(ilist.begin(), ilist.end()); return *this; }

    void assign(std::initializer_list<value_type> ilist) { assign(ilist.begin(), ilist.end()); }
    void assign(HashTable&& rvalue);
    template <typename _It>
    typename std::enable_if<Meta::is_iterator<_It>::value>::type
        assign(_It first, _It last) {
        typedef std::iterator_traits<_It> iterator_traits;
        typedef typename std::iterator_traits<_It>::iterator_category iterator_category;
        clear();
        InsertRange_(first, last, iterator_category());
    }

    iterator find(const key_type& key) {
        size_type slotIndex, dataIndex;
        return (FindUsingProbe_(base_type::MakeProbe_(), key, &slotIndex, &dataIndex) ? begin() + dataIndex : end());
    }

    const_iterator find(const key_type& key) const {
        size_type slotIndex, dataIndex;
        return (FindUsingProbe_(base_type::MakeProbe_(), key, &slotIndex, &dataIndex) ? begin() + dataIndex : end());
    }

    mapped_reference at(const key_type& key) { return table_traits::Value(*find(key)); }
    mapped_const_reference at(const key_type& key) const { return table_traits::Value(*find(key)); }

    mapped_const_reference operator [](const key_type& key) const { return at(key); }

    mapped_reference operator [](const key_type& key) { return table_traits::Value(*try_emplace(key).first); }
    mapped_reference operator [](key_type&& rkey) { return table_traits::Value(*try_emplace(std::move(rkey)).first); }

    Pair<iterator, bool> insert(const value_type& value);
    Pair<iterator, bool> insert(value_type&& rvalue);

    Pair<iterator, bool> insert(iterator hint, const value_type& value) { UNUSED(hint); return insert(value); }
    Pair<iterator, bool> insert(iterator hint, value_type&& rvalue) { UNUSED(hint); return insert(std::move(rvalue)); }

    bool insert_ReturnIfExists(const value_type& value) { return (not insert(value).second); }
    void insert_AssertUnique(const value_type& value) {
        const Pair<iterator, bool> it = insert(value);
        Assert(it.second);
    }

    void insert(std::initializer_list<value_type> ilist) { insert(ilist.begin(), ilist.end()); }
    template <typename _It>
    typename std::enable_if< Meta::is_iterator<_It>::value >::type
        insert(_It first, _It last) {
        typedef std::iterator_traits<_It> iterator_traits;
        typedef typename std::iterator_traits<_It>::iterator_category iterator_category;
        InsertRange_(first, last, iterator_category());
    }

    Pair<iterator, bool> insert_or_assign(const value_type& value);
    Pair<iterator, bool> insert_or_assign(value_type&& rvalue);

    template <typename _MappedLiked>
    Pair<iterator, bool> insert_or_assign(const key_type& key, _MappedLiked&& rvalue) {
        return insert_or_assign(std::move(value_type(key, std::move(rvalue))));
    }
    template <typename _MappedLiked>
    Pair<iterator, bool> insert_or_assign(key_type&& rkey, _MappedLiked&& rvalue) {
        return insert_or_assign(std::move(value_type(std::move(rkey), std::move(rvalue))));
    }

    template <typename... _Args>
    Pair<iterator, bool> emplace(_Args&&... args) {
        value_type tmp(std::forward<_Args>(args)...);
        return insert(std::move(tmp));
    }

    template <typename... _Args>
    bool emplace_ReturnIfExists(_Args&&... args) {
        const Pair<iterator, bool> it = emplace(std::forward<_Args>(args)...);
        return (not it.second);
    }
    template <typename... _Args>
    void emplace_AssertUnique(_Args&&... args) {
        const Pair<iterator, bool> it = emplace(std::forward<_Args>(args)...);
        Assert(it.second);
    }

    template <typename... _Args>
    Pair<iterator, bool> try_emplace(const key_type& key, _Args&&... args);
    template <typename... _Args>
    Pair<iterator, bool> try_emplace(key_type&& rkey, _Args&&... args);

    void erase(const const_iterator& it) {
        Assert(cend() != it);
        Assert(AliasesToContainer(&*it));
        erase_AssertExists(table_traits::Key(*it));
    }

    bool erase(const key_type& key, value_type* pValueIFP);
    bool erase(const key_type& key) { return erase(key, nullptr); }
    void erase(const_iterator first, const_iterator last) {
        for (; first != last; ++first)
            erase(first);
    }

    void erase_AssertExists(const key_type& key) {
        if (not erase(key))
            AssertNotReached();
    }

    value_type erase_ReturnValue(const key_type& key) {
        value_type value;
        if (not erase(key, &value))
            AssertNotReached();
        return std::move(value);
    }

    void clear();
    void clear_ReleaseMemory();

    void rehash(size_type count) { reserve(count); }
    void reserve(size_type count);
    void reserve_Additional(size_type count) { reserve(size() + count); }
    void reserve_AssumeEmpty(size_type count) { Reserve_AssumeEmpty_(count); }
    void shrink_to_fit();

    void swap(HashTable& other);
    friend void swap(HashTable& lhs, HashTable& rhs) { lhs.swap(rhs); }

    bool CheckInvariants() const;

    bool AliasesToContainer(const void* p) const { return (((pointer)p >= _values_hashIndices) && ((pointer)p <= _values_hashIndices + size())); }

    allocator_type get_allocator() const { return static_cast<const allocator_type&>(*this); }

    HashTableStats ProbingStats() const { return MakeProbe_().ProbingStats(); }

private:
    using typename base_type::probe_type;

    using base_type::AliasesToContainer_;
    using base_type::AllocationCountWIndicesFor_;
    using base_type::GrowIFN_ReturnAllocationCount_;
    using base_type::HashIndicesRaw_;
    using base_type::IncSize_;
    using base_type::MakeProbe_;
    using base_type::SetSize_;
    using base_type::ShrinkToFitIFN_ReturnAllocationCount_;

    using base_type::_size_capacityIndex;
    using base_type::_values_hashIndices;

    template <typename _KeyLike>
    static size_type KeyHash_(const _KeyLike& keylike) {
        const size_type h = hasher()(table_traits::Key(keylike));
        return (h | (h == 0)); // 0 is not allowed as a valid hash value (reserved for empty slots)
    }

    template <typename _KeyLike0, typename _KeyLike1>
    static bool KeyEqual_(const _KeyLike0& lhs, const _KeyLike1& rhs) {
        return key_equal()(table_traits::Key(lhs), table_traits::Key(rhs));
    }

    allocator_type& allocator_() { return static_cast<allocator_type&>(*this); }
    const allocator_type& allocator_() const { return static_cast<const allocator_type&>(*this); }

    void allocator_copy_(const allocator_type& other, std::true_type );
    void allocator_copy_(const allocator_type& other, std::false_type ) { UNUSED(other); }

    void allocator_move_(allocator_type&& rvalue, std::true_type );
    void allocator_move_(allocator_type&& rvalue, std::false_type ) { UNUSED(rvalue); }

    void assign_rvalue_(HashTable&& rvalue, std::true_type );
    void assign_rvalue_(HashTable&& rvalue, std::false_type );

    bool FindUsingProbe_(const details::HashTableProbe_& probe, const key_type& key, size_type* pSlotIndex, size_type* pDataIndex) const;
    template <typename _HashWIndices>
    bool FindUsingProbe_(const details::HashTableProbe_& probe, const MemoryView<_HashWIndices>& hashWIndices, const key_type& key, size_type* pSlotIndex, size_type* pDataIndex) const;

    bool InsertUsingProbe_AssumeEnoughCapacity_(details::HashTableProbe_& probe, const key_type& key, size_type* pDataIndex);
    template <typename _HashWIndices>
    bool InsertUsingProbe_AssumeEnoughCapacity_(details::HashTableProbe_& probe, const MemoryView<_HashWIndices>& hashWIndices, const key_type& key, size_type* pDataIndex);

    bool Insert_ReturnIfExists_(const key_type& key, size_type* pDataIndex);

    template <typename _It>
    void InsertRange_(_It first, _It last, std::input_iterator_tag );
    template <typename _It, typename _ItCat>
    void InsertRange_(_It first, _It last, _ItCat );

    void Reserve_AssumeEmpty_(size_type count);
    void RelocateAndRehash_(size_type oldCapacity, size_type allocationCount);
    void RehashUsingProbe_(details::HashTableProbe_& probe);
    void Rehash_();

    void swap_(HashTable& other, std::true_type );
    void swap_(HashTable& other, std::false_type );
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
bool TryGetValue(const HashTable<_Key, _Value, _Hasher, _EqualTo, _Allocator>& hashtable, const _Key& key, _Value *value) {
    Assert(value);

    const auto it = hashtable.find(key);
    if (hashtable.end() == it)
        return false;

    *value = it->second;
    return true;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
bool Insert_ReturnIfExists(HashTable<_Key, _Value, _Hasher, _EqualTo, _Allocator>& hashtable, const _Key& key, const _Value& value) {
    return hashtable.emplace_ReturnIfExists(key, value)
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
void Insert_AssertUnique(HashTable<_Key, _Value, _Hasher, _EqualTo, _Allocator>& hashtable, const _Key& key, const _Value& value) {
    hashtable.emplace_AssertUnique(key, value)
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
bool Remove_ReturnIfExists(HashTable<_Key, _Value, _Hasher, _EqualTo, _Allocator>& hashtable, const _Key& key) {
    return hashtable.erase(key);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
void Remove_AssertExists(HashTable<_Key, _Value, _Hasher, _EqualTo, _Allocator>& hashtable, const _Key& key) {
    hashtable.erase_AssertExists(key);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
_Value Remove_ReturnValue(HashTable<_Key, _Value, _Hasher, _EqualTo, _Allocator>& hashtable, const _Key& key) {
    return std::move(hashtable.erase_ReturnValue(key).second);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
void Remove_AssertExistsAndSameValue(HashTable<_Key, _Value, _Hasher, _EqualTo, _Allocator>& hashtable, const _Key& key, const _Value& value) {
#ifdef WITH_CORE_ASSERT
    const auto stored = hashtable.erase_ReturnValue(key);
    Assert(stored.second == value);
#else
    hashtable.erase_AssertExists(key);
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/Container/HashTable-inl.h"
