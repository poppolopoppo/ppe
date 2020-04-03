#pragma once

#include "Allocator/Allocation.h"
#include "Container/BitMask.h"
#include "HAL/PlatformMaths.h"
#include "HAL/PlatformMemory.h"
#include "Memory/MemoryView.h"
#include "Meta/AlignedStorage.h"
#include "Meta/PointerWFlags.h"
#include "Meta/Iterator.h"

#include "Maths/SSEHelpers.h"

#if USE_PPE_AVX2

#define USE_PPE_SSEHASHSET3_MICROPROFILING 0 //%_NOCOMMIT%
#if USE_PPE_SSEHASHSET3_MICROPROFILING
#   define PPE_SSEHASHSET3_MICROPROFILING NO_INLINE
#else
#   define PPE_SSEHASHSET3_MICROPROFILING
#endif

// using AVX256

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
using FSSEHashStates3 = Meta::TArray<i8, 32>;
//----------------------------------------------------------------------------
ALIGN(32) CONSTEXPR const FSSEHashStates3 GSimdBucketSentinel3{ 0 };
//----------------------------------------------------------------------------
template <typename T>
struct ALIGN(32) TSSEHashBucket3 {
    STATIC_CONST_INTEGRAL(u32, Capacity, 32);
    using bitmask_t = TBitMask<mask32_t>;
    using states_t = FSSEHashStates3;
    using storage_t = POD_STORAGE(T);

    states_t States;
    storage_t Items[Capacity];

    FORCE_INLINE TSSEHashBucket3() noexcept {
        STATIC_ASSERT(sizeof(states_t) == sizeof(m256i_t));
        STATIC_ASSERT(sizeof(storage_t) == sizeof(T));
        STATIC_ASSERT(sizeof(*this) == sizeof(T) * Capacity + sizeof(m256i_t));
        m256i_epi8_store_aligned(&States, m256i_epi8_set_true());
    }

    FORCE_INLINE ~TSSEHashBucket3() {
        clear_LeaveDirty();
    }

    FORCE_INLINE TSSEHashBucket3(const TSSEHashBucket3& other) {
        copy_AssumeEmpty(other);
    }
    FORCE_INLINE TSSEHashBucket3& operator =(const TSSEHashBucket3& other) {
        clear_LeaveDirty();
        copy_AssumeEmpty(other);
        return (*this);
    }

    FORCE_INLINE TSSEHashBucket3(TSSEHashBucket3&& rvalue) {
        move_AssumeEmpty(std::move(rvalue));
    }
    FORCE_INLINE TSSEHashBucket3& operator =(TSSEHashBucket3&& rvalue) {
        clear_LeaveDirty();
        move_AssumeEmpty(std::move(rvalue));
        return (*this);
    }

#if USE_PPE_DEBUG
    bool is_sentinel() const {
        m256i_t st = m256i_epi8_load_aligned(&States);
        const u32 bm = m256i_epi8_findeq(st, m256i_epi8_set_zero());
        return (0xFFFFFFFFu == bm);
    }
#endif

    FORCE_INLINE T& at(u32 i) NOEXCEPT {
        Assert(i < Capacity);
        return (reinterpret_cast<T&>(Items[i]));
    }
    FORCE_INLINE const T& at(u32 i) const NOEXCEPT {
        Assert(i < Capacity);
        return (reinterpret_cast<const T&>(Items[i]));
    }

    FORCE_INLINE void clear_LeaveDirty() NOEXCEPT {
        IF_CONSTEXPR(not Meta::has_trivial_destructor<T>::value) {
            const m256i_t st = m256i_epi8_load_aligned(&States);
            for (bitmask_t bm{ m256i_epi8_findge(st, m256i_epi8_set_zero()) }; bm.Data; )
                Meta::Destroy(&at(bm.PopFront_AssumeNotEmpty()));
        }
    }

    FORCE_INLINE void clear() NOEXCEPT {
        clear_LeaveDirty();
        m256i_epi8_store_aligned(&States, m256i_epi8_set_true());
    }

    FORCE_INLINE void copy_AssumeEmpty(const TSSEHashBucket3& other) {
        const m256i_t st = m256i_epi8_load_aligned(&other.States);
        for (bitmask_t bm{ m256i_epi8_findge(st, m256i_epi8_set_zero()) }; bm.Data; ) {
            const u32 e = bm.PopFront_AssumeNotEmpty();
            Meta::Construct(&at(e), other.at(e));
        }
        m256i_epi8_store_aligned(&States, st);
    }

