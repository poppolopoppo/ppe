﻿// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "TransactionLinker.h"

#include "MetaClass.h"
#include "MetaDatabase.h"
#include "MetaObject.h"
#include "MetaTransaction.h"
#include "IO/TextWriter.h"
#include "RTTI/TypeTraits.h"

namespace PPE {
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FClassNotFound::FClassNotFound(const RTTI::FName& name)
:   FLinkerException("failed to find meta class")
,   _name(name)
{}
//----------------------------------------------------------------------------
#if USE_PPE_EXCEPTION_DESCRIPTION
FTextWriter& FClassNotFound::Description(FTextWriter& oss) const {
    return oss
        << MakeCStringView(What())
        << ": while looking for <"
        << _name
        << "> !";
}
#endif
//----------------------------------------------------------------------------
FObjectNotFound::FObjectNotFound(const RTTI::FPathName& path)
:   FLinkerException("failed to find imported object")
,   _path(path)
{}
//----------------------------------------------------------------------------
#if USE_PPE_EXCEPTION_DESCRIPTION
FTextWriter& FObjectNotFound::Description(FTextWriter& oss) const {
    return oss
        << MakeCStringView(What())
        << ": while looking for '"
        << _path
        << "' !";
}
#endif
//----------------------------------------------------------------------------
FUnexpectedObjectClass::FUnexpectedObjectClass(const RTTI::FMetaClass* expected, const RTTI::FMetaClass* found)
:   FLinkerException("unexpected object class")
,   _expected(expected)
,   _found(found)
{}
//----------------------------------------------------------------------------
#if USE_PPE_EXCEPTION_DESCRIPTION
FTextWriter& FUnexpectedObjectClass::Description(FTextWriter& oss) const {
    return oss
        << MakeCStringView(What())
        << ": got <"
        << _found->Name()
        << "> instead of <"
        << _expected->Name()
        << "> !";
}
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTransactionLinker::FTransactionLinker() = default;
//----------------------------------------------------------------------------
FTransactionLinker::FTransactionLinker(FTransactionLinker&& rvalue) NOEXCEPT {
    operator =(std::move(rvalue));
}
//----------------------------------------------------------------------------
FTransactionLinker& FTransactionLinker::operator =(FTransactionLinker&& rvalue) NOEXCEPT {
    _filename = std::move(rvalue._filename);
    _topObjects = std::move(rvalue._topObjects);
    _imports = std::move(rvalue._imports);
    return (*this);
}
//----------------------------------------------------------------------------
FTransactionLinker::FTransactionLinker(Meta::FForceInit) NOEXCEPT
{} // used for transient serialization
//----------------------------------------------------------------------------
FTransactionLinker::FTransactionLinker(const FFilename& filename)
:   _filename(filename) {
    Assert_NoAssume(not _filename.empty());
}
//----------------------------------------------------------------------------
FTransactionLinker::~FTransactionLinker() = default;
//----------------------------------------------------------------------------
void FTransactionLinker::AddTopObject(RTTI::FMetaObject* topObject) {
    Assert(topObject);

    _topObjects.emplace_back(topObject);
}
//----------------------------------------------------------------------------
void FTransactionLinker::AddExport(const RTTI::FName& name, const RTTI::PMetaObject& src) {
    Assert_NoAssume(not name.empty());
    Assert(src);

    src->RTTI_Export(name);
}
//----------------------------------------------------------------------------
void FTransactionLinker::AddImport(const RTTI::FPathName& path, const RTTI::PTypeTraits& traits, RTTI::PMetaObject* dst) {
    Assert(dst);
    Assert_NoAssume(not path.empty());
    Assert_NoAssume(traits);
    Assert_NoAssume(traits->AsScalar());
    Assert_NoAssume(traits->AsScalar()->ObjectClass());
    Assert_NoAssume(not *dst);

    FImport_* imp = _imports.push_back_Uninitialized();
    imp->Path = path;
    imp->Traits = traits;
    imp->Dst = dst;
}
//----------------------------------------------------------------------------
void FTransactionLinker::AppendTo(FTransactionLinker& other) const {
    other._topObjects.insert(other._topObjects.end(), _topObjects.begin(), _topObjects.end());
    other._imports.insert(other._imports.end(), _imports.begin(), _imports.end());
}
//----------------------------------------------------------------------------
void FTransactionLinker::MoveTo(FTransactionLinker& other) {
    other._filename = _filename;
    other._topObjects = std::move(_topObjects);
    other._imports = std::move(_imports);
}
//----------------------------------------------------------------------------
void FTransactionLinker::Resolve(RTTI::FMetaTransaction& loaded) {
    Assert_NoAssume(not loaded.IsLoaded());

    {
        const RTTI::FMetaDatabaseReadable metaDB;

        for (const FImport_& imp : _imports) {
            Assert_NoAssume(not *imp.Dst);
            imp.Dst->reset(ResolveImport(metaDB, imp.Path, imp.Traits));
        }
    }

    for (const RTTI::PMetaObject& o : _topObjects)
        loaded.Add(o.get());
}
//----------------------------------------------------------------------------
void FTransactionLinker::CheckAssignment(const RTTI::PTypeTraits& traits, const RTTI::FMetaObject& obj) const {
#if !(USE_PPE_FINAL_RELEASE || USE_PPE_PROFILING) // unchecked assignment for optimized builds
    Assert_NoAssume(traits);

    const RTTI::FMetaClass* const expected = traits->ToScalar().ObjectClass();
    const RTTI::FMetaClass* const found = obj.RTTI_Class();
    Assert(expected);
    Assert(found);

    if (not expected->IsAssignableFrom(*found))
        PPE_THROW_IT(FUnexpectedObjectClass(expected, found));
#else
    Unused(traits);
    Unused(obj);
#endif
}
//----------------------------------------------------------------------------
RTTI::FMetaObject* FTransactionLinker::ResolveImport(
    const RTTI::FMetaDatabaseReadable& metaDb,
    const RTTI::FPathName& path,
    const RTTI::PTypeTraits& traits ) const {

    if (RTTI::FMetaObject* obj = metaDb->ObjectIFP(path)) {
#if !(USE_PPE_FINAL_RELEASE || USE_PPE_PROFILING) // unchecked assignment for optimized builds
        if (traits)
            CheckAssignment(traits, *obj);
#else
        Unused(traits);
#endif

        return obj;
    }
    else {
        PPE_THROW_IT(FObjectNotFound(path));
    }
}
//----------------------------------------------------------------------------
const RTTI::FMetaClass* FTransactionLinker::ResolveClass(
    const RTTI::FMetaDatabaseReadable& metaDb,
    const RTTI::FName& name) const {
    Assert_NoAssume(not name.empty());

    const RTTI::FMetaClass* klass = metaDb->ClassIFP(name);
    if (nullptr == klass)
        PPE_THROW_IT(FClassNotFound(name));

    return klass;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace PPE
