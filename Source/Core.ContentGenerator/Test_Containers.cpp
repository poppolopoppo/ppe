#include "stdafx.h"

#include "Core/Container/BurstTrie.h"
#include "Core/Container/HashTable.h"
#include "Core/Container/StringHashSet.h"

#include "Core/Diagnostic/Profiling.h"
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
struct BulkTrieNode : public BulkTrieNode<_Key, void , _BulkSize>{
    _Value Values[_BulkSize];

    BulkTrieNode() {}
};
//----------------------------------------------------------------------------
template <typename _Key, size_t _BulkSize>
struct BulkTrieNode<_Key, void, _BulkSize> {
    typedef uint32_t index_type;

    static constexpr index_type NoIndex = index_type(-1);

    _Key Keys[_BulkSize];
    std::bitset<_BulkSize> HasValue;
    index_type Left[_BulkSize];
    index_type Center[_BulkSize];
    index_type Right[_BulkSize];

    BulkTrieNode() {
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
,   typename _Less = Meta::Less<_Key>
,   typename _Equal = Meta::EqualTo<_Key>
,   typename _Allocator = ALLOCATOR(Container, BulkTrieNode<_Key COMMA _Value COMMA _BulkSize>)
>   class BulkTrie : _Allocator {
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

    typedef MemoryView<const _Key> sequence_type;

    typedef BulkTrieNode<_Key, _Value, _BulkSize> node_type;
    typedef typename node_type::index_type index_type;
    static constexpr index_type NoIndex = node_type::NoIndex;
    static constexpr index_type RootIndex = 0;
    typedef Vector<node_type*, typename _Allocator::template rebind<node_type*>::other> node_vector;

    struct Iterator {
        index_type Bucket;
        index_type Index;
    };

    BulkTrie() : _wordCount(0) {}
    ~BulkTrie() { Clear(); }

    BulkTrie(const BulkTrie&) = delete;
    BulkTrie& operator=(const BulkTrie&) = delete;

    size_type size() const { return _wordCount; }
    bool empty() const { return (0 == _wordCount); }

    bool Insert_ReturnIfExists(Iterator* it, const sequence_type& keys) {
        Assert(nullptr != it);
        Assert(false == keys.empty());

        it->Bucket = BucketIndex_(keys);
        Bucket& bucket = _buckets[it->Bucket];

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

        const Bucket& bucket = _buckets[it->Bucket];

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
        for (Bucket& bucket : _buckets) {
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

    struct Bucket {
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

    Bucket _buckets[BucketCount];
};
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
        const Filename filename = L"Tmp:/koala/a/Test/../robocop/4/../3/2/1/../a/b/c/../robotapp.bin";
        const Filename filename2 = L"Tmp:/Test/toto/../chimpanzee/../../koala/a/b/../c/1/../2/3/robotapp.raw";

        std::cout << filename << std::endl;
        std::cout << filename2 << std::endl;

        Filename normalized = filename.Normalized();
        Filename normalized2 = filename2.Normalized();

        std::cout << normalized << std::endl;
        std::cout << normalized2 << std::endl;

        Filename relative = filename.Relative(filename2.Dirpath());
        Filename relative2 = filename2.Relative(filename.Dirpath());

        std::cout << relative << std::endl;
        std::cout << relative2 << std::endl;

        Filename absolute = relative.Absolute(filename2.Dirpath());
        Filename absolute2 = relative2.Absolute(filename.Dirpath());

        std::cout << absolute << std::endl;
        std::cout << absolute2 << std::endl;

        Assert(absolute == normalized);
        Assert(absolute2 == normalized2);
    }
    {
        const Filename filename = L"Process:/dico.txt";

        VECTOR_THREAD_LOCAL(Container, String) words;
        {
            RAWSTORAGE_THREAD_LOCAL(FileSystem, u8) read;
            if (false == VFS_ReadAll(&read, filename, AccessPolicy::Binary))
                AssertNotReached();

            MEMORYSTREAM_THREAD_LOCAL(FileSystem) iss(std::move(read), read.size());
            char buffer[2048];
            std::streamsize len = 0;
            while (0 < (len = iss.ReadLine(buffer))) {
                const StringSlice line(buffer, checked_cast<size_t>(len));
                const StringSlice word = Chomp(line);
                words.emplace_back(ToString(word));
            }
        }

        //words.resize((UINT16_MAX*80)/100);

        VECTOR_THREAD_LOCAL(Container, StringSlice) input;
        input.reserve(words.size());
        for (const String& word : words)
            input.emplace_back(MakeStringSlice(word));
        std::random_shuffle(input.begin(), input.end());

        VECTOR_THREAD_LOCAL(Container, StringSlice) search(input);
        std::random_shuffle(search.begin(), search.end());

#ifdef WITH_CORE_ASSERT
        static const size_t loops = 10;
#else
        static const size_t loops = 100;
#endif

        /*{
            const BenchmarkScope bench("BurstTrie");

            STRINGTRIE_SET(Container, CaseSensitive::True, 31) set;
            {
                const BenchmarkScope bench("BurstTrie construction");
                PROFILING_SCOPE(Global, 1, "BurstTrie construction");
                for (const StringSlice& word : input)
                    set.Insert_AssertUnique(word);
            }
            {
                const BenchmarkScope bench("BurstTrie optimization");
                PROFILING_SCOPE(Global, 1, "BurstTrie optimization");
                set.Optimize();
            }
            {
                const BenchmarkScope bench("BurstTrie search");
                PROFILING_SCOPE(Global, 2, "BurstTrie search");
                forrange(i, 0, loops) {
                    for (const StringSlice& word : search)
                        if (not set.Contains(word))
                            AssertNotReached();
                }
            }
        }
        {
            const BenchmarkScope bench("BulkTrie");

            BulkTrie<char, void, 8192, 31> set;
            {
                const BenchmarkScope bench("BulkTrie construction");
                PROFILING_SCOPE(Global, 3, "BulkTrie construction");
                for (const StringSlice& word : input)
                    set.Insert_AssertUnique(word);
            }

            {
                const BenchmarkScope bench("BulkTrie search");
                PROFILING_SCOPE(Global, 4, "BulkTrie search");
                forrange(i, 0, loops) {
                    for (const StringSlice& word : search)
                        if (not set.Contains(word))
                            AssertNotReached();
                    }
            }
        }*/
        {
            const BenchmarkScope bench("HashTable");

            typedef HashTable<
                StringSlice, void,
                StringSliceHasher<char, CaseSensitive::True>,
                StringSliceEqualTo<char, CaseSensitive::True>
            >   hashtable_type;


            hashtable_type set;
            {
                const BenchmarkScope bench("HashTable construction");
                PROFILING_SCOPE(Global, 3, "HashTable construction");
                set.reserve(input.size());
                size_t count = 0;
                for (const StringSlice& word : input) {
                    set.insert(word);
                    count++;
                    Assert(set.size() == count);
                }
            }

            HashTableStats stats = set.ProbingStats();
            LOG(Info,   L"[HASHTABLE] Probing stats =\n"
                        L"    Min : {0}\n"
                        L"    Max : {1}\n"
                        L"    Mean: {2}\n"
                        L"    Dev : {3}\n",
                stats.MinProbe, stats.MaxProbe, stats.MeanProbe, stats.DevProbe );

            {
                const BenchmarkScope bench("HashTable search");
                PROFILING_SCOPE(Global, 4, "HashTable search");
                forrange(i, 0, loops) {
                    for (const StringSlice& word : search)
                        if (set.end() == set.find(word))
                            AssertNotReached();
                    }
            }
        }
        {
            const BenchmarkScope bench("HashSet");

            STRINGSLICE_HASHSET(Container, CaseSensitive::True) set;
            {
                const BenchmarkScope bench("HashSet construction");
                PROFILING_SCOPE(Global, 3, "HashSet construction");
                set.reserve(input.size());
                for (const StringSlice& word : input)
                    set.insert(word);
            }

            {
                const BenchmarkScope bench("HashSet search");
                PROFILING_SCOPE(Global, 4, "HashSet search");
                forrange(i, 0, loops) {
                    for (const StringSlice& word : search)
                        if (set.end() == set.find(word))
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
