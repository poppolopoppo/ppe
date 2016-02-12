#include "stdafx.h"

#include "RobotApp.h"

#include "Core/Container/RawStorage.h"
#include "Core/Container/Vector.h"
#include "Core/IO/FileSystem.h"
#include "Core/IO/StringSlice.h"
#include "Core/IO/VirtualFileSystem.h"
#include "Core/Color/Color.h"
#include "Core/Maths/Maths.h"
#include "Core/Maths/RandomGenerator.h"
#include "Core/Memory/Compression.h"
#include "Core/Memory/MemoryStream.h"
#include "Core/Time/TimedScope.h"

#include "Core.Graphics/Device/DeviceAPI.h"
#include "Core.Graphics/Device/DeviceEncapsulator.h"
#include "Core.Graphics/Device/Texture/SurfaceFormat.h"

#include "Core.RTTI/RTTI.h"
#include "Core.RTTI/RTTIMacros.h"
#include "Core.RTTI/RTTIMacros-impl.h"
#include "Core.RTTI/MetaAtomVisitor.h"
#include "Core.RTTI/MetaTransaction.h"

#include "Core.Serialize/Binary/BinarySerializer.h"
#include "Core.Serialize/Text/TextSerializer.h"
#include "Core.Serialize/Text/Grammar.h"
#include "Core.Serialize/Lexer/Lexer.h"
#include "Core.Serialize/Parser/Parser.h"

#include "Core.Application/ApplicationConsole.h"
#include "Core/Container/BurstTrie.h"

#include "Core.RTTI/MetaType.Definitions-inl.h"

#include "Core/Diagnostic/Profiling.h"

#include <bitset>

namespace Core {
namespace ContentGenerator {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
FWD_REFPTR(ContentIdentity);
class ContentIdentity : public RTTI::MetaObject {
public:
    RTTI_CLASS_HEADER(ContentIdentity, RTTI::MetaObject);

    ContentIdentity() {}
    ContentIdentity(const Filename& sourceFile, const DateTime& lastModified)
        : _sourceFile(sourceFile), _lastModified(lastModified) {}

    const Filename& SourceFile() const { return _sourceFile; }
    const DateTime& LastModified() const { return _lastModified; }

private:
    Filename _sourceFile;
    DateTime _lastModified;
};
RTTI_CLASS_BEGIN(ContentIdentity, Concrete)
RTTI_PROPERTY_PRIVATE_FIELD(_sourceFile)
RTTI_PROPERTY_PRIVATE_FIELD(_lastModified)
RTTI_CLASS_END()
//----------------------------------------------------------------------------
FWD_REFPTR(RTTITest2_);
class RTTITest2_ : public RTTI::MetaObject {
public:
    RTTI_CLASS_HEADER(RTTITest2_, RTTI::MetaObject);

    RTTITest2_() {}

private:
    float3 _position;
    RTTI::Vector<RTTI::PMetaAtom> _atomVector;
};
RTTI_CLASS_BEGIN(RTTITest2_, Concrete)
    RTTI_PROPERTY_PRIVATE_FIELD(_position)
    RTTI_PROPERTY_PRIVATE_FIELD(_atomVector)
RTTI_CLASS_END()
//----------------------------------------------------------------------------
FWD_REFPTR(RTTITest_);
class RTTITest_ : public RTTI::MetaObject {
public:
    RTTI_CLASS_HEADER(RTTITest_, RTTI::MetaObject);

    RTTITest_() : _dummy(0)
#define DEF_METATYPE_SCALAR_IMPL_(_Name, T, _TypeId, _Unused) \
    ,   _ ## _Name(RTTI::MetaTypeTraits<T>::meta_type::DefaultValue()) \
    ,   _ ## _Name ## Pair(RTTI::MetaTypeTraits<T>::meta_type::DefaultValue(), RTTI::MetaTypeTraits<T>::meta_type::DefaultValue())
    FOREACH_CORE_RTTI_NATIVE_TYPES(DEF_METATYPE_SCALAR_IMPL_)
#undef DEF_METATYPE_SCALAR_IMPL_
    {}

    bool _dummy;

    template <typename T>
    using yolo_pair_type = Pair<T, T>;
    template <typename T>
    using yolo_dict_type = HASHMAP_THREAD_LOCAL(Container, T, T);
    template <typename T>
    using yolo_vect_type = VECTOR_THREAD_LOCAL(Container, yolo_dict_type<T>);
    /*template <typename T>
    using yolo_type = HASHMAP_THREAD_LOCAL(Container, yolo_pair_type<T>, yolo_vect_type<T>);*/

#define DEF_METATYPE_SCALAR_IMPL_(_Name, T, _TypeId, _Unused) \
    T _ ## _Name; \
    RTTI::Vector<T> _ ## _Name ## Vec; \
    RTTI::Pair<T, T> _ ## _Name ## Pair; \
    RTTI::Dictionary<T, T> _ ## _Name ## Dico; \
    //yolo_type<T> _ ## _Name ## Yolo;
    FOREACH_CORE_RTTI_NATIVE_TYPES(DEF_METATYPE_SCALAR_IMPL_)
#undef DEF_METATYPE_SCALAR_IMPL_
};
RTTI_CLASS_BEGIN(RTTITest_, Concrete)
#define DEF_METATYPE_SCALAR_IMPL_(_Name, T, _TypeId, _Unused) \
    RTTI_PROPERTY_PRIVATE_FIELD(_ ## _Name) \
    RTTI_PROPERTY_PRIVATE_FIELD(_ ## _Name ## Vec) \
    RTTI_PROPERTY_PRIVATE_FIELD(_ ## _Name ## Pair) \
    RTTI_PROPERTY_PRIVATE_FIELD(_ ## _Name ## Dico) \
    //RTTI_PROPERTY_PRIVATE_FIELD(_ ## _Name ## Yolo)
    FOREACH_CORE_RTTI_NATIVE_TYPES(DEF_METATYPE_SCALAR_IMPL_)
#undef DEF_METATYPE_SCALAR_IMPL_

RTTI_CLASS_END()
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
class RTTIAtomRandomizer_ : protected RTTI::MetaAtomWrapMoveVisitor {
    typedef RTTI::MetaAtomWrapMoveVisitor parent_type;
public:
    RTTIAtomRandomizer_(size_t maxDim, u64 seed) : _maxDim(maxDim), _depth(0), _rand(seed) {
        AssertRelease(_maxDim > 0);
    }

    void Randomize(RTTI::MetaObject* pobject);

protected:
    using parent_type::Inspect;
    using parent_type::Visit;

    virtual void Inspect(RTTI::IMetaAtomPair* ppair, RTTI::Pair<RTTI::PMetaAtom, RTTI::PMetaAtom>& pair) override {
        if (nullptr == pair.first)
            pair.first = ppair->FirstTraits()->CreateDefaultValue();

        if (nullptr == pair.second)
            pair.second = ppair->SecondTraits()->CreateDefaultValue();

        Assert(pair.first);
        Assert(pair.second);

        parent_type::Inspect(ppair, pair);
    }

    virtual void Inspect(RTTI::IMetaAtomVector* pvector, RTTI::Vector<RTTI::PMetaAtom>& vector) override {
        const RTTI::IMetaTypeVirtualTraits* traits = pvector->ValueTraits();
        const size_t count = NextRandomDim_();
        vector.reserve(count);
        forrange(i, 0, count)
            vector.emplace_back(traits->CreateDefaultValue());

        parent_type::Inspect(pvector, vector);
    }

    virtual void Inspect(RTTI::IMetaAtomDictionary* pdictionary, RTTI::Dictionary<RTTI::PMetaAtom, RTTI::PMetaAtom>& dictionary) override {
        const RTTI::IMetaTypeVirtualTraits* keyTraits = pdictionary->KeyTraits();
        const RTTI::IMetaTypeVirtualTraits* valueTraits = pdictionary->ValueTraits();
        const size_t count = NextRandomDim_();
        dictionary.reserve(count);
        forrange(i, 0, count)
            dictionary.Vector().emplace_back(
                keyTraits->CreateDefaultValue(),
                valueTraits->CreateDefaultValue() );

        parent_type::Inspect(pdictionary, dictionary);
    }

#define DEF_METATYPE_SCALAR(_Name, T, _TypeId, _Unused) \
    virtual void Visit(RTTI::MetaTypedAtom<T>* scalar) override { \
        Assert(_TypeId == scalar->TypeInfo().Id); \
        Randomize_(scalar->Wrapper()); \
        /*parent_type::Visit(scalar);*/ \
    }
    FOREACH_CORE_RTTI_NATIVE_TYPES(DEF_METATYPE_SCALAR)
#undef DEF_METATYPE_SCALAR

private:
    const size_t _maxDim;
    size_t _depth;
    RandomGenerator _rand;

    size_t NextRandomDim_() { return (_rand.Next() % _maxDim); }

    template <typename T>
    void Randomize_(T& value) {
        _rand.Randomize(value);
    }

    template <typename T, size_t _Width, size_t _Height>
    void Randomize_(ScalarMatrix<T, _Width, _Height>& value) {
        ScalarMatrixData<T, _Width, _Height>& data = value.data();
        const size_t dim = _Width*_Height;
        for (size_t i = 0; i < dim; ++i)
            _rand.Randomize(data.raw[i]);
    }

    void Randomize_(String& str) {
        const size_t count = NextRandomDim_();
        str.resize(count);
        forrange(i, 0, count)
            str[i] = (char)(_rand.Next('a'+0, 'z'+1));
    }

    void Randomize_(WString& wstr) {
        const size_t count = NextRandomDim_();
        wstr.resize(count);
        forrange(i, 0, count)
            wstr[i] = (wchar_t)(_rand.Next(L'a'+0, L'z'+1));
    }

    void Randomize_(RTTI::BinaryData& rawdata) {
        const size_t count = NextRandomDim_()*10;
        rawdata.Resize_DiscardData(count);
        forrange(i, 0, count)
            rawdata[i] = u8(_rand.Next());
    }

    void Randomize_(Core::RTTI::PMetaAtom& atom) {
        const float f = _rand.NextFloatM11();
        if (f < -0.5f)
            atom = RTTI::MakeAtom(_rand.NextU64());
        else if (f < 0.0f)
            atom = RTTI::MakeAtom(_rand.NextFloatM11());
        else if (f < 0.5f)
            atom = RTTI::MakeAtom(float3(_rand.NextFloat01(), _rand.NextFloat01(), _rand.NextFloat01()));
        else
            atom = RTTI::MakeAtom(double(f));
        Assert(atom);
        atom = RTTI::MakeAtom(atom);
    }

    void Randomize_(Core::RTTI::PMetaObject& object) {
        const float d = ((float)_depth)/_maxDim;
        const float r = _rand.NextFloat01() - d;
        if (r - 0.5f < 0)
            return;
        if (nullptr == object)
            object = new RTTITest_();
        Randomize(object.get());
    }
};
//----------------------------------------------------------------------------
void RTTIAtomRandomizer_::Randomize(RTTI::MetaObject* pobject) {
    Assert(pobject);

    ++_depth;

    for (const RTTI::UCMetaProperty& prop : pobject->RTTI_MetaClass()->Properties()) {
        RTTI::PMetaAtom atom = prop->WrapMove(pobject);
        AssertRelease(nullptr != atom);
        parent_type::Append(atom.get());
        prop->MoveFrom(pobject, atom.get());
    }

    Assert(0 < _depth);
    --_depth;
}
//----------------------------------------------------------------------------
} //!namespace
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
,   typename _Hasher = Hash<_Key>
,   typename _Allocator = ALLOCATOR(Container, BulkTrieNode<_Key COMMA _Value COMMA _BulkSize>)
>   class BulkTrie : _Allocator {
public:
    typedef _Key key_type;
    typedef _Value value_type;
    typedef _Less less_functor;
    typedef _Equal equal_to_functor;
    typedef _Hasher hash_functor;
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
        return (hash_functor()(keys.front()) % BucketCount);
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
RobotApp::RobotApp()
: parent_type(
    L"RobotApp", 100, 100,
    Graphics::PresentationParameters(
        640, 480,
        Graphics::SurfaceFormatType::B8G8R8A8_SRGB,
        Graphics::SurfaceFormatType::D24S8,
        false,
        false,
        0,
        Graphics::PresentInterval::Default ),
    Graphics::DeviceAPI::DirectX11,
    true ) {

    typedef RTTITest_ test_type;
    //typedef RTTITest2_ test_type;
    static const size_t test_count = 4;

    Application::ApplicationConsole::RedirectIOToConsole();

    ContentIdentity::MetaClass::Create();
    test_type::MetaClass::Create();
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
                const StringSlice line(buffer, len);
                const StringSlice word = Chomp(line);
                words.emplace_back(ToString(word));
            }
        }

        VECTOR_THREAD_LOCAL(Container, String) shuffled(words);
        std::random_shuffle(shuffled.begin(), shuffled.end());

        {
            const BenchmarkScope bench("BurstTrie");

            STRINGTRIE_SET(Container, CaseSensitive::True, 13) set;
            {
                PROFILING_SCOPE(Global, 4, "BurstTrie construction");
                for (const String& word : shuffled)
                    set.Insert_AssertUnique(MakeStringSlice(word));
            }

            {
                PROFILING_SCOPE(Global, 4, "BurstTrie search");
                forrange(i, 0, 200) {
                    for (const String& word : words)
                        if (not set.Contains(MakeStringSlice(word)))
                            AssertNotReached();
                }
            }
        }
        {
            const BenchmarkScope bench("BulkTrie");

            BulkTrie<char, void, 8192, 13> set;
            {
                PROFILING_SCOPE(Global, 3, "BulkTrie construction");
                for (const String& word : shuffled)
                    set.Insert_AssertUnique(MakeStringSlice(word));
            }

            {
                PROFILING_SCOPE(Global, 4, "BulkTrie search");
                forrange(i, 0, 200) {
                    for (const String& word : words)
                        if (not set.Contains(MakeStringSlice(word)))
                            AssertNotReached();
                    }
            }
        }
    }
    {
        const Filename filename = L"Tmp:/robotapp.bin";
        const Filename filename2 = L"Tmp:/robotapp.raw";

        RTTI::MetaTransaction input;
        {
            RTTIAtomRandomizer_ rand(4, 0xabadcafedeadbeefull);
            forrange(i, 0, test_count) {
                RefPtr<test_type> t = new test_type();
                rand.Randomize(t.get());
                input.Add(t.get());
            }
        }

        {

            Serialize::TextSerializer s;
            auto oss = StdoutWriter();
            s.Serialize(&oss, &input);
        }

        RTTI::MetaTransaction transaction;
        Serialize::BinarySerializer serializer;

        MEMORYSTREAM_THREAD_LOCAL(Serialize) uncompressed;
        {
            serializer.Serialize(&uncompressed, &input);
#if 0
            auto compressed = VFS_OpenBinaryWritable(filename, AccessPolicy::Truncate);
            LZJB::CompressMemory(compressed.get(), uncompressed.MakeView());
#else
            RAWSTORAGE_THREAD_LOCAL(Serialize, u8) compressed;
            const size_t compressedSizeInBytes = Compression::CompressMemory(compressed, uncompressed.MakeView(), Compression::HighCompression);
            Assert(compressedSizeInBytes <= compressed.SizeInBytes());
            const MemoryView<const u8> compressedView = compressed.MakeView().SubRange(0, compressedSizeInBytes);

            RAWSTORAGE_THREAD_LOCAL(Stream, u8) decompressed;
            if (false == Compression::DecompressMemory(decompressed, compressedView))
                AssertNotReached();

            Assert(uncompressed.SizeInBytes() == decompressed.SizeInBytes());
            const size_t k = decompressed.SizeInBytes();
            for (size_t i = 0; i < k; ++i)
                Assert(uncompressed.Pointer()[i] == decompressed.Pointer()[i]);

            if (false == VFS_WriteAll(filename, compressedView, AccessPolicy::Truncate_Binary))
                AssertNotReached();
            if (false == VFS_WriteAll(filename2, decompressed.MakeView(), AccessPolicy::Truncate_Binary))
                AssertNotReached();

            RAWSTORAGE_THREAD_LOCAL(FileSystem, u8) stored;
            if (false == VFS_ReadAll(&stored, filename, AccessPolicy::Binary))
                AssertNotReached();

            Assert(stored.SizeInBytes() == compressedSizeInBytes);
            const size_t n = compressedSizeInBytes;
            for (size_t i = 0; i < n; ++i)
                Assert(stored.Pointer()[i] == compressedView.Pointer()[i]);
#endif
        }

        RTTI::MetaTransaction output;

        {
            RAWSTORAGE_THREAD_LOCAL(FileSystem, u8) compressed;
            if (false == VFS_ReadAll(&compressed, filename, AccessPolicy::Binary))
                AssertNotReached();

            RAWSTORAGE_THREAD_LOCAL(Stream, u8) decompressed;
            if (false == Compression::DecompressMemory(decompressed, compressed.MakeConstView()))
                AssertNotReached();;

            Assert(uncompressed.SizeInBytes() == decompressed.SizeInBytes());
            const size_t k = decompressed.SizeInBytes();
            for (size_t i = 0; i < k; ++i)
                Assert(uncompressed.Pointer()[i] == decompressed.Pointer()[i]);

            serializer.Deserialize(&output, decompressed.MakeConstView());
        }

        AssertRelease(input.size() == output.size());
        if (false == input.DeepEquals(output))
            AssertNotReached();
    }
    {
        Parser::ParseContext globalContext;
        do
        {
            std::cout << "$ ";

            char line[1024];
            std::cin.getline(line, lengthof(line));

            if (0 == CompareNI("exit", line, 5))
                break;

            try {
                Lexer::Lexer lexer(StringSlice(&line[0], Length(line)), L"@in_memory");
                Parser::ParseList input(&lexer);

                Parser::PCParseItem item = Serialize::GrammarStartup::Parse(input);
                AssertRelease(item);

                item->Invoke(&globalContext);
            }
            catch (const Parser::ParserException& e) {
                if (e.Item())
                    Format(std::cerr, "parser error : <{0}> {1}, {2}.\n", e.Item()->ToString(), e.what(), e.Site());
                else
                    Format(std::cerr, "parser error : {0}, {1}.\n", e.what(), e.Site());
            }
            catch (const Lexer::LexerException& e) {
                Format(std::cerr, "lexer error : <{0}>: {1}, {2}.\n", e.Match().Symbol()->CStr(), e.what(), e.Match().Site());
            }
        } while (true);
    }
    test_type::MetaClass::Destroy();
    ContentIdentity::MetaClass::Destroy();

    Serialize::SerializeStartup::ClearAll_UnusedMemory();
    RTTI::RTTIStartup::ClearAll_UnusedMemory();
    Core::CoreStartup::ClearAll_UnusedMemory();
}
//----------------------------------------------------------------------------
void RobotApp::Start() {
    parent_type::Start();

    RenderLoop();
}
//----------------------------------------------------------------------------
void RobotApp::Draw(const Timeline& time) {
    parent_type::Draw(time);

    const double totalSeconds = Units::Time::Seconds(time.Total()).Value();
    const float t = float(Frac(totalSeconds*0.1));
    const float3 hsv(t, 1.0f, 0.5f);
    const float3 rgb = HSV_to_RGB(hsv);
    const ColorRGBAF clearColor(rgb, 1.0f);

    Graphics::IDeviceAPIEncapsulator* const device = DeviceEncapsulator().Device();
    device->Clear(device->BackBufferRenderTarget(), clearColor);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentGenerator
} //!namespace Core