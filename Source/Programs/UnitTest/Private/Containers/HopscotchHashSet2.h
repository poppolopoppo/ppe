﻿#pragma once

#include "Allocator/Allocation.h"
#include "Container/BitMask.h"
#include "HAL/PlatformMaths.h"
#include "HAL/PlatformMemory.h"
#include "Memory/MemoryView.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FHopscotchState2 {
    STATIC_CONST_INTEGRAL(u32, MinCapacity, 8);
    STATIC_CONST_INTEGRAL(u32, NeighborHoodSize, 12);
    STATIC_CONST_INTEGRAL(u32, NeighborHoodMask, 0xFFFul);

    u16 DistP1  : 4;
    u16 Bitmap  : 12;

    CONSTEXPR bool Filled() const NOEXCEPT { return (DistP1 != 0); }
    CONSTEXPR u32 Bucket(u32 mask, u32 index) const NOEXCEPT {
        return ((index + 2) + (mask - DistP1)) & mask; // <=> (index + capacity - DistP1 + 1) & mask
    }
};
STATIC_ASSERT(sizeof(FHopscotchState2) == sizeof(u16));
STATIC_ASSERT(Meta::is_pod_v<FHopscotchState2>);
template <
    typename _Key
,   typename _Hash = Meta::THash<_Key>
,   typename _EqualTo = Meta::TEqualTo<_Key>
,   typename _Allocator = ALLOCATOR(Container)
>   class THopscotchHashSet2 : _Allocator {
public:
    typedef _Key value_type;
    typedef _Hash hasher;
    typedef _EqualTo key_equal;

    using allocator_type = _Allocator;
    using allocator_traits = TAllocatorTraits<allocator_type>;

    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef Meta::TAddPointer<value_type> pointer;
    typedef Meta::TAddPointer<const value_type> const_pointer;

    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

    template <typename T>
    struct TIterator {
        const THopscotchHashSet2* Owner;
        size_t Index;

        TIterator(const THopscotchHashSet2& owner, size_t index)
            : Owner(&owner)
            , Index(index) {
            Assert_NoAssume(Index <= Owner->_capacity);
        }

        TIterator& operator++() { return Advance(); }
        TIterator& operator++(int) { TIterator tmp(*this); Advance(); return tmp; }

        T& operator *() const { Assert_NoAssume(Index < Owner->_capacity); return (Owner->_elements[Index]); }
        T* operator ->() const { Assert_NoAssume(Index < Owner->_capacity); return (Owner->_elements + Index); }

        inline friend bool operator ==(const TIterator& lhs, const TIterator& rhs) {
            Assert_NoAssume(lhs.Owner == rhs.Owner);
            return (lhs.Index == rhs.Index);
        }
        inline friend bool operator !=(const TIterator& lhs, const TIterator& rhs) {
            return (not operator ==(lhs, rhs));
        }

        TIterator& FirstSet() {
            Assert_NoAssume(Index <= Owner->_capacity);
            const u32 n = Owner->_capacity;
            const state_t* const states = Owner->states_();
            for (; Index < n && not states[Index].DistP1; ++Index);
            return (*this);
        }

        TIterator& Advance() {
            Index++;
            return FirstSet();
        }
    };

    using iterator = TIterator<_Key>;
    using const_iterator = TIterator<const _Key>;

    CONSTEXPR THopscotchHashSet2() NOEXCEPT
        : _size(0)
        , _capacity(0)
        , _elements(nullptr)
    {}

    ~THopscotchHashSet2() {
        clear_ReleaseMemory();
    }

    THopscotchHashSet2(const THopscotchHashSet2& other) : THopscotchHashSet2() {
        assign(other);
    }
    THopscotchHashSet2& operator =(const THopscotchHashSet2& other) {
        assign(other);
        return (*this);
    }

    CONSTEXPR THopscotchHashSet2(THopscotchHashSet2&& rvalue) NOEXCEPT
    :   THopscotchHashSet2() {
        std::swap(_size, rvalue._size);
        std::swap(_capacity, rvalue._capacity);
        std::swap(_elements, rvalue._elements);
    }
    THopscotchHashSet2& operator =(THopscotchHashSet2&& rvalue) {
        assign(std::move(rvalue));
        return (*this);
    }

    void assign(const THopscotchHashSet2& other) {
        clear_ReleaseMemory();

        if (not other._size)
            return;

        const u32 allocation_size = allocation_size_(other._capacity);

        _size = other._size;
        _capacity = other._capacity;
        _elements = allocator_traits::template AllocateT<_Key>(*this, allocation_size).data();

        // trivial copy, don't try to rehash
        IF_CONSTEXPR(Meta::has_trivial_copy<value_type>::value) {
            FPlatformMemory::MemcpyLarge(
                _elements,
                other._elements,
                allocation_size * sizeof(value_type));
        }
        else {
            FPlatformMemory::Memzero(_elements, allocation_size * sizeof(value_type));

            state_t* const statesDst = states_();
            const state_t* statesSrc = other.states_();

            forrange(i, 0, _capacity) {
                statesDst[i] = statesSrc[i];
                if (statesSrc[i].Filled())
                    Meta::Construct(_elements + i, other._elements[i]);
            }
        }
    }

    void assign(THopscotchHashSet2&& rvalue) {
        clear_ReleaseMemory();

        std::swap(_size, rvalue._size);
        std::swap(_capacity, rvalue._capacity);
        std::swap(_elements, rvalue._elements);
    }

    bool empty() const { return (0 == _size); }
    size_t size() const { return _size; }
    size_t capacity() const { return _capacity; }

    iterator begin() { return iterator(*this, 0).FirstSet(); }
    iterator end() { return iterator(*this, _capacity); }

    const_iterator begin() const { return const_iterator(*this, 0).FirstSet(); }
    const_iterator end() const { return const_iterator(*this, _capacity); }

    TPair<iterator, bool> insert(const _Key& key) {
        reserve_Additional(1);

        const u32 mask = (_capacity - 1);
        const u32 hash = hash_key_(key);
        const u32 bk = (hash & mask);

        if (states_()[bk].Bitmap) {
            const u32 fnd = find_(mask, hash, key);
            if (fnd < _capacity)
                return MakePair(iterator{ *this, fnd }, false);
        }

        u32 ins = insert_AssumeUnique_(mask, hash, key);
        if (Unlikely(_capacity == ins))
            ins = rehash_ForCollision_(hash, key);

        Assert_NoAssume(ins < _capacity);
        return MakePair(iterator{ *this, ins }, true);
    }

    iterator insert_AssertUnique(const _Key& key) {
        reserve_Additional(1);

        const u32 mask = (_capacity - 1);
        const u32 hash = hash_key_(key);
        const u32 bk = (hash & mask);

        Assert_NoAssume(_capacity == find_(mask, hash, key));

        return iterator{ *this, insert_AssumeUnique_(mask, hash, key) };
    }

    FORCE_INLINE iterator find(const _Key& key) NOEXCEPT {
        if (0 == _size) return end();

        const u32 mask = (_capacity - 1);
        const u32 hash = hash_key_(key);

        return iterator{ *this, find_(mask, hash, key) };
    }

    FORCE_INLINE const_iterator find(const _Key& key) const NOEXCEPT {
        if (0 == _size) return end();

        const u32 mask = (_capacity - 1);
        const u32 hash = hash_key_(key);

        return const_iterator{ *this, find_(mask, hash, key) };
    }

    bool erase(const _Key& key) {
        if (0 == _size) return false;

        const u32 mask = (_capacity - 1);
        const u32 hash = hash_key_(key);
        const u32 fnd = find_(mask, hash, key);

        return (fnd < _capacity
            ? erase_At_(hash & mask, fnd), true
            : false );
    }

    void erase(iterator it) {
        Assert_NoAssume(this == it.Owner);
        erase_It_(it.Index);
    }
    void erase(const_iterator it) {
        Assert_NoAssume(this == it.Owner);
        erase_It_(it.Index);
    }

    FORCE_INLINE void clear() {
        if (_size)
            clear_AssumeNotEmpty_();
    }

    void clear_ReleaseMemory() {
        if (_size | _capacity)
            clear_ReleaseMemory_AssumeNotEmpty_();

        Assert_NoAssume(0 == _size);
        Assert_NoAssume(0 == _capacity);
        Assert_NoAssume(nullptr == _elements);
    }

    FORCE_INLINE void reserve_Additional(size_t num) { reserve(_size + num); }
    void reserve(size_t n) {
        Assert(n);

        n += ((n * SlackFactor) >> 7);
        if (n > _capacity)
            rehash(n);
    }

    NO_INLINE void rehash(size_t n) {
        AssertRelease(_size <= n);

        const u32 oldCapacity = _capacity;
        pointer const oldElements = _elements;
        const state_t* const oldStates = states_();

        _capacity = FPlatformMaths::NextPow2(Max(state_t::MinCapacity, checked_cast<u32>(n)));

        const u32 allocation_size = allocation_size_(_capacity);
        _elements = allocator_traits::template AllocateT<_Key>(*this, allocation_size).data();

        // memzero elements + states
        FPlatformMemory::Memzero(_elements, allocation_size * sizeof(value_type));

        Assert_NoAssume(n <= _capacity);

        if (_size) {
            Assert_NoAssume(_elements);

            ONLY_IF_ASSERT(const u32 oldSize = _size);
            ONLY_IF_ASSERT(size_t dbgSize = 0);
            ONLY_IF_ASSERT(size_t dbgSize2 = 0);

            _size = 0; // reset size before inserting back all elements

            const u32 mask = (_capacity - 1);
            forrange(b, 0, oldCapacity) {
                ONLY_IF_ASSERT(dbgSize2 += FPlatformMaths::popcnt(u32(oldStates[b].Bitmap)));

                if (oldStates[b].Filled()) {
                    const u32 hash = hash_key_(oldElements[b]); // can be amortized with hash memoizer if the hash function is too heavy
                    VerifyRelease(insert_AssumeUnique_(mask, hash, std::move(oldElements[b]))
                        < _capacity ); // already rehashing, can't rehash twice !

                    Meta::Destroy(oldElements + b);

                    ONLY_IF_ASSERT(dbgSize++);
                }
            }

            Assert_NoAssume(oldSize == _size);
            Assert_NoAssume(oldSize == dbgSize);
            Assert_NoAssume(oldSize == dbgSize2 || oldSize == dbgSize2 + 1/* when rehasing for collision there should be one missing bit */);
        }

        if (oldElements)
            allocator_traits::DeallocateT(*this, oldElements, allocation_size_(oldCapacity));
    }

private:
    u32 _size;
    u32 _capacity;
    pointer _elements;

    using state_t = FHopscotchState2;

    STATIC_CONST_INTEGRAL(u32, MaxLoadFactor, 75);
    STATIC_CONST_INTEGRAL(u32, SlackFactor, ((100 - MaxLoadFactor) * 128) / 100);

    FORCE_INLINE allocator_type& allocator_() NOEXCEPT { return static_cast<allocator_type&>(*this); }
    FORCE_INLINE state_t* states_() const NOEXCEPT { return reinterpret_cast<state_t*>(_elements + _capacity); }

    static FORCE_INLINE CONSTEXPR u32 allocation_size_(u32 capacity) NOEXCEPT {
        return u32((capacity * (sizeof(value_type) + sizeof(state_t)) + sizeof(value_type) - 1) / sizeof(value_type));
    }

    static FORCE_INLINE u32 hash_key_(const _Key& key) NOEXCEPT {
        return u32(hasher()(key));
    }

    static FORCE_INLINE CONSTEXPR u32 distance_(u32 wanted, u32 bucket, u32 numStatesM1) NOEXCEPT {
        return (wanted <= bucket
            ? bucket - wanted
            : bucket + (numStatesM1 + 1) - wanted);
    }

    FORCE_INLINE u32 find_(u32 mask, u32 hash, const _Key& key) const NOEXCEPT {
        // always compare the key first, for fast trivial case with only 1 cache miss
        // #TODO ERROR! we assume here than we can compare with an uninitialized key (at least it's zero-ed right after allocation, it should skip most of problems, should it ?)
        return (Likely(key_equal()(_elements[hash & mask], key))
            ? hash & mask
            : find_Bitmap_(mask, hash, key) );
    }

    NO_INLINE u32 find_Bitmap_(u32 mask, u32 hash, const _Key& key) const NOEXCEPT {
        Assert(_size);

        // follow the bitmap to find the key
        const state_t* const states = states_();
        TBitMask<u32> m{ states[hash & mask].Bitmap & u32(~1)/* remove bit 0 since it was already tested in find_() */ };

        while (m) {
            const u32 b = (hash + m.PopFront_AssumeNotEmpty()) & mask;
            Assert_NoAssume(states[b].Filled());
            if (key_equal()(_elements[b], key))
                return b;
        }

        return _capacity;
    }

    NO_INLINE u32 rehash_ForCollision_(u32 hash, const _Key& key) {
        const u32 n = (_capacity * 2);
        rehash(n); // rehash table with next power of 2

        Assert_NoAssume(_capacity == n);

        return find_(_capacity - 1, hash, key);
    }

    template <typename... _Args>
    FORCE_INLINE u32 insert_AssumeUnique_(u32 mask, u32 hash, _Args... args) {
        _size++;

        const u32 bk = (hash & mask);
        state_t& st = states_()[bk];

        // trivial case : wanted slot is available
        if (Likely(not st.Filled())) {
            Assert_NoAssume(not (st.Bitmap & 1));

            st.DistP1 = 1;
            st.Bitmap |= 1;

            Meta::Construct(_elements + bk, std::forward<_Args>(args)...);

            return bk;
        }
        else {
            // fall back to non-trivial insertion
            return insert_NonTrivial_(mask, hash, std::forward<_Args>(args)...);
        }
    }

    NO_INLINE u32 insert_Fixup_(u32 mask, u32 bk, u32 off) {
        state_t* const states = states_();

        // look further ahead, we need to find a free slot for insertion
        u32 ins;
        for (;; ++off) {
            Assert_NoAssume(off < _capacity); // don't loop around, table corrupted ?
            ins = (bk + off) & mask;
            if (not states[ins].Filled())
                break;
        }

        bool firstPass = true;

#define USE_HOPSCOTCH_REPACK_HARDER (0) // <== may degenerate the table, could be better to rehash

        // recursively switch position (if possible) while not in neighborhood
#if USE_HOPSCOTCH_REPACK_HARDER
    HOPSCOTCH_REPACK:
#endif
        for (u32 i = off; i; --i) {
            const u32 swp = (bk + i - 1) & mask;
            Assert_NoAssume(states[swp].Filled());

            const u32 wnt = states[swp].Bucket(mask, swp);
            Assert_NoAssume(distance_(wnt, swp, mask) + 1 == states[swp].DistP1);
            Assert_NoAssume(distance_(wnt, swp, mask) < state_t::NeighborHoodSize);
            Assert_NoAssume(states[wnt].Bitmap & (u32(1) << (states[swp].DistP1 - 1)));

            const u32 d_ins = distance_(wnt, ins, mask);

            // switch closer item with current is still in it's neighborhood
            if (Likely(d_ins < state_t::NeighborHoodSize)) {
                Assert_NoAssume(not (states[wnt].Bitmap & (u64(1) << d_ins)));

                const u32 d_swp = (states[swp].DistP1 - 1);
                Assert_NoAssume(d_swp < d_ins);
                Assert_NoAssume(states[wnt].Bitmap & (u32(1) << d_swp));

                // avoid switching when this is head item, since it disables our best case
                // -> but on second loop allow to move the head, still better than rehashing
                if (firstPass && 0 == d_swp)
                    continue;

                ONLY_IF_ASSERT(const u32 oldBucketSize = FPlatformMaths::popcnt(u32(states[wnt].Bitmap)));

                states[wnt].Bitmap = (states[wnt].Bitmap & ~(u32(1) << d_swp)) | (u32(1) << d_ins);
                states[swp].DistP1 = 0;
                states[ins].DistP1 = (d_ins + 1);

                Assert_NoAssume((d_ins + 1) == states[ins].DistP1);
                Assert_NoAssume(states[ins].Bucket(mask, ins) == wnt);
                Assert_NoAssume(FPlatformMaths::popcnt(u32(states[wnt].Bitmap)) == oldBucketSize);

                Meta::Construct(_elements + ins, std::move(_elements[swp]));
                Meta::Destroy(_elements + swp);

                ins = swp;

                if (i <= state_t::NeighborHoodSize) {
                    Assert_NoAssume(i);
                    return (i - 1);
                }
            }
        }

        off = distance_(bk, ins, mask);
        Assert_NoAssume(off >= state_t::NeighborHoodSize);

        // try to pack a second time, this time heads can be moved
#if USE_HOPSCOTCH_REPACK_HARDER
        if (firstPass) {
            firstPass = false;
            goto HOPSCOTCH_REPACK; // should be very rare to be worth
        }
        else
#endif
        {
            return off; // giving up, the table will be rehashed
        }
    }

    template <typename... _Args>
    NO_INLINE u32 insert_NonTrivial_(u32 mask, u32 hash, _Args... args) {
        Assert_NoAssume(_size <= _capacity); // there must be some free space available (_size already incremented)

        state_t* const states = states_();
        const u32 bk = (hash & mask);

        // first look in the neighborhood
        u32 off = state_t::NeighborHoodSize;
        TBitMask<u32> bits{ u32(~states[bk].Bitmap) & state_t::NeighborHoodMask };
        while (bits.Data) {
            const u32 d = bits.PopFront_AssumeNotEmpty();
            if (not states[(bk + d) & mask].Filled()) {
                off = d;
                break;
            }
        }

        // try to repack neighborhood if we didn't find a free slot
        if (Unlikely(off == state_t::NeighborHoodSize))
            off = insert_Fixup_(mask, bk, off);

        // insert the item even if failed to repack, it will be fixed afterwards
        const u32 ins = (bk + off) & mask;
        Assert_NoAssume(not states[ins].Filled());

        Meta::Construct(_elements + ins, std::forward<_Args>(args)...);

        if (Likely(off < state_t::NeighborHoodSize)) { // can overflow when insert_Fixup_() fails to repack
            Assert_NoAssume(not (states[bk].Bitmap & (u32(1) << off)));

            states[ins].DistP1 = (off + 1);
            states[bk].Bitmap |= u32(1) << off;

            Assert_NoAssume(states[ins].DistP1 == (off + 1));
            Assert_NoAssume(states[ins].Bucket(mask, ins) == bk);

            return ins;
        }
        else {
            states[ins].DistP1 = 1; // will be fixed after rehash, just need to be sure this is filled

            return _capacity; // triggers rehash after return
        }
    }

    void erase_It_(u32 index) {
        AssertRelease(index < _capacity);

        const state_t& st = states_()[index];
        Assert_NoAssume(st.Filled());

        const u32 mask = (_capacity - 1);
        const u32 bk = st.Bucket(mask, index);
        Assert_NoAssume(st.Bitmap & (u32(1) << (st.DistP1 - 1)));

        erase_At_(bk, index);
    }

    void erase_At_(u32 bucket, u32 index) {
        Assert(_size);
        Assert_NoAssume(index < _capacity);

        state_t* const states = states_();
        Assert_NoAssume(states[index].Filled());

        const u32 d = (states[index].DistP1 - 1);
        Assert_NoAssume(states[bucket].Bitmap & (u32(1) << d));

        states[index].DistP1 = 0;
        states[bucket].Bitmap &= ~(u32(1) << d);

        Meta::Destroy(_elements + index);

        _size--;
    }

    void clear_AssumeNotEmpty_() {
        Assert(_size);

        IF_CONSTEXPR(not Meta::has_trivial_destructor<value_type>::value) {
            state_t* const states = states_();

            ONLY_IF_ASSERT(size_t dbgSize = 0);
            ONLY_IF_ASSERT(size_t dbgSize2 = 0);

            forrange(b, 0, _capacity) {
                ONLY_IF_ASSERT(dbgSize2 += FPlatformMaths::popcnt(states[b].Bitmap));

                if (states[b].Filled()) {
                    Meta::Destroy(_elements + b);
                    ONLY_IF_ASSERT(dbgSize++);
                }
            }

            Assert_NoAssume(dbgSize == _size);
            Assert_NoAssume(dbgSize == dbgSize2);
        }

        _size = 0;

        FPlatformMemory::Memzero(_elements, allocation_size_(_capacity) * sizeof(value_type));
    }

    void clear_ReleaseMemory_AssumeNotEmpty_() {
        if (_size)
            clear_AssumeNotEmpty_();

        Assert_NoAssume(0 == _size);

        allocator_traits::DeallocateT(*this, _elements, allocation_size_(_capacity));
        _capacity = 0;
        _elements = nullptr;
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
