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
    STATIC_CONST_INTEGRAL(u32, NeighborHoodSize, 8);
    STATIC_CONST_INTEGRAL(u32, NHS, NeighborHoodSize); // shorter alias

    u32 Hash    : 24;
    u32 Bitmap  : 7;
    u32 Filled  : 1;
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

        T& operator *() const { Assert(Index < Capacity); return (Owner->_elements[Index]); }
        T* operator ->() const { Assert(Index < Capacity); return (Owner->_elements + Index); }

        inline friend bool operator ==(const TIterator& lhs, const TIterator& rhs) {
            Assert_NoAssume(lhs.Owner == rhs.Owner);
            return (lhs.Index == rhs.Index);
        }
        inline friend bool operator !=(const TIterator& lhs, const TIterator& rhs) {
            return (not operator ==(lhs, rhs));
        }

        TIterator& Advance() {
            Assert_NoAssume(Index < Owner->_capacity);
            const state_t* const states = Owner->states_();
            for (; (not states[Index].Filled) & (Index < Owner->_capacity); ++Index);
            return (*this);
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

    THopscotchHashSet(THopscotchHashSet&& rvalue) : THopscotchHashSet() {
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

    iterator begin() { return iterator(this, 0).Advance(); }
    iterator end() { return iterator(this, _capacity); }

    const_iterator begin() const { return const_iterator(this, 0).Advance(); }
    const_iterator end() const { return const_iterator(this, _capacity); }

    TPair<iterator, bool> insert(const _Key& key) {
        if (Unlikely(_size == _capacity))
            reserve_Additional(1);

        const u32 numStatesM1 = (NumStates_(_capacity) - 1);
        const u32 h = u32(hasher()(key));
        u32 s = (h & numStatesM1);
        for (;;) {
            FDenseHashTableState& it = _states[s];
            if (Unlikely(it.Hash == u16(h) && key_equal()(key, _elements[it.Index])))
                return MakePair(MakeCheckedIterator(_elements, _size, it.Index), false);
            if ((it.Index & FDenseHashTableState::EmptyMask) == FDenseHashTableState::EmptyMask)
                break;

            // linear probing
            s = ++s & numStatesM1;
        }

        const u16 index = checked_cast<u16>(_size++);
        key_traits::construct(key_alloc(), _elements + index, key);

        FDenseHashTableState& st = _states[s];
        Assert_NoAssume(
            FDenseHashTableState::EmptyIndex == st.Index ||
            FDenseHashTableState::TombIndex == st.Index );
        Assert_NoAssume(
            FDenseHashTableState::EmptyIndex == st.Hash );

        st.Index = index;
        st.Hash = u16(h);

        return MakePair(MakeCheckedIterator(_elements, _size, index), true);
    }

    const_iterator find(const _Key& key) const {
        if (Unlikely(0 == _size))
            return end();

        const u32 numStatesM1 = (NumStates_(_capacity) - 1);
        const u32 h = u32(hasher()(key));
        u32 s = (h & numStatesM1);
        for (;;) {
            const FDenseHashTableState& it = _states[s];
            if (it.Hash == u16(h) && key_equal()(key, _elements[it.Index]))
                return MakeCheckedIterator(_elements, _size, it.Index);
            if (it.Index == FDenseHashTableState::EmptyIndex)
                return end();

            // linear probing
            s = ++s & numStatesM1;
        }
    }

    bool erase(const _Key& key) {
        if (0 == _size)
            return false;

        const u32 numStatesM1 = (NumStates_(_capacity) - 1);
        u32 h = u32(hasher()(key));
        u32 s = (h & numStatesM1);
        for (;;) {
            const FDenseHashTableState& it = _states[s];
            if (it.Hash == u16(h) && key_equal()(key, _elements[it.Index]))
                break;
            if (it.Index == FDenseHashTableState::EmptyIndex)
                return false;

            // linear probing
            s = ++s & numStatesM1;
        }

        FDenseHashTableState& st = _states[s];
        key_traits::destroy(key_alloc(), _elements + st.Index);

        if (Likely(_size > 1 && st.Index + 1u < _size)) {
            // need to fill the hole in _elements
            u16 replace = checked_cast<u16>(_size - 1);
            h = u32(hasher()(_elements[replace]));
            s = (h & numStatesM1);
            for (;;) {
                const FDenseHashTableState& it = _states[s];
                if (it.Index == replace)
                    break;

                // linear probing
                s = ++s & numStatesM1;
            }

            Assert_NoAssume(u16(h) == _states[s].Hash);
            _states[s].Index = st.Index;

            key_traits::construct(key_alloc(), _elements + st.Index, std::move(_elements[replace]));
            key_traits::destroy(key_alloc(), _elements + replace);
        }

        st.Index = FDenseHashTableState::TombIndex;
        st.Hash = FDenseHashTableState::EmptyIndex;

        _size--;

        return true;
    }

    void clear() {
        if (_size) {
            const size_t numStates = NumStates_(_capacity);
            FPlatformMemory::Memset(_states, 0xFF, numStates * sizeof(*_states));

            forrange(it, _elements, _elements + _size)
                key_traits::destroy(key_alloc(), it);

            _size = 0;
        }
    }

    void clear_ReleaseMemory() {
        if (_capacity) {
            const size_t numStates = NumStates_(_capacity);
            state_traits::deallocate(state_alloc(), _states, numStates);

            forrange(it, _elements, _elements + _size)
                key_traits::destroy(key_alloc(), it);

            key_traits::deallocate(key_alloc(), _elements, _capacity);

            _size = 0;
            _capacity = 0;
            _elements = nullptr;
            _states = nullptr;
        }
        Assert_NoAssume(0 == _size);
        Assert_NoAssume(0 == _capacity);
        Assert_NoAssume(nullptr == _states);
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
        const state_t const oldStates = states_();

        _capacity = FPlatformMaths::NextPow2(n);
        _elements = allocator_traits::allocate(allocator_(), allocation_size_(_capacity));

        Assert_NoAssume(oldCapacity < _capacity);

        state_t* const states = reinterpret_cast<state_t*>(elements + capacity_);
        FPlatformMemory::Memzero(states, sizeof(state_t) * capacity_);

        if (_size) {
            Assert_NoAssume(_elements);
            ONLY_IF_ASSERT(const u32 oldSize = _size);

            const u32 mask = (_capacity - 1);
            forrange(i, 0, oldCapacity) {
                if (oldStates[i]->Filled)
                    insert_(mask, oldStates->Hash, std::move(oldElements[i]));
            }

            Assert_NoAssume(oldSize == _size);
        }

        if (oldElements)
            allocator_traits::deallocate(allocator_(), oldElements, allocation_size_(oldCapacity));

        _capacity = capacity_;
        _elements = elements;

        Assert_NoAssume(n <= _capacity);
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

    template <typename... _Args
    u32 insert_(u32 mask, u32 hash, _Args... args) {
        state_t* const states = states_();

        // first try to find a free slot in neighborhood
        forrange(i, 0, state_t::NHS) {
            const u32 b = (hash + i) & mask;
            if (not states[b].Filled) {
                states[hash & mask].Bitmap |= u32(1) << i;
                states[b].Hash = hash;
                states[b].Filled = true;
                allocator_traits::construct(allocator_(), _elements[b], std::forward<_Args>(args)...);
                return b;
            }
        }

        for (; states[s]->Filled; ++s)
        if ()
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
