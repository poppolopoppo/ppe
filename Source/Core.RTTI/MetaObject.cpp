#include "stdafx.h"

#include "MetaObject.h"

#include "MetaAtom.h"
#include "MetaClass.h"
#include "MetaNamespace.h"
#include "MetaProperty.h"

#include "RTTI_Namespace.h"

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
FMetaNamespace& FMetaObject::FMetaClass::Namespace() {
    return RTTI_NAMESPACE(RTTI);
}
//----------------------------------------------------------------------------
FMetaObject::FMetaClass::FMetaClass(FMetaClassGuid guid, const FMetaNamespace* metaNamespace)
    : metaclass_type(guid, RTTI::FMetaClass::Abstract, FName("FMetaObject"), metaNamespace)
{}
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
    Assert(not (_state ^ Exported));

    _name = name;
    _state = _state | Exported;
}
//----------------------------------------------------------------------------
void FMetaObject::RTTI_Unexport() {
    Assert(!_name.empty());
    Assert(not (_state ^ Exported));

    _name = FName();
    _state = _state & ~Exported;

    Assert(_name.empty());
}
//----------------------------------------------------------------------------
void FMetaObject::RTTI_Load(FMetaLoadContext * /* context */) {
    Assert(not (_state ^ Loaded));
    Assert(not (_state ^ Unloaded));

    _state = _state | Loaded;

#ifdef WITH_RTTI_VERIFY_PREDICATES
    // checks that base method was called :
    Assert(not (_state ^ Verifying));
    _state = _state | Verifying;
    RTTI_VerifyPredicates();
    Assert(not (_state ^ Verifying));
#endif
}
//----------------------------------------------------------------------------
void FMetaObject::RTTI_Unload(FMetaUnloadContext * /* context */) {
    Assert(_state ^ Loaded);
    Assert(not (_state ^ Unloaded));

    _state = (_state & ~Loaded) | Unloaded;

#ifdef WITH_RTTI_VERIFY_PREDICATES
    // checks that base method was called :
    Assert(not (_state ^ Verifying));
    _state = _state | Verifying;
    RTTI_VerifyPredicates();
    Assert(not (_state ^ Verifying));
#endif
}
//----------------------------------------------------------------------------
void FMetaObject::RTTI_CallLoadIFN(FMetaLoadContext *context) {
    Assert(not (_state ^ Unloaded));
    if (not (_state ^ Loaded)) {
        RTTI_Load(context);
        Assert(_state ^ Loaded);
    }
}
//----------------------------------------------------------------------------
void FMetaObject::RTTI_CallUnloadIFN(FMetaUnloadContext *context) {
    Assert(_state ^ Loaded);
    if (not (_state ^ Unloaded)) {
        RTTI_Unload(context);
        Assert(_state ^ Unloaded);
    }
}
//----------------------------------------------------------------------------
#ifdef WITH_RTTI_VERIFY_PREDICATES
void FMetaObject::RTTI_VerifyPredicates() const {
    // checks that base method was called :
    _state = _state & ~Verifying;
}
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
