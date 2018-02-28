#include "stdafx.h"

#include "Core.RTTI/Any.h"
#include "Core.RTTI/Atom.h"
#include "Core.RTTI/AtomVisitor.h"
#include "Core.RTTI/MetaTransaction.h"
#include "Core.RTTI/NativeTypes.h"
#include "Core.RTTI/TypeInfos.h"


#include "Core.RTTI/RTTI_Macros.h"
#include "Core.RTTI/RTTI_Macros-impl.h"
#include "Core.RTTI/RTTI_Namespace.h"
#include "Core.RTTI/RTTI_Namespace-impl.h"

#include "Core.Serialize/Binary/BinarySerializer.h"
#include "Core.Serialize/Json/Json.h"
#include "Core.Serialize/Json/JsonSerializer.h"

#include "Core/Container/AssociativeVector.h"
#include "Core/Container/HashMap.h"
#include "Core/Container/Pair.h"
#include "Core/Container/Vector.h"
#include "Core/Diagnostic/Logger.h"
#include "Core/IO/FileStream.h"
#include "Core/IO/Format.h"
#include "Core/IO/FS/ConstNames.h"
#include "Core/IO/FS/Dirpath.h"
#include "Core/IO/FS/Filename.h"
#include "Core/IO/StringBuilder.h"
#include "Core/IO/StringView.h"
#include "Core/IO/TextWriter.h"
#include "Core/IO/VirtualFileSystem.h"
#include "Core/Maths/RandomGenerator.h"
#include "Core/Maths/ScalarMatrix.h"
#include "Core/Maths/ScalarVector.h"

#include "Core/Memory/Compression.h"
#include "Core/Memory/MemoryStream.h"

#define USE_RTTI_TEST_YOLOTYPES 0

namespace Core {
namespace Test {
LOG_CATEGORY(, Test_RTTI)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
RTTI_NAMESPACE_DECL(, RTTI_UnitTest);
RTTI_NAMESPACE_DEF(, RTTI_UnitTest);
//----------------------------------------------------------------------------
FWD_REFPTR(RTTITestParent_);
class FRTTITestParent_ : public RTTI::FMetaObject {
public:
    enum class ETestEnum32 : u32 {
        A = 0, B, C
    };

    enum class ETestEnum64 : u64 {
        A = 1<<0, B=1<<1, C = 1<<2
    };

    RTTI_CLASS_HEADER(FRTTITestParent_, RTTI::FMetaObject);

    FRTTITestParent_() {}

    const ETestEnum32& Enum32() const { return _testEnum32; }
    void SetEnum32(const ETestEnum32& value) { _testEnum32 = value; }

    const ETestEnum64& Enum64() const { return _testEnum64; }
    void SetEnum64(const ETestEnum64& value) { _testEnum64 = value; }

private:
    FFilename _sourceName;
    ETestEnum32 _testEnum32;
    ETestEnum64 _testEnum64;
    PRTTITestParent_ _child;
};
RTTI_CLASS_BEGIN(RTTI_UnitTest, FRTTITestParent_, RTTI::EClassFlags::Concrete)
    RTTI_PROPERTY_PRIVATE_FIELD(_sourceName)
    RTTI_PROPERTY_PRIVATE_FIELD(_testEnum32)
    RTTI_PROPERTY_PRIVATE_FIELD(_testEnum64)
    RTTI_PROPERTY_PRIVATE_FIELD(_child)
RTTI_CLASS_END()
//----------------------------------------------------------------------------
FWD_REFPTR(RTTITest_);
class FRTTITest_ : public FRTTITestParent_ {
public:
    RTTI_CLASS_HEADER(FRTTITest_, FRTTITestParent_);

    FRTTITest_()
    :   _dummy(0)
    {}

    bool _dummy;

#if USE_RTTI_TEST_YOLOTYPES
    template <typename T>
    using yolo_pair_type = TPair<T, T>;
    template <typename T>
    using yolo_dict_type = HASHMAP_THREAD_LOCAL(Container, T, T);
    template <typename T>
    using yolo_vect_type = VECTOR_THREAD_LOCAL(Container, yolo_dict_type<T>);
    template <typename T>
    using yolo_type = HASHMAP_THREAD_LOCAL(Container, yolo_pair_type<T>, yolo_vect_type<T>);

#define DEF_METATYPE_SCALAR_IMPL_(_Name, T, _TypeId) \
    T _ ## _Name ## Scalar; \
    TVector<T> _ ## _Name ## Vec; \
    TPair<T, T> _ ## _Name ## TPair; \
    TAssociativeVector<T, T> _ ## _Name ## Dico; \
    yolo_type<T> _ ## _Name ## Yolo;*
#else
#define DEF_METATYPE_SCALAR_IMPL_(_Name, T, _TypeId) \
    T _ ## _Name ## Scalar; \
    TVector<T> _ ## _Name ## Vec; \
    TPair<T, T> _ ## _Name ## TPair; \
    TAssociativeVector<T, T> _ ## _Name ## Dico;
#endif
    FOREACH_RTTI_NATIVETYPES(DEF_METATYPE_SCALAR_IMPL_)
#undef DEF_METATYPE_SCALAR_IMPL_
};
RTTI_CLASS_BEGIN(RTTI_UnitTest, FRTTITest_, RTTI::EClassFlags::Concrete)
    RTTI_PROPERTY_PRIVATE_FIELD(_dummy)
#if USE_RTTI_TEST_YOLOTYPES
#define DEF_METATYPE_SCALAR_IMPL_(_Name, T, _TypeId) \
    RTTI_PROPERTY_PRIVATE_FIELD(_ ## _Name ## Scalar) \
    RTTI_PROPERTY_PRIVATE_FIELD(_ ## _Name ## Vec) \
    RTTI_PROPERTY_PRIVATE_FIELD(_ ## _Name ## TPair) \
    RTTI_PROPERTY_PRIVATE_FIELD(_ ## _Name ## Dico) \
    RTTI_PROPERTY_PRIVATE_FIELD(_ ## _Name ## Yolo)
    FOREACH_RTTI_NATIVETYPES(DEF_METATYPE_SCALAR_IMPL_)
#else
#define DEF_METATYPE_SCALAR_IMPL_(_Name, T, _TypeId) \
    RTTI_PROPERTY_PRIVATE_FIELD(_ ## _Name ## Scalar) \
    RTTI_PROPERTY_PRIVATE_FIELD(_ ## _Name ## Vec) \
    RTTI_PROPERTY_PRIVATE_FIELD(_ ## _Name ## TPair) \
    RTTI_PROPERTY_PRIVATE_FIELD(_ ## _Name ## Dico)
    FOREACH_RTTI_NATIVETYPES(DEF_METATYPE_SCALAR_IMPL_)
#endif
#undef DEF_METATYPE_SCALAR_IMPL_
RTTI_CLASS_END()
//----------------------------------------------------------------------------
class FRTTIAtomRandomizer_ : protected RTTI::FBaseAtomVisitor {
    typedef RTTI::FBaseAtomVisitor parent_type;
public:
    FRTTIAtomRandomizer_(size_t maxDim, u64 seed) : _maxDim(maxDim), _depth(0), _rand(seed) {
        AssertRelease(_maxDim > 0);
    }

    void Randomize(const RTTI::FAtom& atom) {
        atom.Accept(this);
    }

    void Randomize(RTTI::FMetaObject* pobject);

protected:
    virtual bool Visit(const RTTI::IPairTraits* pair, const RTTI::FAtom& atom) override final {
        return parent_type::Visit(pair, atom);
    }

    virtual bool Visit(const RTTI::IListTraits* list, const RTTI::FAtom& atom) override final {
        const size_t count = NextRandomDim_();
        list->Reserve(atom, count);
        forrange(i, 0, count)
            list->AddDefault(atom);

        return parent_type::Visit(list, atom);
    }

    virtual bool Visit(const RTTI::IDicoTraits* dico, const RTTI::FAtom& atom) override final {
        RTTI::FAny anyKey;
        RTTI::FAtom keyAtom = anyKey.Reset(dico->KeyTraits());

        const size_t count = NextRandomDim_();
        dico->Reserve(atom, count);
        forrange(i, 0, count) {
            if (not keyAtom.Accept(this))
                return false;

            if (dico->Find(atom, keyAtom))
                continue;

            if (not dico->AddDefault(atom, std::move(keyAtom)).Accept(this))
                return false;
        }

        return true;
    }

#define DEF_METATYPE_SCALAR(_Name, T, _TypeId) \
    virtual bool Visit(const RTTI::IScalarTraits* scalar, T& value) override final { \
        Randomize_(value); \
        return true; \
    }
    FOREACH_RTTI_NATIVETYPES(DEF_METATYPE_SCALAR)
#undef DEF_METATYPE_SCALAR

private:
    const size_t _maxDim;
    size_t _depth;
    FRandomGenerator _rand;

    size_t NextRandomDim_() { return (_rand.Next() % _maxDim); }

    template <typename T>
    void Randomize_(T& value) {
        _rand.Randomize(value);
    }

    template <typename T, size_t _Width, size_t _Height>
    void Randomize_(TScalarMatrix<T, _Width, _Height>& value) {
        ScalarMatrixData<T, _Width, _Height>& data = value.data();
        const size_t dim = _Width*_Height;
        for (size_t i = 0; i < dim; ++i)
            _rand.Randomize(data.raw[i]);
    }

    void Randomize_(FString& str) {
        const size_t count = NextRandomDim_();
        str.resize(count);
        forrange(i, 0, count)
            str[i] = (char)(_rand.Next('a'+0, 'z'+1));
    }

    void Randomize_(FWString& wstr) {
        FString str;
        Randomize_(str);
        wstr = ToWString(str);
    }

    void Randomize_(RTTI::FName& name) {
        const size_t count = NextRandomDim_();
        STACKLOCAL_POD_ARRAY(RTTI::FName::char_type, cstr, count);
        forrange(i, 0, count)
            cstr[i] = (char)(_rand.Next('a'+0, 'z'+1));
        name = RTTI::FName(MakeStringView(cstr));
    }

    void Randomize_(RTTI::FBinaryData& rawdata) {
        const size_t count = NextRandomDim_()*10;
        rawdata.Resize_DiscardData(count);
        forrange(i, 0, count)
            rawdata[i] = u8(_rand.Next());
    }

    void Randomize_(RTTI::FAny& any) {
        RTTI::ENativeType typeId = RTTI::ENativeType(_rand.Next(1, size_t(RTTI::ENativeType::Any)));
        any = RTTI::FAny(typeId);
        if (not any.InnerAtom().Accept(this))
            AssertNotReached();
    }

    void Randomize_(FDirpath&) {}
    void Randomize_(FFilename& ) {}

    void Randomize_(RTTI::PMetaObject& obj) {
        UNUSED(obj);
        // can't do anything since we don't know the metaclass, this is no trivial task ...
        /*
        const float d = ((float)_depth)/_maxDim;
        const float r = _rand.NextFloat01() - d;
        if (r - 0.4f < 0)
        return;
        RTTI::PMetaObject& object = scalar->Wrapper();
        if (nullptr == object) {
        const RTTI::TMetaClass* metaClass = scalar->Traits()->GetMetaClassIFP();
        Assert(metaClass);
        object = metaClass->CreateInstance();
        }
        Randomize(object.get());
        */
    }

};
//----------------------------------------------------------------------------
void FRTTIAtomRandomizer_::Randomize(RTTI::FMetaObject* pobject) {
    Assert(pobject);

    ++_depth;

    const RTTI::FMetaClass* metaClass = pobject->RTTI_Class();

    for (const auto* prop : metaClass->AllProperties()) {
        RTTI::FAtom atom = prop->Get(*pobject);
        AssertRelease(atom);
        atom.Accept(this);
    }

    Assert(0 < _depth);
    --_depth;
}
//----------------------------------------------------------------------------
static void print_atom(const RTTI::FAtom& atom) {
    const RTTI::FTypeInfos type_info = atom.TypeInfos();
    LOG(Test_RTTI, Info, L"{0}[0x{1:#8x}] : {2} = {3} (default:{4:a})",
        type_info.Name(),
        type_info.Id(),
        atom.HashValue(),
        atom,
        atom.IsDefaultValue());
}
//----------------------------------------------------------------------------
static NO_INLINE void Test_Atoms_() {
    int i = 0;
    RTTI::FAtom iatom = RTTI::MakeAtom(&i);
    print_atom(iatom);
    i = 1;
    print_atom(iatom);

    TPair<FString, double> p = { "toto", 42.69 };
    RTTI::FAtom patom = RTTI::MakeAtom(&p);
    print_atom(patom);

    TVector<float2> v = { float2(0), float2(1) };
    RTTI::FAtom vatom = RTTI::MakeAtom(&v);
    print_atom(vatom);

    VECTOR(RTTI, float2) v2;
    RTTI::FAtom vatom2 = RTTI::MakeAtom(&v2);

    if (not vatom.PromoteMove(vatom2))
        AssertNotReached();

    print_atom(vatom);
    print_atom(vatom2);

    TAssociativeVector<RTTI::FName, bool> d;
    d.Add(RTTI::FName("Toto")) = true;
    d.Add(RTTI::FName("Split")) = false;
    RTTI::FAtom datom = RTTI::MakeAtom(&d);
    print_atom(datom);

    TAssociativeVector<FString, TVector<word4> > t;
    t.Add("Toto").assign({ word4(0), word4(1), word4(2) });
    t.Add("Split").assign({ word4(3) });
    t.Add("Tricky").assign({ word4(4), word4(5) });
    RTTI::FAtom tatom = RTTI::MakeAtom(&t);
    print_atom(tatom);

    TAssociativeVector<FString, TVector<word4> > t2;
    RTTI::FAtom tatom2 = RTTI::MakeAtom(&t2);

    if (not tatom.PromoteCopy(tatom2))
        AssertNotReached();

    print_atom(tatom);
    print_atom(tatom2);

    if (not tatom2.PromoteMove(tatom))
        AssertNotReached();

    tatom.Move(tatom2);

    print_atom(tatom);
    print_atom(tatom2);
}
//----------------------------------------------------------------------------
static NO_INLINE void Test_Any_() {
    STATIC_ASSERT(RTTI::TIsSupportedType<decltype(42)>::value);
    STATIC_ASSERT(RTTI::TIsSupportedType<decltype(42.)>::value);
    STATIC_ASSERT(RTTI::TIsSupportedType<FString>::value);

    RTTI::FAny any;
    any.Assign(42);
    RTTI::FAny anyInt = RTTI::MakeAny(42);
    LOG(Test_RTTI, Debug, L"anyInt: {0} -> {1}", anyInt, RTTI::CastChecked<int>(anyInt));
    RTTI::FAny anyAny = RTTI::MakeAny(42.);
    LOG(Test_RTTI, Debug, L"anyAny: {0} -> {1}", anyAny, RTTI::CastChecked<double>(anyAny));
    Assert(anyAny != anyInt);
    anyAny = anyInt;
    LOG(Test_RTTI, Debug, L"anyAny: {0} -> {1}", anyAny, RTTI::CastChecked<int>(anyAny));
    Assert(anyInt == anyAny);
    anyAny = RTTI::MakeAny(FString("toto"));
    LOG(Test_RTTI, Debug, L"anyAny: {0} -> {1}", anyAny, RTTI::CastChecked<FString>(anyAny));
    Assert(anyAny != anyInt);
    Assert(nullptr == RTTI::Cast<double>(anyAny));

    TAssociativeVector<RTTI::FName, RTTI::FAny> d2;
    d2.Add(RTTI::FName("Toto")) = RTTI::MakeAny(float3(1, 2, 3));
    d2.Add(RTTI::FName("Split")) = RTTI::MakeAny(FWString(L"Toto"));
    RTTI::FAtom datom2 = RTTI::MakeAtom(&d2);
    print_atom(datom2);

    RTTI::FName toto("Toto");
    THashMap<RTTI::FAny, RTTI::FAny> d3;
    d3.Add(anyInt) = RTTI::MakeAny(69);
    d3.Add(MakeAny(toto)) = RTTI::MakeAny(float3(1, 2, 3));
    d3.Add(MakeAny(RTTI::FName("Split"))) = RTTI::MakeAny(FWString(L"Toto"));
    RTTI::FAtom datom3 = RTTI::MakeAtom(&d3);
    print_atom(datom3);

    const RTTI::IDicoTraits* dico2 = datom2.Traits()->AsDico();
    Assert(dico2);
    anyAny.Assign(toto);
    RTTI::FAtom dico2_anyAny = dico2->Find(datom2, anyAny);
    LOG(Test_RTTI, Debug, L"dico2_anyAny: {0}", dico2_anyAny.TypedConstData<float3>());
    AssertRelease(RTTI::MakeAny(float3(1, 2, 3)).InnerAtom().Equals(dico2_anyAny));
    //AssertRelease(dico2_anyAny.Equals(RTTI::MakeAny(float3(1, 2, 3)))); %TODO%
}
//----------------------------------------------------------------------------
static NO_INLINE void Test_Serializer_(const RTTI::FMetaTransaction& input, Serialize::ISerializer* serializer, const FFilename& filename) {
    Assert(input.NumTopObjects());
    Assert(serializer);

    const FFilename& fname_bin = filename;
    const FFilename fname_raw = filename.WithReplacedExtension(FFS::Raw());

    MEMORYSTREAM_THREAD_LOCAL(Serialize) uncompressed;
    {
        serializer->Serialize(&uncompressed, &input);
#if 0
        auto compressed = VFS_OpenBinaryWritable(filename, EAccessPolicy::Truncate);
        LZJB::CompressMemory(compressed.get(), uncompressed.MakeView());
#else
        RAWSTORAGE_THREAD_LOCAL(Serialize, u8) compressed;
        const size_t compressedSizeInBytes = Compression::CompressMemory(compressed, uncompressed.MakeView(), Compression::HighCompression);
        Assert(compressedSizeInBytes <= compressed.SizeInBytes());
        const TMemoryView<const u8> compressedView = compressed.MakeView().SubRange(0, compressedSizeInBytes);

        RAWSTORAGE_THREAD_LOCAL(Stream, u8) decompressed;
        if (false == Compression::DecompressMemory(decompressed, compressedView))
            AssertNotReached();

        Assert(checked_cast<size_t>(uncompressed.SizeInBytes()) == decompressed.SizeInBytes());
        const size_t k = decompressed.SizeInBytes();
        for (size_t i = 0; i < k; ++i)
            Assert(uncompressed.Pointer()[i] == decompressed.Pointer()[i]);

        if (false == VFS_WriteAll(fname_bin, compressedView, EAccessPolicy::Create_Binary))
            AssertNotReached();
        if (false == VFS_WriteAll(fname_raw, decompressed.MakeView(), EAccessPolicy::Create_Binary))
            AssertNotReached();
#endif
    }

    RTTI::FMetaTransaction output(RTTI::FName(MakeStringView("UnitTest_Output")));

    {
        RAWSTORAGE_THREAD_LOCAL(FileSystem, u8) compressed;
        if (false == VFS_ReadAll(&compressed, fname_bin, EAccessPolicy::Binary))
            AssertNotReached();

        RAWSTORAGE_THREAD_LOCAL(Stream, u8) decompressed;
        if (false == Compression::DecompressMemory(decompressed, compressed.MakeConstView()))
            AssertNotReached();

        Assert(checked_cast<size_t>(uncompressed.SizeInBytes()) == decompressed.SizeInBytes());
        const size_t k = decompressed.SizeInBytes();
        for (size_t i = 0; i < k; ++i)
            Assert(uncompressed.Pointer()[i] == decompressed.Pointer()[i]);

        serializer->Deserialize(&output, decompressed.MakeConstView());
    }

    AssertRelease(input.NumTopObjects() == output.NumTopObjects());

    if (false == input.DeepEquals(output))
        AssertNotReached();
}
//----------------------------------------------------------------------------
static NO_INLINE void Test_Serialize_() {
    typedef FRTTITest_ test_type;
    static constexpr size_t test_count = 5;

    FRTTIAtomRandomizer_ rand(test_count, 0xabadcafedeadbeefull);

    RTTI_NAMESPACE(RTTI_UnitTest).Start();
    {
        STACKLOCAL_TEXTWRITER(serialized, 1024);

        Serialize::FJson in, out;
        {
            in.Root().SetValue(165674.5454 / 0.666666);
            serialized << in;

            if (not Serialize::FJson::Load(&out, L"memory", serialized.Written()))
                AssertNotReached();

            Assert(in.Root() == out.Root());
        }
        serialized.Reset();
        {
            RTTI::FBinaryData binData;
            rand.Randomize(RTTI::MakeAtom(&binData));

            FStringView str = binData.MakeConstView().Cast<const char>();
            in.Root().SetValue(in.MakeString(str));
            serialized << in;

            if (not Serialize::FJson::Load(&out, L"memory", serialized.Written()))
                AssertNotReached();

            Assert(in.Root() == out.Root());
        }
    }
    {
        RTTI::FMetaTransaction input(RTTI::FName(MakeStringView("UnitTest_Input")));
        {
            FWStringBuilder oss;
            forrange(i, 0, test_count) {
                TRefPtr<test_type> t = new test_type();
                rand.Randomize(t.get());
                input.RegisterObject(t.get());
                RTTI::PrettyPrint(oss, RTTI::MakeAtom(&t));
                LOG(Test_RTTI, Info, oss.ToString());
            }
        }
        {
            Serialize::FBinarySerializer binary;
            Test_Serializer_(input, &binary, L"Saved:/RTTI/robotapp_bin.bin");
        }
        {
            Serialize::FJsonSerializer json;
            json.SetMinify(true);
            Test_Serializer_(input, &json, L"Saved:/RTTI/robotapp_json.json");
        }
    }

    RTTI_NAMESPACE(RTTI_UnitTest).Shutdown();
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
void Test_RTTI() {
    LOG(Test_RTTI, Emphasis, L"starting rtti tests ...");

    Test_Atoms_();
    Test_Any_();
    Test_Serialize_();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Test
} //!namespace Core
