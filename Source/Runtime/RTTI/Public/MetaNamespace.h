#pragma once

#include "RTTI_fwd.h"

#include "RTTI/Typedefs.h"

#include "Container/HashMap.h"
#include "Container/IntrusiveList.h"
#include "Memory/MemoryDomain.h"
#include "Meta/ThreadResource.h"

namespace PPE {
namespace RTTI {
class FMetaClass;
class FMetaEnum;
class FMetaNamespace;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_RTTI_API FMetaClassHandle : Meta::FNonCopyableNorMovable {
public:
    typedef FMetaClass* (*create_func)(FClassId , const FMetaNamespace* );
    typedef void (*destroy_func)(FMetaClass*);

    FMetaClassHandle(FMetaNamespace& metaNamespace, create_func create, destroy_func destroy) NOEXCEPT;
    ~FMetaClassHandle() NOEXCEPT;

    const FMetaClass* Class() const { return _class; }

private:
    friend class FMetaNamespace;

    FMetaClass* _class;

    create_func const _create;
    destroy_func const _destroy;

    TIntrusiveSingleListNode<FMetaClassHandle> _node;
};
//----------------------------------------------------------------------------
class PPE_RTTI_API FMetaEnumHandle : Meta::FNonCopyableNorMovable {
public:
    typedef FMetaEnum* (*create_func)(const FMetaNamespace*);
    typedef void(*destroy_func)(FMetaEnum*);

    FMetaEnumHandle(FMetaNamespace& metaNamespace, create_func create, destroy_func destroy) NOEXCEPT;
    ~FMetaEnumHandle() NOEXCEPT;

    const FMetaEnum* Enum() const { return _enum; }

private:
    friend class FMetaNamespace;

    FMetaEnum* _enum;

    create_func const _create;
    destroy_func const _destroy;

    TIntrusiveSingleListNode<FMetaEnumHandle> _node;
};
//----------------------------------------------------------------------------
class PPE_RTTI_API FMetaNamespace : Meta::FThreadResource {
public:
#if USE_PPE_MEMORYDOMAINS
    FMemoryTracking& TrackingData() const { return _trackingData; }
    FMetaNamespace(const FStringView& name, FMemoryTracking& domain);
#else
    explicit FMetaNamespace(const FStringView& name);
#endif
    ~FMetaNamespace();

    FMetaNamespace(const FMetaNamespace& ) = delete;
    FMetaNamespace& operator =(const FMetaNamespace& ) = delete;

    bool IsStarted() const { return (not _nameToken.empty()); }

    const FName& Name() const {
        Assert(IsStarted());
        return _nameToken;
    }

    void Start();
    void Shutdown();

    /** Classes **/

    const FMetaClass& Class(const FName& name) const;
    const FMetaClass* ClassIFP(const FName& name) const;
    const FMetaClass* ClassIFP(const FStringView& name) const;

    auto Classes() const {
        Assert(IsStarted());
        return _classes.Values();
    }

    void RegisterClass(FMetaClassHandle& handle);

    /** Enums **/

    const FMetaEnum& Enum(const FName& name) const;
    const FMetaEnum* EnumIFP(const FName& name) const;
    const FMetaEnum* EnumIFP(const FStringView& name) const;

    auto Enums() const {
        Assert(IsStarted());
        return _enums.Values();
    }

    void RegisterEnum(FMetaEnumHandle& handle);

private:
    HASHMAP(MetaNamespace, FName, const FMetaClass*) _classes;
    HASHMAP(MetaNamespace, FName, const FMetaEnum*) _enums;

    FName _nameToken;
    size_t _classIdOffset;
    size_t _classCount;
    size_t _enumCount;

    const FStringView _nameCStr;

    INTRUSIVESINGLELIST(&FMetaClassHandle::_node) _classHandles;
    INTRUSIVESINGLELIST(&FMetaEnumHandle::_node) _enumHandles;

#if USE_PPE_MEMORYDOMAINS
    mutable FMemoryTracking _trackingData;
#endif
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE
