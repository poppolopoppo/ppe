#include "stdafx.h"

#include "Container/AssociativeVector.h"
#include "Container/BurstTrie.h"
#include "Container/FixedSizeHashSet.h"
#include "Container/FlatMap.h"
#include "Container/FlatSet.h"
#include "Container/HashTable.h"
#include "Container/StringHashSet.h"

#include "Diagnostic/Benchmark.h"
#include "Diagnostic/Logger.h"

#include "IO/BufferedStream.h"
#include "IO/FileStream.h"
#include "IO/Filename.h"
#include "IO/FormatHelpers.h"
#include "IO/String.h"
#include "IO/StringBuilder.h"
#include "IO/StringView.h"
#include "IO/TextWriter.h"
#include "VirtualFileSystem.h"
#include "HAL/PlatformMaths.h"
#include "Maths/Maths.h"
#include "Maths/PrimeNumbers.h"
#include "Maths/RandomGenerator.h"
#include "Memory/MemoryStream.h"
#include "Time/TimedScope.h"

#include <algorithm>
#include <bitset>
#include <random>
#include <unordered_set>

namespace PPE {
LOG_CATEGORY(, Test_Containers)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template class TAssociativeVector<FString, int>;
template class TFlatMap<FString, int>;
template class TFlatSet<FString>;
template class TBasicHashTable< details::THashMapTraits_<FString, int>, Meta::THash<FString>, Meta::TEqualTo<FString>, ALLOCATOR(Container, TPair<FString COMMA int>)>;
template class TBasicHashTable< details::THashSetTraits_<FString>, Meta::THash<FString>, Meta::TEqualTo<FString>, ALLOCATOR(Container, FString)>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

namespace PPE {
namespace Test {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, size_t _BulkSize>
struct TBulkTrieNode : public TBulkTrieNode<_Key, void , _BulkSize>{
    _Value Values[_BulkSize];

    TBulkTrieNode() {}
};
//----------------------------------------------------------------------------
template <typename _Key, size_t _BulkSize>
struct TBulkTrieNode<_Key, void, _BulkSize> {
    typedef uint32_t index_type;

    static constexpr index_type NoIndex = index_type(-1);

    _Key Keys[_BulkSize];
    std::bitset<_BulkSize> HasValue;
    index_type Left[_BulkSize];
    index_type Center[_BulkSize];
    index_type Right[_BulkSize];

    TBulkTrieNode() {
        std::fill(Left, Left+_BulkSize, NoIndex);
        std::fill(Center, Center+_BulkSize, NoIndex);
        std::fill(Right, Right+_BulkSize, NoIndex);
    }
};
//----------------------------------------------------------------------------
template <
    typename _Key
,   typename _Value
,   size_t _BulkSize = 4096
,   size_t _BucketCount = 11
,   typename _Less = Meta::TLess<_Key>
,   typename _Equal = Meta::TEqualTo<_Key>
,   typename _Allocator = ALLOCATOR(Container, TBulkTrieNode<_Key COMMA _Value COMMA _BulkSize>)
>   class FBulkTrie : _Allocator {
public:
    typedef _Key key_type;
    typedef _Value value_type;
    typedef _Less less_functor;
    typedef _Equal equal_to_functor;
    typedef _Allocator allocator_type;

    typedef std::allocator_traits<allocator_type> allocator_traits;
    typedef typename allocator_traits::difference_type difference_type;
    typedef typename allocator_traits::size_type size_type;

    static constexpr size_type BulkSize = _BulkSize;
    static constexpr size_type BucketCount = _BucketCount;

    typedef TMemoryView<const _Key> sequence_type;

    typedef TBulkTrieNode<_Key, _Value, _BulkSize> node_type;
    typedef typename node_type::index_type index_type;
    static constexpr index_type NoIndex = node_type::NoIndex;
    static constexpr index_type RootIndex = 0;
    typedef TVector<node_type*, typename _Allocator::template rebind<node_type*>::other> node_vector;

    struct Iterator {
        index_type FBucket;
        index_type Index;
    };

    FBulkTrie() : _wordCount(0) {}
    ~FBulkTrie() { Clear(); }

    FBulkTrie(const FBulkTrie&) = delete;
    FBulkTrie& operator=(const FBulkTrie&) = delete;

    size_type size() const { return _wordCount; }
    bool empty() const { return (0 == _wordCount); }

    bool Insert_ReturnIfExists(Iterator* it, const sequence_type& keys) {
        Assert(nullptr != it);
        Assert(false == keys.empty());

        it->Bucket = BucketIndex_(keys);
        FBucket& bucket = _buckets[it->Bucket];

        index_type parent = NoIndex;
        index_type current = RootIndex;

        const size_type n = keys.size();
        forrange(i, 0, n) {
            const _Key& key = keys[i];
            do {
                Assert(NoIndex != current);

                index_type rel;
                node_type* node = nullptr;
                if (current != bucket.Count) {
                    node = bucket.Node(&rel, current);

                    const index_type previous = current;

                    const _Key& other = node->Keys[rel];
                    if (equal_to_functor()(key, other)) {
                        parent = current;
                        if (NoIndex == (current = node->Center[rel]) && i + 1 != n)
                            current = node->Center[rel] = checked_cast<index_type>(bucket.Count);
                        break;
                    }
                    else if (less_functor()(other, key)) {
                        if (NoIndex == (current = node->Left[rel]))
                            current = node->Left[rel] = checked_cast<index_type>(bucket.Count);
                    }
                    else {
                        if (NoIndex == (current = node->Right[rel]))
                            current = node->Right[rel] = checked_cast<index_type>(bucket.Count);
                    }
                }
                else {
                    ++bucket.Count;

                    if (bucket.Nodes.size()*BulkSize <= current) {
                        rel = current%BulkSize;
                        node = allocator_traits::allocate(*this, 1);
                        Assert(node);
                        allocator_traits::construct(*this, node);
                        bucket.Nodes.push_back(node);
                    }
                    else {
                        node = bucket.Node(&rel, current);
                    }

                    node->Keys[rel] = key;

                    Assert(not node->HasValue.test(rel));
                    Assert(NoIndex == node->Left[rel]);
                    Assert(NoIndex == node->Center[rel]);
                    Assert(NoIndex == node->Right[rel]);

                    parent = current;

                    if (NoIndex == (current = node->Center[rel]) && i + 1 != n)
                        current = node->Center[rel] = checked_cast<index_type>(bucket.Count);

                    break;
                }
            }
            while (true);
        }

        Assert(NoIndex != parent);
        it->Index = parent;

        index_type rel;
        node_type* const node = bucket.Node(&rel, parent);
        Assert(keys.back() == node->Keys[rel]);

        if (node->HasValue.test(rel)) {
            return true;
        }
        else {
            _wordCount++;
            node->HasValue.set(rel, true);
            return false;
        }
    }

    Iterator Insert_AssertUnique(const sequence_type& keys) {
        Iterator result;
        if (Insert_ReturnIfExists(&result, keys))
            AssertNotReached();
        return result;
    }

    bool Find_ReturnIfExists(Iterator* it, const sequence_type& keys, Iterator hint = {NoIndex,NoIndex}) const {
        Assert(nullptr != it);
        Assert(false == keys.empty());

        if (0 == _wordCount)
            return false;

        it->Bucket = (NoIndex != hint.Bucket)
            ? hint.Bucket
            : BucketIndex_(keys);

        const FBucket& bucket = _buckets[it->Bucket];

        index_type parent = NoIndex;
        index_type current = (bucket.Count ? RootIndex : NoIndex);

        if (NoIndex != hint.Index) {
            index_type rel;
            const node_type* const node = bucket.Node(&rel, hint.Index);
            current = node->Center[rel];
        }

        const size_type n = keys.size();
        for (size_type i = 0; i < n; ++i) {
            const _Key key = keys[i]; // local cpy
            while (NoIndex != current) {
                index_type rel;
                node_type* const node = bucket.Node(&rel, current);

                parent = current;

                const _Key& other = node->Keys[rel];
                if (equal_to_functor()(key, other)) {
                    current = node->Center[rel];
                    break;
                }
                else if (less_functor()(other, key)) {
                    current = node->Left[rel];
                }
                else {
                    current = node->Right[rel];
                }
            }
        }

        it->Index = parent;
        return (NoIndex != parent);
    }

    bool Contains(const sequence_type& keys) const {
        Iterator it;
        if (false == Find_ReturnIfExists(&it, keys))
            return false;

        index_type rel;
        node_type* const node = _buckets[it.Bucket].Node(&rel, it.Index);

        return (false != node->HasValue.test(rel));
    }

    void Clear() {
        for (FBucket& bucket : _buckets) {
            for (node_type* node : bucket.Nodes) {
                allocator_traits::destroy(*this, node);
                allocator_traits::deallocate(*this, node, 1);
            }

            bucket.Count = 0;
            bucket.Nodes.clear_ReleaseMemory();
        }

        _wordCount = 0;
    }

private:
    static index_type BucketIndex_(const sequence_type& keys) {
        return ((keys.front()) % BucketCount);
    }

    struct FBucket {
        size_type Count = 0;
        node_vector Nodes;

        node_type* Node(index_type* relativeIdx, index_type absoluteIdx) const {
            Assert(NoIndex != absoluteIdx);
            Assert(Count > absoluteIdx);
            Assert(Nodes.size()*BulkSize > absoluteIdx);
            *relativeIdx = absoluteIdx%_BulkSize;
            node_type* const node = Nodes[absoluteIdx/_BulkSize];
            Assert(nullptr != node);
            return node;
        }
    };

    size_type _wordCount;

    FBucket _buckets[BucketCount];
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <
    typename _Key
,   typename _Hash = Meta::THash<_Key>
,   typename _EqualTo = Meta::TEqualTo<_Key>
,   typename _Allocator = ALLOCATOR(Container, _Key)
>   class TCompactHashSet : _Allocator {
public:
    typedef _Key value_type;

    typedef _Hash hasher;
    typedef _EqualTo key_equal;

    typedef _Allocator allocator_type;
    typedef std::allocator_traits<_Allocator> allocator_traits;

    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef Meta::TAddPointer<value_type> pointer;
    typedef Meta::TAddPointer<const value_type> const_pointer;

    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

    static constexpr size_type MaxLoadFactor = 50;

    template <typename T>
    struct TIterator {
        T* Ptr;
        size_t Capacity;
        size_t Index;

        TIterator(T* ptr, size_t capacity, size_t index)
            : Ptr(ptr)
            , Capacity(capacity)
            , Index(index) {
            Assert(Ptr);
            Assert(Index <= Capacity);
        }

        TIterator& operator++() { return Advance(); }
        TIterator& operator++(int) { TIterator tmp(*this); Advance(); return tmp; }

        T& operator *() const { Assert(Index < Capacity); return (Ptr[Index]); }
        T* operator ->() const { Assert(Index < Capacity); return (Ptr + Index); }

        inline friend bool operator ==(const TIterator& lhs, const TIterator& rhs) {
            return (lhs.Ptr == rhs.Ptr && lhs.Index == rhs.Index);
        }
        inline friend bool operator !=(const TIterator& lhs, const TIterator& rhs) {
            return (not operator ==(lhs, rhs));
        }

        TIterator& Advance(size_t amount = 1) {
            Assert(Index < Capacity);
            const value_type empty_key{};
            for (Index += amount; Index < Capacity && empty_key == Ptr[Index]; ++Index);
            return (*this);
        }
    };

    typedef TIterator<const value_type> const_iterator;

    TCompactHashSet() : _values(nullptr), _capacity(0), _size(0) {}
    ~TCompactHashSet() { clear(); }

    TCompactHashSet(const TCompactHashSet& other)
        : TCompactHashSet() {
        if (other.size()) {
            reserve(other.size());
            for (const auto& it : other)
                insert(it);
        }
    }

    TCompactHashSet& operator =(const TCompactHashSet& other) {
        clear();
        if (other.size()) {
            reserve(other.size());
            for (const auto& it : other)
                insert(it);
        }
        return (*this);
    }

    TCompactHashSet(TCompactHashSet&& rvalue)
        : TCompactHashSet() {
        std::swap(_values, rvalue._values);
        std::swap(_capacity, rvalue._capacity);
        std::swap(_size, rvalue._size);
    }

    TCompactHashSet& operator =(TCompactHashSet&& rvalue) {
        clear();
        std::swap(_values, rvalue._values);
        std::swap(_capacity, rvalue._capacity);
        std::swap(_size, rvalue._size);
        return (*this);
    }

    size_type size() const { return _size; }
    bool empty() const { return 0 == _size; }
    size_type capacity() const { return _capacity; }

    const_iterator begin() const { return const_iterator(_values, _capacity, 0).Advance(0); }
    const_iterator end() const { return const_iterator(_values, _capacity, _capacity); }

    void reserve(size_type atleast) {
        atleast = atleast + ((100-MaxLoadFactor)*atleast)/100;
        if (atleast > _capacity) {
            //Assert(atleast <= Primes_[31]);
            const size_type oldcapacity = _capacity;
            _capacity = FPlatformMaths::NextPow2(atleast);
            /*forrange(i, 0, 32)
                if (atleast <= Primes_[i]) {
                    _capacity = Primes_[i];
                    break;
                }*/
            Assert(_capacity >= atleast);
            if (oldcapacity) {
                Assert(_values);
                forrange(i, 0, oldcapacity)
                    allocator_traits::destroy(*this, _values+i);
                allocator_traits::deallocate(*this, _values, oldcapacity);
            }
            _values = allocator_traits::allocate(*this, _capacity);
            forrange(i, 0, _capacity)
                allocator_traits::construct(*this, _values+i);
        }
    }

    bool insert(const_reference value) {
        Assert(0 < _capacity);
        Assert(_size < _capacity);

        const size_t h = hasher()(value);
        /*
        size_type bucket = h % _capacity;
        size_type inc = 1 + (h>>16) % Min(5, _capacity - 1);
        */
        size_type bucket = size_type(h & (_capacity - 1));
        size_type inc = 1;// +((h >> 16) & Min(5ul, _capacity - 1));

        const value_type empty_key{};

        while (not key_equal()(_values[bucket], empty_key) &&
               not key_equal()(_values[bucket], value) )
            bucket = (bucket + inc < _capacity ? bucket + inc : (bucket + inc) - _capacity);

        if (key_equal()(_values[bucket], empty_key)) {
            _values[bucket] = value;
            _size++;
            return false;
        }
        else {
            return true;
        }
    }

    const_iterator find(const_reference value) const {
        Assert(0 < _capacity);
        Assert(_size < _capacity);
        const size_t h = hasher()(value);
        /*
        size_type bucket = h % _capacity;
        size_type inc = 1 + (h>>16) % Min(5, _capacity - 1);
        */
        size_type bucket = size_type(h & (_capacity - 1));
        size_type inc = 1;// +((h >> 16) & Min(5ul, _capacity - 1));

        const value_type empty_key{};

        while ( not key_equal()(_values[bucket], value) &&
                not key_equal()(_values[bucket], empty_key) )
            bucket = (bucket + inc < _capacity ? bucket + inc : (bucket + inc) - _capacity);

        return (not key_equal()(_values[bucket], empty_key) ? const_iterator(_values, _capacity, bucket) : end());
    }

    void erase(const_reference value) { // bubble down
        Assert(0 < _capacity);
        Assert(_size < _capacity);
        const size_t h = hasher()(value);
        /*
        size_type bucket = h % _capacity;
        size_type inc = 1 + (h>>16) % Min(5, _capacity - 1);
        */
        size_type bucket = size_type(h & (_capacity - 1));
        size_type inc = 1;// +((h >> 16) & Min(5ul, _capacity - 1));

        const value_type empty_key{};

        while ( not key_equal()(_values[bucket], value) &&
                not key_equal()(_values[bucket], empty_key) )
            bucket = (bucket + inc < _capacity ? bucket + inc : (bucket + inc) - _capacity);

        if (key_equal()(_values[bucket], empty_key))
            return;

        const size_type todelete = bucket;

        size_type chaintail;
        do {
            chaintail = bucket;
            bucket = (bucket + inc < _capacity ? bucket + inc : (bucket + inc) - _capacity);
        } while (not key_equal()(_values[bucket], empty_key));

        if (todelete != chaintail)
            _values[todelete] = _values[chaintail];

        _values[chaintail] = empty_key;

        _size--;
    }

    void clear() {
        Assert(_values);
        if (_capacity) {
            forrange(i, 0, _capacity)
                allocator_traits::destroy(*this, _values+i);
            allocator_traits::deallocate(*this, _values, _capacity);
            _values = nullptr;
            _capacity = _size = 0;
        }
    }

private:
    /*
    static constexpr size_type Primes_[32] = {
       0x00000000,0x00000003,0x0000000b,0x00000017,0x00000035,0x00000061,0x000000c1,0x00000185,
       0x00000301,0x00000607,0x00000c07,0x00001807,0x00003001,0x00006011,0x0000c005,0x0001800d,
       0x00030005,0x00060019,0x000c0001,0x00180005,0x0030000b,0x0060000d,0x00c00005,0x01800013,
       0x03000005,0x06000017,0x0c000013,0x18000005,0x30000059,0x60000005,0xc0000001,0xfffffffb,
    };
    */

    pointer _values;

    size_type _capacity;
    size_type _size;
};
//----------------------------------------------------------------------------
struct FDenseHashTableState {
    u16 Index;
    u16 Hash;
    STATIC_CONST_INTEGRAL(u16, EmptyIndex, u16(-1));
    STATIC_CONST_INTEGRAL(u16, TombIndex, u16(-2));
    STATIC_CONST_INTEGRAL(u16, EmptyMask, EmptyIndex&TombIndex);
};
STATIC_ASSERT(Meta::TIsPod_v<FDenseHashTableState>);
template <
    typename _Key
    , typename _Hash = Meta::THash<_Key>
    , typename _EqualTo = Meta::TEqualTo<_Key>
    , typename _Allocator = ALLOCATOR(Container, _Key)
>   class EMPTY_BASES TDenseHashSet
:   TRebindAlloc<_Allocator, FDenseHashTableState>
,   _Allocator {
public:
    using allocator_key = _Allocator;
    using allocator_state = TRebindAlloc<_Allocator, FDenseHashTableState>;

    using key_traits = std::allocator_traits<allocator_key>;
    using state_traits = std::allocator_traits<allocator_state>;

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

    TDenseHashSet()
        : _size(0)
        , _capacity(0)
        , _elements(nullptr)
        , _states(nullptr)
    {}

    ~TDenseHashSet() {
        clear_ReleaseMemory();
    }

    TDenseHashSet(const TDenseHashSet& other) : TDenseHashSet() {
        operator =(other);
    }
    TDenseHashSet& operator =(const TDenseHashSet& other) {
        clear();
        if (other.size()) {
            reserve(other.size());
            for (const auto& it : other)
                insert(it);
        }
        return (*this);
    }

    TDenseHashSet(TDenseHashSet&& rvalue) : TDenseHashSet() {
        std::swap(_size, rvalue._size);
        std::swap(_capacity, rvalue._capacity);
        std::swap(_elements, rvalue._elements);
        std::swap(_states, rvalue._states);
    }
    TDenseHashSet& operator =(TDenseHashSet&& rvalue) {
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

    void reserve_Additional(size_t num) { reserve(_size + num); }
    NO_INLINE void reserve(size_t n) {
        Assert(n);
        if (n <= _capacity)
            return;

        const u32 oldCapacity = _capacity;
        _capacity = checked_cast<u32>(SafeAllocatorSnapSize(key_alloc(), n));
        Assert_NoAssume(oldCapacity < _capacity);

        const u32 numStates = NumStates_(_capacity);
        Assert(numStates);

        if (oldCapacity)
            state_traits::deallocate(state_alloc(), _states, NumStates_(oldCapacity));

        _states = state_traits::allocate(state_alloc(), numStates);
        FPlatformMemory::Memset(_states, 0xFF, numStates * sizeof(*_states));

        if (oldCapacity) {
            _elements = Relocate(
                key_alloc(),
                TMemoryView<_Key>(_elements, _size),
                _capacity, oldCapacity);

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

            _elements = key_traits::allocate(key_alloc(), _capacity);
        }

        Assert_NoAssume(n <= _capacity);
    }

private:
    u32 _size;
    u32 _capacity;
    _Key* _elements;
    FDenseHashTableState* _states;

    STATIC_CONST_INTEGRAL(u32, KeyStateDensityRatio, 2);

    FORCE_INLINE allocator_key& key_alloc() { return static_cast<allocator_key&>(*this); }
    FORCE_INLINE allocator_state& state_alloc() { return static_cast<allocator_state&>(*this); }

    static u32 NumStates_(u32 capacity) {
        return (capacity ? FPlatformMaths::NextPow2(capacity * KeyStateDensityRatio) : 0);
    }
};
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
        if (other.size()) {
            reserve(other.size());
            for (const auto& it : other)
                insert(it);
        }
    }
    TDenseHashSet2& operator =(const TDenseHashSet2& other) {
        clear();
        if (other.size()) {
            reserve(other.size());
            for (const auto& it : other)
                insert(it);
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

    TPair<iterator, bool> insert(const _Key& key) {
        if (Unlikely(_size == Capacity_(_numStates)))
            reserve_Additional(1);

        Assert_NoAssume(_numStates);

        const u32 numStatesM1 = (_numStates - 1);
        u16 h = u16(hasher()(key));
        u32 s = (h & numStatesM1);
        u16 i = checked_cast<u16>(_size);
        for (u32 d = 0;; s = ++s & numStatesM1, d++) {
            state_t& it = _states[s];
            if (it.Index == state_t::EmptyIndex)
                break;
            else if (Unlikely(it.Hash == h && key_equal()(key, _elements[it.Index])))
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
        return MakePair(MakeCheckedIterator(_elements, _size, _size - 1), true);
    }

    const_iterator find(const _Key& key) const {
        const u32 numStatesM1 = (_numStates - 1);

        u16 i;
        u16 h = u16(hasher()(key));
        for (u32 s = (h & numStatesM1), d = 0;; s = (s + 1) & numStatesM1, ++d) {
            const state_t& it = _states[s];
            if (it.Hash == h && key_equal()(key, _elements[it.Index]))
                i = it.Index;
            else if (d > DistanceIndex_(it, s, numStatesM1))
                i = u16(_size);
            else
                continue;
            break;
        }

        return MakeCheckedIterator(_elements, _size, i);
    }

    bool erase(const _Key& key) {
        if (0 == _size)
            return false;

        Assert_NoAssume(Meta::IsPow2(_numStates));
        const u32 numStatesM1 = (_numStates - 1);

        u16 h = u16(hasher()(key));
        u32 s = (h & numStatesM1);
        for (u32 d = 0;; s = (s + 1) & numStatesM1, ++d) {
            const state_t& it = _states[s];
            if (it.Hash == h && key_equal()(key, _elements[it.Index]))
                break;
            else if (d > DistanceIndex_(it, s, numStatesM1))
                return false;
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

            IF_CONSTEXPR(not Meta::TIsPod_v<_Key>)
                forrange(it, _elements, _elements + _size)
                    key_traits::destroy(key_alloc(), it);

            _size = 0;
        }
    }

    void clear_ReleaseMemory() {
        if (_elements) {
            state_traits::deallocate(state_alloc(), _states, _numStates);

            forrange(it, _elements, _elements + _size)
                key_traits::destroy(key_alloc(), it);

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

    FORCE_INLINE void reserve_Additional(size_t num) { reserve(_size + num); }
    NO_INLINE void reserve(size_t n) {
        if (n <= Capacity_(_numStates))
            return;

        Assert(n);
        Assert_NoAssume(n > Capacity_(_numStates));

        const state_t* oldStates = _states;
        const u32 oldNumStates = _numStates;

        _numStates = FPlatformMaths::NextPow2(u32(n) * 2);
        Assert_NoAssume(oldNumStates < _numStates);

        const u32 capacity = Capacity_(_numStates);
        Assert(capacity);

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
                for (;;) {
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

                    // linear probing
                    s = ++s & numStatesM1;
                    d++;
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

    STATIC_CONST_INTEGRAL(u32, KeyStateDensityRatio, 2);

    FORCE_INLINE allocator_key& key_alloc() NOEXCEPT { return static_cast<allocator_key&>(*this); }
    FORCE_INLINE allocator_state& state_alloc() NOEXCEPT { return static_cast<allocator_state&>(*this); }

    FORCE_INLINE u32 Capacity_(u32 numStates) const NOEXCEPT {
        return (numStates > 1 ? checked_cast<u32>(SafeAllocatorSnapSize(
            static_cast<const allocator_key&>(*this), (numStates >> 1) + 1)) : 0);
    }

    NO_INLINE void eraseAt_(u32 s) {
        Assert_NoAssume(s < _numStates);

        const u32 numStatesM1 = (_numStates - 1);

        // destroy the element
        state_t& st = _states[s];

        if (_size > 1 && st.Index + 1u != _size) {
            // need to fill the hole in _elements
            const u16 ri = checked_cast<u16>(_size - 1);
            const u16 rh = u16(hasher()(_elements[ri]));

            u32 rs = (rh & numStatesM1);
            for (;;) {
                if (_states[rs].Index == ri)
                    break;
                rs = ++rs & numStatesM1;
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
template <typename _Set, typename T>
static void Test_Container_POD_(
    const FWStringView& name, _Set& set,
    const TMemoryView<const T>& input,
    const TMemoryView<const T>& negative,
    const TMemoryView<const T>& search,
    const TMemoryView<const T>& todelete,
    const TMemoryView<const T>& searchafterdelete
) {
    NOOP(name);

#ifdef WITH_PPE_ASSERT
    static constexpr size_t loops = 100;
#else
    static constexpr size_t loops = 10000;
#endif

    LOG(Test_Containers, Emphasis, L"benchmarking <{0}> with POD", name);
    BENCHMARK_SCOPE(name, L"global");

    {
        BENCHMARK_SCOPE(name, L"construction");
        for (const auto& word : input)
            set.insert(word);
    }
    Assert(set.size() == input.size());
    {
        BENCHMARK_SCOPE(name, L"iteration");
        forrange(i, 0, loops) {
            volatile size_t count = 0;
            for (const auto& it : set) {
                UNUSED(it);
                ++count;
            }
            if (set.size() != count)
                PPE_THROW_IT(std::exception("invalid set iteration"));
        }
    }
    {
        BENCHMARK_SCOPE(name, L"search");
        forrange(i, 0, loops) {
            volatile size_t count = 0;
            for (const auto& word : search) {
                if (set.end() == set.find(word))
                    AssertNotReached();
                else
                    ++count;
            }
            if (search.size() != count)
                PPE_THROW_IT(std::exception("invalid set search"));
        }
    }
    {
        BENCHMARK_SCOPE(name, L"negative search");
        forrange(i, 0, loops) {
            volatile size_t count = 0;
            for (const auto& word : negative) {
                if (set.end() != set.find(word))
                    AssertNotReached();
                else
                    ++count;
            }
            if (negative.size() != count)
                PPE_THROW_IT(std::exception("invalid set negative search"));
        }
    }
    {
        BENCHMARK_SCOPE(name, L"deletion");
        for (const auto& word : todelete)
            set.erase(word);
    }
    {
        BENCHMARK_SCOPE(name, L"iteration after delete");
        forrange(i, 0, loops) {
            volatile size_t count = 0;
            for (const auto& it : set) {
                UNUSED(it);
                ++count;
            }
            if (set.size() != count)
                PPE_THROW_IT(std::exception("invalid set iteration"));
        }
    }
    {
        BENCHMARK_SCOPE(name, L"search after delete");
        forrange(i, 0, loops) {
            volatile size_t pos_count = 0;
            volatile size_t neg_count = 0;
            for (const auto& word : searchafterdelete) {
                if (set.end() == set.find(word))
                    ++neg_count;
                else
                    ++pos_count;
            }
            if (searchafterdelete.size() != pos_count + neg_count)
                PPE_THROW_IT(std::exception("invalid set search after delete"));
        }
    }
}
//----------------------------------------------------------------------------
template <typename _Set, typename T, typename _Adaptor>
static void Test_Container_Obj_(
    const FWStringView& name,
    _Set& set,
    const TMemoryView<const T>& input,
    const TMemoryView<const T>& negative,
    const TMemoryView<const T>& search,
    const TMemoryView<const T>& todelete,
    const TMemoryView<const T>& searchafterdelete,
    const _Adaptor& adaptor
) {
    NOOP(name);

#ifdef WITH_PPE_ASSERT
    static constexpr size_t loops = 10;
#else
    static constexpr size_t loops = 10000;
#endif

    LOG(Test_Containers, Emphasis, L"benchmarking <{0}> with non-POD", name);

    BENCHMARK_SCOPE(name, L"global");

    {
        BENCHMARK_SCOPE(name, L"construction");
        for (const auto& word : input.Map(adaptor))
            set.insert(word);
    }
    Assert(set.size() == input.size());
    {
        BENCHMARK_SCOPE(name, L"search");
        forrange(i, 0, loops) {
            volatile size_t count = 0;
            const auto e = set.end();
            for (const auto& word : search.Map(adaptor))
                count += (e == set.find(word) ? 0 : 1);
            if (search.size() != count)
                PPE_THROW_IT(std::exception("invalid search"));
        }
    }
    {
        BENCHMARK_SCOPE(name, L"iteration");
        forrange(i, 0, loops) {
            volatile size_t count = 0;
            for (const auto& it : set) {
                UNUSED(it);
                ++count;
            }
            if (set.size() != count)
                PPE_THROW_IT(std::exception("invalid set iteration"));
        }
    }
    {
        BENCHMARK_SCOPE(name, L"negative search");
        forrange(i, 0, loops) {
            volatile size_t count = 0;
            const auto e = set.end();
            for (const auto& word : negative.Map(adaptor))
                count += (e == set.find(word) ? 1 : 0);
            if (negative.size() != count)
                PPE_THROW_IT(std::exception("invalid negative search"));
        }
    }
    {
        BENCHMARK_SCOPE(name, L"deletion");
        volatile size_t count = 0;
        for (const auto& word : todelete.Map(adaptor))
            count += (set.erase(word) ? 1 : 0);
        if (todelete.size() != count)
            PPE_THROW_IT(std::exception("invalid deletion"));
    }
    Assert(set.size() == input.size() - todelete.size());
    {
        BENCHMARK_SCOPE(name, L"iteration after delete");
        forrange(i, 0, loops) {
            volatile size_t count = 0;
            for (const auto& it : set) {
                UNUSED(it);
                ++count;
            }
            if (set.size() != count)
                PPE_THROW_IT(std::exception("invalid iteration after delete"));
        }
    }
    {
        BENCHMARK_SCOPE(name, L"search after delete");
        forrange(i, 0, loops) {
            volatile size_t count = 0;
            const auto e = set.end();
            for (const auto& word : searchafterdelete.Map(adaptor))
                count += (e == set.find(word) ? 1 : 0);
            if (set.size() > count)
                PPE_THROW_IT(std::exception("invalid search after delete"));
        }
    }
}
//----------------------------------------------------------------------------
template <typename _Set, typename T>
static void Test_Container_Obj_(
    const FWStringView& name, _Set& set,
    const TMemoryView<const T>& input,
    const TMemoryView<const T>& negative,
    const TMemoryView<const T>& search,
    const TMemoryView<const T>& todelete,
    const TMemoryView<const T>& searchafterdelete
) {
    constexpr auto identity = [](const T& v) -> const T& { return v; };
    return Test_Container_Obj_(name, set, input, negative, search, todelete, searchafterdelete, identity);
}
//----------------------------------------------------------------------------
#if USE_PPE_BENCHMARK
template <typename T>
struct TInputData {
    TMemoryView<const T> Insert;
    TMemoryView<const T> Unkown;
    TMemoryView<const T> Search;
    TMemoryView<const T> Erase;
    TMemoryView<const T> Dense;
    TMemoryView<const T> Sparse;

    template <typename _Container>
    void FillDense(_Container& c) const {
        c.reserve(Insert.size());
        for (const auto& it : Insert)
            c.insert(it);
    }

    template <typename _Container>
    void FillSparse(_Container& c) const {
        c.reserve(Insert.size());
        for (const auto& it : Insert)
            c.insert(it);
        for (const auto& it : Sparse)
            c.erase(it);
    }

};

class construct_noreserve_t : public FBenchmark {
public:
    construct_noreserve_t() : FBenchmark{ "ctor_cold" } {}
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TInputData<T>& input) const {
        for (auto _ : state) {
            auto c{ archetype };
            for (const auto& it : input.Insert)
                c.insert(it);
            FBenchmark::DoNotOptimize(c);
        }
    }
};
class construct_reserve_t : public FBenchmark {
public:
    construct_reserve_t() : FBenchmark{ "ctor_warm" } {}
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TInputData<T>& input) const {
        for (auto _ : state) {
            auto c{ archetype };
            c.reserve(input.Insert.size());
            for (const auto& it : input.Insert)
                c.insert(it);
            FBenchmark::DoNotOptimize(c);
        }
    }
};
class copy_empty_t : public FBenchmark {
public:
    copy_empty_t() : FBenchmark{ "copy_empty" } {}
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TInputData<T>& input) const {
        auto s{ archetype };
        input.FillDense(s);

        auto c{ archetype };

        for (auto _ : state) {
            state.PauseTiming();
            c.clear();
            state.ResumeTiming();
            c = s;
            FBenchmark::DoNotOptimize(c);
        }
    }
};
class copy_dense_t : public FBenchmark {
public:
    copy_dense_t() : FBenchmark{ "copy_dense" } {}
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TInputData<T>& input) const {
        auto s{ archetype };
        input.FillDense(s);

        auto c{ s };

        for (auto _ : state) {
            c = s;
            FBenchmark::DoNotOptimize(c);
        }
    }
};
class copy_sparse_t : public FBenchmark {
public:
    copy_sparse_t() : FBenchmark{ "copy_sparse" } {}
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TInputData<T>& input) const {
        auto s{ archetype };
        input.FillSparse(s);

        auto c{ s };

        for (auto _ : state) {
            c = s;
            FBenchmark::DoNotOptimize(c);
        }
    }
};
class iterate_dense_t : public FBenchmark {
public:
    iterate_dense_t() : FBenchmark{ "iter_dense" } {}
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TInputData<T>& input) const {
        auto c{ archetype };
        input.FillDense(c);

        for (auto _ : state) {
            for (const auto& it : c)
                FBenchmark::DoNotOptimizeLoop(it);
            FBenchmark::ClobberMemory();
        }
    }
};
class iterate_sparse_t : public FBenchmark {
public:
    iterate_sparse_t() : FBenchmark{ "iter_sparse" } {}
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TInputData<T>& input) const {
        auto c{ archetype };
        input.FillSparse(c);

        for (auto _ : state) {
            for (const auto& it : c)
                FBenchmark::DoNotOptimizeLoop(it);
            FBenchmark::ClobberMemory();
        }
    }
};
class find_dense_pos_t : public FBenchmark {
public:
    find_dense_pos_t() : FBenchmark{ "find_dense_pos" } {}
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TInputData<T>& input) const {
        auto c{ archetype };
        input.FillDense(c);

        for (auto _ : state) {
            for (const auto& it : input.Search)
                FBenchmark::DoNotOptimizeLoop(c.find(it));
            FBenchmark::ClobberMemory();
        }
    }
};
class find_dense_neg_t : public FBenchmark {
public:
    find_dense_neg_t() : FBenchmark{ "find_dense_neg" } {}
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TInputData<T>& input) const {
        auto c{ archetype };
        input.FillSparse(c);

        for (auto _ : state) {
            for (const auto& it : input.Unkown)
                FBenchmark::DoNotOptimizeLoop(c.find(it));
            FBenchmark::ClobberMemory();
        }
    }
};
class find_sparse_pos_t : public FBenchmark {
public:
    find_sparse_pos_t() : FBenchmark{ "find_sparse_pos" } {}
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TInputData<T>& input) const {
        auto c{ archetype };
        input.FillSparse(c);

        for (auto _ : state) {
            for (const auto& it : input.Dense)
                FBenchmark::DoNotOptimizeLoop(c.find(it));
            FBenchmark::ClobberMemory();
        }
    }
};
class find_sparse_neg_t : public FBenchmark {
public:
    find_sparse_neg_t() : FBenchmark{ "find_sparse_neg" } {}
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TInputData<T>& input) const {
        auto c{ archetype };
        input.FillSparse(c);

        for (auto _ : state) {
            for (const auto& it : input.Unkown)
                FBenchmark::DoNotOptimizeLoop(c.find(it));
            FBenchmark::ClobberMemory();
        }
    }
};
class erase_dense_pos_t : public FBenchmark {
public:
    erase_dense_pos_t() : FBenchmark{ "erase_dense_pos" } {}
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TInputData<T>& input) const {
        auto s{ archetype };
        input.FillDense(s);

        auto c{ archetype };
        for (auto _ : state) {
            state.PauseTiming();
            c = s;
            state.ResumeTiming();
            for (const auto& it : input.Search)
                FBenchmark::DoNotOptimizeLoop(c.erase(it));
            FBenchmark::ClobberMemory();
        }
    }
};
class erase_dense_neg_t : public FBenchmark {
public:
    erase_dense_neg_t() : FBenchmark{ "erase_dense_neg" } {}
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TInputData<T>& input) const {
        auto s{ archetype };
        input.FillDense(s);

        auto c{ archetype };
        for (auto _ : state) {
            state.PauseTiming();
            c = s;
            state.ResumeTiming();
            for (const auto& it : input.Unkown)
                FBenchmark::DoNotOptimizeLoop(c.erase(it));
            FBenchmark::ClobberMemory();
        }
    }
};
class erase_sparse_pos_t : public FBenchmark {
public:
    erase_sparse_pos_t() : FBenchmark{ "erase_sparse_pos" } {}
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TInputData<T>& input) const {
        auto s{ archetype };
        input.FillDense(s);

        auto c{ archetype };
        for (auto _ : state) {
            state.PauseTiming();
            c = s;
            state.ResumeTiming();
            for (const auto& it : input.Dense)
                FBenchmark::DoNotOptimizeLoop(c.erase(it));
            FBenchmark::ClobberMemory();
        }
    }
};
class erase_sparse_neg_t : public FBenchmark {
public:
    erase_sparse_neg_t() : FBenchmark{ "erase_sparse_neg" } {}
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TInputData<T>& input) const {
        auto s{ archetype };
        input.FillDense(s);

        auto c{ archetype };
        for (auto _ : state) {
            state.PauseTiming();
            c = s;
            state.ResumeTiming();
            for (const auto& it : input.Unkown)
                FBenchmark::DoNotOptimizeLoop(c.erase(it));
            FBenchmark::ClobberMemory();
        }
    }
};
class clear_dense_t : public FBenchmark {
public:
    clear_dense_t() : FBenchmark{ "clear_dense" } {}
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TInputData<T>& input) const {
        auto s{ archetype };
        input.FillDense(s);

        auto c{ archetype };
        for (auto _ : state) {
            state.PauseTiming();
            c = s;
            state.ResumeTiming();
            c.clear();
            FBenchmark::DoNotOptimize(c);
        }
    }
};
class clear_sparse_t : public FBenchmark {
public:
    clear_sparse_t() : FBenchmark{ "clear_sparse" } {}
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TInputData<T>& input) const {
        auto s{ archetype };
        input.FillSparse(s);

        auto c{ archetype };
        for (auto _ : state) {
            state.PauseTiming();
            c = s;
            state.ResumeTiming();
            c.clear();
            FBenchmark::DoNotOptimize(c);
        }
    }
};

