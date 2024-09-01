// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "RTTI.h"

#include "Diagnostic/Logger.h"

#if USE_PPE_RTTI_CHECKS

#include "RTTI/Module.h"
#include "RTTI/Module-impl.h"

#include "RTTI/Any.h"
#include "RTTI/Atom.h"
#include "RTTI/AtomVisitor.h"
#include "RTTI/Macros.h"
#include "RTTI/Macros-impl.h"

#include "MetaClass.h"
#include "MetaEnum.h"
#include "MetaObject.h"
#include "MetaProperty.h"
#include "MetaTransaction.h"

#include "Container/Pair.h"
#include "Container/TupleTie.h"
#include "Diagnostic/CurrentProcess.h"
#include "IO/Dirpath.h"
#include "IO/Filename.h"
#include "IO/Format.h"
#include "IO/FormatHelpers.h"
#include "IO/String.h"
#include "IO/StringBuilder.h"
#include "Maths/ScalarMatrix.h"
#include "Maths/ScalarVector.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Some tests to check template instantiations
//----------------------------------------------------------------------------
//namespace {
//----------------------------------------------------------------------------
LOG_CATEGORY(PPE_RTTI_API, RTTI_UnitTest)
RTTI_MODULE_DECL(, RTTI_UnitTest);
RTTI_MODULE_DEF(, RTTI_UnitTest, MetaObject);
//----------------------------------------------------------------------------
FWD_REFPTR(Titi);
FWD_REFPTR(Toto);
//----------------------------------------------------------------------------
enum class ETutut {
    Nope,
    Wep,
    Whynot,
    Maybe,
};
RTTI_ENUM_HEADER(PPE_RTTI_API, ETutut);
RTTI_ENUM_BEGIN(RTTI_UnitTest, ETutut)
RTTI_ENUM_VALUE(Nope)
RTTI_ENUM_VALUE(Wep)
RTTI_ENUM_VALUE(Whynot)
RTTI_ENUM_VALUE(Maybe)
RTTI_ENUM_END()
//----------------------------------------------------------------------------
struct FAnonymousStructAsTuple {
    ETutut Tutut{ ETutut::Maybe };
    float Ratings;
    FString Name;
    float3 Position;
};
STATIC_ASSERT(struct_num_fields<FAnonymousStructAsTuple>() == 4);
RTTI_STRUCT_DECL(CONSTEXPR, FAnonymousStructAsTuple);
RTTI_STRUCT_DEF(CONSTEXPR, FAnonymousStructAsTuple);
//----------------------------------------------------------------------------
struct FAnonymousStructAsTuple2 {
    FAnonymousStructAsTuple Struct;
    FFilename File;
    RTTI::PMetaObject Obj;
};
RTTI_STRUCT_DECL(CONSTEXPR, FAnonymousStructAsTuple2);
RTTI_STRUCT_DEF(CONSTEXPR, FAnonymousStructAsTuple2);
//----------------------------------------------------------------------------
class FTiti : public PPE::RTTI::FMetaObject {
public:
    FTiti() {}
    virtual ~FTiti() = default;
    RTTI_CLASS_HEADER(, FTiti, PPE::RTTI::FMetaObject);
    void Proc(int, float, const FString&) { NOOP(); }
    float Id(float f) { return f; }
    float IdDeprecated(float f) { return f; }
    void ProcConst(int, float, const FString&) const { NOOP(); }
    int Getter() { return 42; }
    int GetterConst() const { return 69;  }
    FString Func(float f) { return ToString(f); }
    FString FuncConst(float f) const { return ToString(f); }
    void Out(float f, FString& str) { str = ToString(f); }
    void OutConst(float f, FString& str) const { str = ToString(f); }
    RTTI::PMetaObject OutConstReturn(float f, FString& str) const { str = ToString(f); return NEW_RTTI(FTiti); }
    void SetToto(FToto* toto) { _toto.reset(toto); }
    const PToto& Toto() const { return _toto; }
private:
    int _count;
    PPE::FString _name;
    FAnonymousStructAsTuple _structAsTuple;
    TVector<FAnonymousStructAsTuple> _structAsTupleVector;
    FAnonymousStructAsTuple2 _structAsTuple2;
    VECTOR(UnitTest, PTiti) _tities;
    VECTOR(UnitTest, PTiti) _titiesOld;
    VECTOR(UnitTest, PCTiti) _consttities;
    ASSOCIATIVE_VECTOR(UnitTest, PPE::TPair<int COMMA PTiti>, VECTORINSITU(UnitTest, PPE::TPair<float COMMA PPE::FString>, 2)) _dict;
    PToto _toto;
    ETutut _tutut;
    RTTI::TRawData<float4> _positions;
};
RTTI_CLASS_BEGIN(RTTI_UnitTest, FTiti, Public, Concrete)
RTTI_PROPERTY_PRIVATE_FIELD(_count)
RTTI_PROPERTY_FIELD_ALIAS(_name, Name)
RTTI_PROPERTY_PRIVATE_FIELD(_structAsTuple)
RTTI_PROPERTY_PRIVATE_FIELD(_structAsTupleVector)
RTTI_PROPERTY_PRIVATE_FIELD(_structAsTuple2)
RTTI_PROPERTY_PRIVATE_FIELD(_tities)
RTTI_PROPERTY_PRIVATE_DEPRECATED(_titiesOld)
RTTI_PROPERTY_PRIVATE_FIELD(_consttities)
RTTI_PROPERTY_PRIVATE_FIELD(_dict)
RTTI_PROPERTY_PRIVATE_FIELD(_toto)
RTTI_PROPERTY_PRIVATE_FIELD(_tutut)
RTTI_PROPERTY_PRIVATE_FIELD(_positions)
RTTI_FUNCTION(Id, (f))
RTTI_FUNCTION_DEPRECATED(IdDeprecated, (f))
RTTI_FUNCTION(Func, (f))
RTTI_FUNCTION(FuncConst, (f))
RTTI_FUNCTION(Proc, (a, b, c))
RTTI_FUNCTION(ProcConst, (a, b, c))
RTTI_FUNCTION(Getter, ())
RTTI_FUNCTION(GetterConst, ())
RTTI_FUNCTION(Out, (f, str))
RTTI_FUNCTION(OutConst, (f, str))
RTTI_FUNCTION(OutConstReturn, (f, str))
RTTI_CLASS_END()
//----------------------------------------------------------------------------
class FToto : public RTTI::FMetaObject {
public:
    FToto() {}
    virtual ~FToto() = default;
    RTTI_CLASS_HEADER(, FToto, RTTI::FMetaObject);
private:
    typedef TPair<int, FString> value_type;
    int _count;
    FString _name;
    VECTOR(UnitTest, PTiti) _tities;
    VECTORINSITU(UnitTest, PTiti, 2) _tities2;
    VECTOR(UnitTest, FString) _titles;
    TRefPtr<FToto> _parent;
    TPair<int, int> _pair;
    TPair<float, float> _fpair;
    TPair<TPair<int, int>, TPair<int, int> > _vpair;
    TPair<TPair<int, int>, TPair<int, float> > _vpair2;
    HASHMAP(UnitTest, FString, float) _dict;
    HASHMAP(UnitTest, FString, value_type) _dict2;
    HASHMAP(UnitTest, FString, float) _dict3;
};
RTTI_CLASS_BEGIN(RTTI_UnitTest, FToto, Public)
RTTI_PROPERTY_FIELD_ALIAS(_count, Count)
RTTI_PROPERTY_FIELD_ALIAS(_name, Name)
RTTI_PROPERTY_FIELD_ALIAS(_tities, Tities)
RTTI_PROPERTY_FIELD_ALIAS(_tities2, Tities2)
RTTI_PROPERTY_FIELD_ALIAS(_titles, Titles)
RTTI_PROPERTY_FIELD_ALIAS(_parent, Parent)
RTTI_PROPERTY_FIELD_ALIAS(_pair, Pair)
RTTI_PROPERTY_FIELD_ALIAS(_fpair, FPair)
RTTI_PROPERTY_FIELD_ALIAS(_vpair, VPair)
RTTI_PROPERTY_FIELD_ALIAS(_vpair2, VPair2)
RTTI_PROPERTY_FIELD_ALIAS(_dict, Dict)
RTTI_PROPERTY_FIELD_ALIAS(_dict2, Dict2)
RTTI_PROPERTY_FIELD_ALIAS(_dict3, Dict3)
RTTI_CLASS_END()
//----------------------------------------------------------------------------
FWD_REFPTR(Toto2);
class FToto2 : public FToto {
public:
    FToto2() {}
    virtual ~FToto2() = default;
    void SetParent1(FToto* parent) { _parent1.reset(parent); }
    const PToto& Parent1() const { return _parent1; }
    RTTI_CLASS_HEADER(, FToto2, FToto);
private:
    PToto _parent1;
};
RTTI_CLASS_BEGIN(RTTI_UnitTest, FToto2, Public)
RTTI_PROPERTY_PRIVATE_FIELD(_parent1)
RTTI_CLASS_END()
//----------------------------------------------------------------------------
template <typename T>
static void RTTIPrintType_() {
#if USE_PPE_LOGGER
    const RTTI::PTypeTraits traits = RTTI::MakeTraits<T>();
    const RTTI::FNamedTypeInfos typeInfos = traits->NamedTypeInfos();
    STACKLOCAL_ATOM(defaultValue, traits);
    PPE_LOG(RTTI_UnitTest, Debug, "Id = {0}, Name = {1}, Flags = {2}, Default = {3}",
        typeInfos.Id(),
        typeInfos.Name(),
        typeInfos.Flags(),
        defaultValue.MakeAtom() );
#endif
}
//----------------------------------------------------------------------------
template <typename T>
static void RTTIPrintClass_() {
#if USE_PPE_LOGGER
    TRefPtr<T> t(NEW_RTTI(T));
    const RTTI::FMetaClass *metaClass = t->RTTI_Class();

    PPE_LOG(RTTI_UnitTest, Debug, "TMetaClass<{0}> : {1}", metaClass->Name(), metaClass->Flags());

    for (const auto& it : metaClass->AllFunctions()) {
        PPE_LOG(RTTI_UnitTest, Verbose, "   - FUNC {0} {1}({2}) : <{3}>",
            it->HasReturnValue() ? it->Result()->TypeName() : "void",
            it->Name(),
            FTextManipulator([&it](FTextWriter& oss) -> FTextWriter& {
                const auto& prms = it->Parameters();
                forrange(i, 0, prms.size()) {
                    if (i > 0) oss << ", ";
                    oss << prms[i].Name() << " : " << prms[i].Traits()->TypeName() << " <" << prms[i].Flags() << '>';
                }
                return oss;
            }),
            it->Flags() );
    }

    for (const auto& it : metaClass->AllProperties()) {
        const RTTI::FNamedTypeInfos typeInfos = it->Traits()->NamedTypeInfos();
		PPE_LOG(RTTI_UnitTest, Verbose, "   - PROP {0} : <{1}> -> {2} = {3} [{4}]",
            it->Name(),
            it->Flags(),
            typeInfos.Name(),
            typeInfos.SizeInBytes(),
            typeInfos.Id()
        );
    }
#endif
}
//----------------------------------------------------------------------------
static NO_INLINE void TestRTTI_() {
    using namespace PPE;

    RTTI_MODULE(RTTI_UnitTest).Start();

#define DECL_RTTI_NATIVETYPE_PRINT(_Name, T, _TypeId) RTTIPrintType_< T >();
    FOREACH_RTTI_NATIVETYPES(DECL_RTTI_NATIVETYPE_PRINT)
#undef DECL_RTTI_NATIVETYPE_PRINT

    RTTIPrintType_< TVector<int> >();
    RTTIPrintType_< TPair<RTTI::FName, float> >();
    RTTIPrintType_< PTiti >();
    RTTIPrintType_< TPair<FWString, PTiti> >();
    RTTIPrintType_< uword2 >();
    RTTIPrintType_< float3 >();
    RTTIPrintType_< float3x3 >();
    RTTIPrintType_< VECTOR(UnitTest, PTiti) >();
    RTTIPrintType_< HASHMAP(UnitTest, int, int) >();
    RTTIPrintType_< HASHMAP(UnitTest, FString, PTiti) >();
    RTTIPrintType_< ASSOCIATIVE_VECTOR(UnitTest, FString, PTiti) >();
    //RTTIPrintType_< HASHSET(RTTI, FString) >();
    RTTIPrintType_< FAnonymousStructAsTuple >();
    RTTIPrintType_< TVector<FAnonymousStructAsTuple> >();
    RTTIPrintType_< FAnonymousStructAsTuple2 >();

    RTTIPrintType_< ETutut >();

    RTTIPrintClass_<FTiti>();
    RTTIPrintClass_<FToto>();
    RTTIPrintClass_<FToto2>();

    {
        STATIC_ASSERT(struct_num_fields<RTTI::FPathName>() == 2);
        STATIC_ASSERT(has_tie_as_tuple<RTTI::FPathName>());

        RTTIPrintType_<RTTI::FPathName>();
    }

    {
        const RTTI::FMetaEnum* metaEnum = RTTI::MetaEnum<ETutut>();

        const auto defaultValue = metaEnum->DefaultValue();
        Unused(defaultValue);

        PPE_LOG(RTTI_UnitTest, Debug, "Enum<{0}> : {1} = {2} ({3})",
            metaEnum->Name(),
            defaultValue.Name, defaultValue.Ord,
            metaEnum->Flags() );

        for (const auto& it : metaEnum->Values()) {
            PPE_LOG(RTTI_UnitTest, Debug, "  -- '{0}' = {1}", it.Name, it.Ord);

            AssertRelease(metaEnum->NameToValue(it.Name).Ord == it.Ord);
            AssertRelease(metaEnum->ValueToName(it.Ord).Name == it.Name);
        }

        ETutut tutut = ETutut::Maybe;
        RTTI::FAtom atom = RTTI::MakeAtom(&tutut);

        RTTI::FMetaEnum::FExpansion expanded;
        VerifyRelease(RTTI_ETutut::Get()->ExpandValues(atom, &expanded));
        PPE_LOG(RTTI_UnitTest, Debug, "Expanded enum {0} = {1}", atom, Fmt::Join(expanded.MakeView(), L'|'));

        const RTTI::FMetaEnumValue* v;

        RTTI::FName whyNot("Whynot");
        v = RTTI_ETutut::Get()->NameToValueIFP(whyNot);
        AssertRelease(v);

        RTTI_ETutut::Get()->SetValue(atom, *v);
        AssertRelease(tutut == ETutut::Whynot);

        expanded.clear();
        VerifyRelease(RTTI_ETutut::Get()->ExpandValues(atom, &expanded));
        PPE_LOG(RTTI_UnitTest, Debug, "Expanded enum {0} = {1}", atom, Fmt::Join(expanded.MakeView(), L'|'));

        v = RTTI_ETutut::Get()->ValueToNameIFP(i64(ETutut::Nope));
        AssertRelease(v);

        RTTI_ETutut::Get()->SetValue(atom, *v);
        AssertRelease(tutut == ETutut::Nope);

        VerifyRelease(RTTI_ETutut::Get()->ExpandValues(atom, &expanded));
        PPE_LOG(RTTI_UnitTest, Debug, "Expanded enum {0} = {1}", atom, Fmt::Join(expanded.MakeView(), L'|'));

        RTTI::FAtom nameEnum = MakeAtom(&whyNot);
        VerifyRelease(nameEnum.PromoteMove(atom));
        AssertRelease(tutut == ETutut::Whynot);

        tutut = ETutut::Wep;

        VerifyRelease(atom.PromoteCopy(nameEnum));
        AssertRelease(whyNot == "Wep");

        i64 i = 0;
        RTTI::FAtom intEnum = RTTI::MakeAtom(&i);
        VerifyRelease(atom.PromoteCopy(intEnum));
        AssertRelease(i64(ETutut::Wep) == i);
    }

    {
        RTTI::FMetaTransaction transaction(
            RTTI::FName("test"),
            RTTI::ETransactionFlags::KeepIsolated );

        PToto toto(NEW_RTTI(FToto));
        PToto2 toto2(NEW_RTTI(FToto2));
        PToto2 toto3(NEW_RTTI(FToto2));

        transaction.Add(toto.get());
        transaction.Add(toto2.get());
        transaction.Add(toto3.get());

        toto->RTTI_Export(RTTI::FName("toto"));
        toto3->RTTI_Export(RTTI::FName("toto3"));

        toto2->SetParent1(toto3.get());

        transaction.Load();

        if (not RTTI::Cast<FToto>(toto.get()))
            AssertNotReached();
        if (not RTTI::Cast<FToto>(toto2.get()))
            AssertNotReached();
        if (not RTTI::Cast<FToto2>(toto2.get()))
            AssertNotReached();

        {
            const RTTI::FMetaClass* metaClass = toto2->RTTI_Class();
            metaClass->Property(RTTI::FName("Parent1")).CopyFrom(*toto2, MakeAtom(&toto));

            PPE_LOG(RTTI_UnitTest, Debug, "toto2 = {0}", InplaceAtom(toto2));
        }

        {
            RTTI::FMetaTransaction transaction2(
                RTTI::FName("test2"),
                RTTI::ETransactionFlags::KeepIsolated );

            PTiti titi(NEW_RTTI(FTiti));

            titi->RTTI_Export(RTTI::FName("titi"));
            titi->SetToto(toto3.get());

            transaction2.Add(titi.get());
            transaction2.Load();

            const RTTI::FMetaClass* metaClass = titi->RTTI_Class();

            if (RTTI::Cast<FToto>(titi.get()))
                AssertNotReached();

            {
                const auto& prop = metaClass->Property(RTTI::FName("Dict"));
                const auto value = prop.Get(*titi);
                Unused(value);

                PPE_LOG(RTTI_UnitTest, Debug, "{0} = {1}", prop.Name(), value);
            }
            {
                const auto& func = metaClass->Function(RTTI::FName("OutConstReturn"));
                STACKLOCAL_ATOM(result, func.Result());
                FString string;
                func.Invoke(*titi, result, { RTTI::InplaceAtom(42.0f), RTTI::MakeAtom(&string) });
                PPE_LOG(RTTI_UnitTest, Debug, "{0} : {1} = {2}", func.Name(), string, result);
            }
            {
                const auto& func = metaClass->Function(RTTI::FName("IdDeprecated"));
                STACKLOCAL_ATOM(result, func.Result());
                func.Invoke(*titi, result, { RTTI::InplaceAtom(69.0f) });
                PPE_LOG(RTTI_UnitTest, Debug, "{0}({1}) = {2}", func.Name(), 69.0f, result);
            }

            transaction2.Unload();
        }

        transaction.Unload();
    }

    RTTI_MODULE(RTTI_UnitTest).Shutdown();
}
//----------------------------------------------------------------------------
//} //!namespace
//----------------------------------------------------------------------------
namespace RTTI {
void RTTI_UnitTests() {
    const auto& process = FCurrentProcess::Get();
    if (not process.HasArgument(L"-NoUnitTest")) {
        PPE_LOG(RTTI_UnitTest, Debug, "begin unit tests");
        TestRTTI_();
        PPE_LOG(RTTI_UnitTest, Debug, "end unit tests");
    }
}
} //!namespace RTTI
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!USE_PPE_RTTI_CHECKS
