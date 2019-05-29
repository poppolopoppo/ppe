#pragma once

#include "Meta/Assert.h"
#include "Meta/Cast.h"

#if USE_PPE_ASSERT
#   define WITH_PPE_THREADRESOURCE_CHECKS
#endif

#include <mutex>
#include <thread>

namespace PPE {
namespace Meta {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
using FLockGuard = std::lock_guard<std::mutex>;
using FUniqueLock = std::unique_lock<std::mutex>;
//----------------------------------------------------------------------------
using FRecursiveLockGuard = std::lock_guard<std::recursive_mutex>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_CORE_API FThreadResource {
public:
#ifdef WITH_PPE_THREADRESOURCE_CHECKS
    FThreadResource() : FThreadResource(std::this_thread::get_id()) {}
    explicit FThreadResource(std::thread::id threadId) : _threadId(threadId) {}
    ~FThreadResource() { Assert(std::this_thread::get_id() == _threadId); }
    std::thread::id ThreadId() const { return _threadId; }
    void CheckThreadId(std::thread::id threadId) const { Assert(threadId == _threadId); }
    void OwnedByThisThread() const { CheckThreadId(std::this_thread::get_id()); }
    void SetThreadId(std::thread::id threadId) { OwnedByThisThread(); _threadId = threadId; }
private:
    std::thread::id _threadId;
#else
    FThreadResource() {}
    ~FThreadResource() {}
    std::thread::id ThreadId() const { return std::thread::id(); }
    void CheckThreadId(std::thread::id) const {}
    void OwnedByThisThread() const {}
    void SetThreadId(std::thread::id ) {}
#endif
};
//----------------------------------------------------------------------------
#ifdef WITH_PPE_THREADRESOURCE_CHECKS
#   define THREADRESOURCE_CHECKACCESS(_pResource) (_pResource)->OwnedByThisThread()
#else
#   define THREADRESOURCE_CHECKACCESS(_pResource) NOOP()
#endif
//----------------------------------------------------------------------------
#define THIS_THREADRESOURCE_CHECKACCESS() THREADRESOURCE_CHECKACCESS(this)
//----------------------------------------------------------------------------
#define THREADRESOURCE_CHECKACCESS_IFN(_pResource) if (_pResource) THREADRESOURCE_CHECKACCESS(_pResource)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace PPE
