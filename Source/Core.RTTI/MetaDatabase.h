#pragma once

#include "Core.RTTI/RTTI.h"

#include "Core.RTTI/Typedefs.h"

#include "Core/Container/HashMap.h"
#include "Core/Meta/Singleton.h"
#include "Core/Thread/ReadWriteLock.h"

namespace Core {
namespace RTTI {
class FMetaClass;
FWD_REFPTR(MetaObject);
class FMetaNamespace;
FWD_REFPTR(MetaTransaction);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class CORE_RTTI_API FMetaDatabase : Meta::TSingleton<FMetaDatabase> {
public:
    /* Singleton */

#ifdef WITH_CORE_ASSERT
    using Meta::TSingleton<FMetaDatabase>::HasInstance;
#endif
    using Meta::TSingleton<FMetaDatabase>::Instance;

    static void Create() { Meta::TSingleton<FMetaDatabase>::Create(); }
    using Meta::TSingleton<FMetaDatabase>::Destroy;

    /* Transactions */

    void RegisterTransaction(FMetaTransaction* metaTransaction);
    void UnregisterTransaction(FMetaTransaction* metaTransaction);

    FMetaTransaction& Transaction(const FName& name) const;
    FMetaTransaction* TransactionIFP(const FName& name) const;
    FMetaTransaction* TransactionIFP(const FStringView& name) const;

    /* Objects */

    void RegisterObject(FMetaObject* metaObject);
    void UnregisterObject(FMetaObject* metaObject);

    FMetaObject& Object(const FName& name) const;
    FMetaObject* ObjectIFP(const FName& name) const;
    FMetaObject* ObjectIFP(const FStringView& name) const;

    /* Namespaces */

    void RegisterNamespace(const FMetaNamespace* metaNamespace);
    void UnregisterNamespace(const FMetaNamespace* metaNamespace);

    const FMetaNamespace& Namespace(const FName& name) const;
    const FMetaNamespace* NamespaceIFP(const FName& name) const;
    const FMetaNamespace* NamespaceIFP(const FStringView& name) const;

    /* Classes */

    const FMetaClass& Class(const FName& name) const;
    const FMetaClass* ClassIFP(const FName& name) const;
    const FMetaClass* ClassIFP(const FStringView& name) const;

private:
    friend Meta::TSingleton<FMetaDatabase>;

    FMetaDatabase();
    ~FMetaDatabase();

    FReadWriteLock _lockRW;

    // dont hold lifetime, must be handled separately
    HASHMAP(RTTI, FName, SMetaTransaction) _transactions;
    HASHMAP(RTTI, FName, SMetaObject) _objects;
    HASHMAP(RTTI, FName, const FMetaClass*) _classes;

    VECTORINSITU(RTTI, const FMetaNamespace*, 8) _namespaces;
};
//----------------------------------------------------------------------------
inline FMetaDatabase& MetaDB() {
    return FMetaDatabase::Instance();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
