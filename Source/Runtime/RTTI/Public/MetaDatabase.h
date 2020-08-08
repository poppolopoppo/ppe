#pragma once

#include "RTTI_fwd.h"

#include "RTTI/Typedefs.h"

#include "Container/HashMap.h"
#include "Container/Vector.h"
#include "IO/TextWriter_fwd.h"
#include "Meta/Singleton.h"
#include "Thread/ReadWriteLock.h"

namespace PPE {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FMetaDatabaseReadable;
class FMetaDatabaseReadWritable;
class PPE_RTTI_API FMetaDatabase : Meta::TSingleton<FMetaDatabase> {
    friend class Meta::TSingleton<FMetaDatabase>;
    using singleton_type = Meta::TSingleton<FMetaDatabase>;
    static DLL_NOINLINE void* class_singleton_storage() NOEXCEPT;
public:
    /* Singleton */

#if USE_PPE_ASSERT
    using singleton_type::HasInstance;
#endif

    static void Create() { singleton_type::Create(); }
    using singleton_type::Destroy;

    /* Objects */

    void RegisterObject(FMetaObject* metaObject);
    void UnregisterObject(FMetaObject* metaObject);

    FMetaObject& Object(const FPathName& pathName) const;
    FMetaObject* ObjectIFP(const FPathName& pathName) const;
    FMetaObject* ObjectIFP(const FStringView& text) const;
    FMetaObject* ObjectIFP(const FStringView& namespace_, const FStringView& identifier) const;
    FMetaObject* ObjectIFP(const FLazyName& namespace_, const FLazyName& identifier) const;
    FMetaObject* ObjectIFP(const FLazyPathName& pathName) const;
    const auto& Objects() const { return _objects; }

    /* Transaction */

    void RegisterTransaction(const FMetaTransaction* metaTransaction);
    void UnregisterTransaction(const FMetaTransaction* metaTransaction);

    TMemoryView<const SCMetaTransaction> Transaction(const FName& namespace_) const;
    TMemoryView<const SCMetaTransaction> TransactionIFP(const FName& namespace_) const;
    TMemoryView<const SCMetaTransaction> TransactionIFP(const FStringView& namespace_) const;
    TMemoryView<const SCMetaTransaction> TransactionIFP(const FLazyName& namespace_) const;
    const auto& Transactions() const { return _transactions; }

    /* Modules */

    void RegisterModule(const FMetaModule* metaModule);
    void UnregisterModule(const FMetaModule* metaModule);

    const FMetaModule& Module(const FName& name) const;
    const FMetaModule* ModuleIFP(const FName& name) const;
    const FMetaModule* ModuleIFP(const FStringView& name) const;
    const FMetaModule* ModuleIFP(const FLazyName& name) const;
    const auto& Modules() const { return _modules; }

    /* Classes */

    const FMetaClass& Class(const FName& name) const;
    const FMetaClass* ClassIFP(const FName& name) const;
    const FMetaClass* ClassIFP(const FStringView& name) const;
    const FMetaClass* ClassIFP(const FLazyName& name) const;
    const auto& Classes() const { return _classes; }

    /* Enums */

    const FMetaEnum& Enum(const FName& name) const;
    const FMetaEnum* EnumIFP(const FName& name) const;
    const FMetaEnum* EnumIFP(const FStringView& name) const;
    const FMetaEnum* EnumIFP(const FLazyName& name) const;
    const auto& Enums() const { return _enums; }

    /* Traits */

    const PTypeTraits& Traits(const FName& name) const;
    PTypeTraits TraitsIFP(const FName& name) const;
    PTypeTraits TraitsIFP(const FStringView& name) const;
    PTypeTraits TraitsIFP(const FLazyName& name) const;
    const auto& Traits() const { return _traits; }

private:
    friend class FMetaDatabaseReadable;
    friend class FMetaDatabaseReadWritable;

    using transactions_t = VECTOR(MetaDatabase, SCMetaTransaction);

    void InitializeNativeTypes_();
    void RegisterTraits_(const FName& name, const PTypeTraits& traits);
    void UnregisterTraits_(const FName& name, const PTypeTraits& traits);

    // must use FMetaDatabaseReadable or FMetaDatabaseReadWritable helpers (batch your work)
    //using singleton_type::Get;

    FMetaDatabase();
    ~FMetaDatabase();

    FReadWriteLock _lockRW;

    // don't hold lifetime, must be handled separately
    HASHMAP(MetaDatabase, FName, transactions_t) _transactions;
    HASHMAP(MetaDatabase, FPathName, SMetaObject) _objects;
    HASHMAP(MetaDatabase, FName, const FMetaClass*) _classes;
    HASHMAP(MetaDatabase, FName, const FMetaEnum*) _enums;
    HASHMAP(MetaDatabase, FName, PTypeTraits) _traits;

    VECTOR(MetaDatabase, const FMetaModule*) _modules;

};
//----------------------------------------------------------------------------
class FMetaDatabaseReadable : Meta::FNonCopyableNorMovable {
public:
    PPE_RTTI_API FMetaDatabaseReadable();
    PPE_RTTI_API ~FMetaDatabaseReadable();

    const FMetaDatabase& operator *() const { return _db; }
    const FMetaDatabase* operator ->() const { return &_db; }

private:
    const FMetaDatabase& _db;
};
//----------------------------------------------------------------------------
class FMetaDatabaseReadWritable : Meta::FNonCopyableNorMovable {
public:
    PPE_RTTI_API FMetaDatabaseReadWritable();
    PPE_RTTI_API ~FMetaDatabaseReadWritable();

    FMetaDatabase& operator *() const { return _db; }
    FMetaDatabase* operator ->() const { return &_db; }

private:
    FMetaDatabase& _db;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE
