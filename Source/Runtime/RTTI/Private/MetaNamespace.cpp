#include "stdafx.h"

#include "MetaNamespace.h"

#include "MetaClass.h"
#include "MetaDatabase.h"
#include "MetaEnum.h"

#include "Diagnostic/Logger.h"
#include "Thread/ThreadContext.h"

namespace PPE {
namespace RTTI {
EXTERN_LOG_CATEGORY(PPE_RTTI_API, RTTI)
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
FMetaEnumHandle::FMetaEnumHandle(FMetaNamespace& metaNamespace, create_func create, destroy_func destroy)
    : _enum(nullptr)
    , _create(create)
    , _destroy(destroy) {
    Assert(_create);
    Assert(_destroy);

    metaNamespace.RegisterEnum(*this);
}
//----------------------------------------------------------------------------
FMetaEnumHandle::~FMetaEnumHandle() {
    Assert(nullptr == _enum);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if USE_PPE_MEMORYDOMAINS
FMetaNamespace::FMetaNamespace(const FStringView& name, FMemoryTracking& domain)
#else
FMetaNamespace::FMetaNamespace(const FStringView& name)
#endif
    : _classIdOffset(INDEX_NONE)
    , _classCount(0)
    , _enumCount(0)
    , _nameCStr(name)
#if USE_PPE_MEMORYDOMAINS
    , _trackingData(_nameCStr.data(), &domain) {
    RegisterTrackingData(&_trackingData);
#else
    {
#endif
    Assert(not _nameCStr.empty());
}
//----------------------------------------------------------------------------
FMetaNamespace::~FMetaNamespace() {
    Assert(not IsStarted());
    _classHandles.Clear();
    _enumHandles.Clear();
#if USE_PPE_MEMORYDOMAINS
    UnregisterTrackingData(&_trackingData);
#endif
}
//----------------------------------------------------------------------------
void FMetaNamespace::Start() {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(not IsStarted());
    Assert(_classes.empty());

    LOG(RTTI, Debug, L"start namespace <{0}> ({1} handles)", _nameCStr, _classCount);

    _nameToken = FName(_nameCStr);

    if (INDEX_NONE == _classIdOffset) {
        static size_t GClassIdFirstFree = 0;
        _classIdOffset = GClassIdFirstFree;
        GClassIdFirstFree += _classCount;
    }
    Assert(INDEX_NONE != _classIdOffset);

    /* Create meta enums */

    for (FMetaEnumHandle* phandle = _enumHandles.Head(); phandle; phandle = phandle->_node.Next) {
        Assert(nullptr == phandle->_enum);

        phandle->_enum = phandle->_create(this);
        Assert(phandle->_enum);

        const FMetaEnum* metaEnum = phandle->_enum;
        const FName& metaEnumName = metaEnum->Name();
        Insert_AssertUnique(_enums, metaEnumName, metaEnum);

        LOG(RTTI, Info, L"create meta enum <{0}::{1}>", _nameCStr, metaEnumName);
    }

    /* Create meta classes */

    size_t classIndex = 0;
    for (FMetaClassHandle* phandle = _classHandles.Head(); phandle; phandle = phandle->_node.Next, classIndex++) {
        Assert(nullptr == phandle->_class);

        const FClassId classId = FClassId::Prime(_classIdOffset + classIndex);

        phandle->_class = phandle->_create(classId, this);
        Assert(phandle->_class);

        const FMetaClass* metaClass = phandle->_class;
        const FName& metaClassName = metaClass->Name();
        Insert_AssertUnique(_classes, metaClassName, metaClass);

        LOG(RTTI, Info, L"create meta class <{0}::{1}> = #{2}", _nameCStr, metaClassName, classId);
    }
    Assert_NoAssume(classIndex == _classCount);

    /* Call OnRegister() on every classes, every parent are guaranted to be already constructed */

    for (FMetaClassHandle* phandle = _classHandles.Head(); phandle; phandle = phandle->_node.Next) {
        Assert(phandle->_class);
        Assert(phandle->_class->Namespace() == this);

        phandle->_class->CallOnRegister_IFN();

        LOG(RTTI, Info, L"register meta class <{0}::{1}> = #{2}", _nameCStr, phandle->_class->Name(), phandle->_class->Id());
    }

    {
        const FMetaDatabaseReadWritable metaDB;
        metaDB->RegisterNamespace(this);
    }

    Assert(IsStarted());
}
//----------------------------------------------------------------------------
void FMetaNamespace::Shutdown() {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(IsStarted());

    {
        const FMetaDatabaseReadWritable metaDB;
        metaDB->UnregisterNamespace(this);
    }

    LOG(RTTI, Debug, L"shutdown namespace <{0}> ({1} handles)", _nameCStr, _classCount);

    _nameToken = FName();

    /* Call OnRegister() on every classes, every parent are guaranted to be still alive */

    for (FMetaClassHandle* phandle = _classHandles.Head(); phandle; phandle = phandle->_node.Next) {
        Assert(phandle->_class);
        Assert(phandle->_class->Namespace() == this);

        LOG(RTTI, Info, L"unregister meta class <{0}::{1}> = #{2}", _nameCStr, phandle->_class->Name(), phandle->_class->Id());

        phandle->_class->OnUnregister();
    }

    /* Destroy meta classes */

    _classes.clear_ReleaseMemory();

    for (FMetaClassHandle* phandle = _classHandles.Head(); phandle; phandle = phandle->_node.Next) {
        Assert(phandle->_class);
        Assert(phandle->_class->Namespace() == this);

        LOG(RTTI, Info, L"destroy meta class <{0}::{1}> = #{2}", _nameCStr, phandle->_class->Name(), phandle->_class->Id());

        phandle->_destroy(phandle->_class);
        phandle->_class = nullptr;
    }

    /* Destroy meta enums */

    _enums.clear_ReleaseMemory();

    for (FMetaEnumHandle* phandle = _enumHandles.Head(); phandle; phandle = phandle->_node.Next) {
        Assert(phandle->_enum);
        Assert(phandle->_enum->Namespace() == this);

        LOG(RTTI, Info, L"destroy meta class <{0}::{1}>", _nameCStr, phandle->_enum->Name());

        phandle->_destroy(phandle->_enum);
        phandle->_enum = nullptr;
    }

    Assert(not IsStarted());
}
//----------------------------------------------------------------------------
// Classes
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
void FMetaNamespace::RegisterClass(FMetaClassHandle& handle) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(not IsStarted());
    Assert(not _classHandles.Contains(&handle));

    _classCount++;
    _classHandles.PushFront(&handle);
}
//----------------------------------------------------------------------------
// Enums
//----------------------------------------------------------------------------
const FMetaEnum& FMetaNamespace::Enum(const FName& name) const {
    Assert(IsStarted());

    const auto it = _enums.find(name);
    AssertRelease(_enums.end() != it);

    return (*it->second);
}
//----------------------------------------------------------------------------
const FMetaEnum* FMetaNamespace::EnumIFP(const FName& name) const {
    Assert(IsStarted());

    const auto it = _enums.find(name);
    return (_enums.end() == it ? nullptr : it->second);
}
//----------------------------------------------------------------------------
const FMetaEnum* FMetaNamespace::EnumIFP(const FStringView& name) const {
    Assert(IsStarted());

    const hash_t h = FName::HashValue(name);
    const auto it = _enums.find_like(name, h);

    return (_enums.end() == it ? nullptr : it->second);
}
//----------------------------------------------------------------------------
void FMetaNamespace::RegisterEnum(FMetaEnumHandle& handle) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(not IsStarted());
    Assert(not _enumHandles.Contains(&handle));

    _enumCount++;
    _enumHandles.PushFront(&handle);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE
