#pragma once

#include "Allocator/Allocation.h"
#include "Memory/MemoryView.h"

#include <algorithm>
#include <bitset>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FDenseHashTableState {
    u16 Index;
    u16 Hash;
    STATIC_CONST_INTEGRAL(u16, EmptyIndex, u16(-1));
    STATIC_CONST_INTEGRAL(u16, TombIndex, u16(-2));
    STATIC_CONST_INTEGRAL(u16, EmptyMask, EmptyIndex&TombIndex);
};
STATIC_ASSERT(Meta::is_pod_v<FDenseHashTableState>);
template <
    typename _Key
    , typename _Hash = Meta::THash<_Key>
    , typename _EqualTo = Meta::TEqualTo<_Key>
    , typename _Allocator = ALLOCATOR(Container)
>   class TDenseHashSet_Legacy :_Allocator {
public:
    using allocator_type = _Allocator;
    using allocator_traits = TAllocatorTraits<_Allocator>;

    typedef _Key value_type;

    typedef _Hash hasher;
    typedef _EqualTo key_equal;

    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef Meta::TAddPointer<value_type> pointer;
    typedef Meta::TAddPointer<const value_type> const_pointer;

    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

    using iterator = TCheckedArrayIterator<_Key>;
    using const_iterator = TCheckedArrayIterator<const _Key>;

    TDenseHashSet_Legacy()
        : _size(0)
        , _capacity(0)
        , _elements(nullptr)
        , _states(nullptr)
    {}

    ~TDenseHashSet_Legacy() {
        clear_ReleaseMemory();
    }

    TDenseHashSet_Legacy(const TDenseHashSet_Legacy& other) : TDenseHashSet_Legacy() {
        operator =(other);
    }
    TDenseHashSet_Legacy& operator =(const TDenseHashSet_Legacy& other) {
        clear();
        if (other.size()) {
            reserve(other.size());
            for (const auto& it : other)
                insert(it);
        }
        return (*this);
    }

    TDenseHashSet_Legacy(TDenseHashSet_Legacy&& rvalue) : TDenseHashSet_Legacy() {
        std::swap(_size, rvalue._size);
        std::swap(_capacity, rvalue._capacity);
        std::swap(_elements, rvalue._elements);
        std::swap(_states, rvalue._states);
    }
    TDenseHashSet_Legacy& operator =(TDenseHashSet_Legacy&& rvalue) {
        clear_ReleaseMemory();
        std::swap(_size, rvalue._size);
        std::swap(_capacity, rvalue._capacity);
        std::swap(_elements, rvalue._elements);
        std::swap(_states, rvalue._states);
        return (*this);
    }

    bool empty() const { return (0 == _size); }
    size_t size() const { return _size; }
    size_t capacity() const { return _capacity; }

    iterator begin() { return MakeCheckedIterator(_elements, _size, 0); }
    iterator end() { return MakeCheckedIterator(_elements, _size, _size); }

    const_iterator begin() const { return MakeCheckedIterator(_elements, _size, 0); }
    const_iterator end() const { return MakeCheckedIterator(_elements, _size, _size); }

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
        Meta::Construct(_elements + index, key);

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
        Meta::Destroy(_elements + st.Index);

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

            Meta::Construct(_elements + st.Index, std::move(_elements[replace]));
            Meta::Destroy(_elements + replace);
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

            Meta::Destroy(TMemoryView<_Key>{ _elements, _size });

            _size = 0;
        }
    }

    void clear_ReleaseMemory() {
        if (_capacity) {
            const size_t numStates = NumStates_(_capacity);
            allocator_traits::DeallocateT(*this, _states, numStates);

            Meta::Destroy(TMemoryView<_Key>{ _elements, _size });

            allocator_traits::DeallocateT(*this, _elements, _capacity);

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

    void reserve_Additional(size_t num) { reserve(_size + num); }
    NO_INLINE void reserve(size_t n) {
        Assert(n);
        if (n <= _capacity)
            return;

        const u32 oldCapacity = _capacity;
        _capacity = checked_cast<u32>(allocator_traits::template SnapSizeT<_Key>(n));
        Assert_NoAssume(oldCapacity < _capacity);

        const u32 numStates = NumStates_(_capacity);
        Assert(numStates);

        if (oldCapacity)
            allocator_traits::DeallocateT(*this, _states, NumStates_(oldCapacity));

        _states = allocator_traits::template AllocateT<FDenseHashTableState>(*this, numStates).data();
        FPlatformMemory::Memset(_states, 0xFF, numStates * sizeof(*_states));

        if (oldCapacity) {
            TMemoryView<_Key> b{ _elements, _size };
            ReallocateAllocatorBlock(
                allocator_traits::Get(*this),
                b, oldCapacity, _capacity);

            _elements = b.data();

            const u32 numStatesM1 = (numStates - 1);
            forrange(i, 0, checked_cast<u16>(_size)) {
                const u32 h = u32(hasher()(_elements[i]));
                u32 s = (h & numStatesM1);
                for (;;) {
                    if ((_states[s].Index & FDenseHashTableState::EmptyMask) == FDenseHashTableState::EmptyMask)
                        break;

                    // linear probing
                    s = ++s & numStatesM1;
                }

                FDenseHashTableState& st = _states[s];
                st.Index = i;
                st.Hash = u16(h);
            }
        }
        else {
            Assert_NoAssume(0 == _size);
            Assert_NoAssume(nullptr == _elements);

            _elements = allocator_traits::template AllocateT<_Key>(*this, _capacity).data();
        }

        Assert_NoAssume(n <= _capacity);
    }

private:
    u32 _size;
    u32 _capacity;
    _Key* _elements;
    FDenseHashTableState* _states;

    STATIC_CONST_INTEGRAL(u32, KeyStateDensityRatio, 2);

    static u32 NumStates_(u32 capacity) {
        return (capacity ? FPlatformMaths::NextPow2(capacity * KeyStateDensityRatio) : 0);
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
