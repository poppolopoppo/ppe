// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "MetaObject.h"

#include "MetaClass.h"
#include "MetaFunctionHelpers.h"
#include "MetaModule.h"
#include "MetaObjectHelpers.h"
#include "MetaProperty.h"
#include "MetaTransaction.h"

#include "RTTI/Atom.h"
#include "RTTI/Exceptions.h"
#include "RTTI/Module.h"
#include "RTTI/NativeTypes.h"

#include "IO/FormatHelpers.h"
#include "IO/TextWriter.h"
#include "RTTI/Any.h"
#include "RTTI/UserFacetHelpers.h"

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
template <typename _Key>
static const FMetaFunction* RTTI_Function_(const FMetaClass* const klass, _Key funcName) {
    if (Likely(klass)) {
        if (const FMetaFunction* const func = klass->FunctionIFP(funcName))
            return func;
    }
    return nullptr;
}
//----------------------------------------------------------------------------
template <typename _Key>
static const FMetaProperty* RTTI_Property_(const FMetaClass* const klass, _Key propName) {
    if (Likely(klass)) {
        if (const FMetaProperty* const prop = klass->PropertyIFP(propName))
            return prop;
    }
    return nullptr;
}
//----------------------------------------------------------------------------
} //!details
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Used for forward-declaration of PMetaObject
//----------------------------------------------------------------------------
void AddRef(const FMetaObject* objref) {
    PPE::AddRef(static_cast<const FMetaObject*>(objref));
}
//----------------------------------------------------------------------------
void RemoveRef(FMetaObject* objref) {
    PPE::RemoveRef<FMetaObject>(static_cast<FMetaObject*>(objref));
}
//----------------------------------------------------------------------------
void RemoveRef(const FMetaObject* objref) {
	PPE::RemoveRef<const FMetaObject>(static_cast<const FMetaObject*>(objref));
}
//----------------------------------------------------------------------------
hash_t hash_value(const PMetaObject& pobj) {
    return PPE::hash_value(pobj);
}
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
    *pFunc = RTTI_Function_(klass, funcName);

    return (!!*pFunc);
}
//----------------------------------------------------------------------------
bool FMetaObject::RTTI_Function(const FStringView& funcName, const FMetaFunction** pFunc) const NOEXCEPT {
    return RTTI_Function(FLazyName{ funcName }, pFunc);
}
//----------------------------------------------------------------------------
bool FMetaObject::RTTI_Function(const FLazyName& funcName, const FMetaFunction** pFunc) const NOEXCEPT {
    Assert_NoAssume(not funcName.empty());
    Assert(pFunc);

    const FMetaClass* const klass = RTTI_Class();
    *pFunc = RTTI_Function_(klass, funcName);

    return (!!*pFunc);
}
//----------------------------------------------------------------------------
bool FMetaObject::RTTI_Property(const FName& propName, const FMetaProperty** pProp) const NOEXCEPT {
    Assert_NoAssume(not propName.empty());
    Assert(pProp);

    *pProp = RTTI_Property_(RTTI_Class(), propName);
    return (!!*pProp);
}
//----------------------------------------------------------------------------
bool FMetaObject::RTTI_Property(const FStringView& propName, const FMetaProperty** pProp) const NOEXCEPT {
    return RTTI_Property(FLazyName{ propName }, pProp);
}
//----------------------------------------------------------------------------
bool FMetaObject::RTTI_Property(const FLazyName& propName, const FMetaProperty** pProp) const NOEXCEPT {
    Assert_NoAssume(not propName.empty());
    Assert(pProp);

    *pProp = RTTI_Property_(RTTI_Class(), propName);
    return (!!*pProp);
}
//----------------------------------------------------------------------------
bool FMetaObject::RTTI_Property(const FName& propName, FAtom* pValue) const NOEXCEPT {
    Assert_NoAssume(not propName.empty());
    Assert(pValue);

    const FMetaClass* const klass = RTTI_Class();
    if (const FMetaProperty* const prop = RTTI_Property_(klass, propName)) {
        *pValue = prop->Get(*this);
        return true;
    }

    return false;
}
//----------------------------------------------------------------------------
bool FMetaObject::RTTI_Property(const FStringView& propName, FAtom* pValue) const NOEXCEPT {
    return RTTI_Property(FLazyName{ propName }, pValue);
}
//----------------------------------------------------------------------------
bool FMetaObject::RTTI_Property(const FLazyName& propName, FAtom* pValue) const NOEXCEPT {
    Assert_NoAssume(not propName.empty());
    Assert(pValue);

    const FMetaClass* const klass = RTTI_Class();
    if (const FMetaProperty* const prop = RTTI_Property_(klass, propName)) {
        *pValue = prop->Get(*this);
        return true;
    }

    return false;
}
//----------------------------------------------------------------------------
bool FMetaObject::RTTI_PropertyCopyFrom(const FName& propName, const FAtom& src) {
    Assert_NoAssume(not RTTI_IsFrozen());
    Assert_NoAssume(not propName.empty());
    Assert(src);

    const FMetaClass* const klass = RTTI_Class();
    if (const FMetaProperty* const prop = RTTI_Property_(klass, propName)) {
        const FAtom dst = prop->Get(*this);
        src.Copy(dst);
        prop->Facets().Decorate(*prop, dst.Data());
        return true;
    }

    return false;
}
//----------------------------------------------------------------------------
bool FMetaObject::RTTI_PropertyCopyFrom(const FStringView& propName, const FAtom& src) {
    Assert_NoAssume(not RTTI_IsFrozen());
    Assert_NoAssume(not propName.empty());
    Assert(src);

    const FMetaClass* const klass = RTTI_Class();
    if (const FMetaProperty* const prop = RTTI_Property_(klass, propName)) {
        const FAtom dst = prop->Get(*this);
        src.Copy(dst);
        prop->Facets().Decorate(*prop, dst.Data());
        return true;
    }

    return false;
}
//----------------------------------------------------------------------------
bool FMetaObject::RTTI_PropertyMoveFrom(const FName& propName, FAtom& src) NOEXCEPT {
    Assert_NoAssume(not RTTI_IsFrozen());
    Assert_NoAssume(not propName.empty());
    Assert(src);

    const FMetaClass* const klass = RTTI_Class();
    if (const FMetaProperty* const prop = RTTI_Property_(klass, propName)) {
        const FAtom dst = prop->Get(*this);
        src.Move(dst);
        prop->Facets().Decorate(*prop, dst.Data());
        return true;
    }

    return false;
}
//----------------------------------------------------------------------------
bool FMetaObject::RTTI_PropertyMoveFrom(const FStringView& propName, FAtom& src) NOEXCEPT {
    Assert_NoAssume(not RTTI_IsFrozen());
    Assert_NoAssume(not propName.empty());
    Assert(src);

    const FMetaClass* const klass = RTTI_Class();
    if (const FMetaProperty* const prop = RTTI_Property_(klass, propName)) {
        prop->MoveFrom(*this, src);
        return true;
    }

    return false;
}
//----------------------------------------------------------------------------
bool FMetaObject::RTTI_PropertySet(const FName& propName, const FAny& src) {
    Assert_NoAssume(not RTTI_IsFrozen());
    Assert_NoAssume(not propName.empty());
    Assert(src.InnerAtom());

    const FMetaClass* const klass = RTTI_Class();
    if (const FMetaProperty* const prop = RTTI_Property_(klass, propName)) {
        FAtom const dst = prop->Get(*this);
        if (src.PromoteCopy(dst)) {
            prop->Facets().Decorate(*prop, dst.Data());
            return true;
        }
    }

    return false;
}
//----------------------------------------------------------------------------
bool FMetaObject::RTTI_PropertyAdd(const FName& propName, const FAny& item) {
    Assert_NoAssume(not RTTI_IsFrozen());
    Assert_NoAssume(not propName.empty());
    Assert(item.InnerAtom());

    const FMetaClass* const klass = RTTI_Class();
    if (const FMetaProperty* const prop = RTTI_Property_(klass, propName)) {
        FAtom const dst = prop->Get(*this);
        if (const IListTraits* const plist = prop->Traits()->AsList()) {
            if (item.Traits() == plist->ValueTraits()) {
                plist->AddCopy(dst.Data(), item.InnerAtom());
                prop->Facets().Decorate(*prop, dst.Data());
                return true;
            }
            else {
                STACKLOCAL_ATOM(promoted, plist->ValueTraits());
                if (item.PromoteCopy(promoted)) {
                    plist->AddMove(dst.Data(), promoted);
                    prop->Facets().Decorate(*prop, dst.Data());
                    return true;
                }
            }
        }
    }

    return false;
}
//----------------------------------------------------------------------------
bool FMetaObject::RTTI_PropertyRemove(const FName& propName, const FAny& item) {
    Assert_NoAssume(not RTTI_IsFrozen());
    Assert_NoAssume(not propName.empty());
    Assert(item.InnerAtom());

    const FMetaClass* const klass = RTTI_Class();
    if (const FMetaProperty* const prop = RTTI_Property_(klass, propName)) {
        FAtom const dst = prop->Get(*this);
        if (const IListTraits* const plist = prop->Traits()->AsList()) {
            if (item.Traits() == plist->ValueTraits()) {
                plist->Remove(dst.Data(), item.InnerAtom());
                prop->Facets().Decorate(*prop, dst.Data());
                return true;
            }
            else {
                STACKLOCAL_ATOM(promoted, plist->ValueTraits());
                if (item.PromoteCopy(promoted)) {
                    plist->Remove(dst.Data(), promoted);
                    prop->Facets().Decorate(*prop, dst.Data());
                    return true;
                }
            }
        }
    }

    return false;
}
//----------------------------------------------------------------------------
bool FMetaObject::RTTI_PropertyInsert(const FName& propName, const FAny& key, const FAny& value) {
    Assert_NoAssume(not RTTI_IsFrozen());
    Assert_NoAssume(not propName.empty());
    Assert(key.InnerAtom());
    Assert(value.InnerAtom());

    const FMetaClass* const klass = RTTI_Class();
    if (const FMetaProperty* const prop = RTTI_Property_(klass, propName)) {
        FAtom const dst = prop->Get(*this);
        if (const IDicoTraits* const pdico = prop->Traits()->AsDico()) {
            if (pdico->KeyTraits() == key.Traits() &&
                pdico->ValueTraits() == value.Traits() ) {
                pdico->AddCopy(dst.Data(), key.InnerAtom(), value.InnerAtom());
                prop->Facets().Decorate(*prop, dst.Data());
                return true;
            }
            else {
                STACKLOCAL_ATOM(promotedKey, pdico->KeyTraits());
                if (not key.PromoteCopy(promotedKey))
                    return false;

                STACKLOCAL_ATOM(promotedValue, pdico->ValueTraits());
                if (not value.PromoteCopy(promotedValue))
                    return false;

                pdico->AddMove(dst.Data(), promotedKey, promotedValue);
                prop->Facets().Decorate(*prop, dst.Data());
                return true;
            }
        }
    }

    return false;
}
//----------------------------------------------------------------------------
bool FMetaObject::RTTI_PropertyErase(const FName& propName, const FAny& key) {
    Assert_NoAssume(not RTTI_IsFrozen());
    Assert_NoAssume(not propName.empty());
    Assert(key.InnerAtom());

    const FMetaClass* const klass = RTTI_Class();
    if (const FMetaProperty* const prop = RTTI_Property_(klass, propName)) {
        FAtom const dst = prop->Get(*this);
        if (const IDicoTraits* const pdico = prop->Traits()->AsDico()) {
            if (pdico->KeyTraits() == key.Traits()) {
                if (pdico->Remove(dst.Data(), key.InnerAtom())) {
                    prop->Facets().Decorate(*prop, dst.Data());
                    return true;
                }
                return false;
            }
            else {
                STACKLOCAL_ATOM(promotedKey, pdico->KeyTraits());
                if (key.PromoteCopy(promotedKey)) {
                    pdico->Remove(dst.Data(), promotedKey);
                    prop->Facets().Decorate(*prop, dst.Data());
                    return true;
                }
            }
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

    const FMetaClass* const metaClass = RTTI_Class();

    // validate class facets
    metaClass->Facets().Validate(*metaClass, this);

    // validate property facets
    for (const FMetaProperty* prop : metaClass->AllProperties())
        prop->Facets().Decorate(*prop, prop->Get(*this).Data());

    _flags = _flags - EObjectFlags::Verifying;
}
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FMetaObject::RTTI_FMetaClass::RTTI_FMetaClass(FClassId id, const FMetaModule* module) NOEXCEPT
:   FMetaClass(id, FName("MetaObject"), EClassFlags::Abstract, module) {
    // Register common user facets

    Facets().Emplace<FDescriptionFacet>("base class of any object");

    // Register common RTTI functions

    RegisterFunction(RTTI::MakeFunction<&FMetaObject::RTTI_PropertySet>(
        RTTI::FName("_set"), { "propertyName", "value" }) );
    RegisterFunction(RTTI::MakeFunction<&FMetaObject::RTTI_PropertyAdd>(
        RTTI::FName("_add"), { "propertyName", "item" }) );
    RegisterFunction(RTTI::MakeFunction<&FMetaObject::RTTI_PropertyRemove>(
        RTTI::FName("_remove"), { "propertyName", "item" }) );
    RegisterFunction(RTTI::MakeFunction<&FMetaObject::RTTI_PropertyInsert>(
        RTTI::FName("_insert"), { "propertyName", "key", "value" }) );
    RegisterFunction(RTTI::MakeFunction<&FMetaObject::RTTI_PropertyErase>(
        RTTI::FName("_erase"), { "propertyName", "key" }) );
}
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
