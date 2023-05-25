#pragma once

#include "Allocator/Allocation.h"
#include "HAL/PlatformMaths.h"
#include "HAL/PlatformMemory.h"
#include "Maths/SSEHelpers.h"
#include "Memory/MemoryView.h"
#include "Meta/AlignedStorage.h"
#include "Meta/PointerWFlags.h"
#include "Meta/Iterator.h"

#include "Intel_IACA.h"

#define USE_PPE_SSEHASHSET2_MICROPROFILING 0 //%_NOCOMMIT%
#if USE_PPE_SSEHASHSET2_MICROPROFILING
#   define PPE_SSEHASHSET2_MICROPROFILING NO_INLINE
#else
#   define PPE_SSEHASHSET2_MICROPROFILING
#endif

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum ESSEHashState2 : i8 {
    SSEHASHSET2_kSentinel = 0,
    SSEHASHSET2_kDeleted = -1,
};
//----------------------------------------------------------------------------
using FSSEHashStates2 = Meta::TArray<ESSEHashState2, 16>;
//----------------------------------------------------------------------------
ALIGN(16) CONSTEXPR const FSSEHashStates2 GSimdBucketSentinel2{
    SSEHASHSET2_kSentinel, SSEHASHSET2_kSentinel, SSEHASHSET2_kSentinel, SSEHASHSET2_kSentinel,
    SSEHASHSET2_kSentinel, SSEHASHSET2_kSentinel, SSEHASHSET2_kSentinel, SSEHASHSET2_kSentinel,
    SSEHASHSET2_kSentinel, SSEHASHSET2_kSentinel, SSEHASHSET2_kSentinel, SSEHASHSET2_kSentinel,
    SSEHASHSET2_kSentinel, SSEHASHSET2_kSentinel, SSEHASHSET2_kSentinel, SSEHASHSET2_kSentinel,
};
//----------------------------------------------------------------------------
template <typename T>
struct ALIGN(16) TSSEHashBucket2 {
    using index_t = unsigned long;
    using mask_t = size_t;
    using word_t = size_t;
    using state_t = ESSEHashState2;
    using states_t = FSSEHashStates2;
    using storage_t = POD_STORAGE(T);

    STATIC_CONST_INTEGRAL(word_t, Capacity, 16);
    STATIC_CONST_INTEGRAL(word_t, CapacityMask, Capacity - 1);
    STATIC_CONST_INTEGRAL(word_t, HashMask, (sizeof(size_t) << 3) - 1);

    STATIC_CONST_INTEGRAL(state_t, kDeleted, SSEHASHSET2_kDeleted);

    states_t States;
    storage_t Items[Capacity];

    FORCE_INLINE TSSEHashBucket2() noexcept {
        STATIC_ASSERT(sizeof(states_t) == sizeof(m128i_t));
        STATIC_ASSERT(sizeof(storage_t) == sizeof(T));
        STATIC_ASSERT(sizeof(*this) == sizeof(T) * Capacity + sizeof(m128i_t));
        m128i_epi8_store_aligned(&States, m128i_epi8_set_true());
    }

    FORCE_INLINE ~TSSEHashBucket2() {
        clear_LeaveDirty();
    }

    FORCE_INLINE TSSEHashBucket2(const TSSEHashBucket2& other) {
        copy_AssumeEmpty(other);
    }
    FORCE_INLINE TSSEHashBucket2& operator =(const TSSEHashBucket2& other) {
        clear_LeaveDirty();
        copy_AssumeEmpty(other);
        return (*this);
    }

    FORCE_INLINE TSSEHashBucket2(TSSEHashBucket2&& rvalue) {
        move_AssumeEmpty(std::move(rvalue));
    }
    FORCE_INLINE TSSEHashBucket2& operator =(TSSEHashBucket2&& rvalue) {
        clear_LeaveDirty();
        move_AssumeEmpty(std::move(rvalue));
        return (*this);
    }

#if USE_PPE_DEBUG
    bool is_sentinel() const {
        m128i_t st = m128i_epi8_load_aligned(States);
        const word_t bm = m128i_epi8_findeq(st, m128i_epi8_set_zero());
        return (0xFFFFu == bm);
    }
#endif

