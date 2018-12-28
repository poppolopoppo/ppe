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

#include "Allocator/LinearHeap.h"
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
#include "Meta/PointerWFlags.h"
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
//template class TBasicHashTable< details::THashMapTraits_<FString, int>, Meta::THash<FString>, Meta::TEqualTo<FString>, ALLOCATOR(Container, TPair<FString COMMA int>)>;
//template class TBasicHashTable< details::THashSetTraits_<FString>, Meta::THash<FString>, Meta::TEqualTo<FString>, ALLOCATOR(Container, FString)>;
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

    size_type size() const NOEXCEPT { return _size; }
    bool empty() const NOEXCEPT { return 0 == _size; }
    size_type capacity() const NOEXCEPT { return _capacity; }

    static CONSTEXPR size_t capacity_for_n(size_t n) NOEXCEPT { return Max(16, n + ((100 - MaxLoadFactor)*n) / 100); }

    const_iterator begin() const { return const_iterator(_values, _capacity, 0).Advance(0); }
    const_iterator end() const { return const_iterator(_values, _capacity, _capacity); }

    void reserve(size_type n) {
        const size_t atleast = capacity_for_n(n);
        if (atleast > _capacity) {
            //Assert(atleast <= Primes_[31]);
            const size_type oldcapacity = _capacity;
            _capacity = Max(size_t(16), FPlatformMaths::NextPow2(atleast));
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
        if (capacity_for_n(_size + 1) > _capacity)
            reserve(_size + 1);

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

    bool erase(const_reference value) { // bubble down
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
            return false;

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

        return true;
    }

    void clear() {
        if (_capacity) {
            Assert(_values);

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
            if (it.Hash == h && key_equal()(key, _elements[it.Index]))
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
//----------------------------------------------------------------------------
struct FDenseHashTableState3 {
    u16 Index;
    u16 Hash;
    STATIC_CONST_INTEGRAL(u16, EmptyIndex, u16(-1));
    FORCE_INLINE void swap(FDenseHashTableState3& other) NOEXCEPT {
        std::swap(reinterpret_cast<u32&>(*this), reinterpret_cast<u32&>(other));
    }
};
STATIC_ASSERT(Meta::TIsPod_v<FDenseHashTableState3>);
template <
    typename _Key
    , typename _Hash = Meta::THash<_Key>
    , typename _EqualTo = Meta::TEqualTo<_Key>
    , typename _Empty = Meta::TEmptyKey<_Key>
    , typename _Allocator = ALLOCATOR(Container, _Key)
>   class EMPTY_BASES TDenseHashSet3
    : _Allocator {
public:
    typedef FDenseHashTableState3 state_t;
    typedef _Key value_type;
    typedef _Hash hasher;
    typedef _Empty key_empty;
    typedef _EqualTo key_equal;

    using allocator_type = _Allocator;
    using allocator_traits = std::allocator_traits<allocator_type>;

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
            _elements = allocator_traits::allocate(allocator_(), numAllocatedBlocks);

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
            else if (Unlikely(it.Hash == e.Hash && key_equal()(key, _elements[it.Index])))
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

        allocator_traits::construct(allocator_(), _elements + _size++, key);

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
            if (it.Hash == h && key_equal()(key, _elements[it.Index]))
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
    u32 _size : 28;
    u32 _sizeClass : 4;
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
                allocator_traits::destroy(allocator_(), _elements + ri);
            }
        }
        else {
            allocator_traits::destroy(allocator_(), _elements + st.Index);
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
        Assert_NoAssume(sizeClass < NumSizeClasses);

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

        _elements = allocator_traits::allocate(allocator_(), numBlocks);

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
            allocator_traits::deallocate(allocator_(), oldElts, NumAllocatedBlocks_());
        }

        _capacity = capacity;
        _sizeClass = sizeClass;

        Assert_NoAssume(NumAllocatedBlocks_() == numBlocks);
    }

    NO_INLINE void ReleaseMemory_() {
        Assert_NoAssume(_capacity);

        Destroy(allocator_(), MakeView());

        allocator_traits::deallocate(allocator_(), _elements, NumAllocatedBlocks_());

        _size = 0;
        _sizeClass = 0;
        _capacity = 0;
        _elements = (pointer)this;
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if USE_PPE_BENCHMARK
//----------------------------------------------------------------------------
namespace BenchmarkContainers {
template <typename T>
struct TInputData {
    TMemoryView<const T> Insert;
    TMemoryView<const T> Unkown;
    TMemoryView<const T> Search;
    TMemoryView<const T> Erase;
    TMemoryView<const T> Dense;
    TMemoryView<const T> Sparse;
    TMemoryView<const u32> Shuffled;

    template <typename _Container>
    void FillDense(_Container& c) const {
        c.reserve(Insert.size());
#ifdef WITH_PPE_ASSERT
        forrange(i, 0, Insert.size()) {
            c.insert(Insert[i]);
            forrange(j, 0, i+1)
                Assert_NoAssume(c.find(Insert[j]) != c.end());
        }

#else
        for (const auto& it : Insert)
            c.insert(it);
#endif
    }

    template <typename _Container>
    void FillSparse(_Container& c) const {
        FillDense(c);
        for (const auto& it : Sparse)
            Verify(c.erase(it));
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
    copy_empty_t() : FBenchmark{ "copy_pty" } {}
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TInputData<T>& input) const {
        auto s{ archetype };
        input.FillDense(s);

        auto c{ archetype };

        for (auto _ : state) {
            c.clear();
            state.ResetTiming();
            c = s;
            FBenchmark::DoNotOptimize(c);
        }
    }
};
class copy_dense_t : public FBenchmark {
public:
    copy_dense_t() : FBenchmark{ "copy_dns" } {}
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
    copy_sparse_t() : FBenchmark{ "copy_spr" } {}
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
class insert_dense_t : public FBenchmark {
public:
    insert_dense_t() : FBenchmark{ "insert_dns" } {
        //BatchSize = 16; // batching for better accuracy
    }
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TInputData<T>& input) const {
        auto s{ archetype };
        input.FillDense(s);

        for (auto _ : state) {
            state.PauseTiming();
            auto c = s;
            const auto& item = state.Random().RandomElement(input.Unkown);
            state.ResumeTiming();
            c.insert(item);
            FBenchmark::DoNotOptimize(c);
        }
    }
};
class insert_sparse_t : public FBenchmark {
public:
    insert_sparse_t() : FBenchmark{ "insert_spr" } {
        //BatchSize = 16; // batching for better accuracy
    }
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TInputData<T>& input) const {
        auto s{ archetype };
        input.FillSparse(s);

        for (auto _ : state) {
            state.PauseTiming();
            auto c = s;
            const auto& item = state.Random().RandomElement(input.Unkown);
            state.ResumeTiming();
            c.insert(item);
            FBenchmark::DoNotOptimize(c);
        }
    }
};
class iterate_dense_t : public FBenchmark {
public:
    iterate_dense_t() : FBenchmark{ "iter_dns" } {}
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
    iterate_sparse_t() : FBenchmark{ "iter_spr" } {}
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
    find_dense_pos_t() : FBenchmark{ "find_dns_+" } {}
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
    find_dense_neg_t() : FBenchmark{ "find_dns_-" } {}
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
    find_sparse_pos_t() : FBenchmark{ "find_spr_+" } {}
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
    find_sparse_neg_t() : FBenchmark{ "find_spr_-" } {}
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
// trying to measure cache misses by avoiding temporal coherency
// https://www.youtube.com/watch?v=M2fKMP47slQ
class find_cmiss_pos_t : public FBenchmark {
public:
    find_cmiss_pos_t() : FBenchmark{ "find_miss_+" } {}
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TInputData<T>& input) const {
        auto c{ archetype };
        input.FillDense(c);

        STACKLOCAL_STACK(_Container, tables, 128);
        forrange(i, 0, 128)
            tables.Push(c);

        AssertRelease(input.Unkown.size() == input.Shuffled.size());

        for (auto _ : state) {
            const u32* pIndex = input.Shuffled.data();
            for (const auto& it : input.Dense)
                FBenchmark::DoNotOptimizeLoop(tables[*(pIndex++) & 127].find(it));
            FBenchmark::ClobberMemory();
        }
    }
};
class find_cmiss_neg_t : public FBenchmark {
public:
    find_cmiss_neg_t() : FBenchmark{ "find_miss_-" } {}
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TInputData<T>& input) const {
        auto c{ archetype };
        input.FillSparse(c);

        STACKLOCAL_STACK(_Container, tables, 128);
        forrange(i, 0, 128)
            tables.Push(c);

        AssertRelease(input.Unkown.size() == input.Shuffled.size());

        for (auto _ : state) {
            const u32* pIndex = input.Shuffled.data();
            for (const auto& it : input.Unkown)
                FBenchmark::DoNotOptimizeLoop(tables[*(pIndex++) & 127].find(it));
            FBenchmark::ClobberMemory();
        }
    }
};
class erase_dense_pos_t : public FBenchmark {
public:
    erase_dense_pos_t() : FBenchmark{ "erase_dns_+" } {}
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TInputData<T>& input) const {
        auto s{ archetype };
        input.FillDense(s);

        auto c{ archetype };
        for (auto _ : state) {
            c = s;
            state.ResetTiming();
            for (const auto& it : input.Search)
                FBenchmark::DoNotOptimizeLoop(c.erase(it));
            FBenchmark::ClobberMemory();
        }
    }
};
class erase_dense_neg_t : public FBenchmark {
public:
    erase_dense_neg_t() : FBenchmark{ "erase_dns_-" } {}
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TInputData<T>& input) const {
        auto s{ archetype };
        input.FillDense(s);

        auto c{ archetype };
        for (auto _ : state) {
            c = s;
            state.ResetTiming();
            for (const auto& it : input.Unkown)
                FBenchmark::DoNotOptimizeLoop(c.erase(it));
            FBenchmark::ClobberMemory();
        }
    }
};
class erase_sparse_pos_t : public FBenchmark {
public:
    erase_sparse_pos_t() : FBenchmark{ "erase_spr_+" } {}
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TInputData<T>& input) const {
        auto s{ archetype };
        input.FillDense(s);

        auto c{ archetype };
        for (auto _ : state) {
            c = s;
            state.ResetTiming();
            for (const auto& it : input.Dense)
                FBenchmark::DoNotOptimizeLoop(c.erase(it));
            FBenchmark::ClobberMemory();
        }
    }
};
class erase_sparse_neg_t : public FBenchmark {
public:
    erase_sparse_neg_t() : FBenchmark{ "erase_spr_-" } {}
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TInputData<T>& input) const {
        auto s{ archetype };
        input.FillDense(s);

        auto c{ archetype };
        for (auto _ : state) {
            c = s;
            state.ResetTiming();
            for (const auto& it : input.Unkown)
                FBenchmark::DoNotOptimizeLoop(c.erase(it));
            FBenchmark::ClobberMemory();
        }
    }
};
class find_erase_dns_t : public FBenchmark {
public:
    find_erase_dns_t() : FBenchmark{ "find_erase_dns" } {}
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TInputData<T>& input) const {
        auto s{ archetype };
        input.FillDense(s);

        const size_t ns = input.Sparse.size();
        const size_t nu = input.Unkown.size();
        const size_t nn = Min(nu, ns) >> 1;

        auto c{ archetype };
        for (auto _ : state) {
            c = s;
            state.ResetTiming();
            forrange(i, 0, nn) {
                c.insert(input.Unkown[(i << 1)]);
                c.insert(input.Unkown[(i << 1) + 1]);
                c.erase(input.Sparse[i]);
            }
            FBenchmark::DoNotOptimize(c.end());
            FBenchmark::ClobberMemory();
        }
    }
};
class find_erase_spr_t : public FBenchmark {
public:
    find_erase_spr_t() : FBenchmark{ "find_erase_spr" } {}
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TInputData<T>& input) const {
        auto s{ archetype };
        input.FillSparse(s);

        const size_t ns = input.Dense.size();
        const size_t nu = input.Unkown.size();
        const size_t nn = Min(nu, ns) >> 1;

        auto c{ archetype };
        for (auto _ : state) {
            c = s;
            state.ResetTiming();
            forrange(i, 0, nn) {
                c.insert(input.Unkown[(i << 1)]);
                c.insert(input.Unkown[(i << 1) + 1]);
                c.erase(input.Dense[i]);
            }
            FBenchmark::DoNotOptimize(c.end());
            FBenchmark::ClobberMemory();
        }
    }
};
class clear_dense_t : public FBenchmark {
public:
    clear_dense_t() : FBenchmark{ "clear_dns" } {}
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TInputData<T>& input) const {
        auto s{ archetype };
        input.FillDense(s);

        auto c{ archetype };
        for (auto _ : state) {
            c = s;
            state.ResetTiming();
            c.clear();
            FBenchmark::DoNotOptimize(c);
        }
    }
};
class clear_sparse_t : public FBenchmark {
public:
    clear_sparse_t() : FBenchmark{ "clear_spr" } {}
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TInputData<T>& input) const {
        auto s{ archetype };
        input.FillSparse(s);

        auto c{ archetype };
        for (auto _ : state) {
            c = s;
            state.ResetTiming();
            c.clear();
            FBenchmark::DoNotOptimize(c);
        }
    }
};
} //!namespace BenchmarkContainers
//----------------------------------------------------------------------------
#define PPE_RUN_EXHAUSTIVE_BENCHMARKS (0)
template <typename T, typename _Generator, typename _Containers>
static void Benchmark_Containers_Exhaustive_(
    const FStringView& name, size_t dim,
    _Generator&& generator,
    _Containers&& tests ) {
#if !PPE_RUN_EXHAUSTIVE_BENCHMARKS
    UNUSED(name);
    UNUSED(dim);
    UNUSED(generator);
    UNUSED(tests);
#else
    // prepare input data

    LOG(Benchmark, Emphasis, L"Running benchmark <{0}> with {1} tests :", name, dim);

    // mt19937 has better distribution than FRandomGenerator for generating benchmark data
    std::random_device rdevice;
    std::mt19937 rand{ rdevice() };
    rand.seed(0x32F9u); // fixed seed for repro

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

    VECTOR(Benchmark, u32) shuffled;
    shuffled.resize_Uninitialized(unkown.size());
    forrange(i, 0, checked_cast<u32>(unkown.size()))
        shuffled[i] = i;
    std::shuffle(std::begin(shuffled), std::end(shuffled), rand);

    using namespace BenchmarkContainers;

    using FInputData = TInputData<T>;
    FInputData input{ insert, search, unkown, erase, dense, sparse, shuffled };
    Assert_NoAssume(insert.size());
    Assert_NoAssume(search.size());
    Assert_NoAssume(unkown.size());
    Assert_NoAssume(erase.size());
    Assert_NoAssume(dense.size());
    Assert_NoAssume(sparse.size());
    Assert_NoAssume(shuffled.size() == unkown.size());

    // prepare benchmark table

    auto bm = FBenchmark::MakeTable(
        name,
        construct_noreserve_t{},
        construct_reserve_t{},
        copy_empty_t{},
        copy_dense_t{},
        copy_sparse_t{},
        insert_dense_t{},
        insert_sparse_t{},
        iterate_dense_t{},
        iterate_sparse_t{},
        find_dense_pos_t{},
        find_dense_neg_t{},
        find_sparse_pos_t{},
        find_sparse_neg_t{},
        find_cmiss_pos_t{},
        find_cmiss_neg_t{},
        find_erase_dns_t{},
        find_erase_spr_t{},
        erase_dense_pos_t{},
        erase_dense_neg_t{},
        erase_sparse_pos_t{},
        erase_sparse_neg_t{},
        clear_dense_t{},
        clear_sparse_t{} );

    tests(bm, input);

    FBenchmark::Log(bm);

    {
        const FFilename fname{ StringFormat(L"Saved:/Benchmark/{0}/Containers/{1}.csv",
            MakeStringView(WSTRINGIZE(BUILDCONFIG)), name) };

        FStringBuilder sb;
        FBenchmark::Csv(bm, sb);
        FString s{ sb.ToString() };
        VFS_WriteAll(fname, s.MakeView().RawView(), EAccessPolicy::Truncate_Binary|EAccessPolicy::Roll);
    }
#endif //!PPE_RUN_EXHAUSTIVE_BENCHMARKS
}
//----------------------------------------------------------------------------
namespace BenchmarkContainers {
template <typename T>
struct TFindData {
    TMemoryView<const T> Insert;
    TMemoryView<const T> Unknown;
    TMemoryView<const u32> Shuffled;
};
class findspeed_dense_pos_t : public FBenchmark {
public:
    findspeed_dense_pos_t(size_t n, const FStringView& name) : FBenchmark{ name } {
        InputDim = checked_cast<u32>(n);
    }
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TFindData<T>& input) const {
        auto c{ archetype };
        c.reserve(InputDim);
        for (const auto& it : input.Insert.CutBefore(InputDim))
            c.insert(it);

        STACKLOCAL_STACK(_Container, tables, 128);
        forrange(i, 0, 128)
            tables.Push(c);

        for (auto _ : state) {
            uintptr_t hit = 0;
            const u32* pIndex = input.Shuffled.data();
            for (const auto& it : input.Insert.CutBefore(InputDim)) {
                auto& ht = tables[*(pIndex++)];
                hit ^= uintptr_t(&*ht.find(it));
            }
            FBenchmark::DoNotOptimize(hit);
        }
    }
};
} //!namespace BenchmarkContainers
template <typename T, typename _Generator, typename _Containers>
static void Benchmark_Containers_FindSpeed_(const FStringView& name, _Generator&& generator, _Containers&& tests) {
    constexpr size_t dim = 4200;

    LOG(Benchmark, Emphasis, L"Running find benchmarks <{0}> with {1} tests :", name, dim);

    // mt19937 has better distribution than FRandomGenerator for generating benchmark data
    std::random_device rdevice;
    std::mt19937 rand{ rdevice() };
    rand.seed(0x2875u); // fixed seed for repro

    VECTOR(Benchmark, T) samples;
    samples.reserve_Additional(dim * 2);
    forrange(i, 0, dim * 2)
        samples.emplace_back(generator(rand));

    std::shuffle(samples.begin(), samples.end(), rand);

    const TMemoryView<const T> insert = samples.MakeConstView().CutBefore(dim);
    const TMemoryView<const T> unkown = samples.MakeConstView().CutStartingAt(dim);

    VECTOR(Benchmark, u32) shuffled;
    shuffled.resize_Uninitialized(dim);
    forrange(i, 0, dim)
        shuffled[i] = i & 127;
    std::shuffle(shuffled.begin(), shuffled.end(), rand);

    using namespace BenchmarkContainers;

    TFindData<T> input{ insert, unkown, shuffled };

    auto bm = FBenchmark::MakeTable(
        name,
        findspeed_dense_pos_t{ 4, "4" },
        findspeed_dense_pos_t{ 6, "6" },
        findspeed_dense_pos_t{ 9, "9" },
        findspeed_dense_pos_t{ 12, "12" },
        findspeed_dense_pos_t{ 16, "16" },
        findspeed_dense_pos_t{ 20, "20" },
        findspeed_dense_pos_t{ 25, "25" },
        findspeed_dense_pos_t{ 30, "30" },
        findspeed_dense_pos_t{ 36, "36" },
        findspeed_dense_pos_t{ 42, "42" },
        findspeed_dense_pos_t{ 49, "49" },
        findspeed_dense_pos_t{ 56, "56" },
        findspeed_dense_pos_t{ 64, "64" },
        findspeed_dense_pos_t{ 72, "72" },
        findspeed_dense_pos_t{ 81, "81" },
        findspeed_dense_pos_t{ 90, "90" },
        findspeed_dense_pos_t{ 100, "100" },
        findspeed_dense_pos_t{ 110, "110" },
        findspeed_dense_pos_t{ 121, "121" },
        findspeed_dense_pos_t{ 132, "132" },
        findspeed_dense_pos_t{ 144, "144" },
        findspeed_dense_pos_t{ 156, "156" },
        findspeed_dense_pos_t{ 169, "169" },
        findspeed_dense_pos_t{ 182, "182" },
        findspeed_dense_pos_t{ 196, "196" }
#ifndef WITH_PPE_ASSERT
        ,
        findspeed_dense_pos_t{ 210, "210" },
        findspeed_dense_pos_t{ 225, "225" },
        findspeed_dense_pos_t{ 240, "240" },
        findspeed_dense_pos_t{ 256, "256" },
        findspeed_dense_pos_t{ 272, "272" },
        findspeed_dense_pos_t{ 289, "289" },
        findspeed_dense_pos_t{ 306, "306" },
        findspeed_dense_pos_t{ 324, "324" },
        findspeed_dense_pos_t{ 342, "342" },
        findspeed_dense_pos_t{ 361, "361" },
        findspeed_dense_pos_t{ 380, "380" },
        findspeed_dense_pos_t{ 400, "400" },
        findspeed_dense_pos_t{ 420, "420" },
        findspeed_dense_pos_t{ 441, "441" },
        findspeed_dense_pos_t{ 462, "462" },
        findspeed_dense_pos_t{ 484, "484" }
#if PPE_RUN_EXHAUSTIVE_BENCHMARKS
        ,
        findspeed_dense_pos_t{ 506, "506" },
        findspeed_dense_pos_t{ 529, "529" },
        findspeed_dense_pos_t{ 552, "552" },
        findspeed_dense_pos_t{ 576, "576" },
        findspeed_dense_pos_t{ 600, "600" },
        findspeed_dense_pos_t{ 625, "625" },
        findspeed_dense_pos_t{ 650, "650" },
        findspeed_dense_pos_t{ 676, "676" },
        findspeed_dense_pos_t{ 702, "702" },
        findspeed_dense_pos_t{ 729, "729" },
        findspeed_dense_pos_t{ 756, "756" },
        findspeed_dense_pos_t{ 784, "784" },
        findspeed_dense_pos_t{ 812, "812" },
        findspeed_dense_pos_t{ 841, "841" },
        findspeed_dense_pos_t{ 870, "870" },
        findspeed_dense_pos_t{ 900, "900" },
        findspeed_dense_pos_t{ 930, "930" },
        findspeed_dense_pos_t{ 961, "961" },
        findspeed_dense_pos_t{ 992, "992" },
        findspeed_dense_pos_t{ 1024, "1024" },
        findspeed_dense_pos_t{ 1056, "1056" },
        findspeed_dense_pos_t{ 1089, "1089" },
        findspeed_dense_pos_t{ 1122, "1122" }
#endif //!PPE_RUN_EXHAUSTIVE_BENCHMARKS
#endif //!WITH_PPE_ASSERT
    );

    tests(bm, input);

    FBenchmark::Log(bm);

    {
        const FFilename fname{ StringFormat(L"Saved:/Benchmark/{0}/Containers/{1}.csv",
            MakeStringView(WSTRINGIZE(BUILDCONFIG)), name) };

        FStringBuilder sb;
        FBenchmark::Csv(bm, sb);
        FString s{ sb.ToString() };
        VFS_WriteAll(fname, s.MakeView().RawView(), EAccessPolicy::Truncate_Binary | EAccessPolicy::Roll);
    }
}
//----------------------------------------------------------------------------
template <typename T, typename _Generator>
static void Test_PODSet_(const FString& name, const _Generator& generator) {
    auto containers_large = [](auto& bm, const auto& input) {
        /*{ // very buggy
            typedef TCompactHashSet<T> hashtable_type;

            hashtable_type set;
            bm.Run("TCompactHashSet", set, input);
        }*/
        {
            typedef TDenseHashSet2<T> hashtable_type;

            hashtable_type set;
            bm.Run("TDenseHashSet2", set, input);
        }
        {
            typedef TDenseHashSet3<T> hashtable_type;

            hashtable_type set;
            bm.Run("TDenseHashSet3", set, input);
        }
#if 0
        {
            typedef TDenseHashSet2<THashMemoizer<T>> hashtable_type;

            hashtable_type set;
            bm.Run("TDenseHashSet2_M", set, input);
        }
        {
            typedef TDenseHashSet3<THashMemoizer<T>> hashtable_type;

            hashtable_type set;
            bm.Run("TDenseHashSet3_M", set, input);
        }
#endif
        {
            THashSet<T> set;

            bm.Run("THashSet", set, input);
        }
        {
            std::unordered_set<T, Meta::THash<T>, Meta::TEqualTo<T>, ALLOCATOR(Container, T)> set;

            bm.Run("unordered_set", set, input);
        }
    };

    auto containers_all = [&](auto& bm, const auto& input) {
#if PPE_RUN_EXHAUSTIVE_BENCHMARKS
        {
            typedef TVector<T> vector_type;

            vector_type v;

            struct FAdapter_ {
                vector_type v;
                size_t size() const { return v.size(); }
                auto begin() const { return v.begin(); }
                auto end() const { return v.end(); }
                void insert(T i) { v.push_back(i); }
                auto find(T i) const { return std::find(v.begin(), v.end(), i); }
                void reserve(size_t n) { v.reserve(n); }
                bool erase(T i) {
                    auto it = find(i);
                    if (v.end() == it)
                        return false;
                    v.erase_DontPreserveOrder(it);
                    return true;
                }
                void clear() { v.clear(); }
            };

            FAdapter_ set;

            bm.Run("TVector", set, input);
        }
#endif //!PPE_RUN_EXHAUSTIVE_BENCHMARKS
        {
            TFlatSet<T> set;

            bm.Run("TFlatSet", set, input);
        }
        {
            TFixedSizeHashSet<T, 2048> set;

            bm.Run("TFixedSizeHashSet", set, input);
        }
        containers_large(bm, input);
    };

    Benchmark_Containers_FindSpeed_<T>(name + "_find", generator, containers_all);

    Benchmark_Containers_Exhaustive_<T>(name + "_20", 20, generator, containers_all);
    Benchmark_Containers_Exhaustive_<T>(name + "_50", 50, generator, containers_all);
    Benchmark_Containers_Exhaustive_<T>(name + "_200", 200, generator, containers_large);
#ifndef WITH_PPE_ASSERT
    Benchmark_Containers_Exhaustive_<T>(name + "_2000", 2000, generator, containers_large);
    Benchmark_Containers_Exhaustive_<T>(name + "_20000", 20000, generator, containers_large);
#endif
}
//----------------------------------------------------------------------------
static void Test_StringSet_() {
    TRawStorage<char> stringPool;
    stringPool.Resize_DiscardData(64 * 1024); // 64k of text
    FRandomGenerator rnd(42);
    stringPool.MakeView().Collect([&](size_t, char* pch) {
        constexpr char Charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!-*.$^@#~";
        *pch = Charset[rnd.Next(lengthof(Charset) - 1/* null char */)];
    });

    auto generator = [&](auto& rnd) {
        constexpr size_t MinSize = 5;
        constexpr size_t MaxSize = 60;

        const size_t n = (rnd() % (MaxSize - MinSize + 1)) + MinSize;
        const size_t o = (rnd() % (stringPool.size() - n));

        return FStringView(&stringPool[o], n);
    };

    auto containers = [](auto& bm, const auto& input) {
        /*{
            typedef TDenseHashSet<
                FStringView,
                TStringViewHasher<char, ECase::Sensitive>,
                TStringViewEqualTo<char, ECase::Sensitive>
            >   hashtable_type;

            hashtable_type set;
            bm.Run("DenseHashSet", set, input);
        }*//*
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
            bm.Run("TDenseHashSet2", set, input);
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
            bm.Run("TDenseHashSet2_M", set, input);
        }
        {
            typedef TDenseHashSet3<
                FStringView,
                TStringViewHasher<char, ECase::Sensitive>,
                TStringViewEqualTo<char, ECase::Sensitive>
            >   hashtable_type;

            hashtable_type set;
            bm.Run("TDenseHashSet3", set, input);
        }
        {
            typedef TDenseHashSet3<
                THashMemoizer<
                FStringView,
                TStringViewHasher<char, ECase::Sensitive>,
                TStringViewEqualTo<char, ECase::Sensitive>
                >
            >   hashtable_type;

            hashtable_type set;
            bm.Run("TDenseHashSet3_M", set, input);
        }
        {
            STRINGVIEW_HASHSET(Container, ECase::Sensitive) set;

            bm.Run("THashSet", set, input);
        }
        {
            STRINGVIEW_HASHSET_MEMOIZE(Container, ECase::Sensitive) set;

            bm.Run("THashSet_M", set, input);
        }
        /*{
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
        }
        {
            std::unordered_set<
                TBasicStringViewHashMemoizer<char, ECase::Sensitive>,
                Meta::THash< TBasicStringViewHashMemoizer<char, ECase::Sensitive> >
            >   set;

            bm.Run("unordered_set_M", set, input);
        }
    };

    Benchmark_Containers_FindSpeed_<FStringView>("Strings_find", generator, containers);

    Benchmark_Containers_Exhaustive_<FStringView>("Strings_20", 20, generator, containers);
    Benchmark_Containers_Exhaustive_<FStringView>("Strings_50", 50, generator, containers);
#ifndef WITH_PPE_ASSERT
    Benchmark_Containers_Exhaustive_<FStringView>("Strings_200", 200, generator, containers);
    Benchmark_Containers_Exhaustive_<FStringView>("Strings_2000", 2000, generator, containers);
#endif
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
    Test_PODSet_<u32>("u32", [](auto& rnd) { return u32(rnd()); });
    Test_PODSet_<u64>("u64", [](auto& rnd) { return u64(rnd()); });
    Test_PODSet_<u128>("u128", [](auto& rnd) { return u128{ u64(rnd()), u64(rnd()) }; });
    Test_PODSet_<u256>("u256", [](auto& rnd) { return u256{ { u64(rnd()), u64(rnd()) }, { u64(rnd()), u64(rnd()) } }; });
    Test_StringSet_();
#endif

    FLUSH_LOG();
    ReleaseMemoryInModules();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Test
} //!namespace PPE
