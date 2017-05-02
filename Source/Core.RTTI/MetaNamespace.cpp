#include "stdafx.h"

#include "MetaNamespace.h"

#include "MetaClass.h"
#include "MetaDatabase.h"

#include "Core/Container/HashSet.h"
#include "Core/Container/Stack.h"
#include "Core/Diagnostic/Logger.h"
#include "Core/Maths/PrimeNumbers.h"

#include <atomic>

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FMetaClassHandle::FMetaClassHandle(const FMetaNamespace& metaNamespace, create_delegate create) NOEXCEPT
    : _create(create)
    , _metaClass(nullptr) {
    Assert(create);
    const_cast<FMetaNamespace&>(metaNamespace).Append_(this); // register in namespace
}
//----------------------------------------------------------------------------
FMetaClassHandle::~FMetaClassHandle() NOEXCEPT {
    Assert(nullptr == _metaClass);
    // don't unregister to diminish class size, and it's useless anyway
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
// Global lock instead of member (less space used in static vars)
FAtomicSpinLock GMetaNamespaceSpinLock;
// Take a new prime number range for each namespace
static size_t MetaNamespaceGuidOffset_(size_t count) {
    static std::atomic<size_t>  GMetaNamespaceGuidOffset(0);
    const size_t offset = GMetaNamespaceGuidOffset.fetch_add(count);
    Assert(offset + count <= lengthof(GPrimeNumbersU16));
    return offset;
}
} //!namespace
//----------------------------------------------------------------------------
FMetaNamespace::FMetaNamespace(const FStringView& name) NOEXCEPT
    : _nameCStr(name)
    , _guidOffset(size_t(-1)) {
    Assert(not _nameCStr.empty());
}
//----------------------------------------------------------------------------
FMetaNamespace::~FMetaNamespace() NOEXCEPT {
    Assert(_nameTokenized.empty());
    Assert(_metaClasses.empty()); // Shutdown wasn't called !
    ONLY_IF_ASSERT(_handles.Clear()); // Skip list non empty assert
}
//----------------------------------------------------------------------------
void FMetaNamespace::Start() {
    AssertIsMainThread();
    Assert(_metaClasses.empty());
    Assert(_nameTokenized.empty());

    typedef INTRUSIVELIST_ACCESSOR(&FMetaClassHandle::_node) accessor_type;

    const FAtomicSpinLock::FScope scopeLock(GMetaNamespaceSpinLock);

    _nameTokenized = FName(_nameCStr);

    LOG(Info, L"[RTTI] Start namespace <{0}>", _nameTokenized);

    size_t metaClassCount = 0;
    for (FMetaClassHandle* pHandle = _handles.Head(); pHandle; pHandle = accessor_type::Next(pHandle))
        ++metaClassCount;

    if (_guidOffset == size_t(-1))
        _guidOffset = MetaNamespaceGuidOffset_(metaClassCount);

    size_t metaClassIndex = 0;
    const TMemoryView<const u16> metaClassGuid = MakeView(GPrimeNumbersU16).SubRange(_guidOffset, metaClassCount);

    for (FMetaClassHandle* pHandle = _handles.Head(); pHandle; pHandle = accessor_type::Next(pHandle), ++metaClassIndex) {
        Assert(nullptr == pHandle->_metaClass);

        const FMetaClassGuid guid(metaClassGuid[metaClassIndex]);

        Assert(nullptr == pHandle->_metaClass);
        (*pHandle->_create)(&pHandle->_metaClass, guid, this);
        AssertRelease(pHandle->_metaClass);

        LOG(Info, L"[RTTI] Create meta class <{0}>::<{1}> ({2:#16x})",
            _nameTokenized, pHandle->_metaClass->Name(), pHandle->_metaClass->Guid());

        Assert(nullptr != pHandle->_metaClass);
        Assert(not pHandle->_metaClass->Name().empty());

        _metaClasses.emplace_AssertUnique(pHandle->_metaClass->Name(), pHandle->_metaClass);
    }

    if (_metaClasses.size()) {
        STACKLOCAL_POD_ARRAY(FMetaClass*, byInheritance, _metaClasses.size());
        size_t registered = 0;

        for (const auto& it : _metaClasses) {
            for (const FMetaClass* m = it.second; m; m = m->Parent()) {
                if (m->Namespace() == this &&
                    not byInheritance.CutBefore(registered).Contains(const_cast<FMetaClass*>(m)) )
                    byInheritance[registered++] = const_cast<FMetaClass*>(m);
            }
        }

        Assert(registered == _metaClasses.size());
        reverseforeachitem(it, byInheritance) {
            (*it)->Initialize();

            LOG(Info, L"[RTTI] Initialize meta class <{0}>::<{1}> ({2:#16x})",
                _nameTokenized, (*it)->Name(), (*it)->Guid() );
        }
    }

    MetaDB().RegisterNamespace(this);
}
//----------------------------------------------------------------------------
void FMetaNamespace::Shutdown() {
    AssertIsMainThread();
    Assert(_handles.empty() || not _metaClasses.empty());
    Assert(not _nameTokenized.empty());

    const FAtomicSpinLock::FScope scopeLock(GMetaNamespaceSpinLock);

    LOG(Info, L"[RTTI] Shutdown namespace <{0}>", _nameTokenized);

    MetaDB().UnregisterNamespace(this);

    typedef INTRUSIVELIST_ACCESSOR(&FMetaClassHandle::_node) accessor_type;
    for (FMetaClassHandle* pHandle = _handles.Head(); pHandle; pHandle = accessor_type::Next(pHandle)) {
        Assert(nullptr != pHandle->_metaClass);

        LOG(Info, L"[RTTI] Destroy meta class <{0}>::<{1}> ({2:#16x})",
            _nameTokenized, pHandle->_metaClass->Name(), pHandle->_metaClass->Guid());

#ifdef WITH_CORE_ASSERT
        Remove_AssertExistsAndSameValue(_metaClasses, pHandle->_metaClass->Name(), pHandle->_metaClass);
#endif

        checked_delete_ref(pHandle->_metaClass);
    }

    _metaClasses.clear_ReleaseMemory();

    _nameTokenized = FName();
}
//----------------------------------------------------------------------------
const FMetaClass* FMetaNamespace::FindClass(const FName& metaClassName) const {
    Assert(not metaClassName.empty());

    return _metaClasses.at(metaClassName);
}
//----------------------------------------------------------------------------
const FMetaClass* FMetaNamespace::FindClassIFP(const FName& metaClassName) const {
    Assert(not metaClassName.empty());

    const auto it = _metaClasses.find(metaClassName);
    return (_metaClasses.end() != it ? it->second : nullptr);
}
//----------------------------------------------------------------------------
void FMetaNamespace::AllClasses(VECTOR(RTTI, const FMetaClass*)& instances) const {
    instances.reserve_Additional(_metaClasses.size());
    const auto values = _metaClasses.Values();
    instances.insert(instances.end(), std::begin(values), std::end(values));
}
//----------------------------------------------------------------------------
void FMetaNamespace::Append_(FMetaClassHandle* pHandle) {
    Assert(pHandle);

    // thread safety for constants initialization in different threads :
    const FAtomicSpinLock::FScope scopeLock(GMetaNamespaceSpinLock);

    Assert(_metaClasses.empty()); // don't register while already started

    _handles.PushFront(pHandle);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
