#include "stdafx.h"

#include "Core/Container/RawStorage.h"
#include "Core/Container/Vector.h"
#include "Core/IO/FileSystem.h"
#include "Core/IO/StringSlice.h"
#include "Core/IO/VirtualFileSystem.h"
#include "Core/Maths/Maths.h"
#include "Core/Maths/RandomGenerator.h"
#include "Core/Memory/Compression.h"
#include "Core/Memory/MemoryStream.h"

#include "Core.RTTI/RTTI.h"
#include "Core.RTTI/RTTIMacros.h"
#include "Core.RTTI/RTTIMacros-impl.h"
#include "Core.RTTI/RTTI_Tag.h"
#include "Core.RTTI/RTTI_Tag-impl.h"
#include "Core.RTTI/MetaAtomVisitor.h"
#include "Core.RTTI/MetaTransaction.h"
#include "Core.RTTI/MetaType.Definitions-inl.h"

#include "Core.Serialize/Binary/BinarySerializer.h"
#include "Core.Serialize/Text/TextSerializer.h"
#include "Core.Serialize/Text/Grammar.h"
#include "Core.Serialize/Lexer/Lexer.h"
#include "Core.Serialize/Parser/Parser.h"

namespace Core {
namespace ContentGenerator {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
RTTI_TAG_DECL(Test);
RTTI_TAG_DEF(Test);
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
RTTI_CLASS_BEGIN(Test, ContentIdentity, Concrete)
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
RTTI_CLASS_BEGIN(Test, RTTITest2_, Concrete)
    RTTI_PROPERTY_PRIVATE_FIELD(_position)
    RTTI_PROPERTY_PRIVATE_FIELD(_atomVector)
RTTI_CLASS_END()
//----------------------------------------------------------------------------
FWD_REFPTR(RTTITestParent_);
class RTTITestParent_ : public RTTI::MetaObject {
public:
    RTTI_CLASS_HEADER(RTTITestParent_, RTTI::MetaObject);

    RTTITestParent_() {}

private:
    PContentIdentity _identity;
    WString _sourceName;
};
RTTI_CLASS_BEGIN(Test, RTTITestParent_, Concrete)
    RTTI_PROPERTY_PRIVATE_FIELD(_identity)
    RTTI_PROPERTY_PRIVATE_FIELD(_sourceName)
RTTI_CLASS_END()
//----------------------------------------------------------------------------
FWD_REFPTR(RTTITest_);
class RTTITest_ : public RTTITestParent_ {
public:
    RTTI_CLASS_HEADER(RTTITest_, RTTITestParent_);

    RTTITest_()
    :   _dummy(0)
    ,   _half(0.0f)
    ,   _UX10Y10Z10W2N(0)
#define DEF_METATYPE_SCALAR_IMPL_(_Name, T, _TypeId, _Unused) \
    ,   _ ## _Name ## Scalar(RTTI::MetaTypeTraits<T>::meta_type::DefaultValue()) \
    ,   _ ## _Name ## Pair(RTTI::MetaTypeTraits<T>::meta_type::DefaultValue(), RTTI::MetaTypeTraits<T>::meta_type::DefaultValue())
    FOREACH_CORE_RTTI_NATIVE_TYPES(DEF_METATYPE_SCALAR_IMPL_)
#undef DEF_METATYPE_SCALAR_IMPL_
    {}

    bool _dummy;

    half _half;
    byte4n _byte4n;
    ushort2n _ushort2n;
    half2 _half2;
    UX10Y10Z10W2N _UX10Y10Z10W2N;

    template <typename T>
    using yolo_pair_type = Pair<T, T>;
    template <typename T>
    using yolo_dict_type = HASHMAP_THREAD_LOCAL(Container, T, T);
    template <typename T>
    using yolo_vect_type = VECTOR_THREAD_LOCAL(Container, yolo_dict_type<T>);
    /*template <typename T>
    using yolo_type = HASHMAP_THREAD_LOCAL(Container, yolo_pair_type<T>, yolo_vect_type<T>);*/

#define DEF_METATYPE_SCALAR_IMPL_(_Name, T, _TypeId, _Unused) \
    T _ ## _Name ## Scalar; \
    RTTI::Vector<T> _ ## _Name ## Vec; \
    RTTI::Pair<T, T> _ ## _Name ## Pair; \
    RTTI::Dictionary<T, T> _ ## _Name ## Dico; \
    //yolo_type<T> _ ## _Name ## Yolo;
    FOREACH_CORE_RTTI_NATIVE_TYPES(DEF_METATYPE_SCALAR_IMPL_)
#undef DEF_METATYPE_SCALAR_IMPL_
};
RTTI_CLASS_BEGIN(Test, RTTITest_, Concrete)
    RTTI_PROPERTY_PRIVATE_FIELD(_dummy)
    RTTI_PROPERTY_PRIVATE_FIELD(_half)
    RTTI_PROPERTY_PRIVATE_FIELD(_byte4n)
    RTTI_PROPERTY_PRIVATE_FIELD(_ushort2n)
    RTTI_PROPERTY_PRIVATE_FIELD(_half2)
    //RTTI_PROPERTY_PRIVATE_FIELD(_UX10Y10Z10W2N)
#define DEF_METATYPE_SCALAR_IMPL_(_Name, T, _TypeId, _Unused) \
    RTTI_PROPERTY_PRIVATE_FIELD(_ ## _Name ## Scalar) \
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
        RandomizeDispatch_(scalar); \
        /*parent_type::Visit(scalar);*/ \
    }
    FOREACH_CORE_RTTI_NATIVE_TYPES(DEF_METATYPE_SCALAR)
#undef DEF_METATYPE_SCALAR

private:
    const size_t _maxDim;
    size_t _depth;
    RandomGenerator _rand;

    template <typename T>
    void RandomizeDispatch_(RTTI::MetaTypedAtom<T>* scalar) {
        Randomize_(scalar->Wrapper());
    }

    void RandomizeDispatch_(RTTI::MetaTypedAtom<RTTI::PMetaObject>* scalar) {
        UNUSED(scalar);
        // can't do anything since we don't know the metaclass, this is no trivial task ...
        /*
        const float d = ((float)_depth)/_maxDim;
        const float r = _rand.NextFloat01() - d;
        if (r - 0.4f < 0)
            return;
        RTTI::PMetaObject& object = scalar->Wrapper();
        if (nullptr == object) {
            const RTTI::MetaClass* metaClass = scalar->Traits()->GetMetaClassIFP();
            Assert(metaClass);
            object = metaClass->CreateInstance();
        }
        Randomize(object.get());
        */
    }

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

    void Randomize_(RTTI::Name& name) {
        const size_t count = NextRandomDim_();
        STACKLOCAL_POD_ARRAY(RTTI::Name::char_type, cstr, count);
        forrange(i, 0, count)
            cstr[i] = (char)(_rand.Next('a'+0, 'z'+1));
        name = RTTI::Name(MakeStringSlice(cstr));
    }

    void Randomize_(RTTI::BinaryData& rawdata) {
        const size_t count = NextRandomDim_()*10;
        rawdata.Resize_DiscardData(count);
        forrange(i, 0, count)
            rawdata[i] = u8(_rand.Next());
    }

    void Randomize_(RTTI::OpaqueData& opaqueData) {
        const size_t count = NextRandomDim_()*10;
        opaqueData.reserve(count);
        forrange(i, 0, count) {
            RTTI::Name key;
            Randomize_(key);

            RTTI::PMetaAtom value;
            Randomize_(value);

            opaqueData.Insert_KeepOldIFN(key, value);
        }
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
};
//----------------------------------------------------------------------------
void RTTIAtomRandomizer_::Randomize(RTTI::MetaObject* pobject) {
    Assert(pobject);

    ++_depth;

    const RTTI::MetaClass* metaClass = pobject->RTTI_MetaClass();

    while (metaClass) {
        for (const RTTI::UCMetaProperty& prop : metaClass->Properties()) {
            RTTI::PMetaAtom atom = prop->WrapMove(pobject);
            AssertRelease(nullptr != atom);
            parent_type::Append(atom.get());
            prop->MoveFrom(pobject, atom.get());
        }

        metaClass = metaClass->Parent();
    }

    Assert(0 < _depth);
    --_depth;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void Test_RTTI() {
    typedef RTTITest_ test_type;
    //typedef RTTITest2_ test_type;
    static const size_t test_count = 4;

    RTTI_TAG(Test)::Start();
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

            Assert(checked_cast<size_t>(uncompressed.SizeInBytes()) == decompressed.SizeInBytes());
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

            Assert(checked_cast<size_t>(uncompressed.SizeInBytes()) == decompressed.SizeInBytes());
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

            char buffer[1024];
            std::cin.getline(buffer, lengthof(buffer));

            const StringSlice line = MakeStringSlice(buffer, Meta::noinit_tag());

            if (0 == CompareI(MakeStringSlice("exit"), line))
                break;

            const wchar_t filename[] = L"@in_memory";

            try {
                Lexer::Lexer lexer(line, filename, true);
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
    RTTI_TAG(Test)::Shutdown();

    Serialize::SerializeStartup::ClearAll_UnusedMemory();
    RTTI::RTTIStartup::ClearAll_UnusedMemory();
    Core::CoreStartup::ClearAll_UnusedMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentGenerator
} //!namespace Core
