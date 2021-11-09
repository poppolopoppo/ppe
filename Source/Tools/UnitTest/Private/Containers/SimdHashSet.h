#pragma once

#include "Allocator/Allocation.h"
#include "Container/BitMask.h"
#include "HAL/PlatformMaths.h"
#include "HAL/PlatformMemory.h"
#include "Maths/SSEHelpers.h"
#include "Memory/MemoryView.h"
#include "Meta/AlignedStorage.h"
#include "Meta/PointerWFlags.h"
#include "Meta/Iterator.h"

#include "Intel_IACA.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
using FSSEHashStates = Meta::TArray<i8, 16>;
//----------------------------------------------------------------------------
ALIGN(16) CONSTEXPR const FSSEHashStates GSimdBucketSentinel{ 0 };
//----------------------------------------------------------------------------
template <typename T>
struct ALIGN(16) TSSEHashBucket {
    STATIC_CONST_INTEGRAL(u32, Capacity, 16);
    using bitmask_t = TBitMask<u32>;
    using states_t = FSSEHashStates;
    using storage_t = POD_STORAGE(T);

    states_t States;
    storage_t Items[Capacity];

    FORCE_INLINE TSSEHashBucket() noexcept {
        STATIC_ASSERT(sizeof(states_t) == sizeof(m128i_t));
        m128i_epi8_store_aligned(&States, m128i_epi8_set_true());
    }

    FORCE_INLINE ~TSSEHashBucket() {
        clear_LeaveDirty();
    }

    FORCE_INLINE TSSEHashBucket(const TSSEHashBucket& other) {
        copy_AssumeEmpty(other);
    }
    FORCE_INLINE TSSEHashBucket& operator =(const TSSEHashBucket& other) {
        clear_LeaveDirty();
        copy_AssumeEmpty(other);
        return (*this);
    }

    FORCE_INLINE TSSEHashBucket(TSSEHashBucket&& rvalue) {
        move_AssumeEmpty(std::move(rvalue));
    }
    FORCE_INLINE TSSEHashBucket& operator =(TSSEHashBucket&& rvalue) {
        clear_LeaveDirty();
        move_AssumeEmpty(std::move(rvalue));
        return (*this);
    }

#if USE_PPE_DEBUG
    bool is_sentinel() const {
        m128i_t st = m128i_epi8_load_aligned(&States);
        const u32 bm = m128i_epi8_findeq(st, m128i_epi8_set_zero());
        return (0xFFFFu == bm);
    }
#endif

    FORCE_INLINE T& at(size_t i) NOEXCEPT {
        Assert(i < Capacity);
        return reinterpret_cast<T&>(Items[i]);
    }
    const T& at(size_t i) const NOEXCEPT {
        return const_cast<TSSEHashBucket*>(this)->at(i);
    }

    FORCE_INLINE void clear_LeaveDirty() NOEXCEPT {
        IF_CONSTEXPR(not Meta::has_trivial_destructor<T>::value) {
            for (bitmask_t bm{ m128i_epi8_findneq(m128i_epi8_load_aligned(&States), m128i_epi8_set_true()) }; bm.Data; )
                Meta::Destroy(&at(bm.PopFront_AssumeNotEmpty()));
        }
    }

    FORCE_INLINE void clear() NOEXCEPT {
        clear_LeaveDirty();
        m128i_epi8_store_aligned(&States, m128i_epi8_set_true());
    }

    FORCE_INLINE void copy_AssumeEmpty(const TSSEHashBucket& other) {
        m128i_t st = m128i_epi8_load_aligned(&other.States);
        for (bitmask_t bm{ m128i_epi8_findneq(st, m128i_epi8_set_true()) }; bm.Data; ) {
            const u32 e = bm.PopFront_AssumeNotEmpty();
            Meta::Construct(&at(e), other.at(e));
        }
        m128i_epi8_store_aligned(&States, st);
    }

