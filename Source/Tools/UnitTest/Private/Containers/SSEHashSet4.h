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

#define USE_PPE_SSEHASHSET4_MICROPROFILING 0 //%_NOCOMMIT%
#if USE_PPE_SSEHASHSET4_MICROPROFILING
#   define PPE_SSEHASHSET4_MICROPROFILING NO_INLINE
#else
#   define PPE_SSEHASHSET4_MICROPROFILING
#endif

// using AVX256

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FSimdHashData {
    STATIC_CONST_INTEGRAL(u32, Capacity, 16);
    using state_t = i8;
    using bmask_t = TBitMask<u32>;

    enum : state_t {
        kEmpty = -1,
        kSentinel = 0,
    };

    STATIC_CONST_INTEGRAL(u32, FillRatio, 70); // 70% filled <=> 30% slack
    STATIC_CONST_INTEGRAL(u32, SlackFactor, ((100 + (100 - FillRatio)) * 128) / 100);

    static ALIGN(16) CONSTEXPR const Meta::TArray<state_t, Capacity> GSentinel{ kSentinel };

    u32 Size;
    u32 CapacityM1;
    void* StatesAndBuckets;

    CONSTEXPR FSimdHashData(Meta::FNoInit) NOEXCEPT {}
    CONSTEXPR FSimdHashData() NOEXCEPT
    :   Size(0)
    ,	CapacityM1(0)
    ,	StatesAndBuckets(const_cast<void*>(&GSentinel))
    {}

    CONSTEXPR FSimdHashData(FSimdHashData&& rvalue)
    :	FSimdHashData() {
        Swap(rvalue);
    }

    u32 Capacity() const { return (CapacityM1 ? CapacityM1 + 1 : 0); }
    void* Buckets() const { return States(Capacity()); }
    state_t* States(u32 i) const { return (static_cast<state_t*>(StatesAndBuckets) + i); }

    void Swap(FSimdHashData& rvalue) {
        using std::swap;
        swap(Size, rvalue.Size);
        swap(CapacityM1, rvalue.CapacityM1);
        swap(StatesAndBuckets, rvalue.StatesAndBuckets);
    }

    template <typename T>
    CONSTEXPR size_t AllocSize() NOEXCEPT {
        return ((sizeof(T) + sizeof(state_t)) * Capacity());
    }

    CONSTEXPR static size_t SizeWSlack(size_t n) NOEXCEPT {

    }

    template <typename T, typename _Allocator>
    void Allocate(_Allocator& al, size_t capacity) {
        using alloc_traits = TAllocatorTraits<_Allocator>;
        Assert(capacity);
        Assert(nullptr == StatesAndBuckets);
        Assert_NoAssume(Meta::IsPow2(capacity));

        CapacityM1 = (capacity - 1);
        StatesAndBuckets = alloc_traits::Allocate(al, AllocSize<T>()).Data;
    }

    template <typename T, typename _Allocator>
    void Deallocate(_Allocator& al) {
        using alloc_traits = TAllocatorTraits<_Allocator>;
        Assert(CapacityM1);
        Assert(StatesAndBuckets);

        alloc_traits::Deallocate(al, FAllocatorBlock{ StatesAndBuckets, AllocSize<T>() );
    }

    void ResetStates() {
        for(m128i_t* it = (m128i_t*)StatesAndBuckets,
            m128i_t* const iend = (it + Capacity());
            it != iend; ++it) {
            m128i_epi8_store_aligned(it, m128i_epi8_set_true());
        }
    }

    template <typename T>
    void Destroy() NOEXCEPT {
        Assert(CapacityM1);
        Assert(StatesAndBuckets);

        IF_CONSTEXPR(not Meta::has_trivial_destructor<T>::value) {
#if USE_PPE_ASSERT // reset both states and values in debug
            const size_t sizeInBytes = AllocSize<T>();
#else // reset only states otherwise
            const size_t sizeInBytes = (sizeof(state_t) * Capacity());
#endif
            FPlatformMemory::Memset(StatesAndBuckets, kEmpty, sizeInBytes);
        }
        else {
            for(m128i_t* const it = (m128i_t*)StatesAndBuckets,
                m128i_t* const iend = (it + Capacity());
                it != iend; ++it ) {
                const u32 off = checked_cast<u32>((state_t*)it - (state_t*)StatesAndBuckets);
                const m128i_t st = m128i_epi8_load_stream(it);
                for(bmask_t used{ m128i_epi8_findge(st, m128i_epi8_set_zero()) }; used.Data; ) {
                    const u32 e = used.PopFront_AssumeNotEmpty();
                    Meta::Destroy((T*)Buckets() + (off + e));
                }
                m128i_epi8_store_aligned(it, m128i_epi8_set_true());
            }
        }
    }

    template <typename T>
    void Copy(const FSimdHashData& other) NOEXCEPT {
        Assert(CapacityM1 == other.CapacityM1);

        Size = rvalue.Size;

        IF_CONSTEXPR(Meta::has_trivial_copy<T>::value) {
            FPlatformMemory::MemcpyLarge(StatesAndBuckets, other.StatesAndBuckets, AllocSize<T>());
        }
        else {
            for(m128i_t const* psrc = (m128i_t const*)StatesAndBuckets,
                m128i_t* pdst = (m128i_t*)other.StatesAndBuckets,
                m128i_t* const pend = (pdst + Capacity());
                pdst != pend; ++psrc, ++pdst ) {
                const u32 off = checked_cast<u32>((state_t*)pdst - (state_t*)StatesAndBuckets);
                const m128i_t st = m128i_epi8_load_stream(psrc);
                for (bmask_t used{ m128i_epi8_findge(st, m128i_epi8_set_zero()) }; used.Data; ) {
                    const u32 e = used.PopFront_AssumeNotEmpty();
                    Meta::Construct((T*)Buckets() + (off + e), (const T*)other.Buckets() + (off + e));
                }
                m128i_epi8_store_stream(pdst, st);
            }
        }
    }

    template <typename T>
    void Move(FSimdHashData&& rvalue) NOEXCEPT {
        Assert(0 == Size);
        Assert(CapacityM1 == rvalue.CapacityM1);

        Size = rvalue.Size;
        rvalue.Size = 0;

        IF_CONSTEXPR(Meta::has_trivial_move<T>::value) {
            FPlatformMemory::MemcpyLarge(StatesAndBuckets, rvalue.StatesAndBuckets, AllocSize<T>());
            rvalue.Destroy();
        }
        else {
            for(m128i_t* psrc = (m128i_t*)StatesAndBuckets,
                m128i_t* pdst = (m128i_t*)rvalue.StatesAndBuckets,
                m128i_t* const pend = (pdst + Capacity());
                pdst != pend; ++psrc, ++pdst ) {
                const u32 off = checked_cast<u32>((state_t*)pdst - (state_t*)StatesAndBuckets);
                const m128i_t st = m128i_epi8_load_stream(psrc);
                for (bmask_t used{ m128i_epi8_findge(st, m128i_epi8_set_zero()) }; used.Data; ) {
                    const u32 e = used.PopFront_AssumeNotEmpty();
                    T* const tsrc = ((T*)rvalue.Buckets()) + (off + e);
                    Meta::Construct((T*)Buckets() + (off + e), std::move(*tsrc));
                    Meta::Destroy(tsrc);
                }
                m128i_epi8_store_stream(pdst, st);
                m128i_epi8_store_aligned(psrc, m128i_epi8_set_true());
            }
        }
    }
};
//----------------------------------------------------------------------------
template <typename T, bool _Const>
struct TSSEHashIterator4 : Meta::TIterator<Meta::TConditional<_Const, Meta::TAddConst<T>, T>> {
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
    :	pBucket(bucket);
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
>   class TSimdHashSet4 : _Allocator {
public:
    typedef _Key value_type;
    typedef _Hash hasher;
    typedef _EqualTo key_equal;

    using allocator_type = _Allocator;
    using allocator_traits = TAllocatorTraits<allocator_type>;

    using data_t = FSimdHashData;

    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef Meta::TAddPointer<value_type> pointer;
    typedef Meta::TAddPointer<const value_type> const_pointer;

    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

    using iterator = TSimdHashIterator4<value_type, false>;
    using const_iterator = TSimdHashIterator4<value_type, true>;

    CONSTEXPR TSimdHashSet4() NOEXCEPT {}
    CONSTEXPR TSimdHashSet4(Meta::FForceInit fi) NOEXCEPT : _data(fi) {}

    FORCE_INLINE ~TSimdHashSet4() {
        if (has_content_())
            releaseMemory_ForDtor_();
    }

    FORCE_INLINE TSimdHashSet4(const TSimdHashSet4& other) : TSimdHashSet4() {
        assign(other);
    }
    FORCE_INLINE TSimdHashSet4& operator =(const TSimdHashSet4& other) {
        assign(other);
        return (*this);
    }

    CONSTEXPR TSimdHashSet4(TSimdHashSet4&& rvalue) NOEXCEPT
    :   _data(std::move(rvalue.data))
    {}
    FORCE_INLINE TSimdHashSet4& operator =(TSimdHashSet4&& rvalue) {
        assign(std::move(rvalue));
        return (*this);
    }

    void assign(const TSimdHashSet4& other) {
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

    void assign(TSimdHashSet4&& rvalue) {
        clear_ReleaseMemory();
        _data.Swap(rvalue._data);
    }

    CONSTEXPR bool empty() const { return (0 == _size); }
    CONSTEXPR size_t size() const { return _size; }
    CONSTEXPR size_t capacity() const { return (num_buckets_() * bucket_t::Capacity); }

    iterator begin() { return iterator_begin_<false>(); }
    iterator end() { return iterator_end_<false>(); }

    const_iterator begin() const { return iterator_begin_<true>(); }
    const_iterator end() const { return iterator_end_<true>(); }

    PPE_SSEHASHSET4_MICROPROFILING TPair<iterator, bool> insert(_Key&& rkey) {
        const u32 hh = u32(hasher()(rkey));
        const u32 h0 = (hh >> 7);
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

    PPE_SSEHASHSET4_MICROPROFILING iterator find(const _Key& key) NOEXCEPT {
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

    PPE_SSEHASHSET4_MICROPROFILING bool erase(const _Key& key) NOEXCEPT {
        const u32 h0 = u32(hasher()(key));

        bucket_t* const pbucket = &_buckets[(h0 >> 7) & _bucketMask];
        for (bitmask_t bm{ m256i_epi8_findeq(
            m256i_epi8_load_aligned(&pbucket->States),
            m256i_epi8_broadcast(u8(h0 & 0x7f))) }; bm.Data;) {
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