    FORCE_INLINE void move_AssumeEmpty(TSSEHashBucket3&& rvalue) NOEXCEPT {
        const m256i_t st = m256i_epi8_load_aligned(&rvalue.States);
        for (bitmask_t bm{ m256i_epi8_findge(st, m256i_epi8_set_zero()) }; bm.Data; ) {
            const u32 e = bm.PopFront_AssumeNotEmpty();
            Meta::Construct(&at(e), std::move(rvalue.at(e)));
            Meta::Destroy(&rvalue.at(e));
        }
        m256i_epi8_store_aligned(&States, st);
        m256i_epi8_store_aligned(&rvalue.States, m256i_epi8_set_true());
    }

    FORCE_INLINE static TSSEHashBucket3* Sentinel() NOEXCEPT {
        // we guarantee we won't access Items, so use a generic sentinel (avoid bloating rdata)
        return (TSSEHashBucket3*)(&GSimdBucketSentinel);
    }
};
//----------------------------------------------------------------------------
template <typename T, bool _Const>
struct TSSEHashIterator3 : Meta::TIterator<Meta::TConditional<_Const, Meta::TAddConst<T>, T>> {
    using bucket_t = TSSEHashBucket3<T>;
    using bitmask_t = typename bucket_t::bitmask_t;
    using parent_t = Meta::TIterator<Meta::TConditional<_Const, Meta::TAddConst<T>, T>>;

    using typename parent_t::iterator_category;
    using typename parent_t::value_type;
    using typename parent_t::difference_type;
    using typename parent_t::pointer;
    using typename parent_t::reference;

    bucket_t* pBucket;
    u32 iSlot;

    CONSTEXPR TSSEHashIterator3(Meta::FNoInit) NOEXCEPT {}

    CONSTEXPR TSSEHashIterator3(bucket_t* bucket, u32 slot) NOEXCEPT
    :	pBucket(bucket)
    ,	iSlot(slot)
    {}

    template <bool _Other>
    CONSTEXPR TSSEHashIterator3(const TSSEHashIterator3<T, _Other>& other) NOEXCEPT
    :	pBucket(other.pBucket)
    ,	iSlot(other.iSlot)
    {}
    template <bool _Other>
    CONSTEXPR TSSEHashIterator3& operator =(const TSSEHashIterator3<T, _Other>& other) NOEXCEPT {
        pBucket = other.pBucket;
        iSlot = other.iSlot;
        return (*this);
    }

    bucket_t& Bucket() const NOEXCEPT { return (*pBucket); }
    u32 Slot() const NOEXCEPT { return iSlot; }

    TSSEHashIterator3& operator++() { Advance(); return (*this); }
    TSSEHashIterator3& operator++(int) { TSSEHashIterator3 tmp(*this); Advance(); return tmp; }

    reference operator *() const { return pBucket->at(iSlot); }
    pointer operator ->() const { return (&operator *()); }

    CONSTEXPR inline friend bool operator ==(const TSSEHashIterator3& lhs, const TSSEHashIterator3& rhs) NOEXCEPT {
        return (lhs.pBucket == rhs.pBucket && lhs.iSlot == rhs.iSlot);
    }
    CONSTEXPR inline friend bool operator !=(const TSSEHashIterator3& lhs, const TSSEHashIterator3& rhs) NOEXCEPT {
        return (not operator ==(lhs, rhs));
    }

    void FirstSet(i8 slot) NOEXCEPT {
        bucket_t* pbucket = pBucket;
        for (Assert(pbucket);; ++pbucket, slot = -1) {
            const m256i_t st = m256i_epi8_load_aligned(&pbucket->States);
            const bitmask_t bm{ m256i_epi8_findge(st, m256i_epi8_set_zero()) & (u32(0xFFFFFFFF) << (slot + 1)) };
            if (Likely(bm.Data)) {
                pBucket = pbucket;
                iSlot = u32(bm.FirstBitSet_AssumeNotEmpty());
                return;
            }
        }
    }

    FORCE_INLINE void Advance() NOEXCEPT {
        FirstSet(checked_cast<i8>(iSlot));
    }

};
//----------------------------------------------------------------------------
template <
    typename _Key
