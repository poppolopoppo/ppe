#include "stdafx.h"

#include "Core/Container/AssociativeVector.h"
#include "Core/Container/BurstTrie.h"
#include "Core/Container/FlatMap.h"
#include "Core/Container/FlatSet.h"
#include "Core/Container/HashTable.h"
#include "Core/Container/StringHashSet.h"

#include "Core/Diagnostic/Logger.h"
#include "Core/Diagnostic/Profiling.h"
#include "Core/IO/BufferedStream.h"
#include "Core/IO/FileStream.h"
#include "Core/IO/FormatHelpers.h"
#include "Core/IO/FS/Filename.h"
#include "Core/IO/String.h"
#include "Core/IO/StringBuilder.h"
#include "Core/IO/StringView.h"
#include "Core/IO/TextWriter.h"
#include "Core/IO/VirtualFileSystem.h"
#include "Core/Maths/Maths.h"
#include "Core/Maths/PrimeNumbers.h"
#include "Core/Maths/RandomGenerator.h"
#include "Core/Memory/MemoryStream.h"
#include "Core/Time/TimedScope.h"

#include <algorithm>
#include <bitset>
#include <random>
#include <unordered_set>

namespace Core {
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
} //!namespace Core

namespace Core {
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
        TIterator& operator++(int) { TIterator tmp(*this); Advance(); return (*this); }

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

    size_type size() const { return _size; }
    bool empty() const { return 0 == _size; }
    size_type capacity() const { return _capacity; }

    const_iterator begin() const { return const_iterator(_values, _capacity, 0).Advance(0); }
    const_iterator end() const { return const_iterator(_values, _capacity, _capacity); }

