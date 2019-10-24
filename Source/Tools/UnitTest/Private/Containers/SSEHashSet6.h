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

#define USE_PPE_SSEHASHSET6_MICROPROFILING 0 //%_NOCOMMIT%
#if USE_PPE_SSEHASHSET6_MICROPROFILING
#   define PPE_SSEHASHSET6_MICROPROFILING NO_INLINE
#else
#   define PPE_SSEHASHSET6_MICROPROFILING
#endif

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum ESSEHashState6 : i8 {
    SSEHASHSET6_kDeleted = 0,
    SSEHASHSET6_kSentinel = -1,
};
//----------------------------------------------------------------------------
using FSSEHashStates6 = Meta::TArray<ESSEHashState6, 16>;
//----------------------------------------------------------------------------
ALIGN(16) CONSTEXPR const FSSEHashStates6 GSimdBucketSentinel6{
    SSEHASHSET6_kSentinel, SSEHASHSET6_kSentinel, SSEHASHSET6_kSentinel, SSEHASHSET6_kSentinel,
    SSEHASHSET6_kSentinel, SSEHASHSET6_kSentinel, SSEHASHSET6_kSentinel, SSEHASHSET6_kSentinel,
    SSEHASHSET6_kSentinel, SSEHASHSET6_kSentinel, SSEHASHSET6_kSentinel, SSEHASHSET6_kSentinel,
    SSEHASHSET6_kSentinel, SSEHASHSET6_kSentinel, SSEHASHSET6_kSentinel, SSEHASHSET6_kSentinel,
};
//----------------------------------------------------------------------------
template <typename T>
struct ALIGN(16) TSSEHashBucket6 {
    using index_t = unsigned long;
    using mask_t = size_t;
    using word_t = size_t;
    using state_t = ESSEHashState6;
    using states_t = FSSEHashStates6;
    using storage_t = POD_STORAGE(T);

    static CONSTEXPR u32 make_capacity() {
        switch (sizeof(T)) {
            // align on cache line size for small sizes
        case 4:
            return 12; // 1 cache line
        case 8:
            return 14; // 2 cache lines
        case 16:
            return 15; // 4 cache lines
        case 32:
            return 14; // aligned on 16, can't align on cache line (minimized offset though)
        // otherwise use default
        default:
            // first try to align on cache line size
            for (size_t c = 11; c <= 16; ++c) {
                const size_t s = sizeof(states_t) + sizeof(storage_t) * c;
                if (Meta::IsAligned(CACHELINE_SIZE, sizeof(states_t) + sizeof(storage_t) * c))
                    return c; // succeed, bail out
            }

            // then try to align on simd alignment requirement
            for (size_t c = 11; c <= 16; ++c)
                if (Meta::IsAligned(16, sizeof(states_t) + sizeof(storage_t) * c))
                    return c; // succeed, bail out

            return 14; // default choice
        }
    }

    STATIC_CONST_INTEGRAL(u32, Capacity, make_capacity());
    STATIC_CONST_INTEGRAL(u32, FullMask, (u32(1) << Capacity) - 1);

    STATIC_CONST_INTEGRAL(state_t, kDeleted, SSEHASHSET6_kDeleted);
    STATIC_CONST_INTEGRAL(state_t, kSentinel, SSEHASHSET6_kSentinel);

    states_t States;
    storage_t Items[Capacity];

    FORCE_INLINE TSSEHashBucket6() noexcept {
        STATIC_ASSERT(FullMask);
        STATIC_ASSERT(Capacity > 10);
        STATIC_ASSERT(Capacity <= 16);

        Assert(Meta::IsAligned(16, this));
        STATIC_ASSERT(sizeof(states_t) == sizeof(m128i_t));
        STATIC_ASSERT(sizeof(storage_t) == sizeof(T));
        m128i_epi8_store_aligned(States, m128i_epi8_set_zero());
    }

    FORCE_INLINE ~TSSEHashBucket6() {
        clear_LeaveDirty();
    }

