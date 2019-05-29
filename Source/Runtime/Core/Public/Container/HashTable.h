#pragma once

#include "Core.h"

#include "Allocator/Allocation.h"
#include "Container/BitMask.h"
#include "Container/Hash.h"
#include "Container/Pair.h"
#include "HAL/PlatformMaths.h"
#include "HAL/PlatformMemory.h"
#include "Memory/MemoryView.h"
#include "Meta/Iterator.h"

#include <algorithm>
#include <initializer_list>
#include <iterator>
#include <utility>
#include <type_traits>

// CppCon 2017: Matt Kulukundis
// Designing a Fast, Efficient, Cache-friendly Hash Table, Step by Step
// https://www.youtube.com/watch?v=ncHmEUmJZf4

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
template <typename _Key, typename _Value>
struct THashMapTraits_ {
    typedef _Key key_type;
    typedef _Value mapped_type;
    typedef TPair<_Key, _Value> value_type;
    typedef TPair<Meta::TAddConst<_Key>, _Value> public_type;
    typedef Meta::TAddReference<mapped_type> mapped_reference;
    typedef Meta::TAddReference<const mapped_type> mapped_reference_const;
    static key_type& Key(value_type& value) { return value.first; }
    static const key_type& Key(const value_type& value) { return value.first; }
    static const key_type& Key(const public_type& value) { return value.first; }
    static mapped_type& Value(value_type& value) { return value.second; }
    static const mapped_type& Value(const value_type& value) { return value.second; }
    static mapped_type& Value(public_type& value) { return value.second; }
    static const mapped_type& Value(const public_type& value) { return value.second; }
    template <typename _It> static TKeyIterator<_It> MakeKeyIterator(_It&& it) { return PPE::MakeKeyIterator(std::move(it)); }
    template <typename _It> static TValueIterator<_It> MakeValueIterator(_It&& it) { return PPE::MakeValueIterator(std::move(it)); }
    static value_type MakeValue(const _Key& key) { return value_type(key, _Value()); }
    static value_type MakeValue(_Key&& rkey) { return value_type(std::move(rkey), _Value()); }
    static value_type MakeValue(const _Key& key, const _Value& value) { return value_type(key, value); }
    static value_type MakeValue(const _Key& key, _Value&& rvalue) { return value_type(key, std::move(rvalue)); }
    static value_type MakeValue(_Key&& rkey, _Value&& rvalue) { return value_type(std::move(rkey), std::move(rvalue)); }
    static value_type&& MakeValue(value_type&& rvalue) { return std::move(rvalue); }
    static const value_type& MakeValue(const value_type& value) { return value; }
};
template <typename _Key>
struct THashSetTraits_ {
    typedef _Key key_type;
    typedef _Key value_type;
    typedef _Key mapped_type;
    typedef Meta::TAddConst<_Key> public_type;
    typedef Meta::TAddReference<public_type> mapped_reference_const;
    typedef mapped_reference_const mapped_reference;
    static _Key& Key(value_type& value) { return value; }
    static const _Key& Key(const value_type& value) { return value; }
    static mapped_type& Value(value_type& value) { return value; }
    static const mapped_type& Value(const value_type& value) { return value; }
    template <typename _It> static const _It& MakeKeyIterator(const _It& it) { return it; }
    template <typename _It> static const _It& MakeValueIterator(const _It& it) { return it; }
    template <typename... _Args> static value_type MakeValue(_Args&&... args) { return value_type(std::forward<_Args>(args)...); }
};
struct FHashTableData_ {
    u32     Size;
    u32     Capacity;
    void*   StatesAndBuckets;

    FHashTableData_()
        : Size(0)
        , Capacity(0)
        , StatesAndBuckets(nullptr)
    {}

    typedef i8 state_t;
    enum EState : state_t {
        kEmpty      = -1,
        kSentinel   = -2, // for iterators
        kDeleted    = -128,
    };

    typedef ::__m128i group_t;
    static constexpr size_t GGroupSize = 16;

    size_t NumBuckets() const {
        return size_t(Capacity);
    }

    size_t NumStates() const {
        // 15 last states are mirroring the 15th
        // allows to sample state across table boundary
        return size_t(Capacity ? Capacity + GGroupSize/* sentinel */ : 0);
    }

