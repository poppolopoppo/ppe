#pragma once

#include "Allocator/Allocation.h"
#include "Container/BitMask.h"
#include "Container/ByteMask.h"
#include "HAL/PlatformMaths.h"
#include "HAL/PlatformMemory.h"
#include "Memory/MemoryView.h"
#include "Meta/AlignedStorage.h"
#include "Meta/PointerWFlags.h"
#include "Meta/Iterator.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
CONSTEXPR ALIGN(16) const FByteMask16::u8_16_t GSimdBucketSentinel{ 0 };
//----------------------------------------------------------------------------
template <typename T, typename _EqualTo>
struct ALIGN(16) TSimdBucket {
    STATIC_CONST_INTEGRAL(size_t, Capacity, 16);

    using bitmask_t = TBitMask<u32>;
    using bytemask_t = FByteMask16;
    using equal_t = _EqualTo;
    using states_t = typename FByteMask16::u8_16_t;
    using storage_t = POD_STORAGE(T);

    states_t States;
    storage_t Items[Capacity];

    TSimdBucket() noexcept {
        deleted_epi8().store(&States);
    }

    ~TSimdBucket() {
        clear_LeaveDirty();
    }

    TSimdBucket(const TSimdBucket& other) {
        copy_AssumeEmpty(other);
    }
    TSimdBucket& operator =(const TSimdBucket& other) {
        clear_LeaveDirty();
        copy_AssumeEmpty(other);
        return (*this);
    }

    TSimdBucket(TSimdBucket&& rvalue) {
        move_AssumeEmpty(std::move(rvalue));
    }
    TSimdBucket& operator =(TSimdBucket&& rvalue) {
        clear_LeaveDirty();
        move_AssumeEmpty(std::move(rvalue));
        return (*this);
    }

    bool is_sentinel() const {
        bytemask_t st;
        st.load(&States);
        const u16 msk = u16(::_mm_movemask_epi8(::_mm_cmpeq_epi32(st.xmm, ::_mm_setzero_si128())));
        Assert_NoAssume((0 == msk) || (0xFFFFu == msk));
        return (!!msk);
    }

    T& at(size_t i) NOEXCEPT {
        Assert(i < Capacity);
        return reinterpret_cast<T&>(Items[i]);
    }
    const T& at(size_t i) const NOEXCEPT {
        return const_cast<TSimdBucket*>(this)->at(i);
    }

    size_t VECTORCALL insert_assume_unique(bytemask_t h16, T&& x) {
        bytemask_t st;
        st.load(&States);

        const u32 e = st.replace_first_assume_unique(deleted_epi8(), h16);

        if (e < Capacity) {
            st.store(&States);
            Meta::Construct(&at(e), std::move(x));
            return e;
        }
        else {
            return INDEX_NONE;
        }
    }

    size_t VECTORCALL insert_unique(bytemask_t h16, T&& x) {
        u32 e = find(h16, x);
        if (e >= Capacity)
            e = insert_assume_unique(h16, x);
        return e;
    }

    size_t VECTORCALL find(bytemask_t h16, const T& x) const NOEXCEPT {
        bytemask_t st;
        st.load(&States);

        for (bitmask_t bm{ st.find(h16) }; bm.Data; ) {
            const u32 e = bm.PopFront_AssumeNotEmpty();
            if (equal_t()(at(e), x))
                return e;
        }

        return INDEX_NONE;
    }

    bool VECTORCALL erase(bytemask_t h16, const T& x) NOEXCEPT {
        bytemask_t st;
        bytemask_t visited;
        visited.setzero();

        u32 e;
        for (;;) {
            st.load(&States);
            e = st.replace_first_loop(visited, h16, deleted_epi8());
            if (e >= Capacity)
                return false;
            else if (equal_t()(at(e), x))
                break;
        }

        st.store(&States);
        Meta::Destroy(&at(e));
        return true;
    }

    void erase_at(u32 i) NOEXCEPT {
        Assert_NoAssume(i < Capacity);
        Assert_NoAssume(States[i] != 0xFF);

        bytemask_t st;
        st.load(&States);
        st.remove_at(i);
        st.store(&States);

        Meta::Destroy(&at(i));
    }

    void clear_LeaveDirty() NOEXCEPT {
        IF_CONSTEXPR(not Meta::has_trivial_destructor<T>::value) {
            bytemask_t st;
            st.load(&States);
            for (bitmask_t bm{ st.clear_move(deleted_epi8()) }; bm.Data; )
                Meta::Destroy(&at(bm.PopFront_AssumeNotEmpty()));
        }
    }

    void clear() NOEXCEPT {
        clear_LeaveDirty();
        deleted_epi8().store(&States);
    }

    void copy_AssumeEmpty(const TSimdBucket& other) {
        bytemask_t st;
        st.load(&other.States);

        for (bitmask_t bm{ st.find_not(deleted_epi8()) }; bm.Data; ) {
            const u32 e = bm.PopFront_AssumeNotEmpty();
            Meta::Construct(&at(e), other.at(e));
        }

        st.store(&States);
    }

    void move_AssumeEmpty(TSimdBucket&& rvalue) {
        bytemask_t st;
        st.load(&rvalue.States);

        for (bitmask_t bm = st.find_not(deleted_epi8()); bm.Data; ) {
            const u32 e = bm.PopFront_AssumeNotEmpty();
            Meta::Construct(&at(e), std::move(rvalue.at(e)));
            Meta::Destroy(&rvalue.at(e));
        }

        st.store(&rvalue);
        deleted_epi8().store(&rvalue.States);
    }

    bitmask_t each() const NOEXCEPT {
        bytemask_t st;
        st.load(&States);
        return bitmask_t{ st.find_not(deleted_epi8()) };
    }

    bitmask_t find_first_after(i8 i) const NOEXCEPT {
        bytemask_t st;
        st.load(&States);
        return bitmask_t{ st.find_not_after(deleted_epi8(), i) };
    }

    static bytemask_t VECTORCALL key_epi8(u8 h) NOEXCEPT {
        Assert_NoAssume((h ^ 0xFF/* deleted */) | (h ^ 0x0/* sentinel */));
        return { ::_mm_set1_epi8(h) };
    }
    static bytemask_t VECTORCALL deleted_epi8() NOEXCEPT {
        return { ::_mm_cmpeq_epi32(::_mm_setzero_si128(), ::_mm_setzero_si128()) };
    }
    static bytemask_t VECTORCALL sentinel_epi8() NOEXCEPT {
        return { ::_mm_setzero_si128() };
    }

    static TSimdBucket* Sentinel() NOEXCEPT {
        // we guarantee we won't access Items, so use a generic sentinel (avoid bloating rdata)
        return (TSimdBucket*)(&GSimdBucketSentinel);
    }
};
//----------------------------------------------------------------------------
template <
    typename _Key
