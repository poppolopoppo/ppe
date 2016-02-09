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

        STRINGTRIE_SET(Container, CaseSensitive::True, 17) set;

        std::random_shuffle(words.begin(), words.end());

        for (const String& word : words)
            set.Insert_AssertUnique(MakeStringSlice(word));

        std::random_shuffle(words.begin(), words.end());

        for (const String& word : words)
            if (not set.Contains(MakeStringSlice(word)))
                AssertNotReached();

        const auto* const node = set.Find(MakeStringSlice(words.front()));
        Assert(nullptr != node);
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