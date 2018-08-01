#pragma once

#include "RTTI.h"

#include "Typedefs.h"

#include "Container/HashMap.h"
#include "Container/Vector.h"
#include "IO/TextWriter_fwd.h"
#include "Meta/Singleton.h"
#include "Thread/ReadWriteLock.h"

namespace PPE {
namespace RTTI {
class FMetaClass;
FWD_REFPTR(MetaObject);
class FMetaNamespace;
FWD_REFPTR(MetaTransaction);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_RTTI_API FMetaDatabase : Meta::TSingleton<FMetaDatabase> {
public:
    /* Singleton */

#ifdef WITH_PPE_ASSERT
    using Meta::TSingleton<FMetaDatabase>::HasInstance;
#endif
    using Meta::TSingleton<FMetaDatabase>::Get;

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

    FMetaObject& Object(const FPathName& pathName) const;
    FMetaObject* ObjectIFP(const FPathName& pathName) const;
    FMetaObject* ObjectIFP(const FStringView& text) const;

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

    // don't hold lifetime, must be handled separately
    HASHMAP(MetaDatabase, FName, SMetaTransaction) _transactions;
    HASHMAP(MetaDatabase, FPathName, SMetaObject) _objects;
    HASHMAP(MetaDatabase, FName, const FMetaClass*) _classes;

    VECTORINSITU(MetaDatabase, const FMetaNamespace*, 8) _namespaces;
};
//----------------------------------------------------------------------------
inline FMetaDatabase& MetaDB() {
    return FMetaDatabase::Get();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE
