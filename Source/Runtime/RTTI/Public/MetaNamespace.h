#pragma once

#include "RTTI.h"

#include "Typedefs.h"

#include "Container/HashMap.h"
#include "Container/IntrusiveList.h"
#include "Memory/MemoryDomain.h"
#include "Meta/ThreadResource.h"

namespace PPE {
namespace RTTI {
class FMetaClass;
class FMetaNamespace;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_RTTI_API FMetaClassHandle {
public:
    typedef FMetaClass* (*create_func)(FClassId , const FMetaNamespace* );
    typedef void (*destroy_func)(FMetaClass*);

    FMetaClassHandle(class FMetaNamespace& metaNamespace, create_func create, destroy_func destroy);
    ~FMetaClassHandle();

    FMetaClassHandle(const FMetaClassHandle& ) = delete;
    FMetaClassHandle& operator =(const FMetaClassHandle& ) = delete;

    const FMetaClass* Class() const { return _class; }

private:
    friend class FMetaNamespace;

    FMetaClass* _class;

    create_func const _create;
    destroy_func const _destroy;

    TIntrusiveSingleListNode<FMetaClassHandle> _node;
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

    void RegisterClass(FMetaClassHandle& handle);

    void Start();
    void Shutdown();

    const FMetaClass& Class(const FName& name) const;
    const FMetaClass* ClassIFP(const FName& name) const;
    const FMetaClass* ClassIFP(const FStringView& name) const;

    auto Classes() const {
        Assert(IsStarted());
        return _classes.Values();
    }

private:
    HASHMAP(MetaNamespace, FName, const FMetaClass*) _classes;

    FName _nameToken;
    size_t _classIdOffset;
    size_t _classCount;

    const FStringView _nameCStr;

    INTRUSIVESINGLELIST(&FMetaClassHandle::_node) _handles;

#if USE_PPE_MEMORYDOMAINS
    mutable FMemoryTracking _trackingData;
#endif
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE
