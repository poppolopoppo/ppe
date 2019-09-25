#include "stdafx.h"

#include "Thread/Task/CompletionPort.h"

#include "Thread/Task/TaskManagerImpl.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
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
    if (Unlikely(not Running()))
        return;

    // *IMPORTANT* once in the waiting queue, any decref could resume the fiber,
    // so when a fiber is added it *MUST* be dormant and ready to be resumed.
    // This is why we have to switch to a new fiber to queue to the counter.

    FTaskFiberPool::FHandleRef const cur = FTaskFiberPool::CurrentHandleRef();
    FTaskFiberPool::FHandleRef const nxt = fibers.AcquireFiber();

    FInteruptedTask waiting{ ITaskContext::Get(), cur, priority };

    // The wakeup callback will be executed just after the yield, and will perform
    // the actual call to Queue(). If by this time the counter would have been
    // already finished then we'd switch directly back to this current fiber.

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
void FCompletionPort::DependsOn(FCompletionPort* other) {
    Assert(other);

    const FAtomicSpinLock::FScope scopeLock(other->_barrier);

    if (CP_Finished != other->_countDown) {
        Increment(1);
        Add_AssertUnique(other->_children, this);
    }
}
//----------------------------------------------------------------------------
void FCompletionPort::Start(size_t n) NOEXCEPT {
    Assert(n > 0);
    Assert_NoAssume(CP_NotReady == _countDown);

#if USE_PPE_ASSERT
    int expected = CP_NotReady;
    Verify(_countDown.compare_exchange_strong(expected, checked_cast<int>(n)));
#else
    _countDown = checked_cast<int>(n);
#endif
}
//----------------------------------------------------------------------------
void FCompletionPort::Increment(size_t n) NOEXCEPT {
    Assert(n > 0);

    // need to lock when incrementing to avoid finishing the counter
    const FAtomicSpinLock::FScope scopeLock(_barrier);

    Assert_NoAssume(_countDown > 0); // *MUST* be kept alive with FWriteScope
    Assert_NoAssume(_queue.empty());
    Assert_NoAssume(_children.empty());

    _countDown += checked_cast<int>(n);
}
//----------------------------------------------------------------------------
void FCompletionPort::OnJobComplete() {
    Assert_NoAssume(_countDown > 0);

    const size_t n = --_countDown;
    if (0 == n)
        OnCountDownReachedZero_(this);
    else
        Assert_NoAssume(n > 0);
}
//----------------------------------------------------------------------------
void FCompletionPort::OnCountDownReachedZero_(FCompletionPort* port) {
    Assert(port);

    decltype(_queue) localQueue;
    decltype(_children) localChildren;

    for (size_t backoff = 0;;) {
        const FAtomicSpinLock::FTryScope scopeLock(port->_barrier);
        if (scopeLock.Locked) {
            Assert_NoAssume(port->_countDown >= 0);

            int expected = 0;
            if (port->_countDown.compare_exchange_strong(expected, CP_Finished)) {
                localQueue = std::move(port->_queue);
                localChildren = std::move(port->_children);

                ONLY_IF_ASSERT(port = nullptr);
                break; // outside of the loop the lifetime of the port isn't guaranteed
            }
            else {
                Assert_NoAssume(expected > 0);
                return; // something was pushed on this completion port, *BAILING*
            }
        }
        else {
            FPlatformProcess::SleepForSpinning(backoff);
        }
    }

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
    _port.Start(1); // need to keep the port alive until Join()
}
//----------------------------------------------------------------------------
void FAggregationPort::DependsOn(FCompletionPort* other) {
    _port.DependsOn(other);
}
//----------------------------------------------------------------------------
void FAggregationPort::Join(ITaskContext& ctx) {
    Assert_NoAssume(_port.Running()); // should be alive thanks to fake ref
    _port.OnJobComplete(); // remove fake ref
    ctx.WaitFor(_port); // block wait
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
