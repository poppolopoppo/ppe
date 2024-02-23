#pragma once

#include "RTTI_fwd.h"

#include "RTTI/Typedefs.h"

#include "Container/HashMap.h"
#include "Container/IntrusiveList.h"
#include "Memory/MemoryDomain.h"
#include "Meta/ThreadResource.h"
#include "RTTI/UserFacet.h"

namespace PPE {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_RTTI_API FMetaClassHandle : Meta::FNonCopyableNorMovable {
public:
    typedef FMetaClass* (*create_func)(FClassId , const FMetaModule* );
    typedef void (*destroy_func)(FMetaClass*);

    FMetaClassHandle(FMetaModule& metaNamespace, create_func create, destroy_func destroy) NOEXCEPT;
    ~FMetaClassHandle() NOEXCEPT;

    const FMetaClass* Class() const { return _class; }

private:
    friend class FMetaModule;

    FMetaClass* _class;

    create_func const _create;
    destroy_func const _destroy;

    TIntrusiveSingleListNode<FMetaClassHandle> _node;
};
//----------------------------------------------------------------------------
class PPE_RTTI_API FMetaEnumHandle : Meta::FNonCopyableNorMovable {
public:
    typedef FMetaEnum* (*create_func)(const FMetaModule*);
    typedef void(*destroy_func)(FMetaEnum*);

    FMetaEnumHandle(FMetaModule& metaNamespace, create_func create, destroy_func destroy) NOEXCEPT;
    ~FMetaEnumHandle() NOEXCEPT;

    const FMetaEnum* Enum() const { return _enum; }

private:
    friend class FMetaModule;

    FMetaEnum* _enum;

    create_func const _create;
    destroy_func const _destroy;

    TIntrusiveSingleListNode<FMetaEnumHandle> _node;
};
//----------------------------------------------------------------------------
class PPE_RTTI_API FMetaModule : Meta::FThreadResource {
public:
#if USE_PPE_MEMORYDOMAINS
    FMemoryTracking& TrackingData() const { return _trackingData; }
    FMetaModule(FStringLiteral name, FMemoryTracking& domain);
#else
    explicit FMetaModule(FStringLiteral name);
#endif
    ~FMetaModule();

    FMetaModule(const FMetaModule& ) = delete;
    FMetaModule& operator =(const FMetaModule& ) = delete;

    bool IsStarted() const { return (not _nameToken.empty()); }

    const FName& Name() const {
        Assert(IsStarted());
        return _nameToken;
    }

    FMetaModuleFacet& Facets() { return _facets; }
    const FMetaModuleFacet& Facets() const { return _facets; }

    void Start();
    void Shutdown();

    /** Classes **/

    const FMetaClass& Class(const FName& name) const;
    const FMetaClass* ClassIFP(const FName& name) const;
    const FMetaClass* ClassIFP(FStringLiteral name) const;

    auto Classes() const {
        Assert(IsStarted());
        return _classes.Values();
    }

    void RegisterClass(FMetaClassHandle& handle);

    /** Enums **/

    const FMetaEnum& Enum(const FName& name) const;
    const FMetaEnum* EnumIFP(const FName& name) const;
    const FMetaEnum* EnumIFP(FStringLiteral name) const;

    auto Enums() const {
        Assert(IsStarted());
        return _enums.Values();
    }

    void RegisterEnum(FMetaEnumHandle& handle);

private:
    HASHMAP(MetaModule, FName, const FMetaClass*) _classes;
    HASHMAP(MetaModule, FName, const FMetaEnum*) _enums;

    FName _nameToken;
    size_t _classIdOffset;
    size_t _classCount;
    size_t _enumCount;

    const FStringLiteral _nameCStr;

    INTRUSIVESINGLELIST(&FMetaClassHandle::_node) _classHandles;
    INTRUSIVESINGLELIST(&FMetaEnumHandle::_node) _enumHandles;

    FMetaModuleFacet _facets;

#if USE_PPE_MEMORYDOMAINS
    mutable FMemoryTracking _trackingData;
#endif
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE
