#include "stdafx.h"

#include "MetaTransaction.h"
#include "RTTI/Any.h"
#include "RTTI/Atom.h"
#include "RTTI/AtomVisitor.h"
#include "RTTI/Macros.h"
#include "RTTI/Macros-impl.h"
#include "RTTI/Namespace.h"
#include "RTTI/Namespace-impl.h"
#include "RTTI/NativeTypes.h"
#include "RTTI/TypeInfos.h"

#include "Binary/BinarySerializer.h"
#include "Json/Json.h"
#include "Json/JsonSerializer.h"
#include "Text/TextSerializer.h"
#include "TransactionLinker.h"
#include "TransactionSaver.h"

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
#include "HAL/PlatformConsole.h"
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

#define USE_RTTI_TEST_YOLOTYPES 0 //%_NOCOMMIT%

#include <iostream> // Test_InteractiveConsole_

namespace PPE {
namespace Test {
LOG_CATEGORY(, Test_RTTI)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
RTTI_NAMESPACE_DECL(, RTTI_UnitTest);
RTTI_NAMESPACE_DEF(, RTTI_UnitTest, MetaObject);
//----------------------------------------------------------------------------
struct FStructAsTuple {
    float3 Position;
    FString Name;
    TVector<int> Weights;
};
RTTI_STRUCT_DEF(, FStructAsTuple);
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
    RTTI_CLASS_HEADER(FRTTITestSimple_, RTTI::FMetaObject);

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
RTTI_CLASS_BEGIN(RTTI_UnitTest, FRTTITestSimple_, RTTI::EClassFlags::Concrete)
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
enum class ETestEnum64 : u64 {
    A = 1 << 0, B = 1 << 1, C = 1 << 2
};
RTTI_ENUM_HEADER(, ETestEnum64);
RTTI_ENUM_BEGIN(RTTI_UnitTest, ETestEnum64)
RTTI_ENUM_VALUE(A)
RTTI_ENUM_VALUE(B)
RTTI_ENUM_VALUE(C)
RTTI_ENUM_END()
//----------------------------------------------------------------------------
FWD_REFPTR(RTTITestParent_);
class FRTTITestParent_ : public RTTI::FMetaObject {
public:
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
    FStructAsTuple _structAsTuple;
};
RTTI_CLASS_BEGIN(RTTI_UnitTest, FRTTITestParent_, RTTI::EClassFlags::Concrete)
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
    RTTI_CLASS_HEADER(FRTTITest_, FRTTITestParent_);

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
RTTI_CLASS_BEGIN(RTTI_UnitTest, FRTTITest_, RTTI::EClassFlags::Concrete)
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
#else
        if (_import && _rand.NextFloat01() < 0.7f)
            obj = _rand.RandomElement(_import->TopObjects());
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
        AssertRelease(atom);
        atom.Accept(this);
    }

    Assert(0 < _depth);
    --_depth;

    _import = nullptr;
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