,   typename _Hash = Meta::THash<_Key>
,   typename _EqualTo = Meta::TEqualTo<_Key>
,   typename _Allocator = ALLOCATOR(Container)
>   class TSimdHashSet : _Allocator {
public:
    typedef _Key value_type;
    typedef _Hash hasher;
    typedef _EqualTo key_equal;

    using allocator_type = _Allocator;
    using allocator_traits = TAllocatorTraits<allocator_type>;

    using bucket_t = TSimdBucket<value_type, key_equal>;

    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef Meta::TAddPointer<value_type> pointer;
    typedef Meta::TAddPointer<const value_type> const_pointer;

    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

    template <bool _Const>
    struct TIterator : Meta::TIterator<Meta::TConditional<_Const, Meta::TAddConst<value_type>, value_type>> {
        Meta::FHeapPtrWCounter BucketAndSlot;

        TIterator(bucket_t* bucket, u32 slot) NOEXCEPT {
            BucketAndSlot.Reset(bucket, checked_cast<u8>(slot));
        }

        template <bool _Other>
        TIterator(const TIterator<_Other>& other) NOEXCEPT
        :   BucketAndSlot(other.BucketAndSlot)
        {}
        template <bool _Other>
        TIterator& operator =(const TIterator<_Other>& other) NOEXCEPT {
            BucketAndSlot = other.BucketAndSlot;
            return (*this);
        }

        bucket_t& Bucket() const NOEXCEPT { return (*BucketAndSlot.Ptr<bucket_t>()); }
        u32 Slot() const NOEXCEPT { return BucketAndSlot.Counter(); }

        TIterator& operator++() { Advance(); return (*this); }
        TIterator& operator++(int) { TIterator tmp(*this); Advance(); return tmp; }

        reference operator *() const { return Bucket().at(BucketAndSlot.Counter()); }
        pointer operator ->() const { return (&operator *()); }

        inline friend bool operator ==(const TIterator& lhs, const TIterator& rhs) NOEXCEPT {
            return (lhs.BucketAndSlot.Data == rhs.BucketAndSlot.Data);
        }
        inline friend bool operator !=(const TIterator& lhs, const TIterator& rhs) NOEXCEPT {
            return (not operator ==(lhs, rhs));
        }

        void FirstSet(i8 slot) {
            bucket_t* pbucket = &Bucket();
            for (Assert(pbucket);; ++pbucket, slot = -1) {
                const bitmask_t bm{ pbucket->find_first_after(slot) };
                if (Likely(bm.Data)) {
                    slot = i8(bm.FirstBitSet_AssumeNotEmpty());
                    break;
                }
            }
            BucketAndSlot.Reset(pbucket, u8(slot));
        }

        void Advance() {
            FirstSet(checked_cast<i8>(BucketAndSlot.Counter()));
        }

        static TIterator Begin(const TSimdHashSet& owner) NOEXCEPT {
            TIterator it{ owner._buckets, 0 };
            it.FirstSet(i8(-1));
            return it;
        }

        static TIterator End(const TSimdHashSet& owner) NOEXCEPT {
            return TIterator{ owner._buckets + owner.num_buckets_(), 0 };
        }
    };

    using iterator = TIterator<false>;
    using const_iterator = TIterator<true>;

    CONSTEXPR TSimdHashSet() NOEXCEPT
    :   _size(0)
    ,   _bucketMask(0)
    ,   _buckets(bucket_t::Sentinel())
    {}

    ~TSimdHashSet() {
        if (_bucketMask)
            releaseMemory_NotEmpty_LeaveDirty_();
    }

    TSimdHashSet(const TSimdHashSet& other) : TSimdHashSet() {
        assign(other);
    }
    TSimdHashSet& operator =(const TSimdHashSet& other) {
        assign(other);
        return (*this);
    }

    CONSTEXPR TSimdHashSet(TSimdHashSet&& rvalue) NOEXCEPT
    :   TSimdHashSet() {
        std::swap(_size, rvalue._size);
        std::swap(_bucketMask, rvalue._bucketMask);
        std::swap(_buckets, rvalue._buckets);
    }
    TSimdHashSet& operator =(TSimdHashSet&& rvalue) {
        assign(std::move(rvalue));
        return (*this);
    }

    void assign(const TSimdHashSet& other) {
        clear_ReleaseMemory();

        if (not other._size)
            return;

        Assert(other._bucketMask);
        Assert_NoAssume(other._buckets != bucket_t::Sentinel());

        const u32 numBuckets = other.num_buckets_();

        _size = other._size;
        _bucketMask = other._bucketMask;
        _buckets = allocator_traits::template AllocateT<bucket_t>(*this, numBuckets + 1/* sentinel */).data();

        // trivial copy, don't try to rehash
        IF_CONSTEXPR(Meta::has_trivial_copy<value_type>::value) {
            FPlatformMemory::MemcpyLarge(
                _buckets,
                other._buckets,
                (numBuckets + 1/* copy sentinel */) * sizeof(bucket_t));
        }
        else {
            std::uninitialized_copy(
                other._buckets, other._buckets + numBuckets,
                MakeCheckedIterator(_buckets, numBuckets, 0) );

            bucket_t::sentinel_epi8().store(&_buckets[numBuckets].States); // init sentinel
        }

        Assert_NoAssume(_buckets[numBuckets].is_sentinel());
    }

    void assign(TSimdHashSet&& rvalue) {
        clear_ReleaseMemory();

        std::swap(_size, rvalue._size);
        std::swap(_bucketMask, rvalue._bucketMask);
        std::swap(_buckets, rvalue._buckets);
    }

    bool empty() const { return (0 == _size); }
    size_t size() const { return _size; }
    size_t capacity() const { return (num_buckets_() * bucket_t::Capacity); }

    iterator begin() { return iterator::Begin(*this); }
    iterator end() { return iterator::End(*this); }

    const_iterator begin() const { return const_iterator::Begin(*this); }
    const_iterator end() const { return const_iterator::End(*this); }

    TPair<iterator, bool> insert(const _Key& key) {
        return insert(_Key(key));
    }
    TPair<iterator, bool> insert(_Key&& key) {
        const hashed_key_t hash{ key };

        bytemask_t st;
        u32 bk, it;
        for (;;) {
            bk = (hash.H0 & _bucketMask);

            st.load(&_buckets[bk]);

            for (bitmask_t bm{ st.find(hash.H1) }; bm.Data;) {
                it = bm.PopFront_AssumeNotEmpty();
                if (Likely(key_equal()(_buckets[bk].at(it), key)))
                    return MakePair(iterator{ &_buckets[bk], it }, false);
            }

            it = st.replace_first_assume_unique(bucket_t::deleted_epi8(), hash.H1);
            if (Likely(it < bucket_t::Capacity))
                break;

            rehash_ForGrowth_();
        }

        ++_size;
        st.store(&_buckets[bk]);
        Meta::Construct(&_buckets[bk].at(it), std::move(key));

        return MakePair(iterator{ &_buckets[bk], it }, true);
    }

    iterator insert_AssertUnique(const _Key& key) {
        return insert_AssertUnique(_Key(key));
    }
    iterator insert_AssertUnique(_Key&& key) {
        const hashed_key_t hash{ key };

        bytemask_t st;
        u32 bk, it;
        for (;;) {
            bk = (hash.H0 & _bucketMask);
            Assert_NoAssume(_buckets[bk].find(hash.H1, key) == INDEX_NONE);

#if 0
            it = _buckets[b].insert_assume_unique(hash.H1, _Key(key));
            if (it != INDEX_NONE)
                break;

            rehash_ForGrowth_();
#else
            st.load(&_buckets[bk]);

            it = st.replace_first_assume_unique(bucket_t::deleted_epi8(), hash.H1);

            if (Likely(it < bucket_t::Capacity))
                break;

            rehash_ForGrowth_();
#endif
        }

        ++_size;
        st.store(&_buckets[bk]);
        Meta::Construct(&_buckets[bk].at(it), std::move(key));

        return iterator{ &_buckets[bk], it };
    }

    FORCE_INLINE iterator find(const _Key& key) NOEXCEPT {
        const hashed_key_t hash{ key };

        const u32 bk = (hash.H0 & _bucketMask);
#if 0
        const u32 it = _buckets[bk].find(hash.H1, key);

        return (it == INDEX_NONE ? end() : iterator{ &_buckets[bk], it });
#else
        bytemask_t st;
        st.load(&_buckets[bk].States);

        for (bitmask_t bm{ st.find(hash.H1) }; bm.Data;) {
            const u32 it = bm.PopFront_AssumeNotEmpty();
            if (Likely(key_equal()(_buckets[bk].at(it), key)))
                return iterator{ &_buckets[bk], it };
        }

        return end();
#endif
    }

    FORCE_INLINE const_iterator find(const _Key& key) const NOEXCEPT {
        return const_cast<TSimdHashSet*>(this)->find(key);
    }

    bool erase(const _Key& key) NOEXCEPT {
        const hashed_key_t hash{ key };

        const u32 bk = (hash.H0 & _bucketMask);
        if (_buckets[bk].erase(hash.H1, key)) {
            --_size;
            return true;
        }
        else {
            return false;
        }
    }
    void erase(iterator it) NOEXCEPT {
        erase(const_iterator{ it });
    }
    void erase(const_iterator it) NOEXCEPT {
        Assert_NoAssume(_size);
        Assert_NoAssume(AliasesToContainer(it));

        --_size;
        it.Bucket()->erase(it.Slot());
    }

    void clear() {
        if (_size)
            clear_NotEmpty_();
    }

    void clear_ReleaseMemory() {
        if (_size | _bucketMask) {
            releaseMemory_NotEmpty_LeaveDirty_();

            _size = 0;
            _bucketMask = 0;
            _buckets = bucket_t::Sentinel();
        }
    }

    FORCE_INLINE void reserve_Additional(size_t num) { reserve(_size + num); }
    void reserve(size_t n) {
        Assert(n);

        n = size_with_slack_(n);
        if (n > capacity())
            rehash(n);
    }

    NO_INLINE void rehash(size_t n) {
        Assert_NoAssume(_size <= n);

        u32 const oldBucketMask = _bucketMask;
        bucket_t* const oldBuckets = _buckets;

        const u32 numBuckets = Max(MinNumBuckets, FPlatformMaths::NextPow2(
            checked_cast<u32>((n + bucket_t::Capacity - 1) / bucket_t::Capacity)));
        Assert(numBuckets);
        Assert_NoAssume(Meta::IsPow2(numBuckets));

        _bucketMask = (numBuckets - 1);
        _buckets = allocator_traits::template AllocateT<bucket_t>(*this, numBuckets + 1/* sentinel */).data();

        forrange(pbucket, _buckets, _buckets + numBuckets)
            INPLACE_NEW(pbucket, bucket_t);

        bucket_t::sentinel_epi8().store(&_buckets[numBuckets].States); // init sentinel

        Assert_NoAssume(n <= capacity());

        if (_size) {
            Assert(oldBucketMask);
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

        if (oldBucketMask) {
            const u32 oldNumBuckets = (oldBucketMask + 1);
            allocator_traits::DeallocateT(*this, oldBuckets, oldNumBuckets + 1/* sentinel */);
        }
    }

    template <bool _Const>
    bool AliasesToContainer(const TIterator<_Const>& it) const {
        return (FPlatformMemory::Memoverlap(
            it.Bucket(), sizeof(bucket_t),
            _buckets, num_buckets_() * sizeof(bucket_t)));
    }

private:
    using bitmask_t = typename bucket_t::bitmask_t;
    using bytemask_t = typename bucket_t::bytemask_t;

    STATIC_CONST_INTEGRAL(u32, FillRatio, 70); // 70% filled
    STATIC_CONST_INTEGRAL(u32, SlackFactor, ((100 + (100 - FillRatio)) * 128) / 100); // 30% slack
    STATIC_CONST_INTEGRAL(u32, MinNumBuckets, 2);

    u32 _size;
    u32 _bucketMask;
    bucket_t* _buckets;

    struct hashed_key_t {
        bytemask_t H1;
        u32 H0;
        explicit hashed_key_t(const value_type& key) NOEXCEPT
        :   H0(u32(hasher()(key))) {
            H1 = bucket_t::key_epi8(u8(H0 & 0x7f));
            H0 = H0 >> 7;
        }
    };

    STATIC_ASSERT(Meta::has_trivial_destructor<bytemask_t>::value);

    static CONSTEXPR size_t size_with_slack_(size_t n) NOEXCEPT {
        return ((n * SlackFactor) / 128);
    }

    CONSTEXPR u32 num_buckets_() const NOEXCEPT {
        return (_bucketMask ? _bucketMask + 1 : 0);
    }

    void rehash_ForGrowth_() {
        rehash((_bucketMask ? (_bucketMask + 1) : 1) * (bucket_t::Capacity * 2));
    }

    NO_INLINE void clear_NotEmpty_() NOEXCEPT {
        Assert_NoAssume(_bucketMask);

        _size = 0;
        forrange(pbucket, _buckets, _buckets + num_buckets_())
            pbucket->clear();
    }

    NO_INLINE void releaseMemory_NotEmpty_LeaveDirty_() NOEXCEPT {
        Assert_NoAssume(_bucketMask);

        IF_CONSTEXPR(not Meta::has_trivial_destructor<value_type>::value) {
            forrange(pbucket, _buckets, _buckets + num_buckets_())
                pbucket->clear_LeaveDirty();
        }

        allocator_traits::DeallocateT(*this, _buckets, num_buckets_() + 1/* sentinel */);
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