    FORCE_INLINE void move_AssumeEmpty(TSSEHashBucket&& rvalue) NOEXCEPT {
        m128i_t st = m128i_epi8_load_aligned(&rvalue.States);
        for (bitmask_t bm{ m128i_epi8_findneq(st, m128i_epi8_set_true()) }; bm.Data; ) {
            const u32 e = bm.PopFront_AssumeNotEmpty();
            Meta::Construct(&at(e), std::move(rvalue.at(e)));
            Meta::Destroy(&rvalue.at(e));
        }
        m128i_epi8_store_aligned(&States, st);
        m128i_epi8_store_aligned(&rvalue.States, m128i_epi8_set_true());
    }

    FORCE_INLINE bitmask_t each() const NOEXCEPT {
        return bitmask_t{ m128i_epi8_findneq(m128i_epi8_load_aligned(&States), m128i_epi8_set_true()) };
    }

    FORCE_INLINE static TSSEHashBucket* Sentinel() NOEXCEPT {
        // we guarantee we won't access Items, so use a generic sentinel (avoid bloating rdata)
        return (TSSEHashBucket*)(&GSimdBucketSentinel);
    }

};
//----------------------------------------------------------------------------
template <typename T, bool _Const>
struct TSSEHashIterator : Meta::TIterator<Meta::TConditional<_Const, Meta::TAddConst<T>, T>> {
    using bucket_t = TSSEHashBucket<T>;
    using bitmask_t = typename bucket_t::bitmask_t;
    using parent_t = Meta::TIterator<Meta::TConditional<_Const, Meta::TAddConst<T>, T>>;

    using typename parent_t::iterator_category;
    using typename parent_t::value_type;
    using typename parent_t::difference_type;
    using typename parent_t::pointer;
    using typename parent_t::reference;

    Meta::FHeapPtrWCounter BucketAndSlot;

    CONSTEXPR TSSEHashIterator(Meta::FNoInit) NOEXCEPT {}

    CONSTEXPR TSSEHashIterator(bucket_t* bucket, u32 slot) NOEXCEPT {
        BucketAndSlot.Reset(bucket, slot);
    }

    template <bool _Other>
    CONSTEXPR TSSEHashIterator(const TSSEHashIterator<T, _Other>& other) NOEXCEPT
    :   BucketAndSlot(other.BucketAndSlot)
    {}
    template <bool _Other>
    CONSTEXPR TSSEHashIterator& operator =(const TSSEHashIterator<T, _Other>& other) NOEXCEPT {
        BucketAndSlot = other.BucketAndSlot;
        return (*this);
    }

    bucket_t& Bucket() const NOEXCEPT { return (*BucketAndSlot.Ptr<bucket_t>()); }
    u32 Slot() const NOEXCEPT { return BucketAndSlot.Counter(); }

    TSSEHashIterator& operator++() { Advance(); return (*this); }
    TSSEHashIterator& operator++(int) { TSSEHashIterator tmp(*this); Advance(); return tmp; }

    reference operator *() const { return Bucket().at(BucketAndSlot.Counter()); }
    pointer operator ->() const { return (&operator *()); }

    CONSTEXPR inline friend bool operator ==(const TSSEHashIterator& lhs, const TSSEHashIterator& rhs) NOEXCEPT {
        return (lhs.BucketAndSlot.Data == rhs.BucketAndSlot.Data);
    }
    CONSTEXPR inline friend bool operator !=(const TSSEHashIterator& lhs, const TSSEHashIterator& rhs) NOEXCEPT {
        return (not operator ==(lhs, rhs));
    }

    void FirstSet(i8 slot) NOEXCEPT {
        bucket_t* pbucket = &Bucket();
        for (Assert(pbucket);; ++pbucket, slot = -1) {
            const bitmask_t bm{ m128i_epi8_findneq_after(m128i_epi8_load_aligned(&pbucket->States), m128i_epi8_set_true(), slot) };
            if (Likely(bm.Data)) {
                BucketAndSlot.Reset(pbucket, bm.FirstBitSet_AssumeNotEmpty());
                return;
            }
        }
    }

    FORCE_INLINE void Advance() NOEXCEPT {
        FirstSet(checked_cast<i8>(BucketAndSlot.Counter()));
    }

};
//----------------------------------------------------------------------------
template <
    typename _Key
