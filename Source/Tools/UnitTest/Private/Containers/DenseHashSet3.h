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
struct FDenseHashTableState3 {
    u16 Index;
    u16 Hash;
    STATIC_CONST_INTEGRAL(u16, EmptyIndex, u16(-1));
    FORCE_INLINE void swap(FDenseHashTableState3& other) NOEXCEPT {
        std::swap(reinterpret_cast<u32&>(*this), reinterpret_cast<u32&>(other));
    }
};
STATIC_ASSERT(Meta::is_pod_v<FDenseHashTableState3>);
template <
    typename _Key
    , typename _Hash = Meta::THash<_Key>
    , typename _EqualTo = Meta::TEqualTo<_Key>
    , typename _Empty = Meta::TEmptyKey<_Key>
    , typename _Allocator = ALLOCATOR(Container)
>   class TDenseHashSet3 : _Allocator {
public:
    typedef FDenseHashTableState3 state_t;
    typedef _Key value_type;
    typedef _Hash hasher;
    typedef _Empty key_empty;
    typedef _EqualTo key_equal;

    using allocator_type = _Allocator;
    using allocator_traits = TAllocatorTraits<allocator_type>;

    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef Meta::TAddPointer<value_type> pointer;
    typedef Meta::TAddPointer<const value_type> const_pointer;

    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

    using iterator = TCheckedArrayIterator<_Key>;
    using const_iterator = TCheckedArrayIterator<const _Key>;

    TDenseHashSet3() NOEXCEPT
        : _size(0)
        , _sizeClass(0)
        , _capacity(0)
        , _elements((pointer)this) {
        // that way we don't have to check if the container is empty
    }

    ~TDenseHashSet3() {
        clear_ReleaseMemory();
    }

    TDenseHashSet3(const TDenseHashSet3& other) : TDenseHashSet3() {
        operator =(other);
    }
    TDenseHashSet3& operator =(const TDenseHashSet3& other) {
        clear_ReleaseMemory();
        if (other._size) {
            _size = other._size;
            _sizeClass = other._sizeClass;
            _capacity = other._capacity;

            const u32 numAllocatedBlocks = NumAllocatedBlocks_();
            _elements = allocator_traits::template AllocateT<_Key>(*this, numAllocatedBlocks).data();

            // trivial copy, don't try to rehash
            IF_CONSTEXPR(Meta::has_trivial_copy<_Key>::value) {
                FPlatformMemory::MemcpyLarge(
                    _elements,
                    other._elements,
                    numAllocatedBlocks * sizeof(_Key) );
            }
            else {
                std::uninitialized_copy(
                    MakeCheckedIterator(other._elements, _size, 0),
                    MakeCheckedIterator(other._elements, _size, _size),
                    MakeCheckedIterator(_elements, _size, 0) );

                const u32 numStates = NumStates_(_sizeClass);
                FPlatformMemory::MemcpyLarge(
                    _elements + _capacity,
                    other._elements + _capacity,
                    numStates * sizeof(state_t) );
            }
        }
        return (*this);
    }

    TDenseHashSet3(TDenseHashSet3&& rvalue) : TDenseHashSet3() {
        FPlatformMemory::Memswap(this, &rvalue, sizeof(*this));
    }
    TDenseHashSet3& operator =(TDenseHashSet3&& rvalue) {
        Assert(&rvalue != this);
        clear_ReleaseMemory();
        FPlatformMemory::Memswap(this, &rvalue, sizeof(*this));
        return (*this);
    }

    bool empty() const { return (0 == _size); }
    size_t size() const { return _size; }
    size_t capacity() const { return _capacity; }

    iterator begin() { return MakeCheckedIterator(_elements, _size, 0); }
    iterator end() { return MakeCheckedIterator(_elements, _size, _size); }

    const_iterator begin() const { return MakeCheckedIterator(const_cast<const_pointer>(_elements), _size, 0); }
    const_iterator end() const { return MakeCheckedIterator(const_cast<const_pointer>(_elements), _size, _size); }

    auto MakeView() const { return TMemoryView<const _Key>(_elements, _size); }

    TPair<iterator, bool> insert(const _Key& key) {
        if (Unlikely(_size == _capacity))
            Rehash_(_sizeClass + 1);

        Assert_NoAssume(_size < _capacity);
        const u32 numStatesM1 = (NumStates_(_sizeClass) - 1);

        state_t* const states = reinterpret_cast<state_t*>(_elements + _capacity);

        state_t e{ checked_cast<u16>(_size), HashKey_(key) };
        u32 s = (e.Hash & numStatesM1);
        u32 d = 0;
        for (;; s = (s + 1) & numStatesM1, ++d) {
            state_t& it = states[s];
            if (it.Index == state_t::EmptyIndex)
                break;
            else if (Unlikely(((it.Hash == e.Hash) & (it.Index != state_t::EmptyIndex)) && key_equal()(key, _elements[it.Index])))
                return MakePair(MakeCheckedIterator(_elements, _size, it.Index), false);

            // minimize distance between desired pos and insertion pos
            const u32 ds = DistanceIndex_(it, s, numStatesM1);
            if (ds < d) {
                d = ds;
                e.swap(it);
            }
        }

        // no state alteration before here

        state_t& st = states[s];
        Assert_NoAssume(state_t::EmptyIndex == st.Index);
        Assert_NoAssume(u16(s) == st.Hash);

        st = e;

        Meta::Construct(_elements + _size++, key);

        if (Likely(d <= MaxDistance))
            return MakePair(MakeCheckedIterator(_elements, _size, _size - 1), true);
        else
            return MakePair(RehashForGrowth_(key), true);
    }

    iterator find(const _Key& key) NOEXCEPT {
        Assert_NoAssume(_sizeClass < NumSizeClasses);
        const u32 numStatesM1 = (NumStates_(_sizeClass) - 1);

        state_t* const states = reinterpret_cast<state_t*>(_elements + _capacity);

        u16 i;
        const u16 h = HashKey_(key);
        for (u32 s = (h & numStatesM1), d = 0;; s = (s + 1) & numStatesM1, ++d) {
            const state_t& it = states[s];
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

    FORCE_INLINE const_iterator find(const _Key& key) const NOEXCEPT {
        return const_cast<TDenseHashSet3&>(*this).find(key);
    }

    bool erase(const _Key& key) {
        Assert(_sizeClass < NumSizeClasses);
        const u32 numStatesM1 = (NumStates_(_sizeClass) - 1);

        state_t* const states = reinterpret_cast<state_t*>(_elements + _capacity);

        const u16 h = HashKey_(key);
        u32 s = (h & numStatesM1);
        for (u32 d = 0;; s = (s + 1) & numStatesM1, ++d) {
            const state_t& it = states[s];
            if (((it.Hash == h) & (it.Index != state_t::EmptyIndex)) && key_equal()(key, _elements[it.Index]))
                break;
            else if (d > DistanceIndex_(it, s, numStatesM1))
                return false;

            Assert_NoAssume(d <= MaxDistance);
        }

        EraseAt_(s, numStatesM1);
        return true;
    }

    void clear() {
        if (_size) {
            Assert(_sizeClass);
            Assert(_capacity);
            Assert_NoAssume(_sizeClass < NumSizeClasses);

            const u32 numStates = NumStates_(_sizeClass);

            state_t* const states = reinterpret_cast<state_t*>(_elements + _capacity);

            forrange(i, 0, numStates) { // #TODO : faster
                state_t& st = states[i];
                st.Index = state_t::EmptyIndex;
                st.Hash = u16(i);  // reset Hash to Index so DistanceIndex_() == 0
            }

            Destroy(allocator_(), MakeView());

            _size = 0;
        }
    }

    FORCE_INLINE void clear_ReleaseMemory() {
        if (_capacity)
            ReleaseMemory_();
        Assert_NoAssume(0 == _size);
        Assert_NoAssume(0 == _sizeClass);
        Assert_NoAssume(0 == _capacity);
        Assert_NoAssume((_Key*)this == _elements);
    }

    FORCE_INLINE void reserve_Additional(size_t num) { reserve(_size + num); }

    void reserve(size_t n) {
        const u32 sizeClass = MakeSizeClass_(checked_cast<u32>(n));
        if (sizeClass > _sizeClass)
            Rehash_(sizeClass);
        Assert_NoAssume(n <= _capacity);
    }

private:
    u32 _size : 27;
    u32 _sizeClass : 5;
    u32 _capacity;
    pointer _elements;

    FORCE_INLINE allocator_type& allocator_() NOEXCEPT { return static_cast<allocator_type&>(*this); }

    static constexpr u32 MaxDistance = (CACHELINE_SIZE / sizeof(state_t)) - 1;
    static constexpr u32 MinSizeClass = 3;
    static constexpr u32 NumSizeClasses = 14;
    static constexpr u32 MaxLoadFactor = 50;
    static constexpr u32 SlackFactor = (((100 - MaxLoadFactor) * 128) / 100);

    static FORCE_INLINE u16 HashKey_(const _Key& key) NOEXCEPT {
        return u16(hasher()(key)); // the index is used to identify empty slots, so use full 16 bits for hash key
    }

    static FORCE_INLINE u8 MakeSizeClass_(u32 size) NOEXCEPT {
        size += 2 * ((size * SlackFactor) >> 7);
        return checked_cast<u8>(FPlatformMaths::CeilLog2(u32(ROUND_TO_NEXT_16(size))) - MinSizeClass);
    }

    static FORCE_INLINE CONSTEXPR u32 NumStates_(u32 sizeClass) NOEXCEPT {
        Assert_NoAssume(sizeClass < NumSizeClasses);
        return (u32(1) << (sizeClass + MinSizeClass));
    }

    FORCE_INLINE u32 NumAllocatedBlocks_() const NOEXCEPT {
        return (_capacity +
            (NumStates_(_sizeClass) * sizeof(state_t) + sizeof(_Key) - 1) / sizeof(_Key) );
    }

    static FORCE_INLINE CONSTEXPR u32 DistanceIndex_(state_t st, u32 b, u32 numStatesM1) NOEXCEPT {
        return ((st.Hash & numStatesM1) <= b
            ? b - (st.Hash & numStatesM1)
            : b + (numStatesM1 + 1) - (st.Hash & numStatesM1));
    }

    void EraseAt_(u32 s, u32 numStatesM1) {
        Assert(_size);
        Assert_NoAssume(s <= numStatesM1);

        state_t* const states = reinterpret_cast<state_t*>(_elements + _capacity);

        // destroy the element
        state_t& st = states[s];

        if (_size > 1 && st.Index + 1u != _size) {
            // need to fill the hole in _elements
            const u16 ri = checked_cast<u16>(_size - 1);
            const u16 rh = HashKey_(_elements[ri]); // #TODO ? rehashing can be very slow, but THashMemoizer<> can lessen the problem

            u32 rs = (rh & numStatesM1);
            for (;; rs = (rs + 1) & numStatesM1) {
                if (states[rs].Index == ri)
                    break;
            }

            Assert_NoAssume(u16(rh) == states[rs].Hash);
            states[rs].Index = st.Index;

            IF_CONSTEXPR(Meta::has_trivial_move<_Key>::value) {
                FPlatformMemory::Memcpy(_elements + st.Index, _elements + ri, sizeof(_Key));
            }
            else {
                _elements[st.Index] = std::move(_elements[ri]);
                Meta::Destroy(_elements + ri);
            }
        }
        else {
            Meta::Destroy(_elements + st.Index);
        }

        // backward shift deletion to avoid using tombstones
        forrange(i, 0, numStatesM1) {
            const u32 prev = (s + i) & numStatesM1;
            const u32 swap = (s + i + 1) & numStatesM1;

            if (DistanceIndex_(states[swap], swap, numStatesM1) == 0) {
                states[prev].Index = state_t::EmptyIndex;
                states[prev].Hash = u16(prev); // reset Hash to Index so DistanceIndex_() == 0
                break;
            }

            states[prev] = states[swap];
        }

        // finally decrement the size
        _size--;
    }

    iterator RehashForGrowth_(const _Key& key) {
        Rehash_(_sizeClass + 1);
        return find(key);
    }

    NO_INLINE void Rehash_(u32 sizeClass) {
        AssertRelease(sizeClass < NumSizeClasses);

        pointer const oldElts = _elements;
        state_t* const oldStates = reinterpret_cast<state_t*>(oldElts + _capacity);
        const u32 oldNumStates = NumStates_(_sizeClass);

        const u32 numStates = NumStates_(sizeClass);
        const u32 numBlocksForStates = (numStates * sizeof(state_t) + sizeof(_Key) - 1) / sizeof(_Key);
        u32 numBlocks = numBlocksForStates +
            (numStates - ((numStates * SlackFactor) >> 7)); // less elements than states, accounting for slack
        numBlocks = checked_cast<u32>(SafeAllocatorSnapSize(allocator_(), numBlocks));
        const u32 capacity = checked_cast<u16>(numBlocks - numBlocksForStates);
        Assert_NoAssume(capacity <= numStates);
        AssertRelease(capacity <= UINT16_MAX); // trade-off limitation of this container

        _elements = allocator_traits::template AllocateT<_Key>(*this, numBlocks);

        state_t* const states = reinterpret_cast<state_t*>(_elements + capacity);

        forrange(i, 0, numStates) { // #TODO : faster
            state_t& st = states[i];
            st.Index = state_t::EmptyIndex;
            st.Hash = u16(i); // reset Hash to Index so DistanceIndex_() == 0
        }

        if (_size) {
            Assert(oldStates);
            Assert_NoAssume((state_t*)this != oldStates);

            // copy previous elements at the same indices
            IF_CONSTEXPR(Meta::has_trivial_move<_Key>::value) {
                FPlatformMemory::Memcpy(_elements, oldElts, _size * sizeof(_Key));
            }
            else {
                std::uninitialized_move(
                    MakeCheckedIterator(oldElts, _size, 0),
                    MakeCheckedIterator(oldElts, _size, _size),
                    MakeCheckedIterator(_elements, _size, 0) );
            }
            // rehash using previous state which already contains the hash keys
            const u32 numStatesM1 = (numStates - 1);
            forrange(p, oldStates, oldStates + oldNumStates) {
                if (state_t::EmptyIndex == p->Index)
                    continue;

                // use the same insertion index as before
                // don't need more entropy, hash table is bounded to 65536 entries
                state_t e{ p->Index, p->Hash };
                u32 s = (e.Hash & numStatesM1);
                u32 d = 0;
                for (;; s = (s + 1) & numStatesM1, ++d) {
                    state_t& it = states[s];
                    if (Likely(it.Index == state_t::EmptyIndex))
                        break;
                    Assert_NoAssume(e.Index != it.Index);

                    const u32 ds = DistanceIndex_(it, s, numStatesM1);
                    if (ds < d) {
                        d = ds;
                        e.swap(it);
                    }
                }

                states[s] = e;
            }
        }

        if (_capacity) {
            // release previous key & state vectors (guaranteed to don't take benefit of Rellocate() since we snapped to allocator sizes)
            allocator_traits::DeallocateT(*this, oldElts, NumAllocatedBlocks_());
        }

        _capacity = capacity;
        _sizeClass = sizeClass;

        Assert_NoAssume(NumAllocatedBlocks_() == numBlocks);
    }

    NO_INLINE void ReleaseMemory_() {
        Assert_NoAssume(_capacity);

        Destroy(allocator_(), MakeView());

        allocator_traits::DeallocateT(*this, _elements, NumAllocatedBlocks_());

        _size = 0;
        _sizeClass = 0;
        _capacity = 0;
        _elements = (pointer)this;
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
