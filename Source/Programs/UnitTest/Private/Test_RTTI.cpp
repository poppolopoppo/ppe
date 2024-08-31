// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "MetaDatabase.h"
#include "MetaTransaction.h"
#include "RTTI/Any.h"
#include "RTTI/Atom.h"
#include "RTTI/AtomVisitor.h"
#include "RTTI/Macros.h"
#include "RTTI/Macros-impl.h"
#include "RTTI/Module.h"
#include "RTTI/Module-impl.h"
#include "RTTI/NativeTypes.h"
#include "RTTI/TypeInfos.h"

#include "Binary/BinarySerializer.h"
#include "Json/Json.h"
#include "Json/JsonSerializer.h"
#include "Text/TextSerializer.h"
#include "TransactionLinker.h"
#include "TransactionSaver.h"
#include "TransactionSerializer.h"

#include "Text/Grammar.h"
#include "Lexer/Lexer.h"
#include "Parser/ParseContext.h"
#include "Parser/ParseExpression.h"
#include "Parser/ParseList.h"
#include "Parser/ParseStatement.h"

#include "Container/AssociativeVector.h"
#include "Container/HashMap.h"
#include "Container/Pair.h"
#include "Container/Tuple.h"
#include "Container/Vector.h"
#include "Diagnostic/Logger.h"
#include "IO/ConstNames.h"
#include "IO/Dirpath.h"
#include "IO/Filename.h"
#include "IO/FileStream.h"
#include "IO/Format.h"
#include "IO/StringBuilder.h"
#include "IO/StringView.h"
#include "IO/TextWriter.h"
#include "VirtualFileSystem.h"
#include "Maths/RandomGenerator.h"
#include "Maths/ScalarMatrix.h"
#include "Maths/ScalarVector.h"

#include "Memory/Compression.h"
#include "Memory/MemoryStream.h"

#include "HAL/PlatformConsole.h"
#include "HAL/PlatformProcess.h"

#include "Meta/Utility.h"

#define USE_RTTI_TEST_YOLOTYPES 0 //%_NOCOMMIT%

#include <iostream> // Test_InteractiveConsole_

#include "Memory/SharedBuffer.h"

namespace PPE {
namespace Test {
LOG_CATEGORY(, Test_RTTI)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
RTTI_MODULE_DECL(, RTTI_UnitTest);
RTTI_MODULE_DEF(, RTTI_UnitTest, MetaObject);
//----------------------------------------------------------------------------
struct FStructAsTuple {
    float3 Position;
    FString Name;
    TVector<int> Weights;
};
RTTI_STRUCT_DECL(CONSTEXPR, FStructAsTuple);
RTTI_STRUCT_DEF(CONSTEXPR, FStructAsTuple);
//----------------------------------------------------------------------------
enum ETest : u32 {
    A, B, C, D
};
RTTI_ENUM_HEADER(, ETest);
RTTI_ENUM_BEGIN(RTTI_UnitTest, ETest)
RTTI_ENUM_VALUE(A)
RTTI_ENUM_VALUE(B)
RTTI_ENUM_VALUE(C)
RTTI_ENUM_VALUE(D)
RTTI_ENUM_END()
//----------------------------------------------------------------------------
FWD_REFPTR(RTTITestSimple_);
class FRTTITestSimple_ : public RTTI::FMetaObject {
public:
    RTTI_CLASS_HEADER(, FRTTITestSimple_, RTTI::FMetaObject);

    FRTTITestSimple_() {}

private:
    bool _b;
    int _i;
    float _f;
    double _d;
    float3 _vec3;
    TVector<float4> _rots;
    TAssociativeVector<FString, ETest> _assoc;
    TAssociativeVector<u64, u64> _uInt64Dico;
};
RTTI_CLASS_BEGIN(RTTI_UnitTest, FRTTITestSimple_, Concrete)
RTTI_PROPERTY_PRIVATE_FIELD(_b)
RTTI_PROPERTY_PRIVATE_FIELD(_i)
RTTI_PROPERTY_PRIVATE_FIELD(_f)
RTTI_PROPERTY_PRIVATE_FIELD(_d)
RTTI_PROPERTY_PRIVATE_FIELD(_vec3)
RTTI_PROPERTY_PRIVATE_FIELD(_rots)
RTTI_PROPERTY_PRIVATE_FIELD(_assoc)
RTTI_PROPERTY_PRIVATE_FIELD(_uInt64Dico)
RTTI_CLASS_END()
//----------------------------------------------------------------------------
enum class ETestEnum32 : u32 {
    A = 0, B, C
};
RTTI_ENUM_HEADER(, ETestEnum32);
RTTI_ENUM_BEGIN(RTTI_UnitTest, ETestEnum32)
RTTI_ENUM_VALUE(A)
RTTI_ENUM_VALUE(B)
RTTI_ENUM_VALUE(C)
RTTI_ENUM_END()
//----------------------------------------------------------------------------
enum class EFlags : u64 {
    First = 1 << 0, Second = 1 << 1, Third = 1 << 2
};
RTTI_ENUM_HEADER(, EFlags);
RTTI_ENUM_FLAGS_BEGIN(RTTI_UnitTest, EFlags)
RTTI_ENUM_VALUE(First)
RTTI_ENUM_VALUE(Second)
RTTI_ENUM_VALUE(Third)
RTTI_ENUM_END()
//----------------------------------------------------------------------------
FWD_REFPTR(RTTITestParent_);
class FRTTITestParent_ : public RTTI::FMetaObject {
public:
    RTTI_CLASS_HEADER(, FRTTITestParent_, RTTI::FMetaObject);

    FRTTITestParent_() {}

    const ETestEnum32& Enum32() const { return _testEnum32; }
    void SetEnum32(const ETestEnum32& value) { _testEnum32 = value; }

    const EFlags& Enum64() const { return _testEnum64; }
    void SetEnum64(const EFlags& value) { _testEnum64 = value; }

    PRTTITestParent_ Child() const { return _child; }
    void SetChild(const PRTTITestParent_& child) { _child = child; }

private:
    FFilename _sourceName;
    ETestEnum32 _testEnum32;
    EFlags _testEnum64;
    PRTTITestParent_ _child;
    FStructAsTuple _structAsTuple;
};
RTTI_CLASS_BEGIN(RTTI_UnitTest, FRTTITestParent_, Concrete)
    RTTI_PROPERTY_PRIVATE_FIELD(_sourceName)
    RTTI_PROPERTY_PRIVATE_FIELD(_testEnum32)
    RTTI_PROPERTY_PRIVATE_FIELD(_testEnum64)
    RTTI_PROPERTY_PRIVATE_FIELD(_child)
    RTTI_PROPERTY_PRIVATE_FIELD(_structAsTuple)
RTTI_CLASS_END()
//----------------------------------------------------------------------------
FWD_REFPTR(RTTITest_);
class FRTTITest_ : public FRTTITestParent_ {
public:
    RTTI_CLASS_HEADER(, FRTTITest_, FRTTITestParent_);

    FRTTITest_()
    :   _dummy(0)
    {}

