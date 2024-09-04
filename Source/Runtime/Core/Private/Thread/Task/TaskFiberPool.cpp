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
    using FWakeUpEvent = FTaskFiberPool::FWakeUpEvent;

    STATIC_CONST_INTEGRAL(size_t, StackSize, 57344 /* Don't use multiples of 64K to avoid D-cache aliasing conflicts. */);

    FFiber Fiber_DoNotUse;
    FTaskFiberPool* Pool{ nullptr };

    mutable const FHandle* NextHandle{ nullptr };

    mutable FWakeUpEvent OnWakeUp;

#if USE_PPE_SANITIZER
    mutable FHandleRef ASan_Predecessor{ nullptr };

    mutable const void* ASan_NewStackBottom{ nullptr };
    mutable size_t ASan_NewStackSize{ 0 };

    mutable const void* ASan_OldStackBottom{ nullptr };
    mutable size_t ASan_OldStackSize{ 0 };
#endif

#if USE_PPE_ASSERT
    enum EDebugState {
        STATE_Free = 0,
        STATE_Resuming,
        STATE_Running,
        STATE_Stalled,
        STATE_Recycling,
        STATE_Destroyed,
    };

    mutable std::atomic<EDebugState> DebugState{ STATE_Free };
    bool AnyDebugState(std::initializer_list<EDebugState> any) const {
        const EDebugState state = DebugState;
        for (const EDebugState cmp : any) {
            if (cmp == state)
                return true;
        }
        return false;
    }
    EDebugState SetDebugState(EDebugState expected, EDebugState desired) const {
        VerifyRelease( DebugState.compare_exchange_strong(expected, desired) );
        return expected;
    }
#endif

    void AttachWakeUpCallback(FWakeUpEvent&& onWakeUp) const {
        Assert_NoAssume(STATE_Stalled == DebugState);
        Assert(onWakeUp.Valid());
        Assert_NoAssume(Pool);
        Assert_NoAssume(not OnWakeUp.Valid());

        OnWakeUp = std::move(onWakeUp);
    }

    NORETURN void FiberCallback() const {
        Assert_NoAssume(STATE_Running == DebugState);

        Pool->_callback();

        AssertNotReached();
    }

    void ResumeFiber() const {
        ONLY_IF_ASSERT(SetDebugState(FHandle::STATE_Stalled, FHandle::STATE_Resuming));

        Fiber_DoNotUse.Resume();
    }

    void ReleaseFiber() const {
        ONLY_IF_ASSERT(SetDebugState(FHandle::STATE_Stalled, FHandle::STATE_Recycling));

        Pool->ReleaseFiber(this);
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
void STDCALL TaskFiberEntryPoint_(void* arg) {
    FTaskFiberPool::FHandleRef const self = static_cast<const FTaskFiberPool::FHandle*>(arg);

    ONLY_IF_ASSERT(self->SetDebugState(FTaskFiberPool::FHandle::STATE_Resuming, FTaskFiberPool::FHandle::STATE_Running));

#if USE_PPE_SANITIZER
    const void* asan_oldStackBottom;
    size_t asan_oldStackSize;
    __sanitizer_finish_switch_fiber(nullptr/* first run for this fiber: no fake stack to restore */, &asan_oldStackBottom, &asan_oldStackSize);

    if (self->ASan_Predecessor.valid()) {
        AssertRelease_NoAssume(self->ASan_OldStackBottom == asan_oldStackBottom);
        AssertRelease_NoAssume(self->ASan_OldStackSize == asan_oldStackSize);

        self->ASan_Predecessor.reset();
        self->ASan_OldStackBottom = nullptr;
        self->ASan_OldStackSize = 0;
    }

    const void* asan_newStackBottom;
    size_t asan_newStackSize;
    self->Fiber_DoNotUse.StackRegion(&asan_newStackBottom, &asan_newStackSize);
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

    ONLY_IF_ASSERT(self->SetDebugState(FTaskFiberPool::FHandle::STATE_Running, FTaskFiberPool::FHandle::STATE_Stalled));

    // prepare data for next fiber
    if (nullptr == to)
        to = self->Pool->AcquireFiber();

    Assert(to && to->Fiber_DoNotUse);
    AssertRelease(to != self and to->Fiber_DoNotUse != self->Fiber_DoNotUse);

    if (release)
        to->AttachWakeUpCallback(FTaskFiberPool::FWakeUpEvent::Bind<&FTaskFiberPool::FHandle::ReleaseFiber>( self.get() ));

#if USE_PPE_SANITIZER
    AssertRelease_NoAssume(not to->ASan_Predecessor.valid());

    to->ASan_Predecessor = self;
    to->Fiber_DoNotUse.StackRegion(&to->ASan_NewStackBottom, &to->ASan_NewStackSize);
    self->Fiber_DoNotUse.StackRegion(&to->ASan_OldStackBottom, &to->ASan_OldStackSize);

    void* asan_selfFakeStack = nullptr;
    __sanitizer_start_switch_fiber(&asan_selfFakeStack, to->ASan_NewStackBottom, to->ASan_NewStackSize);
#endif

    // <---- yield to another fiber
    to->ResumeFiber();
    // ----> paused or released fiber gets resumed

    ONLY_IF_ASSERT(self->SetDebugState(FTaskFiberPool::FHandle::STATE_Resuming, FTaskFiberPool::FHandle::STATE_Running));

#if USE_PPE_SANITIZER
    AssertRelease_NoAssume(self->ASan_Predecessor.valid());

    const void* asan_oldStackBottom;
    size_t asan_oldStackSize;
    __sanitizer_finish_switch_fiber(asan_selfFakeStack, &asan_oldStackBottom, &asan_oldStackSize);
    AssertRelease_NoAssume(self->ASan_OldStackBottom == asan_oldStackBottom);
    AssertRelease_NoAssume(self->ASan_OldStackSize == asan_oldStackSize);

    self->ASan_Predecessor.reset();
    self->ASan_OldStackBottom = nullptr;
    self->ASan_OldStackSize = 0;

    const void* asan_newStackBottom;
    size_t asan_newStackSize;
    self->Fiber_DoNotUse.StackRegion(&asan_newStackBottom, &asan_newStackSize);
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
    for (const FHandle* freeFiber = _freeFibers.load(std::memory_order_relaxed); freeFiber; ) {
        if (Likely(_freeFibers.compare_exchange_weak(freeFiber, freeFiber->NextHandle,
            std::memory_order_release, std::memory_order_relaxed))) {
            ONLY_IF_ASSERT(freeFiber->SetDebugState(FHandle::STATE_Free, FHandle::STATE_Stalled));

            freeFiber->NextHandle = nullptr;

            ONLY_IF_MEMORYDOMAINS(MEMORYDOMAIN_TRACKING_DATA(Fibers).AllocateUser(FHandle::StackSize));

#if !USE_PPE_FINAL_RELEASE
            Verify( _numFibersAvailable.fetch_sub(1) > 0 );
#endif
            return freeFiber;
        }
    }

    FHandle* const newFiber = TRACKING_NEW(Fibers, FHandle);

    newFiber->Pool = this;
    newFiber->Fiber_DoNotUse.Create(&TaskFiberEntryPoint_, newFiber, FHandle::StackSize);

#if !USE_PPE_FINAL_RELEASE
    _numFibersReserved.fetch_add(1);
#endif

    ONLY_IF_ASSERT(newFiber->SetDebugState(FHandle::STATE_Free, FHandle::STATE_Stalled));
    ONLY_IF_MEMORYDOMAINS(MEMORYDOMAIN_TRACKING_DATA(Fibers).AllocateUser(FHandle::StackSize));

    return newFiber;
}
//----------------------------------------------------------------------------
void FTaskFiberPool::ReleaseFiber(FHandleRef handle) {
    Assert(handle);
    Assert_NoAssume(OwnsFiber(handle));
    Assert_NoAssume(not handle->OnWakeUp.Valid());
    Assert_NoAssume(nullptr == handle->NextHandle);

    ONLY_IF_ASSERT(handle->SetDebugState(FHandle::STATE_Recycling, FHandle::STATE_Free));

    for (handle->NextHandle = _freeFibers.load(std::memory_order_relaxed);;) {
        if (_freeFibers.compare_exchange_weak(handle->NextHandle, handle,
            std::memory_order_release, std::memory_order_relaxed)) {
            ONLY_IF_MEMORYDOMAINS(MEMORYDOMAIN_TRACKING_DATA(Fibers).DeallocateUser(FHandle::StackSize));

#if !USE_PPE_FINAL_RELEASE
            Verify( _numFibersAvailable.fetch_add(1) <= _numFibersReserved );
#endif
            return;
        }
    }
}
//----------------------------------------------------------------------------
void FTaskFiberPool::ReleaseMemory() {
    for (const FHandle* freeFiber = _freeFibers.load(std::memory_order_relaxed); freeFiber; ) {
        if (_freeFibers.compare_exchange_weak(freeFiber, freeFiber->NextHandle,
            std::memory_order_release, std::memory_order_relaxed)) {

            ONLY_IF_ASSERT(freeFiber->SetDebugState(FHandle::STATE_Free, FHandle::STATE_Destroyed));

            Assert_NoAssume(OwnsFiber(freeFiber));
            Assert_NoAssume(not freeFiber->OnWakeUp.Valid());

            FHandle* const releasedFiber = const_cast<FHandle*>(freeFiber);
            freeFiber = freeFiber->NextHandle;

            releasedFiber->Fiber_DoNotUse.Destroy(FHandle::StackSize);

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
void FTaskFiberPool::AttachWakeUpCallback(FHandleRef fiber, FWakeUpEvent&& onWakeUp) {
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
    AssertRelease_NoAssume(not workerFiber->ASan_Predecessor.valid());

    workerFiber->Fiber_DoNotUse.StackRegion(&workerFiber->ASan_NewStackBottom, &workerFiber->ASan_NewStackSize);

    void* asan_threadFakeStack = nullptr;
    __sanitizer_start_switch_fiber(&asan_threadFakeStack, workerFiber->ASan_NewStackBottom, workerFiber->ASan_NewStackSize);
#endif

    // <---- yield to a new fiber with a worker loop
    workerFiber->ResumeFiber();
    // ----> leave fiber later when the worker loop broke

#if USE_PPE_SANITIZER
    __sanitizer_finish_switch_fiber(asan_threadFakeStack, nullptr, nullptr);
#endif
}
//----------------------------------------------------------------------------
void FTaskFiberPool::ShutdownThread() {
    Assert_NoAssume(FFiber::RunningFiber() != FFiber::ThreadFiber());

    const FFiber threadFiber = FFiber::ThreadFiber();

    ONLY_IF_ASSERT(CurrentHandleRef()->SetDebugState(FHandle::STATE_Running, FHandle::STATE_Stalled));

#if USE_PPE_SANITIZER
    AssertRelease_NoAssume(not CurrentHandleRef()->ASan_Predecessor.valid());

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

    FHandleRef result = _lastFreeFiber;
    if (result) {
        ONLY_IF_ASSERT(result->SetDebugState(FTaskFiberPool::FHandle::STATE_Free, FTaskFiberPool::FHandle::STATE_Stalled));

        _lastFreeFiber = nullptr;
    }
    else
        result = _pool.AcquireFiber();

    Assert(result);
    Assert_NoAssume(_pool.OwnsFiber(result));
    return result;
}
//----------------------------------------------------------------------------
void FTaskFiberLocalCache::ReleaseFiber(FHandleRef handle) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(handle);
    Assert_NoAssume(handle->Pool == &_pool);
    Assert_NoAssume(_pool.OwnsFiber(handle));
    Assert_NoAssume(not handle->OnWakeUp.Valid());

#if 0
    ONLY_IF_ASSERT(result->SetDebugState(FTaskFiberPool::FHandle::STATE_Recycling, FTaskFiberPool::FHandle::STATE_Free));

    // release all when full, to avoid releasing at every call
    if (_lastFreeFiber != nullptr)
        ReleaseMemory();

    _lastFreeFiber = handle;
#else
    // always give back to global pool to avoid over allocation of fibers
    handle->ReleaseFiber();
#endif
}
//----------------------------------------------------------------------------
void FTaskFiberLocalCache::ReleaseMemory() {
    THIS_THREADRESOURCE_CHECKACCESS();

    if (_lastFreeFiber != nullptr) {
        Assert_NoAssume(_lastFreeFiber ->Pool == &_pool);
        ONLY_IF_ASSERT(_lastFreeFiber->SetDebugState(FTaskFiberPool::FHandle::STATE_Free, FTaskFiberPool::FHandle::STATE_Stalled));

        _lastFreeFiber->ReleaseFiber();
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
