// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Thread/Task/CompletionPort.h"

#include "Thread/Task/Task.h"
#include "Thread/Task/TaskFiberPool.h"
#include "Thread/Task/TaskManagerImpl.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Check _taskContextAndPriority packing otherwise !
STATIC_ASSERT(uintptr_t(ETaskPriority::High) == 0);
STATIC_ASSERT(uintptr_t(ETaskPriority::Normal) == 1);
STATIC_ASSERT(uintptr_t(ETaskPriority::Low) == 2);
STATIC_ASSERT(uintptr_t(ETaskPriority::Internal) == 3);
// Also checks FCompletionPort alignment for packing in FTaskQueued
#if !USE_PPE_MEMORY_DEBUGGING
//STATIC_ASSERT(Meta::TCheckSizeAligned<FCompletionPort, 16>::value); #TODO: fix alignment automatically (FPolymorphicAllocator)
#endif
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
    // /!\ DO NOT EARLY OUT HERE, WE *NEED* TO LOCK ONCE TO BE THREAD-SAFE !!

    // *IMPORTANT* once in the waiting queue, any decref could resume the fiber,
    // so when a fiber is added it *MUST* be dormant and ready to be resumed.
    // This is why we have to switch to a new fiber to queue to the counter.

    FTaskFiberPool::FHandleRef const cur = FTaskFiberPool::CurrentHandleRef();
    FTaskFiberPool::FHandleRef const nxt = fibers.AcquireFiber();

    FInterruptedTask waiting{ ITaskContext::Get(), cur, priority };

    // The wakeup callback will be executed just after YieldFiber(), and will perform
    // the actual call to Queue(). If by this time the counter would have been
    // already finished then we'd switch directly back to the original fiber.

    nxt->AttachWakeUpCallback([this, waiting]() {
        _queue.push_back(waiting);
        _barrier.Unlock();
    });

    _barrier.Lock();

    if (CP_Finished == _countDown) {
        // task already completed, abort call to YieldFiber()
        _barrier.Unlock();

        nxt->OnWakeUp.Reset();
        fibers.ReleaseFiber(nxt);
    }
    else {
        // the lock will be closed *only* when the next fiber awaken
        cur->YieldFiber(nxt, false/* keep current fiber alive */);
    }
}
//----------------------------------------------------------------------------
void FCompletionPort::Start(size_t n) NOEXCEPT {
    Assert(n > 0);
    Assert_NoAssume(CP_NotReady == _countDown);

    // make sure port is kept alive while working with it
    AddRefIFP(this); // increment only if already ref-counted

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

    const int n = atomic_fetch_sub_explicit(&_countDown, 1, std::memory_order_release);
    if (1 == n) {
        OnCountDownReachedZero_(this);
    }
    else {
        Assert_NoAssume(n > 0);
    }
}
//----------------------------------------------------------------------------
void FCompletionPort::ResetToNotReady_AssumeFinished_() {
    Assert_Lightweight(CP_Finished == _countDown);

    ONLY_IF_ASSERT(const FCriticalScope scopeLock(&_barrier));

    Assert_NoAssume(Finished());
    Assert_NoAssume(_queue.empty());
    Assert_NoAssume(_children.empty());

#if USE_PPE_ASSERT
    int expected = CP_Finished;
    Verify(_countDown.compare_exchange_strong(expected, CP_NotReady));
#else
    _countDown = CP_NotReady;
#endif
}
//----------------------------------------------------------------------------
void FCompletionPort::OnCountDownReachedZero_(FCompletionPort* port) {
    Assert(port);

    PCompletionPort keepAlive;
    decltype(_queue) localQueue;
    decltype(_children) localChildren;
    {
        const FCriticalScope scopeLock(&port->_barrier);
        Assert_NoAssume(port->_countDown >= 0);

        AddRefIFP<FCompletionPort>(keepAlive, port);

        int expected = 0;
        if (port->_countDown.compare_exchange_weak(expected, CP_Finished,
                std::memory_order_acquire, std::memory_order_relaxed)) {
            Assert_NoAssume(port->Finished()); // sanity check
            Assert_NoAssume(port->_queue.size() < 100); // sanity check
            Assert_NoAssume(port->_children.size() < 100); // sanity check

            localQueue = std::move(port->_queue);
            localChildren = std::move(port->_children);

            Assert_NoAssume(port->_queue.empty());
            Assert_NoAssume(port->_children.empty());

            // release our internal ref (if ref-counted), see Start()
            RemoveRefIFP<FCompletionPort>(port); // won't release allocation thx to 'keepAlive'
            // /!\ beware that we still need to unlock the barrier here !
        }
        else {
            Assert_NoAssume(expected > 0);
            return; // something was pushed on this completion port, *BAILING*
        }
    }

    // release completion port immediately (if ref-counted)
    keepAlive.reset(); // must happen *outside* the barrier !
    ONLY_IF_ASSERT(port = nullptr); // port could be destroyed right after this barrier !

    // process collected data
    if (not localQueue.empty())
        FInterruptedTask::Resume(localQueue.MakeView());

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
    //Assert_NoAssume(_port.Running()); // not mandatory

    // early out before locking to avoid contention when finished
    if (FCompletionPort::CP_Finished != dep->_countDown) {

        // needs locking to avoid releasing the lock before we finish attaching
        const FCriticalScope scopeLock(&dep->_barrier);

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

    // increments the port, safe since we force lifetime until Join())
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
    _port.ResetToNotReady_AssumeFinished_();

    // need to keep the port alive until Join()
    _port.Start(1);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