    bool _dummy;

#if USE_RTTI_TEST_YOLOTYPES
    template <typename T>
    using yolo_pair_type = TPair<T, T>;
    template <typename T>
    using yolo_dict_type = HASHMAP(Container, T, T);
    template <typename T>
    using yolo_vect_type = VECTOR(Container, yolo_dict_type<T>);
    template <typename T>
    using yolo_type = HASHMAP(Container, TPair<T COMMA T>, yolo_vect_type<T>);

#define DEF_METATYPE_SCALAR_IMPL_(_Name, T, _TypeId) \
    T _ ## _Name ## Scalar; \
    TVector<T> _ ## _Name ## Vec; \
    TPair<T, T> _ ## _Name ## TPair; \
    TAssociativeVector<T, T> _ ## _Name ## Dico; \
    yolo_vect_type<T> _ ## _Name ## Vect; \
    yolo_dict_type<T> _ ## _Name ## HashMap; \
    /*yolo_type<T> _ ## _Name ## MultiMap;*/
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
RTTI_CLASS_BEGIN(RTTI_UnitTest, FRTTITest_, Concrete)
    RTTI_PROPERTY_PRIVATE_FIELD(_dummy)
#if USE_RTTI_TEST_YOLOTYPES
#define DEF_METATYPE_SCALAR_IMPL_(_Name, T, _TypeId) \
    RTTI_PROPERTY_PRIVATE_FIELD(_ ## _Name ## Scalar) \
    RTTI_PROPERTY_PRIVATE_FIELD(_ ## _Name ## Vec) \
    RTTI_PROPERTY_PRIVATE_FIELD(_ ## _Name ## TPair) \
    RTTI_PROPERTY_PRIVATE_FIELD(_ ## _Name ## Dico) \
    RTTI_PROPERTY_PRIVATE_FIELD(_ ## _Name ## Vect) \
    RTTI_PROPERTY_PRIVATE_FIELD(_ ## _Name ## HashMap) \
    /*RTTI_PROPERTY_PRIVATE_FIELD(_ ## _Name ## MultiMap)*/
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
    FRTTIAtomRandomizer_(size_t maxDim, u64 seed)
    :   _maxDim(maxDim)
    ,   _depth(0)
    ,   _rand(seed)
    ,   _import(nullptr) {
        AssertRelease(_maxDim > 0);
    }

    void Randomize(const RTTI::FAtom& atom) {
        atom.Accept(this);
    }

    void Randomize(RTTI::FMetaObject* pobject, const RTTI::FMetaTransaction* import = nullptr);

protected:
    virtual bool Visit(const RTTI::ITupleTraits* tuple, void* data) override final {
        return parent_type::Visit(tuple, data);
    }

    virtual bool Visit(const RTTI::IListTraits* list, void* data) override final {
        const size_t count = NextRandomDim_();
        list->Reserve(data, count);
        forrange(i, 0, count)
            list->AddDefault(data);

        return parent_type::Visit(list, data);
    }

    virtual bool Visit(const RTTI::IDicoTraits* dico, void* data) override final {
        STACKLOCAL_ATOM(keyData, dico->KeyTraits());
        const RTTI::FAtom keyAtom = keyData.MakeAtom();

        const size_t count = NextRandomDim_();
        dico->Reserve(data, count);
        forrange(i, 0, count) {
            if (not keyAtom.Accept(this))
                return false;

            if (dico->Find(data, keyAtom))
                continue;

            if (not dico->AddDefaultMove(data, std::move(keyAtom)).Accept(this))
                return false;
        }

        return true;
    }

#define DEF_METATYPE_SCALAR(_Name, T, _TypeId) \
    virtual bool Visit(const RTTI::IScalarTraits* , T& value) override final { \
        Randomize_(value); \
        return true; \
    }
    FOREACH_RTTI_NATIVETYPES(DEF_METATYPE_SCALAR)
#undef DEF_METATYPE_SCALAR

private:
    const size_t _maxDim;
    size_t _depth;
    FRandomGenerator _rand;
    const RTTI::FMetaTransaction* _import;

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
        const size_t count = 1 + NextRandomDim_();
        STACKLOCAL_POD_ARRAY(RTTI::FName::char_type, cstr, count);
        forrange(i, 0, count)
            cstr[i] = (char)(_rand.Next('a'+0, 'z'+1));
        name = RTTI::FName(MakeStringView(cstr));
    }

    void Randomize_(RTTI::FBinaryData& rawdata) {
        const size_t count = NextRandomDim_()*10;
        rawdata.Resize_DiscardData(count);
        forrange(i, 0, count)
            rawdata[i] = u8(_rand.Next() | 1);
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
#if 0
        Unused(obj);
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
#else
        if (_import && _rand.NextFloat01() < 0.7f)
            obj = _rand.RandomElement(_import->TopObjects().MakeView());
#endif
    }

};
//----------------------------------------------------------------------------
void FRTTIAtomRandomizer_::Randomize(RTTI::FMetaObject* pobject, const RTTI::FMetaTransaction* import) {
    Assert(pobject);

    _import = import;

    ++_depth;

    const RTTI::FMetaClass* metaClass = pobject->RTTI_Class();

    for (const auto* prop : metaClass->AllProperties()) {
        RTTI::FAtom atom = prop->Get(*pobject);
        PPE_LOG_CHECKVOID(Test_RTTI, atom);
        atom.Accept(this);
    }

    Assert(0 < _depth);
    --_depth;

    _import = nullptr;
}
//----------------------------------------------------------------------------
static void print_atom(const RTTI::FAtom& atom) {
#if USE_PPE_LOGGER
    const RTTI::FNamedTypeInfos typeInfo = atom.NamedTypeInfos();
    PPE_LOG(Test_RTTI, Info, "{0}[0x{1:#8x}] : {2} = {3} (flags: {4}, default:{5:a})",
        typeInfo.Name(),
        typeInfo.Id(),
        atom.HashValue(),
        atom,
        typeInfo.Flags(),
        atom.IsDefaultValue());
#else
    Unused(atom);
#endif
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

    VECTOR(Atom, float2) v2;
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
    STATIC_ASSERT(RTTI::has_support_for_v<decltype(42)>);
    STATIC_ASSERT(RTTI::has_support_for_v<decltype(42.)>);
    STATIC_ASSERT(RTTI::has_support_for_v<FString>);

    RTTI::FAny any;
    any.Assign(42);
    RTTI::FAny anyInt = RTTI::MakeAny(42);
    PPE_LOG(Test_RTTI, Debug, "anyInt: {0} -> {1}", anyInt, RTTI::CastChecked<int>(anyInt));
    RTTI::FAny anyAny = RTTI::MakeAny(42.);
    PPE_LOG(Test_RTTI, Debug, "anyAny: {0} -> {1}", anyAny, RTTI::CastChecked<double>(anyAny));
    Assert(anyAny != anyInt);
    anyAny = anyInt;
    PPE_LOG(Test_RTTI, Debug, "anyAny: {0} -> {1}", anyAny, RTTI::CastChecked<int>(anyAny));
    Assert(anyInt == anyAny);
    anyAny = RTTI::MakeAny(FString("toto"));
    PPE_LOG(Test_RTTI, Debug, "anyAny: {0} -> {1}", anyAny, RTTI::CastChecked<FString>(anyAny));
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
    RTTI::FAtom dico2_anyAny = dico2->Find(datom2.Data(), anyAny);
    PPE_LOG(Test_RTTI, Debug, "dico2_anyAny: {0}", dico2_anyAny.TypedConstData<float3>());
    PPE_LOG_CHECKVOID(Test_RTTI, RTTI::MakeAny(float3(1, 2, 3)).InnerAtom().Equals(dico2_anyAny));
    //PPE_LOG_CHECKVOID(Test_RTTI, dico2_anyAny.Equals(RTTI::MakeAny(float3(1, 2, 3)))); #TODO

    RTTI::FAny anyAny2{ RTTI::ENativeType::Any };
    PPE_LOG_CHECKVOID(Test_RTTI, not anyAny.IsDefaultValue());
    PPE_LOG_CHECKVOID(Test_RTTI, &anyAny2 == anyAny2.InnerAtom().Data());
    VerifyRelease(anyAny.PromoteCopy(anyAny2.InnerAtom()));
    PPE_LOG_CHECKVOID(Test_RTTI, anyAny2.Equals(anyAny));
    print_atom(anyAny2);

    anyAny.Reset();
    PPE_LOG_CHECKVOID(Test_RTTI, not anyAny);
    PPE_LOG_CHECKVOID(Test_RTTI, anyAny.IsDefaultValue());
    PPE_LOG_CHECKVOID(Test_RTTI, not anyAny2.IsDefaultValue());
    VerifyRelease(anyAny2.PromoteMove(anyAny.InnerAtom()));
    print_atom(anyAny2);
    print_atom(anyAny);
    PPE_LOG_CHECKVOID(Test_RTTI, anyAny2);
    PPE_LOG_CHECKVOID(Test_RTTI, not anyAny.IsDefaultValue());
    PPE_LOG_CHECKVOID(Test_RTTI, anyAny.Equals(anyAny2));
}
//----------------------------------------------------------------------------
static NO_INLINE void Test_CircularReferences_() {
    PRTTITest_ riri{ NEW_RTTI(FRTTITest_) };
    PRTTITest_ fifi{ NEW_RTTI(FRTTITest_) };
    PRTTITest_ lulu{ NEW_RTTI(FRTTITest_) };

    riri->RTTI_Export(RTTI::FName{ "riri" });
    fifi->RTTI_Export(RTTI::FName{ "fifi" });
    lulu->RTTI_Export(RTTI::FName{ "lulu" });

    riri->SetChild(fifi);
    fifi->_MetaObjectVec.push_back(lulu);
    lulu->_MetaObjectTPair = MakePair(RTTI::PMetaObject{}, riri);

    const RTTI::PMetaObject objs[3] = { riri, fifi, lulu };

    VerifyRelease(not RTTI::CheckCircularReferences(MakeView(objs)));

    riri->SetChild(nullptr);

    VerifyRelease(RTTI::CheckCircularReferences(MakeView(objs)));
}
//----------------------------------------------------------------------------
static NO_INLINE void Test_Serializer_(const RTTI::FMetaTransaction& input, Serialize::ISerializer& serializer, const FFilename& filename) {
    PPE_LOG_CHECKVOID(Test_RTTI, not input.empty());

    const FFilename fname_binz = filename.WithReplacedExtension(FFS::Z());
    const FFilename& fname_raw = filename;

    MEMORYSTREAM(Stream) uncompressed;
    {
        Serialize::FTransactionSaver saver{ input, filename };
        serializer.Serialize(saver, &uncompressed);
#if 0
        auto compressed = VFS_OpenBinaryWritable(filename, EAccessPolicy::Truncate);
        LZJB::CompressMemory(compressed.get(), uncompressed.MakeView());
#else
        RAWSTORAGE(Stream, u8) compressed;
        const size_t compressedSize = Compression::CompressMemory(compressed, uncompressed.MakeView(), Compression::HighCompression);
        const TMemoryView<const u8> compressedView = compressed.MakeView().FirstNElements(compressedSize);

        RAWSTORAGE(Stream, u8) decompressed;
        if (false == Compression::DecompressMemory(decompressed, compressedView))
            AssertNotReached();

        Assert(checked_cast<size_t>(uncompressed.SizeInBytes()) == decompressed.SizeInBytes());
        const size_t k = decompressed.SizeInBytes();
        for (size_t i = 0; i < k; ++i)
            Assert(uncompressed.Pointer()[i] == decompressed.Pointer()[i]);

        if (false == VFS_WriteAll(fname_binz, compressedView, EAccessPolicy::Truncate_Binary))
            AssertNotReached();
        if (false == VFS_WriteAll(fname_raw, decompressed.MakeView(), EAccessPolicy::Truncate_Binary))
            AssertNotReached();
#endif
    }

    RTTI::FMetaTransaction output(RTTI::FName(MakeStringView("UnitTest_Output")));

    {
        const Meta::TOptional<FUniqueBuffer> compressed = VFS_ReadAll(fname_binz, EAccessPolicy::Binary);
        PPE_LOG_CHECKVOID(Test_RTTI, compressed);

        FUniqueBuffer decompressed = Compression::DecompressBuffer( *compressed);
        PPE_LOG_CHECKVOID(Test_RTTI, checked_cast<size_t>(uncompressed.SizeInBytes()) == decompressed.SizeInBytes());
        PPE_LOG_CHECKVOID(Test_RTTI, uncompressed.MakeView().RangeEqual(decompressed.MakeView()));

        Serialize::FTransactionLinker linker{ filename };
        Serialize::ISerializer::Deserialize(serializer, decompressed.MakeView(), &linker);
        linker.Resolve(output);
    }

    PPE_LOG_CHECKVOID(Test_RTTI, input.TopObjects().size() == output.TopObjects().size());

    if (false == input.DeepEquals(output))
        AssertNotReached();
}
//----------------------------------------------------------------------------
FWD_REFPTR(RTTIConsole_);
class FRTTIConsole_ : public RTTI::FMetaObject {
public:
    RTTI_CLASS_HEADER(, FRTTIConsole_, RTTI::FMetaObject);

    FRTTIConsole_() {}

    u32 PID;
    FString Hostname;

    auto classes() const {
        VECTOR(Script, RTTI::FName) results;
        const RTTI::FMetaDatabaseReadable db;
        const auto names = db->Classes().Keys();
        results.insert(results.end(), names.begin(), names.end());
        return results;
    }

    auto enums() const {
        VECTOR(Script, RTTI::FName) results;
        const RTTI::FMetaDatabaseReadable db;
        const auto names = db->Enums().Keys();
        results.insert(results.end(), names.begin(), names.end());
        std::sort(results.begin(), results.end());
        return results;
    }

    auto objects() const {
        VECTOR(Script, RTTI::FPathName) results;
        const RTTI::FMetaDatabaseReadable db;
        const auto names = db->Objects().Keys();
        results.insert(results.end(), names.begin(), names.end());
        std::sort(results.begin(), results.end());
        return results;
    }

    auto functions(const RTTI::PMetaObject& obj) const {
        VECTOR(Script, FString) results;
        if (obj) {
            const RTTI::FMetaClass* const klass = obj->RTTI_Class();
            for (const RTTI::FMetaFunction* f : klass->AllFunctions())
                results.push_back(ToString(*f));
            std::sort(results.begin(), results.end());
        }
        return results;
    }

    auto properties(const RTTI::PMetaObject& obj) const {
        VECTOR(Script, RTTI::FName) results;
        if (obj) {
            const RTTI::FMetaClass* const klass = obj->RTTI_Class();
            for (const RTTI::FMetaProperty* p : klass->AllProperties())
                results.push_back(p->Name());
            std::sort(results.begin(), results.end());
        }
        return results;
    }

    RTTI::PMetaObject object(const RTTI::FPathName& path) const {
        const RTTI::FMetaDatabaseReadable db;
        return db->ObjectIFP(path);
    }

    RTTI::FPathName share(const RTTI::PMetaObject& obj) const {
        if (not obj || not obj->RTTI_IsExported() || not obj->RTTI_Outer())
            return RTTI::FPathName{};

        const RTTI::FMetaDatabaseReadWritable db;
        db->RegisterObject(obj.get());

        return RTTI::FPathName::FromObject(*obj);
    }

    void inspect(const RTTI::FAny& any) const {
        FStringBuilder oss;
        RTTI::PrettyPrint(oss, any.InnerAtom(), RTTI::EPrettyPrintFlags::Full);
        oss << Endl;
        FPlatformConsole::SyntaxicHighlight(oss.Written(), FPlatformConsole::WHITE_ON_BLACK);
    }

    int mul(int x, int m) const {
        return x * m;
    }

    FString formatA(const FString& fmt, const VECTOR(Atom, RTTI::FAny)& args) const {
        STACKLOCAL_POD_ARRAY(FFormatArg, fmtArgs, args.size());
        forrange(i, 0, args.size())
            fmtArgs[i] = MakeFormatArg<char>(args[i]);

        FStringBuilder sb;
        if (UnsafeFormatArgs<char>(sb, fmt.c_str(), fmtArgs))
            return sb.ToString();

        return "error!";
    }
    FWString formatW(const FWString& fmt, const VECTOR(Atom, RTTI::FAny)& args) const {
        STACKLOCAL_POD_ARRAY(FWFormatArg, fmtArgs, args.size());
        forrange(i, 0, args.size())
            fmtArgs[i] = MakeFormatArg<wchar_t>(args[i]);

        FWStringBuilder sb;
        if (UnsafeFormatArgs<wchar_t>(sb, fmt.c_str(), fmtArgs))
            return sb.ToString();

        return L"error!";
    }
};
RTTI_CLASS_BEGIN(RTTI_UnitTest, FRTTIConsole_, Concrete)
RTTI_PROPERTY_PUBLIC_FIELD(PID)
RTTI_PROPERTY_PUBLIC_FIELD(Hostname)
RTTI_FUNCTION(classes, ())
RTTI_FUNCTION(objects, ())
RTTI_FUNCTION(enums, ())
RTTI_FUNCTION(functions, (obj))
RTTI_FUNCTION(properties, (obj))
RTTI_FUNCTION(object, (path))
RTTI_FUNCTION(share, (obj))
RTTI_FUNCTION(inspect, (any))
RTTI_FUNCTION(mul, (x, m))
RTTI_FUNCTION(formatA, (fmt, args))
RTTI_FUNCTION(formatW, (fmt, args))
RTTI_CLASS_END()
//----------------------------------------------------------------------------
static NO_INLINE void Test_InteractiveConsole_() {
#if !USE_PPE_PROFILING
    PPE_LOG_FLUSH();

    FPlatformConsole::Open();

    auto diagnosticCarret = [](const Lexer::FSpan& site, FPlatformConsole::EAttribute attrs) NOEXCEPT{
        FPlatformConsole::Write(" ");
        forrange(i, 0, site.Column)
            FPlatformConsole::Write(" ");
        FPlatformConsole::Write(L"\u2558", attrs);
        forrange(i, 0, Max(site.Length, 1_u32))
            FPlatformConsole::Write(L"\u2550", attrs);
        FPlatformConsole::Write(L"\u255B", attrs);
        FPlatformConsole::Write("\n");
    };

    {
        PRTTIConsole_ env{ NEW_RTTI(FRTTIConsole_) };
        env->PID = u32(FPlatformProcess::CurrentPID());
        VerifyRelease(FPlatformProcess::Name(&env->Hostname, FPlatformProcess::CurrentProcess()));

        char buffer[1024];
        Parser::FParseContext globalContext(Meta::ForceInit);
        globalContext.AddGlobal(nullptr, RTTI::FName("_"), RTTI::MakeAtom(&env));

        do
        {
            FPlatformConsole::Write(L" \u25C0", FPlatformConsole::FG_BLUE|FPlatformConsole::FG_INTENSITY);
            FPlatformConsole::Write(L" ", FPlatformConsole::WHITE_ON_BLACK);

            const size_t length = FPlatformConsole::Read(buffer);
            if (0 == length)
                break;

            FStringView line{ buffer, length };
            line = Strip(Chomp(line));

            if (line.empty())
                continue;
            else if (0 == CompareI(MakeStringView("exit"), line))
                break;

            CONSTEXPR const FWStringLiteral filename = L"@in_memory";

            try {
                FMemoryViewReader reader(line.Cast<const u8>());
                Lexer::FLexer lexer(reader, filename.MakeView(), true);

                Parser::FParseList input;
                input.Parse(&lexer);

                Parser::PCParseExpression expr = Serialize::FGrammarStartup::ParseExpression(input);
                PPE_LOG_CHECKVOID(Test_RTTI, expr);

                const RTTI::FAtom value = expr->Eval(&globalContext);

                FPlatformConsole::Write(L" \u25B6 ", FPlatformConsole::FG_GREEN | FPlatformConsole::FG_INTENSITY);

                if (value)
                    FPlatformConsole::Write(StringFormat("{0}\n", value), FPlatformConsole::FG_GREEN);
                else
                    FPlatformConsole::Write("void\n", FPlatformConsole::FG_GREEN);
            }
            catch (const Parser::FParserException& e) {
                diagnosticCarret(e.Site(), FPlatformConsole::FG_RED | FPlatformConsole::FG_INTENSITY);
                FPlatformConsole::Write(L" \u203C ", FPlatformConsole::FG_RED | FPlatformConsole::FG_INTENSITY);
                if (e.Item())
                    FPlatformConsole::Write(StringFormat("parsing error : <{0}> {1}, {2}.\n", e.Item()->ToString(), MakeCStringView(e.What()), e.Site()), FPlatformConsole::FG_RED);
                else
                    FPlatformConsole::Write(StringFormat("parsing error : {0}, {1}.\n", MakeCStringView(e.What()), e.Site()), FPlatformConsole::FG_RED | FPlatformConsole::FG_INTENSITY);
            }
            catch (const Lexer::FLexerException& e) {
                diagnosticCarret(e.Match().Site(), FPlatformConsole::FG_YELLOW | FPlatformConsole::FG_INTENSITY);
                FPlatformConsole::Write(L" \u00BF ", FPlatformConsole::FG_YELLOW | FPlatformConsole::FG_INTENSITY);
                FPlatformConsole::Write(StringFormat("lexing error : <{0}>: {1}, {2}.\n", e.Match().Symbol()->CStr(), MakeCStringView(e.What()), e.Match().Site()), FPlatformConsole::FG_YELLOW);
            }

        } while (true);
    }

    FPlatformConsole::Close();

#endif //!!USE_PPE_PROFILING
}
//----------------------------------------------------------------------------
template <typename T>
static NO_INLINE void Test_TransactionSerialization_() {
#if USE_PPE_ASSERT
    static constexpr bool minify = false;
    static constexpr size_t test_count = 5;
#else
    static constexpr bool minify = true;
    static constexpr size_t test_count = 50;
#endif

    struct FLoadindScope {
        RTTI::FMetaTransaction& Outer;
        FLoadindScope(RTTI::FMetaTransaction& outer)
        :   Outer(outer) {
            Outer.LoadAndMount();
        }
        ~FLoadindScope() {
            Outer.UnmountAndUnload();
        }
    };

    FRTTIAtomRandomizer_ rand(test_count, 0xabadcafedeadbeefull);

    RTTI::FMetaTransaction import(RTTI::FName(MakeStringView("UnitTest_Import")));
    {
        FWStringBuilder oss;

        forrange(i, 0, test_count) {
            TRefPtr<T> t = NEW_RTTI(T);
            rand.Randomize(t.get());
            t->RTTI_Export(RTTI::FName(StringFormat("import_{0}", intptr_t(t.get()))));
            import.Add(t.get());
        }
    }

    const FLoadindScope importScope(import);
    {
        const FWString basePath = StringFormat(L"Saved:/RTTI/UnitTest_Import_{0}", RTTI::MetaClass<T>()->Name());
        {
            Serialize::USerializer bin{ Serialize::FBinarySerializer::Get() };
            Test_Serializer_(import, *bin, basePath + L"_bin.bin");
        }
        {
            Serialize::USerializer json{ Serialize::FJsonSerializer::Get() };
            json->SetMinify(minify);
            Test_Serializer_(import, *json, basePath + L"_json.json");
        }
        {
            Serialize::USerializer text{ Serialize::FTextSerializer::Get() };
            text->SetMinify(minify);
            Test_Serializer_(import, *text, basePath + L"_text.txt");
        }
    }

    RTTI::FMetaTransaction input(RTTI::FName(MakeStringView("UnitTest_Input")));
    {
        FWStringBuilder oss;

        forrange(i, 0, test_count) {
            TRefPtr<T> t = NEW_RTTI(T);
            rand.Randomize(t.get(), &import);
            input.Add(t.get());
        }
    }

    const FLoadindScope inputScope(input);
    {
        const FWString basePath = StringFormat(L"Saved:/RTTI/UnitTest_Input_{0}", RTTI::MetaClass<T>()->Name());
        {
            Serialize::USerializer bin{ Serialize::FBinarySerializer::Get() };
            Test_Serializer_(input, *bin, basePath + L"_bin.bin");
        }
        {
            Serialize::USerializer json{ Serialize::FJsonSerializer::Get() };
            json->SetMinify(minify);
            Test_Serializer_(input, *json, basePath + L"_json.json");
        }
        {
            Serialize::USerializer text{ Serialize::FTextSerializer::Get() };
            text->SetMinify(minify);
            Test_Serializer_(input, *text, basePath + L"_text.txt");
        }
    }
}
//----------------------------------------------------------------------------
static NO_INLINE void Test_TransactionSerializer_() {
    Serialize::FDirectoryTransaction import(
        RTTI::FName("Robotapp"),
        RTTI::FName("UnitTest_Import"),
        L"UnitTest_Import_.*\\.txt",
        { L"Saved:/RTTI" } );

    Serialize::FTransactionSources importSources;
    import.BuildTransaction(importSources);
    import.SaveTransaction();
    import.UnloadTransaction();
    import.LoadTransaction();
    import.MountToDB();

    Serialize::FDirectoryTransaction input(
        RTTI::FName("Robotapp"),
        RTTI::FName("UnitTest_Input"),
        L"UnitTest_Input_.*\\.txt",
        { L"Saved:/RTTI" });

    Serialize::FTransactionSources inputSources;
    input.BuildTransaction(inputSources);
    input.SaveTransaction();
    input.UnloadTransaction();
    input.LoadTransaction();
    input.MountToDB();

    input.UnmountFromDB();
    input.UnloadTransaction();

    import.UnmountFromDB();
    import.UnloadTransaction();
}
//----------------------------------------------------------------------------
static NO_INLINE void Test_Serialize_() {
    {
        FStringBuilder serialized;
        Serialize::FJson::FAllocator alloc;

        Serialize::FJson in{ alloc }, out{ alloc };
        {
            in.Root() = 165674.5454 / 0.666666;
            serialized << in;

            if (not Serialize::FJson::Load(&out, L"memory"_view, serialized.Written()))
                AssertNotReached();

            Assert(in.Root() == out.Root());
        }
        serialized.Reset();
        {
            FRTTIAtomRandomizer_ rand(128, 123456);

            RTTI::FBinaryData binData;
            rand.Randomize(RTTI::MakeAtom(&binData));

            const FStringView str = binData.MakeConstView().Cast<const char>();
            in.Root() = in.AnsiText().Append(str);
            serialized << in;

            if (not Serialize::FJson::Load(&out, L"memory"_view, serialized.Written()))
                AssertNotReached();

            Assert(in.Root() == out.Root());
        }
    }
    {
        const FStringLiteral RTTITestSimple_{ FRTTITestSimple_::RTTI_FMetaClass::Get()->Name() };
        const FStringLiteral RTTITest_{ FRTTITest_::RTTI_FMetaClass::Get()->Name() };
        PPE_LOG_CHECKVOID(Test_RTTI, RTTITestSimple_ == "RTTITestSimple_");
        PPE_LOG_CHECKVOID(Test_RTTI, RTTITest_ == "RTTITest_");
    }
    {
        Test_TransactionSerialization_<FRTTITestSimple_>();
        Test_TransactionSerialization_<FRTTITest_>();
    }
    {
        Test_TransactionSerializer_();
    }
}
//----------------------------------------------------------------------------
static bool EvalExpr_(Parser::FParseContext* context, FStringLiteral input) {
    PPE_LOG(Test_RTTI, Info, "EvalExpr('{0}') :", input);
    try
    {
        FMemoryViewReader reader(input.MakeView().RawView());
        Lexer::FLexer lexer(reader, L"@memory"_view, true);

        Parser::FParseList parser;
        parser.Parse(&lexer);

        Parser::PCParseExpression expr = Serialize::FGrammarStartup::ParseExpression(parser);
        PPE_LOG_CHECK(Test_RTTI, expr);

        const RTTI::FAtom result = expr->Eval(context);

        Unused(result);
        PPE_LOG(Test_RTTI, Info, " -> {0} : {1}", result, result.NamedTypeInfos());

        return true;
    }
    catch (const Parser::FParserException& e) {
        Unused(e);
        if (e.Item())
            PPE_LOG(Test_RTTI, Error, " !! Parser error : <{0}> {1}, {2}.", e.Item()->ToString(), MakeCStringView(e.What()), e.Site());
        else
            PPE_LOG(Test_RTTI, Error, " !! Parser error : {0}, {1}.", MakeCStringView(e.What()), e.Site());
    }
    catch (const Lexer::FLexerException& e) {
        Unused(e);
        PPE_LOG(Test_RTTI, Error, " ?? Lexer error : <{0}>: {1}, {2}.", e.Match().Symbol()->CStr(), MakeCStringView(e.What()), e.Match().Site());
    }

    PPE_LOG_FLUSH();
    return false;
}
//----------------------------------------------------------------------------
static void Test_Grammar_() {
    Parser::FParseContext context(Meta::ForceInit);
    VerifyRelease(EvalExpr_(&context, "2*(3+5)"));
    VerifyRelease(EvalExpr_(&context, "(1)"));
    VerifyRelease(EvalExpr_(&context, "('foo','bar')"));
    VerifyRelease(EvalExpr_(&context, "(1,true,'toto')"));
    VerifyRelease(EvalExpr_(&context, "[]"));
    VerifyRelease(EvalExpr_(&context, "['toto']"));
    VerifyRelease(EvalExpr_(&context, "[1,2,3]"));
    VerifyRelease(EvalExpr_(&context, "[1,2,3][2]"));
    VerifyRelease(EvalExpr_(&context, "[1,2,3][-2]"));
    VerifyRelease(EvalExpr_(&context, "{}"));
    VerifyRelease(EvalExpr_(&context, "{('a',1.0)}"));
    VerifyRelease(EvalExpr_(&context, "{('a',1),('b',2),('c',3)}"));
    VerifyRelease(EvalExpr_(&context, "{('a',1),('b',2),('c',3)}['a']"));
    VerifyRelease(EvalExpr_(&context, "{('a',1),('b',2),('c',3)}[String:'b']"));
    VerifyRelease(EvalExpr_(&context, "Int32:0"));
    VerifyRelease(EvalExpr_(&context, "Name:'toto'"));
    VerifyRelease(EvalExpr_(&context, "Float:-1"));
    VerifyRelease(EvalExpr_(&context, "String:('toto'+'tata')"));
    VerifyRelease(EvalExpr_(&context, "(Any:\"toto\",Any:42,Any:3.456)"));
    VerifyRelease(EvalExpr_(&context, "[Any:42]"));
    VerifyRelease(EvalExpr_(&context, "Any:[1,2,3]"));
    VerifyRelease(EvalExpr_(&context, "Any:[1,2,3][0]"));
    VerifyRelease(EvalExpr_(&context, "Any:[Any:\"toto\"]"));
    VerifyRelease(EvalExpr_(&context, "Any:[Any:[Any:\"totototototototototototototoot\"]]"));
    VerifyRelease(EvalExpr_(&context, "(Int32:-1317311908, WString:'zee')"));
    VerifyRelease(EvalExpr_(&context, "ETest:'a'+ETest:1"));
    VerifyRelease(EvalExpr_(&context, "RTTITest_ {}"));
    VerifyRelease(EvalExpr_(&context, "RTTITest__xxx is RTTITest_ {}"));
    VerifyRelease(EvalExpr_(&context, "RTTITest__yyy is RTTITest_ { Int32Scalar = 69 }"));
    VerifyRelease(EvalExpr_(&context, "RTTITest__zzz is RTTITest_ { AnyTPair = ('toto', 42) }"));
    VerifyRelease(EvalExpr_(&context, "RTTITest__www is RTTITest_ { AnyTPair = (Int32:-1317311908, WString:'zee') }"));
    VerifyRelease(EvalExpr_(&context, "Any:[{(1,[(Double:Int64:(1+(2+(3*(2*(4+1))))),Int32:1)])}]"));
    VerifyRelease(EvalExpr_(&context, "true ? 42.0 : 'toto'"));
    VerifyRelease(EvalExpr_(&context, "((1 >= 3) ? Float:42 : (1+2)*3)"));
    VerifyRelease(EvalExpr_(&context, "Filename:\"c:/Windows/system32/notepad.exe\""));
    VerifyRelease(EvalExpr_(&context, "BinaryData:Any:\"ucFFy1fT2feXC4Ft6VM/FyFxt6MxDwP3zf0Ld6N3P+2NbTPZq5MRTQ==\""));
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
void Test_RTTI() {
    PPE_DEBUG_NAMEDSCOPE("Test_RTTI");

    PPE_LOG(Test_RTTI, Emphasis, "starting rtti tests ...");

    RTTI_MODULE(RTTI_UnitTest).Start();

    Test_Atoms_();
    Test_Any_();
    Test_CircularReferences_();
    Test_Grammar_();
    Test_Serialize_();
    Test_InteractiveConsole_();

    RTTI_MODULE(RTTI_UnitTest).Shutdown();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Test
} //!namespace PPE