template <typename T, typename _Generator, typename _Containers>
static void Benchmark_Containers_(
    const FStringView& name, size_t dim,
    _Generator generator,
    _Containers tests ) {
    // prepare input data

    std::random_device rdevice;
    std::mt19937 rand{ rdevice() };

    VECTOR(Benchmark, T) samples;
    samples.reserve_Additional(dim * 2);
    forrange(i, 0, dim * 2)
        samples.emplace_back(generator(rand));

    std::shuffle(samples.begin(), samples.end(), rand);

    const TMemoryView<const T> insert = samples.MakeConstView().CutBefore(dim);
    const TMemoryView<const T> unkown = samples.MakeConstView().CutStartingAt(dim);

    constexpr size_t sparse_factor = 70;
    VECTOR(Benchmark, T) search { insert };
    VECTOR(Benchmark, T) erase { search };
    VECTOR(Benchmark, T) sparse {
        erase
            .MakeConstView()
            .CutBeforeConst((sparse_factor * erase.size()) / 100)
    };
    VECTOR(Benchmark, T) dense {
        erase
            .MakeConstView()
            .CutStartingAt(sparse.size())
    };

    std::shuffle(std::begin(erase), std::end(erase), rand);
    std::shuffle(std::begin(search), std::end(search), rand);
    std::shuffle(std::begin(dense), std::end(dense), rand);
    std::shuffle(std::begin(sparse), std::end(sparse), rand);

    using FInputData = TInputData<T>;
    FInputData input{ insert, search, unkown, erase, dense, sparse };
    Assert_NoAssume(insert.size());
    Assert_NoAssume(search.size());
    Assert_NoAssume(unkown.size());
    Assert_NoAssume(erase.size());
    Assert_NoAssume(dense.size());
    Assert_NoAssume(sparse.size());

    // prepare benchmark table

    auto bm = FBenchmark::MakeTable(
        name,
        construct_noreserve_t{},
        construct_reserve_t{},
        copy_empty_t{},
        copy_dense_t{},
        copy_sparse_t{},
        iterate_dense_t{},
        iterate_sparse_t{},
        find_dense_pos_t{},
        find_dense_neg_t{},
        find_sparse_pos_t{},
        find_sparse_neg_t{},
        erase_dense_pos_t{},
        erase_dense_neg_t{},
        erase_sparse_pos_t{},
        erase_sparse_neg_t{},
        clear_dense_t{},
        clear_sparse_t{} );

    tests(bm, input);

    FBenchmark::Log(bm);

    {
        const FFilename fname{ StringFormat(L"Saved:/Benchmark/Containers/{0}.csv", name) };
        FStringBuilder sb;
        FBenchmark::Csv(bm, sb);
        FString s{ sb.ToString() };
        VFS_WriteAll(fname, s.MakeView().RawView(), EAccessPolicy::Truncate_Binary);
    }
}
//----------------------------------------------------------------------------
static void Test_StringSet_() {
    auto generator = [](auto& rnd) {
        constexpr char Charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!-*.$^@#~";
        constexpr size_t MinSize = 5;
        constexpr size_t MaxSize = 60;

        const size_t n = (rnd() % (MaxSize - MinSize + 1)) + MinSize;

        FString s;
        s.reserve(n);

        forrange(i, 0, n)
            s.append(Charset[rnd() % (lengthof(Charset) - 1)]);

        return s;
    };

    auto containers = [](auto& bm, const auto& input) {
        {
            typedef TDenseHashSet<
                FStringView,
                TStringViewHasher<char, ECase::Sensitive>,
                TStringViewEqualTo<char, ECase::Sensitive>
            >   hashtable_type;

            hashtable_type set;
            bm.Run("DenseHashSet", set, input);
        }/*
        {
            typedef TDenseHashSet<
                THashMemoizer<
                FStringView,
                TStringViewHasher<char, ECase::Sensitive>,
                TStringViewEqualTo<char, ECase::Sensitive>
                >
            >   hashtable_type;

            hashtable_type set;
            bm.Run("DenseHashSet_M", set, input);
        }*/
        {
            typedef TDenseHashSet2<
                FStringView,
                TStringViewHasher<char, ECase::Sensitive>,
                TStringViewEqualTo<char, ECase::Sensitive>
            >   hashtable_type;

            hashtable_type set;
            bm.Run("DenseHashSet2", set, input);
        }/*
        {
            typedef TDenseHashSet2<
                THashMemoizer<
                FStringView,
                TStringViewHasher<char, ECase::Sensitive>,
                TStringViewEqualTo<char, ECase::Sensitive>
                >
            >   hashtable_type;

            hashtable_type set;
            bm.Run("DenseHashSet2_M", set, input);
        }*/
        {
            STRINGVIEW_HASHSET(Container, ECase::Sensitive) set;

            bm.Run("THashSet", set, input);
        }/*
        {
            STRINGVIEW_HASHSET_MEMOIZE(Container, ECase::Sensitive) set;

            bm.Run("THashSet_M", set, input);
        }*//*
        {
            CONSTCHAR_HASHSET_MEMOIZE(Container, ECase::Sensitive) set;

            bm.Run("TConstCharHashSet_M", set, input);
        }*/
        {
            std::unordered_set<
                FStringView,
                TStringViewHasher<char, ECase::Sensitive>,
                TStringViewEqualTo<char, ECase::Sensitive>,
                ALLOCATOR(Container, FStringView)
            >   set;

            bm.Run("unordered_set", set, input);
        }/*
        {
            std::unordered_set<
                TBasicStringViewHashMemoizer<char, ECase::Sensitive>,
                Meta::THash< TBasicStringViewHashMemoizer<char, ECase::Sensitive> >
            >   set;

            bm.Run("unordered_set_M", set, input);
        }*/
    };

    Benchmark_Containers_<FString>("Strings20", 20, generator, containers);
    Benchmark_Containers_<FString>("Strings50", 50, generator, containers);
    Benchmark_Containers_<FString>("Strings200", 200, generator, containers);
    Benchmark_Containers_<FString>("Strings2000", 2000, generator, containers);
}
#endif //!USE_PPE_BENCHMARK
//----------------------------------------------------------------------------
static void Test_StealFromDifferentAllocator_() {
    // steal allocations from different tracking domains
    {
        VECTOR(NativeTypes, int) u = { 1, 2, 3 };
        VECTOR(Container, int) v = u;
        VECTOR(Container, int) w = std::move(u);
    }
    {
        HASHSET(NativeTypes, int) u = { 1, 2, 3 };
        HASHSET(Container, int) w = std::move(u);
    }
}
//----------------------------------------------------------------------------
void Test_Containers() {
    LOG(Test_Containers, Emphasis, L"starting container tests ...");

    Test_StealFromDifferentAllocator_();
#if USE_PPE_BENCHMARK
    Test_StringSet_();
#endif
#if 0

    {
        LOG(Test_Containers, Info, L"{0}", Fmt::Repeat(MakeStringView(L">>="), 20));
        LOG(Test_Containers, Emphasis, L"FStringView collection");

        const FFilename filename = L"Saved:/dico.txt";

        static constexpr u32 GMaxWords = 16000;// u32(-1); // no limit

        VECTOR(Container, FString) words;
        {
            const UStreamReader reader = VFS_OpenBinaryReadable(filename);
            if (not reader)
                AssertNotReached();

            UsingBufferedStream(reader.get(), [&words](IBufferedStreamReader* iss) {
                char buffer[512];
                while (true) {
                    const FStringView line = iss->ReadLine(buffer);
                    if (line.empty())
                        break;

                    const FStringView word = Chomp(line);
                    words.push_back_Default().assign(word);

                    if (words.size() == GMaxWords)
                        break;
                }
            });
        }

        //words.resize((UINT16_MAX*80)/100);
        //words.resize(8);

        std::random_device rdevice;
        std::mt19937 rand(rdevice());

        VECTOR(Container, FStringView) all;
        all.reserve(words.size());
        for (const FString& word : words)
            all.emplace_back(MakeStringView(word));
        std::shuffle(all.begin(), all.end(), rand);

        const size_t k = (all.size() * 80) / 100;

        const auto input = all.MakeConstView().CutBefore(k);
        const auto negative = all.MakeConstView().CutStartingAt(k);

        VECTOR(Container, FStringView) search(input);
        std::shuffle(search.begin(), search.end(), rand);

        VECTOR(Container, FStringView) todelete(search);
        std::shuffle(todelete.begin(), todelete.end(), rand);
        todelete.resize(k / 2);

        VECTOR(Container, FStringView) searchafterdelete(input);
        std::shuffle(searchafterdelete.begin(), searchafterdelete.end(), rand);

        const hash_t h0 = hash_value(words.front());
        const hash_t h1 = hash_string(words.front().MakeView());
        const hash_t h2 = TConstCharHasher<char, ECase::Sensitive>()(FConstChar(words.front().MakeView()));
        AssertRelease(h0 == h1);
        AssertRelease(h1 == h2);
        /*{
            typedef TCompactHashSet<
                FStringView,
                TStringViewHasher<char, ECase::Sensitive>,
                TStringViewEqualTo<char, ECase::Sensitive>
            >   hashtable_type;

            hashtable_type set;
            set.resize(input.size());

            Test_Container_Obj_(L"TCompactHashSet", set, input, negative, search.MakeConstView(), todelete.MakeConstView(), searchafterdelete.MakeConstView());
        }
        {
            typedef TCompactHashSet<
                THashMemoizer<
                    FStringView,
                    TStringViewHasher<char, ECase::Sensitive>,
                    TStringViewEqualTo<char, ECase::Sensitive>
                >
            >   hashtable_type;

            hashtable_type set;
            set.resize(input.size());

            Test_Container_Obj_(L"TCompactHashSet Memoize", set, input, negative, search.MakeConstView(), todelete.MakeConstView(), searchafterdelete.MakeConstView());
        }*/
        {
            typedef TDenseHashSet<
                FStringView,
                TStringViewHasher<char, ECase::Sensitive>,
                TStringViewEqualTo<char, ECase::Sensitive>
            >   hashtable_type;

            hashtable_type set;
            set.reserve(input.size());

            Test_Container_Obj_(L"TDenseHashSet", set, input, negative, search.MakeConstView(), todelete.MakeConstView(), searchafterdelete.MakeConstView());
        }
        {
            typedef TDenseHashSet<
                THashMemoizer<
                    FStringView,
                    TStringViewHasher<char, ECase::Sensitive>,
                    TStringViewEqualTo<char, ECase::Sensitive>
                >
            >   hashtable_type;

            hashtable_type set;
            set.reserve(input.size());

            Test_Container_Obj_(L"TDenseHashSet Memoize", set, input, negative, search.MakeConstView(), todelete.MakeConstView(), searchafterdelete.MakeConstView());
        }
        {
            typedef TDenseHashSet2<
                FStringView,
                TStringViewHasher<char, ECase::Sensitive>,
                TStringViewEqualTo<char, ECase::Sensitive>
            >   hashtable_type;

            hashtable_type set;
            set.reserve(input.size());

            Test_Container_Obj_(L"TDenseHashSet2", set, input, negative, search.MakeConstView(), todelete.MakeConstView(), searchafterdelete.MakeConstView());
        }
        {
            typedef TDenseHashSet2<
                THashMemoizer<
                FStringView,
                TStringViewHasher<char, ECase::Sensitive>,
                TStringViewEqualTo<char, ECase::Sensitive>
                >
            >   hashtable_type;

            hashtable_type set;
            set.reserve(input.size());

            Test_Container_Obj_(L"TDenseHashSet2 Memoize", set, input, negative, search.MakeConstView(), todelete.MakeConstView(), searchafterdelete.MakeConstView());
        }
        {
            STRINGVIEW_HASHSET(Container, ECase::Sensitive) set;

            Test_Container_Obj_(L"THashSet", set, input, negative, search.MakeConstView(), todelete.MakeConstView(), searchafterdelete.MakeConstView());

            LOG(Test_Containers, Info, L"THashSet load_factor = {0:#f2}% max probe dist = {1}", set.load_factor(), set.max_probe_dist());
        }
        {
            STRINGVIEW_HASHSET_MEMOIZE(Container, ECase::Sensitive) set;

            Test_Container_Obj_(L"THashSet Memoize", set, input, negative, search.MakeConstView(), todelete.MakeConstView(), searchafterdelete.MakeConstView());

            LOG(Test_Containers, Info, L"THashSet load_factor = {0:#f2}% max probe dist = {1}", set.load_factor(), set.max_probe_dist());
        }
        {
            CONSTCHAR_HASHSET_MEMOIZE(Container, ECase::Sensitive) set;

            Test_Container_Obj_(L"TConstCharHashSet Memoize", set, input, negative, search.MakeConstView(), todelete.MakeConstView(), searchafterdelete.MakeConstView(),
                [](const FStringView& str) -> FConstChar {
                    return FConstChar(str);
                });

            LOG(Test_Containers, Info, L"TConstCharHashSet load_factor = {0:#f2}% max probe dist = {1}", set.load_factor(), set.max_probe_dist());
        }
        {
            std::unordered_set<
                FStringView,
                TStringViewHasher<char, ECase::Sensitive>,
                TStringViewEqualTo<char, ECase::Sensitive>
            >   set;

            Test_Container_Obj_(L"std::unordered_set", set, input, negative, search.MakeConstView(), todelete.MakeConstView(), searchafterdelete.MakeConstView());
        }
        {
            std::unordered_set<
                TBasicStringViewHashMemoizer<char, ECase::Sensitive>,
                Meta::THash< TBasicStringViewHashMemoizer<char, ECase::Sensitive> >
            >   set;

            Test_Container_Obj_(L"std::unordered_set Memoize", set, input, negative, search.MakeConstView(), todelete.MakeConstView(), searchafterdelete.MakeConstView());
        }
        /*{
            TFlatSet<
                FStringView,
                TStringViewEqualTo<char, ECase::Sensitive>,
                TStringViewLess<char, ECase::Sensitive>
            >   set;

            Test_Container_Obj_(L"TFlatSet", set, input, negative, search.MakeConstView(), todelete.MakeConstView(), searchafterdelete.MakeConstView());
        }
        {
            STRINGTRIE_SET(Container, ECase::Sensitive, 31) set;

            Test_Container_Obj_(L"TBurstTrie", set, input, negative, search.MakeConstView(), todelete.MakeConstView(), searchafterdelete.MakeConstView());
        }
        {
            FBulkTrie<char, void, 8192, 31> set;

            Test_Container_Obj_(L"FBulkTrie", set, input, negative, search.MakeConstView(), todelete.MakeConstView(), searchafterdelete.MakeConstView());
        }*/
    }
    FLUSH_LOG();
    {
        const size_t COUNT = 2000;

        LOG(Test_Containers, Info, L"{0}", Fmt::Repeat(MakeStringView(L">>="), 20));
        LOG(Test_Containers, Emphasis, L"Medium double collection ({0})", COUNT);

        typedef double value_type;

        std::random_device rdevice;
        std::mt19937 rand(rdevice());

        VECTOR(Container, value_type) all;
        all.reserve(COUNT);
        forrange(i, 1, COUNT+1)
            all.push_back(value_type(i / value_type(COUNT)) + rand() * COUNT);

        std::shuffle(all.begin(), all.end(), rand);

        const size_t k = (all.size() * 80) / 100;

        const auto input = all.MakeConstView().CutBefore(k);
        const auto negative = all.MakeConstView().CutStartingAt(k);

        VECTOR(Container, value_type) search(input);
        std::shuffle(search.begin(), search.end(), rand);

        VECTOR(Container, value_type) todelete(search);
        std::shuffle(todelete.begin(), todelete.end(), rand);
        todelete.resize(k / 2);

        VECTOR(Container, value_type) searchafterdelete(input);
        std::shuffle(searchafterdelete.begin(), searchafterdelete.end(), rand);

        {
            typedef TCompactHashSet<value_type>   hashtable_type;

            hashtable_type set;
            set.resize(input.size());

            Test_Container_POD_(L"TCompactHashSet", set, input, negative, search.MakeConstView(), todelete.MakeConstView(), searchafterdelete.MakeConstView());
        }
        {
            typedef TDenseHashSet<value_type>   hashtable_type;

            hashtable_type set;
            set.reserve(input.size());

            Test_Container_POD_(L"TDenseHashSet", set, input, negative, search.MakeConstView(), todelete.MakeConstView(), searchafterdelete.MakeConstView());
        }
        {
            typedef TDenseHashSet2<value_type>   hashtable_type;

            hashtable_type set;
            set.reserve(input.size());

            Test_Container_POD_(L"TDenseHashSet2", set, input, negative, search.MakeConstView(), todelete.MakeConstView(), searchafterdelete.MakeConstView());
        }
        {
            THashSet<value_type> set;

            Test_Container_POD_(L"THashSet", set, input, negative, search.MakeConstView(), todelete.MakeConstView(), searchafterdelete.MakeConstView());

            LOG(Test_Containers, Info, L"THashSet load_factor = {0:#f2}% max probe dist = {1}", set.load_factor(), set.max_probe_dist());
        }
        {
            std::unordered_set<value_type, Meta::THash<value_type>> set;

            Test_Container_POD_(L"std::unordered_set", set, input, negative, search.MakeConstView(), todelete.MakeConstView(), searchafterdelete.MakeConstView());
        }
        {
            TFlatSet<value_type> set;

            Test_Container_POD_(L"TFlatSet", set, input, negative, search.MakeConstView(), todelete.MakeConstView(), searchafterdelete.MakeConstView());
        }

    }
    FLUSH_LOG();
    {
        const size_t COUNT = 28;

        LOG(Test_Containers, Info, L"{0}", Fmt::Repeat(MakeStringView(L">>="), 20));
        LOG(Test_Containers, Emphasis, L"Small integer collection ({0})", COUNT);

        typedef int value_type;

        std::random_device rdevice;
        std::mt19937 rand(rdevice());

        VECTOR(Container, value_type) all;
        all.reserve(COUNT);
        forrange(i, 1, COUNT + 1)
            all.push_back(value_type(i + Min(rand(), COUNT) * COUNT));

        std::shuffle(all.begin(), all.end(), rand);

        const size_t k = (all.size() * 80) / 100;

        const auto input = all.MakeConstView().CutBefore(k);
        const auto negative = all.MakeConstView().CutStartingAt(k);

        VECTOR(Container, value_type) search(input);
        std::shuffle(search.begin(), search.end(), rand);

        VECTOR(Container, value_type) todelete(search);
        std::shuffle(todelete.begin(), todelete.end(), rand);
        todelete.resize(k / 2);

        VECTOR(Container, value_type) searchafterdelete(input);
        std::shuffle(searchafterdelete.begin(), searchafterdelete.end(), rand);

        {
            typedef TVectorInSitu<value_type, COUNT> vector_type;

            vector_type v;

            struct FAdapter_ {
                vector_type& v;
                size_t size() const { return v.size(); }
                auto begin() const { return v.begin(); }
                auto end() const { return v.end(); }
                void insert(value_type i) { v.push_back(i); }
                auto find(value_type i) const { return std::find(v.begin(), v.end(), i); }
                bool erase(value_type i) {
                    auto it = find(i);
                    if (v.end() == it)
                        return false;
                    v.erase_DontPreserveOrder(it);
                    return true;
                }
                void clear() { v.clear(); }
            };

            FAdapter_ set{ v };

            Test_Container_POD_(L"TVectorInSitu", set, input, negative, search.MakeConstView(), todelete.MakeConstView(), searchafterdelete.MakeConstView());
        }
        {
            typedef TFixedSizeHashSet<value_type, COUNT> set_type;

            set_type s;

            struct FAdapter_ {
                set_type& s;
                size_t size() const { return s.size(); }
                auto begin() const { return s.begin(); }
                auto end() const { return s.end(); }
                void insert(value_type i) { s.Add_AssertUnique(i); }
                auto find(value_type i) const { return s.Find(i); }
                bool erase(value_type i) { return s.Remove_ReturnIfExists(i); }
                void clear() { s.Clear(); }
            };

            FAdapter_ set{ s };

            Test_Container_POD_(L"TFixedSizeHashSet", set, input, negative, search.MakeConstView(), todelete.MakeConstView(), searchafterdelete.MakeConstView());
        }
        {
            typedef TFixedSizeHashSet<value_type, ROUND_TO_NEXT_16(COUNT*2)> set_type;

            set_type s;

            struct FAdapter_ {
                set_type& s;
                size_t size() const { return s.size(); }
                auto begin() const { return s.begin(); }
                auto end() const { return s.end(); }
                void insert(value_type i) { s.Add_AssertUnique(i); }
                auto find(value_type i) const { return s.Find(i); }
                bool erase(value_type i) { return s.Remove_ReturnIfExists(i); }
                void clear() { s.Clear(); }
            };

            FAdapter_ set{ s };

            Test_Container_POD_(L"TFixedSizeHashSet2", set, input, negative, search.MakeConstView(), todelete.MakeConstView(), searchafterdelete.MakeConstView());
        }
        {
            typedef TCompactHashSet<value_type>   hashtable_type;

            hashtable_type set;
            set.resize(input.size());

            Test_Container_POD_(L"TCompactHashSet", set, input, negative, search.MakeConstView(), todelete.MakeConstView(), searchafterdelete.MakeConstView());
        }
        {
            typedef TDenseHashSet<value_type>   hashtable_type;

            hashtable_type set;
            set.reserve(input.size());

            Test_Container_POD_(L"TDenseHashSet", set, input, negative, search.MakeConstView(), todelete.MakeConstView(), searchafterdelete.MakeConstView());
        }
        {
            typedef TDenseHashSet2<value_type>   hashtable_type;

            hashtable_type set;
            set.reserve(input.size());

            Test_Container_POD_(L"TDenseHashSet2", set, input, negative, search.MakeConstView(), todelete.MakeConstView(), searchafterdelete.MakeConstView());
        }
        {
            THashSet<value_type> set;

            Test_Container_POD_(L"THashSet", set, input, negative, search.MakeConstView(), todelete.MakeConstView(), searchafterdelete.MakeConstView());

            LOG(Test_Containers, Info, L"THashSet load_factor = {0:#f2}% max probe dist = {1}", set.load_factor(), set.max_probe_dist());
        }
        {
            std::unordered_set<value_type, Meta::THash<value_type>> set;

            Test_Container_POD_(L"std::unordered_set", set, input, negative, search.MakeConstView(), todelete.MakeConstView(), searchafterdelete.MakeConstView());
        }
        {
            TFlatSet<value_type> set;

            Test_Container_POD_(L"TFlatSet", set, input, negative, search.MakeConstView(), todelete.MakeConstView(), searchafterdelete.MakeConstView());
        }

    }
#endif //!0

    FLUSH_LOG();
    ReleaseMemoryInModules();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Test
} //!namespace PPE
