#pragma once

#include "Core_fwd.h"

#include "Thread/Task_fwd.h"

#include "Container/Vector.h"
#include "Memory/WeakPtr.h"
#include "Meta/PointerWFlags.h"
#include "Thread/AtomicSpinLock.h"

#include <atomic>

namespace PPE {
class FTaskFiberLocalCache;
class FTaskManagerImpl;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Holds a fiber with its original ITaskContext* and priority for resuming
//----------------------------------------------------------------------------
using FTaskFiberRef = const void*;
class FInterruptedTask {
public:
    FInterruptedTask() = default;

    FInterruptedTask(
        ITaskContext& ctx,
        FTaskFiberRef fiber,
        ETaskPriority priority ) NOEXCEPT
    :   _fiber(fiber) {
        _taskContextAndPriority.Reset(&ctx, uintptr_t(priority));
    }

    FTaskFiberRef Fiber() const { return _fiber; }
    ITaskContext* Context() const { return _taskContextAndPriority.Get(); }
    ETaskPriority Priority() const { return ETaskPriority(_taskContextAndPriority.Flag01()); }

    friend bool operator <(const FInterruptedTask& lhs, const FInterruptedTask& rhs) NOEXCEPT {
        return (lhs.Priority() < rhs.Priority());
    }
    friend bool operator >=(const FInterruptedTask& lhs, const FInterruptedTask& rhs) NOEXCEPT {
        return (not operator <(lhs, rhs));
    }

    static PPE_CORE_API FTaskFunc ResumeTask(const FInterruptedTask& task);
    static PPE_CORE_API void Resume(const TMemoryView<FInterruptedTask>& tasks);

private:
    FTaskFiberRef _fiber;
    Meta::TPointerWFlags<ITaskContext> _taskContextAndPriority;
};
PPE_ASSUME_TYPE_AS_POD(FInterruptedTask)
//----------------------------------------------------------------------------
// This is the main synchronization API :
//  - use only when you need to wait for task(s) ;
//  - must be handled through ITaskContext ;
//  - supports weakref counting for thread-safe attachment ;
//  - see FAggregationPort bellow to wait for tasks already in-flight.
//----------------------------------------------------------------------------
FWD_WEAKPTR(CompletionPort);
class PPE_CORE_API FCompletionPort : public FWeakRefCountable, Meta::FNonCopyableNorMovable {
public:
    FCompletionPort() = default;

#if USE_PPE_ASSERT
    ~FCompletionPort();
#endif

    bool Running() const { return (_countDown >= 0); }
    bool Finished() const { return (CP_Finished == _countDown); }

private: // only accessible through ITaskContext
    friend class FTaskManagerImpl;

    void AttachCurrentFiber(FTaskFiberLocalCache& fibers, ETaskPriority priority) NOEXCEPT;
    void Start(size_t n) NOEXCEPT;
    void OnJobComplete();

private:
    friend class FAggregationPort;

    enum state_t : int {
        CP_NotReady = -1,
        CP_Finished = -2,
    };
    std::atomic<int> _countDown{ CP_NotReady };

    FAtomicSpinLock _barrier;
    VECTORINSITU(Task, FInterruptedTask, 8) _queue;
    VECTORINSITU(Task, FCompletionPort*, 8) _children;

    void ResetToNotReady_AssumeFinished_();

    static NO_INLINE void OnCountDownReachedZero_(FCompletionPort* port);
};
//----------------------------------------------------------------------------
// Can wait on multiple FCompletionPorts, *MUST* call Join() before destruction
//----------------------------------------------------------------------------
class PPE_CORE_API FAggregationPort {
public:
    FAggregationPort() NOEXCEPT;

    bool Running() const { return _port.Running(); }
    bool Finished() const { return _port.Finished(); }

    bool Attach(FCompletionPort* dep);
    bool Attach(FAggregationPort* other);

    void Join(ITaskContext& ctx);
    void JoinAndReset(ITaskContext& ctx);

private: // only accessible through ITaskContext
    friend class FTaskManagerImpl;

    FCompletionPort* Increment(size_t n) NOEXCEPT;

private:
    FCompletionPort _port;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
