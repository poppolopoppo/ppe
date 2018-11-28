#include "stdafx.h"

#include "TransactionSaver.h"

#include "MetaObject.h"

namespace PPE {
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTransactionSaver::FTransactionSaver(
    const RTTI::FName& namespace_,
    const FFilename& filename )
:   _namespace(namespace_)
,   _filename(filename) {
    Assert_NoAssume(not _namespace.empty());
    Assert_NoAssume(not _filename.empty());
}
//----------------------------------------------------------------------------
FTransactionSaver::FTransactionSaver(
    const RTTI::FMetaTransaction& transaction,
    const FFilename& filename )
:   FTransactionSaver(transaction.Name(), filename) {

    Append(transaction);
}
//----------------------------------------------------------------------------
FTransactionSaver::~FTransactionSaver() {
    // unfreeze all object before exiting
    for (const RTTI::FMetaObjectRef& obj : _objects)
        obj->RTTI_Unfreeze();
}
//----------------------------------------------------------------------------
void FTransactionSaver::Append(const RTTI::FMetaObjectRef& obj) {
    Assert(obj.Get());

    Add_AssertUnique(_objects, obj);

    // freeze object before serialization
    obj->RTTI_Freeze();
}
//----------------------------------------------------------------------------
void FTransactionSaver::Append(const RTTI::FMetaTransaction& transaction) {
    Assert(transaction.IsLoaded());

    const size_t sizeBefore = _objects.size();
    transaction.Linearize(&_objects);

    // freeze added objects before serialization
    forrange(it, _objects.begin() + sizeBefore, _objects.end())
        (*it)->RTTI_Freeze();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace PPE
