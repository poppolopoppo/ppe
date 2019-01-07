#pragma once

#include "Allocator/Allocation.h"
#include "HAL/PlatformMemory.h"
#include "Memory/MemoryView.h"

#include <algorithm>
#include <bitset>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FDenseHashTableState2 {
    u16 Index;
    u16 Hash;
    STATIC_CONST_INTEGRAL(u16, EmptyIndex, u16(-1));
};
STATIC_ASSERT(Meta::TIsPod_v<FDenseHashTableState2>);
template <
    typename _Key
    , typename _Hash = Meta::THash<_Key>
    , typename _EqualTo = Meta::TEqualTo<_Key>
    , typename _Allocator = ALLOCATOR(Container, _Key)
    , u32 _MaxDistance = 5
>   class EMPTY_BASES TDenseHashSet2
    : TRebindAlloc<_Allocator, FDenseHashTableState2>
    , _Allocator {
public:
    typedef FDenseHashTableState2 state_t;
    typedef _Key value_type;
    typedef _Hash hasher;
    typedef _EqualTo key_equal;

    using allocator_key = _Allocator;
    using allocator_state = TRebindAlloc<_Allocator, state_t>;

    using key_traits = std::allocator_traits<allocator_key>;
    using state_traits = std::allocator_traits<allocator_state>;

    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef Meta::TAddPointer<value_type> pointer;
    typedef Meta::TAddPointer<const value_type> const_pointer;

    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

    using iterator = TCheckedArrayIterator<_Key>;
    using const_iterator = TCheckedArrayIterator<const _Key>;

    TDenseHashSet2()
        : _size(0)
        , _numStates(1)
        , _elements(nullptr)
        , _states((state_t*)&_elements) // that way we don't have to check if the container is empty
    {}

    ~TDenseHashSet2() {
        clear_ReleaseMemory();
    }

    TDenseHashSet2(const TDenseHashSet2& other) : TDenseHashSet2() {
        operator =(other);
    }
    TDenseHashSet2& operator =(const TDenseHashSet2& other) {
        clear_ReleaseMemory();
        if (other.size()) {
            _size = other._size;
            _numStates = other._numStates;

            _states = state_traits::allocate(state_alloc(), _numStates);
            _elements = key_traits::allocate(key_alloc(), Capacity_(_numStates));

            FPlatformMemory::MemcpyLarge(_states, other._states, _numStates * sizeof(state_t));

            std::uninitialized_copy(
                MakeCheckedIterator(other._elements, _size, 0),
                MakeCheckedIterator(other._elements, _size, _size),
                MakeCheckedIterator(_elements, _size, 0));
        }
        return (*this);
    }

    TDenseHashSet2(TDenseHashSet2&& rvalue) : TDenseHashSet2() {
        std::swap(_size, rvalue._size);
        std::swap(_numStates, rvalue._numStates);
        std::swap(_elements, rvalue._elements);
        std::swap(_states, rvalue._states);
    }
    TDenseHashSet2& operator =(TDenseHashSet2&& rvalue) {
        clear_ReleaseMemory();
        std::swap(_size, rvalue._size);
        std::swap(_numStates, rvalue._numStates);
        std::swap(_elements, rvalue._elements);
        std::swap(_states, rvalue._states);
        return (*this);
    }

    bool empty() const { return (0 == _size); }
    size_t size() const { return _size; }
    size_t capacity() const { return Capacity_(_numStates); }

    iterator begin() { return MakeCheckedIterator(_elements, _size, 0); }
    iterator end() { return MakeCheckedIterator(_elements, _size, _size); }

    const_iterator begin() const { return MakeCheckedIterator(_elements, _size, 0); }
    const_iterator end() const { return MakeCheckedIterator(_elements, _size, _size); }

    TMemoryView<const _Key> MakeView() const { return TMemoryView<_Key>(_elements, _size); }

    TPair<iterator, bool> insert(const _Key& key) {
        if (Unlikely(_size == Capacity_(_numStates)))
            reserve_Additional(1);

        Assert_NoAssume(_numStates);

        const u32 numStatesM1 = (_numStates - 1);
        u16 h = HashKey_(key);
        u32 s = (h & numStatesM1);
        u16 i = checked_cast<u16>(_size);
        u32 d = 0;
        for (;; s = (s + 1) & numStatesM1, ++d) {
            state_t& it = _states[s];
            if (it.Index == state_t::EmptyIndex)
                break;
            else if (Unlikely(((it.Hash == h) & (it.Index != state_t::EmptyIndex)) && key_equal()(key, _elements[it.Index])))
                return MakePair(MakeCheckedIterator(_elements, _size, it.Index), false);

            // minimize distance between desired pos and insertion pos
            const u32 ds = DistanceIndex_(it, s, numStatesM1);
            if (ds < d) {
                d = ds;
                std::swap(i, it.Index);
                std::swap(h, it.Hash);
            }
        }

        state_t& st = _states[s];
        Assert_NoAssume(state_t::EmptyIndex == st.Index);
        Assert_NoAssume(u16(s) == st.Hash);

        st.Index = i;
        st.Hash = h;

        _size++;

        key_traits::construct(key_alloc(), _elements + _size - 1, key);

        if (Likely(d <= MaxDistance))
            return MakePair(MakeCheckedIterator(_elements, _size, _size - 1), true);
        else
            return MakePair(grow_rehash(key), true);
    }

    iterator find(const _Key& key) {
        const u32 numStatesM1 = (_numStates - 1);

        u16 i;
        const u16 h = HashKey_(key);
        for (u32 s = (h & numStatesM1), d = 0;; s = (s + 1) & numStatesM1, ++d) {
            const state_t& it = _states[s];
            if (((it.Hash == h) & (it.Index != state_t::EmptyIndex)) && key_equal()(key, _elements[it.Index]))
                i = it.Index;
            else if (d > DistanceIndex_(it, s, numStatesM1))
                i = u16(_size);
            else
                continue;
            break;
        }

        return MakeCheckedIterator(_elements, _size, i);
    }

    const_iterator find(const _Key& key) const {
        return const_cast<TDenseHashSet2&>(*this).find(key);
    }

    bool erase(const _Key& key) {
        Assert_NoAssume(Meta::IsPow2(_numStates));
        const u32 numStatesM1 = (_numStates - 1);

        const u16 h = HashKey_(key);
        u32 s = (h & numStatesM1);
        for (u32 d = 0;; s = (s + 1) & numStatesM1, ++d) {
            const state_t& it = _states[s];
            if (((it.Hash == h) & (it.Index != state_t::EmptyIndex)) && key_equal()(key, _elements[it.Index]))
                break;
            else if (d > DistanceIndex_(it, s, numStatesM1))
                return false;

            Assert_NoAssume(d <= MaxDistance);
        }

        eraseAt_(s);
        return true;
    }

    void clear() {
        if (_size) {
            Assert(_numStates);

            forrange(i, 0, _numStates) {
                state_t& st = _states[i];
                st.Index = state_t::EmptyIndex;
                st.Hash = u16(i);  // reset Hash to Index so DistanceIndex_() == 0
            }

            Destroy(key_alloc(), MakeView());

            _size = 0;
        }
    }

    void clear_ReleaseMemory() {
        if (_elements) {
            state_traits::deallocate(state_alloc(), _states, _numStates);

            Destroy(key_alloc(), MakeView());

            key_traits::deallocate(key_alloc(), _elements, Capacity_(_numStates));

            _size = 0;
            _numStates = 1;
            _elements = nullptr;
            _states = (state_t*)&_elements; // that way we don't have to check if the container is empty
        }
        Assert_NoAssume(0 == _size);
        Assert_NoAssume(1 == _numStates);
        Assert_NoAssume((state_t*)&_elements == _states);
        Assert_NoAssume(nullptr == _elements);
    }

    NO_INLINE iterator grow_rehash(const _Key& key) {
        rehash(Capacity_(_numStates) + 1); // rounded to next pow2, can be *huge*
        return find(key);
    }

    FORCE_INLINE void reserve_Additional(size_t num) { reserve(_size + num); }
    NO_INLINE void reserve(size_t n) {
        if (n <= Capacity_(_numStates))
            return;

        rehash(n);
    }

    void rehash(size_t n) {
        Assert(n);
        Assert_NoAssume(n > Capacity_(_numStates));

        const state_t* oldStates = _states;
        const u32 oldNumStates = _numStates;

        _numStates = checked_cast<u32>(Max(16, FPlatformMaths::NextPow2(
            SafeAllocatorSnapSize(state_alloc(), n * 2 + 1))));
        Assert_NoAssume(oldNumStates < _numStates);

        const u32 capacity = Capacity_(_numStates);
        Assert(capacity >= n);
        Assert_NoAssume(capacity <= _numStates);

        _states = state_traits::allocate(state_alloc(), _numStates);

        forrange(i, 0, _numStates) {
            state_t& st = _states[i];
            st.Index = state_t::EmptyIndex;
            st.Hash = u16(i);  // reset Hash to Index so DistanceIndex_() == 0
        }

        if (_elements) {
            Assert(oldStates);
            Assert_NoAssume((state_t*)&_elements != oldStates);

            _elements = Relocate(
                key_alloc(),
                TMemoryView<_Key>(_elements, _size),
                capacity, Capacity_(oldNumStates) );

            // rehash using previous state which already contains the hash keys
            const u32 numStatesM1 = (_numStates - 1);
            forrange(p, oldStates, oldStates + oldNumStates) {
                if (state_t::EmptyIndex == p->Index)
                    continue;

                u16 i = p->Index;
                u16 h = p->Hash; // don't need more entropy
                u32 s = (h & numStatesM1);
                u32 d = 0;
                for (;; s = (s + 1) & numStatesM1, ++d) {
                    state_t& it = _states[s];
                    if (it.Index == state_t::EmptyIndex)
                        break;

                    Assert_NoAssume(i != it.Index);

                    const u32 ds = DistanceIndex_(it, s, numStatesM1);
                    if (ds < d) {
                        d = ds;
                        std::swap(i, it.Index);
                        std::swap(h, it.Hash);
                    }
                }

                state_t& st = _states[s];
                st.Index = i;
                st.Hash = h;
            }

            // release previous state vector
            state_traits::deallocate(state_alloc(), const_cast<state_t*>(oldStates), oldNumStates);
        }
        else {
            Assert_NoAssume(0 == _size);
            Assert_NoAssume(nullptr == _elements);
            Assert_NoAssume((state_t*)&_elements == oldStates);

            _elements = key_traits::allocate(key_alloc(), capacity);
        }

        Assert_NoAssume(n <= capacity);
    }

private:
    u32 _size;
    u32 _numStates;
    _Key* _elements;
    state_t* _states;

    static constexpr u32 MaxDistance = _MaxDistance;
    static constexpr u32 MaxLoadFactor = 75;
    static constexpr u32 SlackFactor = (((100 - MaxLoadFactor) * 128) / 100);

    FORCE_INLINE allocator_key& key_alloc() NOEXCEPT { return static_cast<allocator_key&>(*this); }
    FORCE_INLINE allocator_state& state_alloc() NOEXCEPT { return static_cast<allocator_state&>(*this); }

    static FORCE_INLINE u16 HashKey_(const _Key& key) NOEXCEPT {
        return u16(hasher()(key)); // the index is used to identify empty slots, so use full 16 bits for hash key
    }

    FORCE_INLINE u32 Capacity_(u32 numStates) const NOEXCEPT {
        return (numStates > 1 ? checked_cast<u32>(SafeAllocatorSnapSize(
            static_cast<const allocator_key&>(*this), numStates - ((numStates * SlackFactor) >> 7) + 1)) : 0);
    }

    NO_INLINE void eraseAt_(u32 s) {
        Assert(_size);
        Assert_NoAssume(s < _numStates);

        const u32 numStatesM1 = (_numStates - 1);

        // destroy the element
        state_t& st = _states[s];

        if (_size > 1 && st.Index + 1u != _size) {
            // need to fill the hole in _elements
            const u16 ri = checked_cast<u16>(_size - 1);
            const u16 rh = HashKey_(_elements[ri]);

            u32 rs = (rh & numStatesM1);
            for (;; rs = (rs + 1) & numStatesM1) {
                if (_states[rs].Index == ri)
                    break;
            }

            Assert_NoAssume(u16(rh) == _states[rs].Hash);
            _states[rs].Index = st.Index;

            _elements[st.Index] = std::move(_elements[ri]);

            key_traits::destroy(key_alloc(), _elements + ri);
        }
        else {
            key_traits::destroy(key_alloc(), _elements + st.Index);
        }

        // backward shift deletion to avoid using tombstones
        forrange(i, 0, numStatesM1) {
            const u32 prev = (s + i) & numStatesM1;
            const u32 swap = (s + i + 1) & numStatesM1;

            if (DistanceIndex_(_states[swap], swap, numStatesM1) == 0) {
                _states[prev].Index = state_t::EmptyIndex;
                _states[prev].Hash = u16(prev); // reset Hash to Index so DistanceIndex_() == 0
                break;
            }

            _states[prev] = _states[swap];
        }

        // finally decrement the size
        _size--;
    }

    static FORCE_INLINE CONSTEXPR u32 DistanceIndex_(state_t st, u32 b, u32 numStatesM1) NOEXCEPT {
        return ((st.Hash & numStatesM1) <= b
            ? b - (st.Hash & numStatesM1)
            : b + (numStatesM1 + 1 - (st.Hash & numStatesM1)) );
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
