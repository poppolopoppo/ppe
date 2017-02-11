#include "stdafx.h"

#include "Core/Container/AssociativeVector.h"
#include "Core/Container/BurstTrie.h"
#include "Core/Container/FlatMap.h"
#include "Core/Container/FlatSet.h"
#include "Core/Container/HashTable.h"
#include "Core/Container/StringHashSet.h"

#include "Core/Diagnostic/Profiling.h"
#include "Core/IO/StringView.h"
#include "Core/IO/VFS/VirtualFileSystemStream.h"
#include "Core/Maths/Maths.h"
#include "Core/Memory/MemoryStream.h"
#include "Core/Time/TimedScope.h"

#include <bitset>
#include <fstream>

namespace Core {
namespace ContentGenerator {
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
,   typename _Hash = THash<_Key>
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
    typedef typename std::add_pointer<value_type>::type pointer;
    typedef typename std::add_pointer<const value_type>::type const_pointer;

    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

    static constexpr size_type MaxLoadFactor = 50;

    TCompactHashSet() : _values(nullptr), _capacity(0), _size(0) {}
    ~TCompactHashSet() { clear(); }

    size_type size() const { return _size; }
    bool empty() const { return 0 == _size; }
    size_type capacity() const { return _capacity; }

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

    pointer find(const_reference value) const {
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

        return (not key_equal()(_values[bucket], empty_key) ? _values + bucket : nullptr);
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
template class TAssociativeVector<FString, int>;
template class TFlatMap<FString, int>;
template class TFlatSet<FString>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void Test_Containers() {
    {
        float4x4 m = Make3DTransformMatrix(float3(1,2,3), 10.0f, float3::Z(), Radians(33.0f));
        float4 p = float4::W();
        float4 ws = m.Multiply(p);
        float4 ss = Transform4(m, p);
    }
    {
        const FFilename filename = L"Tmp:/koala/a/Test/../robocop/4/../3/2/1/../a/b/c/../robotapp.bin";
        const FFilename filename2 = L"Tmp:/Test/toto/../chimpanzee/../../koala/a/b/../c/1/../2/3/robotapp.raw";

        std::cout << filename << eol;
        std::cout << filename2 << eol;

        FFilename normalized = filename.Normalized();
        FFilename normalized2 = filename2.Normalized();

        std::cout << normalized << eol;
        std::cout << normalized2 << eol;

        FFilename relative = filename.Relative(filename2.Dirpath());
        FFilename relative2 = filename2.Relative(filename.Dirpath());

        std::cout << relative << eol;
        std::cout << relative2 << eol;

        FFilename absolute = relative.Absolute(filename2.Dirpath());
        FFilename absolute2 = relative2.Absolute(filename.Dirpath());

        std::cout << absolute << eol;
        std::cout << absolute2 << eol;

        Assert(absolute == normalized);
        Assert(absolute2 == normalized2);
    }
    {
        LOG(Info, L"{0}", Repeat<20>(L">>="));
        LOG(Info, L"FStringView collection");

        const FFilename filename = L"Process:/dico.txt";

        VECTOR_THREAD_LOCAL(Container, FString) words;
        {
            const TUniquePtr<IVirtualFileSystemIStream> reader = VFS_OpenBinaryReadable(filename);
            if (not reader)
                AssertNotReached();

            char buffer[2048];
            while (true) {
                const FStringView line = reader->ReadLine(buffer);
                if (line.empty())
                    break;

                const FStringView word = Chomp(line);
                words.emplace_back(ToString(word));
            }
        }

        //words.resize((UINT16_MAX*80)/100);
        //words.resize(8);

        VECTOR_THREAD_LOCAL(Container, FStringView) all;
        all.reserve(words.size());
        for (const FString& word : words)
            all.emplace_back(MakeStringView(word));
        std::random_shuffle(all.begin(), all.end());

        const size_t k = (all.size() * 80) / 100;

        const auto input = all.MakeConstView().CutBefore(k);
        const auto negative = all.MakeConstView().CutStartingAt(k);

        VECTOR_THREAD_LOCAL(Container, FStringView) search(input);
        std::random_shuffle(search.begin(), search.end());

#ifdef WITH_CORE_ASSERT
        static constexpr size_t loops = 10;
#else
        static constexpr size_t loops = 100;
#endif

        /*{
            LOG(Info, L"{0}", Repeat<20>(L"-*=*"));
            const FBenchmarkScope bench("TBurstTrie");

            STRINGTRIE_SET(Container, ECase::Sensitive, 31) set;
            {
                const FBenchmarkScope bench("TBurstTrie construction");
                PROFILING_SCOPE(Global, 1, "TBurstTrie construction");
                for (const FStringView& word : input)
                    set.Insert_AssertUnique(word);
            }
            {
                const FBenchmarkScope bench("TBurstTrie optimization");
                PROFILING_SCOPE(Global, 1, "TBurstTrie optimization");
                set.Optimize();
            }
            {
                const FBenchmarkScope bench("TBurstTrie search");
                PROFILING_SCOPE(Global, 2, "TBurstTrie search");
                forrange(i, 0, loops) {
                    for (const FStringView& word : search)
                        if (not set.Contains(word))
                            AssertNotReached();
                }
            }
        }
        {
            LOG(Info, L"{0}", Repeat<20>(L"-*=*"));
            const FBenchmarkScope bench("FBulkTrie");

            FBulkTrie<char, void, 8192, 31> set;
            {
                const FBenchmarkScope bench("FBulkTrie construction");
                PROFILING_SCOPE(Global, 3, "FBulkTrie construction");
                for (const FStringView& word : input)
                    set.Insert_AssertUnique(word);
            }

            {
                const FBenchmarkScope bench("FBulkTrie search");
                PROFILING_SCOPE(Global, 4, "FBulkTrie search");
                forrange(i, 0, loops) {
                    for (const FStringView& word : search)
                        if (not set.Contains(word))
                            AssertNotReached();
                    }
            }
        }*/
        {
            LOG(Info, L"{0}", Repeat<20>(L"-*=*"));

            const FBenchmarkScope bench("TCompactHashSet");

            typedef TCompactHashSet<
                FStringView,
                TStringViewHasher<char, ECase::Sensitive>,
                TStringViewEqualTo<char, ECase::Sensitive>
            >   hashtable_type;

            hashtable_type set;
            {
                const FBenchmarkScope subbench("TCompactHashSet construction");
                PROFILING_SCOPE(Global, 3, "TCompactHashSet construction");
                set.resize(input.size());
                for (const FStringView& word : input)
                    set.insert(word);
            }
            Assert(set.size() == input.size());
            {
                const FBenchmarkScope subbench("TCompactHashSet search");
                PROFILING_SCOPE(Global, 4, "TCompactHashSet search");
                forrange(i, 0, loops) {
                    for (const FStringView& word : search)
                        if (nullptr == set.find(word))
                            AssertNotReached();
                    }
            }
            {
                const FBenchmarkScope subbench("TCompactHashSet negative search");
                PROFILING_SCOPE(Global, 4, "TCompactHashSet negative search");
                forrange(i, 0, loops) {
                    for (const FStringView& word : negative)
                        if (nullptr != set.find(word))
                            AssertNotReached();
                    }
            }
        }
        {
            LOG(Info, L"{0}", Repeat<20>(L"-*=*"));

            const FBenchmarkScope bench("TCompactHashSet Memoize");

            typedef TCompactHashSet<
                THashMemoizer<
                    FStringView,
                    TStringViewHasher<char, ECase::Sensitive>,
                    TStringViewEqualTo<char, ECase::Sensitive>
                >
            >   hashtable_type;

            hashtable_type set;
            {
                const FBenchmarkScope subbench("TCompactHashSet Memoize construction");
                PROFILING_SCOPE(Global, 3, "TCompactHashSet Memoize construction");
                set.resize(input.size());
                for (const FStringView& word : input)
                    set.insert(word);
            }
            Assert(set.size() == input.size());
            {
                const FBenchmarkScope subbench("TCompactHashSet Memoize search");
                PROFILING_SCOPE(Global, 4, "TCompactHashSet Memoize search");
                forrange(i, 0, loops) {
                    for (const FStringView& word : search)
                        if (nullptr == set.find(word))
                            AssertNotReached();
                }
            }
            {
                const FBenchmarkScope subbench("TCompactHashSet Memoize negative search");
                PROFILING_SCOPE(Global, 4, "TCompactHashSet Memoize negative search");
                forrange(i, 0, loops) {
                    for (const FStringView& word : negative)
                        if (nullptr != set.find(word))
                            AssertNotReached();
                }
            }
        }
        {
            LOG(Info, L"{0}", Repeat<20>(L"-*=*"));
            const FBenchmarkScope bench("THashTable");

            typedef THashTable<
                FStringView, void,
                TStringViewHasher<char, ECase::Sensitive>,
                TStringViewEqualTo<char, ECase::Sensitive>
            >   hashtable_type;

            hashtable_type set;
            {
                const FBenchmarkScope subbench("THashTable construction");
                PROFILING_SCOPE(Global, 3, "THashTable construction");
                set.reserve(input.size());
                size_t count = 0;
                for (const FStringView& word : input) {
                    set.insert(word);
                    count++;
Assert(set.size() == count);
                }
            }
            Assert(set.size() == input.size());
#ifndef PROFILING_ENABLED
            FHashTableStats stats = set.ProbingStats();
            LOG(Info, L"[HASHTABLE] Probing stats =\n"
                L"    Min : {0}\n"
                L"    Max : {1}\n"
                L"    Mean: {2}\n"
                L"    Dev : {3}\n",
                stats.MinProbe, stats.MaxProbe, stats.MeanProbe, stats.DevProbe);
#endif
            {
                const FBenchmarkScope subbench("THashTable search");
                PROFILING_SCOPE(Global, 4, "THashTable search");
                forrange(i, 0, loops) {
                    for (const FStringView& word : search)
                        if (set.end() == set.find(word))
                            AssertNotReached();
                }
            }
            {
                const FBenchmarkScope subbench("THashTable negative search");
                PROFILING_SCOPE(Global, 4, "THashTable negative search");
                forrange(i, 0, loops) {
                    for (const FStringView& word : negative)
                        if (set.end() != set.find(word))
                            AssertNotReached();
                }
            }
        }
        {
            LOG(Info, L"{0}", Repeat<20>(L"-*=*"));
            const FBenchmarkScope bench("THashTable Memoize");

            typedef THashTable<
                THashMemoizer<
                    FStringView,
                    TStringViewHasher<char, ECase::Sensitive>,
                    TStringViewEqualTo<char, ECase::Sensitive>
                >,
                void
            >   hashtable_type;

            hashtable_type set;
            {
                const FBenchmarkScope subbench("THashTable Memoize construction");
                PROFILING_SCOPE(Global, 3, "THashTable Memoize construction");
                set.reserve(input.size());
                size_t count = 0;
                for (const FStringView& word : input) {
                    set.insert(word);
                    count++;
                    Assert(set.size() == count);
                }
            }
            Assert(set.size() == input.size());
#ifndef PROFILING_ENABLED
            FHashTableStats stats = set.ProbingStats();
            LOG(Info, L"[HASHTABLE] Probing stats =\n"
                L"    Min : {0}\n"
                L"    Max : {1}\n"
                L"    Mean: {2}\n"
                L"    Dev : {3}\n",
                stats.MinProbe, stats.MaxProbe, stats.MeanProbe, stats.DevProbe);
#endif
            {
                const FBenchmarkScope subbench("THashTable Memoize search");
                PROFILING_SCOPE(Global, 4, "THashTable Memoize search");
                forrange(i, 0, loops) {
                    for (const FStringView& word : search)
                        if (set.end() == set.find(word))
                            AssertNotReached();
                }
            }
            {
                const FBenchmarkScope subbench("THashTable Memoize negative search");
                PROFILING_SCOPE(Global, 4, "THashTable Memoize negative search");
                forrange(i, 0, loops) {
                    for (const FStringView& word : negative)
                        if (set.end() != set.find(word))
                            AssertNotReached();
                }
            }
        }
        {
            LOG(Info, L"{0}", Repeat<20>(L"-*=*"));
            const FBenchmarkScope bench("TFlatSet");

            TFlatSet<
                FStringView,
                TStringViewEqualTo<char, ECase::Sensitive>,
                TStringViewLess<char, ECase::Sensitive>
            >   set;
            {
                const FBenchmarkScope subbench("TFlatSet construction");
                PROFILING_SCOPE(Global, 3, "TFlatSet construction");
                set.reserve(input.size());
                set.insert(input.begin(), input.end());
            }
            Assert(set.size() == input.size());
            {
                const FBenchmarkScope subbench("TFlatSet search");
                PROFILING_SCOPE(Global, 4, "TFlatSet search");
                forrange(i, 0, loops) {
                    for (const FStringView& word : search)
                        if (set.end() == set.find(word))
                            AssertNotReached();
                }
                }
            {
                const FBenchmarkScope subbench("TFlatSet negative search");
                PROFILING_SCOPE(Global, 4, "TFlatSet negative search");
                forrange(i, 0, loops) {
                    for (const FStringView& word : negative)
                        if (set.end() != set.find(word))
                            AssertNotReached();
                }
            }
        }
        {
            LOG(Info, L"{0}", Repeat<20>(L"-*=*"));
            const FBenchmarkScope bench("THashSet");

            STRINGVIEW_HASHSET(Container, ECase::Sensitive) set;
            {
                const FBenchmarkScope subbench("THashSet construction");
                PROFILING_SCOPE(Global, 3, "THashSet construction");
                set.reserve(input.size());
                set.insert(input.begin(), input.end());
            }
            Assert(set.size() == input.size());
            {
                const FBenchmarkScope subbench("THashSet search");
                PROFILING_SCOPE(Global, 4, "THashSet search");
                forrange(i, 0, loops) {
                    for (const FStringView& word : search)
                        if (set.end() == set.find(word))
                            AssertNotReached();
                }
            }
            {
                const FBenchmarkScope subbench("THashSet negative search");
                PROFILING_SCOPE(Global, 4, "THashSet negative search");
                forrange(i, 0, loops) {
                    for (const FStringView& word : negative)
                        if (set.end() != set.find(word))
                            AssertNotReached();
                }
            }
        }
        {
            LOG(Info, L"{0}", Repeat<20>(L"-*=*"));
            const FBenchmarkScope bench("THashSet Memoize");

            STRINGVIEW_HASHSET_MEMOIZE(Container, ECase::Sensitive) set;
            {
                const FBenchmarkScope subbench("THashSet Memoize construction");
                PROFILING_SCOPE(Global, 3, "THashSet Memoize construction");
                set.reserve(input.size());
                set.insert(input.begin(), input.end());
            }
            Assert(set.size() == input.size());
            {
                const FBenchmarkScope subbench("THashSet Memoize search");
                PROFILING_SCOPE(Global, 4, "THashSet Memoize search");
                forrange(i, 0, loops) {
                    for (const FStringView& word : search)
                        if (set.end() == set.find(word))
                            AssertNotReached();
                }
            }
            {
                const FBenchmarkScope subbench("THashSet Memoize negative search");
                PROFILING_SCOPE(Global, 4, "THashSet Memoize negative search");
                forrange(i, 0, loops) {
                    for (const FStringView& word : negative)
                        if (set.end() != set.find(word))
                            AssertNotReached();
                }
            }
        }

    }
    {
        LOG(Info, L"{0}", Repeat<20>(L">>="));
        LOG(Info, L"Integer collection");

        typedef double value_type;

        const size_t COUNT = 8192;

        VECTOR_THREAD_LOCAL(Container, value_type) all;
        all.reserve(COUNT);
        forrange(i, 1, COUNT+1)
            all.push_back((value_type)(i/value_type(COUNT)));

        std::random_shuffle(all.begin(), all.end());

        const size_t k = (all.size() * 80) / 100;

        const auto input = all.MakeConstView().CutBefore(k);
        const auto negative = all.MakeConstView().CutStartingAt(k);

        VECTOR_THREAD_LOCAL(Container, value_type) search(input);
        std::random_shuffle(search.begin(), search.end());

#ifdef WITH_CORE_ASSERT
        static constexpr size_t loops = 100;
#else
        static constexpr size_t loops = 1000;
#endif
        {
            LOG(Info, L"{0}", Repeat<20>(L"-*=*"));
            const FBenchmarkScope bench("TCompactHashSet");

            typedef TCompactHashSet<value_type>   hashtable_type;

            hashtable_type set;
            {
                const FBenchmarkScope subbench("TCompactHashSet construction");
                PROFILING_SCOPE(Global, 3, "TCompactHashSet construction");
                set.resize(input.size());
                for (const auto& word : input)
                    set.insert(word);
            }
            Assert(set.size() == input.size());
            {
                const FBenchmarkScope subbench("TCompactHashSet search");
                PROFILING_SCOPE(Global, 4, "TCompactHashSet search");
                forrange(i, 0, loops) {
                    for (const auto& word : search)
                        if (nullptr == set.find(word))
                            AssertNotReached();
                    }
            }
            {
                const FBenchmarkScope subbench("TCompactHashSet negative search");
                PROFILING_SCOPE(Global, 4, "TCompactHashSet negative search");
                forrange(i, 0, loops) {
                    for (const auto& word : negative)
                        if (nullptr != set.find(word))
                            AssertNotReached();
                    }
            }
        }
        {
            LOG(Info, L"{0}", Repeat<20>(L"-*=*"));
            const FBenchmarkScope bench("THashTable");

            typedef THashTable<value_type, void>   hashtable_type;

            hashtable_type set;
            {
                const FBenchmarkScope subbench("THashTable construction");
                PROFILING_SCOPE(Global, 3, "THashTable construction");
                set.reserve(input.size());
                size_t count = 0;
                for (const auto& word : input) {
                    set.insert(word);
                    count++;
Assert(set.size() == count);
                }
            }
            Assert(set.size() == input.size());
#ifndef PROFILING_ENABLED
            FHashTableStats stats = set.ProbingStats();
            LOG(Info, L"[HASHTABLE] Probing stats =\n"
                L"    Min : {0}\n"
                L"    Max : {1}\n"
                L"    Mean: {2}\n"
                L"    Dev : {3}\n",
                stats.MinProbe, stats.MaxProbe, stats.MeanProbe, stats.DevProbe);
#endif
            {
                const FBenchmarkScope subbench("THashTable search");
                PROFILING_SCOPE(Global, 4, "THashTable search");
                forrange(i, 0, loops) {
                    for (const auto& word : search)
                        if (set.end() == set.find(word))
                            AssertNotReached();
                }
            }
            {
                const FBenchmarkScope subbench("THashTable negative search");
                PROFILING_SCOPE(Global, 4, "THashTable negative search");
                forrange(i, 0, loops) {
                    for (const auto& word : negative)
                        if (set.end() != set.find(word))
                            AssertNotReached();
                }
            }
        }
        {
            LOG(Info, L"{0}", Repeat<20>(L"-*=*"));
            const FBenchmarkScope bench("TFlatSet");

            TFlatSet<value_type> set;
            {
                const FBenchmarkScope subbench("TFlatSet construction");
                PROFILING_SCOPE(Global, 3, "TFlatSet construction");
                set.reserve(input.size());
                set.insert(input.begin(), input.end());
            }
            Assert(set.size() == input.size());
            {
                const FBenchmarkScope subbench("TFlatSet search");
                PROFILING_SCOPE(Global, 4, "TFlatSet search");
                forrange(i, 0, loops) {
                    for (const auto& word : search)
                        if (set.end() == set.find(word))
                            AssertNotReached();
                }
            }
            {
                const FBenchmarkScope subbench("TFlatSet negative search");
                PROFILING_SCOPE(Global, 4, "TFlatSet negative search");
                forrange(i, 0, loops) {
                    for (const auto& word : negative)
                        if (set.end() != set.find(word))
                            AssertNotReached();
                }
            }
        }
        {
            LOG(Info, L"{0}", Repeat<20>(L"-*=*"));
            const FBenchmarkScope bench("THashSet");

            THashSet<value_type> set;
            {
                const FBenchmarkScope subbench("THashSet construction");
                PROFILING_SCOPE(Global, 3, "THashSet construction");
                set.reserve(input.size());
                set.insert(input.begin(), input.end());
            }
            Assert(set.size() == input.size());
            {
                const FBenchmarkScope subbench("THashSet search");
                PROFILING_SCOPE(Global, 4, "THashSet search");
                forrange(i, 0, loops) {
                    for (const auto& word : search)
                        if (set.end() == set.find(word))
                            AssertNotReached();
                }
            }
            {
                const FBenchmarkScope subbench("THashSet negative search");
                PROFILING_SCOPE(Global, 4, "THashSet negative search");
                forrange(i, 0, loops) {
                    for (const auto& word : negative)
                        if (set.end() != set.find(word))
                            AssertNotReached();
                }
            }
        }

    }

    Core::CoreStartup::ClearAll_UnusedMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentGenerator
} //!namespace Core