    FORCE_INLINE TSSEHashBucket6(const TSSEHashBucket6& other) {
        copy_AssumeEmpty(other);
    }
    FORCE_INLINE TSSEHashBucket6& operator =(const TSSEHashBucket6& other) {
        clear_LeaveDirty();
        copy_AssumeEmpty(other);
        return (*this);
    }

    FORCE_INLINE TSSEHashBucket6(TSSEHashBucket6&& rvalue) {
        move_AssumeEmpty(std::move(rvalue));
    }
    FORCE_INLINE TSSEHashBucket6& operator =(TSSEHashBucket6&& rvalue) {
        clear_LeaveDirty();
        move_AssumeEmpty(std::move(rvalue));
        return (*this);
    }

#if USE_PPE_DEBUG
    bool is_valid() const {
        const m128i_t st = m128i_epi8_load_aligned(States);
        const auto occupied = m128i_epi8_movemask(st);
        return (not (occupied & ~FullMask));
    }
    bool is_sentinel() const {
        const m128i_t st = m128i_epi8_load_aligned(States);
        const auto bm = m128i_epi8_findeq(st, m128i_epi8_broadcast(kSentinel));
        return (0xFFFF == bm);
    }
#endif

    void set_sentinel() {
        m128i_epi8_store_aligned(States, m128i_epi8_broadcast(kSentinel));
    }

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
            for (mask_t bm{ m128i_epi8_movemask(st) };
                FPlatformMaths::bsf(&e, bm); bm &= (bm - 1)) {
                Meta::Destroy(&at(e));
            }
        }
    }

    FORCE_INLINE void clear() NOEXCEPT {
        clear_LeaveDirty();
        m128i_epi8_store_aligned(States, m128i_epi8_set_zero());
    }

    FORCE_INLINE void copy_AssumeEmpty(const TSSEHashBucket6& other) {
        const m128i_t st = m128i_epi8_load_aligned(other.States);

        m128i_epi8_store_aligned(States, st);

        word_t e;
        for (mask_t bm{ m128i_epi8_movemask(st) };
            FPlatformMaths::bsf(&e, bm); bm &= (bm - 1)) {
            Meta::Construct(&at(e), other.at(e));
        }
    }

    FORCE_INLINE void move_AssumeEmpty(TSSEHashBucket6&& rvalue) NOEXCEPT {
        const m128i_t st = m128i_epi8_load_aligned(rvalue.States);

        m128i_epi8_store_aligned(States, st);
        m128i_epi8_store_aligned(rvalue.States, m128i_epi8_set_zero());

        word_t e;
        for (mask_t bm{ m128i_epi8_movemask(st) };
            FPlatformMaths::bsf(&e, bm); bm &= (bm - 1)) {
            Meta::Construct(&at(e), std::move(rvalue.at(e)));
            Meta::Destroy(&rvalue.at(e));
        }
    }

    FORCE_INLINE static TSSEHashBucket6* Sentinel() NOEXCEPT {
        // we guarantee we won't access Items, so use a generic sentinel (avoid bloating rdata)
        return (TSSEHashBucket6*)(&GSimdBucketSentinel6);
    }
};
//----------------------------------------------------------------------------
template <typename T, bool _Const>
struct TSSEHashIterator6 : Meta::TIterator<Meta::TConditional<_Const, Meta::TAddConst<T>, T>> {
    using bucket_t = TSSEHashBucket6<T>;
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

    CONSTEXPR explicit TSSEHashIterator6(Meta::FNoInit) NOEXCEPT {}

    CONSTEXPR explicit TSSEHashIterator6(state_t* state) NOEXCEPT
    :   State(state) {
        Assert(state);
        Assert_NoAssume(*state != bucket_t::kDeleted);
    }

    template <bool _Other>
    CONSTEXPR TSSEHashIterator6(const TSSEHashIterator6<T, _Other>& other) NOEXCEPT : State(other.State) {}
    template <bool _Other>
    CONSTEXPR TSSEHashIterator6& operator =(const TSSEHashIterator6<T, _Other>& other) NOEXCEPT {
        State = other.State;
        return (*this);
    }