    const state_t* State(size_t index) const {
        Assert(index < Capacity);
        return ((const state_t*)StatesAndBuckets + index);
    }

    PPE_CORE_API state_t SetState(size_t index, state_t state);

    bool SetElement(size_t index, size_t hash);
    PPE_CORE_API void SetDeleted(size_t index);

    PPE_CORE_API void ResetStates();

    group_t SIMD_INLINE GroupAt(size_t first) const {
        return ::_mm_lddqu_si128((const ::__m128i*)State(first));
    }

    group_t SIMD_INLINE GroupAt_StreamLoad(size_t first) const {
        Assert(Meta::IsAligned(16, State(first)));
        return ::_mm_stream_load_si128((const ::__m128i*)State(first));
    }

    static FBitMask SIMD_INLINE Match(group_t group, group_t state) {
        return FBitMask{ size_t(::_mm_movemask_epi8(::_mm_cmpeq_epi8(group, state))) };
    }

    static FBitMask SIMD_INLINE MatchEmpty(group_t group) {
        return FBitMask{ size_t(::_mm_movemask_epi8(::_mm_cmpeq_epi8(group, ::_mm_set1_epi8(kEmpty)))) };
    }

    static FBitMask SIMD_INLINE MatchNonEmpty(group_t group) {
        return FBitMask{ (~size_t(::_mm_movemask_epi8(::_mm_cmpeq_epi8(group, ::_mm_set1_epi8(kEmpty))))) & 0xFFFFu };
    }

    static FBitMask SIMD_INLINE MatchFreeBucket(group_t group) {
        return FBitMask{ size_t(::_mm_movemask_epi8(::_mm_and_si128(group, ::_mm_set1_epi8(kDeleted)))) };
    }

    static FBitMask SIMD_INLINE MatchFilledBucket(group_t group) {
        return FBitMask{ size_t(::_mm_movemask_epi8(::_mm_andnot_si128(group, ::_mm_set1_epi8(kDeleted)))) };
    }

    void Swap(FHashTableData_& other);

    FORCE_INLINE static constexpr size_t H1(size_t hash) { return hash >> 7; }
    FORCE_INLINE static constexpr state_t H2(size_t hash) { return state_t(hash & 0x7f); }

    size_t FirstFilledIndex() const {
        const size_t first = (Size
            ? FirstFilledBucket_ReturnOffset((const state_t*)StatesAndBuckets)
            : size_t(Capacity));
        return first;
    }

    static PPE_CORE_API size_t FirstFilledBucket_ReturnOffset(const state_t* states);
};
template <typename T>
class THashTableIterator_ : public Meta::TIterator<T, std::forward_iterator_tag> {
public:
    typedef Meta::TIterator<T, std::forward_iterator_tag> parent_type;

    using state_t = typename FHashTableData_::state_t;

    using typename parent_type::value_type;
    using typename parent_type::reference;
    using typename parent_type::pointer;
    using typename parent_type::difference_type;
    using typename parent_type::iterator_category;

    THashTableIterator_() NOEXCEPT : THashTableIterator_(nullptr, nullptr) {}
    THashTableIterator_(const state_t* states, pointer buckets, size_t capacity, size_t index) NOEXCEPT
        : _state(states + index)
        , _bucket(MakeCheckedIterator(buckets, capacity, index))
    {}

    template <typename U>
    THashTableIterator_(const THashTableIterator_<U>& other)
        : _state(other.state())
        , _bucket(other.bucket())
    {}
    template <typename U>
    THashTableIterator_& operator =(const THashTableIterator_<U>& other) {
        _state = other.state();
        _bucket = other.bucket();
        return *this;
    }

    const state_t* state() const { return _state; }
    const TCheckedArrayIterator<T>& bucket() const { return _bucket; }

    pointer data() const { return std::addressof(*_bucket); }

    THashTableIterator_& operator++() /* prefix */ { return GotoNextBucket_(); }
    THashTableIterator_ operator++(int) /* postfix */ {
        THashTableIterator_ tmp(*this);
        ++(*this);
        return tmp;
    }

    reference operator*() const { return (*_bucket); }
    pointer operator->() const { return data(); }

    void Swap(THashTableIterator_& other) {
        std::swap(_state, other._state);
        std::swap(_bucket, other._bucket);
    }