    FORCE_INLINE T* at(index_t i) NOEXCEPT {
        Assert(i < Capacity);
        return (reinterpret_cast<T*>(&Items[i]));
    }
    FORCE_INLINE const T* at(index_t i) const NOEXCEPT {
        Assert(i < Capacity);
        return (reinterpret_cast<const T*>(&Items[i]));
    }

    FORCE_INLINE void clear_LeaveDirty() NOEXCEPT {
        IF_CONSTEXPR(not Meta::has_trivial_destructor<T>::value) {
            const m128i_t st = m128i_epi8_load_aligned(States);

            word_t e;
            for (mask_t bm{ m128i_epi8_findge(st, m128i_epi8_set_zero()) };
                FPlatformMaths::bsf(&e, bm); bm &= ~(mask_t(1) << e)) {
                Meta::Destroy(&at(e));
            }
        }
    }

    FORCE_INLINE void clear() NOEXCEPT {
        clear_LeaveDirty();
        m128i_epi8_store_aligned(&States, m128i_epi8_set_true());
    }

    FORCE_INLINE void copy_AssumeEmpty(const TSSEHashBucket2& other) {
        const m128i_t st = m128i_epi8_load_aligned(other.States);

        word_t e;
        for (mask_t bm{ m128i_epi8_findge(st, m128i_epi8_set_zero()) };
            FPlatformMaths::bsf(&e, bm); bm &= ~(mask_t(1) << e)) {
            Meta::Construct(&at(e), other.at(e));
        }

        m128i_epi8_store_aligned(&States, st);
    }

    FORCE_INLINE void move_AssumeEmpty(TSSEHashBucket2&& rvalue) NOEXCEPT {
        const m128i_t st = m128i_epi8_load_aligned(rvalue.States);

        word_t e;
        for (mask_t bm{ m128i_epi8_findge(st, m128i_epi8_set_zero()) };
            FPlatformMaths::bsf(&e, bm); bm &= ~(mask_t(1) << e)) {
            Meta::Construct(&at(e), std::move(rvalue.at(e)));
            Meta::Destroy(&rvalue.at(e));
        }

        m128i_epi8_store_aligned(&States, st);
        m128i_epi8_store_aligned(&rvalue.States, m128i_epi8_set_true());
    }

    FORCE_INLINE static TSSEHashBucket2* Sentinel() NOEXCEPT {
        // we guarantee we won't access Items, so use a generic sentinel (avoid bloating rdata)
        return (TSSEHashBucket2*)(&GSimdBucketSentinel2);
    }
};
//----------------------------------------------------------------------------
template <typename T, bool _Const>
struct TSSEHashIterator2 : Meta::TIterator<Meta::TConditional<_Const, Meta::TAddConst<T>, T>> {
    using bucket_t = TSSEHashBucket2<T>;
    using index_t = typename bucket_t::index_t;
    using mask_t = typename bucket_t::mask_t;
    using word_t = typename bucket_t::word_t;
    using state_t = typename bucket_t::state_t;
    using parent_t = Meta::TIterator<Meta::TConditional<_Const, Meta::TAddConst<T>, T>>;

    using typename parent_t::iterator_category;
    using typename parent_t::value_type;
    using typename parent_t::difference_type;
    using typename parent_t::pointer;
    using typename parent_t::reference;

    state_t* State;

    CONSTEXPR TSSEHashIterator2(Meta::FNoInit) NOEXCEPT {}
    CONSTEXPR explicit TSSEHashIterator2(state_t* state) NOEXCEPT : State(state) {}

    template <bool _Other>
    CONSTEXPR TSSEHashIterator2(const TSSEHashIterator2<T, _Other>& other) NOEXCEPT : State(other.State) {}
    template <bool _Other>
    CONSTEXPR TSSEHashIterator2& operator =(const TSSEHashIterator2<T, _Other>& other) NOEXCEPT {
        State = other.State;
        return (*this);
    }

