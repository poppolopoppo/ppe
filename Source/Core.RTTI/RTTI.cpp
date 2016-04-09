#include "stdafx.h"

#include "RTTI.h"
#include "RTTI_fwd.h"
#include "RTTI_Tag.h"

#include "MetaAtomDatabase.h"
#include "MetaClassDatabase.h"
#include "MetaClassName.h"
#include "MetaObject.h"
#include "MetaObjectName.h"
#include "MetaObjectHelpers.h"
#include "MetaPropertyName.h"

#include "Core/Allocator/PoolAllocatorTag-impl.h"

#if !(defined(FINAL_RELEASE) || defined(PROFILING_ENABLED))
//#   define WITH_RTTI_UNITTESTS %_NOCOMMIT%
#endif

#ifdef OS_WINDOWS
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
    POOL_TAG(RTTI)::Start();

    MetaClassName::Start(256);
    MetaPropertyName::Start(512);
    MetaObjectName::Start(1024);

    MetaClassDatabase::Create();
    MetaAtomDatabase::Create();

    RTTI_TAG(Default)::Start();

#ifdef WITH_RTTI_UNITTESTS
    RTTI_UnitTests();
#endif
}
//----------------------------------------------------------------------------
void RTTIStartup::Shutdown() {
    RTTI_TAG(Default)::Shutdown();

    MetaAtomDatabase::Destroy();
    MetaClassDatabase::Destroy();

    MetaObjectName::Shutdown();
    MetaPropertyName::Shutdown();
    MetaClassName::Shutdown();

    POOL_TAG(RTTI)::Shutdown();
}
//----------------------------------------------------------------------------
void RTTIStartup::Clear() {
    MetaAtomDatabase::Instance().Clear();
    MetaClassDatabase::Instance().Clear();

    MetaObjectName::Clear();
    MetaPropertyName::Clear();
    MetaClassName::Clear();

    POOL_TAG(RTTI)::ClearAll_UnusedMemory();
}
//----------------------------------------------------------------------------
void RTTIStartup::ClearAll_UnusedMemory() {
    POOL_TAG(RTTI)::ClearAll_UnusedMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core

#ifdef WITH_RTTI_UNITTESTS

#include "RTTI_fwd.h"
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
namespace {
//----------------------------------------------------------------------------
FWD_REFPTR(Titi);
class Titi : public Core::RTTI::MetaObject {
public:
    Titi() {}
    virtual ~Titi() {}
    RTTI_CLASS_HEADER(Titi, Core::RTTI::MetaObject);
private:
    int _count;
    Core::String _name;
    VECTOR(Internal, PTiti) _tities;
    VECTOR(Internal, PCTiti) _consttities;
    ASSOCIATIVE_VECTOR(Internal, Core::Pair<int COMMA PTiti>, VECTORINSITU(RTTI, Core::Pair<float COMMA Core::String>, 2)) _dict;
};
RTTI_CLASS_BEGIN(Titi, Concrete)
RTTI_PROPERTY_PRIVATE_FIELD(_count)
RTTI_PROPERTY_FIELD_ALIAS(_name, Name)
RTTI_PROPERTY_PRIVATE_FIELD(_tities)
RTTI_PROPERTY_DEPRECATED(VECTOR(Internal, PTiti), Tities2)
RTTI_PROPERTY_PRIVATE_FIELD(_consttities)
RTTI_PROPERTY_PRIVATE_FIELD(_dict)
RTTI_CLASS_END()
//----------------------------------------------------------------------------
FWD_REFPTR(Toto);
class Toto : public Core::RTTI::MetaObject {
public:
    Toto() {}
    virtual ~Toto() {}
    RTTI_CLASS_HEADER(Toto, Core::RTTI::MetaObject);
private:
    typedef Core::Pair<int COMMA Core::Pair<float COMMA Core::String>> value_type;
    int _count;
    Core::String _name;
    VECTOR(Internal, PTiti) _tities;
    VECTOR_THREAD_LOCAL(Internal, Core::String) _titles;
    Core::RefPtr<Toto> _parent;
    Core::Pair<int, int> _pair;
    Core::Pair<float, float> _fpair;
    Core::Pair<Core::Pair<int, int>, Core::Pair<int, int> > _vpair;
    Core::Pair<Core::Pair<int, int>, Core::Pair<int, float> > _vpair2;
    HASHMAP_THREAD_LOCAL(Internal, Core::String, float) _dict;
    HASHMAP_THREAD_LOCAL(Internal, Core::String, value_type) _dict2;
    ASSOCIATIVE_VECTOR(Internal, Core::Pair<int COMMA float>, VECTORINSITU(RTTI, Core::Pair<float COMMA Core::String>, 2)) _dict3;
};
RTTI_CLASS_BEGIN(Toto, Concrete)
RTTI_PROPERTY_FIELD_ALIAS(_count, Count)
RTTI_PROPERTY_FIELD_ALIAS(_name, Name)
RTTI_PROPERTY_FIELD_ALIAS(_tities, Tities)
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
template <typename T>
static void TestRTTIWrap_() {
    typedef Core::RTTI::MetaTypeTraits< T > type_traits;
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

    LOG(Debug, L"[RTTI] Id = {0}, Name = {1}, Default = {2}, Flags = {3}",
        RTTI::MetaType< int32_t >::Id(),
        RTTI::MetaType< int32_t >::Name(),
        RTTI::MetaType< int32_t >::DefaultValue(),
        (size_t)RTTI::MetaType< int32_t >::Flags()
        );

    LOG(Debug, L"[RTTI] Id = {0}, Name = {1}, Default = {2}, Flags = {3}",
        RTTI::MetaType< int >::Id(),
        RTTI::MetaType< int >::Name(),
        RTTI::MetaType< int >::DefaultValue(),
        (size_t)RTTI::MetaType< int >::Flags()
        );

    LOG(Debug, L"[RTTI] Id = {0}, Name = {1}, Default = {2}, Flags = {3}",
        RTTI::MetaType< size_t >::Id(),
        RTTI::MetaType< size_t >::Name(),
        RTTI::MetaType< size_t >::DefaultValue(),
        (size_t)RTTI::MetaType< size_t >::Flags()
        );

    LOG(Debug, L"[RTTI] Id = {0}, Name = {1}, Default = {2}, Flags = {3}",
        RTTI::MetaType< String >::Id(),
        RTTI::MetaType< String >::Name(),
        RTTI::MetaType< String >::DefaultValue(),
        (size_t)RTTI::MetaType< String >::Flags()
        );

    LOG(Debug, L"[RTTI] Id = {0}, Name = {1}, Default = {2}, Flags = {3}",
        RTTI::MetaType< RTTI::Vector<int> >::Id(),
        RTTI::MetaType< RTTI::Vector<int> >::Name(),
        RTTI::MetaType< RTTI::Vector<int> >::DefaultValue(),
        (size_t)RTTI::MetaType< RTTI::Vector<int> >::Flags()
        );

    LOG(Debug, L"[RTTI] Id = {0}, Name = {1}, Default = {2}, Flags = {3}",
        RTTI::MetaType< Pair<float, int> >::Id(),
        RTTI::MetaType< Pair<float, int> >::Name(),
        RTTI::MetaType< Pair<float, int> >::DefaultValue(),
        (size_t)RTTI::MetaType< Pair<float, int> >::Flags()
        );

    RTTI::MetaTypeTraits< int >::meta_type::Name();

    int i = 0;
    RTTI::MetaTypeTraits< int >::UnwrapCopy(i, 42);

    RTTI::PMetaObject o;
    RTTI::MetaTypeTraits< RTTI::PMetaObject >::UnwrapCopy(o, nullptr);

    PTiti t;
    RTTI::MetaTypeTraits< PTiti >::UnwrapCopy(t, nullptr);
    RTTI::MetaTypeTraits< PTiti >::WrapMove(o, std::move(t));

    TestRTTIWrap_< PTiti >();
    TestRTTIWrap_< Pair<WString, PTiti> >();
    TestRTTIWrap_< VECTOR_THREAD_LOCAL(RTTI, PTiti) >();
    TestRTTIWrap_< HASHMAP(RTTI, int, int) >();
    TestRTTIWrap_< HASHMAP(RTTI, String, PTiti) >();
    TestRTTIWrap_< ASSOCIATIVE_VECTOR(RTTI, String, PTiti) >();
    //TestRTTIWrap_< HASHSET(RTTI, String) >();

    RTTI::MetaClassSingleton<Titi>::Create();

    t = new Titi();
    const RTTI::MetaClass *metaClass = t->RTTI_MetaClass();

    LOG(Info, L"[RTTI] MetaClass<{0}> : {1}", metaClass->Name(), metaClass->Attributes());
    for (const auto& it : metaClass->Properties())
        LOG(Info, L"[RTTI]   - {0} : {1} -> {2}", it.first, it.second->Attributes(), it.second->TypeInfo());

    const RTTI::MetaProperty *prop = metaClass->PropertyIFP("Count");

    int value;
    prop->Cast<int>()->GetCopy(t.get(), value);
    prop->Cast<int>()->SetMove(t.get(), 42);
    Assert(!prop->IsDefaultValue(t.get()));
    prop->Cast<int>()->SetMove(t.get(), int());
    Assert(prop->IsDefaultValue(t.get()));

    PTiti t2 = new Titi();
    prop->Swap(t.get(), t2.get());
    Swap(*t, *t2);

    RTTI::PMetaAtom atom = prop->WrapCopy(t.get());
    RTTI::PMetaAtom defaultAtom = atom->Traits()->CreateDefaultValue();
    Assert(defaultAtom->IsDefaultValue());
    LOG(Info, L"[RTTI] {0} ({1})", *atom->Cast<int>(), atom->HashValue());

    prop = metaClass->PropertyIFP("Dict");
    defaultAtom = prop->Traits()->CreateDefaultValue();
    Assert(defaultAtom->IsDefaultValue());
    atom = prop->WrapMove(t.get());

    prop = metaClass->PropertyIFP("Tities");
    defaultAtom = prop->Traits()->CreateDefaultValue();
    Assert(defaultAtom->IsDefaultValue());
    atom = prop->WrapCopy(t.get());

    auto typedAtom = atom->Cast< VECTOR(Internal, PTiti) >();
    typedAtom->Wrapper().push_back(new Titi());

    prop->MoveFrom(t.get(), typedAtom);

    //auto wrongAtom = atom->Cast< int >();
    auto wrongAtom2 = atom->As< int >();
    AssertRelease(!wrongAtom2);

#if 0
    const int v = wrongAtom2->Wrapper(); // crash !
    LOG(Info, L"[RTTI] wrong atom = {0}", v);
#endif

    RTTI::MetaClassSingleton<Titi>::Destroy();
}
//----------------------------------------------------------------------------
} //!namespace
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
