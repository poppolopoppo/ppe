#pragma once

#include "Core.RTTI/RTTI.h"

#include "Core.RTTI/Typedefs.h"

#include "Core/Container/HashMap.h"
#include "Core/Container/IntrusiveList.h"
#include "Core/Meta/ThreadResource.h"

namespace Core {
namespace RTTI {
class FMetaClass;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class CORE_RTTI_API FMetaClassHandle {
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
class CORE_RTTI_API FMetaNamespace : Meta::FThreadResource {
public:
    explicit FMetaNamespace(const FStringView& name);

#ifdef WITH_CORE_ASSERT
    ~FMetaNamespace() {
        Assert(not IsStarted());
        _handles.Clear();
    }
#endif

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
    HASHMAP(RTTI, FName, const FMetaClass*) _classes;

    FName _nameToken;
    size_t _classIdOffset;
    size_t _classCount;

    const FStringView _nameCStr;

    INTRUSIVESINGLELIST(&FMetaClassHandle::_node) _handles;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