    bucket_t* Bucket() const NOEXCEPT { return (reinterpret_cast<bucket_t*>(uintptr_t(State) & ~15)); }
    index_t Slot() const NOEXCEPT { return checked_cast<index_t>(State - (state_t*)Bucket()); }

    pointer get() const NOEXCEPT {
        Assert_NoAssume(bucket_t::kDeleted != *State);
        return Bucket()->at(Slot());
    }

    TSSEHashIterator2& operator++() NOEXCEPT { Advance(); return (*this); }
    TSSEHashIterator2& operator++(int) NOEXCEPT { TSSEHashIterator2 tmp(*this); Advance(); return tmp; }

    reference operator *() const NOEXCEPT { return *get(); }
    pointer operator ->() const NOEXCEPT { return get(); }

    CONSTEXPR inline friend bool operator ==(const TSSEHashIterator2& lhs, const TSSEHashIterator2& rhs) NOEXCEPT {
        return (lhs.State == rhs.State);
    }
    CONSTEXPR inline friend bool operator !=(const TSSEHashIterator2& lhs, const TSSEHashIterator2& rhs) NOEXCEPT {
        return (not operator ==(lhs, rhs));
    }

    void FirstSet(long slot) NOEXCEPT {
        bucket_t* __restrict pbucket = Bucket();
        for (Assert(pbucket);; ++pbucket, slot = -1) {
            const m128i_t st = m128i_epi8_load_aligned(pbucket->States);
            const mask_t bm{ m128i_epi8_findge(st, m128i_epi8_set_zero()) & (mask_t(0xFFFF) << mask_t(slot + 1)) };

            index_t e;
            if (Likely(FPlatformMaths::bsf(&e, bm))) {
                State = (pbucket->States + e);
                return;
            }
        }
    }

    FORCE_INLINE void Advance() NOEXCEPT {
        FirstSet(checked_cast<long>(Slot()));
    }

};
//----------------------------------------------------------------------------
template <
    typename _Key
