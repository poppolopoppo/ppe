#include "stdafx.h"

#include "RTTI.h"

#include "Diagnostic/Logger.h"

namespace PPE {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
LOG_CATEGORY(PPE_RTTI_API, RTTI);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE

#if USE_PPE_RTTI_CHECKS

#include "RTTI/Namespace.h"
#include "RTTI/Namespace-impl.h"

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
RTTI_NAMESPACE_DECL(, RTTI_UnitTest);
RTTI_NAMESPACE_DEF(, RTTI_UnitTest, MetaObject);
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
    ETutut Tutut;
    float Ratings;
    FString Name;
    float3 Position;
};
STATIC_ASSERT(struct_num_fields<FAnonymousStructAsTuple>() == 4);
RTTI_STRUCT_DEF(CONSTEXPR, FAnonymousStructAsTuple);
//----------------------------------------------------------------------------
#if 0//_DEBUG - just keep track of reverse work for debugging boost::pfr
template <class T, std::size_t N>
using TupleRTTI = details::enable_if_constructible_helper_t<T, N>;

template <class T, std::size_t N>
constexpr auto detect_fields_count_greedy_remember2(details::size_t_<N>) noexcept
-> TupleRTTI<T, N> {
    return N;
}

template <class T, std::size_t N>
constexpr std::size_t detect_fields_count2(details::size_t_<N>, details::size_t_<N>, long) noexcept {
    return N;
}

template <class T, std::size_t Begin, std::size_t Middle>
constexpr std::size_t detect_fields_count2(details::size_t_<Begin>, details::size_t_<Middle>, int) noexcept;

template <class T, std::size_t Begin, std::size_t Middle>
constexpr auto detect_fields_count2(details::size_t_<Begin>, details::size_t_<Middle>, long) noexcept
-> TupleRTTI<T, Middle> {
    return detect_fields_count2<T>(
        details::size_t_<Middle>{},
        details::size_t_<(Middle + (Middle - Begin + 1) / 2)>{}, 1L);
}
template <class T, std::size_t Begin, std::size_t Middle>
constexpr std::size_t detect_fields_count2(details::size_t_<Begin>, details::size_t_<Middle>, int) noexcept {
    return detect_fields_count2<T>(
        details::size_t_<Begin>{},
        details::size_t_<((Begin + Middle) / 2)>{}, 1L);
}

template <typename T, size_t Begin, size_t Middle>
struct detect_fields_count3_t {
    template <size_t X, class = details::enable_if_constructible_helper_t<T, X> >
    static constexpr size_t Next(long) {
        return detect_fields_count3_t<T, Middle, Middle + (Middle - Begin + 1) / 2>::value;
    }
    template <size_t X>
    static constexpr size_t Next(int) {
        return detect_fields_count3_t<T, Begin, (Begin + Middle) / 2>::value;
    }
    static constexpr size_t value = Next<Middle>(1L);
};

template <typename T, size_t N>
struct detect_fields_count3_t<T, N, N> {
    static constexpr size_t value = N;
};

STATIC_ASSERT(sizeof(details::enable_if_constructible_helper_t<FAnonymousStructAsTuple, 0>));
STATIC_ASSERT(sizeof(details::enable_if_constructible_helper_t<FAnonymousStructAsTuple, 1>));
STATIC_ASSERT(sizeof(details::enable_if_constructible_helper_t<FAnonymousStructAsTuple, 2>));
STATIC_ASSERT(sizeof(details::enable_if_constructible_helper_t<FAnonymousStructAsTuple, 3>));
STATIC_ASSERT(sizeof(details::enable_if_constructible_helper_t<FAnonymousStructAsTuple, 4>));
STATIC_ASSERT(sizeof(TestRTTI<FAnonymousStructAsTuple, 4>) == sizeof(size_t));
STATIC_ASSERT(detect_fields_count_greedy_remember2<FAnonymousStructAsTuple, 0>(details::size_t_<0>{}) == 0);
STATIC_ASSERT(detect_fields_count_greedy_remember2<FAnonymousStructAsTuple, 1>(details::size_t_<1>{}) == 1);
STATIC_ASSERT(detect_fields_count_greedy_remember2<FAnonymousStructAsTuple, 2>(details::size_t_<2>{}) == 2);
STATIC_ASSERT(detect_fields_count_greedy_remember2<FAnonymousStructAsTuple, 3>(details::size_t_<3>{}) == 3);
STATIC_ASSERT(detect_fields_count_greedy_remember2<FAnonymousStructAsTuple, 4>(details::size_t_<4>{}) == 4);
STATIC_ASSERT(details::detect_fields_count_greedy_remember<FAnonymousStructAsTuple, 0>(details::size_t_<0>{}, long(1)) == 0);
STATIC_ASSERT(details::detect_fields_count_greedy_remember<FAnonymousStructAsTuple, 1>(details::size_t_<1>{}, long(1)) == 1);
STATIC_ASSERT(details::detect_fields_count_greedy_remember<FAnonymousStructAsTuple, 2>(details::size_t_<2>{}, long(1)) == 2);
STATIC_ASSERT(details::detect_fields_count_greedy_remember<FAnonymousStructAsTuple, 3>(details::size_t_<3>{}, long(1)) == 3);
STATIC_ASSERT(details::detect_fields_count_greedy_remember<FAnonymousStructAsTuple, 4>(details::size_t_<4>{}, long(1)) == 4);
//STATIC_ASSERT(details::detect_fields_count_dispatch<FAnonymousStructAsTuple>(details::size_t_<8>{}, 1L, 1L) == 0);
STATIC_ASSERT(details::detect_fields_count_greedy<FAnonymousStructAsTuple>(details::size_t_<0>{}, details::size_t_<sizeof(FAnonymousStructAsTuple)>{}) == 4);
STATIC_ASSERT(detect_fields_count3_t<FAnonymousStructAsTuple, 0, sizeof(FAnonymousStructAsTuple)> ::value == 4);
STATIC_ASSERT(detect_fields_count2<FAnonymousStructAsTuple>(details::size_t_<4>{}, details::size_t_<4>{}, 1L) == 4);
STATIC_ASSERT(detect_fields_count2<FAnonymousStructAsTuple, 0, 4>(details::size_t_<0>{}, details::size_t_<4>{}, 1L) == 4);
STATIC_ASSERT(detect_fields_count2<FAnonymousStructAsTuple, 4, 5>(details::size_t_<4>{}, details::size_t_<5>{}, 1L) == 4);
//STATIC_ASSERT(detect_fields_count2<FAnonymousStructAsTuple, 0, sizeof(FAnonymousStructAsTuple) / 2 + 1>(details::size_t_<0>{}, details::size_t_<sizeof(FAnonymousStructAsTuple) / 2 + 1>{}, 1L) == 4);
STATIC_ASSERT(details::detect_fields_count<FAnonymousStructAsTuple>(details::size_t_<0>{}, details::size_t_<sizeof(FAnonymousStructAsTuple) / 2 + 1>{}, 1L) == 4);
STATIC_ASSERT(details::detect_fields_count_dispatch<FAnonymousStructAsTuple>(details::size_t_<8>{}, 1L, 1L) == 4);
#endif //!_DEBUG
//----------------------------------------------------------------------------
struct FAnonymousStructAsTuple2 {
    FAnonymousStructAsTuple Struct;
    FFilename File;
};
RTTI_STRUCT_DEF(CONSTEXPR, FAnonymousStructAsTuple2);
//----------------------------------------------------------------------------
class FTiti : public PPE::RTTI::FMetaObject {
public:
    FTiti() {}
    virtual ~FTiti() {}
    RTTI_CLASS_HEADER(FTiti, PPE::RTTI::FMetaObject);
    void Proc(int a, float b, const FString& c) { NOOP(a, b, c); }
    float Id(float f) { return f; }
    float IdDeprecated(float f) { return f; }
    void ProcConst(int a, float b, const FString& c) const { NOOP(a, b, c); }
    int Getter() { return 42; }
    int GetterConst() const { return 69;  }
    FString Func(float f) { return ToString(f); }
    FString FuncConst(float f) const { return ToString(f); }
    void Out(float f, FString& str) { str = ToString(f); }
    void OutConst(float f, FString& str) const { str = ToString(f); }
    RTTI::PMetaObject OutConstReturn(float f, FString& str) const { str = ToString(f); return PTiti(NEW_RTTI(FTiti)()); }
    void SetToto(FToto* toto) { _toto = toto; }
    const PToto& Toto() const { return _toto; }
private:
    int _count;
    PPE::FString _name;
    FAnonymousStructAsTuple _structAsTuple;
    TVector<FAnonymousStructAsTuple> _structAsTupleVector;
    FAnonymousStructAsTuple2 _structAsTuple2;
    VECTOR(NativeTypes, PTiti) _tities;
    VECTOR(NativeTypes, PTiti) _titiesOld;
    VECTOR(NativeTypes, PCTiti) _consttities;
    ASSOCIATIVE_VECTOR(NativeTypes, PPE::TPair<int COMMA PTiti>, VECTORINSITU(NativeTypes, PPE::TPair<float COMMA PPE::FString>, 2)) _dict;
    PToto _toto;
    ETutut _tutut;
};
RTTI_CLASS_BEGIN(RTTI_UnitTest, FTiti, RTTI::EClassFlags::Public)
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
RTTI_FUNCTION(Id, f)
RTTI_FUNCTION_DEPRECATED(IdDeprecated, f)
RTTI_FUNCTION(Func, f)
RTTI_FUNCTION(FuncConst, f)
RTTI_FUNCTION(Proc, a, b, c)
RTTI_FUNCTION(ProcConst, a, b, c)
RTTI_FUNCTION(Getter)
RTTI_FUNCTION(GetterConst)
RTTI_FUNCTION(Out, f, str)
RTTI_FUNCTION(OutConst, f, str)
RTTI_FUNCTION(OutConstReturn, f, str)
RTTI_CLASS_END()
//----------------------------------------------------------------------------
class FToto : public RTTI::FMetaObject {
public:
    FToto() {}
    virtual ~FToto() {}
    RTTI_CLASS_HEADER(FToto, RTTI::FMetaObject);
private:
    typedef TPair<int, FString> value_type;
    int _count;
    FString _name;
    VECTOR(NativeTypes, PTiti) _tities;
    VECTORINSITU(NativeTypes, PTiti, 2) _tities2;
    VECTOR(NativeTypes, FString) _titles;
    TRefPtr<FToto> _parent;
    TPair<int, int> _pair;
    TPair<float, float> _fpair;
    TPair<TPair<int, int>, TPair<int, int> > _vpair;
    TPair<TPair<int, int>, TPair<int, float> > _vpair2;
    HASHMAP(NativeTypes, FString, float) _dict;
    HASHMAP(NativeTypes, FString, value_type) _dict2;
    HASHMAP(NativeTypes, FString, float) _dict3;
};
RTTI_CLASS_BEGIN(RTTI_UnitTest, FToto, RTTI::EClassFlags::Public)
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
    virtual ~FToto2() {}
    void SetParent1(FToto* parent) { _parent1 = parent; }
    const PToto& Parent1() const { return _parent1; }
    RTTI_CLASS_HEADER(FToto2, FToto);
