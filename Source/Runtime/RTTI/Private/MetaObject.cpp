#include "stdafx.h"

#include "MetaObject.h"

#include "MetaClass.h"
#include "MetaProperty.h"
#include "MetaObjectHelpers.h"
#include "MetaTransaction.h"

#include "RTTI/Atom.h"
#include "RTTI/Exceptions.h"
#include "RTTI/Module.h"

#include "IO/FormatHelpers.h"
#include "IO/TextWriter.h"

namespace PPE {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static const FMetaClassHandle GMetaObject_MetaClassHandle_{
    FMetaObject::RTTI_FMetaClass::Module(),
    &FMetaObject::RTTI_FMetaClass::CreateMetaClass,
    [](FMetaClass* metaClass) {
        TRACKING_DELETE(MetaClass, metaClass);
    }};
//----------------------------------------------------------------------------
} //!details
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FMetaObject::FMetaObject()
:   _flags(EObjectFlags::Unloaded)
{}
//----------------------------------------------------------------------------
FMetaObject::~FMetaObject() {
    Assert_NoAssume(RTTI_IsUnloaded());
    Assert_NoAssume(not RTTI_IsLoaded());
    Assert(nullptr == _outer);
}
//----------------------------------------------------------------------------
void FMetaObject::RTTI_SetOuter(const FMetaTransaction* outer, const FMetaTransaction* prevOuterForDbg/* = nullptr */) {
    Assert_NoAssume(RTTI_IsLoaded());
    Assert(prevOuterForDbg == _outer);

    _outer = outer;
}
//----------------------------------------------------------------------------
FPathName FMetaObject::RTTI_PathName() const {
    Assert_NoAssume(RTTI_IsExported());
    Assert_NoAssume(RTTI_IsLoaded());

    return FPathName::FromObject(*this);
}
//----------------------------------------------------------------------------
bool FMetaObject::RTTI_IsA(const FMetaClass& metaClass) const {
    return (RTTI_Class() == &metaClass);
}
//----------------------------------------------------------------------------
bool FMetaObject::RTTI_CastTo(const FMetaClass& metaClass) const {
    return RTTI_Class()->CastTo(metaClass);
}
//----------------------------------------------------------------------------
bool FMetaObject::RTTI_InheritsFrom(const FMetaClass& metaClass) const {
    return RTTI_Class()->InheritsFrom(metaClass);
}
//----------------------------------------------------------------------------
bool FMetaObject::RTTI_IsAssignableFrom(const FMetaClass& metaClass) const {
    return RTTI_Class()->IsAssignableFrom(metaClass);
}
//----------------------------------------------------------------------------
void FMetaObject::RTTI_ResetToDefaultValue() {
    ResetToDefaultValue(*this);
}
//----------------------------------------------------------------------------
void FMetaObject::RTTI_Freeze() {
    Assert_NoAssume(not RTTI_IsFrozen());

    _flags = _flags + EObjectFlags::Frozen;
}
//----------------------------------------------------------------------------
void FMetaObject::RTTI_Unfreeze() {
    Assert_NoAssume(RTTI_IsFrozen());

    _flags = _flags - EObjectFlags::Frozen;
}
//----------------------------------------------------------------------------
void FMetaObject::RTTI_Export(const FName& name) {
    Assert_NoAssume(not RTTI_IsExported());
    Assert_NoAssume(not RTTI_IsLoaded());
    Assert_NoAssume(not RTTI_IsFrozen());
    Assert_NoAssume(_name.empty());

    _flags = _flags + EObjectFlags::Exported;
    _name = name;

    Assert_NoAssume(RTTI_IsExported());
}
//----------------------------------------------------------------------------
void FMetaObject::RTTI_Unexport() {
    Assert_NoAssume(RTTI_IsExported());
    Assert_NoAssume(not RTTI_IsLoaded());
    Assert_NoAssume(not RTTI_IsFrozen());
    Assert_NoAssume(not _name.empty());

    _flags = _flags - EObjectFlags::Exported;
    _name = FName();

    Assert_NoAssume(not RTTI_IsExported());
}
//----------------------------------------------------------------------------
bool FMetaObject::RTTI_Function(const FName& funcName, const FMetaFunction** pFunc) const NOEXCEPT {
    Assert_NoAssume(not funcName.empty());
    Assert(pFunc);

    const FMetaClass* const klass = RTTI_Class();
    if (Likely(klass)) {
        if (const FMetaFunction* const func = klass->FunctionIFP(funcName)) {
            *pFunc = func;
            return true;
        }
    }

    return false;
}
//----------------------------------------------------------------------------
bool FMetaObject::RTTI_Function(const FStringView& funcName, const FMetaFunction** pFunc) const NOEXCEPT {
    Assert_NoAssume(not funcName.empty());
    Assert(pFunc);

    const FMetaClass* const klass = RTTI_Class();
    if (Likely(klass)) {
        if (const FMetaFunction* const func = klass->FunctionIFP(funcName)) {
            *pFunc = func;
            return true;
        }
    }

    return false;
}
//----------------------------------------------------------------------------
bool FMetaObject::RTTI_Property(const FName& propName, FAtom* pValue) const NOEXCEPT {
    Assert_NoAssume(not propName.empty());
    Assert(pValue);

    const FMetaClass* const klass = RTTI_Class();
    if (Likely(klass)) {
        if (const FMetaProperty* const prop = klass->PropertyIFP(propName)) {
            *pValue = prop->Get(*this);
            return true;
        }
    }

    return false;
}
//----------------------------------------------------------------------------
bool FMetaObject::RTTI_Property(const FStringView& propName, FAtom* pValue) const NOEXCEPT {
    Assert_NoAssume(not propName.empty());
    Assert(pValue);

    const FMetaClass* const klass = RTTI_Class();
    if (Likely(klass)) {
        if (const FMetaProperty* const prop = klass->PropertyIFP(propName)) {
            *pValue = prop->Get(*this);
            return true;
        }
    }

    return false;
}
//----------------------------------------------------------------------------
bool FMetaObject::RTTI_PropertyCopyFrom(const FName& propName, const FAtom& src) {
    Assert_NoAssume(not RTTI_IsFrozen());
    Assert_NoAssume(not propName.empty());
    Assert(src);

    const FMetaClass* const klass = RTTI_Class();
    if (Likely(klass)) {
        if (const FMetaProperty* const prop = klass->PropertyIFP(propName)) {
            prop->CopyFrom(*this, src);
            return true;
        }
    }

    return false;
}
//----------------------------------------------------------------------------
bool FMetaObject::RTTI_PropertyCopyFrom(const FStringView& propName, const FAtom& src) {
    Assert_NoAssume(not RTTI_IsFrozen());
    Assert_NoAssume(not propName.empty());
    Assert(src);

    const FMetaClass* const klass = RTTI_Class();
    if (Likely(klass)) {
        if (const FMetaProperty* const prop = klass->PropertyIFP(propName)) {
            prop->CopyFrom(*this, src);
            return true;
        }
    }

    return false;
}
//----------------------------------------------------------------------------
bool FMetaObject::RTTI_PropertyMoveFrom(const FName& propName, FAtom& src) NOEXCEPT {
    Assert_NoAssume(not RTTI_IsFrozen());
    Assert_NoAssume(not propName.empty());
    Assert(src);

    const FMetaClass* const klass = RTTI_Class();
    if (Likely(klass)) {
        if (const FMetaProperty* const prop = klass->PropertyIFP(propName)) {
            prop->MoveFrom(*this, src);
            return true;
        }
    }

    return false;
}
//----------------------------------------------------------------------------
bool FMetaObject::RTTI_PropertyMoveFrom(const FStringView& propName, FAtom& src) NOEXCEPT {
    Assert_NoAssume(not RTTI_IsFrozen());
    Assert_NoAssume(not propName.empty());
    Assert(src);

    const FMetaClass* const klass = RTTI_Class();
    if (Likely(klass)) {
        if (const FMetaProperty* const prop = klass->PropertyIFP(propName)) {
            prop->MoveFrom(*this, src);
            return true;
        }
    }

    return false;
}
//----------------------------------------------------------------------------
void FMetaObject::RTTI_Load(ILoadContext& context) {
    Assert_NoAssume(not RTTI_IsLoaded());
    Assert_NoAssume(not RTTI_IsFrozen());
    Assert_NoAssume(RTTI_IsUnloaded());

    _flags = _flags + EObjectFlags::Loaded - EObjectFlags::Unloaded;

#ifdef WITH_RTTI_VERIFY_PREDICATES
    // checks that base method was called :
    Assert_NoAssume(not (_flags ^ EObjectFlags::Verifying));

    _flags = _flags + EObjectFlags::Verifying;

    RTTI_VerifyPredicates();

    Assert_NoAssume(not (_flags ^ EObjectFlags::Verifying)); // checks that parent method was called
#endif

    context.OnLoadObject(*this);

    Assert_NoAssume(RTTI_IsLoaded());
    Assert_NoAssume(not RTTI_IsUnloaded());
}
//----------------------------------------------------------------------------
void FMetaObject::RTTI_Unload(IUnloadContext& context) {
    Assert_NoAssume(RTTI_IsLoaded());
    Assert_NoAssume(not RTTI_IsFrozen());
    Assert_NoAssume(not RTTI_IsUnloaded());

    context.OnUnloadObject(*this);

    _flags = _flags - EObjectFlags::Loaded + EObjectFlags::Unloaded;

    Assert_NoAssume(not RTTI_IsLoaded());
    Assert_NoAssume(RTTI_IsUnloaded());
    Assert(nullptr == _outer);
}
//----------------------------------------------------------------------------
bool FMetaObject::RTTI_CallLoadIFN(ILoadContext& context) {
    Assert_NoAssume(not RTTI_IsFrozen());

    if (RTTI_IsLoaded()) {
        Assert(not RTTI_IsUnloaded());
        return false;
    }
    else {
        RTTI_Load(context);
        return true;
    }
}
//----------------------------------------------------------------------------
bool FMetaObject::RTTI_CallUnloadIFN(IUnloadContext& context) {
    Assert_NoAssume(not RTTI_IsFrozen());

    if (RTTI_IsLoaded()) {
        RTTI_Unload(context);
        return true;
    }
    else {
        Assert(RTTI_IsUnloaded());
        return false;
    }
}
//----------------------------------------------------------------------------
void FMetaObject::RTTI_MarkAsTopObject() {
    Assert_NoAssume(RTTI_IsUnloaded());
    Assert_NoAssume(not RTTI_IsFrozen());
    Assert_NoAssume(not RTTI_IsTopObject());

    _flags = _flags + EObjectFlags::TopObject;
}
//----------------------------------------------------------------------------
void FMetaObject::RTTI_UnmarkAsTopObject() {
    Assert_NoAssume(RTTI_IsUnloaded());
    Assert_NoAssume(not RTTI_IsFrozen());
    Assert_NoAssume(RTTI_IsTopObject());

    _flags = _flags - EObjectFlags::TopObject;
}
//----------------------------------------------------------------------------
#ifdef WITH_RTTI_VERIFY_PREDICATES
void FMetaObject::RTTI_VerifyPredicates() const PPE_THROW() {
    if (not (_flags ^ EObjectFlags::Verifying))
        PPE_THROW_IT(FClassException("missing call to RTTI_parent_type::RTTI_VerifyPredicates()", RTTI_Class()));

    _flags = _flags - EObjectFlags::Verifying;
}
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FMetaObject::RTTI_FMetaClass::RTTI_FMetaClass(FClassId id, const FMetaModule* module) NOEXCEPT
:   FMetaClass(id, FName("FMetaObject"), EClassFlags::Abstract, module)
{}
//----------------------------------------------------------------------------
const FMetaClass* FMetaObject::RTTI_FMetaClass::Parent() const NOEXCEPT {
    return nullptr;
}
//----------------------------------------------------------------------------
bool FMetaObject::RTTI_FMetaClass::CreateInstance(PMetaObject& dst, bool resetToDefaultValue/* = true */) const {
    return RTTI::CreateMetaObject<FMetaObject>(dst, resetToDefaultValue);
}
//----------------------------------------------------------------------------
PTypeTraits FMetaObject::RTTI_FMetaClass::MakeTraits() const NOEXCEPT {
    return RTTI::MakeTraits<PMetaObject>();
}
//----------------------------------------------------------------------------
const FMetaClass* FMetaObject::RTTI_FMetaClass::Get() NOEXCEPT {
    Assert(GMetaObject_MetaClassHandle_.Class());
    return GMetaObject_MetaClassHandle_.Class();
}
//----------------------------------------------------------------------------
FMetaModule& FMetaObject::RTTI_FMetaClass::Module() NOEXCEPT {
    return RTTI_MODULE(RTTI);
}
//----------------------------------------------------------------------------
FMetaClass* FMetaObject::RTTI_FMetaClass::CreateMetaClass(FClassId id, const FMetaModule* module) {
    return TRACKING_NEW(MetaClass, FMetaObject::RTTI_FMetaClass) { id, module };
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, RTTI::EObjectFlags flags) {
    if (flags == RTTI::EObjectFlags::None)
        return oss << "None";

    auto sep = Fmt::NotFirstTime('|');

    if (flags & RTTI::EObjectFlags::Loaded)     { oss << sep << "Loaded"; }
    if (flags & RTTI::EObjectFlags::Unloaded)   { oss << sep << "Unloaded"; }
    if (flags & RTTI::EObjectFlags::Exported)   { oss << sep << "Exported"; }
    if (flags & RTTI::EObjectFlags::TopObject)  { oss << sep << "TopObject"; }
    if (flags & RTTI::EObjectFlags::Transient)  { oss << sep << "Transient"; }
    if (flags & RTTI::EObjectFlags::Frozen)     { oss << sep << "Frozen"; }
#ifdef WITH_RTTI_VERIFY_PREDICATES
    if (flags & RTTI::EObjectFlags::Verifying)  { oss << sep << "Verifying"; }
#endif

    return oss;
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, RTTI::EObjectFlags flags) {
    if (flags == RTTI::EObjectFlags::None)
        return oss << L"None";

    auto sep = Fmt::NotFirstTime(L'|');

    if (flags & RTTI::EObjectFlags::Loaded)     { oss << sep << L"Loaded"; }
    if (flags & RTTI::EObjectFlags::Unloaded)   { oss << sep << L"Unloaded"; }
    if (flags & RTTI::EObjectFlags::Exported)   { oss << sep << L"Exported"; }
    if (flags & RTTI::EObjectFlags::TopObject)  { oss << sep << L"TopObject"; }
    if (flags & RTTI::EObjectFlags::Transient)  { oss << sep << L"Transient"; }
    if (flags & RTTI::EObjectFlags::Frozen)     { oss << sep << L"Frozen"; }
#ifdef WITH_RTTI_VERIFY_PREDICATES
    if (flags & RTTI::EObjectFlags::Verifying)  { oss << sep << L"Verifying"; }
#endif

    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
