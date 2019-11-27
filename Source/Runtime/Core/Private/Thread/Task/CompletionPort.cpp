#include "stdafx.h"

#include "Thread/Task/CompletionPort.h"

#include "Thread/Task/Task.h"
#include "Thread/Task/TaskFiberPool.h"
#include "Thread/Task/TaskManagerImpl.h"

#define USE_PPE_COMPLETIONPORT_LIFETIME_CHECKS (0) //%_NOCOMMIT%

#if USE_PPE_COMPLETIONPORT_LIFETIME_CHECKS
#   include "Container/HashHelpers.h"
#   include "Container/HashSet.h"
#   include <mutex>
#endif

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
#if USE_PPE_COMPLETIONPORT_LIFETIME_CHECKS
struct FCompletionPortHolder_ {
    using port_t = THashMemoizer<FCompletionPort*>;
    struct bucket_t {
        std::mutex Barrier;
        HASHSET(Task, port_t) Ports;
    };
    STATIC_CONST_INTEGRAL(size_t, NumBuckets, 0x29/*0x89*/);
    bucket_t Buckets[NumBuckets];

    FCompletionPortHolder_() = default;
    ~FCompletionPortHolder_() {
        forrange(b, Buckets, Buckets + NumBuckets) {
            const Meta::FLockGuard scopeLock(b->Barrier);
            AssertRelease(b->Ports.empty());
        }
    }

    bucket_t& Bucket(hash_t h) { return Buckets[h % NumBuckets]; }

    void Add(FCompletionPort& port) {
        const hash_t h = hash_ptr(&port);
        bucket_t& b = Bucket(h);
        const Meta::FLockGuard scopeLock(b.Barrier);
        Insert_AssertUnique(b.Ports, port_t{ &port, h });
    }

