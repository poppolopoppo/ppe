#pragma once

#include "Allocator/Allocation.h"
#include "Container/BitMask.h"
#include "HAL/PlatformMaths.h"
#include "HAL/PlatformMemory.h"
#include "Memory/MemoryView.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FHopscotchState {
    STATIC_CONST_INTEGRAL(u32, HashMask, 0xFFFF);
    STATIC_CONST_INTEGRAL(u32, NeighborHoodSize, 15);
    STATIC_CONST_INTEGRAL(u32, NeighborHoodMask, 0x7FFF);

    u32 Hash    : 16;
    u32 Filled  : 1;
    u32 Bitmap  : 15;
};
STATIC_ASSERT(Meta::TIsPod_v<FHopscotchState>);
template <
    typename _Key
,   typename _Hash = Meta::THash<_Key>
,   typename _EqualTo = Meta::TEqualTo<_Key>
,   typename _Allocator = ALLOCATOR(Container, _Key)
>   class EMPTY_BASES THopscotchHashSet
:   _Allocator {
public:
    typedef _Key value_type;
    typedef _Hash hasher;
    typedef _EqualTo key_equal;

    using allocator_type = _Allocator;
    using allocator_traits = std::allocator_traits<allocator_type>;

    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef Meta::TAddPointer<value_type> pointer;
    typedef Meta::TAddPointer<const value_type> const_pointer;

    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

    template <typename T>
    struct TIterator {
        const THopscotchHashSet* Owner;
        size_t Index;

        TIterator(THopscotchHashSet* owner, size_t index)
            : Owner(owner)
            , Index(index) {
            Assert_NoAssume(Owner);
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
            const state_t* const states = Owner->states_();
            for (; Index < Owner->_capacity && not states[Index].Filled; ++Index);
            return (*this);
        }

        TIterator& Advance() {
            Index++;
            return FirstSet();
        }
    };

    using iterator = TIterator<_Key>;
    using const_iterator = TIterator<const _Key>;

    CONSTEXPR THopscotchHashSet() NOEXCEPT
        : _size(0)
        , _capacity(0)
        , _elements(nullptr)
    {}

    ~THopscotchHashSet() {
        clear_ReleaseMemory();
    }

    THopscotchHashSet(const THopscotchHashSet& other) : THopscotchHashSet() {
        operator =(other);
    }
    THopscotchHashSet& operator =(const THopscotchHashSet& other) {
        clear();
        if (other._size) {
            const u32 allocation_size = allocation_size_(other._capacity);

            _size = other._size;
            _capacity = other._capacity;
            _elements = allocator_traits::allocate(allocator_(), allocation_size);

            // trivial copy, don't try to rehash
            IF_CONSTEXPR(Meta::has_trivial_copy<value_type>::value) {
                FPlatformMemory::MemcpyLarge(
                    _elements,
                    other._elements,
                    allocation_size * sizeof(value_type) );
            }
            else {
                state_t* const statesDst = states_();
                const state_t* statesSrc = other.states_();

                forrange(i, 0, _capacity) {
                    statesDst[i] = statesSrc[i];
                    if (statesSrc[i].Filled) {
                        allocator_traits::construct(allocator_(),
                            _elements + i,
                            other._elements[i] );
                    }
                }
            }
        }
        return (*this);
    }

    CONSTEXPR THopscotchHashSet(THopscotchHashSet&& rvalue) NOEXCEPT : THopscotchHashSet() {
        std::swap(_size, rvalue._size);
        std::swap(_capacity, rvalue._capacity);
        std::swap(_elements, rvalue._elements);
    }
    THopscotchHashSet& operator =(THopscotchHashSet&& rvalue) {
        clear_ReleaseMemory();
        std::swap(_size, rvalue._size);
        std::swap(_capacity, rvalue._capacity);
        std::swap(_elements, rvalue._elements);
        return (*this);
    }

    bool empty() const { return (0 == _size); }
    size_t size() const { return _size; }
    size_t capacity() const { return _capacity; }

    iterator begin() { return iterator(this, 0).FirstSet(); }
    iterator end() { return iterator(this, _capacity); }

    const_iterator begin() const { return const_iterator(this, 0).FirstSet(); }
    const_iterator end() const { return const_iterator(this, _capacity); }

    TPair<iterator, bool> insert(const _Key& key) {
        reserve_Additional(1);

        const u32 mask = (_capacity - 1);
        const u32 hash = hash_key_(key);
        const u32 bk = (hash & mask);

        if (states_()[bk].Bitmap) {
            const u32 fnd = find_(mask, hash, key);
            if (fnd < _capacity)
                return MakePair(iterator{ this, fnd }, false);
        }

        u32 ins = insert_AssumeUnique_(mask, hash, key);
        if (Unlikely(_capacity == ins))
            ins = rehash_ForCollision_(hash, key);

        Assert_NoAssume(ins < _capacity);
        return MakePair(iterator{ this, ins }, true);
    }

    iterator insert_AssertUnique(const _Key& key) {
        reserve_Additional(1);

        const u32 mask = (_capacity - 1);
        const u32 hash = hash_key_(key);
        const u32 bk = (hash & mask);

        Assert_NoAssume(_capacity == find_(mask, hash, key));

        return insert_AssumeUnique_(mask, hash, key);
    }

    FORCE_INLINE iterator find(const _Key& key) {
        const u32 mask = (_capacity - 1);
        const u32 hash = hash_key_(key);
        const u32 bk = (hash & mask);

        return iterator{ this, find_(mask, hash, key) };
    }

    FORCE_INLINE const_iterator find(const _Key& key) const {
        const u32 mask = (_capacity - 1);
        const u32 hash = hash_key_(key);
        const u32 bk = (hash & mask);

        return const_iterator{ this, find_(mask, hash, key) };
    }

    bool erase(const _Key& key) {
        const u32 mask = (_capacity - 1);
        const u32 hash = hash_key_(key);
        const u32 bk = (hash & mask);

        const u32 fnd = find_(mask, hash, key);
        return (fnd < _capacity
            ? erase_At_(mask, fnd), true
            : false );
    }

    void erase(iterator it) {
        Assert_NoAssume(this == it.Owner);
        AssertRelease(it.Index < _capacity);
        erase_At_(_capacity - 1, it.Index);
    }
    void erase(const_iterator it) {
        Assert_NoAssume(this == it.Owner);
        AssertRelease(it.Index < _capacity);
        erase_At_(_capacity - 1, it.Index);
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
        const u32 oldCapacity = _capacity;
        pointer const oldElements = _elements;
        const state_t* const oldStates = states_();

        _capacity = FPlatformMaths::NextPow2(checked_cast<u32>(n));
        _elements = allocator_traits::allocate(allocator_(), allocation_size_(_capacity));

        Assert_NoAssume(oldCapacity < _capacity);
        Assert_NoAssume(n <= _capacity);
        AssertRelease(_capacity <= state_t::HashMask); // else we would need more bits in state_t::Hash

        state_t* const states = states_();
        FPlatformMemory::Memzero(states, sizeof(state_t) * _capacity);

        if (_size) {
            Assert_NoAssume(_elements);
            ONLY_IF_ASSERT(const u32 oldSize = _size);

            _size = 0; // reset size before inserting back all elements

            const u32 mask = (_capacity - 1);
            forrange(i, 0, oldCapacity) {
                if (oldStates[i].Filled) {
                    VerifyRelease(insert_AssumeUnique_(
                        mask, oldStates[i].Hash, // reuse stored hash value
                        std::move(oldElements[i]) )
                        < _capacity); // already rehashing, can't rehash twice !
                    Meta::Destroy(oldElements + i);
                }
            }

            Assert_NoAssume(oldSize == _size);
        }

        if (oldElements)
            allocator_traits::deallocate(allocator_(), oldElements, allocation_size_(oldCapacity));
    }

private:
    u32 _size;
    u32 _capacity;
    pointer _elements;

    using state_t = FHopscotchState;

    STATIC_CONST_INTEGRAL(u32, MaxLoadFactor, 90);
    STATIC_CONST_INTEGRAL(u32, SlackFactor, ((100 - MaxLoadFactor) * 128) / 100);

    FORCE_INLINE allocator_type& allocator_() NOEXCEPT { return static_cast<allocator_type&>(*this); }
    FORCE_INLINE state_t* states_() const NOEXCEPT { return reinterpret_cast<state_t*>(_elements + _capacity); }

    static FORCE_INLINE CONSTEXPR u32 allocation_size_(u32 capacity) NOEXCEPT {
        return u32((capacity * (sizeof(value_type) + sizeof(state_t)) + sizeof(value_type) - 1) / sizeof(value_type));
    }

    static FORCE_INLINE u32 hash_key_(const _Key& key) NOEXCEPT {
        return (u32(hasher()(key)) & state_t::HashMask);
    }

    static FORCE_INLINE CONSTEXPR u32 distance_(u32 wanted, u32 bucket, u32 numStatesM1) NOEXCEPT {
        return (wanted <= bucket
            ? bucket - wanted
            : bucket + (numStatesM1 + 1) - wanted);
    }

    FORCE_INLINE u32 find_(u32 mask, u32 hash, const _Key& key) const {
        // always compare the key first, for fast trivial case with only 1 cache miss
        return (Likely(_size && key_equal()(_elements[hash & mask], key))
            ? hash & mask
            : find_Bitmap_(mask, hash, key) );
    }

    NO_INLINE u32 find_Bitmap_(u32 mask, u32 hash, const _Key& key) const {
        if (Likely(_size)) {
            // follow the bitmap to find the key
            const state_t* const states = states_();

            const u32 bk = (hash & mask);
            const state_t& st = states[bk];

            TBitMask<u32> m{ st.Bitmap };
            while (m) {
                const u32 off = m.PopFront_AssumeNotEmpty();
                const u32 idx = (hash + off) & mask;
                if (states[idx].Hash == hash && key_equal()(_elements[idx], key))
                    return idx;
            }
        }
        return _capacity;
    }

    NO_INLINE u32 rehash_ForCollision_(u32 hash, const _Key& key) {
        const u32 n = _capacity * 2;
        rehash(n); // rehash table with next power of 2
        Assert_NoAssume(_capacity == n);
        return find_(_capacity - 1, hash, key);
    }

    template <typename... _Args>
    u32 insert_AssumeUnique_(u32 mask, u32 hash, _Args... args) {
        _size++;

        state_t* const states = states_();

        const u32 bk = (hash & mask);
        state_t& st = states[bk];

        // trivial case : there's empty space in bucket's neighborhood
        TBitMask<u32> bits{ ~u32(st.Bitmap) & state_t::NeighborHoodMask };
        while (Likely(bits.Data)) {
            const u32 off = bits.PopFront_AssumeNotEmpty();
            const u32 idx = (bk + off) & mask;
            state_t& ot = states[idx];
            if (not ot.Filled) {
                ot.Hash = hash;
                ot.Filled = true;
                st.Bitmap |= (u32(1) << off);
                Meta::Construct(_elements + idx, std::forward<_Args>(args)...);
                return idx;
            }
        }

        // fall back to non-trivial insertion
        return insert_NonTrivial_(mask, hash, std::forward<_Args>(args)...);
    }

    template <typename... _Args>
    NO_INLINE u32 insert_NonTrivial_(u32 mask, u32 hash, _Args... args) {
        Assert_NoAssume(_size < _capacity); // there must be some free space available

        state_t* const states = states_();
        const u32 bk = (hash & mask);

#ifdef WITH_PPE_ASSERT
        // check that everything is filled in the neighborhood (for debug)
        forrange(off, 0, state_t::NeighborHoodSize) {
            const u32 idx = (bk + off) & mask;
            Assert(states[idx].Filled);
        }
#endif //!WITH_PPE_ASSERT

        // first we need to find a free slot for insertion
        u32 off = state_t::NeighborHoodSize;
        for (;; ++off) {
            const u32 idx = (bk + off) & mask;
            if (not states[idx].Filled)
                break;
        }

        // recursively switch position (if possible) while not in neighborhood
        u32 ins = (bk + off) & mask;
        for (u32 i = off; i; --i) {
            const u32 swp = (bk + i - 1) & mask;
            const u32 wnt = (states[swp].Hash & mask);
            Assert_NoAssume(states[swp].Filled);

            const u32 d_ins = distance_(wnt, ins, mask);
            Assert_NoAssume(not (states[wnt].Bitmap & (u64(1) << d_ins)));

            if (d_ins < state_t::NeighborHoodSize) {
                const u32 d_swp = distance_(wnt, swp, mask);
                Assert_NoAssume(d_swp < d_ins);
                Assert_NoAssume(states[wnt].Bitmap & (u32(1) << d_swp));

                states[wnt].Bitmap = (states[wnt].Bitmap & ~(u32(1) << d_swp)) | (u32(1) << d_ins);
                states[ins].Filled = true;
                states[ins].Hash = states[swp].Hash;

                Meta::Construct(_elements + ins, std::move(_elements[swp]));
                Meta::Destroy(_elements + swp);

                ins = swp;

                if (i <= state_t::NeighborHoodSize)
                    break;
            }
        }

        const u32 d = distance_(bk, ins, mask);

        states[bk].Bitmap |= (u32(1) << d);
        states[ins].Filled = true;
        states[ins].Hash = hash;

        Meta::Construct(_elements + ins, std::forward<_Args>(args)...);

        return (d < state_t::NeighborHoodSize ? ins : _capacity);
    }

    void erase_At_(u32 mask, u32 index) {
        Assert(_size);
        Assert_NoAssume(index < _capacity);

        state_t* const states = states_();

        const u32 bk = (states[index].Hash & mask);
        const u32 d = distance_(bk, index, mask);

        Assert_NoAssume(states[index].Filled);
        Assert_NoAssume(states[bk].Bitmap & (u32(1) << d));

        states[bk].Bitmap &= ~(u32(1) << d);
        states[index].Filled = false;
        states[index].Hash = 0;

        Meta::Destroy(_elements + index);

        _size--;
    }

    void clear_AssumeNotEmpty_() {
        Assert(_size);

        state_t* const states = states_();

        IF_CONSTEXPR(not Meta::has_trivial_destructor<value_type>::value) {
            ONLY_IF_ASSERT(size_t dbgSize = 0);
            forrange(b, 0, _capacity) {
                if (states[b].Filled) {
                    Meta::Destroy(_elements + b);
                    ONLY_IF_ASSERT(dbgSize++);
                }
            }
            Assert_NoAssume(dbgSize == _size);
        }

        ONLY_IF_ASSERT(FPlatformMemory::Memdeadbeef(_elements, sizeof(value_type) * _capacity));

        FPlatformMemory::Memzero(states, sizeof(state_t) * _capacity);

        _size = 0;
    }

    void clear_ReleaseMemory_AssumeNotEmpty_() {
        if (_size)
            clear_AssumeNotEmpty_();

        allocator_traits::deallocate(allocator_(), _elements, allocation_size_(_capacity));
        _capacity = 0;
        _elements = nullptr;
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
