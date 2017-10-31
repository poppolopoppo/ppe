#include "stdafx.h"

#include "MetaDatabase.h"

#include "MetaObject.h"
#include "MetaNamespace.h"
#include "MetaTransaction.h"

#include "Core/Diagnostic/Logger.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FMetaDatabase::FMetaDatabase() {
    LOG(Info, L"[RTTI] Create DB");
}
//----------------------------------------------------------------------------
FMetaDatabase::~FMetaDatabase() {
    LOG(Info, L"[RTTI] Destroy DB");

    Assert(_namespaces.empty());
    Assert(_objects.empty());
    Assert(_classes.empty());
}
//----------------------------------------------------------------------------
// Transactions
//----------------------------------------------------------------------------
void FMetaDatabase::RegisterTransaction(FMetaTransaction* metaTransaction) {
    Assert(metaTransaction);
    Assert(metaTransaction->IsLoaded());

    const FName& exportName = metaTransaction->Name();
    Assert(not exportName.empty());

    LOG(Info, L"[RTTI] Register transaction in DB : '{0}', {1} top objects, {2} loaded objects",
        exportName,
        metaTransaction->TopObjects().size(),
        metaTransaction->LoadedObjects().size() );

    WRITESCOPELOCK(_lockRW);

    Insert_AssertUnique(_transactions, exportName, SMetaTransaction(metaTransaction));
}
//----------------------------------------------------------------------------
void FMetaDatabase::UnregisterTransaction(FMetaTransaction* metaTransaction) {
    Assert(metaTransaction);
    Assert(metaTransaction->IsLoaded());

    const FName& exportName = metaTransaction->Name();
    Assert(not exportName.empty());

    LOG(Info, L"[RTTI] Unregister transaction in DB : '{0}', {1} top objects, {2} loaded objects",
        exportName,
        metaTransaction->TopObjects().size(),
        metaTransaction->LoadedObjects().size() );

    WRITESCOPELOCK(_lockRW);

    Remove_AssertExistsAndSameValue(_transactions, exportName, SMetaTransaction(metaTransaction));
}
//----------------------------------------------------------------------------
FMetaTransaction& FMetaDatabase::Transaction(const FName& name) const {
    Assert(not name.empty());

    READSCOPELOCK(_lockRW);
    return (*_transactions[name]);
}
//----------------------------------------------------------------------------
FMetaTransaction* FMetaDatabase::TransactionIFP(const FName& name) const {
    Assert(not name.empty());

    READSCOPELOCK(_lockRW);

    const auto it = _transactions.find(name);

    return (_transactions.end() == it ? nullptr : it->second);
}
//----------------------------------------------------------------------------
FMetaTransaction* FMetaDatabase::TransactionIFP(const FStringView& name) const {
    Assert(not name.empty());

    READSCOPELOCK(_lockRW);

    const hash_t h = FName::HashValue(name);
    const auto it = _transactions.find_like(name, h);

    return (_transactions.end() == it ? nullptr : it->second);
}
//----------------------------------------------------------------------------
// Objects
//----------------------------------------------------------------------------
void FMetaDatabase::RegisterObject(FMetaObject* metaObject) {
    Assert(metaObject);

    const FName& exportName = metaObject->RTTI_Name();
    Assert(not exportName.empty());

    LOG(Info, L"[RTTI] Register object in DB : <{0}::{1}> '{2}'",
        metaObject->RTTI_Class()->Namespace()->Name(),
        metaObject->RTTI_Class()->Name(),
        exportName );

    Assert(metaObject->RTTI_IsExported());

    WRITESCOPELOCK(_lockRW);

    Assert(Contains(_namespaces, metaObject->RTTI_Class()->Namespace()));

    Insert_AssertUnique(_objects, exportName, SMetaObject(metaObject));
}
//----------------------------------------------------------------------------
void FMetaDatabase::UnregisterObject(FMetaObject* metaObject) {
    Assert(metaObject);

    const FName& exportName = metaObject->RTTI_Name();
    Assert(not exportName.empty());

    LOG(Info, L"[RTTI] Unregister object from DB : <{0}::{1}> '{2}'",
        metaObject->RTTI_Class()->Namespace()->Name(),
        metaObject->RTTI_Class()->Name(),
        exportName );

    Assert(metaObject->RTTI_IsExported());

    WRITESCOPELOCK(_lockRW);

    Assert(Contains(_namespaces, metaObject->RTTI_Class()->Namespace()));

    Remove_AssertExistsAndSameValue(_objects, exportName, SMetaObject(metaObject));
}
//----------------------------------------------------------------------------
FMetaObject& FMetaDatabase::Object(const FName& name) const {
    Assert(not name.empty());

    READSCOPELOCK(_lockRW);
    return (*_objects[name]);
}
//----------------------------------------------------------------------------
FMetaObject* FMetaDatabase::ObjectIFP(const FName& name) const {
    Assert(not name.empty());

    READSCOPELOCK(_lockRW);

    const auto it = _objects.find(name);

    return (_objects.end() == it ? nullptr : it->second);
}
//----------------------------------------------------------------------------
FMetaObject* FMetaDatabase::ObjectIFP(const FStringView& name) const {
    Assert(not name.empty());

    READSCOPELOCK(_lockRW);

    const hash_t h = FName::HashValue(name);
    const auto it = _objects.find_like(name, h);

    return (_objects.end() == it ? nullptr : it->second);
}
//----------------------------------------------------------------------------
// Namespaces
//----------------------------------------------------------------------------
void FMetaDatabase::RegisterNamespace(const FMetaNamespace* metaNamespace) {
    Assert(metaNamespace);

    LOG(Info, L"[RTTI] Register namespace in DB : <{0}>", metaNamespace->Name());

    Assert(metaNamespace->IsStarted());

    WRITESCOPELOCK(_lockRW);

    Add_AssertUnique(_namespaces, metaNamespace);

    for (const FMetaClass* metaClass : metaNamespace->Classes()) {
        Assert(metaClass);
        Assert(metaClass->IsRegistered());
        Assert(metaClass->Namespace() == metaNamespace);

        Insert_AssertUnique(_classes, metaClass->Name(), metaClass);
    }
}
//----------------------------------------------------------------------------
void FMetaDatabase::UnregisterNamespace(const FMetaNamespace* metaNamespace) {
    Assert(metaNamespace);

    LOG(Info, L"[RTTI] Unregister namespace from DB : <{0}>", metaNamespace->Name());

    Assert(metaNamespace->IsStarted());

    WRITESCOPELOCK(_lockRW);

    for (const FMetaClass* metaClass : metaNamespace->Classes()) {
        Assert(metaClass);
        Assert(metaClass->IsRegistered());
        Assert(metaClass->Namespace() == metaNamespace);

        Remove_AssertExistsAndSameValue(_classes, metaClass->Name(), metaClass);
    }

    Remove_AssertExists(_namespaces, metaNamespace);

#ifdef WITH_CORE_ASSERT
    // Check that no object belonging to this namespace if still referenced
    for (const auto& it : _objects) {
        const FMetaClass* metaClass = it.second->RTTI_Class();
        Assert(metaClass->Namespace() != metaNamespace);
    }
#endif
}
//----------------------------------------------------------------------------
const FMetaNamespace& FMetaDatabase::Namespace(const FName& name) const {
    Assert(not name.empty());

    READSCOPELOCK(_lockRW);

    const auto it = std::find_if(_namespaces.begin(), _namespaces.end(), [&name](const FMetaNamespace* metaNamespace) {
        return (metaNamespace->Name() == name);
    });
    AssertRelease(_namespaces.end() != it);

    return (**it);
}
//----------------------------------------------------------------------------
const FMetaNamespace* FMetaDatabase::NamespaceIFP(const FName& name) const {
    Assert(not name.empty());

    READSCOPELOCK(_lockRW);

    const auto it = std::find_if(_namespaces.begin(), _namespaces.end(), [&name](const FMetaNamespace* metaNamespace) {
        return (metaNamespace->Name() == name);
    });

    return (_namespaces.end() == it ? nullptr : *it);
}
//----------------------------------------------------------------------------
const FMetaNamespace* FMetaDatabase::NamespaceIFP(const FStringView& name) const {
    Assert(not name.empty());

    READSCOPELOCK(_lockRW);

    const auto it = std::find_if(_namespaces.begin(), _namespaces.end(), [&name](const FMetaNamespace* metaNamespace) {
        return (metaNamespace->Name() == name);
    });

    return (_namespaces.end() == it ? nullptr : *it);
}
//----------------------------------------------------------------------------
// Classes
//----------------------------------------------------------------------------
const FMetaClass& FMetaDatabase::Class(const FName& name) const {
    Assert(not name.empty());

    READSCOPELOCK(_lockRW);

    return (*_classes.at(name));
}
//----------------------------------------------------------------------------
const FMetaClass* FMetaDatabase::ClassIFP(const FName& name) const {
    Assert(not name.empty());

    READSCOPELOCK(_lockRW);

    const auto it = _classes.find(name);

    return (_classes.end() == it ? nullptr : it->second);
}
//----------------------------------------------------------------------------
const FMetaClass* FMetaDatabase::ClassIFP(const FStringView& name) const {
    Assert(not name.empty());

    READSCOPELOCK(_lockRW);

    const hash_t h = FName::HashValue(name);
    const auto it = _classes.find_like(name, h);

    return (_classes.end() == it ? nullptr : it->second);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
