#include "stdafx.h"

#include "MetaNamespace.h"

#include "MetaClass.h"
#include "MetaDatabase.h"

#include "Core/Diagnostic/Logger.h"
#include "Core/Thread/ThreadContext.h"

namespace Core {
namespace RTTI {
EXTERN_LOG_CATEGORY(CORE_RTTI_API, RTTI);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FMetaClassHandle::FMetaClassHandle(FMetaNamespace& metaNamespace, create_func create, destroy_func destroy)
    : _class(nullptr)
    , _create(create)
    , _destroy(destroy) {
    Assert(_create);
    Assert(_destroy);

    metaNamespace.RegisterClass(*this);
}
//----------------------------------------------------------------------------
FMetaClassHandle::~FMetaClassHandle() {
    Assert(nullptr == _class);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FMetaNamespace::FMetaNamespace(const FStringView& name)
    : _classIdOffset(INDEX_NONE)
    , _classCount(0)
    , _nameCStr(name) {
    Assert(not _nameCStr.empty());
}
//----------------------------------------------------------------------------
void FMetaNamespace::RegisterClass(FMetaClassHandle& handle) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(not IsStarted());
    Assert(not _handles.Contains(&handle));

    _classCount++;
    _handles.PushFront(&handle);
}
//----------------------------------------------------------------------------
void FMetaNamespace::Start() {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(not IsStarted());
    Assert(_classes.empty());

    LOG(RTTI, Info, L"start namespace <{0}> ({1} handles)", _nameCStr, _classCount);

    _nameToken = FName(_nameCStr);

    if (INDEX_NONE == _classIdOffset) {
        static size_t GClassIdFirstFree = 0;
        _classIdOffset = GClassIdFirstFree;
        GClassIdFirstFree += _classCount;
    }
    Assert(INDEX_NONE != _classIdOffset);

    /* Create meta classes */

    size_t index = 0;
    for (FMetaClassHandle* phandle = _handles.Head(); phandle; phandle = phandle->_node.Next, index++) {
        Assert(nullptr == phandle->_class);

        const FClassId classId = FClassId::Prime(_classIdOffset + index);

        phandle->_class = phandle->_create(classId, this);
        Assert(phandle->_class);

        const FMetaClass* metaClass = phandle->_class;
        const FName& metaClassName = metaClass->Name();
        Insert_AssertUnique(_classes, metaClassName, metaClass);

        LOG(RTTI, Info, L"create meta class <{0}::{1}> = #{2}", _nameCStr, metaClassName, classId);
    }
    Assert(index == _classCount);

    /* Call OnRegister() on every classes, every parent are guaranted to be already constructed */

    for (FMetaClassHandle* phandle = _handles.Head(); phandle; phandle = phandle->_node.Next) {
        Assert(phandle->_class);
        Assert(phandle->_class->Namespace() == this);

        phandle->_class->CallOnRegister_IFN();

        LOG(RTTI, Info, L"register meta class <{0}::{1}> = #{2}", _nameCStr, phandle->_class->Name(), phandle->_class->Id());
    }

    MetaDB().RegisterNamespace(this);

    Assert(IsStarted());
}
//----------------------------------------------------------------------------
void FMetaNamespace::Shutdown() {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(IsStarted());

    MetaDB().UnregisterNamespace(this);

    LOG(RTTI, Info, L"shutdown namespace <{0}> ({1} handles)", _nameCStr, _classCount);

    _nameToken = FName();

    /* Call OnRegister() on every classes, every parent are guaranted to be still alive */

    for (FMetaClassHandle* phandle = _handles.Head(); phandle; phandle = phandle->_node.Next) {
        Assert(phandle->_class);
        Assert(phandle->_class->Namespace() == this);

        LOG(RTTI, Info, L"unregister meta class <{0}::{1}> = #{2}", _nameCStr, phandle->_class->Name(), phandle->_class->Id());

        phandle->_class->OnUnregister();
    }

    /* Destroy meta classes */

    _classes.clear_ReleaseMemory();

    for (FMetaClassHandle* phandle = _handles.Head(); phandle; phandle = phandle->_node.Next) {
        Assert(phandle->_class);
        Assert(phandle->_class->Namespace() == this);

        LOG(RTTI, Info, L"destroy meta class <{0}::{1}> = #{2}", _nameCStr, phandle->_class->Name(), phandle->_class->Id());

        phandle->_destroy(phandle->_class);
        phandle->_class = nullptr;
    }

    Assert(not IsStarted());
}
//----------------------------------------------------------------------------
const FMetaClass& FMetaNamespace::Class(const FName& name) const {
    Assert(IsStarted());

    const auto it = _classes.find(name);
    AssertRelease(_classes.end() != it);

    return (*it->second);
}
//----------------------------------------------------------------------------
const FMetaClass* FMetaNamespace::ClassIFP(const FName& name) const {
    Assert(IsStarted());

    const auto it = _classes.find(name);
    return (_classes.end() == it ? nullptr : it->second);
}
//----------------------------------------------------------------------------
const FMetaClass* FMetaNamespace::ClassIFP(const FStringView& name) const {
    Assert(IsStarted());

    const hash_t h = FName::HashValue(name);
    const auto it = _classes.find_like(name, h);

    return (_classes.end() == it ? nullptr : it->second);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
