#include "stdafx.h"

#include "MetaObject.h"

#include "MetaAtom.h"
#include "MetaClass.h"
#include "MetaClassDatabase.h"
#include "MetaClassSingleton.h"
#include "MetaProperty.h"

#include "RTTI_Tag-impl.h"
#include "RTTIMacros-impl.h"

#include "Core/Container/Hash.h"
#include "Core/Container/HashMap.h"
#include "Core/Container/Token.h"
#include "Core/Container/Vector.h"

#include "Core/IO/Format.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
_RTTI_CLASS_AUTOREGISTER(Default, MetaObject);
//----------------------------------------------------------------------------
MetaObject::MetaClass::MetaClass()
:   RTTI::DefaultMetaClass<MetaObject>("MetaObject", RTTI::MetaClass::Abstract)
{}
//----------------------------------------------------------------------------
MetaObject::MetaClass::~MetaClass() {}
//----------------------------------------------------------------------------
void MetaObject::MetaClass::Create() {
    Core::RTTI::MetaClassSingleton< MetaObject >::Create();
}
//----------------------------------------------------------------------------
void MetaObject::MetaClass::Destroy() {
    Core::RTTI::MetaClassSingleton< MetaObject >::Destroy();
}
//----------------------------------------------------------------------------
bool MetaObject::MetaClass::HasInstance() {
    return Core::RTTI::MetaClassSingleton< MetaObject >::HasInstance();
}
//----------------------------------------------------------------------------
const MetaObject::MetaClass *MetaObject::MetaClass::Instance() {
    return &Core::RTTI::MetaClassSingleton< MetaObject >::Instance();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
MetaObject::MetaObject()
:   _state(None) {}
//----------------------------------------------------------------------------
MetaObject::~MetaObject() {}
//----------------------------------------------------------------------------
void MetaObject::RTTI_Export(const RTTI::Name& name) {
    Assert(!name.empty());
    Assert(_name.empty());
    Assert(0 == (_state & Exported));

    _name = name;
    _state = Flags(_state | Exported);
}
//----------------------------------------------------------------------------
void MetaObject::RTTI_Unexport() {
    Assert(!_name.empty());
    Assert(Exported == (_state & Exported));

    _name = RTTI::Name();
    _state = Flags(_state & ~Exported);

    Assert(_name.empty());
}
//----------------------------------------------------------------------------
void MetaObject::RTTI_Load(MetaLoadContext * /* context */) {
    Assert(0 == (_state & Loaded));
    Assert(0 == (_state & Unloaded));
    _state = Flags(_state | Loaded);

#ifdef WITH_RTTI_VERIFY_PREDICATES
    // checks that base method was called :
    Assert(0 == (_state & Verifying));
    _state = Flags(_state | Verifying);
    RTTI_VerifyPredicates();
    Assert(0 == (_state & Verifying));
#endif
}
//----------------------------------------------------------------------------
void MetaObject::RTTI_Unload(MetaUnloadContext * /* context */) {
    Assert(1 == (_state & Loaded));
    Assert(0 == (_state & Unloaded));
    _state = Flags((_state & ~Loaded) | Unloaded);

#ifdef WITH_RTTI_VERIFY_PREDICATES
    // checks that base method was called :
    Assert(0 == (_state & Verifying));
    _state = Flags(_state | Verifying);
    RTTI_VerifyPredicates();
    Assert(0 == (_state & Verifying));
#endif
}
//----------------------------------------------------------------------------
void MetaObject::RTTI_CallLoadIFN(MetaLoadContext *context) {
    if (0 == (_state & Loaded)) {
        RTTI_Load(context);
        Assert(Loaded == (_state & Loaded));
    }
}
//----------------------------------------------------------------------------
void MetaObject::RTTI_CallUnloadIFN(MetaUnloadContext *context) {
    Assert(Loaded == (_state & Loaded));
    if (0 == (_state & Unloaded)) {
        RTTI_Unload(context);
        Assert(Unloaded == (_state & Unloaded));
    }
}
//----------------------------------------------------------------------------
#ifdef WITH_RTTI_VERIFY_PREDICATES
void MetaObject::RTTI_VerifyPredicates() const {
    // checks that base method was called :
    _state = Flags(_state & ~Verifying);
}
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
