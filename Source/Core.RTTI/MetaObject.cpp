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
_RTTI_CLASS_AUTOREGISTER(Default, FMetaObject);
//----------------------------------------------------------------------------
FMetaObject::FMetaClass::FMetaClass()
:   RTTI::TDefaultMetaClass<FMetaObject>("FMetaObject", RTTI::FMetaClass::Abstract)
{}
//----------------------------------------------------------------------------
FMetaObject::FMetaClass::~FMetaClass() {}
//----------------------------------------------------------------------------
void FMetaObject::FMetaClass::Create() {
    Core::RTTI::TMetaClassSingleton< FMetaObject >::Create();
}
//----------------------------------------------------------------------------
void FMetaObject::FMetaClass::Destroy() {
    Core::RTTI::TMetaClassSingleton< FMetaObject >::Destroy();
}
//----------------------------------------------------------------------------
bool FMetaObject::FMetaClass::HasInstance() {
    return Core::RTTI::TMetaClassSingleton< FMetaObject >::HasInstance();
}
//----------------------------------------------------------------------------
const FMetaObject::FMetaClass *FMetaObject::FMetaClass::Instance() {
    return &Core::RTTI::TMetaClassSingleton< FMetaObject >::Instance();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FMetaObject::FMetaObject()
:   _state(None) {}
//----------------------------------------------------------------------------
FMetaObject::~FMetaObject() {}
//----------------------------------------------------------------------------
void FMetaObject::RTTI_Export(const FName& name) {
    Assert(!name.empty());
    Assert(_name.empty());
    Assert(0 == (_state & Exported));

    _name = name;
    _state = EFlags(_state | Exported);
}
//----------------------------------------------------------------------------
void FMetaObject::RTTI_Unexport() {
    Assert(!_name.empty());
    Assert(Exported == (_state & Exported));

    _name = FName();
    _state = EFlags(_state & ~Exported);

    Assert(_name.empty());
}
//----------------------------------------------------------------------------
void FMetaObject::RTTI_Load(FMetaLoadContext * /* context */) {
    Assert(0 == (_state & Loaded));
    Assert(0 == (_state & Unloaded));
    _state = EFlags(_state | Loaded);

#ifdef WITH_RTTI_VERIFY_PREDICATES
    // checks that base method was called :
    Assert(0 == (_state & Verifying));
    _state = EFlags(_state | Verifying);
    RTTI_VerifyPredicates();
    Assert(0 == (_state & Verifying));
#endif
}
//----------------------------------------------------------------------------
void FMetaObject::RTTI_Unload(FMetaUnloadContext * /* context */) {
    Assert(1 == (_state & Loaded));
    Assert(0 == (_state & Unloaded));
    _state = EFlags((_state & ~Loaded) | Unloaded);

#ifdef WITH_RTTI_VERIFY_PREDICATES
    // checks that base method was called :
    Assert(0 == (_state & Verifying));
    _state = EFlags(_state | Verifying);
    RTTI_VerifyPredicates();
    Assert(0 == (_state & Verifying));
#endif
}
//----------------------------------------------------------------------------
void FMetaObject::RTTI_CallLoadIFN(FMetaLoadContext *context) {
    if (0 == (_state & Loaded)) {
        RTTI_Load(context);
        Assert(Loaded == (_state & Loaded));
    }
}
//----------------------------------------------------------------------------
void FMetaObject::RTTI_CallUnloadIFN(FMetaUnloadContext *context) {
    Assert(Loaded == (_state & Loaded));
    if (0 == (_state & Unloaded)) {
        RTTI_Unload(context);
        Assert(Unloaded == (_state & Unloaded));
    }
}
//----------------------------------------------------------------------------
#ifdef WITH_RTTI_VERIFY_PREDICATES
void FMetaObject::RTTI_VerifyPredicates() const {
    // checks that base method was called :
    _state = EFlags(_state & ~Verifying);
}
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