    void Remove(FCompletionPort& port) {
        const hash_t h = hash_ptr(&port);
        bucket_t& b = Bucket(h);
        const Meta::FLockGuard scopeLock(b.Barrier);
        Remove_AssertExists(b.Ports, port_t{ &port, h });
    }
};
static FCompletionPortHolder_ GCompletionsPorts;
#endif //!USE_PPE_COMPLETIONPORT_LIFETIME_CHECKS
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Check _taskContextAndPriority packing otherwise !
STATIC_ASSERT(uintptr_t(ETaskPriority::High) == 0);
STATIC_ASSERT(uintptr_t(ETaskPriority::Normal) == 1);
STATIC_ASSERT(uintptr_t(ETaskPriority::Low) == 2);
STATIC_ASSERT(uintptr_t(ETaskPriority::Internal) == 3);
//----------------------------------------------------------------------------
#if USE_PPE_ASSERT
FCompletionPort::~FCompletionPort() {
    Assert((CP_NotReady == _countDown) || (CP_Finished == _countDown));
    Assert(_queue.empty());
    Assert(_children.empty());
}
#endif //!USE_PPE_ASSERT
//----------------------------------------------------------------------------
void FCompletionPort::AttachCurrentFiber(FTaskFiberLocalCache& fibers, ETaskPriority priority) NOEXCEPT {
    // check that the port is still running before allocating and yielding to a new fiber
    if (Unlikely(_countDown == CP_Finished))
        return;

    // *IMPORTANT* once in the waiting queue, any decref could resume the fiber,
    // so when a fiber is added it *MUST* be dormant and ready to be resumed.
    // This is why we have to switch to a new fiber to queue to the counter.

    FTaskFiberPool::FHandleRef const cur = FTaskFiberPool::CurrentHandleRef();
    FTaskFiberPool::FHandleRef const nxt = fibers.AcquireFiber();

    FInteruptedTask waiting{ ITaskContext::Get(), cur, priority };

    // The wakeup callback will be executed just after the yield, and will perform
    // the actual call to Queue(). If by this time the counter would have been
    // already finished then we'd switch directly back to the original fiber.

    nxt->AttachWakeUpCallback([this, waiting]() {
        _queue.push_back(waiting);
        _barrier.Unlock();
    });

    _barrier.Lock();

    if (CP_Finished == _countDown) {
        _barrier.Unlock();

        nxt->OnWakeUp.Reset();
        fibers.ReleaseFiber(nxt);
    }
    else {
        // the lock will be closed when the next fiber awaken
        cur->YieldFiber(nxt, false/* keep current fiber alive */);
    }
}
//----------------------------------------------------------------------------
void FCompletionPort::Start(size_t n) NOEXCEPT {
    Assert(n > 0);
    Assert_NoAssume(CP_NotReady == _countDown);

#if USE_PPE_COMPLETIONPORT_LIFETIME_CHECKS
    GCompletionsPorts.Add(*this);
#endif

#if USE_PPE_ASSERT
    int expected = CP_NotReady;
    Verify(_countDown.compare_exchange_strong(expected, checked_cast<int>(n)));
#else
    _countDown = checked_cast<int>(n);
#endif
}
//----------------------------------------------------------------------------
void FCompletionPort::OnJobComplete() {
    Assert_NoAssume(_countDown > 0);

    const size_t n = atomic_fetch_sub_explicit(&_countDown, 1, std::memory_order_relaxed);
    if (1 == n) {
        OnCountDownReachedZero_(this);
    }
    else {
        Assert_NoAssume(n > 0);
    }
}
//----------------------------------------------------------------------------
void FCompletionPort::OnCountDownReachedZero_(FCompletionPort* port) {
    Assert(port);

    decltype(_queue) localQueue;
    decltype(_children) localChildren;
    {
        const FAtomicSpinLock::FScope scopeLock(port->_barrier);
        Assert_NoAssume(port->_countDown >= 0);

        int expected = 0;
        if (port->_countDown.compare_exchange_weak(expected, CP_Finished,
                std::memory_order_release, std::memory_order_relaxed)) {
            Assert_NoAssume(port->Finished());

            localQueue = std::move(port->_queue);
            localChildren = std::move(port->_children);

            ONLY_IF_ASSERT(port = nullptr);
        }
        else {
            Assert_NoAssume(expected > 0);
            return; // something was pushed on this completion port, *BAILING*
        }
    }

#if USE_PPE_COMPLETIONPORT_LIFETIME_CHECKS
    GCompletionsPorts.Remove(*port);
#endif

    if (not localQueue.empty())
        FInteruptedTask::Resume(localQueue.MakeView());

    // decrement the ref of potentially attached parent counters
    // only need to decrement the parents once, since their _countDown was incremented from 1 only once
    for (FCompletionPort* child : localChildren)
        child->OnJobComplete();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FAggregationPort::FAggregationPort() NOEXCEPT {
    // need to keep the port alive until Join()
    _port.Start(1);
}
//----------------------------------------------------------------------------
bool FAggregationPort::Attach(FCompletionPort* dep) {
    Assert(dep);
    Assert_NoAssume(dep != &_port);
    Assert_NoAssume(_port.Running());

    // early out before locking to avoid contention when finished
    if (FCompletionPort::CP_Finished != dep->_countDown) {
        // needs locking to avoid releasing the lock before we finish attaching
        const FAtomicSpinLock::FScope scopeLock(dep->_barrier);

        if (FCompletionPort::CP_Finished != dep->_countDown) {
            Assert_NoAssume(not Contains(_port._children, dep)); // *circular dependencies* !

            Add_AssertUnique(dep->_children, Increment(1));
            return true;
        }
    }

    return false;
}
//----------------------------------------------------------------------------
bool FAggregationPort::Attach(FAggregationPort* other) {
    Assert(other);

    return Attach(&other->_port);
}
//----------------------------------------------------------------------------
FCompletionPort* FAggregationPort::Increment(size_t n) NOEXCEPT {
    Assert(_port.Running());

    // increments the port, safe since we force lifetime until Join()

    _port._countDown += checked_cast<int>(n);

    return (&_port);
}
//----------------------------------------------------------------------------
void FAggregationPort::Join(ITaskContext& ctx) {
    Assert_NoAssume(_port.Running()); // should be alive thanks to fake ref

    _port.OnJobComplete(); // remove fake ref added in ctor
    ctx.WaitFor(_port); // block wait

    Assert_NoAssume(_port.Finished()); // should be finished, and not reset-able
}
//----------------------------------------------------------------------------
void FAggregationPort::JoinAndReset(ITaskContext& ctx) {
    Join(ctx);

    // reset the completion in-place
    Meta::Destroy(&_port);
    Meta::Construct(&_port);

    // need to keep the port alive until Join()
    _port.Start(1);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
