// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "MetaModule.h"

#include "MetaClass.h"
#include "MetaDatabase.h"
#include "MetaEnum.h"

#include "Diagnostic/Logger.h"
#include "IO/FormatHelpers.h"
#include "Thread/ThreadContext.h"

namespace PPE {
namespace RTTI {
EXTERN_LOG_CATEGORY(PPE_RTTI_API, RTTI)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FMetaClassHandle::FMetaClassHandle(FMetaModule& metaNamespace, create_func create, destroy_func destroy) NOEXCEPT
:   _class(nullptr)
,   _create(create)
,   _destroy(destroy) {
    Assert(_create);
    Assert(_destroy);

    metaNamespace.RegisterClass(*this);
}
//----------------------------------------------------------------------------
FMetaClassHandle::~FMetaClassHandle() NOEXCEPT {
    Assert(nullptr == _class);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FMetaEnumHandle::FMetaEnumHandle(FMetaModule& metaNamespace, create_func create, destroy_func destroy) NOEXCEPT
:   _enum(nullptr)
,   _create(create)
,   _destroy(destroy) {
    Assert(_create);
    Assert(_destroy);

    metaNamespace.RegisterEnum(*this);
}
//----------------------------------------------------------------------------
FMetaEnumHandle::~FMetaEnumHandle() NOEXCEPT {
    Assert(nullptr == _enum);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if USE_PPE_MEMORYDOMAINS
FMetaModule::FMetaModule(const FStringView& name, FMemoryTracking& domain)
#else
FMetaModule::FMetaModule(const FStringView& name)
#endif
:   _classIdOffset(INDEX_NONE)
,   _classCount(0)
,   _enumCount(0)
,   _nameCStr(name)
#if USE_PPE_MEMORYDOMAINS
    , _trackingData(_nameCStr.data(), &domain) {
    RegisterTrackingData(&_trackingData);
#else
    {
#endif
    Assert(not _nameCStr.empty());
}
//----------------------------------------------------------------------------
FMetaModule::~FMetaModule() {
    Assert(not IsStarted());
    _classHandles.Clear();
    _enumHandles.Clear();
#if USE_PPE_MEMORYDOMAINS
    UnregisterTrackingData(&_trackingData);
#endif
}
//----------------------------------------------------------------------------
void FMetaModule::Start() {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(not IsStarted());
    Assert(_classes.empty());

    PPE_LOG(RTTI, Debug, "start namespace <{0}> ({1} handles)", _nameCStr, _classCount);

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

        PPE_LOG(RTTI, Info, "create meta enum <{0}::{1}> ({2})", _nameCStr, metaEnumName, Fmt::Pointer(phandle));

        Insert_AssertUnique(_enums, metaEnumName, metaEnum);
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

        PPE_LOG(RTTI, Info, "create meta class <{0}::{1}> = #{2} ({3})", _nameCStr, metaClassName, classId, Fmt::Pointer(phandle));

        Insert_AssertUnique(_classes, metaClassName, metaClass);
    }
    Assert_NoAssume(classIndex == _classCount);

    /* Call OnRegister() on every classes, every parent are guaranteed to be already constructed */

    for (FMetaClassHandle* phandle = _classHandles.Head(); phandle; phandle = phandle->_node.Next) {
        Assert(phandle->_class);
        Assert(phandle->_class->Module() == this);

        PPE_LOG(RTTI, Info, "register meta class <{0}::{1}> = #{2}", _nameCStr, phandle->_class->Name(), phandle->_class->Id());

        phandle->_class->CallOnRegister_IFN();
    }

    {
        const FMetaDatabaseReadWritable metaDB;
        metaDB->RegisterModule(this);
    }

    Assert(IsStarted());
}
//----------------------------------------------------------------------------
void FMetaModule::Shutdown() {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(IsStarted());

    {
        const FMetaDatabaseReadWritable metaDB;
        metaDB->UnregisterModule(this);
    }

    PPE_LOG(RTTI, Debug, "shutdown namespace <{0}> ({1} handles)", _nameCStr, _classCount);

    _nameToken = FName();

    /* Call OnRegister() on every classes, every parent are guaranteed to be still alive */

    for (FMetaClassHandle* phandle = _classHandles.Head(); phandle; phandle = phandle->_node.Next) {
        Assert(phandle->_class);
        Assert(phandle->_class->Module() == this);

        PPE_LOG(RTTI, Info, "unregister meta class <{0}::{1}> = #{2}", _nameCStr, phandle->_class->Name(), phandle->_class->Id());

        phandle->_class->OnUnregister();
    }

    /* Destroy meta classes */

    _classes.clear_ReleaseMemory();

    for (FMetaClassHandle* phandle = _classHandles.Head(); phandle; phandle = phandle->_node.Next) {
        Assert(phandle->_class);
        Assert(phandle->_class->Module() == this);

        PPE_LOG(RTTI, Info, "destroy meta class <{0}::{1}> = #{2}", _nameCStr, phandle->_class->Name(), phandle->_class->Id());

        phandle->_destroy(phandle->_class);
        phandle->_class = nullptr;
    }

    /* Destroy meta enums */

    _enums.clear_ReleaseMemory();

    for (FMetaEnumHandle* phandle = _enumHandles.Head(); phandle; phandle = phandle->_node.Next) {
        Assert(phandle->_enum);
        Assert(phandle->_enum->Module() == this);

        PPE_LOG(RTTI, Info, "destroy meta class <{0}::{1}>", _nameCStr, phandle->_enum->Name());

        phandle->_destroy(phandle->_enum);
        phandle->_enum = nullptr;
    }

    Assert(not IsStarted());
}
//----------------------------------------------------------------------------
// Classes
//----------------------------------------------------------------------------
const FMetaClass& FMetaModule::Class(const FName& name) const {
    Assert(IsStarted());

    const auto it = _classes.find(name);
    AssertRelease(_classes.end() != it);

    return (*it->second);
}
//----------------------------------------------------------------------------
const FMetaClass* FMetaModule::ClassIFP(const FName& name) const {
    Assert(IsStarted());

    const auto it = _classes.find(name);
    return (_classes.end() == it ? nullptr : it->second);
}
//----------------------------------------------------------------------------
const FMetaClass* FMetaModule::ClassIFP(const FStringView& name) const {
    Assert(IsStarted());

    const hash_t h = FName::HashValue(name);
    const auto it = _classes.find_like(name, h);

    return (_classes.end() == it ? nullptr : it->second);
}
//----------------------------------------------------------------------------
void FMetaModule::RegisterClass(FMetaClassHandle& handle) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(not IsStarted());
    Assert(not _classHandles.Contains(&handle));

    _classCount++;
    _classHandles.PushHead(&handle);
}
//----------------------------------------------------------------------------
// Enums
//----------------------------------------------------------------------------
const FMetaEnum& FMetaModule::Enum(const FName& name) const {
    Assert(IsStarted());

    const auto it = _enums.find(name);
    AssertRelease(_enums.end() != it);

    return (*it->second);
}
//----------------------------------------------------------------------------
const FMetaEnum* FMetaModule::EnumIFP(const FName& name) const {
    Assert(IsStarted());

    const auto it = _enums.find(name);
    return (_enums.end() == it ? nullptr : it->second);
}
//----------------------------------------------------------------------------
const FMetaEnum* FMetaModule::EnumIFP(const FStringView& name) const {
    Assert(IsStarted());

    const hash_t h = FName::HashValue(name);
    const auto it = _enums.find_like(name, h);

    return (_enums.end() == it ? nullptr : it->second);
}
//----------------------------------------------------------------------------
void FMetaModule::RegisterEnum(FMetaEnumHandle& handle) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(not IsStarted());
    Assert(not _enumHandles.Contains(&handle));

    _enumCount++;
    _enumHandles.PushHead(&handle);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE
