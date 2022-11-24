// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "TransactionSaver.h"

#include "MetaObject.h"
#include "MetaTransaction.h"

#define USE_PPE_SERIALIZE_FREEZE (USE_PPE_ASSERT)

namespace PPE {
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTransactionSaver::FTransactionSaver(
    const RTTI::FMetaTransaction& outer,
    const FFilename& filename )
:   _outer(&outer)
,   _filename(filename) {
    Assert_NoAssume(_outer->IsLoaded() || _outer->IsMounted());

#if USE_PPE_SERIALIZE_FREEZE
    for (const RTTI::SMetaObject& ref : _outer->Linearized().LoadedRefs)
        ref->RTTI_Freeze();
#endif
}
//----------------------------------------------------------------------------
TMemoryView<const RTTI::PMetaObject> FTransactionSaver::TopRefs() const {
    return _outer->TopObjects().MakeView();
}
//----------------------------------------------------------------------------
TMemoryView<const RTTI::SMetaObject> FTransactionSaver::ImportedRefs() const {
    return _outer->Linearized().ImportedRefs;
}
//----------------------------------------------------------------------------
TMemoryView<const RTTI::SMetaObject> FTransactionSaver::LoadedRefs() const {
    return _outer->Linearized().LoadedRefs;
}
//----------------------------------------------------------------------------
TMemoryView<const RTTI::SMetaObject> FTransactionSaver::ExportedRefs() const {
    return _outer->Linearized().ExportedRefs;
}
//----------------------------------------------------------------------------
FTransactionSaver::~FTransactionSaver() {
    Assert_NoAssume(_outer->IsLoaded() || _outer->IsMounted());

#if USE_PPE_SERIALIZE_FREEZE
    // unfreeze all object before exiting
    for (const RTTI::SMetaObject& ref : _outer->Linearized().LoadedRefs)
        ref->RTTI_Unfreeze();
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace PPE
