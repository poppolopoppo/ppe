#pragma once

#include "MetaModule.h"
#include "RTTI_fwd.h"

#include "RTTI/TypeTraits.h"
#include "RTTI/Typedefs.h"

#include "Container/HashMap.h"
#include "Container/Vector.h"
#include "Meta/Iterator.h"
#include "Meta/Optional.h"
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

    /* Traits */

    const PTypeTraits& Traits(const FName& name) const;
    PTypeTraits TraitsIFP(const FName& name) const;
    PTypeTraits TraitsIFP(const FStringView& name) const;
    PTypeTraits TraitsIFP(const FLazyName& name) const;

    auto Traits() const NOEXCEPT;

    /* Objects */

    void RegisterObject(FMetaObject* metaObject);
    void UnregisterObject(FMetaObject* metaObject);

    FMetaObject& Object(const FPathName& pathName) const;
    FMetaObject* ObjectIFP(const FPathName& pathName) const;
    FMetaObject* ObjectIFP(const FStringView& text) const;
    FMetaObject* ObjectIFP(const FStringView& namespace_, const FStringView& identifier) const;
    FMetaObject* ObjectIFP(const FLazyName& namespace_, const FLazyName& identifier) const;
    FMetaObject* ObjectIFP(const FLazyPathName& pathName) const;

    const auto& Objects() const NOEXCEPT;

    /* Transaction */

    void RegisterTransaction(const FMetaTransaction* metaTransaction);
    void UnregisterTransaction(const FMetaTransaction* metaTransaction);

    TMemoryView<const SCMetaTransaction> Transaction(const FName& namespace_) const;
    TMemoryView<const SCMetaTransaction> TransactionIFP(const FName& namespace_) const;
    TMemoryView<const SCMetaTransaction> TransactionIFP(const FStringView& namespace_) const;
    TMemoryView<const SCMetaTransaction> TransactionIFP(const FLazyName& namespace_) const;

    const auto& Transactions() const NOEXCEPT;
    auto Namespaces() const NOEXCEPT;

    /* Modules */

    void RegisterModule(const FMetaModule* metaModule);
    void UnregisterModule(const FMetaModule* metaModule);

    const FMetaModule& Module(const FName& name) const;
    const FMetaModule* ModuleIFP(const FName& name) const;
    const FMetaModule* ModuleIFP(const FStringView& name) const;
    const FMetaModule* ModuleIFP(const FLazyName& name) const;

    auto Modules() const NOEXCEPT;
    template <typename _Facet>
    auto ModulesByFacet() const NOEXCEPT;

    /* Classes */

    const FMetaClass& Class(const FName& name) const;
    const FMetaClass* ClassIFP(const FName& name) const;
    const FMetaClass* ClassIFP(const FStringView& name) const;
    const FMetaClass* ClassIFP(const FLazyName& name) const;

    const auto& Classes() const NOEXCEPT;

    template <typename _Facet>
    auto ClassesByFacet() const {
        return _classes.Values()
            .Select([](const FMetaClass* class_) -> Meta::TOptional<TPair<const FMetaClass*, const _Facet*>> {
                if (const _Facet* facet = UserFacetIFP<_Facet>(*class_))
                    return MakePair(class_, facet);
                return Default;
            });
    }

    /* Enums */

    const FMetaEnum& Enum(const FName& name) const;
    const FMetaEnum* EnumIFP(const FName& name) const;
    const FMetaEnum* EnumIFP(const FStringView& name) const;
    const FMetaEnum* EnumIFP(const FLazyName& name) const;

    const auto& Enums() const NOEXCEPT;

    template <typename _Facet>
    auto EnumsByFacet() const {
        return _enums.Values()
            .Select([](const FMetaEnum* enum_) -> Meta::TOptional<TPair<const FMetaEnum*, const _Facet*>> {
                if (const _Facet* facet = UserFacetIFP<_Facet>(*enum_))
                    return MakePair(enum_, facet);
                return Default;
            });
    }

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
FORCE_INLINE auto FMetaDatabase::Traits() const NOEXCEPT {
    return MakeIterable(_traits);
}
//----------------------------------------------------------------------------
FORCE_INLINE const auto& FMetaDatabase::Objects() const NOEXCEPT {
    return _objects;
}
//----------------------------------------------------------------------------
FORCE_INLINE const auto& FMetaDatabase::Transactions() const NOEXCEPT {
    return _transactions;
}
//----------------------------------------------------------------------------
FORCE_INLINE auto FMetaDatabase::Namespaces() const NOEXCEPT {
    return _transactions.Keys();
}
//----------------------------------------------------------------------------
FORCE_INLINE auto FMetaDatabase::Modules() const NOEXCEPT {
    return _modules.MakeView().Iterable();
}
//----------------------------------------------------------------------------
template <typename _Facet>
auto FMetaDatabase::ModulesByFacet() const NOEXCEPT {
    return Modules()
        .Select([](const FMetaModule* module) -> Meta::TOptional<TPair<const FMetaModule*, const _Facet*>> {
        if (const _Facet* facet = UserFacetIFP<_Facet>(*module))
            return MakePair(module, facet);
        return Default;
            });
}
//----------------------------------------------------------------------------
FORCE_INLINE const auto& FMetaDatabase::Classes() const NOEXCEPT {
    return _classes;
}
//----------------------------------------------------------------------------
FORCE_INLINE const auto& FMetaDatabase::Enums() const NOEXCEPT {
    return _enums;
}
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
