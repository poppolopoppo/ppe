#include "stdafx.h"

#include "MetaObject.h"

#include "MetaClass.h"
#include "MetaObjectHelpers.h"
#include "MetaTransaction.h"

#include "RTTI_Namespace.h"

#include "Core/IO/FormatHelpers.h"
#include "Core/IO/TextWriter.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FMetaObject::FMetaObject()
    : _flags(EObjectFlags::Unloaded)
{}
//----------------------------------------------------------------------------
FMetaObject::~FMetaObject() {
    Assert(RTTI_IsUnloaded());
    Assert(not RTTI_IsLoaded());
    Assert(not RTTI_IsExported());
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
void FMetaObject::RTTI_Export(const FName& name) {
    Assert(RTTI_IsLoaded());
    Assert(not RTTI_IsExported());
    Assert(_name.empty());

    _flags = _flags + EObjectFlags::Exported;
    _name = name;

    Assert(RTTI_IsExported());
}
//----------------------------------------------------------------------------
void FMetaObject::RTTI_Unexport() {
    Assert(RTTI_IsLoaded());
    Assert(RTTI_IsExported());
    Assert(not _name.empty());

    _flags = _flags - EObjectFlags::Exported;
    _name = FName();

    Assert(not RTTI_IsExported());
}
//----------------------------------------------------------------------------
void FMetaObject::RTTI_Load(ILoadContext* context) {
    Assert(not RTTI_IsLoaded());
    Assert(RTTI_IsUnloaded());

    _flags = _flags + EObjectFlags::Loaded - EObjectFlags::Unloaded;

#ifdef WITH_RTTI_VERIFY_PREDICATES
    // checks that base method was called :
    Assert(not (_flags ^ EObjectFlags::Verifying));

    _flags = _flags + EObjectFlags::Verifying;

    RTTI_VerifyPredicates();

    Assert(not (_flags ^ EObjectFlags::Verifying)); // checks that parent method was called
#endif

    if (context)
        context->OnLoadObject(*this);

    Assert(RTTI_IsLoaded());
    Assert(not RTTI_IsUnloaded());
}
//----------------------------------------------------------------------------
void FMetaObject::RTTI_Unload(IUnloadContext* context) {
    Assert(RTTI_IsLoaded());
    Assert(not RTTI_IsUnloaded());

    if (context)
        context->OnUnloadObject(*this);

    _flags = _flags - EObjectFlags::Loaded + EObjectFlags::Unloaded;

    Assert(not RTTI_IsLoaded());
    Assert(RTTI_IsUnloaded());
}
//----------------------------------------------------------------------------
void FMetaObject::RTTI_CallLoadIFN(ILoadContext* context) {
    if (not RTTI_IsLoaded())
        RTTI_Load(context);

    Assert(RTTI_IsLoaded());
}
//----------------------------------------------------------------------------
void FMetaObject::RTTI_CallUnloadIFN(IUnloadContext* context) {
    if (RTTI_IsLoaded())
        RTTI_Unload(context);

    Assert(RTTI_IsUnloaded());
}
//----------------------------------------------------------------------------
void FMetaObject::RTTI_MarkAsTopObject() {
    Assert(RTTI_IsUnloaded());
    Assert(not RTTI_IsTopObject());

    _flags = _flags + EObjectFlags::TopObject;
}
//----------------------------------------------------------------------------
void FMetaObject::RTTI_UnmarkAsTopObject() {
    Assert(RTTI_IsUnloaded());
    Assert(RTTI_IsTopObject());

    _flags = _flags - EObjectFlags::TopObject;
}
//----------------------------------------------------------------------------
#ifdef WITH_RTTI_VERIFY_PREDICATES
void FMetaObject::RTTI_VerifyPredicates() const {
    Assert(_flags ^ EObjectFlags::Verifying);

    _flags = _flags - EObjectFlags::Verifying;
}
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FMetaNamespace& FMetaObject::RTTI_FMetaClass::Namespace() {
    return RTTI_NAMESPACE(RTTI);
}
//----------------------------------------------------------------------------
FMetaObject::RTTI_FMetaClass::RTTI_FMetaClass(FClassId id, const FMetaNamespace* metaNamespace)
    : metaclass_type(id, FName("FMetaObject"), EClassFlags::Abstract, metaNamespace)
{}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, RTTI::EObjectFlags flags) {
    if (flags == RTTI::EObjectFlags::None)
        return oss << "None";

    auto sep = Fmt::NotFirstTime('|');

    if (flags & RTTI::EObjectFlags::Loaded)     { oss << sep << L"Loaded"; }
    if (flags & RTTI::EObjectFlags::Unloaded)   { oss << sep << L"Unloaded"; }
    if (flags & RTTI::EObjectFlags::Exported)   { oss << sep << L"Exported"; }
    if (flags & RTTI::EObjectFlags::TopObject)  { oss << sep << L"TopObject"; }
#ifdef WITH_RTTI_VERIFY_PREDICATES
    if (flags & RTTI::EObjectFlags::Verifying)  { oss << sep << L"Verifying"; }
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
#ifdef WITH_RTTI_VERIFY_PREDICATES
    if (flags & RTTI::EObjectFlags::Verifying)  { oss << sep << L"Verifying"; }
#endif

    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
