#include "stdafx.h"

#include "RTTI.h"
#include "RTTI_fwd.h"
#include "RTTI_Namespace.h"
#include "RTTI_Namespace-impl.h"

#include "AtomVisitor.h"
#include "MetaDatabase.h"
#include "MetaObjectHelpers.h"
#include "MetaTransaction.h"

#include "Core/Allocator/PoolAllocatorTag-impl.h"
#include "Core/Diagnostic/Logger.h"
#include "Core/IO/StringBuilder.h"
#include "Core/IO/TextWriter.h"

#if !(defined(FINAL_RELEASE) || defined(PROFILING_ENABLED))
#   define WITH_RTTI_UNITTESTS //%_NOCOMMIT%
#endif

PRAGMA_INITSEG_LIB

namespace Core {
namespace RTTI {
POOL_TAG_DEF(RTTI);
RTTI_NAMESPACE_DEF(CORE_RTTI_API, RTTI);
LOG_CATEGORY(CORE_RTTI_API, RTTI)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifdef WITH_RTTI_UNITTESTS
static void RTTI_UnitTests();
#endif
void FRTTIModule::Start() {
    CORE_MODULE_START(RTTI);

    POOL_TAG(RTTI)::Start();

    FName::Start();

    FMetaDatabase::Create();

    RTTI_NAMESPACE(RTTI).Start();

#ifdef WITH_RTTI_UNITTESTS
    RTTI_UnitTests();
#endif
}
//----------------------------------------------------------------------------
void FRTTIModule::Shutdown() {
    CORE_MODULE_SHUTDOWN(RTTI);

    RTTI_NAMESPACE(RTTI).Shutdown();

    FMetaDatabase::Destroy();

    FName::Shutdown();

    POOL_TAG(RTTI)::Shutdown();
}
//----------------------------------------------------------------------------
void FRTTIModule::Clear() {
    CORE_MODULE_CLEARALL(RTTI);

    POOL_TAG(RTTI)::ClearAll_UnusedMemory();
}
//----------------------------------------------------------------------------
void FRTTIModule::ClearAll_UnusedMemory() {
    CORE_MODULE_CLEARALL(RTTI);

    POOL_TAG(RTTI)::ClearAll_UnusedMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core

#ifdef WITH_RTTI_UNITTESTS

#include "RTTI_fwd.h"
#include "RTTI_Namespace-impl.h"
#include "Atom.h"
#include "AtomVisitor.h"
#include "MetaObject.h"
#include "MetaProperty.h"
#include "RTTI_Macros.h"
#include "RTTI_Macros-impl.h"

#include "Core/Container/Pair.h"
#include "Core/Diagnostic/Logger.h"
#include "Core/IO/Format.h"
#include "Core/IO/FormatHelpers.h"
#include "Core/IO/String.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Some tests to check template instantiations
//----------------------------------------------------------------------------
//namespace {
//----------------------------------------------------------------------------
LOG_CATEGORY(CORE_RTTI_API, RTTI_UnitTest)
RTTI_NAMESPACE_DECL(, RTTI_UnitTest);
RTTI_NAMESPACE_DEF(, RTTI_UnitTest);
//----------------------------------------------------------------------------
FWD_REFPTR(Titi);
FWD_REFPTR(Toto);
//----------------------------------------------------------------------------
class FTiti : public Core::RTTI::FMetaObject {
public:
    FTiti() {}
    virtual ~FTiti() {}
    RTTI_CLASS_HEADER(FTiti, Core::RTTI::FMetaObject);
    void Proc(int a, float b, const FString& c) {}
    float Id(float f) { return f; }
    float IdDeprecated(float f) { return f; }
    void ProcConst(int a, float b, const FString& c) const {}
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
    Core::FString _name;
    VECTOR(NativeTypes, PTiti) _tities;
    VECTOR(NativeTypes, PTiti) _titiesOld;
    VECTOR(NativeTypes, PCTiti) _consttities;
    ASSOCIATIVE_VECTOR(NativeTypes, Core::TPair<int COMMA PTiti>, VECTORINSITU(NativeTypes, Core::TPair<float COMMA Core::FString>, 2)) _dict;
    PToto _toto;
};
RTTI_CLASS_BEGIN(RTTI_UnitTest, FTiti, RTTI::EClassFlags::Public)
RTTI_PROPERTY_PRIVATE_FIELD(_count)
RTTI_PROPERTY_FIELD_ALIAS(_name, Name)
RTTI_PROPERTY_PRIVATE_FIELD(_tities)
RTTI_PROPERTY_PRIVATE_DEPRECATED(_titiesOld)
RTTI_PROPERTY_PRIVATE_FIELD(_consttities)
RTTI_PROPERTY_PRIVATE_FIELD(_dict)
RTTI_PROPERTY_PRIVATE_FIELD(_toto)
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
    using namespace Core;

    RTTI_NAMESPACE(RTTI_UnitTest).Start();

    RTTIPrintType_< int >();
    RTTIPrintType_< size_t >();
    RTTIPrintType_< FString >();
    RTTIPrintType_< TVector<int> >();
    RTTIPrintType_< TPair<RTTI::FName, float> >();
    RTTIPrintType_< PTiti >();
    RTTIPrintType_< TPair<FWString, PTiti> >();
    RTTIPrintType_< VECTOR(NativeTypes, PTiti) >();
    RTTIPrintType_< HASHMAP(NativeTypes, int, int) >();
    RTTIPrintType_< HASHMAP(NativeTypes, FString, PTiti) >();
    RTTIPrintType_< ASSOCIATIVE_VECTOR(NativeTypes, FString, PTiti) >();
    //RTTIPrintType_< HASHSET(RTTI, FString) >();

    RTTIPrintClass_<FTiti>();
    RTTIPrintClass_<FToto>();
    RTTIPrintClass_<FToto2>();

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

        const RTTI::FMetaClass* metaClass = toto2->RTTI_Class();
        metaClass->Property(RTTI::FName("Parent1")).CopyFrom(*toto2, MakeAtom(&toto));

        LOG(RTTI_UnitTest, Debug, L"toto2 = {0}", InplaceAtom(toto2));

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
static void RTTI_UnitTests() {
    LOG(RTTI_UnitTest, Debug, L"begin unit tests");
    TestRTTI_();
    LOG(RTTI_UnitTest, Debug, L"end unit tests");
}
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#endif //!WITH_RTTI_UNITTESTS