    void resize(size_type atleast) {
        atleast = atleast + ((100-MaxLoadFactor)*atleast)/100;
        if (atleast > _capacity) {
            //Assert(atleast <= Primes_[31]);
            const size_type oldcapacity = _capacity;
            _capacity = Meta::NextPow2(atleast);
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
template <typename _Key>
struct TRobinHoodHashEmptyKey {
    STATIC_ASSERT(sizeof(_Key) >= sizeof(u32));
    static constexpr u32 Empty = 0xBADDADD1ul;
    static bool IsEmpty(const _Key& key) { return (reinterpret_cast<const u32&>(key) == Empty); }
    static void MarkAsEmpty(_Key* pkey) { reinterpret_cast<u32&>(*pkey) = Empty; }
};
//----------------------------------------------------------------------------
template <typename _Key, typename _EmptyKey = TRobinHoodHashEmptyKey<_Key> >
struct TRobinHoodHashSetTraits {
    typedef _Key key_type;
    typedef _Key mapped_type;
    typedef _Key value_type;
    typedef _Key public_type;
    typedef _Key emptykey_type;
    static _Key& Key(value_type& v) { return v; }
    static const _Key& Key(const value_type& v) { return v; }
};
//----------------------------------------------------------------------------
/*
template <
    typename _Traits
,   typename _Hash = Meta::THash<typename _Traits::key_type>
,   typename _EqualTo = Meta::TEqualTo<typename _Traits::key_type>
,   typename _Allocator = ALLOCATOR(Container, typename _Traits::value_type)
>   class TRobinHoodHashTable : TRebindAlloc< _Allocator, typename _Traits::value_type > {
public:
    typedef _Traits traits_type;
    typedef typename traits_type::key_type key_type;
    typedef typename traits_type::mapped_type mapped_type;
    typedef typename traits_type::value_type value_type;
    typedef typename traits_type::public_type public_type;
    typedef typename traits_type::public_type public_type;
    typedef typename traits_type::emptykey_type emptykey_type;

    typedef _Hash hasher;
    typedef _EqualTo key_equal;

    typedef TRebindAlloc<_Allocator, value_type> allocator_type;
    typedef std::allocator_traits<allocator_type> allocator_traits;

    typedef Meta::TAddReference<public_type> reference;
    typedef Meta::TAddReference<const public_type> const_reference;
    typedef Meta::TAddPointer<public_type> pointer;
    typedef Meta::TAddPointer<const public_type> const_pointer;

    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

    static constexpr size_type MaxLoadFactor = 90;

    TRobinHoodHashTable() : _values(nullptr), _capacity(0), _size(0) {}
    ~TRobinHoodHashTable() { clear_ReleaseMemory(); }

    size_type size() const { return _size; }
    bool empty() const { return 0 == _size; }
    size_type capacity() const { return _capacity; }

    void resize(size_type atleast) {
        if (atleast <= (_capacity * MaxLoadFactor) / 100)
            return;

        const pointer oldBuckets = _buckets;
        const u32 oldCapacity = _capacity;

        _capacity = 1;
        _buckets = allocator_traits::allocate()

        forrange(pbucket, oldBuckets, oldBuckets + _capacity) {
            if (IsEmpty_(*pbucket))
                continue;

            Insert_(HashValue_(*pbucket), std::move(*pbucket));
            allocator_traits::destroy(pbucket);
        }

        allocator_traits::deallocate(oldBuckets, _capacity);
    }

    bool insert(const_reference value) {
        if (_size + 1 > (_capacity * MaxLoadFactor) / 100)
            resize(_capacity * 2);
    }

    pointer end() const { return nullptr; }

    pointer find(const_reference value) const {

    }

    void erase(const_reference value) {

    }

    void clear() {
        for (value_type& v : MakeView(_values, _values + _capacity)) {
            if (v.)
        }
    }

    void clear_ReleaseMemory() {
        if (nullptr == _values)
            return;

        clear();
        allocator_traits::deallocate(*this, _values);
        _values = nullptr;
        _capacity = 0;
    }

    size_type load_factor() const {
        return ((_size * 100) / _capacity);
    }

private:
    static bool IsEqual_(const value_type& lhs, const value_type& rhs) {
        return key_equal()(trais_type::Key(lhs), traits_type::Key(rhs));
    }

    static bool IsEmpty_(const value_type& v) {
        return emptykey_type::IsEmpty(traits_type::Key(v));
    }

    static hash_t HashValue_(const value_type& v) {
        return hasher()(traits_type::Key(v));
    }

    size_t DesiredPos_(hash_t h) const {
        return Bounded(u32(h._value), _capacity);
    }

    size_t ProbeDistance_(hash_t h, size_t pos) {
        const size_t b = DesiredPos_(h);
        return (b < pos ? b + _capacity - pos : b - pos);
    }

    size_t Insert_(hash_t hash, value_type&& value) {
        Assert(_values);
        Assert(load_factor() <= MaxLoadFactor);

        size_t dist = 0;
        for(size_t b = Bounded(hash, _capacity); ; ++dist, b = (b + 1 < _capacity ? b + 1 : 0) ) {
            value_type& bucket = _buckets[b];

            if (IsEmpty_(bucket)) {
                allocator_traits::construct(*this, &bucket, std::move(value));
                _size++;
                return b;
            }

            Assert(IsEqual_(value, bucket) == false);

            const hash_t h = HashValue_(bucket);
            const size_t d = ProbeDistance_(h, b);
            if (d < dist) {
                using std::swap;
                swap(bucket, value);
                hash = h;
                dist = d;
            }
        }

        AssertNotReached();
        return size_t(-1);
    }

    u32 _capacity;
    u32 _size;
    pointer _buckets;
};
*/
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

#ifdef WITH_CORE_ASSERT
    static constexpr size_t loops = 100;
#else
    static constexpr size_t loops = 1000;
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
                CORE_THROW_IT(std::exception("invalid set iteration"));
        }
    }
    {
        BENCHMARK_SCOPE(name, L"search");
        forrange(i, 0, loops) {
            volatile size_t count = 0;
            for (const auto& word : search)
                if (set.end() == set.find(word)) {
                    AssertNotReached();
                }
                else {
                    ++count;
                }
            if (search.size() != count)
                CORE_THROW_IT(std::exception("invalid set search"));
        }
    }
    {
        BENCHMARK_SCOPE(name, L"negative search");
        forrange(i, 0, loops) {
            volatile size_t count = 0;
            for (const auto& word : negative)
                if (set.end() != set.find(word)) {
                    AssertNotReached();
                }
                else {
                    ++count;
                }
            if (negative.size() != count)
                CORE_THROW_IT(std::exception("invalid set negative search"));
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
                CORE_THROW_IT(std::exception("invalid set iteration"));
        }
    }
    {
        BENCHMARK_SCOPE(name, L"search after delete");
        forrange(i, 0, loops) {
            volatile size_t pos_count = 0;
            volatile size_t neg_count = 0;
            for (const auto& word : searchafterdelete)
                if (set.end() == set.find(word)) {
                    ++neg_count;
                }
                else {
                    ++pos_count;
                }
            if (searchafterdelete.size() != pos_count + neg_count)
                CORE_THROW_IT(std::exception("invalid set search after delete"));
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

#ifdef WITH_CORE_ASSERT
    static constexpr size_t loops = 10;
#else
    static constexpr size_t loops = 1000;
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
                CORE_THROW_IT(std::exception("invalid search"));
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
                CORE_THROW_IT(std::exception("invalid set iteration"));
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
                CORE_THROW_IT(std::exception("invalid negative search"));
        }
    }
    {
        BENCHMARK_SCOPE(name, L"deletion");
        volatile size_t count = 0;
        for (const auto& word : todelete.Map(adaptor))
            count += (set.erase(word) ? 1 : 0);
        if (todelete.size() != count)
            CORE_THROW_IT(std::exception("invalid deletion"));
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
                CORE_THROW_IT(std::exception("invalid iteration after delete"));
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
                CORE_THROW_IT(std::exception("invalid search after delete"));
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
void Test_Containers() {
    LOG(Test_Containers, Emphasis, L"starting container tests ...");
    {
        float4x4 m = Make3DTransformMatrix(float3(1,2,3), 10.0f, float3::Z(), Radians(33.0f));
        float4 p = float4::W();
        float4 ws = m.Multiply(p);
        float4 ss = Transform4(m, p);
        UNUSED(ws);
        UNUSED(ss);
    }
    {
        const FFilename filename = L"Tmp:/koala/a/Test/../robocop/4/../3/2/1/../a/b/c/../robotapp.bin";
        const FFilename filename2 = L"Tmp:/Test/toto/../chimpanzee/../../koala/a/b/../c/1/../2/3/robotapp.raw";

        LOG(Test_Containers, Info, L"{0}", filename);
        LOG(Test_Containers, Info, L"{0}", filename2);

        FFilename normalized = filename.Normalized();
        FFilename normalized2 = filename2.Normalized();

        LOG(Test_Containers, Info, L"{0}", normalized);
        LOG(Test_Containers, Info, L"{0}", normalized2);

        FFilename relative = filename.Relative(filename2.Dirpath());
        FFilename relative2 = filename2.Relative(filename.Dirpath());

        LOG(Test_Containers, Info, L"{0}", relative);
        LOG(Test_Containers, Info, L"{0}", relative2);

        FFilename absolute = relative.Absolute(filename2.Dirpath());
        FFilename absolute2 = relative2.Absolute(filename.Dirpath());

        LOG(Test_Containers, Info, L"{0}", absolute);
        LOG(Test_Containers, Info, L"{0}", absolute2);

        Assert(absolute == normalized);
        Assert(absolute2 == normalized2);
    }
    {
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
    {
        LOG(Test_Containers, Info, L"{0}", Fmt::Repeat(MakeStringView(L">>="), 20));
        LOG(Test_Containers, Info, L"FStringView collection");

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
    {
        LOG(Test_Containers, Info, L"{0}", Fmt::Repeat(MakeStringView(L">>="), 20));
        LOG(Test_Containers, Info, L"Integer collection");

        typedef double value_type;

        const size_t COUNT = 2000;

        std::random_device rdevice;
        std::mt19937 rand(rdevice());

        VECTOR(Container, value_type) all;
        all.reserve(COUNT);
        forrange(i, 1, COUNT+1)
            all.push_back((value_type)(i/value_type(COUNT)));

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

    Core::FCoreModule::ClearAll_UnusedMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Test
} //!namespace Core
