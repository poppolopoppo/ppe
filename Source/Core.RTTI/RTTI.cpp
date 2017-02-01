#include "stdafx.h"

#include "RTTI.h"
#include "RTTI_fwd.h"
#include "RTTI_Tag.h"

#include "MetaAtomDatabase.h"
#include "MetaClassDatabase.h"
#include "MetaObject.h"
#include "MetaObjectHelpers.h"
#include "MetaType.h"

#include "Core/Allocator/PoolAllocatorTag-impl.h"

#if !(defined(FINAL_RELEASE) || defined(PROFILING_ENABLED))
//#   define WITH_RTTI_UNITTESTS %_NOCOMMIT%
#endif

#ifdef CPP_VISUALSTUDIO
#   pragma warning(disable: 4073) // initialiseurs placés dans la zone d'initialisation d'une bibliothèque
#   pragma init_seg(lib)
#else
#   error "missing compiler specific command"
#endif

namespace Core {
namespace RTTI {
POOL_TAG_DEF(RTTI);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifdef WITH_RTTI_UNITTESTS
static void RTTI_UnitTests();
#endif
void RTTIStartup::Start() {
    CORE_MODULE_START(RTTI);

    POOL_TAG(RTTI)::Start();

    FName::Start(2048);

    FMetaClassDatabase::Create();
    FMetaAtomDatabase::Create();

    RTTI_TAG(Default)::Start();


#ifdef WITH_RTTI_UNITTESTS
    RTTI_UnitTests();
#endif
}
//----------------------------------------------------------------------------
void RTTIStartup::Shutdown() {
    CORE_MODULE_SHUTDOWN(RTTI);

    RTTI_TAG(Default)::Shutdown();

    FMetaAtomDatabase::Destroy();
    FMetaClassDatabase::Destroy();

    FName::Shutdown();

    POOL_TAG(RTTI)::Shutdown();
}
//----------------------------------------------------------------------------
void RTTIStartup::Clear() {
    CORE_MODULE_CLEARALL(RTTI);

    FMetaAtomDatabase::Instance().Clear();
    FMetaClassDatabase::Instance().Clear();

    FName::Clear();

    POOL_TAG(RTTI)::ClearAll_UnusedMemory();
}
//----------------------------------------------------------------------------
void RTTIStartup::ClearAll_UnusedMemory() {
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
#include "RTTI_Tag-impl.h"
#include "MetaAtom.h"
#include "MetaObject.h"
#include "MetaProperty.h"
#include "RTTIMacros.h"
#include "RTTIMacros-impl.h"

#include "Core/IO/String.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Some tests to check template instantiations
//----------------------------------------------------------------------------
//namespace {
//----------------------------------------------------------------------------
RTTI_TAG_DECL(RTTI_UnitTest);
RTTI_TAG_DEF(RTTI_UnitTest);
//----------------------------------------------------------------------------
FWD_REFPTR(Titi);
class FTiti : public Core::RTTI::FMetaObject {
public:
    FTiti() {}
    virtual ~FTiti() {}
    RTTI_CLASS_HEADER(FTiti, Core::RTTI::FMetaObject);
    void Proc(int, float, const FString& ) {}
    float Id(float f) { return f; }
    void ProcConst(int, float, const FString&) const {}
    int Getter() { return 42; }
    int GetterConst() const { return 69;  }
    FString Func(float f) { return ToString(f); }
    FString FuncConst(float f) const { return ToString(f); }
    void Out(float f, FString& str) { str = ToString(f); }
    void OutConst(float f, FString& str) const { str = ToString(f); }
    RTTI::PMetaObject OutConstReturn(float f, FString& str) const { str = ToString(f); return PTiti(new FTiti()); }
private:
    int _count;
    Core::FString _name;
    VECTOR(Internal, PTiti) _tities;
    VECTOR(Internal, PCTiti) _consttities;
    //ASSOCIATIVE_VECTOR(Internal, Core::TPair<int COMMA PTiti>, VECTORINSITU(RTTI, Core::TPair<float COMMA Core::FString>, 2)) _dict;
};
RTTI_CLASS_BEGIN(RTTI_UnitTest, FTiti, Concrete)
RTTI_PROPERTY_PRIVATE_FIELD(_count)
RTTI_PROPERTY_FIELD_ALIAS(_name, Name)
RTTI_PROPERTY_PRIVATE_FIELD(_tities)
RTTI_PROPERTY_DEPRECATED(VECTOR(Internal, PTiti), Tities2)
RTTI_PROPERTY_PRIVATE_FIELD(_consttities)
//RTTI_PROPERTY_PRIVATE_FIELD(_dict)
RTTI_FUNCTION(Id)
RTTI_FUNCTION(Func)
RTTI_FUNCTION(FuncConst)
RTTI_FUNCTION(Proc)
RTTI_FUNCTION(ProcConst)
RTTI_FUNCTION(Getter)
RTTI_FUNCTION(GetterConst)
RTTI_FUNCTION(Out)
RTTI_FUNCTION(OutConst)
RTTI_FUNCTION(OutConstReturn)
RTTI_CLASS_END()
//----------------------------------------------------------------------------
FWD_REFPTR(Toto);
class FToto : public Core::RTTI::FMetaObject {
public:
    FToto() {}
    virtual ~FToto() {}
    RTTI_CLASS_HEADER(FToto, Core::RTTI::FMetaObject);
private:
    typedef Core::TPair<int COMMA Core::TPair<float COMMA Core::FString>> value_type;
    int _count;
    Core::FString _name;
    VECTOR(Internal, PTiti) _tities;
    VECTOR_THREAD_LOCAL(Internal, Core::FString) _titles;
    Core::TRefPtr<FToto> _parent;
    Core::TPair<int, int> _pair;
    Core::TPair<float, float> _fpair;
    Core::TPair<Core::TPair<int, int>, Core::TPair<int, int> > _vpair;
    Core::TPair<Core::TPair<int, int>, Core::TPair<int, float> > _vpair2;
    HASHMAP_THREAD_LOCAL(Internal, Core::FString, float) _dict;
    HASHMAP_THREAD_LOCAL(Internal, Core::FString, value_type) _dict2;
    /*
    INSTANTIATE_CLASS_TYPEDEF(key_type3, Core::TPair<int COMMA float>);
    INSTANTIATE_CLASS_TYPEDEF(value_type3, VECTORINSITU(RTTI, Core::TPair<float COMMA Core::FString>, 2));
    ASSOCIATIVE_VECTOR(Internal, key_type3, value_type3) _dict3;
    */
};
RTTI_CLASS_BEGIN(RTTI_UnitTest, FToto, Concrete)
RTTI_PROPERTY_FIELD_ALIAS(_count, Count)
RTTI_PROPERTY_FIELD_ALIAS(_name, Name)
RTTI_PROPERTY_FIELD_ALIAS(_tities, Tities)
RTTI_PROPERTY_FIELD_ALIAS(_titles, Titles)
RTTI_PROPERTY_FIELD_ALIAS(_parent, Parent)
RTTI_PROPERTY_FIELD_ALIAS(_pair, TPair)
RTTI_PROPERTY_FIELD_ALIAS(_fpair, FPair)
RTTI_PROPERTY_FIELD_ALIAS(_vpair, VPair)
RTTI_PROPERTY_FIELD_ALIAS(_vpair2, VPair2)
RTTI_PROPERTY_FIELD_ALIAS(_dict, Dict)
RTTI_PROPERTY_FIELD_ALIAS(_dict2, Dict2)
//RTTI_PROPERTY_FIELD_ALIAS(_dict3, Dict3)
RTTI_CLASS_END()
//----------------------------------------------------------------------------
template <typename T>
static void TestRTTIWrap_() {
    typedef Core::RTTI::TMetaTypeTraits< T > type_traits;
    T wrapped;
    typename type_traits::wrapper_type wrapper;
    type_traits::WrapCopy(wrapper, wrapped);
    type_traits::WrapMove(wrapper, std::move(wrapped));
    type_traits::UnwrapCopy(wrapped, wrapper);
    type_traits::UnwrapMove(wrapped, std::move(wrapper));
}
//----------------------------------------------------------------------------
static void TestRTTI_() {
    using namespace Core;

    const RTTI_TAG(RTTI_UnitTest) TagScope;

    LOG(Debug, L"[RTTI] Id = {0}, Name = {1}, Default = {2}, EFlags = {3}",
        RTTI::TMetaType< int32_t >::Id(),
        RTTI::TMetaType< int32_t >::Name(),
        RTTI::TMetaType< int32_t >::DefaultValue(),
        (size_t)RTTI::TMetaType< int32_t >::Flags()
        );

    LOG(Debug, L"[RTTI] Id = {0}, Name = {1}, Default = {2}, EFlags = {3}",
        RTTI::TMetaType< int >::Id(),
        RTTI::TMetaType< int >::Name(),
        RTTI::TMetaType< int >::DefaultValue(),
        (size_t)RTTI::TMetaType< int >::Flags()
        );

    LOG(Debug, L"[RTTI] Id = {0}, Name = {1}, Default = {2}, EFlags = {3}",
        RTTI::TMetaType< size_t >::Id(),
        RTTI::TMetaType< size_t >::Name(),
        RTTI::TMetaType< size_t >::DefaultValue(),
        (size_t)RTTI::TMetaType< size_t >::Flags()
        );

    LOG(Debug, L"[RTTI] Id = {0}, Name = {1}, Default = {2}, EFlags = {3}",
        RTTI::TMetaType< FString >::Id(),
        RTTI::TMetaType< FString >::Name(),
        RTTI::TMetaType< FString >::DefaultValue(),
        (size_t)RTTI::TMetaType< FString >::Flags()
        );

    LOG(Debug, L"[RTTI] Id = {0}, Name = {1}, Default = {2}, EFlags = {3}",
        RTTI::TMetaType< RTTI::TVector<int> >::Id(),
        RTTI::TMetaType< RTTI::TVector<int> >::Name(),
        RTTI::TMetaType< RTTI::TVector<int> >::DefaultValue(),
        (size_t)RTTI::TMetaType< RTTI::TVector<int> >::Flags()
        );

    LOG(Debug, L"[RTTI] Id = {0}, Name = {1}, Default = {2}, EFlags = {3}",
        RTTI::TMetaType< TPair<float, int> >::Id(),
        RTTI::TMetaType< TPair<float, int> >::Name(),
        RTTI::TMetaType< TPair<float, int> >::DefaultValue(),
        (size_t)RTTI::TMetaType< TPair<float, int> >::Flags()
        );

    RTTI::TMetaTypeTraits< int >::meta_type::Name();

    int i = 0;
    RTTI::TMetaTypeTraits< int >::UnwrapCopy(i, 42);

    RTTI::PMetaObject o;
    RTTI::TMetaTypeTraits< RTTI::PMetaObject >::UnwrapCopy(o, nullptr);

    PTiti t;
    RTTI::TMetaTypeTraits< PTiti >::UnwrapCopy(t, nullptr);
    RTTI::TMetaTypeTraits< PTiti >::WrapMove(o, std::move(t));

    TestRTTIWrap_< PTiti >();
    TestRTTIWrap_< TPair<FWString, PTiti> >();
    TestRTTIWrap_< VECTOR_THREAD_LOCAL(RTTI, PTiti) >();
    TestRTTIWrap_< HASHMAP(RTTI, int, int) >();
    TestRTTIWrap_< HASHMAP(RTTI, FString, PTiti) >();
    TestRTTIWrap_< ASSOCIATIVE_VECTOR(RTTI, FString, PTiti) >();
    //TestRTTIWrap_< HASHSET(RTTI, FString) >();

    t = new FTiti();
    const RTTI::FMetaClass *metaClass = t->RTTI_MetaClass();

    LOG(Info, L"[RTTI] TMetaClass<{0}> : {1}", metaClass->Name(), metaClass->Attributes());
    for (const auto& it : metaClass->Functions())
        LOG(Info, L"[RTTI]   - FUNC {0} {1}({2}) : {3}", it->SignatureInfos().front(), it->Name(), CommaSeparated(it->SignatureInfos().ShiftFront()), it->Attributes());
    for (const auto& it : metaClass->Properties())
        LOG(Info, L"[RTTI]   - PROP {0} : {1} -> {2}", it->Name(), it->Attributes(), it->TypeInfo());

    const RTTI::FMetaProperty *prop = metaClass->PropertyIFP("Count");

    int value;
    prop->Cast<int>()->GetCopy(t.get(), value);
    prop->Cast<int>()->SetMove(t.get(), 42);
    Assert(!prop->IsDefaultValue(t.get()));
    prop->Cast<int>()->SetMove(t.get(), int());
    Assert(prop->IsDefaultValue(t.get()));

    PTiti t2 = new FTiti();
    prop->Swap(t.get(), t2.get());
    Swap(*t, *t2);

    RTTI::PMetaAtom atom = prop->WrapCopy(t.get());
    RTTI::PMetaAtom defaultAtom = atom->Traits()->CreateDefaultValue();
    Assert(defaultAtom->IsDefaultValue());
    LOG(Info, L"[RTTI] {0} ({1})", *atom->Cast<int>(), atom->HashValue());
    /*
    prop = metaClass->PropertyIFP("Dict");
    defaultAtom = prop->Traits()->CreateDefaultValue();
    Assert(defaultAtom->IsDefaultValue());
    atom = prop->WrapMove(t.get());
    */
    prop = metaClass->PropertyIFP("Tities");
    defaultAtom = prop->Traits()->CreateDefaultValue();
    Assert(defaultAtom->IsDefaultValue());
    atom = prop->WrapCopy(t.get());

    auto typedAtom = atom->Cast< VECTOR(Internal, PTiti) >();
    typedAtom->Wrapper().push_back(new FTiti());

    prop->MoveFrom(t.get(), typedAtom);

    //auto wrongAtom = atom->Cast< int >();
    auto wrongAtom2 = atom->As< int >();
    AssertRelease(!wrongAtom2);

    const RTTI::FMetaFunction* func = metaClass->FunctionIFP("OutConstReturn");

    RTTI::PMetaAtom args[] = {
        RTTI::MakeAtom(42.0f) ,
        RTTI::MakeAtom(FString("string"))
    };
    RTTI::PMetaAtom result;
    if (not func->Invoke(t.get(), result, args))
    {
        AssertNotReached();
    }

    RTTI::PMetaAtom args2[] = {
        RTTI::MakeAtom(42) ,
        RTTI::MakeAtom(FString("string"))
    };
    if (not func->PromoteInvoke(t.get(), result, args2))
    {
        AssertNotReached();
    }

#if 0
    const int v = wrongAtom2->Wrapper(); // crash !
    LOG(Info, L"[RTTI] wrong atom = {0}", v);
#endif
}
//----------------------------------------------------------------------------
//} //!namespace
//----------------------------------------------------------------------------
namespace RTTI {
static void RTTI_UnitTests() {
    LOG(Debug, L"[RTTI] Begin unit tests");
    TestRTTI_();
    LOG(Debug, L"[RTTI] End unit tests");
}
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#endif //!WITH_RTTI_UNITTESTS
