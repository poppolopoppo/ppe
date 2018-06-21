#include "stdafx.h"

#include "MetaDatabase.h"

#include "MetaObject.h"
#include "MetaNamespace.h"
#include "MetaTransaction.h"

#include "Core/Diagnostic/Logger.h"
#include "Core/IO/TextWriter.h"

namespace Core {
namespace RTTI {
EXTERN_LOG_CATEGORY(CORE_RTTI_API, RTTI)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FMetaDatabase::FMetaDatabase() {
    LOG(RTTI, Info, L"create meta database");
}
//----------------------------------------------------------------------------
FMetaDatabase::~FMetaDatabase() {
    LOG(RTTI, Info, L"destroy meta database");

    Assert(_namespaces.empty());
    Assert(_objects.empty());
    Assert(_classes.empty());
}
//----------------------------------------------------------------------------
// Transactions
//----------------------------------------------------------------------------
void FMetaDatabase::RegisterTransaction(FMetaTransaction* metaTransaction) {
    Assert(metaTransaction);
    Assert(metaTransaction->IsLoading());

    const FName& exportName = metaTransaction->Name();
    Assert(not exportName.empty());

    LOG(RTTI, Info, L"register transaction in DB : '{0}'", exportName);

    WRITESCOPELOCK(_lockRW);
    
    CORE_LEAKDETECTOR_WHITELIST_SCOPE();

    Insert_AssertUnique(_transactions, exportName, SMetaTransaction(metaTransaction));
}
//----------------------------------------------------------------------------
void FMetaDatabase::UnregisterTransaction(FMetaTransaction* metaTransaction) {
    Assert(metaTransaction);
    Assert(metaTransaction->IsUnloading());

    const FName& exportName = metaTransaction->Name();
    Assert(not exportName.empty());

    LOG(RTTI, Info, L"unregister transaction in DB : '{0}'", exportName);

    WRITESCOPELOCK(_lockRW);

#ifdef WITH_CORE_ASSERT
    // Check that all objects from this transaction were unregistered
    for (const auto& it : _objects)
        Assert(it.second->RTTI_Outer() != metaTransaction);
#endif

    CORE_LEAKDETECTOR_WHITELIST_SCOPE();

    Remove_AssertExistsAndSameValue(_transactions, exportName, SMetaTransaction(metaTransaction));
}
//----------------------------------------------------------------------------
FMetaTransaction& FMetaDatabase::Transaction(const FName& name) const {
    Assert(not name.empty());

    READSCOPELOCK(_lockRW);

    FMetaTransaction& transaction = (*_transactions[name]);
    Assert(transaction.IsLoaded());

    return transaction;
}
//----------------------------------------------------------------------------
FMetaTransaction* FMetaDatabase::TransactionIFP(const FName& name) const {
    Assert(not name.empty());

    READSCOPELOCK(_lockRW);

    const auto it = _transactions.find(name);
    if (_transactions.end() == it)
        return nullptr;

    Assert(it->second->IsLoaded());
    return it->second.get();
}
//----------------------------------------------------------------------------
FMetaTransaction* FMetaDatabase::TransactionIFP(const FStringView& name) const {
    Assert(not name.empty());

    READSCOPELOCK(_lockRW);

    const hash_t h = FName::HashValue(name);
    const auto it = _transactions.find_like(name, h);
    if (_transactions.end() == it)
        return nullptr;

    Assert(it->second->IsLoaded());
    return it->second.get();
}
//----------------------------------------------------------------------------
// Objects
//----------------------------------------------------------------------------
void FMetaDatabase::RegisterObject(FMetaObject* metaObject) {
    Assert(metaObject);
    Assert(metaObject->RTTI_Outer());
    Assert(metaObject->RTTI_Outer()->IsLoading());

    const FPathName exportPath{ *metaObject };

    LOG(RTTI, Info, L"register object in DB : <{0}::{1}> '{2}'",
        metaObject->RTTI_Class()->Namespace()->Name(),
        metaObject->RTTI_Class()->Name(),
        exportPath );

    Assert(metaObject->RTTI_IsExported());
    Assert(metaObject->RTTI_IsLoaded());

    WRITESCOPELOCK(_lockRW);

    Assert(Contains(_namespaces, metaObject->RTTI_Class()->Namespace()));
    Assert(_transactions.Contains(exportPath.Transaction));

    CORE_LEAKDETECTOR_WHITELIST_SCOPE();

    Insert_AssertUnique(_objects, exportPath, SMetaObject(metaObject));
}
//----------------------------------------------------------------------------
void FMetaDatabase::UnregisterObject(FMetaObject* metaObject) {
    Assert(metaObject);
    Assert(metaObject->RTTI_Outer());
    Assert(metaObject->RTTI_Outer()->IsUnloading());

    const FPathName exportPath{ *metaObject };

    LOG(RTTI, Info, L"unregister object from DB : <{0}::{1}> '{2}'",
        metaObject->RTTI_Class()->Namespace()->Name(),
        metaObject->RTTI_Class()->Name(),
        exportPath );

    Assert(metaObject->RTTI_IsExported());
    Assert(metaObject->RTTI_IsLoaded());

    WRITESCOPELOCK(_lockRW);

    Assert(Contains(_namespaces, metaObject->RTTI_Class()->Namespace()));
    Assert(_transactions.Contains(exportPath.Transaction));

    CORE_LEAKDETECTOR_WHITELIST_SCOPE();

#ifdef WITH_CORE_ASSERT
    Remove_AssertExistsAndSameValue(_objects, exportPath, SMetaObject{ metaObject });
#else
    _objects.erase(exportPath);
#endif
}
//----------------------------------------------------------------------------
FMetaObject& FMetaDatabase::Object(const FPathName& pathName) const {
    Assert(not pathName.empty());
    Assert(not pathName.Transaction.empty());

    READSCOPELOCK(_lockRW);

    FMetaObject& obj = (*_objects.at(pathName));
    Assert(obj.RTTI_IsLoaded());

    return obj;
}
//----------------------------------------------------------------------------
FMetaObject* FMetaDatabase::ObjectIFP(const FPathName& pathName) const {
    Assert(not pathName.empty());
    Assert(not pathName.Transaction.empty());

    READSCOPELOCK(_lockRW);

    const auto it = _objects.find(pathName);
    if (_objects.end() == it)
        return nullptr;

    Assert(it->second->RTTI_IsLoaded());
    return it->second.get();
}
//----------------------------------------------------------------------------
FMetaObject* FMetaDatabase::ObjectIFP(const FStringView& text) const {
    Assert(not text.empty());

    FPathName pathName;
    VerifyRelease(FPathName::Parse(&pathName, text));

    return ObjectIFP(pathName);
}
//----------------------------------------------------------------------------
// Namespaces
//----------------------------------------------------------------------------
void FMetaDatabase::RegisterNamespace(const FMetaNamespace* metaNamespace) {
    Assert(metaNamespace);

    LOG(RTTI, Info, L"register namespace in DB : <{0}>", metaNamespace->Name());

    Assert(metaNamespace->IsStarted());

    WRITESCOPELOCK(_lockRW);

    CORE_LEAKDETECTOR_WHITELIST_SCOPE();

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

    LOG(RTTI, Info, L"unregister namespace from DB : <{0}>", metaNamespace->Name());

    Assert(metaNamespace->IsStarted());

    WRITESCOPELOCK(_lockRW);

    CORE_LEAKDETECTOR_WHITELIST_SCOPE();

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