    bucket_t* Bucket() const NOEXCEPT { return (reinterpret_cast<bucket_t*>(uintptr_t(State) & ~15)); }
    index_t Slot() const NOEXCEPT { return checked_cast<index_t>(State - (state_t*)Bucket()); }

    pointer get() const NOEXCEPT {
        Assert_NoAssume(bucket_t::kDeleted != *State);
        return Bucket()->at(Slot());
    }

    TSSEHashIterator6& operator++() NOEXCEPT { Advance(); return (*this); }
    TSSEHashIterator6& operator++(int) NOEXCEPT { TSSEHashIterator6 tmp(*this); Advance(); return tmp; }

    reference operator *() const NOEXCEPT { return *get(); }
    pointer operator ->() const NOEXCEPT { return get(); }

    CONSTEXPR inline friend bool operator ==(const TSSEHashIterator6& lhs, const TSSEHashIterator6& rhs) NOEXCEPT {
        return (lhs.State == rhs.State);
    }
    CONSTEXPR inline friend bool operator !=(const TSSEHashIterator6& lhs, const TSSEHashIterator6& rhs) NOEXCEPT {
        return (not operator ==(lhs, rhs));
    }

    void FirstSet(long slot) NOEXCEPT {
        bucket_t* __restrict pbucket = Bucket();
        auto off{ slot + 1 };
        for (Assert(pbucket);; ++pbucket, off = 0) {
            const m128i_t st = m128i_epi8_load_aligned(pbucket->States);
            const auto bm{ m128i_epi8_movemask(st) & (bucket_t::FullMask << off) };

            index_t e;
            if (Likely(FPlatformMaths::bsf(&e, bm))) {
                Assert_NoAssume(bucket_t::Capacity > e);
                Assert_NoAssume(bucket_t::kDeleted != pbucket->States[e]);

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
>   class TSSEHashSet6 : _Allocator {
public:
    typedef _Key value_type;
    typedef _Hash hasher;
    typedef _EqualTo key_equal;

    using allocator_type = _Allocator;
    using allocator_traits = TAllocatorTraits<allocator_type>;

    using bucket_t = TSSEHashBucket6<value_type>;
    using state_t = typename bucket_t::state_t;

    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef Meta::TAddPointer<value_type> pointer;
    typedef Meta::TAddPointer<const value_type> const_pointer;

    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

    using iterator = TSSEHashIterator6<value_type, false>;
    using const_iterator = TSSEHashIterator6<value_type, true>;

    CONSTEXPR TSSEHashSet6() NOEXCEPT
    :   _size(0)
    ,   _bucketMask(0)
    ,   _buckets(bucket_t::Sentinel())
    {}

    FORCE_INLINE ~TSSEHashSet6() {
        if (has_content_())
            releaseMemory_ForDtor_();
    }

    FORCE_INLINE TSSEHashSet6(const TSSEHashSet6& other) : TSSEHashSet6() {
        assign(other);
    }
    FORCE_INLINE TSSEHashSet6& operator =(const TSSEHashSet6& other) {
        assign(other);
        return (*this);
    }

    CONSTEXPR TSSEHashSet6(TSSEHashSet6&& rvalue) NOEXCEPT
    :   TSSEHashSet6() {
        std::swap(_size, rvalue._size);
        std::swap(_bucketMask, rvalue._bucketMask);
        std::swap(_buckets, rvalue._buckets);
    }
    FORCE_INLINE TSSEHashSet6& operator =(TSSEHashSet6&& rvalue) {
        assign(std::move(rvalue));
        return (*this);
    }

    void assign(const TSSEHashSet6& other) {
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
            _buckets[numBuckets].set_sentinel();
        }

        Assert_NoAssume(_buckets[numBuckets].is_sentinel());
    }

    void assign(TSSEHashSet6&& rvalue) {
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

    PPE_SSEHASHSET6_MICROPROFILING TPair<iterator, bool> insert(_Key&& __restrict rkey) {
        INTEL_IACA_START();

        const keyhash_t kh = keyhash_(rkey);
        const m128i_t h1 = kh.broadcast_tag();

        for (;;) {
            bucket_t* const __restrict pbucket = bucket_(kh.Hash);

            const m128i_t st = m128i_epi8_load_aligned(pbucket->States);
            const m128i_t ce = m128i_epi8_cmpeq(st, h1);

            auto mf{ m128i_epi8_movemask(st) };
            auto me{ m128i_epi8_movemask(ce) };

            mf = (~mf & bucket_t::FullMask);
            const auto ins = FPlatformMaths::tzcnt(mf);

        RETRY:
            const auto fnd = FPlatformMaths::tzcnt(me);
            if (Unlikely(me)) {
                if (Likely(key_equal()(*pbucket->at(fnd), rkey))) {
                    return std::make_pair(make_iterator_(pbucket, fnd), false);
                }
                else {
                    me &= (me - 1);
                    goto RETRY;
                }
            }

            if (Likely(mf)) {
                Assert_NoAssume(has_content_());
                Assert_NoAssume(_size < capacity());
                Assert_NoAssume(pbucket->is_valid());
                Assert_NoAssume(not pbucket->is_sentinel());

                ++_size;
                Meta::Construct(pbucket->at(ins), std::move(rkey));
                pbucket->States[ins] = kh.state_tag(ins);

                Assert_NoAssume(pbucket->is_valid());

                INTEL_IACA_END();
                return std::make_pair(make_iterator_(pbucket, ins), true);
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
        const keyhash_t kh = keyhash_(rkey);

        for (;;) {
            bucket_t* __restrict pbucket = bucket_(kh.Hash);
            const m128i_t st = m128i_epi8_load_aligned(pbucket->States);
            const auto mf{ (~m128i_epi8_movemask(st) & bucket_t::FullMask) };

            index_t ins;
            if (Likely(FPlatformMaths::bsf(&ins, mf))) {
                Assert_NoAssume(has_content_());
                Assert_NoAssume(_size < capacity());
                Assert_NoAssume(not pbucket->is_sentinel());
                Assert_NoAssume(pbucket->is_valid());

                ++_size;
                Meta::Construct(pbucket->at(ins), std::move(rkey));
                pbucket->States[ins] = kh.state_tag(ins);

                Assert_NoAssume(pbucket->is_valid());

                return make_iterator_(pbucket, ins);
            }
            else {
                rehash_ForCollision_();
            }
        }
    }
    iterator insert_AssertUnique(const _Key& key) {
        return insert_AssertUnique(_Key(key));
    }

    PPE_SSEHASHSET6_MICROPROFILING iterator find(const _Key& key) NOEXCEPT {
        return find_const_(key);
    }
    PPE_SSEHASHSET6_MICROPROFILING const_iterator find(const _Key& key) const NOEXCEPT {
        return find_const_(key);
    }

    PPE_SSEHASHSET6_MICROPROFILING bool erase(const _Key& key) NOEXCEPT {
        const keyhash_t kh = keyhash_(key);

        bucket_t* __restrict pbucket = bucket_(kh.Hash);

        const m128i_t st = m128i_epi8_load_aligned(pbucket->States);
        const m128i_t h1 = kh.broadcast_tag();
        const m128i_t ce = m128i_epi8_cmpeq(st, h1);

        index_t fnd;
        for (auto me{ m128i_epi8_movemask(ce) };
            FPlatformMaths::bsf(&fnd, me); me &= (me - 1)) {
            if (Likely(key_equal()(*pbucket->at(fnd), key))) {
                Assert(_size);
                Assert_NoAssume(has_content_());

                pbucket->States[fnd] = bucket_t::kDeleted; // mark as deleted
                Meta::Destroy(pbucket->at(fnd));

                Assert_NoAssume(pbucket->is_valid());

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

        bucket_t* __restrict const pbucket = &it.Bucket();
        const auto e = it.Slot();

        Assert_NoAssume(e < bucket_t::Capacity);
        Assert_NoAssume(pbucket->States[e] < 0);

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
    bool AliasesToContainer(const TSSEHashIterator6<value_type, _Const>& it) const {
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

    struct keyhash_t {
        size_t Hash;
        size_t STag;

        m128i_t VECTORCALL broadcast_tag() const NOEXCEPT {
            return CODE3264(::_mm_set1_epi32, ::_mm_set1_epi64x)(STag);
        }
        state_t state_tag(size_t slot) const NOEXCEPT {
            return (state_t)(STag >> ((slot & CODE3264(3, 7)) << 3));
        }
    };

    CONSTEXPR bucket_t* __restrict bucket_(size_t hsh) const NOEXCEPT {
        bucket_t* const __restrict pbucket{ _buckets + (hsh & _bucketMask) };
        Assert_NoAssume(pbucket->is_sentinel() or pbucket->is_valid());
        return pbucket;
    }

    keyhash_t keyhash_(const _Key& value) const NOEXCEPT {
        const auto hsh = (hasher()(value));
        FPlatformMemory::ReadPrefetch(bucket_(hsh));
        const auto tag = (FPlatformHash::HashUInt(hsh) | CODE3264(0x80808080UL, 0x8080808080808080ULL));
        return keyhash_t{ hsh, tag };
    }

    CONSTEXPR static size_t allocation_size_(size_t numBuckets) NOEXCEPT {
        return (numBuckets * sizeof(bucket_t) + sizeof(FSSEHashStates6)/* sentinel */);
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

    FORCE_INLINE iterator find_const_(const _Key& key) const NOEXCEPT {
        const keyhash_t kh = keyhash_(key);

        bucket_t* const __restrict pbucket = bucket_(kh.Hash);

        const m128i_t st = m128i_epi8_load_aligned(pbucket->States);
        const m128i_t h1 = kh.broadcast_tag();
        const m128i_t ce = m128i_epi8_cmpeq(st, h1);

        index_t fnd;
        for (auto me{ m128i_epi8_movemask(ce) };
            FPlatformMaths::bsf(&fnd, me); me &= (me - 1)) {
            if (Likely(key_equal()(*pbucket->at(fnd), key)))
                return make_iterator_(pbucket, fnd);
        }

        return end();
    }

    NO_INLINE void rehash_ForCollision_() {
        rehash_(Max(mask_t(1), num_buckets_() * 2) * bucket_t::Capacity);
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
        _buckets[numBuckets].set_sentinel();
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
                for (mask_t bm{ m128i_epi8_movemask(st) };
                    FPlatformMaths::bsf(&it, bm); bm &= (bm - 1)) {

                    value_type& rkey = *pbucket->at(it);

                    const keyhash_t kh = keyhash_(rkey);

                    bucket_t* __restrict const qbucket = bucket_(kh.Hash);

                    const m128i_t qt = m128i_epi8_load_aligned(qbucket->States);
                    const auto free{ (~m128i_epi8_movemask(qt)) & bucket_t::FullMask };
                    AssertRelease(free);

                    const auto ins = FPlatformMaths::tzcnt(free);
                    Assert(ins < bucket_t::Capacity);
                    Assert_NoAssume(qbucket->States[ins] == bucket_t::kDeleted);

                    qbucket->States[ins] = kh.state_tag(ins);

                    Meta::Construct(qbucket->at(ins), std::move(rkey));
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

    FORCE_INLINE iterator make_iterator_(bucket_t* bucket, index_t slot) const NOEXCEPT {
        Assert(slot < bucket_t::Capacity);
        const iterator it{ bucket->States + slot };
        Assert_NoAssume(it.Bucket() == bucket);
        Assert_NoAssume(it.Slot() == slot);
        return it;
    }

    template <bool _Const>
    FORCE_INLINE TSSEHashIterator6<value_type, _Const> iterator_begin_() const NOEXCEPT {
        TSSEHashIterator6<value_type, _Const> it{ Meta::NoInit };
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
    FORCE_INLINE TSSEHashIterator6<value_type, _Const> iterator_end_() const NOEXCEPT {
        return TSSEHashIterator6<value_type, _Const>{ _buckets[num_buckets_()].States };
    }

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
