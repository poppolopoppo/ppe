#include "stdafx.h"

#include "TransactionLinker.h"

#include "MetaClass.h"
#include "MetaDatabase.h"
#include "MetaObject.h"
#include "MetaTransaction.h"
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
FObjectNotFound::FObjectNotFound(const RTTI::FPathName& path)
:   FLinkerException("failed to find imported object")
,   _path(path)
{}
//----------------------------------------------------------------------------
FUnexpectedObjectClass::FUnexpectedObjectClass(const RTTI::FMetaClass* expected, const RTTI::FMetaClass* found)
:   FLinkerException("unexpected object class")
,   _expected(expected)
,   _found(found)
{}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTransactionLinker::FTransactionLinker(RTTI::FMetaTransaction* loaded, const FFilename& filename)
:   _loaded(*loaded)
,   _filename(filename) {
    Assert_NoAssume(not _loaded.IsLoaded());
    Assert_NoAssume(not _filename.empty());
}
//----------------------------------------------------------------------------
FTransactionLinker::~FTransactionLinker()
{}
//----------------------------------------------------------------------------
void FTransactionLinker::AddTopObject(RTTI::FMetaObject* topObject) {
    Assert(topObject);

    _loaded.RegisterObject(topObject);
}
//----------------------------------------------------------------------------
void FTransactionLinker::AddExport(const RTTI::FName& name, const RTTI::PMetaObject& src) {
    Assert_NoAssume(not name.empty());
    Assert(src);

    src->RTTI_Export(name);
}
//----------------------------------------------------------------------------
void FTransactionLinker::AddImport(const RTTI::FPathName& path, const RTTI::PTypeTraits& traits, RTTI::PMetaObject* dst) {
    Assert_NoAssume(not path.empty());
    Assert_NoAssume(traits);
    Assert_NoAssume(traits->AsScalar());
    Assert_NoAssume(traits->AsScalar()->ObjectClass());
    Assert(dst);
    Assert_NoAssume(not *dst);
    Assert_NoAssume(path.Transaction != _loaded.Name());

    FImport_& imp = _imports.Add();
    imp.Path = path;
    imp.Traits = traits;
    imp.Dst = dst;
}
//----------------------------------------------------------------------------
void FTransactionLinker::ResolveImports() {
    const RTTI::FMetaDatabaseReadable metaDB;

    for (const FImport_& imp : _imports) {
        Assert_NoAssume(not *imp.Dst);
        imp.Dst->reset(ResolveImport(metaDB, imp.Path, imp.Traits));
    }
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
    UNUSED(traits);
    UNUSED(obj);
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
        UNUSED(traits);
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