,   typename _Hash = Meta::THash<_Key>
,   typename _EqualTo = Meta::TEqualTo<_Key>
,   typename _Allocator = ALLOCATOR(Container)
>   class TSSEHashSet2 : _Allocator {
public:
    typedef _Key value_type;
    typedef _Hash hasher;
    typedef _EqualTo key_equal;

    using allocator_type = _Allocator;
    using allocator_traits = TAllocatorTraits<allocator_type>;

    using bucket_t = TSSEHashBucket2<value_type>;
    using state_t = typename bucket_t::state_t;

    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef Meta::TAddPointer<value_type> pointer;
    typedef Meta::TAddPointer<const value_type> const_pointer;

    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

    using iterator = TSSEHashIterator2<value_type, false>;
    using const_iterator = TSSEHashIterator2<value_type, true>;

    CONSTEXPR TSSEHashSet2() NOEXCEPT
    :   _size(0)
    ,   _bucketMask(0)
    ,   _buckets(bucket_t::Sentinel())
    {}

    FORCE_INLINE ~TSSEHashSet2() {
        if (has_content_())
            releaseMemory_ForDtor_();
    }

    FORCE_INLINE TSSEHashSet2(const TSSEHashSet2& other) : TSSEHashSet2() {
        assign(other);
    }
    FORCE_INLINE TSSEHashSet2& operator =(const TSSEHashSet2& other) {
        assign(other);
        return (*this);
    }

    CONSTEXPR TSSEHashSet2(TSSEHashSet2&& rvalue) NOEXCEPT
    :   TSSEHashSet2() {
        std::swap(_size, rvalue._size);
        std::swap(_bucketMask, rvalue._bucketMask);
        std::swap(_buckets, rvalue._buckets);
    }
    FORCE_INLINE TSSEHashSet2& operator =(TSSEHashSet2&& rvalue) {
        assign(std::move(rvalue));
        return (*this);
    }

    void assign(const TSSEHashSet2& other) {
        Assert(&other != this);

        if (not other._size) {
            clear();
            return;
        }

        Assert_NoAssume(other.num_buckets_());
        Assert_NoAssume(bucket_t::Sentinel() != other._buckets);

        const word_t numBuckets = other.num_buckets_();
        const word_t allocSize = checked_cast<word_t>(allocation_size_(numBuckets));

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
            m128i_epi8_store_aligned(&_buckets[numBuckets].States, m128i_epi8_set_zero());
        }

        Assert_NoAssume(_buckets[numBuckets].is_sentinel());
    }

    void assign(TSSEHashSet2&& rvalue) {
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

    PPE_SSEHASHSET2_MICROPROFILING TPair<iterator, bool> insert(_Key&& __restrict rkey) {
        //INTEL_IACA_START();

        const size_t h0 = H0(rkey);
        m128i_t h1 = H1(h0);

        for (;;) {

            bucket_t* const __restrict pbucket = bucket_from_H0_(h0);
            const m128i_t st = m128i_epi8_load_aligned(pbucket->States);

            const m128i_t ce{ m128i_epi8_cmpeq(st, h1) };
            const m128i_t cf{ m128i_epi8_cmplt(st, m128i_epi8_set_zero()) };

            u32 me{ m128i_epi8_movemask(ce) };
            const u32 mf{ m128i_epi8_movemask(cf) };

            const u32 slot = FPlatformMaths::tzcnt(mf);

            for (; me;) {
                const u32 fnd = FPlatformMaths::tzcnt(me);
                if (Likely(key_equal()(*(pointer)(pbucket->Items + fnd), rkey)))
                    return MakePair(iterator{ pbucket->States + fnd }, false);
                else
                    me &= ~(u32(1) << fnd);
            }

            if (Likely(mf)) {
                Assert_NoAssume(has_content_());
                Assert_NoAssume(_size < capacity());
                Assert_NoAssume(not pbucket->is_sentinel());

                ++_size;
                Meta::Construct((pointer)(pbucket->Items + slot), std::move(rkey));
                pbucket->States[slot] = (state_t)((i8)h0 & (i8)0x7f);

                //INTEL_IACA_END();
                return MakePair(iterator{ pbucket->States + slot }, true);
            }
            else {
                rehash_ForCollision_();
            }
        }
    }
    FORCE_INLINE TPair<iterator, bool> insert(const _Key& key) {
        return insert(_Key(key));
    }

    iterator insert_AssertUnique(_Key&& rkey) {
        const size_t h0 = H0(rkey);
        const m128i_t h1 = H1(h0);

        for (;;) {
            bucket_t* __restrict pbucket = bucket_from_H0_(h0);
            const m128i_t st = m128i_epi8_load_aligned(pbucket->States);
            const m128i_t cf = m128i_epi8_cmplt(st, m128i_epi8_set_zero());
            const mask_t mf{ m128i_epi8_movemask(cf) };

            index_t slot;
            if (Likely(FPlatformMaths::bsf(&slot, mf))) {
                Assert_NoAssume(has_content_());
                Assert_NoAssume(_size < capacity());
                Assert_NoAssume(not pbucket->is_sentinel());

                pbucket->States[slot] = state_t{ H1(h0, slot) };
                Meta::Construct(pbucket->at(slot), std::move(rkey));

                ++_size;

                return iterator{ pbucket->States + slot };
            }
            else {
                rehash_ForCollision_();
            }
        }
    }
    iterator insert_AssertUnique(const _Key& key) {
        return insert_AssertUnique(_Key(key));
    }

    PPE_SSEHASHSET2_MICROPROFILING iterator find(const _Key& key) NOEXCEPT {
        return find_const_(key);
    }
    PPE_SSEHASHSET2_MICROPROFILING const_iterator find(const _Key& key) const NOEXCEPT {
        return find_const_(key);
    }

    PPE_SSEHASHSET2_MICROPROFILING bool erase(const _Key& key) NOEXCEPT {
        const size_t h0 = H0(key);

        bucket_t* const __restrict pbucket = bucket_from_H0_(h0);

        const m128i_t st = m128i_epi8_load_aligned(pbucket->States);
        const m128i_t h1 = H1(h0);
        const m128i_t ce = m128i_epi8_cmpeq(st, h1);

        index_t fnd;
        for (mask_t me{ m128i_epi8_movemask(ce) };
            FPlatformMaths::bsf(&fnd, me); me &= ~(mask_t(1) << fnd)) {
            if (Likely(key_equal()(*pbucket->at(fnd), key))) {
                Assert(_size);
                Assert_NoAssume(has_content_());

                pbucket->States[fnd] = bucket_t::kDeleted; // mark as deleted
                Meta::Destroy(pbucket->at(fnd));

                --_size;

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
        const word_t e = it.Slot();

        Assert_NoAssume(e < bucket_t::Capacity);
        Assert_NoAssume(pbucket->States[e] >= 0);

        pbucket->States[e] = bucket_t::kDeleted; // mark as deleted
        Meta::Destroy(&pbucket->at(e));

        --_size;
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
    bool AliasesToContainer(const TSSEHashIterator2<value_type, _Const>& it) const {
        return (FPlatformMemory::Memoverlap(
            it.Bucket(), sizeof(bucket_t),
            _buckets, num_buckets_() * sizeof(bucket_t)));
    }
#endif

private:
    using index_t = typename bucket_t::index_t;
    using mask_t = typename bucket_t::mask_t;
    using word_t = typename bucket_t::word_t;

    STATIC_ASSERT(Meta::has_trivial_destructor<hasher>::value);
    STATIC_CONST_INTEGRAL(word_t, FillRatio, 70); // 70% filled <=> 30% slack
    STATIC_CONST_INTEGRAL(word_t, SlackFactor, ((100 + (100 - FillRatio)) * 128) / 100);

    u32 _size;
    u32 _bucketMask;
    bucket_t* _buckets;

    CONSTEXPR static size_t allocation_size_(size_t numBuckets) NOEXCEPT {
        return (numBuckets * sizeof(bucket_t) + sizeof(FSSEHashStates2)/* sentinel */);
    }
    CONSTEXPR static size_t size_with_slack_(size_t n) NOEXCEPT {
        return ((n * SlackFactor) / 128);
    }

    CONSTEXPR bool has_content_() const NOEXCEPT {
        return (bucket_t::Sentinel() != _buckets);
    }
    CONSTEXPR u32 num_buckets_() const NOEXCEPT {
        return (has_content_() ? _bucketMask + 1 : 0);
    }

#if 0
    FORCE_INLINE static size_t H0(const _Key& value) NOEXCEPT {
        return hasher()(value);
    }
    FORCE_INLINE static m128i_t VECTORCALL H1(size_t h0) NOEXCEPT {
#ifdef ARCH_X64
        return ::_mm_set1_epi64x(h0 & 0x7f7f7f7f7f7f7f7fULL);
#else
        return ::_mm_set1_epi32(h0 & 0x7f7f7f7fULL);
#endif
    }
    CONSTEXPR static state_t H1(size_t h0, size_t slot) NOEXCEPT {
        return state_t((h0 & CODE3264(0x7f7f7f7fULL, 0x7f7f7f7f7f7f7f7fULL)) >> ((slot * 8) & bucket_t::HashMask));
    }
    CONSTEXPR bucket_t* bucket_from_H0_(size_t h0) const NOEXCEPT {
        return (_buckets + ((h0 * GOLDEN_RATIO_PRIME) & _bucketMask));
    }
#else
    FORCE_INLINE static size_t H0(const _Key& value) NOEXCEPT {
        return hasher()(value);
    }
    FORCE_INLINE static m128i_t VECTORCALL H1(size_t h0) NOEXCEPT {
        return ::_mm_set1_epi8((i8)(h0 & 0x7f));
    }
    CONSTEXPR static state_t H1(size_t h0, size_t ) NOEXCEPT {
        return (state_t)(h0 & 0x7f);
    }
    CONSTEXPR bucket_t* bucket_from_H0_(size_t h0) const NOEXCEPT {
        return (_buckets + ((h0 >> 7) & _bucketMask));
    }
#endif

    FORCE_INLINE iterator find_const_(const _Key& key) const NOEXCEPT {
        const size_t h0 = H0(key);

        bucket_t* const __restrict pbucket = bucket_from_H0_(h0);

        index_t fnd;
        for (mask_t me{ m128i_epi8_findeq(
                m128i_epi8_load_aligned(pbucket->States), H1(h0) ) };
            FPlatformMaths::bsf(&fnd, me); me &= ~(mask_t(1) << fnd)) {
            if (Likely(key_equal()(*pbucket->at(fnd), key)))
                return iterator{ pbucket->States + fnd };
        }

        return end();
    }

#if USE_PPE_ASSERT
    static size_t VECTORCALL find_ForAssert(const bucket_t& bucket, m128i_t st, m128i_t h16, const value_type& x) NOEXCEPT {
        index_t fnd;
        for (mask_t me{ m128i_epi8_findeq(st, h16) };
            FPlatformMaths::bsf(&fnd, me); me &= ~(mask_t(1) << fnd)) {
            if (key_equal()(*bucket.at(fnd), x))
                return fnd;
        }
        return INDEX_NONE;
    }
#endif

    NO_INLINE void rehash_ForCollision_() {
        rehash_(Max(1_size_t, num_buckets_() * 2_size_t) * bucket_t::Capacity);
    }
    NO_INLINE void rehash_ForGrowth_(size_t n) {
        rehash_(n);
    }

    FORCE_INLINE void rehash_(size_t n) {
        Assert_NoAssume(_size <= n);

        u32 const oldBucketMask = _bucketMask;
        bucket_t* const __restrict oldBuckets = _buckets;

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

        ONLY_IF_ASSERT(const size_t oldSize = _size);

        if (_size) {
            Assert(oldBuckets);
            Assert_NoAssume(bucket_t::Sentinel() != oldBuckets);

            ONLY_IF_ASSERT(_size = 0);

            forrange(pbucket, oldBuckets, oldBuckets + (oldBucketMask + 1)) {
                const m128i_t st = m128i_epi8_load_aligned(pbucket->States);

                index_t it;
                for (mask_t bm{ m128i_epi8_findge(st, m128i_epi8_set_zero()) };
                    FPlatformMaths::bsf(&it, bm); bm &= ~(mask_t(1) << it)) {

                    value_type& rkey = *(pointer)(pbucket->Items + it);
                    const size_t h0 = H0(rkey);

                    bucket_t* __restrict qbucket = bucket_from_H0_(h0);

                    const m128i_t qt = m128i_epi8_load_aligned(qbucket->States);
                    const mask_t free{ m128i_epi8_findlt(qt, m128i_epi8_set_zero()) };
                    AssertRelease(free);

                    const auto ins = FPlatformMaths::tzcnt(free);
                    Assert(ins < bucket_t::Capacity);
                    Assert_NoAssume(qbucket->States[ins] == bucket_t::kDeleted);

                    qbucket->States[ins] = state_t{ H1(h0, ins) };

                    Meta::Construct((pointer)(qbucket->Items + ins), std::move(rkey));
                    Meta::Destroy(&rkey);

                    ONLY_IF_ASSERT(++_size);
                }
            }
        }

        Assert_NoAssume(oldSize == _size);

        if (bucket_t::Sentinel() != oldBuckets) {
            Assert_NoAssume(not oldBuckets->is_sentinel());
            const word_t oldNumBuckets = (oldBucketMask + 1);
            allocator_traits::Deallocate(*this, FAllocatorBlock{ oldBuckets, allocation_size_(oldNumBuckets) });
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
    FORCE_INLINE TSSEHashIterator2<value_type, _Const> iterator_begin_() const NOEXCEPT {
        TSSEHashIterator2<value_type, _Const> it{ Meta::NoInit };
        if (_size) {
            it.State = _buckets->States;
            it.FirstSet(-1);
        }
        else {
            // avoids traversing empty containers with reserved memory
            it.State = _buckets[num_buckets_()].States;
        }
        return it;
    }

    template <bool _Const>
    FORCE_INLINE TSSEHashIterator2<value_type, _Const> iterator_end_() const NOEXCEPT {
        return TSSEHashIterator2<value_type, _Const>{ _buckets[num_buckets_()].States };
    }

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