,   typename _Hash = Meta::THash<_Key>
,   typename _EqualTo = Meta::TEqualTo<_Key>
,   typename _Allocator = ALLOCATOR(Container)
>   class TSSEHashSet3 : _Allocator {
public:
    typedef _Key value_type;
    typedef _Hash hasher;
    typedef _EqualTo key_equal;

    using allocator_type = _Allocator;
    using allocator_traits = TAllocatorTraits<allocator_type>;

    using bucket_t = TSSEHashBucket3<value_type>;

    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef Meta::TAddPointer<value_type> pointer;
    typedef Meta::TAddPointer<const value_type> const_pointer;

    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

    using iterator = TSSEHashIterator3<value_type, false>;
    using const_iterator = TSSEHashIterator3<value_type, true>;

    CONSTEXPR TSSEHashSet3() NOEXCEPT
    :   _size(0)
    ,   _bucketMask(0)
    ,   _buckets(bucket_t::Sentinel())
    {}

    FORCE_INLINE ~TSSEHashSet3() {
        if (has_content_())
            releaseMemory_ForDtor_();
    }

    FORCE_INLINE TSSEHashSet3(const TSSEHashSet3& other) : TSSEHashSet3() {
        assign(other);
    }
    FORCE_INLINE TSSEHashSet3& operator =(const TSSEHashSet3& other) {
        assign(other);
        return (*this);
    }

    CONSTEXPR TSSEHashSet3(TSSEHashSet3&& rvalue) NOEXCEPT
    :   TSSEHashSet3() {
        std::swap(_size, rvalue._size);
        std::swap(_bucketMask, rvalue._bucketMask);
        std::swap(_buckets, rvalue._buckets);
    }
    FORCE_INLINE TSSEHashSet3& operator =(TSSEHashSet3&& rvalue) {
        assign(std::move(rvalue));
        return (*this);
    }

    void assign(const TSSEHashSet3& other) {
        Assert(&other != this);

        if (not other._size) {
            clear();
            return;
        }

        Assert_NoAssume(other.num_buckets_());
        Assert_NoAssume(other._buckets != bucket_t::Sentinel());

        const u32 numBuckets = other.num_buckets_();
        const u32 allocSize = checked_cast<u32>(allocation_size_(numBuckets));

        if (num_buckets_() != other.num_buckets_()) {
            if (has_content_())
                releaseMemory_ForDtor_();

            _bucketMask = other._bucketMask;
            _buckets = (bucket_t*)allocator_traits::Allocate(*this, allocSize).Data;
        }
        else {
            IF_CONSTEXPR(not Meta::has_trivial_destructor<value_type>::value) {
                clear();
            }

            Assert_NoAssume(other._bucketMask == _bucketMask);
        }

        _size = other._size;

        // trivial copy, don't try to rehash
        IF_CONSTEXPR(Meta::has_trivial_copy<value_type>::value) {
            FPlatformMemory::MemcpyLarge(_buckets, other._buckets, allocSize);
        }
        else {
            std::uninitialized_copy(
                other._buckets, other._buckets + numBuckets,
                MakeCheckedIterator(_buckets, numBuckets, 0) );

            // initialize sentinel :
            m256i_epi8_store_aligned(&_buckets[numBuckets].States, m256i_epi8_set_zero());
        }

        Assert_NoAssume(_buckets[numBuckets].is_sentinel());
    }

    void assign(TSSEHashSet3&& rvalue) {
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

    PPE_SSEHASHSET3_MICROPROFILING TPair<iterator, bool> insert(_Key&& rkey) {
        const u32 hh = u32(hasher()(rkey));
        const u32 h0 = (hh >> 7);

        FPlatformMemory::ReadPrefetch(_buckets, h0 & _bucketMask);

        const i8 h1 = i8(hh & 0x7f);

        for (;;) {
            bucket_t* const pbucket = _buckets + (h0 & _bucketMask);

            const m256i_t ex{ m256i_epi8_broadcast(h1) };
            const m256i_t st{ m256i_epi8_load_aligned(pbucket) };
            const bitmask_t free{ m256i_epi8_findlt(st, m256i_epi8_set_zero()) };

            for (bitmask_t exist{ m256i_epi8_findeq(st, ex) }; exist; ) {
                const u32 e = u32(exist.PopFront_AssumeNotEmpty());
                if (Likely(key_equal()(pbucket->at(e), rkey)))
                    return MakePair(iterator{ pbucket, e }, false);
            }

            if (Likely(free)) {
                const u32 e = u32(free.FirstBitSet_AssumeNotEmpty());
                Assert_NoAssume(not pbucket->is_sentinel());

                ++_size;
                pbucket->States[e] = h1;
                Assert_NoAssume(not pbucket->is_sentinel());

                Meta::Construct(&pbucket->at(e), std::move(rkey));
                return MakePair(iterator{ pbucket, e }, true);
            }
            else {
                rehash_ForCollision_();
            }
        }
    }
    FORCE_INLINE TPair<iterator, bool> insert(const _Key& key) {
        return insert(_Key(key));
    }

    iterator insert_AssertUnique(_Key&& key) {
        const u32 h0 = u32(hasher()(key));
        const m256i_t h1 = m256i_epi8_broadcast(u8(h0 & 0x7f));

        u32 e;
    RETRY_INSERT:
        u32 bk = ((h0 >> 7) & _bucketMask);

        const m256i_t st = m256i_epi8_load_aligned(&_buckets[bk].States);
        Assert_NoAssume(find_ForAssert(_buckets[bk], st, h1, key) == INDEX_NONE);

        if (const bitmask_t bm{ m256i_epi8_findlt(st, m256i_epi8_set_zero()) }) {
            e = u32(bm.FirstBitSet_AssumeNotEmpty());
        }
        else {
            rehash_ForCollision_();
            goto RETRY_INSERT;
        }

        Assert_NoAssume(bk < num_buckets_());
        Assert(e < bucket_t::Capacity);
        Assert_NoAssume(not _buckets[bk].is_sentinel());

        ++_size;
        _buckets[bk].States[e] = u8(h0 & 0x7f);
        Assert_NoAssume(not _buckets[bk].is_sentinel());
        Meta::Construct(&_buckets[bk].at(e), std::move(key));

        return iterator{ &_buckets[bk], e };
    }
    iterator insert_AssertUnique(const _Key& key) {
        return insert_AssertUnique(_Key(key));
    }

    PPE_SSEHASHSET3_MICROPROFILING iterator find(const _Key& key) NOEXCEPT {
        const u32 h0 = u32(hasher()(key));

        bucket_t* const pbucket = &_buckets[(h0 >> 7) & _bucketMask];
        for (bitmask_t bm{ m256i_epi8_findeq(
            m256i_epi8_load_aligned(&pbucket->States),
            m256i_epi8_broadcast(u8(h0 & 0x7f))) }; bm.Data;) {
            const u32 e = bm.PopFront_AssumeNotEmpty();
            if (Likely(key_equal()(pbucket->at(e), key)))
                return iterator{ pbucket, u32(e) };
        }

        return end();
    }
    FORCE_INLINE const_iterator find(const _Key& key) const NOEXCEPT {
        return const_cast<TSSEHashSet3*>(this)->find(key);
    }

    PPE_SSEHASHSET3_MICROPROFILING bool erase(const _Key& key) NOEXCEPT {
        const u32 h0 = u32(hasher()(key));

        bucket_t* const pbucket = &_buckets[(h0 >> 7) & _bucketMask];
        for (bitmask_t bm{ m256i_epi8_findeq(
            m256i_epi8_load_aligned(&pbucket->States),
            m256i_epi8_broadcast(i8(h0 & 0x7f))) }; bm.Data;) {
            const u32 e = bm.PopFront_AssumeNotEmpty();

            pointer const pkey = &pbucket->at(e);
            if (Likely(key_equal()(*pkey, key))) {
                Assert(_size);

                --_size;
                pbucket->States[e] = u8(0xff); // mark as deleted
                Meta::Destroy(pkey);

                return true;
            }
        }

        return false;
    }
    FORCE_INLINE void erase(iterator it) NOEXCEPT {
        erase(const_iterator{ it });
    }
    void erase(const_iterator it) NOEXCEPT {
        Assert(_size);
        Assert_NoAssume(AliasesToContainer(it));

        bucket_t* const pbucket = &it.Bucket();
        const u32 e = it.Slot();

        Assert_NoAssume(e < bucket_t::Capacity);
        Assert_NoAssume(pbucket->States[e] != u8(0xff));

        --_size;
        pbucket->States[e] = u8(0xff); // mark as deleted
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
    bool AliasesToContainer(const TSSEHashIterator3<value_type, _Const>& it) const {
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
        return (numBuckets * sizeof(bucket_t) + sizeof(FSSEHashStates3)/* sentinel */);
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
        m256i_epi8_store_aligned(&_buckets[numBuckets].States, m256i_epi8_set_zero());
        Assert_NoAssume(_buckets[numBuckets].is_sentinel());

        Assert_NoAssume(n <= capacity());

        if (_size) {
            Assert(oldBuckets);

            ONLY_IF_ASSERT(const u32 oldSize = _size);
            const u32 oldNumBuckets = (oldBucketMask + 1);

            _size = 0; // reset size before inserting back all elements

            forrange(pbucket, oldBuckets, oldBuckets + oldNumBuckets) {
                const m256i_t st = m256i_epi8_load_aligned(&pbucket->States);
                for (bitmask_t bm{ m256i_epi8_findge(st, m256i_epi8_set_zero()) }; bm.Data; ) {
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
    FORCE_INLINE TSSEHashIterator3<value_type, _Const> iterator_begin_() const NOEXCEPT {
        TSSEHashIterator3<value_type, _Const> it{ Meta::NoInit };
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
    FORCE_INLINE TSSEHashIterator3<value_type, _Const> iterator_end_() const NOEXCEPT {
        return TSSEHashIterator3<value_type, _Const>{ _buckets + num_buckets_(), 0 };
    }

#if USE_PPE_ASSERT
    static size_t VECTORCALL find_ForAssert(const bucket_t& bucket, const m256i_t& st, const m256i_t& h16, const value_type& x) NOEXCEPT {
        for (bitmask_t bm{ m256i_epi8_findeq(st, h16) }; bm.Data; ) {
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

#endif //!USE_PPE_AVX2