,   typename _Hash = Meta::THash<_Key>
,   typename _EqualTo = Meta::TEqualTo<_Key>
,   typename _Allocator = ALLOCATOR(Container)
>   class TSSEHashSet : _Allocator {
public:
    typedef _Key value_type;
    typedef _Hash hasher;
    typedef _EqualTo key_equal;

    using allocator_type = _Allocator;
    using allocator_traits = TAllocatorTraits<allocator_type>;

    using bucket_t = TSSEHashBucket<value_type>;

    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef Meta::TAddPointer<value_type> pointer;
    typedef Meta::TAddPointer<const value_type> const_pointer;

    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

    using iterator = TSSEHashIterator<value_type, false>;
    using const_iterator = TSSEHashIterator<value_type, true>;

    CONSTEXPR TSSEHashSet() NOEXCEPT
    :   _size(0)
    ,   _bucketMask(0)
    ,   _buckets(bucket_t::Sentinel())
    {}

    FORCE_INLINE ~TSSEHashSet() {
        if (has_content_())
            releaseMemory_ForDtor_();
    }

    FORCE_INLINE TSSEHashSet(const TSSEHashSet& other) : TSSEHashSet() {
        assign(other);
    }
    FORCE_INLINE TSSEHashSet& operator =(const TSSEHashSet& other) {
        assign(other);
        return (*this);
    }

    CONSTEXPR TSSEHashSet(TSSEHashSet&& rvalue) NOEXCEPT
    :   TSSEHashSet() {
        std::swap(_size, rvalue._size);
        std::swap(_bucketMask, rvalue._bucketMask);
        std::swap(_buckets, rvalue._buckets);
    }
    FORCE_INLINE TSSEHashSet& operator =(TSSEHashSet&& rvalue) {
        assign(std::move(rvalue));
        return (*this);
    }

    void assign(const TSSEHashSet& other) {
        clear_ReleaseMemory();

        if (not other._size)
            return;

        Assert_NoAssume(other.num_buckets_());
        Assert_NoAssume(other._buckets != bucket_t::Sentinel());

        const u32 numBuckets = other.num_buckets_();
        const u32 allocSize = checked_cast<u32>(allocation_size_(numBuckets));

        _size = other._size;
        _bucketMask = other._bucketMask;
        _buckets = (bucket_t*)allocator_traits::Allocate(*this, allocSize).Data;

        // trivial copy, don't try to rehash
        IF_CONSTEXPR(Meta::has_trivial_copy<value_type>::value) {
            FPlatformMemory::MemcpyLarge(_buckets, other._buckets, allocSize);
        }
        else {
            std::uninitialized_copy(
                other._buckets, other._buckets + numBuckets,
                MakeCheckedIterator(_buckets, numBuckets, 0) );

            // initialize sentinel :
            m128i_epi8_store_aligned(&_buckets[numBuckets].States, m128i_epi8_set_zero());
        }

        Assert_NoAssume(_buckets[numBuckets].is_sentinel());
    }

    void assign(TSSEHashSet&& rvalue) {
        clear_ReleaseMemory();

        std::swap(_size, rvalue._size);
        std::swap(_bucketMask, rvalue._bucketMask);
        std::swap(_buckets, rvalue._buckets);
    }

    CONSTEXPR bool empty() const { return (0 == _size); }
    CONSTEXPR size_t size() const { return _size; }
    CONSTEXPR size_t capacity() const { return (num_buckets_() * bucket_t::Capacity); }

    iterator begin() { return iterator_begin_<false>(); }
    iterator end() { return iterator_end_<false>(); }

    const_iterator begin() const { return iterator_begin_<true>(); }
    const_iterator end() const { return iterator_end_<true>(); }

    TPair<iterator, bool> insert(const _Key& key) {
        return insert(_Key(key));
    }
    TPair<iterator, bool> insert(_Key&& key) {
        //INTEL_IACA_START();
        u32 h0 = u32(hasher()(key));
        m128i_t h1 = m128i_epi8_broadcast(i8(h0 & 0x7f));
        h0 = (h0 >> 7);

        m128i_t st;
        u32 bk, it;
        for (;;) {
            bk = (h0 & _bucketMask);

            Assert(Meta::IsAlignedPow2(16, &_buckets[bk].States));
            st = m128i_epi8_load_aligned(&_buckets[bk].States);

            for (bitmask_t bm{ m128i_epi8_findeq(st, h1) }; bm.Data; ) {
                it = bm.PopFront_AssumeNotEmpty();
                if (Likely(key_equal()(_buckets[bk].at(it), key)))
                    return MakePair(iterator{ &_buckets[bk], it }, false);
            }

            m128i_t mask_epi8 = m128i_epi8_tzcnt_mask(::_mm_cmplt_epi8(st, m128i_epi8_set_zero()));
            st = m128i_epi8_blend(st, h1, mask_epi8);
            it = u32(FPlatformMaths::tzcnt(m128i_epi8_tznct_mask() | mask16_t(::_mm_movemask_epi8(mask_epi8))));
            if (Likely(it < bucket_t::Capacity))
                break;

            rehash_ForCollision_();
        }
        Assert_NoAssume(bk < num_buckets_());
        Assert_NoAssume(not _buckets[bk].is_sentinel());

        m128i_epi8_store_aligned(&_buckets[bk].States, st);

        Assert_NoAssume(not _buckets[bk].is_sentinel());
        Meta::Construct(&_buckets[bk].at(it), std::move(key));

        ++_size;

        //INTEL_IACA_END();
        return MakePair(iterator{ &_buckets[bk], it }, true);
    }

    iterator insert_AssertUnique(const _Key& key) {
        return insert_AssertUnique(_Key(key));
    }
    iterator insert_AssertUnique(_Key&& key) {
        u32 h0 = u32(hasher()(key));
        m128i_t h1 = m128i_epi8_broadcast(i8(h0 & 0x7f));
        h0 = (h0 >> 7);

        m128i_t st;
        u32 bk, it;
        for (;;) {
            bk = (h0 & _bucketMask);
            st = m128i_epi8_load_aligned(&_buckets[bk].States);
            Assert_NoAssume(find_ForAssert(_buckets[bk], st, h1, key) == INDEX_NONE);

            it = u32(m128i_epi8_replace_first_assume_unique(&st, m128i_epi8_set_true(), h1));
            if (Likely(it < bucket_t::Capacity))
                break;

            rehash_ForCollision_();
        }
        Assert_NoAssume(bk < num_buckets_());
        Assert_NoAssume(not _buckets[bk].is_sentinel());

        ++_size;
        m128i_epi8_store_aligned(&_buckets[bk].States, st);
        Assert_NoAssume(not _buckets[bk].is_sentinel());
        Meta::Construct(&_buckets[bk].at(it), std::move(key));

        return iterator{ &_buckets[bk], it };
    }

    iterator find(const _Key& key) NOEXCEPT {
        u32 h0 = u32(hasher()(key));
        m128i_t h1 = m128i_epi8_broadcast(i8(h0 & 0x7f));
        h0 = (h0 >> 7);

        bucket_t* const pbucket = &_buckets[h0 & _bucketMask];
        for (bitmask_t bm{ m128i_epi8_findeq(m128i_epi8_load_aligned(&pbucket->States), h1) }; bm.Data;) {
            const u32 it = bm.PopFront_AssumeNotEmpty();
            if (Likely(key_equal()(pbucket->at(it), key)))
                return iterator{ pbucket, it };
        }

        return end();
    }

    const_iterator find(const _Key& key) const NOEXCEPT {
        return const_cast<TSSEHashSet*>(this)->find(key);
    }

    bool erase(const _Key& key) NOEXCEPT {
        u32 h0 = u32(hasher()(key));
        m128i_t h1 = m128i_epi8_broadcast(i8(h0 & 0x7f));
        h0 = (h0 >> 7);

        bucket_t* const pbucket = &_buckets[h0 & _bucketMask];

        m128i_t visited = m128i_epi8_set_zero();

        u32 it;
        m128i_t st;
        for (;;) {
            st = m128i_epi8_load_aligned(pbucket);
            it = u32(m128i_epi8_replace_first_loop(&st, &visited, h1, m128i_epi8_set_true()));
            if (it >= bucket_t::Capacity)
                return false;
            else if (Likely(key_equal()(pbucket->at(it), key)))
                break;
        }
        Assert_NoAssume(checked_cast<u32>(pbucket - _buckets) < num_buckets_());
        Assert_NoAssume(not pbucket->is_sentinel());

        --_size;
        m128i_epi8_store_aligned(&pbucket->States, st);
        Meta::Destroy(&pbucket->at(it));

        return true;
    }
    void erase(iterator it) NOEXCEPT {
        erase(const_iterator{ it });
    }
    void erase(const_iterator it) NOEXCEPT {
        Assert_NoAssume(_size);
        Assert_NoAssume(AliasesToContainer(it));

        bucket_t* const pbucket = &it.Bucket();
        const u32 e = it.Slot();

        Assert_NoAssume(e < bucket_t::Capacity);
        Assert_NoAssume(pbucket->States[e] != 0xFF);

        m128i_t st = m128i_epi8_load_aligned(&pbucket->States);
        st = m128i_epi8_remove_at(st, e);
        m128i_epi8_store_aligned(&pbucket->States, st);

        --_size;
        Meta::Destroy(&pbucket->at(e));
    }

    FORCE_INLINE void clear() {
        if (_size)
            clear_NotEmpty_();
    }

    FORCE_INLINE void clear_ReleaseMemory() {
        if (has_content_())
            releaseMemory_ForClear_();
    }

    FORCE_INLINE void reserve_Additional(size_t n) { reserve(_size + n); }
    void reserve(size_t n) {
        Assert(n);

        n = size_with_slack_(n);
        if (n > capacity())
            rehash(n);
    }

    FORCE_INLINE void rehash(size_t n) {
        if (_size <= n)
            rehash_ForGrowth_(n);
    }

#if USE_PPE_ASSERT
    template <bool _Const>
    bool AliasesToContainer(const TSSEHashIterator<value_type, _Const>& it) const {
        return (FPlatformMemory::Memoverlap(
            it.Bucket(), sizeof(bucket_t),
            _buckets, num_buckets_() * sizeof(bucket_t)));
    }
#endif

private:
    using bitmask_t = typename bucket_t::bitmask_t;

    STATIC_ASSERT(Meta::has_trivial_destructor<hasher>::value);
    STATIC_CONST_INTEGRAL(u32, FillRatio, 70); // 70% filled <=> 30% slack
    STATIC_CONST_INTEGRAL(u32, SlackFactor, ((100 + (100 - FillRatio)) * 128) / 100);

    u32 _size;
    u32 _bucketMask;
    bucket_t* _buckets;

    static CONSTEXPR size_t size_with_slack_(size_t n) NOEXCEPT {
        return ((n * SlackFactor) / 128);
    }

    CONSTEXPR bool has_content_() const NOEXCEPT {
        return (_bucketMask | _size);
    }
    CONSTEXPR u32 num_buckets_() const NOEXCEPT {
        return (bucket_t::Sentinel() != _buckets ? _bucketMask + 1 : 0);
    }
    CONSTEXPR static size_t allocation_size_(size_t numBuckets) NOEXCEPT {
        return (numBuckets * sizeof(bucket_t) + sizeof(FSSEHashStates)/* sentinel */);
    }

    NO_INLINE void rehash_ForCollision_() {
        rehash_(Max(u32(1), num_buckets_() * 2) * bucket_t::Capacity);
    }
    NO_INLINE void rehash_ForGrowth_(size_t n) {
        rehash_(n);
    }

    FORCE_INLINE void rehash_(size_t n) {
        Assert_NoAssume(_size <= n);

        u32 const oldBucketMask = _bucketMask;
        bucket_t* const oldBuckets = _buckets;

        const u32 numBuckets = FPlatformMaths::NextPow2(
            checked_cast<u32>((n + bucket_t::Capacity - 1) / bucket_t::Capacity));
        Assert(numBuckets);
        Assert_NoAssume(Meta::IsPow2(numBuckets));

        _bucketMask = (numBuckets - 1);
        _buckets = (bucket_t*)allocator_traits::Allocate(*this, allocation_size_(numBuckets)).Data;

        forrange(pbucket, _buckets, _buckets + numBuckets)
            INPLACE_NEW(pbucket, bucket_t);

        // initialize sentinel :
        m128i_epi8_store_aligned(&_buckets[numBuckets].States, m128i_epi8_set_zero());
        Assert_NoAssume(_buckets[numBuckets].is_sentinel());

        Assert_NoAssume(n <= capacity());

        if (_size) {
            Assert(oldBuckets);

            ONLY_IF_ASSERT(const u32 oldSize = _size);
            const u32 oldNumBuckets = (oldBucketMask + 1);

            _size = 0; // reset size before inserting back all elements

            forrange(pbucket, oldBuckets, oldBuckets + oldNumBuckets) {
                for (bitmask_t bm{ pbucket->each() }; bm.Data; ) {
                    const u32 it = bm.PopFront_AssumeNotEmpty();
                    value_type* const pkey = &pbucket->at(it);
                    insert_AssertUnique(std::move(*pkey));
                    Meta::Destroy(pkey);
                }
            }

            Assert_NoAssume(oldSize == _size);
        }

        if (oldBucketMask | _size) {
            Assert_NoAssume(not oldBuckets->is_sentinel());
            const u32 oldNumBuckets = (oldBucketMask + 1);
            allocator_traits::Deallocate(*this, FAllocatorBlock{ oldBuckets, allocation_size_(oldNumBuckets) });
        }
        else {
            Assert_NoAssume(bucket_t::Sentinel() == oldBuckets);
        }
    }

    NO_INLINE void clear_NotEmpty_() NOEXCEPT {
        Assert_NoAssume(_bucketMask);

        _size = 0;
        forrange(pbucket, _buckets, _buckets + num_buckets_())
            pbucket->clear();
    }

    NO_INLINE void releaseMemory_ForDtor_() NOEXCEPT {
        releaseMemory_NotEmpty_LeaveDirty_();
    }
    NO_INLINE void releaseMemory_ForClear_() NOEXCEPT {
        releaseMemory_NotEmpty_LeaveDirty_();

        _size = 0;
        _bucketMask = 0;
        _buckets = bucket_t::Sentinel();
    }

    FORCE_INLINE void releaseMemory_NotEmpty_LeaveDirty_() NOEXCEPT {
        Assert_NoAssume(has_content_());

        IF_CONSTEXPR(not Meta::has_trivial_destructor<value_type>::value) {
            forrange(pbucket, _buckets, _buckets + num_buckets_())
                pbucket->clear_LeaveDirty();
        }

        allocator_traits::Deallocate(*this, FAllocatorBlock{ _buckets, allocation_size_(num_buckets_()) });
    }

    template <bool _Const>
    FORCE_INLINE TSSEHashIterator<value_type, _Const> iterator_begin_() const NOEXCEPT {
        TSSEHashIterator<value_type, _Const> it{ Meta::NoInit };
        if (_size) {
            it.BucketAndSlot.Reset(_buckets, 0);
            it.FirstSet(i8(-1));
        }
        else {
            // avoids traversing empty containers with reserved memory
            it.BucketAndSlot.Reset(_buckets + num_buckets_(), 0);
        }
        return it;
    }

    template <bool _Const>
    FORCE_INLINE TSSEHashIterator<value_type, _Const> iterator_end_() const NOEXCEPT {
        return TSSEHashIterator<value_type, _Const>{ _buckets + num_buckets_(), 0 };
    }

#if USE_PPE_ASSERT
    static size_t VECTORCALL find_ForAssert(const bucket_t& bucket, const m128i_t& st, const m128i_t& h16, const value_type& x) NOEXCEPT {
        for (bitmask_t bm{ m128i_epi8_findeq(st, h16) }; bm.Data; ) {
            const u32 e = bm.PopFront_AssumeNotEmpty();
            if (key_equal()(bucket.at(e), x))
                return e;
        }
        return INDEX_NONE;
    }
#endif
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
