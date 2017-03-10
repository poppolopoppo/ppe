#pragma once

#include "Core.RTTI/RTTI.h"

#include "Core.RTTI/Typedefs.h"

#include "Core/Container/HashMap.h"
#include "Core/Container/IntrusiveList.h"
#include "Core/Thread/AtomicSpinLock.h"

namespace Core {
namespace RTTI {
class FMetaClass;
class FMetaClassGuid;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FMetaClassHandle {
public:
    friend class FMetaNamespace;

    typedef void (*create_delegate)(const FMetaClass** , FMetaClassGuid, const FMetaNamespace* );

    FMetaClassHandle(const FMetaNamespace& metaNamespace, create_delegate create) NOEXCEPT;
    ~FMetaClassHandle() NOEXCEPT;

    FMetaClassHandle(const FMetaClassHandle&) = delete;
    FMetaClassHandle& operator =(const FMetaClassHandle&) = delete;

    FMetaClassHandle(FMetaClassHandle&&) = delete;
    FMetaClassHandle& operator =(FMetaClassHandle&&) = delete;

    const FMetaClass* MetaClass() const { return _metaClass; }

private:
    const create_delegate _create;
    const FMetaClass* _metaClass;
    TIntrusiveListNode<FMetaClassHandle> _node;
};
//----------------------------------------------------------------------------
class FMetaNamespace {
public:
    friend class FMetaClassHandle;

    explicit FMetaNamespace(const FStringView& name) NOEXCEPT;
    ~FMetaNamespace() NOEXCEPT;

    FMetaNamespace(const FMetaNamespace&) = delete;
    FMetaNamespace& operator =(const FMetaNamespace&) = delete;

    const FName& Name() const {
        Assert(not _nameTokenized.empty());
        return _nameTokenized;
    }

    void Start();
    void Shutdown();

    const FMetaClass* FindClass(const FName& metaClassName) const;
    const FMetaClass* FindClassIFP(const FName& metaClassName) const;

    void AllClasses(VECTOR(RTTI, const FMetaClass*)& instances) const;

private:
    void Append_(FMetaClassHandle* pHandle);

    FStringView _nameCStr;
    FName _nameTokenized; // only available after Start() and before Shutdown()

    size_t _guidOffset;

    HASHMAP(RTTI, FName, const FMetaClass*) _metaClasses;
    INTRUSIVELIST(&FMetaClassHandle::_node) _handles;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