    VECTOR(NativeTypes, float2) v2;
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
static NO_INLINE void Test_Serializer_(const RTTI::FMetaTransaction& input, Serialize::ISerializer& serializer, const FFilename& filename) {
    Assert(input.NumTopObjects());

    const FFilename fname_binz = filename.WithReplacedExtension(FFS::Z());
    const FFilename& fname_raw = filename;

    MEMORYSTREAM(NativeTypes) uncompressed;
    {
        Serialize::FTransactionSaver saver{ input, filename };
        serializer.Serialize(saver, &uncompressed);
#if 0
        auto compressed = VFS_OpenBinaryWritable(filename, EAccessPolicy::Truncate);
        LZJB::CompressMemory(compressed.get(), uncompressed.MakeView());
#else
        RAWSTORAGE(NativeTypes, u8) compressed;
        const size_t compressedSizeInBytes = Compression::CompressMemory(compressed, uncompressed.MakeView(), Compression::HighCompression);
        Assert(compressedSizeInBytes <= compressed.SizeInBytes());
        const TMemoryView<const u8> compressedView = compressed.MakeView().SubRange(0, compressedSizeInBytes);

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
        RAWSTORAGE(FileSystem, u8) compressed;
        if (false == VFS_ReadAll(&compressed, fname_binz, EAccessPolicy::Binary))
            AssertNotReached();

        RAWSTORAGE(Stream, u8) decompressed;
        if (false == Compression::DecompressMemory(decompressed, compressed.MakeConstView()))
            AssertNotReached();

        Assert(checked_cast<size_t>(uncompressed.SizeInBytes()) == decompressed.SizeInBytes());
        const size_t k = decompressed.SizeInBytes();
        for (size_t i = 0; i < k; ++i)
            Assert(uncompressed.Pointer()[i] == decompressed.Pointer()[i]);

        Serialize::FTransactionLinker linker{ &output, filename };
        Serialize::ISerializer::Deserialize(serializer, decompressed.MakeView(), &linker);
    }

    AssertRelease(input.NumTopObjects() == output.NumTopObjects());

    if (false == input.DeepEquals(output))
        AssertNotReached();
}
//----------------------------------------------------------------------------
static NO_INLINE void Test_InteractiveConsole_() {
#if !USE_PPE_PROFILING
    FLUSH_LOG();

    char buffer[1024];
    Parser::FParseContext globalContext(Meta::ForceInit);

    FPlatformConsole::Open();

    do
    {
        FPlatformConsole::Write("$>", FPlatformConsole::BLACK_ON_WHITE);
        FPlatformConsole::Write("  ", FPlatformConsole::WHITE_ON_BLACK);

        const size_t length = FPlatformConsole::Read(buffer);

        FStringView line{ buffer, length };
        line = Strip(Chomp(line));

        if (line.empty()) {
            continue;
        }
        else if (0 == CompareI(MakeStringView("exit"), line)) {
            break;
        }

        const wchar_t filename[] = L"@in_memory";

        try {
            FMemoryViewReader reader(line.Cast<const u8>());
            Lexer::FLexer lexer(&reader, filename, true);

            Parser::FParseList input;
            input.Parse(&lexer);

            Parser::PCParseExpression expr = Serialize::FGrammarStartup::ParseExpression(input);
            AssertRelease(expr);

            const RTTI::FAtom value = expr->Eval(&globalContext);
            Assert(value);

            FPlatformConsole::Write(StringFormat(" => {0}\n", value), FPlatformConsole::FG_GREEN | FPlatformConsole::FG_INTENSITY);
        }
        catch (const Parser::FParserException& e) {
            if (e.Item())
                FPlatformConsole::Write(StringFormat(" !! Parser error : <{0}> {1}, {2}.\n", e.Item()->ToString(), MakeCStringView(e.What()), e.Site()), FPlatformConsole::FG_RED);
            else
                FPlatformConsole::Write(StringFormat(" !! Parser error : {0}, {1}.\n", MakeCStringView(e.What()), e.Site()), FPlatformConsole::FG_RED | FPlatformConsole::FG_INTENSITY);
        }
        catch (const Lexer::FLexerException& e) {
            FPlatformConsole::Write(StringFormat(" ?? Lexer error : <{0}>: {1}, {2}.\n", e.Match().Symbol()->CStr(), MakeCStringView(e.What()), e.Match().Site()), FPlatformConsole::FG_YELLOW);
        }

    } while (true);

    FPlatformConsole::Close();
#endif //!!USE_PPE_PROFILING
}
//----------------------------------------------------------------------------
template <typename T>
static NO_INLINE void Test_TransactionSerialization_() {
#ifdef WITH_PPE_ASSERT
    static constexpr bool minify = false;
    static constexpr size_t test_count = 5;
#else
    static constexpr bool minify = true;
    static constexpr size_t test_count = 50;
#endif

    FRTTIAtomRandomizer_ rand(test_count, 0xabadcafedeadbeefull);

    RTTI::FMetaTransaction import(RTTI::FName(MakeStringView("UnitTest_Import")));
    {
        FWStringBuilder oss;

        forrange(i, 0, test_count) {
            TRefPtr<T> t = NEW_RTTI(T)();
            rand.Randomize(t.get());
            t->RTTI_Export(RTTI::FName(StringFormat("import_{0}", intptr_t(t.get()))));
            import.RegisterObject(t.get());
        }
    }
    const RTTI::FMetaTransaction::FLoadingScope ANONYMIZE(loadingScope)(import);
    RTTI::FMetaTransaction input(RTTI::FName(MakeStringView("UnitTest_Input")));
    {
        FWStringBuilder oss;

        forrange(i, 0, test_count) {
            TRefPtr<T> t = NEW_RTTI(T)();
            rand.Randomize(t.get(), &import);
            input.RegisterObject(t.get());
        }

        ReportAllTrackingData(); // inspect this transaction allocations
    }
    const RTTI::FMetaTransaction::FLoadingScope ANONYMIZE(loadingScope)(input);

    const FWString basePath = StringFormat(L"Saved:/RTTI/robotapp_{0}", RTTI::MetaClass<T>()->Name());

    {
        Serialize::PSerializer bin{ Serialize::FBinarySerializer::Get() };
        Test_Serializer_(input, *bin, basePath + L"_bin.bin");
    }
    {
        Serialize::PSerializer json{ Serialize::FJsonSerializer::Get() };
        json->SetMinify(minify);
        Test_Serializer_(input, *json, basePath + L"_json.json");
    }
    {
        Serialize::PSerializer text{ Serialize::FTextSerializer::Get() };
        text->SetMinify(minify);
        Test_Serializer_(input, *text, basePath + L"_text.txt");
    }
}
//----------------------------------------------------------------------------
static NO_INLINE void Test_Serialize_() {
    RTTI_NAMESPACE(RTTI_UnitTest).Start();
    {
        FStringBuilder serialized;

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
            FRTTIAtomRandomizer_ rand(128, 123456);

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
        Test_TransactionSerialization_<FRTTITestSimple_>();
        Test_TransactionSerialization_<FRTTITest_>();
    }

    RTTI_NAMESPACE(RTTI_UnitTest).Shutdown();
}
//----------------------------------------------------------------------------
static RTTI::FAtom EvalExpr_(Parser::FParseContext* context, const FStringView& input) {
    LOG(Test_RTTI, Info, L"EvalExpr('{0}') :", input);
    try
    {
        FMemoryViewReader reader(input.RawView());
        Lexer::FLexer lexer(&reader, L"@memory", true);

        Parser::FParseList parser;
        parser.Parse(&lexer);

        Parser::PCParseExpression expr = Serialize::FGrammarStartup::ParseExpression(parser);
        AssertRelease(expr);

        const RTTI::FAtom result = expr->Eval(context);

        LOG(Test_RTTI, Info, L" -> {0}", result);

        return result;
    }
    catch (const Parser::FParserException& e) {
        UNUSED(e);
        if (e.Item())
            LOG(Test_RTTI, Error, L" !! Parser error : <{0}> {1}, {2}.\n", e.Item()->ToString(), MakeCStringView(e.What()), e.Site());
        else
            LOG(Test_RTTI, Error, L" !! Parser error : {0}, {1}.\n", MakeCStringView(e.What()), e.Site());
    }
    catch (const Lexer::FLexerException& e) {
        UNUSED(e);
        LOG(Test_RTTI, Error, L" ?? Lexer error : <{0}>: {1}, {2}.\n", e.Match().Symbol()->CStr(), MakeCStringView(e.What()), e.Match().Site());
    }

    return RTTI::FAtom();
}
static void Test_Grammar_() {
    Parser::FParseContext context(Meta::ForceInit);
    EvalExpr_(&context, "(Any:\"toto\",Any:42,Any:3.456)");
    EvalExpr_(&context, "[Any:42]");
    EvalExpr_(&context, "Any:[Any:[Any:\"totototototototototototototoot\"]]");
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
void Test_RTTI() {
    LOG(Test_RTTI, Emphasis, L"starting rtti tests ...");

    Test_Atoms_();
    Test_Any_();
    Test_Grammar_();
    Test_Serialize_();
    Test_InteractiveConsole_();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Test
} //!namespace PPE