    bool AliasesToContainer(const TMemoryView<const state_t>& states) const {
        return states.AliasesToContainer(*_state);
    }

    template <typename U>
    bool operator ==(const THashTableIterator_<U>& other) const {
#if USE_PPE_ASSERT
        if (_state == other.state()) {
            Assert(_bucket == other.bucket());
            return true;
        }
        else {
            Assert(_bucket != other.bucket());
            return false;
        }
#else
        return (_state == other.state());
#endif
    }
    template <typename U>
    bool operator !=(const THashTableIterator_<U>& other) const { return (not operator ==(other)); }

private:
    THashTableIterator_& GotoNextBucket_() NOEXCEPT {
        using FHTD = FHashTableData_;
        const size_t offset = (1 + FHTD::FirstFilledBucket_ReturnOffset(_state + 1));
        _bucket += offset; // _bucket first since it's a checked iterator
        _state += offset;
        return (*this);
    }

    const state_t* _state;
    TCheckedArrayIterator<T> _bucket;
};
} //!details
//----------------------------------------------------------------------------
template <typename T>
inline void swap(details::THashTableIterator_<T>& lhs, details::THashTableIterator_<T>& rhs) {
    lhs.Swap(rhs);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
class EMPTY_BASES TBasicHashTable : _Hasher, _EqualTo, _Allocator {
    using FHTD = typename details::FHashTableData_;
public:
    template <typename _Traits2, typename _Hasher2, typename _EqualTo2, typename _Allocator2>
    friend class TBasicHashTable;

    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef typename FHTD::state_t state_t;

    typedef _Traits table_traits;

    typedef typename table_traits::key_type key_type;
    typedef typename table_traits::mapped_type mapped_type;
    typedef typename table_traits::value_type value_type;
    typedef typename table_traits::public_type public_type;

    typedef typename table_traits::mapped_reference mapped_reference;
    typedef typename table_traits::mapped_reference_const mapped_reference_const;

    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef Meta::TAddPointer<value_type> pointer;
    typedef Meta::TAddPointer<const value_type> const_pointer;

    typedef _Hasher hasher;
    typedef _EqualTo key_equal;

    typedef _Allocator allocator_type;
    typedef TAllocatorTraits<allocator_type> allocator_traits;

    typedef details::THashTableIterator_<public_type> iterator;
    typedef details::THashTableIterator_<Meta::TAddConst<public_type>> const_iterator;

    TBasicHashTable() NOEXCEPT {}

    ~TBasicHashTable() {
        Assert(CheckInvariants());
        clear_ReleaseMemory();
    }

    explicit TBasicHashTable(allocator_type&& alloc) : allocator_type(std::move(alloc)) {}
    explicit TBasicHashTable(const allocator_type& alloc) : allocator_type(alloc) {}

    TBasicHashTable(hasher&& hash, key_equal&& equalto) : hasher(std::move(hash)), key_equal(std::move(equalto)) {}
    TBasicHashTable(hasher&& hash, key_equal&& equalto, allocator_type&& alloc) : hasher(std::move(hash)), key_equal(std::move(equalto)), allocator_type(std::move(alloc)) {}

    explicit TBasicHashTable(size_type capacity) : TBasicHashTable() { reserve(capacity); }
    TBasicHashTable(size_type capacity, const allocator_type& alloc) : TBasicHashTable(alloc) { reserve(capacity); }

    TBasicHashTable(const TBasicHashTable& other) : TBasicHashTable(allocator_traits::SelectOnCopy(other)) { assign(other.begin(), other.end()); }
    TBasicHashTable(const TBasicHashTable& other, const allocator_type& alloc) : TBasicHashTable(alloc) { assign(other.begin(), other.end()); }
    TBasicHashTable& operator=(const TBasicHashTable& other);

    TBasicHashTable(TBasicHashTable&& rvalue) NOEXCEPT : TBasicHashTable(allocator_traits::SelectOnMove(std::move(rvalue))) { assign(std::move(rvalue)); }
    TBasicHashTable(TBasicHashTable&& rvalue, const allocator_type& alloc) NOEXCEPT : TBasicHashTable(alloc) { assign(std::move(rvalue)); }
    TBasicHashTable& operator=(TBasicHashTable&& rvalue) NOEXCEPT;

    TBasicHashTable(std::initializer_list<value_type> ilist) : TBasicHashTable() { assign(ilist.begin(), ilist.end()); }
    TBasicHashTable(std::initializer_list<value_type> ilist, const allocator_type& alloc) : TBasicHashTable(alloc) { assign(ilist.begin(), ilist.end()); }
    TBasicHashTable& operator=(std::initializer_list<value_type> ilist) { assign(ilist.begin(), ilist.end()); return *this; }

    template <typename _OtherAllocator, typename = Meta::TEnableIf<has_stealallocatorblock_v<_Allocator, _OtherAllocator>> >
    TBasicHashTable(TBasicHashTable<_Traits, _Hasher, _EqualTo, _OtherAllocator>&& rvalue) : TBasicHashTable() { operator =(std::move(rvalue)); }
    template <typename _OtherAllocator, typename = Meta::TEnableIf<has_stealallocatorblock_v<_Allocator, _OtherAllocator>> >
    TBasicHashTable& operator =(TBasicHashTable<_Traits, _Hasher, _EqualTo, _OtherAllocator>&& rvalue) {
        if (_data.StatesAndBuckets)
            clear_ReleaseMemory();

        const TMemoryView<value_type> b = rvalue.allocated_block_();
        Verify(TAllocatorTraits<_OtherAllocator>::StealAndAcquire(
            &allocator_traits::Get(*this),
            TAllocatorTraits<_OtherAllocator>::Get(rvalue),
            FAllocatorBlock::From(b) ));

        std::swap(_data, rvalue._data);
        Assert_NoAssume(allocated_block_() == b);
        Assert_NoAssume(nullptr == rvalue._data.StatesAndBuckets);

        return (*this);
    }

    size_type capacity() const { return (_data.Capacity); }
    bool empty() const { return (0 == _data.Size); }
    size_type size() const { return size_type(_data.Size); }

    size_type bucket_count() const { return capacity(); }
    size_type max_bucket_count() const { return Min(size_type(1ull << 24) - 1, size_type(allocator_traits::MaxSize() / sizeof(value_type))); }

    float load_factor() const;
    size_type max_probe_dist() const;

    iterator begin() { return MakeIterator_(_data.FirstFilledIndex()); }
    iterator end() { return MakeIterator_(_data.NumBuckets()); }

    const_iterator begin() const { return MakeIterator_(_data.FirstFilledIndex()); }
    const_iterator end() const { return MakeIterator_(_data.NumBuckets()); }

    const_iterator cbegin() const { return begin(); }
    const_iterator cend() const { return end(); }

    auto Keys() { return MakeIterable(table_traits::MakeKeyIterator(begin()), table_traits::MakeKeyIterator(end())); }
    auto Keys() const { return MakeIterable(table_traits::MakeKeyIterator(cbegin()), table_traits::MakeKeyIterator(cend())); }

    auto Values() { return MakeIterable(table_traits::MakeValueIterator(begin()), table_traits::MakeValueIterator(end())); }
    auto Values() const { return MakeIterable(table_traits::MakeValueIterator(cbegin()), table_traits::MakeValueIterator(cend())); }

    void assign(std::initializer_list<value_type> ilist) { assign(ilist.begin(), ilist.end()); }
    void assign(TBasicHashTable&& rvalue);
    template <typename _It>
    typename std::enable_if<Meta::is_iterator<_It>::value>::type
        assign(_It first, _It last) {
        typedef std::iterator_traits<_It> iterator_traits;
        typedef typename std::iterator_traits<_It>::iterator_category iterator_category;
        clear();

        if (first != last) {
            Assert_NoAssume(not AliasesToContainer(*first));
            insert_(first, last, iterator_category());
        }
    }

    void append(const TBasicHashTable& other) {
        insert(other.begin(), other.end());
    }
    void append_AssertUnique(const TBasicHashTable& other) {
        reserve_Additional(other.size());
        insert_AssertUnique(other.begin(), other.end());
    }

    iterator find(const key_type& key) NOEXCEPT;
    const_iterator find(const key_type& key) const NOEXCEPT;

    template <typename _KeyLike>
    iterator find_like(const _KeyLike& keyLike, hash_t hash) NOEXCEPT;
    template <typename _KeyLike>
    const_iterator find_like(const _KeyLike& keyLike, hash_t hash) const NOEXCEPT;

    bool Contains(const key_type& key) const NOEXCEPT { return (end() != find(key)); }

    mapped_reference at(const key_type& key) { return table_traits::Value(*find(key)); }
    mapped_reference_const at(const key_type& key) const { return table_traits::Value(*find(key)); }

    mapped_reference_const operator [](const key_type& key) const { return at(key); }

    mapped_reference Add(const key_type& key);
    mapped_reference Add(key_type&& rkey);

    mapped_reference operator [](const key_type& key) { return table_traits::Value(*try_emplace(key).first); }
    mapped_reference operator [](key_type&& rkey) { return table_traits::Value(*try_emplace(std::move(rkey)).first); }

    TPair<iterator, bool> insert(const value_type& value);
    TPair<iterator, bool> insert(value_type&& rvalue);

    TPair<iterator, bool> insert(iterator hint, const value_type& value) { UNUSED(hint); return insert(value); }
    TPair<iterator, bool> insert(iterator hint, value_type&& rvalue) { UNUSED(hint); return insert(std::move(rvalue)); }

    bool insert_ReturnIfExists(const value_type& value) { return (not insert(value).second); }
    void insert_AssertUnique(const value_type& value);

    bool insert_ReturnIfExists(value_type&& rvalue) { return (not insert(std::move(rvalue)).second); }
    void insert_AssertUnique(value_type&& rvalue);

    void insert(std::initializer_list<value_type> ilist) { insert(ilist.begin(), ilist.end()); }

    template <typename _It>
    typename std::enable_if< Meta::is_iterator<_It>::value >::type
        insert(_It first, _It last) {
        typedef std::iterator_traits<_It> iterator_traits;
        typedef typename std::iterator_traits<_It>::iterator_category iterator_category;
        insert_(first, last, iterator_category());
    }

    template <typename _It>
    void insert_AssertUnique(_It first, _It last) {
#if USE_PPE_ASSERT
        for (; first != last; ++first)
            insert_AssertUnique(*first);
#else
        insert(first, last);
#endif
    }

    TPair<iterator, bool> insert_or_assign(const value_type& value);
    TPair<iterator, bool> insert_or_assign(value_type&& rvalue);

    template <typename _MappedLiked>
    TPair<iterator, bool> insert_or_assign(const key_type& key, _MappedLiked&& rvalue) {
        return insert_or_assign(std::move(value_type(key, std::move(rvalue))));
    }
    template <typename _MappedLiked>
    TPair<iterator, bool> insert_or_assign(key_type&& rkey, _MappedLiked&& rvalue) {
        return insert_or_assign(std::move(value_type(std::move(rkey), std::move(rvalue))));
    }

    template <typename... _Args>
    TPair<iterator, bool> emplace(_Args&&... args) {
        value_type tmp(std::forward<_Args>(args)...);
        return insert(std::move(tmp));
    }

    template <typename... _Args>
    bool emplace_ReturnIfExists(_Args&&... args) {
        const TPair<iterator, bool> it = emplace(std::forward<_Args>(args)...);
        return (not it.second);
    }
    template <typename... _Args>
    iterator emplace_AssertUnique(_Args&&... args) {
        const TPair<iterator, bool> it = emplace(std::forward<_Args>(args)...);
        Assert(it.second);
        return (it.first);
    }

    template <typename... _Args>
    TPair<iterator, bool> try_emplace(const key_type& key, _Args&&... args);
    template <typename... _Args>
    TPair<iterator, bool> try_emplace(key_type&& rkey, _Args&&... args);

    void erase(const const_iterator& it);

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

    void rehash(size_type count);
    void reserve(size_type count);
    void reserve_Additional(size_type count) { reserve(size() + count); }
    void shrink_to_fit();

    void swap(TBasicHashTable& other);
    friend void swap(TBasicHashTable& lhs, TBasicHashTable& rhs) { lhs.swap(rhs); }

    allocator_type get_allocator() const { return static_cast<const allocator_type&>(*this); }

    bool CheckInvariants() const;

    bool AliasesToContainer(const void* p) const;
    bool AliasesToContainer(const_reference v) const;
    bool AliasesToContainer(const_iterator it) const;

    bool operator ==(const TBasicHashTable& other) const;
    bool operator !=(const TBasicHashTable& other) const { return (not operator ==(other)); }

private:
    STATIC_CONST_INTEGRAL(size_type, MaxLoadFactor, 80);
    STATIC_CONST_INTEGRAL(size_type, SlackFactor, ((100 - MaxLoadFactor) * 128) / 100);

    allocator_type& allocator_() { return static_cast<allocator_type&>(*this); }
    const allocator_type& allocator_() const { return static_cast<const allocator_type&>(*this); }

    TMemoryView<value_type> allocated_block_() const {
        return TMemoryView<value_type>(
            reinterpret_cast<value_type*>(_data.StatesAndBuckets),
            OffsetOfBuckets_() + _data.Capacity );
    }

    void allocator_copy_(const allocator_type& other, std::true_type);
    void allocator_copy_(const allocator_type& , std::false_type) {}

    void allocator_move_(allocator_type&& rvalue, std::true_type);
    void allocator_move_(allocator_type&& , std::false_type) {}

    template <typename _It>
    void insert_(_It first, _It last, std::forward_iterator_tag);
    template <typename _It>
    void insert_(_It first, _It last, std::bidirectional_iterator_tag);
    template <typename _It>
    void insert_(_It first, _It last, std::random_access_iterator_tag);

    void clear_keepSound_(std::true_type);
    void clear_keepSound_(std::false_type);

    void clear_leaveDirty_(std::true_type);
    void clear_leaveDirty_(std::false_type);

    void clear_ReleaseMemory_(std::true_type);
    void clear_ReleaseMemory_(std::false_type);

    void swap_(TBasicHashTable& other, std::true_type);
    void swap_(TBasicHashTable& other, std::false_type);

    size_type GrowIFN_ReturnNewCapacity_(size_type atleast) const;
    size_type ShrinkIFN_ReturnNewCapacity_(size_type atleast) const;

    template <typename... _Args>
    TPair<iterator, bool> InsertIFN_(const key_type& key, _Args&&... args);

    template <typename _KeyLike>
    size_type FindFilledBucket_(const _KeyLike& key, size_t hash) const NOEXCEPT;
#if USE_PPE_ASSERT
    template <typename _KeyLike>
    size_type FindEmptyBucket_(const _KeyLike& key, size_t hash) const NOEXCEPT;
#else
    size_type FindEmptyBucket_(size_t hash) const NOEXCEPT;
#endif
    template <typename _KeyLike>
    size_type FindOrAllocateBucket_(const _KeyLike& key, size_t hash) const NOEXCEPT;

    void RelocateRehash_(size_type newCapacity);

    size_t OffsetOfBuckets_() const {
        return ((_data.NumStates() + (sizeof(value_type) - 1)) / sizeof(value_type));
    }

    pointer BucketAtUnsafe_(size_t index) const NOEXCEPT {
        return ((pointer)_data.StatesAndBuckets + OffsetOfBuckets_() + index);
    }
    pointer BucketAt_(size_t index) const {
        Assert(index < (_data.Capacity));
        return BucketAtUnsafe_(index);
    }

    iterator MakeIterator_(size_t index) NOEXCEPT {
        return iterator(
            (const state_t*)_data.StatesAndBuckets,
            (typename iterator::pointer)_data.StatesAndBuckets + OffsetOfBuckets_(),
            capacity(),
            index );
    }

    const_iterator MakeIterator_(size_t index) const NOEXCEPT {
        return const_iterator(
            (const state_t*)_data.StatesAndBuckets,
            (typename const_iterator::pointer)_data.StatesAndBuckets + OffsetOfBuckets_(),
            capacity(),
            index );
    }

    static FORCE_INLINE size_t SeedHash_(size_t h) {
        return (h ^ PPE_HASH_VALUE_SEED/* don't want hash == 0 */);
    }

    size_t HashKey_(const key_type& key) const {
        return SeedHash_(HashKeyNoSeed_(key));
    }

    size_t HashKeyNoSeed_(const key_type& key) const {
        return static_cast<const _Hasher&>(*this)(key);
    }

    size_t HashValue_(const value_type& value) const {
        return HashKey_(table_traits::Key(value));
    }

    FHTD _data;
};
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
hash_t hash_value(const TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>& hashTable) {
    return hash_range(hashTable.begin(), hashTable.end());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#include "Container/HashTable-inl.h"