private:
    PToto _parent1;
};
RTTI_CLASS_BEGIN(RTTI_UnitTest, FToto2, RTTI::EClassFlags::Public)
RTTI_PROPERTY_PRIVATE_FIELD(_parent1)
RTTI_CLASS_END()
//----------------------------------------------------------------------------
template <typename T>
static void RTTIPrintType_() {
    const RTTI::PTypeTraits traits = RTTI::MakeTraits<T>();
    const RTTI::FTypeInfos typeInfos = traits->TypeInfos();
    STACKLOCAL_ATOM(defaultValue, traits);
    LOG(RTTI_UnitTest, Debug, L"Id = {0}, Name = {1}, Flags = {2}, Default = {3}",
        typeInfos.Id(),
        typeInfos.Name(),
        typeInfos.Flags(),
        defaultValue.MakeAtom() );
}
//----------------------------------------------------------------------------
template <typename T>
static void RTTIPrintClass_() {
    TRefPtr<T> t(NEW_RTTI(T)());
    const RTTI::FMetaClass *metaClass = t->RTTI_Class();

    LOG(RTTI_UnitTest, Debug, L"TMetaClass<{0}> : {1}", metaClass->Name(), metaClass->Flags());

    for (const auto& it : metaClass->AllFunctions()) {
        LOG(RTTI_UnitTest, Debug, L"   - FUNC {0} {1}({2}) : <{3}>",
            it->HasReturnValue() ? it->Result()->TypeInfos().Name() : "void",
            it->Name(),
            Fmt::FWFormator([&it](FWTextWriter& oss) {
                const auto& prms = it->Parameters();
                forrange(i, 0, prms.size()) {
                    if (i > 0) oss << L", ";
                    oss << prms[i].Name() << L" : " << prms[i].Traits()->TypeInfos().Name() << L" <" << prms[i].Flags() << L'>';
                }
            }),
            it->Flags()
        );
    }

    for (const auto& it : metaClass->AllProperties()) {
        const RTTI::FTypeInfos typeInfos = it->Traits()->TypeInfos();
        LOG(RTTI_UnitTest, Debug, L"   - PROP {0} : <{1}> -> {2} = {3} [{4}]",
            it->Name(),
            it->Flags(),
            typeInfos.Name(),
            typeInfos.SizeInBytes(),
            typeInfos.Id()
        );
    }
}
//----------------------------------------------------------------------------
static void TestRTTI_() {
    using namespace PPE;

    RTTI_NAMESPACE(RTTI_UnitTest).Start();

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
    RTTIPrintType_< VECTOR(NativeTypes, PTiti) >();
    RTTIPrintType_< HASHMAP(NativeTypes, int, int) >();
    RTTIPrintType_< HASHMAP(NativeTypes, FString, PTiti) >();
    RTTIPrintType_< ASSOCIATIVE_VECTOR(NativeTypes, FString, PTiti) >();
    //RTTIPrintType_< HASHSET(RTTI, FString) >();
    RTTIPrintType_< FAnonymousStructAsTuple >();
    RTTIPrintType_< TVector<FAnonymousStructAsTuple> >();
    RTTIPrintType_< FAnonymousStructAsTuple2 >();

    RTTIPrintType_< ETutut >();

    RTTIPrintClass_<FTiti>();
    RTTIPrintClass_<FToto>();
    RTTIPrintClass_<FToto2>();

    {
        const RTTI::FMetaEnum* metaEnum = RTTI::MetaEnum<ETutut>();

        const auto defaultValue = metaEnum->DefaultValue();
        LOG(RTTI_UnitTest, Debug, L"Enum<{0}> : {1} = {2} ({3})",
            metaEnum->Name(),
            defaultValue.Name, defaultValue.Value,
            metaEnum->Flags() );

        for (const auto it : metaEnum->Values()) {
            LOG(RTTI_UnitTest, Debug, L"  -- '{0}' = {1}", it.Name, it.Value);

            AssertRelease(metaEnum->NameToValue(it.Name).Value == it.Value);
            AssertRelease(metaEnum->ValueToName(it.Value).Name == it.Name);
        }

        ETutut tutut = ETutut::Maybe;
        RTTI::FAtom atom = RTTI::MakeAtom(&tutut);

        RTTI::FMetaEnum::FExpansion expanded;
        VerifyRelease(RTTI_ETutut::Get()->ExpandValues(atom, &expanded));
        LOG(RTTI_UnitTest, Debug, L"Expanded enum {0} = {1}", atom, Fmt::Join(expanded.MakeView(), L'|'));

        const RTTI::FMetaEnumValue* v;

        RTTI::FName whyNot("Whynot");
        v = RTTI_ETutut::Get()->NameToValueIFP(whyNot);
        AssertRelease(v);

        RTTI_ETutut::Get()->SetValue(atom, *v);
        AssertRelease(tutut == ETutut::Whynot);

        expanded.clear();
        VerifyRelease(RTTI_ETutut::Get()->ExpandValues(atom, &expanded));
        LOG(RTTI_UnitTest, Debug, L"Expanded enum {0} = {1}", atom, Fmt::Join(expanded.MakeView(), L'|'));

        v = RTTI_ETutut::Get()->ValueToNameIFP(i64(ETutut::Nope));
        AssertRelease(v);

        RTTI_ETutut::Get()->SetValue(atom, *v);
        AssertRelease(tutut == ETutut::Nope);

        VerifyRelease(RTTI_ETutut::Get()->ExpandValues(atom, &expanded));
        LOG(RTTI_UnitTest, Debug, L"Expanded enum {0} = {1}", atom, Fmt::Join(expanded.MakeView(), L'|'));

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
        RTTI::FMetaTransaction transaction(RTTI::FName("test"));

        PToto toto(NEW_RTTI(FToto)());
        PToto2 toto2(NEW_RTTI(FToto2)());
        PToto2 toto3(NEW_RTTI(FToto2)());

        transaction.RegisterObject(toto.get());
        transaction.RegisterObject(toto2.get());

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

            LOG(RTTI_UnitTest, Debug, L"toto2 = {0}", InplaceAtom(toto2));
        }

        {
            RTTI::FMetaTransaction transaction2(RTTI::FName("test2"));

            PTiti titi(NEW_RTTI(FTiti)());

            titi->RTTI_Export(RTTI::FName("titi"));
            titi->SetToto(toto3.get());

            transaction2.RegisterObject(titi.get());
            transaction2.Load();

            const RTTI::FMetaClass* metaClass = titi->RTTI_Class();

            if (RTTI::Cast<FToto>(titi.get()))
                AssertNotReached();

            {
                auto prop = metaClass->Property(RTTI::FName("Dict"));
                auto value = prop.Get(*titi);
                LOG(RTTI_UnitTest, Debug, L"{0} = {1}", prop.Name(), value);
            }
            {
                auto func = metaClass->Function(RTTI::FName("OutConstReturn"));
                STACKLOCAL_ATOM(result, func.Result());
                FString string;
                func.Invoke(*titi, result, { RTTI::InplaceAtom(42.0f), RTTI::MakeAtom(&string) });
                LOG(RTTI_UnitTest, Debug, L"{0} : {1} = {2}", func.Name(), string, result);
            }
            {
                auto func = metaClass->Function(RTTI::FName("IdDeprecated"));
                STACKLOCAL_ATOM(result, func.Result());
                func.Invoke(*titi, result, { RTTI::InplaceAtom(69.0f) });
                LOG(RTTI_UnitTest, Debug, L"{0}({1}) = {2}", func.Name(), 69.0f, result);
            }

            transaction2.Unload();
        }

        transaction.Unload();
    }

    RTTI_NAMESPACE(RTTI_UnitTest).Shutdown();
}
//----------------------------------------------------------------------------
//} //!namespace
//----------------------------------------------------------------------------
namespace RTTI {
void RTTI_UnitTests() {
    LOG(RTTI_UnitTest, Debug, L"begin unit tests");
    TestRTTI_();
    LOG(RTTI_UnitTest, Debug, L"end unit tests");
}
} //!namespace RTTI
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!USE_PPE_RTTI_CHECKS
