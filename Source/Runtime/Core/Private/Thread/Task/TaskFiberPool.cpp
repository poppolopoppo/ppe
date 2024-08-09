// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Thread/Task/TaskFiberPool.h"

#include "Allocator/Alloca.h" // for debug only
#include "Allocator/TrackingMalloc.h"
#include "Container/IntrusiveList.h"
#include "Memory/MemoryTracking.h"
#include "Meta/Singleton.h"
#include "Meta/Utility.h"
#include "Misc/Function.h"
#include "Thread/AtomicPool.h"

#if USE_PPE_SANITIZER
#include "HAL/PlatformMemory.h"

#include <sanitizer/asan_interface.h>
#include <sanitizer/hwasan_interface.h>
#endif

PRAGMA_MSVC_WARNING_PUSH()
PRAGMA_MSVC_WARNING_DISABLE(4324) // 'XXX' structure was padded due to alignment

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct CACHELINE_ALIGNED FTaskFiberPool::FHandle {
    using FWakeUpCallback = TFunctionRef<void()>;

    STATIC_CONST_INTEGRAL(size_t, StackSize, 57344 /* Don't use multiples of 64K to avoid D-cache aliasing conflicts. */);

    FFiber Fiber;
    FTaskFiberPool* Pool{ nullptr };

    mutable FHandleRef NextHandle{ nullptr };

    mutable FWakeUpCallback OnWakeUp;

#if USE_PPE_SANITIZER
    mutable FHandleRef ASan_Predecessor{ nullptr };

    mutable const void* ASan_NewStackBottom{ nullptr };
    mutable size_t ASan_NewStackSize{ 0 };

    mutable const void* ASan_OldStackBottom{ nullptr };
    mutable size_t ASan_OldStackSize{ 0 };
#endif

    void AttachWakeUpCallback(FWakeUpCallback&& onWakeUp) const {
        Assert(onWakeUp.Valid());
        Assert_NoAssume(Pool);
        Assert_NoAssume(not OnWakeUp.Valid());

        OnWakeUp = std::move(onWakeUp);
    }

    NORETURN void FiberCallback() const {
        Pool->_callback();
        AssertNotReached();
    }

    void ReleaseFiber() const {
        Pool->ReleaseFiber(this);
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
void STDCALL TaskFiberEntryPoint_(void* arg) {
    FTaskFiberPool::FHandleRef const self = static_cast<FTaskFiberPool::FHandleRef>(arg);

#if USE_PPE_SANITIZER
    const void* asan_oldStackBottom;
    size_t asan_oldStackSize;
    __sanitizer_finish_switch_fiber(nullptr/* first run for this fiber: no fake stack to restore */, &asan_oldStackBottom, &asan_oldStackSize);

    if (self->ASan_Predecessor != nullptr) {
        AssertRelease_NoAssume(self->ASan_OldStackBottom == asan_oldStackBottom);
        AssertRelease_NoAssume(self->ASan_OldStackSize == asan_oldStackSize);

        self->ASan_Predecessor = nullptr;
        self->ASan_OldStackBottom = nullptr;
        self->ASan_OldStackSize = 0;
    }

    const void* asan_newStackBottom;
    size_t asan_newStackSize;
    self->Fiber.StackRegion(&asan_newStackBottom, &asan_newStackSize);
    AssertRelease_NoAssume(self->ASan_NewStackBottom == asan_newStackBottom);
    AssertRelease_NoAssume(self->ASan_NewStackSize == asan_newStackSize);

    ONLY_IF_ASSERT_RELEASE(const FPlatformMemory::FStackUsage stackUsage = FPlatformMemory::StackUsage());
    AssertRelease_NoAssume(stackUsage.BaseAddr == asan_newStackBottom);
    AssertRelease_NoAssume(stackUsage.Committed == asan_newStackSize);
#endif

    Assert_NoAssume(FTaskFiberPool::CurrentHandleRef() == self);

    if (self->OnWakeUp.Valid())
        self->OnWakeUp.FireAndForget();

    self->FiberCallback();
}
//----------------------------------------------------------------------------
static void YieldTaskFiber_(FTaskFiberPool::FHandleRef self, FTaskFiberPool::FHandleRef to, bool release) {
    Assert(self);
    Assert_NoAssume(self == FTaskFiberPool::CurrentHandleRef());
    Assert_NoAssume(AllocaDepth() == 0); // can't switch fibers with live TLS block(s)
    Assert_NoAssume(not self->OnWakeUp.Valid());

    // prepare data for next fiber
    if (nullptr == to)
        to = self->Pool->AcquireFiber();

    Assert(to && to->Fiber);
    Assert(to != self);

    if (release) {
        TFunctionRef<void()> wakeUp{ Meta::StaticFunction<&FTaskFiberPool::FHandle::ReleaseFiber>, self };
        to->AttachWakeUpCallback(std::move(wakeUp));
    }

#if USE_PPE_SANITIZER
    AssertRelease_NoAssume(to->ASan_Predecessor == nullptr);

    to->ASan_Predecessor = self;
    to->Fiber.StackRegion(&to->ASan_NewStackBottom, &to->ASan_NewStackSize);
    self->Fiber.StackRegion(&to->ASan_OldStackBottom, &to->ASan_OldStackSize);

    void* asan_selfFakeStack = nullptr;
    __sanitizer_start_switch_fiber(&asan_selfFakeStack, to->ASan_NewStackBottom, to->ASan_NewStackSize);
#endif

    // <---- yield to another fiber
    to->Fiber.Resume();
    // ----> paused or released fiber gets resumed

#if USE_PPE_SANITIZER
    AssertRelease_NoAssume(self->ASan_Predecessor != nullptr);

    const void* asan_oldStackBottom;
    size_t asan_oldStackSize;
    __sanitizer_finish_switch_fiber(asan_selfFakeStack, &asan_oldStackBottom, &asan_oldStackSize);
    AssertRelease_NoAssume(self->ASan_OldStackBottom == asan_oldStackBottom);
    AssertRelease_NoAssume(self->ASan_OldStackSize == asan_oldStackSize);

    self->ASan_Predecessor = nullptr;
    self->ASan_OldStackBottom = nullptr;
    self->ASan_OldStackSize = 0;

    const void* asan_newStackBottom;
    size_t asan_newStackSize;
    self->Fiber.StackRegion(&asan_newStackBottom, &asan_newStackSize);
    AssertRelease_NoAssume(self->ASan_NewStackBottom == asan_newStackBottom);
    AssertRelease_NoAssume(self->ASan_NewStackSize == asan_newStackSize);

    ONLY_IF_ASSERT_RELEASE(const FPlatformMemory::FStackUsage stackUsage = FPlatformMemory::StackUsage());
    AssertRelease_NoAssume(stackUsage.BaseAddr == asan_newStackBottom);
    AssertRelease_NoAssume(stackUsage.Committed == asan_newStackSize);
#endif

    // wake up, you've been resumed
    Assert_NoAssume(self == FTaskFiberPool::CurrentHandleRef());

    if (self->OnWakeUp.Valid())
        self->OnWakeUp.FireAndForget();

    // resume what the fiber was doing before being interrupted
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTaskFiberPool::FTaskFiberPool(FCallback&& callback) NOEXCEPT
:   _callback(callback) {
    Assert_NoAssume(_callback);
}
//----------------------------------------------------------------------------
FTaskFiberPool::~FTaskFiberPool() {
    ReleaseMemory();

    Assert_NoAssume(nullptr == _freeFibers);
    Assert_NoAssume(0 == _numFibersAvailable);
    AssertRelease_NoAssume(0 == _numFibersReserved);
}
//----------------------------------------------------------------------------
bool FTaskFiberPool::OwnsFiber(FHandleRef handle) const NOEXCEPT {
    Assert(handle);
    return (handle->Pool == this);
}
//----------------------------------------------------------------------------
auto FTaskFiberPool::AcquireFiber() -> FHandleRef {
    for (FHandleRef freeFiber = _freeFibers.load(std::memory_order_relaxed); freeFiber;) {
        if (Likely(freeFiber && _freeFibers.compare_exchange_weak(freeFiber, freeFiber->NextHandle,
            std::memory_order_release, std::memory_order_relaxed))) {
            freeFiber->NextHandle = nullptr;

            ONLY_IF_MEMORYDOMAINS(MEMORYDOMAIN_TRACKING_DATA(Fibers).AllocateUser(FHandle::StackSize));

#if !USE_PPE_FINAL_RELEASE
            _numFibersAvailable.fetch_sub(1);
#endif
            return freeFiber;
        }
    }

    FHandle* const newFiber = TRACKING_NEW(Fibers, FHandle);
    newFiber->Pool = this;
    newFiber->Fiber.Create(&TaskFiberEntryPoint_, newFiber, FHandle::StackSize);

#if !USE_PPE_FINAL_RELEASE
    _numFibersReserved.fetch_add(1);
#endif
    ONLY_IF_MEMORYDOMAINS(MEMORYDOMAIN_TRACKING_DATA(Fibers).AllocateUser(FHandle::StackSize));

    return newFiber;
}
//----------------------------------------------------------------------------
void FTaskFiberPool::ReleaseFiber(FHandleRef handle) {
    Assert(handle);
    Assert_NoAssume(OwnsFiber(handle));
    Assert_NoAssume(not handle->OnWakeUp.Valid());
    Assert_NoAssume(nullptr == handle->NextHandle);

    for (handle->NextHandle = _freeFibers.load(std::memory_order_relaxed);;) {
        if (_freeFibers.compare_exchange_weak(handle->NextHandle, handle,
            std::memory_order_release, std::memory_order_relaxed)) {
            ONLY_IF_MEMORYDOMAINS(MEMORYDOMAIN_TRACKING_DATA(Fibers).DeallocateUser(FHandle::StackSize));

#if !USE_PPE_FINAL_RELEASE
            _numFibersAvailable.fetch_add(1);
#endif
            return;
        }
    }
}
//----------------------------------------------------------------------------
void FTaskFiberPool::ReleaseMemory() {
    for (FHandleRef freeFiber = _freeFibers.load(std::memory_order_relaxed); freeFiber; ) {
        if (_freeFibers.compare_exchange_weak(freeFiber, freeFiber->NextHandle,
            std::memory_order_release, std::memory_order_relaxed)) {

            Assert_NoAssume(OwnsFiber(freeFiber));
            Assert_NoAssume(not freeFiber->OnWakeUp.Valid());

            FHandle* const releasedFiber = const_cast<FHandle*>(freeFiber);
            freeFiber = freeFiber->NextHandle;

            releasedFiber->Fiber.Destroy(FHandle::StackSize);

#if !USE_PPE_FINAL_RELEASE
            VerifyRelease(_numFibersAvailable.fetch_sub(1) > 0);
            VerifyRelease(_numFibersReserved.fetch_sub(1) > 0);
#endif

            TRACKING_DELETE(Fibers, releasedFiber);
        }
    }
}
//----------------------------------------------------------------------------
void FTaskFiberPool::YieldCurrentFiber(FHandleRef to, bool release) {
    YieldTaskFiber_(CurrentHandleRef(), to, release);
}
//----------------------------------------------------------------------------
void FTaskFiberPool::YieldFiber(FHandleRef self, FHandleRef to, bool release) {
    Assert_NoAssume(OwnsFiber(self));

    YieldTaskFiber_(self, to, release);
}
//----------------------------------------------------------------------------
void FTaskFiberPool::AttachWakeUpCallback(FHandleRef fiber, TFunction<void()>&& onWakeUp) {
    fiber->AttachWakeUpCallback(std::move(onWakeUp));
}
//----------------------------------------------------------------------------
void FTaskFiberPool::ResetWakeUpCallback(FHandleRef fiber) {
    fiber->OnWakeUp.Reset();
}
//----------------------------------------------------------------------------
size_t FTaskFiberPool::ReservedStackSize() NOEXCEPT {
    return FHandle::StackSize;
}
//----------------------------------------------------------------------------
void FTaskFiberPool::StartThread() {
    Assert_NoAssume(FFiber::RunningFiber() == FFiber::ThreadFiber());

    FHandleRef const workerFiber = AcquireFiber();

#if USE_PPE_SANITIZER
    AssertRelease_NoAssume(workerFiber->ASan_Predecessor == nullptr);

    workerFiber->Fiber.StackRegion(&workerFiber->ASan_NewStackBottom, &workerFiber->ASan_NewStackSize);

    void* asan_threadFakeStack = nullptr;
    __sanitizer_start_switch_fiber(&asan_threadFakeStack, workerFiber->ASan_NewStackBottom, workerFiber->ASan_NewStackSize);
#endif

    // <---- yield to a new fiber with a worker loop
    workerFiber->Fiber.Resume();
    // ----> leave fiber later when the worker loop broke

#if USE_PPE_SANITIZER
    __sanitizer_finish_switch_fiber(asan_threadFakeStack, nullptr, nullptr);
#endif
}
//----------------------------------------------------------------------------
void FTaskFiberPool::ShutdownThread() {
    Assert_NoAssume(FFiber::RunningFiber() != FFiber::ThreadFiber());

    const FFiber threadFiber = FFiber::ThreadFiber();

#if USE_PPE_SANITIZER
    AssertRelease_NoAssume(CurrentHandleRef()->ASan_Predecessor == nullptr);

    const void* asan_threadStackBottom;
    size_t asan_threadStackSize;
    threadFiber.StackRegion(&asan_threadStackBottom, &asan_threadStackSize);

    __sanitizer_start_switch_fiber(nullptr, asan_threadStackBottom, asan_threadStackSize);
#endif

    // <---- yield to a original thread fiber
    threadFiber.Resume();
    // ----> this will *NEVER* be executed

    AssertNotReached();
}
//----------------------------------------------------------------------------
#if !USE_PPE_FINAL_RELEASE
void FTaskFiberPool::UsageStats(size_t* reserved, size_t* inUse) NOEXCEPT {
    std::atomic_thread_fence(std::memory_order_acquire);
    *reserved = checked_cast<size_t>(_numFibersReserved.load(std::memory_order_relaxed));
    *inUse = *reserved - checked_cast<size_t>(_numFibersAvailable.load(std::memory_order_relaxed));
}
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Meant to be used as a thread local cache to remove contention on FTaskFiberPool
//----------------------------------------------------------------------------
FTaskFiberLocalCache::FTaskFiberLocalCache(FTaskFiberPool& pool) NOEXCEPT
:   _pool(pool)
{}
//----------------------------------------------------------------------------
FTaskFiberLocalCache::~FTaskFiberLocalCache() {
    ReleaseMemory();
}
//----------------------------------------------------------------------------
auto FTaskFiberLocalCache::AcquireFiber() -> FHandleRef {
    THIS_THREADRESOURCE_CHECKACCESS();

    FHandleRef result = nullptr;
    if (_lastFreeFiber == nullptr) {
        result = _pool.AcquireFiber();
    }
    else {
        result = _lastFreeFiber;
        _lastFreeFiber = nullptr;
    }

    Assert(result);
    Assert_NoAssume(_pool.OwnsFiber(result));
    return result;
}
//----------------------------------------------------------------------------
void FTaskFiberLocalCache::ReleaseFiber(FHandleRef handle) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(handle);
    Assert_NoAssume(_pool.OwnsFiber(handle));
    Assert_NoAssume(not handle->OnWakeUp.Valid());

#if 0
    // release all when full, to avoid releasing at every call
    if (_lastFreeFiber != nullptr)
        ReleaseMemory();

    _lastFreeFiber = handle;
#else
    // always give back to global pool to avoid over allocation of fibers
    _pool.ReleaseFiber(handle);
#endif
}
//----------------------------------------------------------------------------
void FTaskFiberLocalCache::ReleaseMemory() {
    THIS_THREADRESOURCE_CHECKACCESS();

    if (_lastFreeFiber != nullptr) {
        _pool.ReleaseFiber(_lastFreeFiber);
        _lastFreeFiber = nullptr;
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void* FGlobalFiberPool::class_singleton_storage() NOEXCEPT {
    return singleton_type::make_singleton_storage(); // for shared libs
}
//----------------------------------------------------------------------------
void FGlobalFiberPool::Create(FCallback&& callback) {
    singleton_type::Create(std::move(callback));
}
//----------------------------------------------------------------------------
void FGlobalFiberPool::Destroy() {
    singleton_type::Destroy();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

PRAGMA_MSVC_WARNING_POP()
